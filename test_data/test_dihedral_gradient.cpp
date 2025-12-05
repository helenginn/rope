#include <iostream>
#include <cmath>
#include <glm/glm.hpp>

// include the header that declares measure_bond_torsion
#include "../vagabond/core/matrix_functions.h"

static float numerical_dphi_dDx(const glm::vec3& a, 
                                const glm::vec3& b,
                                const glm::vec3& p1,
                                const glm::vec3& p2,
                                const glm::vec3& p3,
                                const glm::vec3& p4,
                                float h = 1e-5f)
{
    glm::vec3 p4p = p4 + glm::vec3(h,0,0);
    glm::vec3 p4m = p4 - glm::vec3(h,0,0);
    glm::vec3 frac1[4]{};
    frac1[0] = p1;
    frac1[1] = p2;
    frac1[2] = p3;
    frac1[3] = p4p;

    glm::vec3 frac2[4]{};
    frac2[0] = p1;
    frac2[1] = p2;
    frac2[2] = p3;
    frac2[3] = p4m;


    float phi_plus = measure_bond_torsion(frac1);
    float phi_minus = measure_bond_torsion(frac2);

    return (phi_plus - phi_minus) / (2*h);
}

static float check_gradient(const glm::vec3& a,
                            const glm::vec3& b, 
                            const glm::vec3 pts[4], 
                            const char* name)
{
    float analytic = bond_rotation_on_torsion_gradient(a, b, pts[0], pts[1], pts[2], pts[3]);
    float numeric = numerical_dphi_dDx(a, b, pts[0], pts[1], pts[2], pts[3]);
    double err = std::fabs(analytic - numeric);
    std::cout << name << ": analytic = " << analytic
              << " , numeric = " << numeric
              << " , |err| = " << err << "\n";

    // if (err > 1e-3)
    // {
    //     std::cerr << "[FAIL] " << name << " (gradient mismatch)\n";
    //     std::exit(1);
    // }  
}                                                                                                              

int main()
{
    // ==== Example 1: 90 deg torsion ====
    glm::vec3 axisA(0,0,0);
    glm::vec3 axisB(1,0,0);
    glm::vec3 pts1[4] = {
        {0,0,0},
        {1,0,0},
        {1,1,0},
        {1,1,1}
    };

    check_gradient(axisA, axisB, pts1, "Right-angle gradient");


    return 0;
}