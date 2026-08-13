//
// Created by romain on 08/07/2026.
//

#include "RotamerStore.h"
RotamerStore RotamerStore::store(std::map<ResidueId, std::map<Atom*,  std::vector<glm::vec3>>> &rotamers)
{
    RotamerStore newStore {};
    int globalPos = 0;

    for (auto& [res, atomPos] : rotamers)
    {
        ResRot residueRot;
        residueRot.Id() = res;
        for (int rotaNumber = 0; rotaNumber < atomPos.begin()->second.size(); rotaNumber++)
        {
            int counter = 0;
            for (auto& [atom, positions] : atomPos)
            {
                newStore.atoms.emplace_back(atom);
                newStore.positionArray.emplace_back(positions[rotaNumber]);
                counter++;
            }
            residueRot.Rotamer() = rotaNumber;
            newStore.storage[residueRot].first = globalPos;
            newStore.storage[residueRot].second = counter;
            globalPos += counter;
        }
    }
    return newStore;
}

std::vector<RotamerStore> RotamerStore::residueStore()
{
    std::map<ResidueId,std::map<ResRot,LookUpInfo>> tempStorage;
    std::vector<RotamerStore> residueStore;
    for (auto index: storage)
    {
        tempStorage[index.first.first].emplace(index);
    }
    for (auto restore: tempStorage)
    {
        RotamerStore sto = RotamerStore();
        for (auto [object, lookup]: restore.second)
        {
            int counter = 0;
            int globalPos = 0;
            for (int x = lookup.start(); x < (lookup.start()+lookup.length()); x++)
            {
                std::cout << x << std::endl;
                sto.atoms.emplace_back(atoms[x]);
                sto.positionArray.emplace_back(positionArray[x]);
                counter++;

            }
            sto.storage[object].first = globalPos;
            sto.storage[object].second = counter;
        }
        residueStore.emplace_back(sto);
    }
    return residueStore;
}

std::vector<glm::vec3> RotamerStore::getPos()
{
    return positionArray;
}

std::vector<glm::vec3> RotamerStore::positionFor(ResRot const &rotamer)
{
    std::vector<glm::vec3> positions{};
    LookUpInfo lookup = storage[rotamer];
    for (int x = lookup.start(); x < (lookup.start() + lookup.length()); x++)
    {
        positions.emplace_back(positionArray[x]);
    }
    return positions;
}

void RotamerStore::updatePositions(glm::mat4x4 transformationMat)
{
    transformationMat = glm::inverse(transformationMat);
    for (glm::vec3 &pos : positionArray)
    {
        pos = transformationMat * glm::vec4(pos,1.0f);
    }
}

void RotamerStore::move(glm::mat4x4 parameters, std::string chain)
{
    for (int length = 0; length <= positionArray.size()-1; length++)
    {
        if (atoms[length]->chain() == chain || chain.empty())
        {
            positionArray[length] =glm::vec3(parameters*glm::vec4(positionArray[length],1.0f));
        }
    }
}

AtomPosMap RotamerStore::extractForGUI(RotamerStore &storeToRender)
{
    AtomPosMap positionMap {};
    int max {};
    for (auto pairs : storeToRender.storage)
    {
        int rotaNumber = pairs.first.second;
        if (rotaNumber > max)
            max = rotaNumber;
    }
    std::pair<ResRot, LookUpInfo> memoryResRot {};
    for (auto pairs : storeToRender.storage)
    {
        if (positionMap.empty() || pairs.first.first == memoryResRot.first.first)
        {
            memoryResRot = pairs;
            for (int i = pairs.second.start(); i < (pairs.second.start() + pairs.second.length()); i++)
            {
                positionMap[storeToRender.atoms[i]].samples.emplace_back(storeToRender.positionArray[i]);
            }
        }
        else
        {
            int counter = max-memoryResRot.first.Rotamer();
            for (int i = 0; i < counter; i++)
            {
                for (int X = memoryResRot.second.start(); X < (memoryResRot.second.start() + memoryResRot.second.length()); X++)
                {
                    positionMap[storeToRender.atoms[X]].samples.emplace_back(storeToRender.positionArray[X]);
                }
            }
            memoryResRot = pairs;
            for (int i = pairs.second.start(); i < (pairs.second.start() + pairs.second.length()); i++)
            {
                positionMap[storeToRender.atoms[i]].samples.emplace_back(storeToRender.positionArray[i]);
            }
        }
    }
    return positionMap;
}

std::pair<ResidueId,std::string> RotamerStore::name()
{
    return std::pair(atoms[0]->residueId(), atoms[0]->chain());
}