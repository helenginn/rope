//
// Created by romain on 11/05/2026.
//

#include "RotamerModifier.h"

#include "vagabond/utils/AcquireCoord.h"

#include <vagabond/core/engine/Task.h>
#include <vagabond/core/TabulatedData.h>
#include <vagabond/core/BondCalculator.h>
#include <vagabond/core/BondSequenceHandler.h>
#include <vagabond/core/LocalMotion.h>
#include <vagabond/core/Instance.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/TorsionBasis.h>
#include <vagabond/core/Result.h>
#include <vagabond/core/engine/CoordManager.h>
#include <vagabond/core/Rotamers.h>
#include <vagabond/core/Parameter.h>
#include <vagabond/core/AtomPosMap.h>

#include "vagabond/gui/elements/Line.h"


RotamerModifier::RotamerModifier(Instance *inst)
{
    setInstance(inst);
    _instance->load();
    _lib = new RotamerLibrary;
    prepareResources();
    prepareMemory();
    //submitJobAndRetrieve(0,Map);
    unifiedTorsionFetcher();
    //generateRotamerMapPosition();
    //axisForChain();

}
RotamerModifier::~RotamerModifier()
{
    _instance->unload();
}

float RotamerModifier::submitJobAndRetrieve(float weight, parameter a)
{
    _mode = a;
    if (_mode != MoveX && _mode != MoveY )
    {
        submitJob(weight);

        Result *r = _resources.calculator->acquireObject();;

        if (_mode == Map && !_map) //generating an AtomPosMap from the iteration of all the rotamers (to generate the multi-rotamers structure)
        {
            AtomPosList list = r->apl;
            for (auto const& atomWithPos : list)
            {

                _testGroup->add(atomWithPos.atom);
                // _allRotamer.atoms[atomWithPos.atom->code()].emplace_back(atomWithPos.atom);
                // _allRotamer.positions[atomWithPos.atom].emplace_back(atomWithPos.wp.ave);>
                bool isPresent {false};
                if (!_allRotamer.positions[atomWithPos.atom].empty()/*&& !atomWithPos.atom->isMainChain()*/)
                {
                    int x = 0;
                    while (x <= _allRotamer.positions[atomWithPos.atom].size())
                    {

                        if (_allRotamer.positions[atomWithPos.atom][x] == atomWithPos.wp.ave)
                        {
                            isPresent = true;
                            break;
                        }
                        x++;
                    }
                }
                if (isPresent == false)
                {
                    _allRotamer.positions[atomWithPos.atom].emplace_back(atomWithPos.wp.ave);
                }
                _atomPosMap[atomWithPos.atom].samples.emplace_back(atomWithPos.wp.ave);


            }

            r->transplantPositions(false);
            r->destroy();

            return weight;
        }

        r->transplantPositions(false);

        r->destroy();
        return weight;
    }

    move(weight, _mode);
    return weight;

}

void RotamerModifier::move(float weight, parameter xy)
{
    Result *r = new Result;
    if (!_atomPosMap.empty())
    {
        _atomPosMap2.clear();
        if (xy == MoveX)
        {
            _xTrans = _x*glm::vec3(weight*3);
        }
        if (xy == MoveY)
        {
            _yTrans = _y*glm::vec3(weight*3);
        }
        for ( auto const& x : _atomPosMap)
        {
            if (x.first->chain() == "B")
            {
                for (auto &pos : x.second.samples)
                {
                    _atomPosMap2[x.first].samples.emplace_back(_xTrans+_yTrans+ pos);
                    r->aps[x.first].samples.emplace_back(_xTrans+_yTrans+ pos);
                }
            }
            else for (auto &pos : x.second.samples)
                _atomPosMap2[x.first].samples.emplace_back(pos);

        }

        r->transplantPositions(false);
        r->destroy();
    }
    primaryCollisionBoxes();

    if (boxes["A"].xMin)
    {
        if (isIntersection(boxes["A"], boxes["B"])==true)
            std::cout << "CONTACT"<< std::endl;
        else
            std::cout << "TOO FAR"<< std::endl;
    }
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
    Flag::Calc calc = Flag::Calc(Flag::DoTorsions /*|Flag::DoSuperpose*/); // Sets calculation flags

    sequences->calculate(calc, {weight}, &first_hook, &final_hook); // Calculates sequences

    BondSequence* firstSequence = sequences->sequence(); // Gets the first sequence
    Flag::Extract gets = Flag::Extract(Flag::AtomVector); // Sets extraction flags

    let_sequence_go = sequences->extract(gets, submit_result, final_hook); // Extracts data
    _resources.tasks->addTask(first_hook); // Adds task to the task list

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
                Rot->initialAngle  = torsion->refinedAngle();
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
    _group = _instance->currentAtoms();
    _group->recalculate();
    std::vector<AtomGroup *> subsets = _group->connectedGroups();
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

void RotamerModifier::saveStructure(std::string name)
{
    AtomGroup *group = _instance->currentAtoms();
    group->writeToFile(name);
}

void RotamerModifier::unifiedTorsionFetcher()
{
    CoordManager* coordManager = _resources.sequences->manager();
    auto sideChainPlusX = [this](const Coord::Get &get, const int &idx)
    {
        if (_params[idx]->isTorsion())
            {
                BondTorsion *torsion = static_cast<BondTorsion *>(_params[idx]);
                if (!_params[idx]->coversMainChain())
                {
                    float initialTorsion = torsion->refinedAngle();
                    std::string resName = _params[idx]->owningAtom()->code(); //get the 3 letters code of the bond's residue -> maybe bug there? (is it possible to get the wrong residue?)

                    if (torsion->shortDesc().substr(0,3) == "chi")
                    {
                        int rotamerNumber = 0;
                        if (_mode == Slider)
                        {
                            rotamerNumber = get(0) * (_lib->_allRotamers[resName].size()-1.f); // slider goes from 0 to 1, so this line enable going through all the rotamers for each residue, with the same slider range
                            _RotamerMemory[idx].loaded = false;
                            _RotamerMemory[idx].RotamerValue = rotamerNumber; // I let these commented line because it can help me debug the function
                            float targetTorsion = _lib->_allRotamers[resName][rotamerNumber].chi[torsion->shortDesc()[3]-1 - '0']; // don't like the operation of char substraction in chi[index]
                            return targetTorsion - initialTorsion;
                        }
                        if (_mode == Reset)
                        {
                            _RotamerMemory[idx].RotamerValue = 0;
                            _RotamerMemory[idx].loaded = true;
                            return _RotamerMemory[idx].initialAngle-initialTorsion;
                        }
                        if (_mode == Map)
                        {
                            rotamerNumber = get(0);
                            if (rotamerNumber > _lib->_allRotamers[resName].size()-1)
                            {
                                return 0.f;
                            }
                            float targetTorsion = _lib->_allRotamers[resName][rotamerNumber].chi[torsion->shortDesc()[3]-1 - '0'];
                            return targetTorsion - initialTorsion;
                        }
                    }
                }
            }
            return 0.f;
    };
    coordManager->setTorsionFetcher(sideChainPlusX);
}

void RotamerModifier::generateRotamerMapPosition() //iteration for all the rotamers, maybe optimization issues
{
    if (_map == false)
    {
        for (int x = 0; x <= 34; x++)
        {
            submitJobAndRetrieve(x, Map);
        }
        Result *r = new Result;
        r->aps = _atomPosMap;
        r->transplantPositions();
        r->destroy();
        bouquetInitializer();
    }
    //axisForChain();
    _map = true;
}

void RotamerModifier::bouquetInitializer()
{

    for ( auto& [atom, pos] : _allRotamer.positions)
    {
        for (auto x : pos)
        {
            _allRotamer.resBouquet[atom->residueId()][atom].push_back(x);
        }
    }

}


glm::vec3 RotamerModifier::axisForChain(points p, std::string chainName)
/* Plan :
 * - get two atoms at the start and end of each structures (considered to be alpha helices from start to end)
 * - draw an axis from them
 * - conpute them for both of the present helix
 */
{
    glm::vec3 firstPosition {};
    glm::vec3 secondPosition {};
    bool firstFound = false;

    for (auto atom : _group->atomVector())
    {
        if (atom->chain() == chainName && atom->atomName() == "CA" && !firstFound)
        {
            firstPosition = atom->initialPosition();
            firstFound = true;
        }
        if (atom->chain() == chainName && atom->atomName() == "CA" && firstFound)
        {
            secondPosition = atom->initialPosition();
        }
        std::cout << "\tFirst atom position : " << firstPosition << " and second atom position : " << secondPosition << std::endl;

    }
    std::cout << "First atom position : " << firstPosition << " and second atom position : " << secondPosition << std::endl;
    _axis1 = secondPosition - firstPosition;
    glm::vec3 firstPointLeft = firstPosition + glm::vec3(0,5,0)-_axis1;
    glm::vec3 bigLine = firstPosition +_axis1*glm::vec3(1.1);
    std::cout << "axisForChain :" << std::endl << "\tVector _axis1 :  " << _axis1 << std::endl;

    if (p == Start)
        return firstPosition-_axis1*glm::vec3(0.3);
    if (p == End)
        return bigLine;
    //Line axis1 = new Line();
    // Line axis2 = new Line(true);
    // // axis1.addPoint(firstPosition);
    // // axis1.addPoint(secondPosition);
    // axis2.addPoint(bigLine);
    // axis2.addPoint(firstPosition);
    //Result *r = new Result;


}
bool RotamerModifier::isIntersection(CollisionBox a, CollisionBox b)
{
return (
    a.xMin <= b.xMax &&
    a.xMax >= b.xMin &&
    a.yMin <= b.yMax &&
    a.yMax >= b.yMin &&
    a.zMin <= b.zMax &&
    a.zMax >= b.zMin
    );
}

std::vector<glm::vec3> RotamerModifier::makePlan()
/* Plan:
 * - take the reference helix (the one that will stay static)
 * - generate a plan perpendicular to this helix' axis
 * - create two vectors that will enable movement of the moving helix
 */
{
    std::vector<glm::vec3> axis;

    glm::vec3 p1 = axisForChain(Start, "A");
    glm::vec3 p2 = axisForChain(End, "A");
    glm::vec3 normal = glm::normalize(p2 - p1);
    glm::vec3 midPoint = (p1 + p2)/ glm::vec3(2.f);
    _x = glm::normalize(cross(normal, vec3(1.0,0.0,0)));
    _y = glm:: normalize(cross(normal, _x));
    axis.emplace_back(normal);
    axis.emplace_back(_x);
    axis.emplace_back(_y);
    return axis;
}


void RotamerModifier::primaryCollisionBoxes()
{
    boxes.clear();
    for (auto &atom : _atomPosMap2)
    {
        std::string chainName = atom.first->chain();
        for (auto &pos : atom.second.samples)
        {
            if (!boxes[chainName].xMin)
            {
                boxes[chainName].xMin = pos.x;
                boxes[chainName].xMax = pos.x;
            }
            else if (pos.x < boxes[chainName].xMin)
            {
                boxes[chainName].xMin = pos.x;
            }
            else if (pos.x > boxes[chainName].xMax)
            {
                boxes[chainName].xMax = pos.x;
            }

            if (!boxes[chainName].yMin)
            {
                boxes[chainName].yMin = pos.y;
                boxes[chainName].yMax = pos.y;

            }
            else if (pos.y < boxes[chainName].yMin)
            {
                boxes[chainName].yMin = pos.y;
            }
            else if (pos.y > boxes[chainName].yMax)
            {
                boxes[chainName].yMax = pos.y;
            }

            if (!boxes[chainName].zMin)
            {
                boxes[chainName].zMin = pos.z;
                boxes[chainName].zMax = pos.z;

            }

            if (pos.z < boxes[chainName].zMin)
            {
                boxes[chainName].zMin = pos.z;
            }
            else if (pos.z > boxes[chainName].zMax)
            {
                boxes[chainName].zMax = pos.z;
            }
        }
    }
//std::cout << "Collision boxes made" << std::endl << '\t' << boxes["B"].xMin << std::endl;
}