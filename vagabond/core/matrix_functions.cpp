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

template <class T>
bool fix_unit_cell_angles(T &alpha, T &beta, T &gamma)
{
	double sum = alpha + beta + gamma;
	if (sum >= 360)
	{
		double minus = sum - 360;
		alpha -= 2 * minus / 3;
		beta -= 2 * minus / 3;
		gamma -= 2 * minus / 3;
		return true;
	}

	return false;
}

void torsion_basis(mat4x4 &target, const vec4 &self, 
                     const vec3 &prev, const vec4 &next)
{
	/* previous bond direction is the old Z direction. This will be in
	 * the same plane as the new X direction. */

	/* previous bond basis placement is equal to the current position of self */
	vec3 prevdir = prev - vec3(self);
	
	/* current bond direction will become the new Z direction */
	vec3 curr = vec3(next - self);
	curr = normalize(curr);

	/* cross and normalise to get the new Y direction */
	vec3 y_dir = cross(curr, prevdir);
	y_dir = normalize(y_dir);

	/* "prev" won't be at a right-angle, so we need to re-make it.
	 * it comes already normalised. */
	vec3 x_dir = cross(curr, y_dir);
	y_dir *= -1;

	/* construct final matrix */
	target[0] = vec4(x_dir, 0);
	target[1] = vec4(y_dir, 0);
	target[2] = vec4(curr, 0);
	
	/* translation to next position */
	target[3] = next;
	target[3][3] = 1;
}

mat3x3 mat3x3_from_unit_cell(double a, double b, double c, 
                             double alpha, double beta, double gamma)
{
	bool flipped = fix_unit_cell_angles(alpha, beta, gamma);

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
	
	if (mat[2][1] != mat[2][1])
	{
		mat[2][1] = 0;
	}

	return mat;
}

glm::mat3x3 bond_aligned_matrix(double a, double b, double c, 
                                double alpha, double beta, double gamma)
{
	bool flipped = fix_unit_cell_angles(alpha, beta, gamma);

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
	mat[0][2] = -a;
	mat[1][0] = -sinC * b;
	mat[1][1] = 0;
	mat[1][2] = -cosC * b;
	mat[2][0] = -c * (cosA - cosB * cosC) / sinC;
	mat[2][1] = volume / (a * b * sinC);
	mat[2][2] = -cosB * c;

	if (mat[2][1] != mat[2][1])
	{
		mat[2][1] = 0;
	}

	return mat;
}

void insert_four_atoms(mat4x4 &ret, float *lengths, float *angles)
{
	ret = bond_aligned_matrix(lengths[0], lengths[1], lengths[2],
	                          angles[1], angles[2], angles[0]);

	mat4x4 tmp = bond_aligned_matrix(lengths[0], lengths[1], lengths[3],
	                                 /* a - b - d */
	                                 angles[3], angles[4], angles[0]);
									 /*b-o-d, a-o-d, b-o-a */
	tmp[2].y *= -1;

	ret[3] = tmp[2];
}

void insert_three_atoms(mat4x4 &ret, float *lengths, float *angles)
{
	bool flipped = fix_unit_cell_angles(angles[0], angles[1], angles[2]);

	ret = bond_aligned_matrix(lengths[0], lengths[1], lengths[2],
	                          angles[1], angles[2], angles[0]);
	
	if (flipped)
	{
		for (size_t i = 0; i < 4; i++)
		{
			ret[i].y *= -1;
		}
	}
}

void insert_two_atoms(mat4x4 &ret, float *lengths, float angle)
{
	ret[0] = vec4(0.);
	ret[1] = vec4(0.);

	/* bond points in negative Z direction */
	ret[0][2] = -lengths[0];

	double ratio = tan(deg2rad(angle) - M_PI / 2);

	ret[1][0] = -lengths[1];
	ret[1][2] = lengths[1] * ratio;
	
	ret[1] = lengths[1] * normalize(ret[1]);
}

void insert_one_atom(mat4x4 &ret, float length)
{
	ret[0] = vec4(0.);
	/* bond points in negative Z direction */
	ret[0][2] = -length;
}

double measure_bond_torsion(glm::vec3 poz[4])
{
	glm::vec3 bond = normalize(poz[2] - poz[1]);
	glm::vec3 prev = normalize(poz[0] - poz[1]);
	glm::vec3 next = normalize(poz[3] - poz[2]);
	
	glm::mat3x3 squish;
	squish[0] = prev;
	squish[2] = bond;
	squish[1] = normalize(glm::cross(prev, bond));
	squish[0] = glm::cross(squish[1], bond);
	
	glm::mat3x3 inv = glm::transpose(squish);

	glm::vec3 p = (inv * prev);
	glm::vec3 n = (inv * next);
	n[2] = 0.;
	p[2] = 0.;
	n = normalize(n);
	p = normalize(p);
	
	double angle = glm::angle(p, n);
	glm::vec3 cr = glm::cross(p, n);

	if (cr.z < 0)
	{
		angle *= -1;
	}

	return rad2deg(angle);
}

float bond_rotation_on_distance_gradient(const glm::vec3 &a, const glm::vec3 &b,
                                         const glm::vec3 &c, const glm::vec3 &d)
{
	/* b = bond axis */
	glm::vec3 bond = b - a;
	
	/* for ease of reading: x,y,z = bond axis direction */
	float &x = bond.x;
	float &y = bond.y;
	float &z = bond.z;
	
	/* p and q are distances to query atoms c and d */
	glm::vec3 p = c - b;
	glm::vec3 q = d - b;
	
	/* when torsion angle = 0, q.x etc. simplify to q.x */

	double fx = ((q.x - p.x) * (q.x - p.x) + (q.y - p.y) * (q.y - p.y) + 
	             (q.z - p.z) * (q.z - p.z));
	
	/* derivative of q.x with respect to alpha */
	double dQx_by_dA = y * q.z - z * q.y;
	double dQy_by_dA = z * q.x - x * q.z;
	double dQz_by_dA = x * q.y - y * q.x;

	/* dfx = derivative of fx */
	double dfx = (2*(q.x - p.x) * dQx_by_dA + 
	              2*(q.y - p.y) * dQy_by_dA +
	              2*(q.z - p.z) * dQz_by_dA);

	double dD_by_dA = -pow(fx, -0.5) * dfx / 2;

	return dD_by_dA;
}

#endif


