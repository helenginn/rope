//
// Created by romain on 08/07/2026.
//

#ifndef __vagabond__RotamerBouquet__
#define __vagabond__RotamerBouquet__

#include "RotamerStore.h"
#include "RotamerCollisionBox.h"

class Bouquet2
{
public:
    RotamerStore store {};
    CollisionBox collision {};
    std::vector<Bouquet2> residueBouquets;
    void storeRotamers(std::map<ResidueId, std::map<Atom*,  std::vector<glm::vec3>>> &rotamers)
    {
        store = store.store(rotamers);
    }

    void storeRotRes()
    {
        std::vector<RotamerStore> residueStore {};
        residueStore = store.residueStore();
        for (auto positions : residueStore)
        {
            Bouquet2 resBouquet = Bouquet2(positions);
            resBouquet.collision = CollisionBox(positions.getPos());
            residueBouquets.emplace_back(resBouquet);
        }
    }

    void updatePosition(glm::mat3x3 transformationMat)
    {
        store.updatePositions(transformationMat);
    }

    AtomPosMap move(glm::mat4x4 const &transformationMat, std::string const &chain = "")
    {
        if (transformationMat != glm::mat4x4(1.0))
        {
            store.move(transformationMat, chain);
            for (auto &bouquets : residueBouquets)
            {
                bouquets.store.move(transformationMat, chain);
                bouquets.collision.updateVert(bouquets.store.getPos());
            }
            collisionList();
        }
        return extractForGUI(store);
    }

    std::pair<ResidueId,std::string> name()
    {
        return store.name();
    }

    AtomPosMap extractForGUI(RotamerStore &storeToRender)
    {
        return store.extractForGUI(storeToRender);
    }

    std::vector<std::vector<glm::vec3>> getVertices()
    {
        std::vector<std::vector<glm::vec3>> vert;
        for (auto bouquets : residueBouquets)
            if (bouquets.collision.colliding())
                vert.emplace_back(bouquets.collision.collisionCube());
        return vert;
    }
    std::vector<std::pair<ResidueId,ResidueId>> collisionList()
    {
        for (auto &residues : residueBouquets)
            residues.collision.clearCollision();
        std::vector<std::pair<ResidueId, ResidueId>> collisions {};
        for (int x = 0; x <= (residueBouquets.size()-1)/2; x++)
        {
            for (int y = 0; y < residueBouquets.size(); y++)
            if (x != y  && residueBouquets[x].name().second != residueBouquets[y].name().second)
            {
                residueBouquets[x].collision.collidesWith(residueBouquets[y].collision);
            }
        }
        return collisions;
    }
};
#endif
