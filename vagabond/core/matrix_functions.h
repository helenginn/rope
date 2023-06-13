#ifndef __matrix__functions__
#define __matrix__functions__

#include "../utils/glm_import.h"

using std::isfinite;
glm::mat3x3 mat3x3_rhbasis(glm::vec3 a, glm::vec3 b);

template <class T>
inline bool is_glm_vec_sane(T vec)
{
	for (size_t i = 0; i < vec.length(); i++)
	{
		if (vec[i] != vec[i] || !isfinite(vec[i]))
		{
			return false;
		}
	}
	
	return true;
}

float mat3x3_volume(const glm::mat3x3 &mat);

/** returns the gradient of the distance of length CD when C is rotated
 *  by [angle] around bond AB */
float bond_rotation_on_distance_gradient(const glm::vec3 &a, const glm::vec3 &b,
                                         const glm::vec3 &c, const glm::vec3 &d);

glm::mat3x3 unit_vec_rotation(glm::vec3 axis, double radians);

/** Rotate vector (vec1) around axis (axis) by angle theta. Find value of
* theta for which the angle between (vec1) and (vec2) is minimised. */
glm::mat3x3 closest_rot_mat(glm::vec3 vec1, glm::vec3 vec2, 
                                   glm::vec3 axis, float *best, bool unity);

double measure_bond_torsion(glm::vec3 positions[4]);

/** matrix from three bond lengths and three bond angles, aligned with
 *  a-vector along canonical bond direction (0., 0., 1)
 * @param a unit length
 * @param b unit length
 * @param c unit length
 * @param alpha in degrees
 * @param beta in degrees
 * @param gamma in degrees
 * @returns matrix where vectors point in directions obeying supplied restraints
 * */
glm::mat3x3 bond_aligned_matrix(double a, double b, double c, 
                                double alpha, double beta, double gamma);

/** matrix from three bond lengths and three bond angles
 * @param a unit length
 * @param b unit length
 * @param c unit length
 * @param alpha in degrees
 * @param beta in degrees
 * @param gamma in degrees
 * @returns matrix where vectors point in directions obeying supplied restraints
 * */
glm::mat3x3 mat3x3_from_unit_cell(double a, double b, double c, 
                                  double alpha, double beta, double gamma);

/** unit cell dimensions from supplied matrix */
void unit_cell_from_mat3x3(const glm::mat3x3 &mat, double *uc_ptr);

/** updates torsion basis matrix based on real space atom positions
 * @param target matrix to update
 * @param self current atom's position
 * @param prev previous atom's position
 * @param the child atom's position, whose matrix needs updating
 */
void torsion_basis(glm::mat4x4 &target, const glm::vec4 &self, 
                   const glm::vec3 &prev, const glm::vec4 &next);

/** inserts four coordinated atoms using lengths and angles relating positions.
 * @param ret matrix to insert results into
 * @param lengths four lengths: central atom (O) to A, B, C, D in order
 * @param angles five angles: A-O-B, B-O-C, C-O-A, B-O-D, A-O-D in order
 */
void insert_four_atoms(glm::mat4x4 &ret, float *lengths, float *angles);

/** inserts three coordinated atoms using lengths and angles relating positions.
 * @param ret matrix to insert results into
 * @param lengths three lengths: central atom (O) to A, B, C in order
 * @param angles three angles: A-O-B, B-O-C and C-O-A in order
 */
void insert_three_atoms(glm::mat4x4 &ret, float *lengths, float *angles);
void insert_two_atoms(glm::mat4x4 &ret, float *lengths, float angle);
void insert_one_atom(glm::mat4x4 &ret, float length);

#endif

