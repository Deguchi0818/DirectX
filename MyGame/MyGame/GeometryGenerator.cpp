#include "GeometryGenerator.h"

void GeometryGenerator::CreateCube(float size, const MyVector4& color, std::vector<Vertex>& outVertices, std::vector<unsigned short>& outIndices) {
    float h = size * 0.5f; // 半径

    // 頂点データ (8頂点)
    outVertices = {
        { -h,  h, -h, color.x, color.y, color.z, color.w }, // 0
        {  h,  h, -h, color.x, color.y, color.z, color.w }, // 1
        {  h, -h, -h, color.x, color.y, color.z, color.w }, // 2
        { -h, -h, -h, color.x, color.y, color.z, color.w }, // 3
        { -h,  h,  h, color.x, color.y, color.z, color.w }, // 4
        {  h,  h,  h, color.x, color.y, color.z, color.w }, // 5
        {  h, -h,  h, color.x, color.y, color.z, color.w }, // 6
        { -h, -h,  h, color.x, color.y, color.z, color.w }, // 7
    };

    // インデックスデータ (36個 / 12ポリゴン)
    outIndices = {
        0, 1, 2, 0, 2, 3, // 前
        4, 6, 5, 4, 7, 6, // 後
        4, 5, 1, 4, 1, 0, // 上
        3, 2, 6, 3, 6, 7, // 下
        4, 0, 3, 4, 3, 7, // 左
        1, 5, 6, 1, 6, 2  // 右
    };
}

void GeometryGenerator::CreatePlane(float width, float depth, const MyVector4& color, std::vector<Vertex>& outVertices, std::vector<unsigned short>& outIndices) {
    float w = width * 0.5f;
    float d = depth * 0.5f;

    outVertices = {
        { -w, 0.0f,  d, color.x, color.y, color.z, color.w },
        {  w, 0.0f,  d, color.x, color.y, color.z, color.w },
        {  w, 0.0f, -d, color.x, color.y, color.z, color.w },
        { -w, 0.0f, -d, color.x, color.y, color.z, color.w },
    };

    outIndices = { 0, 1, 2, 0, 2, 3 };
}