//
// Created by romain on 29/04/2026.
//

#include "Rotamers.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include "config/config.h"

RotamerLibrary::RotamerLibrary()
{
    _allRotamers = loadRotamersFromJson(_filepath);
    std::cout << "All rotamers: " << _allRotamers.size() << '\n';
}

std::map<std::string,std::vector<Rotamer>> RotamerLibrary::loadRotamersFromJson(std::string const &filepath)
{
    std::ifstream f;
    std::cout << std::string(DATA_DIRECTORY) + "/" + filepath << " test " << std::endl;
    f.open( std::string(DATA_DIRECTORY) + "/" + filepath);
    if (!f.is_open())
    {
        throw std::runtime_error("Could not open rotamer JSON: " + filepath);
    }

    nlohmann::json j;
    f >> j;
    f.close();

    std::vector<Rotamer> rotamers;
    std::map<std::string,std::vector<Rotamer>> allRotamers;

    for (auto &[resname, entries] : j.items())
    {
        for (auto &entry : entries)
        {
            Rotamer r;
            r.id = entry["id"];
            r.name = entry["name"];
            r.residue = resname;

            // parse chi angles dynamically (chi_1 to chi_4)
            for (int i = 1; i <= 4; i++)
            {
                std::string chiKey = "chi_" + std::to_string(i);
                std::string widthKey = "chi-width_" + std::to_string(i);

                if (entry.contains(chiKey))
                {
                    r.chi.push_back(entry[chiKey].is_null() ? 0.0f // 0 == no chi value
                                                               : entry[chiKey].get<float>());
                    r.chi_width.push_back(entry[widthKey].is_null() ? -1.0f //
                                                               : entry[widthKey].get<float>());
                }
            }
            rotamers.push_back(r);
        }
        allRotamers[resname] = rotamers;
        rotamers.clear();
    }

    return allRotamers;
}

std::vector<Rotamer> RotamerLibrary::rotamersForResidues(std::string resName)
{
    std::vector<Rotamer> result;
    for (const Rotamer &r : _allRotamers[resName])
    {
        result.push_back(r);
    }
    return result;
}