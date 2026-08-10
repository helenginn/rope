//
// Created by romain on 08/07/2026.
//

#ifndef __vagabond__RotamerCollisionBox__
#define __vagabond__RotamerCollisionBox__
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <iostream>


class CollisionBox
{
    std::vector<glm::vec3> vertices {};
    std::string name {};
    bool collision {false};
    float xMin {};
    float xMax {};

    float yMin {};
    float yMax {};

    float zMin {};
    float zMax {};

public:

    CollisionBox()
    {

    }
    CollisionBox(std::vector<glm::vec3> positions)
    {
        updateVert(positions);
    }
    void updateVert(std::vector<glm::vec3> positions)
    {
        vertices.clear();
        float xMint {};
        float xMaxt {};

        float yMint {};
        float yMaxt {};

        float zMint {};
        float zMaxt {};
        for (auto pos: positions)
        {
            if (!xMint)
            {
                xMint = pos.x;
                xMaxt = pos.x;
            } else if (pos.x < xMint)
            {
                xMint = pos.x;
            } else if (pos.x > xMaxt)
            {
                xMaxt = pos.x;
            }
            if (!yMint)
            {
                yMint = pos.y;
                yMaxt = pos.y;
            } else if (pos.y < yMint)
            {
                yMint = pos.y;
            } else if (pos.y > yMaxt)
            {
                yMaxt = pos.y;
            }
            if (!zMint)
            {
                zMint = pos.z;
                zMaxt = pos.z;
            }

            else if (pos.z < zMint)
            {
                zMint = pos.z;
            } else if (pos.z > zMaxt)
            {
                zMaxt = pos.z;
            }
        }
        vertices.emplace_back(xMint, yMint, zMint);
        vertices.emplace_back(xMint, yMint, zMaxt);
        vertices.emplace_back(xMint, yMaxt, zMaxt);
        vertices.emplace_back(xMint, yMaxt, zMint);
        vertices.emplace_back(xMint, yMint, zMint);
        vertices.emplace_back(xMaxt, yMint, zMint);
        vertices.emplace_back(xMaxt, yMint, zMaxt);
        vertices.emplace_back(xMint, yMint, zMaxt);
        vertices.emplace_back(xMaxt, yMint, zMaxt);
        vertices.emplace_back(xMaxt, yMaxt, zMaxt);
        vertices.emplace_back(xMint, yMaxt, zMaxt);
        vertices.emplace_back(xMint, yMaxt, zMint);
        vertices.emplace_back(xMaxt, yMaxt, zMint);
        vertices.emplace_back(xMaxt, yMaxt, zMaxt);
        vertices.emplace_back(xMaxt, yMaxt, zMint);
        vertices.emplace_back(xMaxt, yMint, zMint);
        xMax = xMaxt;
        xMin = xMint;
        yMax = yMaxt;
        yMin = yMint;
        zMax = zMaxt;
        zMin = zMint;
    }

    std::vector<glm::vec3> collisionCube()
    {
        return vertices;
    }
    bool collidesWith(CollisionBox &other)
    {
        if (
        xMin <= other.xMax &&
        xMax >= other.xMin &&
        yMin <= other.yMax &&
        yMax >= other.yMin &&
        zMin <= other.zMax &&
        zMax >= other.zMin
        )
        {
            this->collision = true;
            other.collision = true;
            return true;
        }
        return false;
    }
    void clearCollision()
    {
        this->collision = false;
    }
bool colliding()
    {return collision;}
};
#endif
