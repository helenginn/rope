//
// Created by romain on 29/04/2026.
//
#ifndef __vagabond__Rotamers__
#define __vagabond__Rotamers__
#include <string>
#include <vector>
#include <map>
struct Rotamer
{
    int id{};
    std::string name;
    std::string residue;
    std::vector<float> chi;
    std::vector<float> chi_width;
};
struct RotamerMap
{
    bool loaded {false};
    float initialAngle {};
    int RotamerValue {0};  // store the rotamer number present in the penultimate_lib. if rotamer number = 0, initialChiAngles are loaded
};
class RotamerLibrary
{
public:
    RotamerLibrary();
    std::map<std::string,std::vector<Rotamer>> loadRotamersFromJson(std::string const &filepath);
    std::vector<Rotamer> rotamersForResidues(std::string resName);
    std::map<std::string, std::vector<Rotamer>> _allRotamers {};


private:
    std::string const _filepath = "assets/geometry/penultimate_lib.json";
};

#endif
