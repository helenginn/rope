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

#include "CalcForces.h"
#include "files/File.h"
#include "forces/ForceAnalysis.h"
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <cctype>
#include <variant>

namespace {
using FileResult = std::variant<std::unique_ptr<File>, std::string>;

struct CalcForceArgs
{
  std::string path;
  bool calcWithUnits = false;
};

bool parseBool(std::string value, bool &out)
{
  for (char &c : value)
  {
    c = std::tolower(static_cast<unsigned char>(c));
  }

  if (value == "true" || value == "1" || value == "yes" || value == "on")
  {
    out = true;
    return true;
  }

  if (value == "false" || value == "0" || value == "no" || value == "off")
  {
    out = false;
    return true;
  }

  return false;
}

bool parseCalcForceArgs(const std::string &last, CalcForceArgs &args)
{
  if (last.empty())
  {
    return false;
  }

  size_t comma = last.find(',');
  args.path = (comma == std::string::npos) ? last : last.substr(0, comma);
  args.calcWithUnits = false;

  if (comma != std::string::npos)
  {
    std::string boolText = last.substr(comma + 1);
    if (!parseBool(boolText, args.calcWithUnits))
    {
      std::cout << "Invalid unit toggle '" << boolText
                << "'. Use true/false (or 1/0)." << std::endl;
      return false;
    }
  }

  return !args.path.empty();
}

std::unique_ptr<File> validateInput(std::string first, std::string last) {
  if (last.empty()) {
    std::cout << "Usage: calc_stress_strain=/path/to/pdb[,true|false]"
              << std::endl;
    return nullptr;
  }

  std::unique_ptr<File> file = nullptr;

  try {
    file.reset(File::loadUnknown(last));
  } catch (const std::exception &e) {
    std::cout << "Could not load file: " << e.what() << std::endl;
    return nullptr;
  }

  if (!file) {
    std::cout << "Could not load file: " << last << std::endl;
    return nullptr;
  }
  File::Type type = file->cursoryLook();
  if (!(type & File::MacroAtoms) && !(type & File::CompAtoms)) {
    std::cout << "File has no atoms for force analysis: " << last << std::endl;
    return nullptr;
  }
  return file;
};
} // namespace

void handleCalcForces(std::string first, std::string last) {
  CalcForceArgs args;
  if (!parseCalcForceArgs(last, args))
  {
    std::cout << "Usage: calc_stress_strain=/path/to/pdb[,true|false]"
              << std::endl;
    return;
  }

  auto file = validateInput(first, args.path);
  if (!file) {
    return;
  }

  AtomContent *atoms = file->atoms();
  if (!atoms) {
    std::cout << "Could not extract atom content from file: " << args.path
              << std::endl;
    return;
  }
  ForceAnalysis analysis(atoms);

  analysis.convert(args.calcWithUnits);

  analysis.toggleReason(AbstractForce::ReasonBondLength, false);
  analysis.toggleReason(AbstractForce::ReasonBondTorsion, false);
  analysis.toggleReason(AbstractForce::ReasonVdwContact, false);
  analysis.toggleReason(AbstractForce::ReasonElectrostaticContact, false);

  analysis.toggleReason(AbstractForce::ReasonBondAngle, true);
  analysis.calculateUnknown(args.calcWithUnits);
}
