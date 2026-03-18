#include "Model.h"
#include "GeometryGenerator.h"
#include "Common.h"
#include "Shader.h"
#include <WICTextureLoader.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <system_error>
#include <vector>
#include <map>

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
bool IsSameFuzzy(std::wstring s1, std::wstring s2) {
    if (s1.empty() || s2.empty()) return false;
    std::transform(s1.begin(), s1.end(), s1.begin(), ::towlower);
    std::transform(s2.begin(), s2.end(), s2.begin(), ::towlower);
    return (s1 == s2 || s1.find(s2) != std::wstring::npos || s2.find(s1) != std::wstring::npos);
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
    


    for (auto& vertex : v)
    {
        vertex.r = color.x;
        vertex.g = color.y;
        vertex.b = color.z;
        vertex.a = color.w;
        float len = sqrt(vertex.x * vertex.x + vertex.y * vertex.y + vertex.z * vertex.z);
        for (int k = 0; k < 4; k++) {
            vertex.BoneIndices[k] = 0;
            vertex.BoneWeights[k] = 0.0f;
        }
        if (len > 0) {
            vertex.nx = vertex.x / len; vertex.ny = vertex.y / len; vertex.nz = vertex.z / len;
        }
    }

    m_mesh.Create(device, v.data(), (int)v.size(), i.data(), (int)i.size());

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
    
    for (auto& vertex : v) 
    {
        vertex.r = color.x;
        vertex.g = color.y;
        vertex.b = color.z;
        vertex.a = color.w;
        for (int k = 0; k < 4; k++) {
            vertex.BoneIndices[k] = 0;
            vertex.BoneWeights[k] = 0.0f;
        }
        vertex.nx = 0.0f; vertex.ny = 1.0f; vertex.nz = 0.0f;
    }

    m_mesh.Create(device, v.data(), (int)v.size(), i.data(), (int)i.size());

    m_subsets.clear();
    Subset s;
    s.startIndex = 0;
    s.indexCount = (unsigned int)i.size();
    s.textureView = nullptr;
    m_subsets.push_back(s);
}

void Model::Draw(ID3D11DeviceContext* context, Shader* shader) {
    m_mesh.BindBuffers(context);
    if (m_toonTexture) {
        context->PSSetShaderResources(1, 1, m_toonTexture.GetAddressOf());
    }
    for (const auto& subset : m_subsets) {

        bool hasTexture = (subset.textureView != nullptr);
        shader->SetMaterial(context, hasTexture);

        if (hasTexture) {
            context->PSSetShaderResources(0, 1, subset.textureView.GetAddressOf());
        }
        else {
            // 修正: テクスチャがない場合は nullptr をセットして前の設定をクリアする
            ID3D11ShaderResourceView* nullSRV = nullptr;
            context->PSSetShaderResources(0, 1, &nullSRV);
        }

        context->DrawIndexed(subset.indexCount, subset.startIndex, 0);
    }
    // 描き終わったらリセット
    ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
    context->PSSetShaderResources(0, 2, nullSRVs);
}

bool Model::LoadFromFile(ID3D11Device* device, const std::string& filename) {
    m_bones.clear();
    m_subsets.clear();
    Assimp::Importer importer;

    // 法線がないモデルのために自動生成フラグを追加
    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_ConvertToLeftHanded | aiProcess_GenSmoothNormals);

    if (!scene || !scene->mRootNode) return false;

    std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textures;
    fs::path modelDir = fs::path(filename).parent_path();

    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* material = scene->mMaterials[i];
        aiString texPath;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texView = nullptr;

        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
            std::string rawPath = texPath.C_Str();
            std::wstring targetFileName = fs::path(UTF8ToWide(rawPath)).filename().wstring();
            std::wstring targetFileNameSJIS = fs::path(SJISToWide(rawPath)).filename().wstring();

            fs::path directPath = modelDir / UTF8ToWide(rawPath);
            if (!fs::exists(directPath)) directPath = modelDir / SJISToWide(rawPath);

            if (fs::exists(directPath)) {
                DirectX::CreateWICTextureFromFile(device, directPath.c_str(), nullptr, &texView);
            }

            // 見つからない場合のみ再帰検索を行う
            if (!texView) {
                std::error_code ec;
                for (const auto& entry : fs::recursive_directory_iterator(modelDir, ec)) {
                    if (!entry.is_regular_file()) continue;
                    std::wstring diskName = entry.path().filename().wstring();
                    if (_wcsicmp(diskName.c_str(), targetFileName.c_str()) == 0 ||
                        _wcsicmp(diskName.c_str(), targetFileNameSJIS.c_str()) == 0) {
                        DirectX::CreateWICTextureFromFile(device, entry.path().c_str(), nullptr, &texView);
                        if (texView) break;
                    }
                }
            }
        }
        textures.push_back(texView);
    }

    std::vector<Vertex> allVertices;
    std::vector<unsigned int> allIndices;
    unsigned int vertexOffset = 0;

    std::map<std::string, unsigned int> boneMapping;

    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];

        // テクスチャがある場合は頂点色を白にして濁りを防ぐ
        aiColor4D diff(1, 1, 1, 1);
        if (mesh->mMaterialIndex < textures.size() && !textures[mesh->mMaterialIndex]) {
            scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, diff);
        }

        Subset subset = { (unsigned int)mesh->mNumFaces * 3, (unsigned int)allIndices.size(), nullptr };
        if (mesh->mMaterialIndex < (unsigned int)textures.size()) subset.textureView = textures[mesh->mMaterialIndex];

        // 頂点ループは必ず「1回」にする！
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex v = {};
            v.x = mesh->mVertices[i].x; v.y = mesh->mVertices[i].y; v.z = mesh->mVertices[i].z;
            v.nx = mesh->mNormals[i].x; v.ny = mesh->mNormals[i].y; v.nz = mesh->mNormals[i].z;
            if (mesh->HasTextureCoords(0)) { v.u = mesh->mTextureCoords[0][i].x; v.v = mesh->mTextureCoords[0][i].y; }

            // テクスチャがある場合は頂点色を白にし、ない場合はマテリアルの色（地面の色など）を入れる
            if (subset.textureView) { v.r = v.g = v.b = v.a = 1.0f; }
            else { v.r = diff.r; v.g = diff.g; v.b = diff.b; v.a = diff.a; }

            allVertices.push_back(v);
        }

        // インデックスの追加
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            for (unsigned int j = 0; j < 3; j++) {
                allIndices.push_back(mesh->mFaces[i].mIndices[j] + vertexOffset);
            }
        }

        for (unsigned int i = 0; i < mesh->mNumBones; i++)
        {
            aiBone* bone = mesh->mBones[i];
            std::string boneName = bone->mName.C_Str();

            unsigned int actualBoneID = 0;

            // すでに登録されているボーンかチェック
            if (boneMapping.find(boneName) == boneMapping.end())
            {
                // 新しいボーンなら、新しいIDを割り当てて行列を保存
                actualBoneID = (unsigned int)m_bones.size();
                boneMapping[boneName] = actualBoneID;

                BoneInfo info;
                info.name = boneName;

                // Assimpの行列をDirectx::XMMATRIXに変換して保存
                aiMatrix4x4 m_assimp = bone->mOffsetMatrix;
                info.offset = DirectX::XMMatrixTranspose(DirectX::XMMatrixSet(
                    m_assimp.a1, m_assimp.a2, m_assimp.a3, m_assimp.a4,
                    m_assimp.b1, m_assimp.b2, m_assimp.b3, m_assimp.b4,
                    m_assimp.c1, m_assimp.c2, m_assimp.c3, m_assimp.c4,
                    m_assimp.d1, m_assimp.d2, m_assimp.d3, m_assimp.d4
                ));
                m_bones.push_back(info);
            }
            else
            {
                // すでに登録済みなら、その時のIDを使う
                actualBoneID = boneMapping[boneName];
            }

            // ウェイトの設定
            for (unsigned int j = 0; j < bone->mNumWeights; j++)
            {
                unsigned int vertexID = vertexOffset + bone->mWeights[j].mVertexId;
                float weight = bone->mWeights[j].mWeight;

                for (int k = 0; k < 4; k++)
                {
                    if (allVertices[vertexID].BoneWeights[k] == 0.0f)
                    {
                        // 重複を排除した「actualBoneID」をセットする
                        allVertices[vertexID].BoneIndices[k] = actualBoneID;
                        allVertices[vertexID].BoneWeights[k] = weight;
                        break;
                    }
                }
            }
        }

        vertexOffset = (unsigned int)allVertices.size();
        m_subsets.push_back(subset);
    }

    for (auto& bone : m_bones)
    {
        bone.parentIndex = -1;
        // Assimp のシーン全体からそのボーン名のノードを探す
        aiNode* node = scene->mRootNode->FindNode(bone.name.c_str());
        if (node) {
            aiNode* parentNode = node->mParent;
            // 直属の親だけでなく、ボーンとして登録されている親が見つかるまで遡り続ける
            while (parentNode) {
                std::string parentName = parentNode->mName.C_Str();
                if (boneMapping.count(parentName)) {
                    bone.parentIndex = boneMapping[parentName];
                    break; // 見つかったらループ終了
                }
                parentNode = parentNode->mParent; // 見つからなければさらに上の階層へ
            }
        }
    }
    for (auto& v : allVertices) {
        float sum = v.BoneWeights[0] + v.BoneWeights[1] + v.BoneWeights[2] + v.BoneWeights[3];
        if (sum > 0.0f) {
            v.BoneWeights[0] /= sum;
            v.BoneWeights[1] /= sum;
            v.BoneWeights[2] /= sum;
            v.BoneWeights[3] /= sum;
        }
    }

    m_mesh.Create(device, allVertices.data(), (int)allVertices.size(), allIndices.data(), (int)allIndices.size());
    return true;
}