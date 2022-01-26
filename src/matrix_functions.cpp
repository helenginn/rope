#ifndef __matrix__functions__
#define __matrix__functions__

#include "matrix_functions.h"
#include <iostream>

using namespace glm;

mat3x3 mat3x3_rhbasis(vec3 a, vec3 b)
{
	normalize(a);
	normalize(b);
	vec3 c = cross(a, b);
	normalize(c);
	c *= -1;

	mat3x3 mat;

	mat[0] = a;
	mat[1] = c;
	mat[2] = b;

	return mat;
}

void fix_unit_cell_angles(double &alpha, double &beta, double &gamma)
{
	double sum = alpha + beta + gamma;
	if (sum >= 360)
	{
		double minus = sum - 360;
		alpha -= minus / 3;
		beta -= minus / 3;
		gamma -= minus / 3;
	}
}

glm::mat3x3 bond_aligned_matrix(double a, double b, double c, 
                                double alpha, double beta, double gamma)
{
	fix_unit_cell_angles(alpha, beta, gamma);

	double cosA = cos(deg2rad(alpha));
	double cosB = cos(deg2rad(beta));
	double cosC = cos(deg2rad(gamma));

	double sinC = sin(deg2rad(gamma));
	
	double vol_bit = 1 - cosA * cosA - cosB * cosB - cosC * cosC;
	vol_bit += 2 * cosA * cosB * cosC;
	double volume = a * b * c * sqrt(vol_bit);

	mat3x3 mat;
	mat[0][0] = 0;
	mat[0][1] = 0;
	mat[0][2] = a;
	mat[1][0] = 0;
	mat[1][1] = -sinC * b;
	mat[1][2] = cosC * b;
	mat[2][0] = volume / (a * b * sinC);
	mat[2][1] = -c * (cosA - cosB * cosC) / sinC;
	mat[2][2] = cosB * c;

	return mat;
}

mat3x3 mat3x3_from_unit_cell(double a, double b, double c, 
                             double alpha, double beta, double gamma)
{
	fix_unit_cell_angles(alpha, beta, gamma);

	double cosA = cos(deg2rad(alpha));
	double cosB = cos(deg2rad(beta));
	double cosC = cos(deg2rad(gamma));

	double sinC = sin(deg2rad(gamma));
	
	double vol_bit = 1 - cosA * cosA - cosB * cosB - cosC * cosC;
	vol_bit += 2 * cosA * cosB * cosC;
	double volume = a * b * c * sqrt(vol_bit);

	mat3x3 mat;
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

mat4x4 torsion_basis(mat4x4 prior, vec3 prev, vec4 next)
{
	/* previous bond direction is the old Z direction. This will be in
	 * the same plane as the new X direction. */

	/* previous bond basis placement is equal to the current position of self */
	vec3 self = vec3(prior[3]);
	vec3 prevdir = self - prev;
	prevdir = normalize(prevdir);
	
	/* current bond direction will become the new Z direction */
	vec3 curr = vec3(next) - self;
	curr = normalize(curr);

	/* cross and normalise to get the new Y direction */
	vec3 y_dir = cross(curr, prevdir);
	y_dir = normalize(y_dir);

	/* "prev" won't be at a right-angle, so we need to re-make it.
	 * it comes already normalised. */
	vec3 x_dir = cross(curr, y_dir);
	x_dir *= -1;

	/* construct final matrix */
	mat4x4 result;
	result[0] = vec4(x_dir, 0);
	result[1] = vec4(y_dir, 0);
	result[2] = vec4(curr, 0);
	
	/* translation to next position */
	result[3] = next;
	result[3][3] = 1;

	return result;
}

void insert_four_atoms(mat4x4 &ret, float *lengths, float *angles)
{

}

void insert_three_atoms(mat4x4 &ret, float *lengths, float *angles)
{

}

void insert_two_atoms(mat4x4 &ret, float *lengths, float angle)
{

}

void insert_one_atom(mat4x4 &ret, float length)
{

}

#endif


