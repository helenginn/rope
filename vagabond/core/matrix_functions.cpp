#include "matrix_functions.h"
#include <iostream>

using namespace glm;

mat3x3 mat3x3_rhbasis(vec3 a, vec3 b)
{
	a = normalize(a);
	b = normalize(b);
	vec3 c = cross(a, b);
	c = normalize(c);
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

float mat3x3_volume(const mat3x3 &mat)
{
	double ucs[6];
	unit_cell_from_mat3x3(mat, ucs);

	double cosA = cos(deg2rad(ucs[3]));
	double cosB = cos(deg2rad(ucs[4]));
	double cosC = cos(deg2rad(ucs[5]));

	double sinC = sin(deg2rad(ucs[5]));
	
	double vol_bit = 1 - cosA * cosA - cosB * cosB - cosC * cosC;
	vol_bit += 2 * cosA * cosB * cosC;
	float volume = ucs[0] * ucs[1] * ucs[2] * sqrt(vol_bit);
	return volume;
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
	
	if (mat[2][1] != mat[2][1])
	{
		mat[2][1] = 0;
	}

	return mat;
}

glm::mat3x3 bond_aligned_matrix(double a, double b, double c, 
                                double alpha, double beta, double gamma)
{
	fix_unit_cell_angles(alpha, beta, gamma);
	int mult = 1;

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
	mat[0][2] = -a * mult;
	mat[1][0] = -sinC * b;
	mat[1][1] = 0;
	mat[1][2] = -cosC * b * mult;
	mat[2][0] = -c * (cosA - cosB * cosC) / sinC;
	mat[2][1] = volume / (a * b * sinC);
	mat[2][2] = -cosB * c * mult;

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
	glm::vec3 bond = glm::normalize(b - a);
	
	/* for ease of reading: x,y,z = bond axis direction */
	float &x = bond.x;
	float &y = bond.y;
	float &z = bond.z;
	
	/* p and q are vectors to query atoms c and d */
	glm::vec3 p = c - b;
	glm::vec3 q = d - b;
	
	/* when torsion angle = 0, q.x etc. simplify to q.x */

	float fx = ((q.x - p.x) * (q.x - p.x) + (q.y - p.y) * (q.y - p.y) + 
	             (q.z - p.z) * (q.z - p.z));
	
	/* derivative of q.x with respect to alpha */
	/*
	float dQx_by_dA = y * q.z - z * q.y;
	float dQy_by_dA = z * q.x - x * q.z;
	float dQz_by_dA = x * q.y - y * q.x;
	*/
	float dQx_by_dA = q.z * (y - x * z) - q.y * (x * y + z);
	float dQy_by_dA = q.x * (z - y * x) - q.z * (y * z + x);
	float dQz_by_dA = q.y * (x - z * y) - q.x * (z * x + y);

	/* dfx = derivative of fx */
	float dfx = (2*(q.x - p.x) * dQx_by_dA + 2*(q.y - p.y) * dQy_by_dA +
	             2*(q.z - p.z) * dQz_by_dA);

	float dD_by_dA = -1/sqrt(fx) * dfx / 2;

	return dD_by_dA;
}

mat3x3 unit_vec_rotation(vec3 axis, double radians)
{
	mat3x3 mat{};

	float &x = axis.x;
	float x2 = x * x;

	float &y = axis.y;
	float y2 = y * y;

	float &z = axis.z;
	float z2 = z * z;

	float cosa = cos(radians);
	float sina = sin(radians);
	
	mat[0][0] = cosa + x2 * (1 - cosa);
	mat[1][0] = x * y * (1 - cosa) - z * sina;
	mat[2][0] = x * z * (1 - cosa) + y * sina;

	mat[0][1] = y * x * (1 - cosa) + z * sina;
	mat[1][1] = cosa + y2 * (1 - cosa);
	mat[2][1] = z * y * (1 - cosa) - x * sina;

	mat[0][2] = z * x * (1 - cosa) - y * sina;
	mat[1][2] = z * y * (1 - cosa) + x * sina;
	mat[2][2] = cosa + z2 * (1 - cosa);

	return mat;
}

/* Rotate vector (vec1) around axis (axis) by angle theta. Find value of
* theta for which the angle between (vec1) and (vec2) is minimised. */
mat3x3 closest_rot_mat(vec3 vec1, vec3 vec2, vec3 axis, float *best, bool unity)
{
	/* Let's have unit vectors */
	
	if (!unity)
	{
		vec1 = normalize(vec1);
		vec2 = normalize(vec2);
		axis = normalize(axis);
	}

	/* Redeclaring these to try and maintain readability and
	* check-ability against the maths I wrote down */
	float a = vec2.x; float b = vec2.y; float c = vec2.z;
	float p = vec1.x; float q = vec1.y; float r = vec1.z;
	float x = axis.x; float y = axis.y; float z = axis.z;

	/* Components in handwritten maths online when I upload it */
	float A = a*(p*x*x - p + x*y*q + x*z*r) +
	b*(p*x*y + q*y*y - q + r*y*z) +
	c*(p*x*z + q*y*z + r*z*z - r);

	float B = a*(y*r - z*q) + b*(p*z - r*x) + c*(q*x - p*y);

	float tan_theta = - B / A;
	float theta = atan(tan_theta);
	float &best_angle = theta;
	
	mat3x3 m = unit_vec_rotation(axis, best_angle);
	vec3 rotated = m * vec1;
	rotated -= vec2;
	float rotated_sqdist = dot(rotated, rotated);

	mat3x3 extra = unit_vec_rotation(axis, best_angle + M_PI);
	vec3 extra_rot = extra * vec1;
	extra_rot -= vec2;
	float extra_sqdist = dot(extra_rot, extra_rot);
	
	bool other = (extra_sqdist < rotated_sqdist);
	float addition = (other ? M_PI : 0);
	
	if (best)
	{
		*best = theta + addition;
	}
	
	return (other ? extra : m);
}

void unit_cell_from_mat3x3(const glm::mat3x3 &mat, double *uc_ptr)
{
	for (size_t i = 0; i < 3; i++)
	{
		uc_ptr[i] = glm::length(mat[i]);
	}
	
	for (int i = 0; i < 3; i++)
	{
		int j = (i + 1) % 3;
		int k = (j + 1) % 3;

		float rad = glm::angle(glm::normalize(mat[i]), 
		                       glm::normalize(mat[j]));
		
		float deg = rad2deg(rad);
		uc_ptr[3 + k] = deg;
	}
}



