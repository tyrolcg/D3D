#define NOMINMAX
#include "Mesh.h"
#include <Windows.h>
#include <dxgiformat.h>
#include <cassert>
#include <codecvt>
#include <iostream>

using namespace std;

#define FMT_FLOAT3 DXGI_FORMAT_R32G32B32_FLOAT
#define FMT_FLOAT2 DXGI_FORMAT_R32G32_FLOAT
#define APPEND D3D12_APPEND_ALIGNED_ELEMENT
#define IL_VERTEX D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA

const D3D12_INPUT_ELEMENT_DESC MeshVertex::InputElements[] = {
	{ "POSITION", 0, FMT_FLOAT3, 0, APPEND, IL_VERTEX, 0 },
	{ "NORMAL",   0, FMT_FLOAT3, 0, APPEND, IL_VERTEX, 0 },
	{ "TEXCOORD", 0, FMT_FLOAT2, 0, APPEND, IL_VERTEX, 0 },
	{ "TANGENT",  0, FMT_FLOAT3, 0, APPEND, IL_VERTEX, 0 }
};
const D3D12_INPUT_LAYOUT_DESC MeshVertex::InputLayout = {
	MeshVertex::InputElements,
	MeshVertex::InputElementCount
};

// wchar_t* -> UTF-8 string
string ToUTF8(const std::wstring& value) 
{
	auto length = WideCharToMultiByte(
		CP_UTF8, 0u, value.data(), -1, nullptr, 0, nullptr, nullptr);
	auto buffer = new char[length];

	WideCharToMultiByte(CP_UTF8, 0u, value.data(), -1, buffer, length, nullptr, nullptr);
	string result(buffer);
	delete[] buffer;
	buffer = nullptr;

	return result;
}

namespace
{

	class MeshLoader
	{
	public:
		MeshLoader();
		~MeshLoader();

		bool Load(const wchar_t* fileName, vector<Mesh>& meshes, vector<Material>& materials);

	private:
		void ParseMesh(Mesh& dstMesh, const aiMesh* pSrcMesh);
		void ParseMaterial(Material& dstMaterial, const aiMaterial* pSrcMaterial);
	};

	MeshLoader::MeshLoader()
	{
	}
	MeshLoader::~MeshLoader()
	{
	}

	bool MeshLoader::Load(const wchar_t* fileName, vector<Mesh>& meshes, vector <Material>& materials)
	{
		if (fileName == nullptr) {
			cerr << "fileName is null." << endl;
			return false;
		}

		auto path = ToUTF8(std::wstring(fileName));

		Assimp::Importer importer;
		int flag = 0;
		flag |= aiProcess_Triangulate; // 三角形化
		flag |= aiProcess_JoinIdenticalVertices; // 同じ頂点を結合
		flag |= aiProcess_CalcTangentSpace; // 接線空間の計算
		flag |= aiProcess_GenSmoothNormals; // スムーズシェーディング用の法線を生成
		flag |= aiProcess_GenUVCoords; // UV座標を生成
		flag |= aiProcess_OptimizeMeshes; // メッシュの最適化
		flag |= aiProcess_CalcTangentSpace; // 接線空間の計算

		auto pScene = importer.ReadFile(path, flag);
		if (pScene == nullptr) 
		{
			cerr << "Failed to load model: " << path << endl;
			return false;
		}

		// メッシュの読み込み
		meshes.clear();
		meshes.resize(pScene->mNumMeshes);
		for (unsigned int i = 0; i < meshes.size(); i++) 
		{
			auto pSrcMesh = pScene->mMeshes[i];
			auto& dstMesh = meshes[i];
			ParseMesh(dstMesh, pSrcMesh);
		}

		// マテリアルの読み込み
		materials.clear();
		materials.resize(pScene->mNumMaterials);
		for (unsigned int i = 0; i < materials.size(); i++) 
		{
			auto pSrcMaterial = pScene->mMaterials[i];
			auto& dstMaterial = materials[i];
			ParseMaterial(dstMaterial, pSrcMaterial);
		}
		
		pScene = nullptr;

		return true;
	}

	void MeshLoader::ParseMesh(Mesh& dstMesh, const aiMesh* pSrcMesh)
	{
		assert(pSrcMesh != nullptr);
		// 頂点データの読み込み
		dstMesh.Vertices.resize(pSrcMesh->mNumVertices);
		for (unsigned int i = 0; i < pSrcMesh->mNumVertices; i++) 
		{
			auto& v = dstMesh.Vertices[i];
			// 位置
			v.Position.x = pSrcMesh->mVertices[i].x;
			v.Position.y = pSrcMesh->mVertices[i].y;
			v.Position.z = pSrcMesh->mVertices[i].z;
			// 法線
			if (pSrcMesh->HasNormals()) 
			{
				v.Normal.x = pSrcMesh->mNormals[i].x;
				v.Normal.y = pSrcMesh->mNormals[i].y;
				v.Normal.z = pSrcMesh->mNormals[i].z;
			}
			// テクスチャ座標
			if (pSrcMesh->HasTextureCoords(0)) 
			{
				v.TexCoord.x = pSrcMesh->mTextureCoords[0][i].x;
				v.TexCoord.y = pSrcMesh->mTextureCoords[0][i].y;
			}
			// 接線
			if (pSrcMesh->HasTangentsAndBitangents()) 
			{
				v.Tangent.x = pSrcMesh->mTangents[i].x;
				v.Tangent.y = pSrcMesh->mTangents[i].y;
				v.Tangent.z = pSrcMesh->mTangents[i].z;
			}
		}
		// インデックスデータの読み込み
		dstMesh.Indices.clear();
		for (unsigned int i = 0; i < pSrcMesh->mNumFaces; i++) 
		{
			auto& face = pSrcMesh->mFaces[i];
			assert(face.mNumIndices == 3); // 三角形化しているはずなので3つのはず
			for (unsigned int j = 0; j < face.mNumIndices; j++) 
			{
				dstMesh.Indices.push_back(face.mIndices[j]);
			}
		}
		// マテリアル番号の読み込み
		dstMesh.MaterialIndex = pSrcMesh->mMaterialIndex;
	}

	void MeshLoader::ParseMaterial(Material& dstMaterial, const aiMaterial* pSrcMaterial)
	{
		assert(pSrcMaterial != nullptr);
		aiColor4D color;
		float shininess = 0.0f;
		// 環境光
		if (AI_SUCCESS == pSrcMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color))
		{
			dstMaterial.Ambient.x = color.r;
			dstMaterial.Ambient.y = color.g;
			dstMaterial.Ambient.z = color.b;
			dstMaterial.Ambient.w = color.a;
		}
		else
		{
			dstMaterial.Ambient = { 0.0f, 0.0f, 0.0f, 1.0f };
		}
		// 拡散反射光
		if (AI_SUCCESS == pSrcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color))
		{
			dstMaterial.Diffuse.x = color.r;
			dstMaterial.Diffuse.y = color.g;
			dstMaterial.Diffuse.z = color.b;
			dstMaterial.Diffuse.w = color.a;
		}
		else
		{
			dstMaterial.Diffuse = { 0.0f, 0.0f, 0.0f, 1.0f };
		}
		// 鏡面反射光
		if (AI_SUCCESS == pSrcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color))
		{
			dstMaterial.Specular.x = color.r;
			dstMaterial.Specular.y = color.g;
			dstMaterial.Specular.z = color.b;
			dstMaterial.Specular.w = color.a;
		}
		else
		{
			dstMaterial.Specular = { 0.0f, 0.0f, 0.0f, 1.0f };
		}
		// 反射光の初期化はAssimp 4.x以降不要
		//dstMaterial.Reflect = { 0.0f, 0.0f, 0.0f, 1.0f };
		// DiffuseMap
		{
			aiString path;
			if (pSrcMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS)
			{
				dstMaterial.DiffuseMap = string(path.C_Str());
			}
		}
	}

	static_assert((sizeof(MeshVertex) == (3 + 3 + 2 + 3) * sizeof(float)), "MeshVertex structure size is incorrect.");

}

bool LoadMesh(const wchar_t* fileName, vector<Mesh>& meshes, vector<Material>& materials)
{
	MeshLoader loader;
	return loader.Load(fileName, meshes, materials);
}

// 追加: std::string 版の LoadMesh 実装
bool LoadMesh(const std::string& filename, std::vector<Mesh>& meshes, std::vector<Material>& materials)
{
    // std::string から std::wstring へ変換
    std::wstring wfilename(filename.begin(), filename.end());
    return LoadMesh(wfilename.c_str(), meshes, materials);
}

// wstring版
bool LoadMesh(
	const std::wstring& filename,
	std::vector<Mesh>& meshes,
	std::vector<Material>& materials)
{
	return LoadMesh(filename.c_str(), meshes, materials);
}