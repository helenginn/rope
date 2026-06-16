//
// Created by romain on 11/05/2026.
//

#ifndef __vagabond__RotamerModifier__
#define __vagabond__RotamerModifier__

#include "Rotamers.h"
#include "StructureModification.h"

#include "AtomGroup.h"
#include "vagabond/gui/elements/Line.h"

class Parameter;
struct Bouquet
{
    std::string resName;
    ResidueId resNumber;
    std::map<std::string, std::vector<Atom*>> atoms;

    std::map<Atom*, std::vector<glm::vec3>> positions; // Atoms* ; positions
    std::map<ResidueId, std::map<Atom*,  std::vector<glm::vec3>>> resBouquet;
};
struct CollisionBox
{
    float xMin {};
    float xMax {};

    float yMin {};
    float yMax {};

    float zMin {};
    float zMax {};

    std::string name {};
};
class RotamerModifier : public StructureModification
{
public:
    RotamerModifier(Instance *inst);
    ~RotamerModifier();
    void prepareResources();
    enum points{Start, End};
    enum axis{X, Y};
    enum parameter {Default, Slider, Reset, Map, MoveX, MoveY};
    float submitJobAndRetrieve(float weight, parameter a = Default);
    void submitJob(float weight);
    void prepareMemory();
    void saveStructure(std::string name);
    void unifiedTorsionFetcher();
    void generateRotamerMapPosition();
    void bouquetInitializer();
    glm::vec3 axisForChain(points p, std::string chainName);
    Line lineAxis(std::string chain);

    std::vector<glm::vec3> makePlan();
    void move(float weight, parameter xy);

    void primaryCollisionBoxes();
    bool isIntersection(CollisionBox a, CollisionBox b);
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
    AtomPosMap _atomPosMap2 {};

    parameter _mode;
    AtomGroup *_testGroup = new AtomGroup();
	glm::mat4x4 _transform = glm::mat4(1.f);
    glm::vec3 _axis1 {};
    glm::vec3 _x {};
    glm::vec3 _y {};
    glm::vec3 _xTrans {};
    glm::vec3 _yTrans {};

    std::map<std::string, CollisionBox> boxes;
    bool _map {false};
};
#endif
