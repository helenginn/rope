#ifndef __derivative__functions__
#define __derivative__functions__

#include "../utils/glm_import.h"
using namespace glm; 

float alphaGradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                       const glm::vec3 &D, const glm::vec3 &H,
                                       const glm::vec3 &A, bool isDHBond);

float alphaGradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                       const glm::vec3 &D, const glm::vec3 &H,
                                       const glm::vec3 &A);

float betaGradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                      const glm::vec3 &H, const glm::vec3 &A,
                                      const glm::vec3 &AA, bool isAABond);

float betaGradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                      const glm::vec3 &H, const glm::vec3 &A,
                                      const glm::vec3 &AA);

float dihedral1GradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &C, const glm::vec3 &D,
                                           const glm::vec3 &H, const glm::vec3 &A, bool isDHBond);

float dihedral1GradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &C, const glm::vec3 &D,
                                           const glm::vec3 &H, const glm::vec3 &A);

float dihedral2GradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &D, const glm::vec3 &H,
                                           const glm::vec3 &A, const glm::vec3 &AA);

float dihedral2GradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &D, const glm::vec3 &H,
                                           const glm::vec3 &A, const glm::vec3 &AA, bool isAABond);


#endif















