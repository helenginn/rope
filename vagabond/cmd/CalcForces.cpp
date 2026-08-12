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
#include "../utils/ResultType.h"
#include "files/File.h"
#include "forces/ForceAnalysis.h"
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <variant>

namespace {
using FileResult = std::variant<std::unique_ptr<File>, std::string>;
std::unique_ptr<File> validateInput(std::string first, std::string last) {
  if (last.empty()) {
    std::cout << "Usage: calc_stress_strain=/path/to/pdb" << std::endl;
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

Result<int, std::string> divide(int a, int b) {
  if (b == 0) {
    return Err(std::string("Division by 0 is not allowed!"));
  }
  return Ok(a / b);
}

void handleTestResult(std::string first, std::string last) {
  int success = divide(10, 2).unwrap();
  std::cout << "10 / 2 = " << success << std::endl;

  int fallback = divide(10, 0).unwrap_or(-1);
  std::cout << "10 / 0 = " << fallback << std::endl;

  auto res = divide(5,0);
  if (res.is_err()) {
    std::cout << "Aborted: " << res.error() << std::endl;
  }
}

void handleCalcForces(std::string first, std::string last) {
  auto file = validateInput(first, last);
  if (!file) {
    return;
  }

  AtomContent *atoms = file->atoms();
  if (!atoms) {
    std::cout << "Could not extract atom content from file: " << last
              << std::endl;
    return;
  }
  ForceAnalysis analysis(atoms);

  analysis.convert();

  analysis.toggleReason(AbstractForce::ReasonBondLength, false);
  analysis.toggleReason(AbstractForce::ReasonBondTorsion, false);
  analysis.toggleReason(AbstractForce::ReasonVdwContact, false);
  analysis.toggleReason(AbstractForce::ReasonElectrostaticContact, false);

  analysis.toggleReason(AbstractForce::ReasonBondAngle, true);
  analysis.calculateUnknown();
}
