//
// Created by romain on 08/07/2026.
//

#ifndef __vagabond__RotamerCollisionBox__
#define __vagabond__RotamerCollisionBox__
#include <glm/glm.hpp>
#include <string>
#include <vector>


class CollisionBox
{
    std::vector<glm::vec3> vertices {};
    std::string name {};
    bool collision {false};
    float xMin {}; //Goal might be to remove these variable (but can be useful for Collisions checks?)
    float xMax {};

    float yMin {};
    float yMax {};

    float zMin {};
    float zMax {};

public:

    CollisionBox()
    {

    }
    CollisionBox(std::vector<glm::vec3> const &positions)
    {
        updateVert(positions);
    }
    void updateVert(std::vector<glm::vec3> const &positions)
    {
        vertices.clear();
        float xMint {};
        float xMaxt {};

        float yMint {};
        float yMaxt {};

        float zMint {};
        float zMaxt {};
        for (auto const pos: positions)
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
        xMax = xMaxt;
        xMin = xMint;
        yMax = yMaxt;
        yMin = yMint;
        zMax = zMaxt;
        zMin = zMint;
    }

    std::pair<glm::vec3,glm::vec3> collisionCube() const
    {
        glm::vec3 min {xMin,yMin,zMin};
        glm::vec3 max {xMax,yMax,zMax};
        return std::pair<glm::vec3,glm::vec3> {min,max};
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
    bool colliding() const
    {return collision;}
};
#endif
