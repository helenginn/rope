//
// Created by romain on 11/05/2026.
//

#ifndef __vagabond__RotamerModifier__
#define __vagabond__RotamerModifier__

#include "Rotamers.h"
#include "RotamerBouquet.h"
#include "StructureModification.h"
#include "AtomGroup.h"

class Parameter;
struct Bouquet
{
    std::vector<Atom*> atoms;
    std::string resName;
    ResidueId resNumber;

    std::map<Atom*, std::vector<glm::vec3>> positions;
    std::map<ResidueId, std::map<Atom*,  std::vector<glm::vec3>>> resBouquet {};

    std::map<std::string,std::vector<CollisionBox>> collisions {};
};
class RotamerModifier : public StructureModification
{
protected:
    void prepareResources();
public:
    RotamerModifier(Instance *inst);
    void setup();
    ~RotamerModifier();
    enum points{Start, End};
    enum axis{X, Y};
    enum parameter {Default, Slider, Reset, Map, MoveX, MoveY};
    float submitJobAndRetrieve(float weight, parameter a = Default);
    void submitJob(float weight);
    void prepareMemory();
    void saveStructure(std::string name);
    void unifiedTorsionFetcher();
    void generateRotamerMapPosition();
    glm::vec3 axisForChain(points p, std::string chainName);

    void makePlan();
    std::vector<glm::vec3> drawAxis();

    void move(float weight, parameter xy);

    // void primaryCollisionBoxes();
    // void secondaryCollisionBoxes(std::map<ResidueId,CollisionBox> &secondaryBoxes, std::string chain, bool individual = false);
    //
    // bool isIntersection(CollisionBox &a, CollisionBox &b, glm::vec3 translation = {0,0,0});
    // std::vector<glm::vec3> intersectionBox();
    //
    // void IntersectionList(std::map<ResidueId, CollisionBox> &collisionBoxList);
    // void residueCollisions();

    // std::vector<glm::vec3> getVertices(std::string chain)
    // {
    //     std::vector<glm::vec3> transVertices {};
    //     if (chain == "A")
    //     {
    //         for (glm::vec3 vertex : _allRotamer.primaryBoxes[chain].vertices)
    //             transVertices.emplace_back(vertex + (_xTrans+_yTrans));
    //         return transVertices;
    //     }
    //     return _allRotamer.primaryBoxes[chain].vertices;
    // }
    // std::vector<glm::vec3> getSecondaryVertices(std::string chain, bool onlyColliding = false)
    // {
    //     std::vector<glm::vec3> transVertices {};
    //      for (auto pair: _allRotamer.secondaryBoxes)
    //      {
    //          if (onlyColliding && !pair.second.collision)
    //              continue;
    //         for (glm::vec3 pos : pair.second.vertices)
    //         {
    //             if (pair.first.insert[0] != chain[0])
    //                 continue;
    //             if (pair.first.insert[0] == 'A')
    //             {
    //                 transVertices.emplace_back(pos + (_xTrans+_yTrans));
    //                 continue;
    //             }
    //             transVertices.emplace_back(pos);
    //         }
    //     }
    //         return transVertices;
    //
    // }
    // std::vector<glm::vec3> getIndividualVertices(std::string chain, bool onlyColliding = false)
    // {
    //     std::vector<glm::vec3> transVertices {};
    //     for (auto pair: _allRotamer.individualBoxes)
    //     {
    //         if (onlyColliding && !pair.second.collision)
    //             continue;
    //         for (glm::vec3 pos : pair.second.vertices)
    //         {
    //             if (pair.first.insert[0] != chain[0])
    //                 continue;
    //             if (pair.first.insert[0] == 'A')
    //             {
    //                 transVertices.emplace_back(pos + (_xTrans+_yTrans));
    //                 continue;
    //             }
    //             transVertices.emplace_back(pos);
    //         }
    //     }
    //     return transVertices;
    //
    // }


    std::vector<std::vector<glm::vec3>> getVertices()
    {
        return _bouquet->getVertices();
    }
private:
    AtomGroup *_group;
    std::vector<Rotamer> *_rotamers;
    std::vector<std::vector<float>> _allTorsions;
    float _value {};
    std::vector<Parameter *> _params;
    RotamerLibrary *_lib;
    RotamerMap *Rot;
    std::map<int, RotamerMap> _RotamerMemory;
    Bouquet _allRotamer {};
    AtomPosMap _atomPosMap {};
    AtomPosMap _atomPosMapBouquet {};

    Bouquet2 *_bouquet {};

    parameter _mode;

    glm::vec3 _axis1 {};
    glm::vec3 _x {};
    glm::vec3 _y {};
    glm::vec3 _normal {};
    glm::vec3 _xTrans {0};
    glm::vec3 _yTrans {0};
    float _memoryY {0};
    float _memoryX {0};
    bool _referential {false};
    bool _map {false};
};
#endif
