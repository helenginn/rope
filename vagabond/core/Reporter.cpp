// vagabond
// Copyright (C) 2022 Helen Ginn
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Please email: vagabond @ hginn.co.uk for more details.

#include "Reporter.h"

#include <iostream>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/FileManager.h>
#include <vagabond/core/Model.h>
#include <vagabond/core/ModelManager.h>
#include <vagabond/core/Sequence.h>

Reporter::Reporter()
{
    FileManager fm = FileManager();
    // BUG: this doesn't actually print anything!
    std::cout << "Reading data from: " << fm.userDirectory() << std::endl << std::endl;
}

void Reporter::report()
{
    std::cout << "ENTITIES" << std::endl;
    std::cout << "========" << std::endl;
    EntityManager *entity_manager = Environment::env().entityManager();
    std::cout << "No. of entities: " << entity_manager->objectCount() << std::endl;
    for (int i = 0; i < entity_manager->objectCount(); i++)
    {
        Entity &entity = entity_manager->object(i);
        Sequence *sequence = entity.sequence();
        std::cout << "Entity " << i << ": " << entity.name() << std::endl;
        std::cout << "    No. of residues: " << sequence->str().length() << std::endl;
        std::cout << "    Sequence: " << sequence->str() << std::endl;
        std::cout << "    No. of models: " << entity.modelCount() << std::endl;
        std::cout << "    Models: [index, name, no. of molecules]" << std::endl;
        for (int m = 0; m < entity.modelCount(); m++)
        {
            Model *model = entity.models()[m];
            std::cout << "        Model " << m << ": " << model->name() << " (";
            std::cout << model->moleculesForEntity(&entity).size() << ")" << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "MODELS" << std::endl;
    std::cout << "======" << std::endl;
    ModelManager *model_manager = Environment::env().modelManager();
    std::cout << "No. of models: " << model_manager->objectCount() << std::endl;
    std::cout << "[R: refined, NR: not refined]" << std::endl;
    for (int i = 0; i < model_manager->objectCount(); i++)
    {
        Model &model = model_manager->object(i);
        std::cout << "Model " << i << ": " << model.name() << std::endl;
        for (int e = 0; e < entity_manager->objectCount(); e++)
        {
            Entity &entity = entity_manager->object(e);
            std::cout << "    Entity " << e << " (" << entity.name() <<  "): ";
            std::set<Molecule *> molecules = model.moleculesForEntity(&entity);
            int index = 0;
            for (Molecule* m: molecules)
            {
                if (index == 0)
                {
                    std::cout << m->id();
                }
                else
                {
                    std::cout << ", " << m->id();
                }
                if (m->isRefined())
                {
                    std::cout << " [R]";
                }
                else
                {
                    std::cout << " [NR]";
                }
                index++;
            }
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}