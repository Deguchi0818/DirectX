#include "Model.h"
#include "GeometryGenerator.h"
#include "Common.h"
#include <WICTextureLoader.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

std::wstring SJISToWide(const std::string& sjis) {
    if (sjis.empty()) return L"";
    // ヌル終端を含めたサイズを取得
    int size = MultiByteToWideChar(932, 0, sjis.c_str(), -1, nullptr, 0);
    if (size <= 0) return L"";
    std::wstring wide(size, 0);
    MultiByteToWideChar(932, 0, sjis.c_str(), -1, &wide[0], size);

    // std::wstringの末尾のヌル文字を適切に処理
    size_t pos = wide.find(L'\0');
    if (pos != std::wstring::npos) wide.resize(pos);
    return wide;
}

std::string ToHexString(const std::string& s) {
    std::ostringstream oss;
    for (unsigned char c : s) oss << std::hex << std::setw(2) << std::setfill('0') << (int)c << " ";
    return oss.str();
}

void Model::CreateCube(ID3D11Device* device, float size, const MyVector4& color) {
    std::vector<Vertex> v;
    std::vector<unsigned short> i_short;
    GeometryGenerator::CreateCube(size, color, v, i_short);
    std::vector<unsigned int> i(i_short.begin(), i_short.end()); // 型変換
    m_mesh.Create(device, v.data(), (int)v.size(), i.data(), (int)i.size());

    for (auto& vertex : v) {
        float len = sqrt(vertex.x * vertex.x + vertex.y * vertex.y + vertex.z * vertex.z);
        if (len > 0) {
            vertex.nx = vertex.x / len; vertex.ny = vertex.y / len; vertex.nz = vertex.z / len;
        }
    }

    m_subsets.clear();
    Subset s;
    s.startIndex = 0;
    s.indexCount = (unsigned int)i.size();
    s.textureView = nullptr; // テクスチャなし
    m_subsets.push_back(s);
}

void Model::CreatePlane(ID3D11Device* device, float width, float depth, const MyVector4& color) {
    std::vector<Vertex> v;
    std::vector<unsigned short> i_short;
    GeometryGenerator::CreatePlane(width, depth, color, v, i_short);
    std::vector<unsigned int> i(i_short.begin(), i_short.end()); // 型変換
    m_mesh.Create(device, v.data(), (int)v.size(), i.data(), (int)i.size());

    for (auto& vertex : v) {
        vertex.nx = 0.0f; vertex.ny = 1.0f; vertex.nz = 0.0f;
    }

    m_subsets.clear();
    Subset s;
    s.startIndex = 0;
    s.indexCount = (unsigned int)i.size();
    s.textureView = nullptr;
    m_subsets.push_back(s);
}

void Model::Draw(ID3D11DeviceContext* context)
{
    m_mesh.BindBuffers(context);

    if (m_toonTexture) {
        context->PSSetShaderResources(1, 1, m_toonTexture.GetAddressOf());
    }

    for (const auto& subset : m_subsets)
    {
        ID3D11ShaderResourceView* srv = subset.textureView.Get();
        context->PSSetShaderResources(0, 1, subset.textureView.GetAddressOf());
        context->DrawIndexed(subset.indexCount, subset.startIndex, 0);
    }

    // 描き終わったらリセット
    ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
    context->PSSetShaderResources(0, 2, nullSRVs);
}

bool Model::LoadFromFile(ID3D11Device* device, const std::string& filename) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_OptimizeMeshes | aiProcess_FlipUVs);

    if (!scene || !scene->mRootNode) return false;

    std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textures;
    fs::path modelDir = fs::path(filename).parent_path();

    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* material = scene->mMaterials[i];
        aiString texPath;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texView = nullptr;

        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
            std::string rawName = texPath.C_Str();

            // --- デバッグ：ここから ---
            // ログに「生データ（16進数）」を出すことで、本当の正体が見えます
            OutputDebugStringA(("RawBytes: " + ToHexString(rawName) + "\n").c_str());
            // --- デバッグ：ここまで ---

            // 1. Shift-JISとして変換
            std::wstring decodedName = SJISToWide(rawName);
            // 2. UTF-8として変換（Assimpが変換済みの場合の対策）
            std::wstring utf8Name = L"";
            int uSize = MultiByteToWideChar(CP_UTF8, 0, rawName.c_str(), -1, nullptr, 0);
            if (uSize > 0) {
                utf8Name.resize(uSize);
                MultiByteToWideChar(CP_UTF8, 0, rawName.c_str(), -1, &utf8Name[0], uSize);
                size_t p = utf8Name.find(L'\0'); if (p != std::wstring::npos) utf8Name.resize(p);
            }

            // 検索するファイル名の候補
            std::vector<std::wstring> nameCandidates = {
                fs::path(decodedName).filename().wstring(), // 本命
                fs::path(utf8Name).filename().wstring(),    // 予備1
                fs::path(SJISToWide(rawName)).wstring()     // 予備2（フルパス）
            };

            bool found = false;
            for (const auto& targetName : nameCandidates) {
                if (targetName.empty()) continue;

                std::error_code ec;
                for (const auto& entry : fs::directory_iterator(modelDir, ec)) {
                    if (!entry.is_regular_file()) continue;

                    std::wstring diskName = entry.path().stem().wstring();
                    // 名前が一致（部分一致含む）したら採用
                    if (diskName.find(targetName) != std::wstring::npos || targetName.find(diskName) != std::wstring::npos) {
                        DirectX::CreateWICTextureFromFile(device, entry.path().c_str(), nullptr, &texView);
                        if (texView) { found = true; break; }
                    }
                }
                if (found) break;
            }
        }
        textures.push_back(texView);
    }

    // --- メッシュ読み込み（変更なし） ---
    std::vector<Vertex> allVertices;
    std::vector<unsigned int> allIndices;
    unsigned int vertexOffset = 0;
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];
        Subset subset = { (unsigned int)allIndices.size(), mesh->mNumFaces * 3, nullptr };
        if (mesh->mMaterialIndex < textures.size()) subset.textureView = textures[mesh->mMaterialIndex];

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex v = {};
            v.x = mesh->mVertices[i].x; v.y = mesh->mVertices[i].y; v.z = mesh->mVertices[i].z;
            v.r = 1.0f; v.g = 1.0f; v.b = 1.0f; v.a = 1.0f;
            if (mesh->HasNormals()) {
                v.nx = mesh->mNormals[i].x; v.ny = mesh->mNormals[i].y; v.nz = mesh->mNormals[i].z;
            }
            if (mesh->HasTextureCoords(0)) {
                v.u = mesh->mTextureCoords[0][i].x;
                v.v = mesh->mTextureCoords[0][i].y;
            }
            allVertices.push_back(v);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            for (unsigned int j = 0; j < 3; j++) allIndices.push_back(mesh->mFaces[i].mIndices[j] + vertexOffset);
        }
        vertexOffset = (unsigned int)allVertices.size();
        m_subsets.push_back(subset);
    }
    m_mesh.Create(device, allVertices.data(), (int)allVertices.size(), allIndices.data(), (int)allIndices.size());
    return true;
}