//
// Created by romain on 08/07/2026.
//

#ifndef __vagabond__RotamerBouquet__
#define __vagabond__RotamerBouquet__

#include "RotamerStore.h"
#include "RotamerCollisionBox.h"

class Bouquet
{
public:
    void storeRotamers(std::map<ResidueId, std::map<Atom*,  std::vector<glm::vec3>>> &rotamers)
    {
        store = store.store(rotamers);
        storeRotRes();
    }

    void storeRotamers(RotamerStore &pos)
    {
        store = pos;
    }

    void storeRotRes()
    {
        std::vector<RotamerStore> residueStore {};
        residueStore = store.residueStore();
        for (auto &positions : residueStore)
        {
            Bouquet *resBouquet = new Bouquet();
            resBouquet->storeRotamers(positions);
            resBouquet->collision = CollisionBox(positions.getPos());
            residueBouquets.emplace_back(resBouquet);
        }
    }

    AtomPosMap move(glm::mat4x4 const &transformationMat, std::string const &chain = "")
    {
        if (transformationMat != glm::mat4x4(1.0))
        {
            store.move(transformationMat, chain);
            for (auto &bouquets : residueBouquets)
            {
                bouquets->store.move(transformationMat, chain);
                bouquets->collision.updateVert(bouquets->store.getPos());
            }
            collisionList();
        }
        return extractForGUI();
    }

    std::pair<ResidueId,std::string> name()
    {
        return store.name();
    }
    std::vector<Bouquet *> bouquetsForChain(const std::string& Name)
    {
        std::vector<Bouquet *> bouquetsChain {};
        for (auto bouquets : residueBouquets)
        {
            // std::cout << bouquets->name().first.insert << std::endl;
            if (bouquets->name().first.insert == Name)
                bouquetsChain.push_back(bouquets);
        }
        return bouquetsChain;
    }

    AtomPosMap extractForGUI()
    {
        return store.extractForGUI();
    }

    std::vector<std::pair<glm::vec3,glm::vec3>> getVertices() const
    {
        std::vector<std::pair<glm::vec3,glm::vec3>> vert;
        for (auto const &bouquets : residueBouquets)
            if (bouquets->collision.colliding())
                vert.emplace_back(bouquets->collision.collisionCube());
        return vert;
    }

    std::vector<std::pair<ResidueId,ResidueId>> collisionList()
    {
        for (auto &residues : residueBouquets)
            residues->collision.clearCollision();
        std::vector<std::pair<ResidueId, ResidueId>> collisions {};
        for (int x = 0; x <= (residueBouquets.size()-1)/2; x++)
        {
            for (int y = 0; y < residueBouquets.size(); y++)
            if (x != y  && residueBouquets[x]->name().second != residueBouquets[y]->name().second)
            {
                residueBouquets[x]->collision.collidesWith(residueBouquets[y]->collision);
            }
        }
        return collisions;
    }

    glm::vec3 positionReporter(Bouquet* bouquet)
    {
        return bouquet->store.positionForReporter();
    }
    
    std::vector<glm::vec3> axis(const std::string &chainName, bool trueAxis = false)
    {
        glm::vec3 firstPoint {};
        glm::vec3 endPoint {};
        int min {};
        int max {};
        const std::vector<Bouquet*> bouquetsList {bouquetsForChain(chainName)};

        if (trueAxis)
        {
            int y {0};
            int z {0};
            int counter {0};
            glm::vec3 currentPos {};
        //     std::vector<glm::vec3> positionsArray {};
        //     glm::vec3 centroid {0};
        //     for (auto const &bouquets : bouquetsList)
        //     {
        //         currentPos = bouquets->store.positionForReporter();
        //         positionsArray.push_back(currentPos);
        //         centroid += currentPos;
        //     }
        //     centroid /= positionsArray.size();
            for (auto const &bouquets : bouquetsList)
            {
                currentPos = bouquets->store.positionForReporter();
                if (!firstPoint.x)
                {
                    firstPoint.x = currentPos.x;
                    endPoint.x = currentPos.x;
                }
                else if (currentPos.x < firstPoint.x)
                {
                    firstPoint.x = currentPos.x;
                }
                else if (currentPos.x > endPoint.x)
                {
                    endPoint.x = currentPos.x;
                }
                y = y+currentPos.y;
                z = z+currentPos.z;
                counter ++;
            }
            y = y/counter;
            z = z/counter;
            firstPoint.y = y;
            firstPoint.z = z;
            endPoint.y = y;
            endPoint.z = z;
            return {firstPoint, endPoint};
        }
        for ( auto const &bouquets : bouquetsList)
        {
            if (!firstPoint.x)
            {
                firstPoint = bouquets->store.positionForReporter();
                endPoint = firstPoint;
                min = bouquets->store.name().first.as_num();
                max = min;
            }
            else if (bouquets->store.name().first.as_num() < min)
            {
                firstPoint = bouquets->store.positionForReporter();
                min = bouquets->store.name().first.as_num();
            }
            else if (bouquets->store.name().first.as_num() > max)
            {
                endPoint = bouquets->store.positionForReporter();
                max = bouquets->store.name().first.as_num();
            }
        }
        return {firstPoint, endPoint};
    }
protected:
    RotamerStore store {};
    CollisionBox collision {};
    std::vector<Bouquet*> residueBouquets;
};
#endif
