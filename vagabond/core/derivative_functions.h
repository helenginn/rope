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


/** rigid body derivatives
*   traslational rotation of H-A, where either the H-side (Donor+Hydrogen)
*   or A-side (Acceptor+ParentAcceptor) moves as one rigid unit
*/

float distanceGradientTranslation(const glm::vec3 &direction, 
                                  const glm::vec3 &H, const glm::vec3 &A, 
                                  bool isHside);

float alphaGradientTranslation(const glm::vec3 &directions, 
                               const glm::vec3 &D, const glm::vec3 &H, 
                               const glm::vec3 &A, bool isHSide);

float betaGradientTranslation(const glm::vec3 &directions, 
                              const glm::vec3 &H, const glm::vec3 &A, 
                              const glm::vec3 &AA, bool isHSide);

float dihedral1GradientTranslation(const glm::vec3 &directions, 
                              const glm::vec3 &DD, const glm::vec3 &D, 
                              const glm::vec3 &H, const glm::vec3 &A,
                              bool isHSide);

float dihedral2GradientTranslation(const glm::vec3 &directions, 
                              const glm::vec3 &D, const glm::vec3 &H, 
                              const glm::vec3 &A, const glm::vec3 &AA, 
                              bool isHSide);



#endif















