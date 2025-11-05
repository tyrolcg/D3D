#pragma once
#include <d3d12.h>
#include <windows.h>
#include <vector>
#include "App.h"

struct PBRMaterialCB;
using PBRMappedPtr = unsigned char*;

// Added uploadQueue parameter to allow using the application's command queue for texture uploads
bool ImGui_Init(HWND hWnd, ID3D12Device* device, ID3D12CommandQueue* uploadQueue, int numFramesInFlight, DXGI_FORMAT rtvFormat);
void ImGui_Shutdown();
void ImGui_NewFrame();
void ImGui_Render(ID3D12GraphicsCommandList* cmdList);
void ImGui_DrawMaterialEditor(std::vector<PBRMaterialCB>& materials, std::vector<PBRMappedPtr>& mapped);

// Return ImGui's internal shader-visible CBV/SRV/UAV descriptor heap (may be nullptr if not initialized)
ID3D12DescriptorHeap* ImGui_GetDescriptorHeap();