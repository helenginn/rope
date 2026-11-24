#pragma once

#include <vagabond/utils/glm_import.h>

class BondAngle;

namespace rope::force_calc
{
  enum class ForceModel
{
  ZScore,
  Units,
};

glm::vec3 bondAngleAxis(const BondAngle &angle);

// Returns the signed torque magnitude
//
}
