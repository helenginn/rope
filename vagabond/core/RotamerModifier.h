//
// Created by romain on 11/05/2026.
//

#ifndef __vagabond__RotamerModifier__
#define __vagabond__RotamerModifier__
#include "Rotamers.h"
#include "StructureModification.h"
#include "Parameter.h"

//Why do the header do not need to see the Class ("Class Rotamer" without including the Rotamer.h -> The header doesn't need to know, because it wont use it')
class RotamerModifier : public StructureModification
{
public:
    RotamerModifier(Instance *inst);
    void prepareResources();
    void filteredAngles(float x);
    float submitJobAndRetrieve(float weight);
    float resetRotamers();
    void submitJob(float weight);
    void prepareMemory();

private:
 //   Instance *_instance{};
    RTAngles _rawAngles;
    AtomGroup *_group;
    std::vector<Rotamer> *_rotamers;
    std::vector<std::vector<float>> _allTorsions;
    float _value {};
    std::vector<Parameter *> _params;
    RotamerLibrary *_lib;
    RotamerMap *Rot;
    std::map<int, RotamerMap> _RotamerMemory;
    //std::list<Residue> _residues;
};
#endif
