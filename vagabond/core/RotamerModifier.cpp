//
// Created by romain on 11/05/2026.
//

#include "RotamerModifier.h"

#include "vagabond/utils/AcquireCoord.h"


#include <vagabond/core/engine/Task.h>
#include <vagabond/core/grids/AtomMap.h>
#include <vagabond/core/grids/ArbitraryMap.h>
#include <vagabond/core/TabulatedData.h>
#include <vagabond/core/BondCalculator.h>
#include <vagabond/core/BondSequenceHandler.h>
#include <vagabond/core/LocalMotion.h>
#include <vagabond/core/GroupBounds.h>
#include <vagabond/core/Instance.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/Torsion2Atomic.h>
#include <vagabond/core/TorsionBasis.h>
#include <vagabond/core/Result.h>
#include <vagabond/core/engine/CoordManager.h>
#include <vagabond/core/Rotamers.h>

#include "BondAngle.h"


RotamerModifier::RotamerModifier(Instance *inst)
{
    setInstance(inst);
    _instance->load();
    _lib = new RotamerLibrary;
    prepareResources();
    prepareMemory();
}

float RotamerModifier::resetRotamers()
{
    CoordManager* coordManager = _resources.sequences->manager();

    auto sideChainPlusX = [this](const Coord::Get &get, const int &idx)
    {
        if (_params[idx]->isTorsion())
        {
            BondTorsion *torsion = static_cast<BondTorsion *>(_params[idx]);
            if (!_params[idx]->coversMainChain())
            {
                float initialTorsion = torsion->angle();
                std::string resName = _params[idx]->owningAtom()->code();  //get the 3 letters code of the bond's residue -> maybe bug there? (is it possible to get the wrong residue?)
                if (torsion->shortDesc().substr(0,3) == "chi")
                {
                    _RotamerMemory[idx].RotamerValue = 0;
                    _RotamerMemory[idx].loaded = true;
                    return _RotamerMemory[idx].initialAngle-initialTorsion;
                }
            }
        }
        return 0.f;

    };
    coordManager->setTorsionFetcher(sideChainPlusX);
    submitJob(0);

    Result *r = _resources.calculator->acquireObject();
    r->transplantPositions(false);

    r->destroy();
    return 0;
}


float RotamerModifier::submitJobAndRetrieve(float weight)
{
    filteredAngles(weight);
    submitJob(weight);

    Result *r = _resources.calculator->acquireObject();
    r->transplantPositions(false);

    r->destroy();
    return weight;
}
void RotamerModifier::submitJob(float weight)
{
    BaseTask *first_hook = nullptr; // Initialize first hook
    CalcTask *final_hook = nullptr; // Initialize final hook

    CalcTask *calc_hook = nullptr; // Initialize calc hook
    Task<BondSequence *, void *> *let_sequence_go = nullptr; // Initialize let_sequence_go

    BondCalculator *const &calculator = _resources.calculator; // Gets the calculator
    BondSequenceHandler *sequences = _resources.sequences; // Gets the sequences

    /* this final task returns the result to the pool to collect later */
    Task<Result, void *> *submit_result = calculator->actOfSubmission(0); // Submits the result
    Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoSuperpose); // Sets calculation flags

    sequences->calculate(calc, {weight}, &first_hook, &final_hook); // Calculates sequences

    BondSequence* firstSequence = sequences->sequence(); // Gets the first sequence
    Flag::Extract gets = Flag::Extract(Flag::AtomVector); // Sets extraction flags

    let_sequence_go = sequences->extract(gets, submit_result, final_hook); // Extracts data
    _resources.tasks->addTask(first_hook); // Adds task to the task list

}


void RotamerModifier::filteredAngles(float x)
{
    CoordManager* coordManager = _resources.sequences->manager();
    auto sideChainPlusX = [this, x](const Coord::Get &get, const int &idx)
    {
        if (_params[idx]->isTorsion())
        {
            BondTorsion *torsion = static_cast<BondTorsion *>(_params[idx]);
            if (!_params[idx]->coversMainChain())
            {
                float initialTorsion = torsion->angle();
                std::string resName = _params[idx]->owningAtom()->code();  //get the 3 letters code of the bond's residue -> maybe bug there? (is it possible to get the wrong residue?)
                int rotamerNumber = x * (_lib->_allRotamers[resName].size()-1); // slider goes from 0 to 1, so this line enable going through all the rotamers for each residue, with the same slider range
                if (torsion->shortDesc().substr(0,3) == "chi")
                {
                    _RotamerMemory[idx].RotamerValue = rotamerNumber;
                    // if (_RotamerMemory[idx].loaded)
                    // {
                    //     _RotamerMemory[idx].loaded = false;
                    //     return _lib->_allRotamers[resName][rotamerNumber].chi[torsion->shortDesc()[3]-1 - '0'];
                    // }

                    return _lib->_allRotamers[resName][rotamerNumber].chi[torsion->shortDesc()[3]-1 - '0'] // don't like the operation of char substraction in chi[index]
                    - initialTorsion;
                }
            }
        }
        return 0.f;
    };
    coordManager->setTorsionFetcher(sideChainPlusX);
}
void RotamerModifier::prepareMemory()//const Coord::Get &get, const int &idx)
{
    for (int idx = 0; idx <= _params.size()-1; idx++)
    {
        if (_params[idx]->isTorsion())
        {
            BondTorsion *torsion = static_cast<BondTorsion *>(_params[idx]);
            Rot = new RotamerMap;
            if (!_params[idx]->coversMainChain())
            {
                Rot->initialAngle  = torsion->angle();
                Rot->RotamerValue = 0;
                Rot->loaded = true;
                _RotamerMemory[idx] = *Rot;
            }
        }
    }
}

void RotamerModifier::prepareResources()
{
    const int threads = 1;
    _resources.allocateMinimum(threads);
    // set up per-bond/atom calculation
    AtomGroup *group = _instance->currentAtoms();

    std::vector<AtomGroup *> subsets = group->connectedGroups();
    for (AtomGroup *subset : subsets)
    {
        Atom *anchor = subset->chosenAnchor();
        _resources.sequences->addAnchorExtension(anchor);
    }
    _resources.sequences->setIgnoreHydrogens(true);
    _resources.sequences->setup();
    _resources.sequences->prepareSequences();
     _params =
    _resources.sequences->torsionBasis()->parameters();
}

/*
BondSequenceHandler *sequences = _resources.sequences;
const std::vector<Parameter *> &params =
sequences->torsionBasis()->parameters();
RTAngles filtered = filteredAngles();
filtered.attachInstance(_instance);
filtered.filter_according_to(params);
auto grab_torsion = [filtered](const Coord::Get &get, const int &idx) -> float
{
    return filtered.storage(idx) * get(0);
};
CoordManager *manager = _resources.sequences->manager();
const std::vector<AtomBlock> &blocks =
_resources.sequences->sequence()->blocks();
manager->setTorsionFetcher(grab_torsion);*/



// For debugging purposes

// void RotamerModifier::filteredAngles(float x)
// {
//     //std::vector<Rotamer> RotLib = _lib->rotamersForResidues("lys");
//     CoordManager* coordManager = _resources.sequences->manager();
//     auto sideChainPlusX = [this, x](const Coord::Get &get, const int &idx)
//     {
//         if (_params[idx]->isTorsion())
//         {
//
//             BondTorsion *torsion = static_cast<BondTorsion *>(_params[idx]);
//             if (!_params[idx]->coversMainChain())
//             {
//                 float initialTorsion = torsion->angle();
//                 std::string resName = _params[idx]->owningAtom()->code();
//                 int rotamerNumber = x * (_lib->_allRotamers[resName].size()-1);
//                 int rotamerNumberDebug = x * (_lib->_allRotamers["ASN"].size()-1);
//                 float testangle = _lib->_allRotamers[resName][rotamerNumber].chi[torsion->shortDesc()[3]-1 -'0'];
//
//                 if (torsion->shortDesc().substr(0,3) == "chi")
//                 {
//                     std::string descTot = torsion->shortDesc();
//                     int desc = torsion->shortDesc()[3] - '0';
//                     std::cout << torsion->shortDesc()[3];
//                     return _lib->_allRotamers[resName][rotamerNumber].chi[torsion->shortDesc()[3]-1 - '0'] - initialTorsion;
//                 }
//                 return 0.f;
//             }
//             else
//             {
//                 return 0.f;
//             }
//         }
//         return 0.f;
//     };
//     coordManager->setTorsionFetcher(sideChainPlusX);
// }