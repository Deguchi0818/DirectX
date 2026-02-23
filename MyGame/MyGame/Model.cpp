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

std::wstring UTF8ToWide(const std::string& utf8) {
    if (utf8.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    if (size <= 0) return L"";
    std::wstring wide(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wide[0], size);
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

void Model::Draw(ID3D11DeviceContext* context) {
    m_mesh.BindBuffers(context);
    if (m_toonTexture) {
        context->PSSetShaderResources(1, 1, m_toonTexture.GetAddressOf());
    }
    for (const auto& subset : m_subsets) {

        ID3D11ShaderResourceView* srv = subset.textureView.Get();
        context->PSSetShaderResources(0, 1, &srv);

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
            std::string raw = texPath.C_Str();
            std::wstring wideUTF8 = UTF8ToWide(raw);
            std::wstring wideSJIS = SJISToWide(raw);

            // 検索候補の名前を抽出 (例: "Texture\眼球.bmp" -> "眼球")
            std::wstring stemUTF8 = fs::path(wideUTF8).stem().wstring();
            std::wstring stemSJIS = fs::path(wideSJIS).stem().wstring();

            bool found = false;
            std::error_code ec;

            //
            if (fs::exists(modelDir, ec)) {
                for (const auto& entry : fs::recursive_directory_iterator(modelDir, ec)) {
                    if (!entry.is_regular_file()) continue;

                    std::wstring diskStem = entry.path().stem().wstring();
                    std::wstring diskFull = entry.path().filename().wstring();


                    if (diskFull == wideUTF8 || diskFull == wideSJIS ||
                        diskStem == stemUTF8 || diskStem == stemSJIS)
                    {
                        DirectX::CreateWICTextureFromFile(device, entry.path().c_str(), nullptr, &texView);
                        if (texView) { found = true; break; }
                    }
                }
            }

            if (!texView) {
                OutputDebugStringW((L"【失敗】テクスチャが見つかりません: " + wideSJIS + L"\n").c_str());
            }
        }
        textures.push_back(texView);
    }

    // --- メッシュ読み込み処理（提示されたループ構造のまま、オフセット処理を維持） ---
    std::vector<Vertex> allVertices;
    std::vector<unsigned int> allIndices;
    unsigned int vertexOffset = 0;

    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        aiColor4D diffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);

        Subset subset = { (unsigned int)allIndices.size(), mesh->mNumFaces * 3, nullptr };
        if (mesh->mMaterialIndex < (unsigned int)textures.size()) {
            subset.textureView = textures[mesh->mMaterialIndex];
        }

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex v = {};
            v.x = mesh->mVertices[i].x; v.y = mesh->mVertices[i].y; v.z = mesh->mVertices[i].z;
            v.r = diffuseColor.r; v.g = diffuseColor.g; v.b = diffuseColor.b; v.a = diffuseColor.a;

            if (mesh->HasNormals()) {
                v.nx = mesh->mNormals[i].x; v.ny = mesh->mNormals[i].y; v.nz = mesh->mNormals[i].z;
            }
            if (mesh->HasTextureCoords(0)) {
                v.u = mesh->mTextureCoords[0][i].x; v.v = mesh->mTextureCoords[0][i].y;
            }
            allVertices.push_back(v);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            for (unsigned int j = 0; j < 3; j++) {
                allIndices.push_back(mesh->mFaces[i].mIndices[j] + vertexOffset);
            }
        }
        vertexOffset = (unsigned int)allVertices.size();
        m_subsets.push_back(subset);
    }

    if (allVertices.empty() || allIndices.empty()) return false;
    m_mesh.Create(device, allVertices.data(), (int)allVertices.size(), allIndices.data(), (int)allIndices.size());

    return true;
}