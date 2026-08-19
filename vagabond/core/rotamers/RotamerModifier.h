//
// Created by romain on 11/05/2026.
//

#ifndef __vagabond__RotamerModifier__
#define __vagabond__RotamerModifier__

#include "Rotamers.h"
#include "RotamerBouquet.h"
#include "StructureModification.h"
#include "AtomGroup.h"

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
    std::vector<glm::vec3> axisForChain(std::string const &chainName);

    void makePlan();
    std::vector<glm::vec3> drawChainAxis();
    std::vector<glm::vec3> drawAxis();

    void move(float weight, parameter xy);
    void analysis(int timePoints, std::vector<glm::vec3> startPos);
    void analysisTest(int timePoints, std::vector<glm::vec3> startPos);
    std::vector<std::pair<glm::vec3,glm::vec3> > getVertices() const
    {
        return _bouquet->getVertices();
    }
private:
    std::map<ResidueId, std::map<Atom*,  std::vector<glm::vec3>>> _resBouquet {};
    AtomGroup *_group;
    std::vector<Parameter *> _params;
    RotamerLibrary *_lib;
    RotamerMap *Rot;
    std::map<int, RotamerMap> _RotMem;

    Bouquet *_bouquet;
    parameter _mode;
    glm::vec3 _axisMain {};
    glm::vec3 _axisSecondary {};
    glm::vec3 _x {};
    glm::vec3 _y {};
    glm::vec3 _normal {};
    float _memoryY {0};
    float _memoryX {0};
    bool _referential {false};
    bool _map {false};
};
#endif
