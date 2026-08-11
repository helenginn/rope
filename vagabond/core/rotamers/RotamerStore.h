//
// Created by romain on 08/07/2026.
//

#ifndef __vagabond__RotamerStore__
#define __vagabond__RotamerStore__
#include <utility>
#include <glm/vec3.hpp>

#include "vagabond/core/AtomGroup.h"
#include "vagabond/core/ResidueId.h"

struct ResRot : std::pair<ResidueId, int>
{
    ResidueId &Id()
    {
        return first;
    }
    int &Rotamer()
    {
        return second;
    }
};

struct LookUpInfo : std::pair<int, int>
{
    int &start()
    {
        return first;
    }
    int &length()
    {
        return second;
    }
};

class RotamerStore
{
public:
    RotamerStore store(std::map<ResidueId, std::map<Atom*,  std::vector<glm::vec3>>> &rotamers);
    std::vector<RotamerStore> residueStore();
    std::vector<glm::vec3> getPos();
    std::vector<glm::vec3> positionFor(ResRot const &rotamer);

    void updatePositions(glm::mat4x4 transformationMat);
    void move(glm::mat4x4 parameters,std::string chain = "");
    AtomPosMap extractForGUI(RotamerStore &storeToRender);
    std::pair<ResidueId,std::string> name();

private:
    std::map<ResRot,LookUpInfo> storage;
    std::vector<glm::vec3> positionArray;
    std::vector<Atom*> atoms;
};
#endif
