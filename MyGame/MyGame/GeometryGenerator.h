#pragma once
#include "Common.h"
#include <vector>
#include <DirectXMath.h>

class GeometryGenerator {
public:
    // —§•û‘Ì‚ğ¶¬‚·‚é
    static void CreateCube(float size, const MyVector4& color, std::vector<Vertex>& outVertices, std::vector<unsigned short>& outIndices);

    // •½–Êi’n–Êj‚ğ¶¬‚·‚é
    static void CreatePlane(float width, float depth, const MyVector4&, std::vector<Vertex>& outVertices, std::vector<unsigned short>& outIndices);
};