#include "derivative_functions.h"

float alphaGradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                       const glm::vec3 &D, const glm::vec3 &H,
                                       const glm::vec3 &A, bool isDHBond)
{
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 u = D - H;
    glm::vec3 w = A - H;
    float u_len = glm::length(u);
    float w_len = glm::length(w);
    float dot_uw = glm::dot(u, w);

    glm::vec3 dFdD = (w / (u_len * w_len)) - (dot_uw / (u_len*u_len*u_len*w_len)) * u;
    glm::vec3 dFdA = (u / (u_len * w_len)) - (dot_uw / (u_len*w_len*w_len*w_len)) * w;
    glm::vec3 dFdH = -(dFdD + dFdA);

    float deriv = glm::dot(dFdH, glm::cross(axis, (H - axisA)));
    if (!isDHBond)
        deriv += glm::dot(dFdD, glm::cross(axis, (D - axisA)));

    return deriv;
}

float alphaGradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                       const glm::vec3 &D, const glm::vec3 &H,
                                       const glm::vec3 &A)
{
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 u = D - H;
    glm::vec3 w = A - H;
    float u_len = glm::length(u);
    float w_len = glm::length(w);
    float dot_uw = glm::dot(u, w);

    glm::vec3 dFdA = (u / (u_len * w_len)) - (dot_uw / (u_len*w_len*w_len*w_len)) * w;
    return glm::dot(dFdA, glm::cross(axis, (A - axisA)));
}


float betaGradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                      const glm::vec3 &H, const glm::vec3 &A,
                                      const glm::vec3 &AA, bool isAABond)
{
    // A-side: A moves, H fixed, AA is direct bond so AA does NOT move
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 p = H - A;
    glm::vec3 r = AA - A;
    float p_len = glm::length(p);
    float r_len = glm::length(r);
    float dot_pr = glm::dot(p, r);

    glm::vec3 dFdH = (r / (p_len * r_len)) - (dot_pr / (p_len*p_len*p_len*r_len)) * p;
    glm::vec3 dFdAA = (p / (p_len * r_len)) - (dot_pr / (p_len*r_len*r_len*r_len)) * r;
    glm::vec3 dFdA =  -(dFdH + dFdAA);

    float deriv = glm::dot(dFdA, glm::cross(axis, (A - axisA)));
    if (!isAABond)
        deriv += glm::dot(dFdAA, glm::cross(axis, (AA - axisA)));
    return deriv; 
}

float betaGradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                      const glm::vec3 &H, const glm::vec3 &A,
                                      const glm::vec3 &AA)
{
    // Φ₃ = cos(H-A-AA), vertex at A
    // H-side: only H moves, A and AA are fixed
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 p = H - A;
    glm::vec3 r = AA - A;
    float p_len = glm::length(p);
    float r_len = glm::length(r);
    float dot_pr = glm::dot(p, r);

    glm::vec3 dFdH  = (r / (p_len * r_len)) - (dot_pr / (p_len*p_len*p_len*r_len)) * p;

    return glm::dot(dFdH, glm::cross(axis, (H - axisA)));
}

float dihedral1GradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &C, const glm::vec3 &D,
                                           const glm::vec3 &H, const glm::vec3 &A, bool isDHBond)
{
    // τ(C,D,H,A): H-side, direct bond assumption → only H moves
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 b1 = D - C;
    glm::vec3 b2 = H - D;
    glm::vec3 b3 = A - H;
    glm::vec3 n1 = glm::cross(b1, b2);
    glm::vec3 n2 = glm::cross(b2, b3);
    float n1_norm = glm::dot(n1, n1);
    float n2_norm = glm::dot(n2, n2);
    float b2_norm = glm::length(b2);
    glm::vec3 dTau_dC = -(b2_norm / n1_norm) * n1;
    glm::vec3 dTau_dA =  (b2_norm / n2_norm) * n2;
    glm::vec3 dTau_dD = ((glm::dot(b1,b2)/n1_norm)*n1) - ((glm::dot(b3,b2)/n2_norm)*n2);
    glm::vec3 dTau_dH = -dTau_dC - dTau_dD - dTau_dA;

    // H always moves
    float deriv = glm::dot(dTau_dH, glm::cross(axis, (H - axisA)));
    // D and C move only if torsion is above D-H bond
    if (!isDHBond)
    {
        deriv += glm::dot(dTau_dD, glm::cross(axis, (D - axisA)));
        deriv += glm::dot(dTau_dC, glm::cross(axis, (C - axisA)));
    }
    return deriv;
}

float dihedral1GradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &C, const glm::vec3 &D,
                                           const glm::vec3 &H, const glm::vec3 &A)
{
    // τ(C,D,H,A): A-side, only A moves
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 b1 = D - C;
    glm::vec3 b2 = H - D;
    glm::vec3 b3 = A - H;
    glm::vec3 n1 = glm::cross(b1, b2);
    glm::vec3 n2 = glm::cross(b2, b3);
    float n1_norm = glm::dot(n1, n1);
    float n2_norm = glm::dot(n2, n2);
    float b2_norm = glm::length(b2);
    glm::vec3 dTau_dA = (b2_norm / n2_norm) * n2;

    // only A moves
    return glm::dot(dTau_dA, glm::cross(axis, (A - axisA)));
}

float dihedral2GradientHSide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &D, const glm::vec3 &H,
                                           const glm::vec3 &A, const glm::vec3 &AA)
{
    // τ(D,H,A,AA): H-side, direct bond assumption → only H moves
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 b1 = H - D;
    glm::vec3 b2 = A - H;
    glm::vec3 b3 = AA - A;
    glm::vec3 n1 = glm::cross(b1, b2);
    glm::vec3 n2 = glm::cross(b2, b3);
    float n1_norm = glm::dot(n1, n1);
    float n2_norm = glm::dot(n2, n2);
    float b2_norm = glm::length(b2);
    glm::vec3 dTau_dD  = -(b2_norm / n1_norm) * n1;
    glm::vec3 dTau_dAA =  (b2_norm / n2_norm) * n2;
    glm::vec3 dTau_dH  = ((glm::dot(b1,b2)/n1_norm)*n1) - ((glm::dot(b3,b2)/n2_norm)*n2);
    glm::vec3 dTau_dA  = -dTau_dD - dTau_dH - dTau_dAA;

    // only H moves
    return glm::dot(dTau_dH, glm::cross(axis, (H - axisA)));
}

float dihedral2GradientASide(const glm::vec3 &axisA, const glm::vec3 &axisB,
                                           const glm::vec3 &D, const glm::vec3 &H,
                                           const glm::vec3 &A, const glm::vec3 &AA, bool isAABond)
{
    // tau(D,H,A,AA): A-side, direct bond assumption → only A moves
    glm::vec3 axis = glm::normalize(axisB - axisA);
    glm::vec3 b1 = H - D;
    glm::vec3 b2 = A - H;
    glm::vec3 b3 = AA - A;
    glm::vec3 n1 = glm::cross(b1, b2);
    glm::vec3 n2 = glm::cross(b2, b3);
    float n1_norm = glm::dot(n1, n1);
    float n2_norm = glm::dot(n2, n2);
    float b2_norm = glm::length(b2);
    glm::vec3 dTau_dD  = -(b2_norm / n1_norm) * n1;
    glm::vec3 dTau_dAA =  (b2_norm / n2_norm) * n2;
    glm::vec3 dTau_dH  = ((glm::dot(b1,b2)/n1_norm)*n1) - ((glm::dot(b3,b2)/n2_norm)*n2);
    glm::vec3 dTau_dA  = -dTau_dD - dTau_dH - dTau_dAA;

    // A always moves
    float deriv = glm::dot(dTau_dA, glm::cross(axis, (A - axisA)));
    // AA moves only if torsion is above A-AA bond
    if (!isAABond)
        deriv += glm::dot(dTau_dAA, glm::cross(axis, (AA - axisA)));
    return deriv;

}

float distanceGradientTranslation(const glm::vec3 &direction, 
                                  const glm::vec3 &H, const glm::vec3 &A, 
                                  bool isHSide)
{
    glm::vec3 bond = glm::normalize(H - A); //  dF/dH = bond, dF/dA = -bond
    return isHSide ? glm::dot(bond, direction) : glm::dot(-bond, direction);
}

float alphaGradientTranslation(const glm::vec3 &direction, 
                               const glm::vec3 &D, const glm::vec3 &H, 
                               const glm::vec3 &A, bool isHSide)
{
    glm::vec3 u = D - H;
    glm::vec3 w = A - H;
    float u_len = glm::length(u);
    float w_len = glm::length(w);
    float dot_uw = glm::dot(u, w);

    glm::vec3 dFdD = (w / (u_len * w_len)) - (dot_uw / (u_len*u_len*u_len*w_len)) * u;
    glm::vec3 dFdA = (u / (u_len * w_len)) - (dot_uw / (u_len*w_len*w_len*w_len)) * w;
    glm::vec3 dFdH = -(dFdD + dFdA);

    return isHSide ? glm::dot(dFdD + dFdH, direction) : glm::dot(dFdA, direction);
}

float betaGradientTranslation(const glm::vec3 &direction, 
                              const glm::vec3 &H, const glm::vec3 &A, 
                              const glm::vec3 &AA, bool isHSide)
{
    glm::vec3 p = H - A;
    glm::vec3 r = AA - A;
    float p_len = glm::length(p);
    float r_len = glm::length(r);
    float dot_pr = glm::dot(p,r);

    glm::vec3 dFdH = (r / (p_len * r_len)) - (dot_pr / (p_len*p_len*p_len*r_len)) * p;
    glm::vec3 dFdAA = (p / (p_len * r_len)) - (dot_pr / (p_len*r_len*r_len*r_len)) * r;
    glm::vec3 dFdA = -(dFdH + dFdAA);

    return isHSide ? glm::dot(dFdH, direction) : glm::dot(dFdA + dFdAA, direction);
}

float dihedral1GradientTranslation(const glm::vec3 &direction, 
                              const glm::vec3 &DD, const glm::vec3 &D, 
                              const glm::vec3 &H, const glm::vec3 &A,
                              bool isHSide)
{
   glm::vec3 b1 = D - DD; 
    glm::vec3 b2 = H - D;
    glm::vec3 b3 = A - H;
    glm::vec3 n1 = glm::cross(b1, b2);
    glm::vec3 n2 = glm::cross(b2, b3);
    float n1_norm = glm::dot(n1, n1);
    float n2_norm = glm::dot(n2, n2);
    float b2_norm = glm::length(b2);

    glm::vec3 dTau_dDD = -(b2_norm / n1_norm) * n1;
    glm::vec3 dTau_dA =  (b2_norm / n2_norm) * n2;
    glm::vec3 dTau_dD = ((glm::dot(b1,b2)/n1_norm)*n1) - ((glm::dot(b3,b2)/n2_norm)*n2);
    glm::vec3 dTau_dH = -dTau_dDD - dTau_dD - dTau_dA;

    // if HsideL DD, D, H move together, if Aside A onlu moves
    return isHSide ? glm::dot(dTau_dDD + dTau_dD + dTau_dH, direction) : glm::dot(dTau_dA, direction); 
}

float dihedral2GradientTranslation(const glm::vec3 &direction, 
                              const glm::vec3 &D, const glm::vec3 &H, 
                              const glm::vec3 &A, const glm::vec3 &AA, 
                              bool isHSide)
{
    glm::vec3 b1 = H - D; 
    glm::vec3 b2 = A - H;
    glm::vec3 b3 = AA - A;
    glm::vec3 n1 = glm::cross(b1, b2);
    glm::vec3 n2 = glm::cross(b2, b3);
    float n1_norm = glm::dot(n1, n1);
    float n2_norm = glm::dot(n2, n2);
    float b2_norm = glm::length(b2);

    glm::vec3 dTau_dD = -(b2_norm / n1_norm) * n1;
    glm::vec3 dTau_dAA = (b2_norm / n2_norm) * n2;
    glm::vec3 dTau_dH = ((glm::dot(b1,b2)/n1_norm)*n1) - ((glm::dot(b3,b2)/n2_norm)*n2);
    glm::vec3 dTau_dA = -dTau_dD - dTau_dH - dTau_dAA;

    // if HsideL C, D, H move together, if Aside A onlu moves
    return isHSide ? glm::dot(dTau_dD + dTau_dH, direction) : glm::dot(dTau_dA + dTau_dAA, direction); 
}












