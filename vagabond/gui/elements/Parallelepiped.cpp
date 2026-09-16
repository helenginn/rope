//
// Created by romain on 12/08/2026.
//

#include "Parallelepiped.h"


Parallelepiped::Parallelepiped(bool proj, bool plain) : SimplePolygon()
{
    setName("Parallelepiped");
    if (plain)
        _renderType = GL_TRIANGLES;
    else
        _renderType = GL_LINES;

    setUsesProjection(true);
    setVertexShaderFile("assets/shaders/with_matrix.vsh");
    setFragmentShaderFile("assets/shaders/color_only.fsh");
}

void Parallelepiped::addParallelepiped(glm::vec3 min, glm::vec3 max)
{
    glm::vec3 p1{min};
    glm::vec3 p2{max.x, min.y, min.z};
    glm::vec3 p3{max.x, max.y, min.z};
    glm::vec3 p4{min.x, max.y, min.z};
    glm::vec3 p5{min.x, min.y, max.z};
    glm::vec3 p6{max.x, min.y, max.z};
    glm::vec3 p7{max};
    glm::vec3 p8{min.x, max.y, max.z};

    addVertex(p1);
    addVertex(p2);
    addVertex(p3);
    addVertex(p4);
    addVertex(p5);
    addVertex(p6);
    addVertex(p7);
    addVertex(p8);

    int vertexNum = vertexCount() - 9;
    addIndices(vertexNum + 1, vertexNum + 2);
    addIndices(vertexNum + 2, vertexNum + 3);
    addIndices(vertexNum + 3, vertexNum + 4);
    addIndices(vertexNum + 4, vertexNum + 1);
    addIndices(vertexNum + 1, vertexNum + 5);
    addIndices(vertexNum + 2, vertexNum + 6);
    addIndices(vertexNum + 3, vertexNum + 7);
    addIndices(vertexNum + 4, vertexNum + 8);
    addIndices(vertexNum + 8, vertexNum + 5);
    addIndices(vertexNum + 5, vertexNum + 6);
    addIndices(vertexNum + 6, vertexNum + 7);
    addIndices(vertexNum + 7, vertexNum + 8);
}
void Parallelepiped::addTrueParallelepiped(glm::vec3 startPos, glm::vec3 height, float diameter, float offset)
{
    glm::vec3 helper = (glm::abs(height.x) < 0.9f)
                       ? glm::vec3(1, 0, 0)
                       : glm::vec3(0, 1, 0);

    glm::vec3 ortho1 = glm::cross(glm::vec3(height), helper);
    ortho1 = glm::normalize(ortho1) * glm::vec3(diameter);

    glm::vec3 ortho2 = glm::cross(glm::vec3(height), ortho1);
    ortho2 = glm::normalize(ortho2) * glm::vec3(diameter);

    glm::vec3 min = startPos - ortho1*glm::vec3(0.5) - ortho2 * glm::vec3(0.5);
    glm::vec3 p1{startPos+(height-ortho1*glm::vec3(0.5) - ortho2 * glm::vec3(0.5))*glm::vec3(offset)};
    glm::vec3 p2{startPos+(height+ortho1*glm::vec3(0.5) - ortho2 * glm::vec3(0.5))*glm::vec3(offset)};
    glm::vec3 p3{min + ortho1 + height};
    glm::vec3 p4{min + height};
    glm::vec3 p5{startPos+(height-ortho1*glm::vec3(0.5) + ortho2 * glm::vec3(0.5))*glm::vec3(offset)};
    glm::vec3 p6{startPos+(height+ortho1*glm::vec3(0.5) + ortho2 * glm::vec3(0.5))*glm::vec3(offset)};
    glm::vec3 p7{min + ortho1 + height + ortho2};
    glm::vec3 p8{min + height + ortho2};

    // pyramids
    // glm::vec3 p1{startPos};
    // glm::vec3 p2{startPos};
    // glm::vec3 p3{min + ortho1 + height};
    // glm::vec3 p4{min + height};
    // glm::vec3 p5{startPos};
    // glm::vec3 p6{startPos};
    // glm::vec3 p7{min + ortho1 + height + ortho2};
    // glm::vec3 p8{min + height + ortho2};

    // classical boxes
    // glm::vec3 p1{min};
    // glm::vec3 p2{min + ortho1};
    // glm::vec3 p3{min + ortho1 + height};
    // glm::vec3 p4{min + height};
    // glm::vec3 p5{min + ortho2};
    // glm::vec3 p6{min + ortho1 + ortho2};
    // glm::vec3 p7{min + ortho1 + height + ortho2};
    // glm::vec3 p8{min + height + ortho2};

    addVertex(p1);
    addVertex(p2);
    addVertex(p3);
    addVertex(p4);
    addVertex(p5);
    addVertex(p6);
    addVertex(p7);
    addVertex(p8);

    int vertexNum = vertexCount() - 9;
    addIndices(vertexNum + 1,vertexNum + 2,vertexNum + 4);
    addIndices(vertexNum + 3,vertexNum + 4,vertexNum + 2);
    addIndices(vertexNum + 1,vertexNum + 2,vertexNum + 5);
    addIndices(vertexNum + 6,vertexNum + 5,vertexNum + 2);
    addIndices(vertexNum + 5,vertexNum + 6,vertexNum + 8);
    addIndices(vertexNum + 7,vertexNum + 8,vertexNum + 6);
    addIndices(vertexNum + 7,vertexNum + 8,vertexNum + 3);
    addIndices(vertexNum + 4,vertexNum + 8,vertexNum + 3);
    addIndices(vertexNum + 6,vertexNum + 2,vertexNum + 7);
    addIndices(vertexNum + 3,vertexNum + 7,vertexNum + 2);
    addIndices(vertexNum + 5,vertexNum + 8,vertexNum + 1);
    addIndices(vertexNum + 4,vertexNum + 8,vertexNum + 1);
}
