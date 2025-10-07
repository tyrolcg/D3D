#pragma once

#ifdef _DEBUG
#pragma comment(lib, "assimp-vc143-mtd.lib")
#else
#pragma comment(lib, "assimp-vc143-mt.lib")
#endif


#include<d3d12.h>
#include<DirectXMath.h>
#include<string>
#include<vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct MeshVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexCoord;
	DirectX::XMFLOAT3 Tangent;

	MeshVertex() = default;

	MeshVertex(
		const DirectX::XMFLOAT3& position,
		const DirectX::XMFLOAT3& normal,
		const DirectX::XMFLOAT2& texCoord,
		const DirectX::XMFLOAT3& tangent
	) noexcept
		: Position(position)
		, Normal(normal)
		, TexCoord(texCoord)
		, Tangent(tangent)
	{
		// do nothing
	}

	static const int InputElementCount = 4;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
	static const D3D12_INPUT_LAYOUT_DESC InputLayout;
};

struct Material
{
	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular; // w = Power
	DirectX::XMFLOAT4 Reflect;
	float alpha;
	std::string DiffuseMap; // Texture file path
	Material() = default;
	Material(
		const DirectX::XMFLOAT4& ambient,
		const DirectX::XMFLOAT4& diffuse,
		const DirectX::XMFLOAT4& specular,
		const DirectX::XMFLOAT4& reflect
	) noexcept
		: Ambient(ambient)
		, Diffuse(diffuse)
		, Specular(specular)
		, Reflect(reflect)
	{
		// do nothing
	}
};

struct Mesh
{
	std::vector<MeshVertex> Vertices;
	std::vector<uint32_t> Indices;
	int MaterialIndex = -1;
};

bool LoadMesh(
	const std::string& filename, 
	std::vector<Mesh>& meshes,
	std::vector<Material>& materials
);

bool LoadMesh(
	const wchar_t* filename,
	std::vector<Mesh>& meshes,
	std::vector<Material>& materials
);