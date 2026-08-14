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
    for (auto &index: storage)
    {
        tempStorage[index.first.first].emplace(index);
    }
    for (auto &restore: tempStorage)
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
glm::vec3 RotamerStore::positionForReporter()
{
    glm::vec3 position{};
    LookUpInfo lookup = storage.begin()->second;
    for (int x = lookup.start(); x < (lookup.start() + lookup.length()); x++)
    {
        if (atoms[x]->isReporterAtom())
        {
            position = positionArray[x];
            break;
        }
    }
    return position;
}

void RotamerStore::move(glm::mat4x4 parameters, std::string const &chain)
{
    for (int length = 0; length <= positionArray.size()-1; length++)
    {
        if (atoms[length]->chain() == chain || chain.empty())
        {
            positionArray[length] =glm::vec3(parameters*glm::vec4(positionArray[length],1.0f));
        }
    }
}

AtomPosMap RotamerStore::extractForGUI()
{
    AtomPosMap positionMap {};
    int max {};
    for (auto pairs : storage)
    {
        int rotaNumber = pairs.first.second;
        if (rotaNumber > max)
            max = rotaNumber;
    }
    std::pair<ResRot, LookUpInfo> memoryResRot {};
    for (auto pairs : storage)
    {
        if (positionMap.empty() || pairs.first.first == memoryResRot.first.first)
        {
            memoryResRot = pairs;
            for (int i = pairs.second.start(); i < (pairs.second.start() + pairs.second.length()); i++)
            {
                positionMap[atoms[i]].samples.emplace_back(positionArray[i]);
            }
        }
        else
        {
            int counter = max-memoryResRot.first.Rotamer();
            for (int i = 0; i < counter; i++)
            {
                for (int X = memoryResRot.second.start(); X < (memoryResRot.second.start() + memoryResRot.second.length()); X++)
                {
                    positionMap[atoms[X]].samples.emplace_back(positionArray[X]);
                }
            }
            memoryResRot = pairs;
            for (int i = pairs.second.start(); i < (pairs.second.start() + pairs.second.length()); i++)
            {
                positionMap[atoms[i]].samples.emplace_back(positionArray[i]);
            }
        }
    }
    return positionMap;
}

std::pair<ResidueId,std::string> RotamerStore::name()
{
    return {storage.begin()->first.first, atoms[0]->chain()};
}