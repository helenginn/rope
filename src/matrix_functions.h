#include "glm_import.h"

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
