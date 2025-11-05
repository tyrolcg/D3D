#include "ImGuiIntegration.h"
#include "../../Library/imgui/imgui.h"
#include "../../Library/imgui/imgui_impl_win32.h"
#include "../../Library/imgui/imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi.h>
#include <stdexcept>


static IDXGIFactory* g_dummy = nullptr;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
static ID3D12Resource* g_pFontTexture = nullptr; // フォントテクスチャ保持
static int g_NumFrames = 0;

// Note: now accepts an uploadQueue pointer
bool ImGui_Init(HWND hWnd, ID3D12Device* device, ID3D12CommandQueue* uploadQueue, int numFramesInFlight, DXGI_FORMAT rtvFormat, D3D12_DESCRIPTOR_HEAP_DESC srvDesc)
{
    std::cout << "Initializing ImGui..." << std::endl;
    g_NumFrames = numFramesInFlight;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(hWnd))
    {
        std::cerr << "ImGui_ImplWin32_Init failed" << std::endl;
        return false;
    }

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = numFramesInFlight; // match frames-in-flight as expected by ImGui DX12 backend
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask = 0;

    if (FAILED(device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&g_pd3dSrvDescHeap))))
    {
        std::cerr << "Failed to create ImGui descriptor heap." << std::endl;
        return false;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();

    if (!ImGui_ImplDX12_Init(device, g_NumFrames, rtvFormat, g_pd3dSrvDescHeap, cpu_handle, gpu_handle))
    {
        std::cerr << "ImGui_ImplDX12_Init failed" << std::endl;
        return false;
    }

    // Ensure DX12 backend creates its device objects (including font atlas).
    if (!ImGui_ImplDX12_CreateDeviceObjects())
    {
        std::cerr << "ImGui_ImplDX12_CreateDeviceObjects failed" << std::endl;
        return false;
    }

    // ImGui_ImplDX12_Init creates device objects including font texture when needed.
    // Remove manual font atlas upload to avoid duplicate texture creation and assertions.

    return true;
}

void ImGui_Shutdown()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    if (g_pd3dSrvDescHeap) { g_pd3dSrvDescHeap->Release(); g_pd3dSrvDescHeap = nullptr; }
    if (g_pFontTexture) { g_pFontTexture->Release(); g_pFontTexture = nullptr; }
}

void ImGui_NewFrame()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGui_Render(ID3D12GraphicsCommandList* cmdList)
{
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (draw_data)
    {
        if (g_pd3dSrvDescHeap) {
            ID3D12DescriptorHeap* ppHeaps[] = { g_pd3dSrvDescHeap };
            cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
        }
        ImGui_ImplDX12_RenderDrawData(draw_data, cmdList);
    }
}

void ImGui_DrawMaterialEditor(std::vector<PBRMaterialCB>& materials, std::vector<PBRMappedPtr>& mapped)
{
    if (materials.empty()) return;
    ImGui::Begin("PBR Materials");

    static int current = 0;
    ImGui::Text("Materials: %d", (int)materials.size());
    ImGui::Separator();

    for (int i = 0; i < (int)materials.size(); ++i) {
        char buf[64];
        sprintf_s(buf, "Material %d", i);
        if (ImGui::Selectable(buf, current == i)) current = i;
    }

    // 全てのマテリアルのMetalic以外のパラメータを編集

    if (current >= 0 && current < (int)materials.size()) {
        auto& m = materials[current];
        // ベースカラー (XMFLOAT3)
        float baseColor[3] = { m.BaseColor.x, m.BaseColor.y, m.BaseColor.z };
        if (ImGui::ColorEdit3("BaseColor", baseColor)) {
            m.BaseColor.x = baseColor[0];
            m.BaseColor.y = baseColor[1];
            m.BaseColor.z = baseColor[2];
        }
        ImGui::SliderFloat("Metallic", &m.Metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &m.Roughness, 0.0f, 1.0f);
        ImGui::SliderFloat("Subsurface", &m.Subsurface, 0.0f, 1.0f);
        ImGui::SliderFloat("Specular", &m.Specular, 0.0f, 1.0f);
        ImGui::SliderFloat("SpecularTint", &m.SpecularTint, 0.0f, 1.0f);
        ImGui::SliderFloat("Anisotropic", &m.Anisotropic, 0.0f, 1.0f);
        ImGui::SliderFloat("Sheen", &m.Sheen, 0.0f, 1.0f);
        ImGui::SliderFloat("SheenTint", &m.SheenTint, 0.0f, 1.0f);
        ImGui::SliderFloat("Clearcoat", &m.Clearcoat, 0.0f, 1.0f);
        ImGui::SliderFloat("ClearcoatGloss", &m.ClearcoatGloss, 0.0f, 1.0f);
        ImGui::SliderFloat("AmbientFactor", &m.AmbientFactor, 0.0f, 1.0f);

        // 変更をマップ済み領域へ即反映
        if (mapped.size() > (size_t)current && mapped[current] != nullptr) {
            memcpy(mapped[current], &m, sizeof(PBRMaterialCB));
        }
    }

    std::cerr << "ImGui Draw Material Editor" << std::endl;
    ImGui::End();
}

ID3D12DescriptorHeap* ImGui_GetDescriptorHeap()
{
    return g_pd3dSrvDescHeap;
}