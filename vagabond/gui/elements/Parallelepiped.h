//
// Created by romain on 12/08/2026.
//


#ifndef __vagabond__Parallelepiped__
#define __vagabond__Parallelepiped__

#include "SimplePolygon.h"

class Parallelepiped : public SimplePolygon
{
public:
    Parallelepiped(bool proj = true);

    void addParallelepiped(glm::vec3 min, glm::vec3 max);
private:
};

#endif