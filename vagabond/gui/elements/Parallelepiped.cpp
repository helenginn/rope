//
// Created by romain on 12/08/2026.
//

#include "Parallelepiped.h"


Parallelepiped::Parallelepiped(bool proj) : SimplePolygon()
{
    setName("Parallelepiped");
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

    int vertexNum = vertexCount()-9;


    addIndices(vertexNum + 1,vertexNum + 2);
    addIndices(vertexNum + 2,vertexNum + 3);
    addIndices(vertexNum + 3,vertexNum + 4);
    addIndices(vertexNum + 4,vertexNum + 1);
    addIndices(vertexNum + 1,vertexNum + 5);
    addIndices(vertexNum + 2,vertexNum + 6);
    addIndices(vertexNum + 3,vertexNum + 7);
    addIndices(vertexNum + 4,vertexNum + 8);
    addIndices(vertexNum + 8,vertexNum + 5);
    addIndices(vertexNum + 5,vertexNum + 6);
    addIndices(vertexNum + 6,vertexNum + 7);
    addIndices(vertexNum + 7,vertexNum + 8);





    // addIndices(vertexNum + 1,vertexNum + 2,vertexNum + 4);
    // addIndices(vertexNum + 3,vertexNum + 4,vertexNum + 2);
    // addIndices(vertexNum + 4,vertexNum + 3,vertexNum + 8);
    // addIndices(vertexNum + 7,vertexNum + 8,vertexNum + 3);
    // addIndices(vertexNum + 6,vertexNum + 7,vertexNum + 5);
    // addIndices(vertexNum + 8,vertexNum + 5,vertexNum + 7);
    // addIndices(vertexNum + 1,vertexNum + 2,vertexNum + 4);
    // addIndices(vertexNum + 1,vertexNum + 2,vertexNum + 5);
    // addIndices(vertexNum + 6,vertexNum + 5,vertexNum + 2);

    // addIndices(1, 2, 4);
    // addIndices(1, 2, 4);
    // addIndices(1, 2, 4);
    // addIndices(1, 2, 4);
    // addIndices(1, 2, 4);
    // addIndices(1, 2, 4);
    // addIndices(1, 2, 4);



    // std::vector<glm::vec3> face1{p1, p2, p3, p4};
    // std::vector<glm::vec3> face2{p1, p2, p6, p5};
    // std::vector<glm::vec3> face3{p5, p6, p7, p8};
    // std::vector<glm::vec3> face4{p4, p3, p7, p8};
    // std::vector<glm::vec3> face5{p1, p5, p8, p4};
    // std::vector<glm::vec3> face6{p2, p6, p7, p3};
    // std::vector<std::vector<glm::vec3> > cube{face1, face2, face3, face4, face5, face6};
    // int faceNum {1};

   //  for (auto face: cube)
   //  {
   //      // std::cout << "Face number: " << faceNum << std::endl;
   //      // int pointNum {1};
   //      for (auto point: face)
   //      {
   //          addVertex(point);
   //          // std::cout << "\tpoint num: " << pointNum << '\t' << vertexCount() << std::endl;
   //          // pointNum += 1;
   //      }
   //      // faceNum += 1;
   //      int vertexNumber = vertexCount();
   //      // std::cout << "VERTEX NUM AFTER POINTS: " << vertexNumber;
   //      (vertexNumber-4 >=0 ? vertexNumber-=4 : vertexNumber = 0);
   //      // std::cout << '\t' << "AND AFTER substraction: " << vertexNumber << std::endl;
   //      // std::cout << vertexNumber << ", " << std::endl;
   //      _vertices[vertexNumber].tex[0] = 0;
   //      _vertices[vertexNumber].tex[1] = 1;
   //      _vertices[vertexNumber + 1].tex[0] = 0;
   //      _vertices[vertexNumber + 1].tex[1] = 0;
   //      _vertices[vertexNumber + 2].tex[0] = 1;
   //      _vertices[vertexNumber + 2].tex[1] = 1;
   //      _vertices[vertexNumber + 3].tex[0] = 1;
   //      _vertices[vertexNumber + 3].tex[1] = 0;
   //      addIndices(vertexNumber + 0, vertexNumber + 1, vertexNumber + 3);
   //      addIndices(vertexNumber + 2, vertexNumber + 3, vertexNumber + 1);
   // }
}
