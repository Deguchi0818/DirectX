#include "Model.h"
#include "GeometryGenerator.h"
#include "Common.h"
#include "Shader.h"
#include <WICTextureLoader.h>


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
            // テクスチャがない場合は nullptr をセットして前の設定をクリアする
            ID3D11ShaderResourceView* nullSRV = nullptr;
            context->PSSetShaderResources(0, 1, &nullSRV);
        }

        context->DrawIndexed(subset.indexCount, subset.startIndex, 0);
    }
    // 描き終わったらリセット
    ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
    context->PSSetShaderResources(0, 2, nullSRVs);
}

bool Model::LoadFromFile(ID3D11Device* device, const std::string& filename, const std::string& defaultAnimName)
{
    m_bones.clear();
    m_subsets.clear();
    Assimp::Importer importer;

    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

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

    if (scene->HasAnimations() && !defaultAnimName.empty())
    {
        ExtractAnimations(scene, defaultAnimName);
    }

    m_mesh.Create(device, allVertices.data(), (int)allVertices.size(), allIndices.data(), (int)allIndices.size());
    return true;
}

void Model::ExtractAnimations(const aiScene* scene, const std::string& animName) {
    //m_animations.clear();

    // FBXに入っているすべてのアニメーション（走る、ジャンプなど）をループ
    for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
        aiAnimation* aiAnim = scene->mAnimations[i];
        AnimationClip clip;
        clip.name = animName;
        clip.duration = (float)aiAnim->mDuration;
        clip.ticksPerSecond = (float)(aiAnim->mTicksPerSecond != 0 ? aiAnim->mTicksPerSecond : 25.0f);

        // そのアニメーションで動くボーン（チャンネル）をループ
        for (unsigned int j = 0; j < aiAnim->mNumChannels; j++)
        {
            aiNodeAnim* channel = aiAnim->mChannels[j];
            BoneAnimation boneAnim;
            boneAnim.boneName = channel->mNodeName.C_Str();

            // 位置のキーフレームを保存
            for (unsigned int p = 0; p < channel->mNumPositionKeys; p++) {
                auto& key = channel->mPositionKeys[p];
                boneAnim.positionKeys.push_back({ (float)key.mTime, {key.mValue.x, key.mValue.y, key.mValue.z} });
            }

            // 回転（クォータニオン）のキーフレームを保存
            for (unsigned int r = 0; r < channel->mNumRotationKeys; r++) {
                auto& key = channel->mRotationKeys[r];
                // Assimpのクォータニオンは (w, x, y, z) の順なので注意！
                DirectX::XMVECTOR quat = DirectX::XMVectorSet(key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w);
                boneAnim.rotationKeys.push_back({ (float)key.mTime, quat });
            }

            // スケールのキーフレームを保存
            for (unsigned int s = 0; s < channel->mNumScalingKeys; s++) {
                auto& key = channel->mScalingKeys[s];
                boneAnim.scaleKeys.push_back({ (float)key.mTime, {key.mValue.x, key.mValue.y, key.mValue.z} });
            }

            // 辞書に登録
            clip.channels[boneAnim.boneName] = boneAnim;
        }

        m_animations[animName] = clip;
    }
}

// キーフレームを探すための便利なヘルパー関数
template <typename T>
int FindKeyIndex(float animationTime, const std::vector<AnimKeyFrame<T>>& keys) {
    // アニメーション時間が最後のキーフレームを超えていたら、最後の区間を返す
    if (animationTime >= keys.back().time) {
        return (int)keys.size() - 2;
    }
    for (size_t i = 0; i < keys.size() - 1; i++) {
        if (animationTime < keys[i + 1].time) {
            return (int)i;
        }
    }
    return 0;
}

void Model::UpdateAnimation(const std::string& animName, float timeInSeconds, std::vector<DirectX::XMMATRIX>& outLocalMatrices, std::vector<bool>& outHasAnim, bool isLoop)
{
    outLocalMatrices.resize(m_bones.size(), DirectX::XMMatrixIdentity());               
    outHasAnim.resize(m_bones.size(), false);

    if (m_animations.empty()) return;

    AnimationClip& clip = m_animations[animName];

    //AnimationClip& clip = m_animations[0];

    float timeInTicks = timeInSeconds * clip.ticksPerSecond;
    float animationTime = fmod(timeInTicks, clip.duration);

    if (isLoop)
    {
        animationTime = fmod(timeInTicks, clip.duration);
    }
    else 
    {
        animationTime = std::min(timeInTicks, clip.duration - 0.001f);
    }

    for (int i = 0; i < (int)m_bones.size(); i++)
    {
        std::string boneName = m_bones[i].name;

        std::string searchName = "";

        // 1. まずは完全一致を試す
        if (clip.channels.count(boneName) > 0)
        {
            searchName = boneName;
        }
        else
        {
            // 2. "mixamorig:" などの装飾を取り除いた「純粋な骨の名前」を作る
            std::string pureBoneName = boneName;
            size_t pos = pureBoneName.find("mixamorig:");
            if (pos != std::string::npos) pureBoneName = pureBoneName.substr(pos + 10);

            // 3. アニメーションの中から、純粋な名前が「完全一致」するものを探す！
            for (auto& pair : clip.channels)
            {
                std::string pureChannelName = pair.first;

                // チャンネル名からも "mixamorig:" を消す
                size_t pos2 = pureChannelName.find("mixamorig:");
                if (pos2 != std::string::npos) pureChannelName = pureChannelName.substr(pos2 + 10);

                // Assimp特有のゴミ文字（_$AssimpFbx$_など）を消す
                size_t pos3 = pureChannelName.find("_$");
                if (pos3 != std::string::npos) pureChannelName = pureChannelName.substr(0, pos3);

                if (pureBoneName == pureChannelName)
                {
                    searchName = pair.first;
                    break;
                }
            }
        }

        if (clip.channels.count(searchName) > 0)
        {
            outHasAnim[i] = true;
            BoneAnimation& anim = clip.channels[searchName];

            // 大きさ（Scale）
            DirectX::XMVECTOR scale = DirectX::XMVectorSet(1, 1, 1, 0);
            if (anim.scaleKeys.size() == 1) {
                scale = DirectX::XMVectorSet(anim.scaleKeys[0].value.x, anim.scaleKeys[0].value.y, anim.scaleKeys[0].value.z, 0);
            }
            else if (anim.scaleKeys.size() > 1) {
                int idx = FindKeyIndex(animationTime, anim.scaleKeys);
                int nextIdx = idx + 1;
                float dt = anim.scaleKeys[nextIdx].time - anim.scaleKeys[idx].time;
                float factor = (dt > 0.0f) ? (animationTime - anim.scaleKeys[idx].time) / dt : 0.0f;
                factor = std::clamp(factor, 0.0f, 1.0f);
                DirectX::XMVECTOR start = DirectX::XMVectorSet(anim.scaleKeys[idx].value.x, anim.scaleKeys[idx].value.y, anim.scaleKeys[idx].value.z, 0);
                DirectX::XMVECTOR end = DirectX::XMVectorSet(anim.scaleKeys[nextIdx].value.x, anim.scaleKeys[nextIdx].value.y, anim.scaleKeys[nextIdx].value.z, 0);
                scale = DirectX::XMVectorLerp(start, end, factor);
            }

            // 回転（Rotation）
            DirectX::XMVECTOR rotation = DirectX::XMQuaternionIdentity();
            if (anim.rotationKeys.size() == 1) {
                rotation = anim.rotationKeys[0].value;
            }
            else if (anim.rotationKeys.size() > 1) {
                int idx = FindKeyIndex(animationTime, anim.rotationKeys);
                int nextIdx = idx + 1;
                float dt = anim.rotationKeys[nextIdx].time - anim.rotationKeys[idx].time;
                float factor = (dt > 0.0f) ? (animationTime - anim.rotationKeys[idx].time) / dt : 0.0f;
                factor = std::clamp(factor, 0.0f, 1.0f);
                rotation = DirectX::XMQuaternionSlerp(anim.rotationKeys[idx].value, anim.rotationKeys[nextIdx].value, factor);
            }

            // 位置（Position）
            DirectX::XMVECTOR position = DirectX::XMVectorSet(0, 0, 0, 1);
            if (anim.positionKeys.size() == 1) {
                position = DirectX::XMVectorSet(anim.positionKeys[0].value.x, anim.positionKeys[0].value.y, anim.positionKeys[0].value.z, 1);
            }
            else if (anim.positionKeys.size() > 1) {
                int idx = FindKeyIndex(animationTime, anim.positionKeys);
                int nextIdx = idx + 1;
                float dt = anim.positionKeys[nextIdx].time - anim.positionKeys[idx].time;
                float factor = (dt > 0.0f) ? (animationTime - anim.positionKeys[idx].time) / dt : 0.0f;
                factor = std::clamp(factor, 0.0f, 1.0f);
                DirectX::XMVECTOR start = DirectX::XMVectorSet(anim.positionKeys[idx].value.x, anim.positionKeys[idx].value.y, anim.positionKeys[idx].value.z, 1);
                DirectX::XMVECTOR end = DirectX::XMVectorSet(anim.positionKeys[nextIdx].value.x, anim.positionKeys[nextIdx].value.y, anim.positionKeys[nextIdx].value.z, 1);
                position = DirectX::XMVectorLerp(start, end, factor);
            }

            // 合成
            DirectX::XMMATRIX scaleMat = DirectX::XMMatrixScalingFromVector(scale);
            DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationQuaternion(rotation);
            DirectX::XMMATRIX transMat = DirectX::XMMatrixTranslationFromVector(position);

            outLocalMatrices[i] = scaleMat * rotMat * transMat;
        }
    }
}

bool Model::LoadAnimation(const std::string& animName, const std::string& filename) {
    Assimp::Importer importer;

    // バケモノ化を防ぐ魔法の設定はここでも必須！
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

    // アニメーションだけ欲しいので、メッシュの計算（Triangulateなど）は不要。左手系変換だけ行う。
    const aiScene* scene = importer.ReadFile(filename, aiProcess_ConvertToLeftHanded);

    if (!scene || !scene->mRootNode) return false;

    // アニメーションが入っていれば、リストに追記する！
    if (scene->HasAnimations()) {
        ExtractAnimations(scene, animName);
    }
    return true;
}