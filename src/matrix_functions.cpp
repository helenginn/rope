#ifndef __matrix__functions__
#define __matrix__functions__

#include "matrix_functions.h"
#include <iostream>

glm::mat3x3 mat3x3_rhbasis(glm::vec3 a, glm::vec3 b)
{
	glm::normalize(a);
	glm::normalize(b);
	glm::vec3 c = cross(a, b);
	glm::normalize(c);
	c *= -1;

	glm::mat3x3 mat;

	mat[0] = a;
	mat[1] = c;
	mat[2] = b;

	return mat;
}

glm::mat3x3 mat3x3_from_unit_cell(double a, double b, double c, 
                             double alpha, double beta, double gamma)
{
	double sum = alpha + beta + gamma;
	if (sum >= 360)
	{
		double minus = sum - 360;
		alpha -= minus / 3;
		beta -= minus / 3;
		gamma -= minus / 3;
	}

	double cosA = cos(deg2rad(alpha));
	double cosB = cos(deg2rad(beta));
	double cosC = cos(deg2rad(gamma));

	double sinC = sin(deg2rad(gamma));
	
	double vol_bit = 1 - cosA * cosA - cosB * cosB - cosC * cosC;
	vol_bit += 2 * cosA * cosB * cosC;
	double volume = a * b * c * sqrt(vol_bit);

	glm::mat3x3 mat;
	mat[0][0] = a;
	mat[0][1] = 0;
	mat[0][2] = 0;
	mat[1][0] = cosC * b;
	mat[1][1] = sinC * b;
	mat[1][2] = 0;
	mat[2][0] = cosB * c;
	mat[2][1] = c * (cosA - cosB * cosC) / sinC;
	mat[2][2] = volume / (a * b * sinC);

	return mat;
}

#endif
