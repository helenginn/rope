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

/** returns the gradient of the distance of length CD when C is rotated
 *  by [angle] around bond AB */
float bond_rotation_on_distance_gradient(const glm::vec3 &a, const glm::vec3 &b,
                                         const glm::vec3 &c, const glm::vec3 &d);

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
