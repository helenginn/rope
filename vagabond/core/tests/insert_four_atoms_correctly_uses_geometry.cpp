#include "../matrix_functions.h"
#include <iostream>

int main()
{

	float lengths[4] = {2, 1, 3, 4};
	float angles[5] = {100, 120, 110, 105, 90};
	
	glm::mat4x4 ret = glm::mat4(0.f);
	insert_four_atoms(ret, lengths, angles);
	
	for (size_t i = 0; i < 4; i++)
	{
		float l = glm::length(ret[i]);
		
		if (fabs(lengths[i] - l) > 1e-6)
		{
			std::cout << "Mismatch of length, expected " << lengths[i] << 
			", got " << l << std::endl;
			return 1;
		}
		
		ret[i] = normalize(ret[i]);
	}
	
	// check angles
	
	float compare[5];
	compare[0] = rad2deg(glm::angle(ret[0], ret[1]));
	compare[1] = rad2deg(glm::angle(ret[1], ret[2]));
	compare[2] = rad2deg(glm::angle(ret[2], ret[0]));
	compare[3] = rad2deg(glm::angle(ret[1], ret[3]));
	compare[4] = rad2deg(glm::angle(ret[0], ret[3]));
	
	bool bad = false;
	for (size_t i = 0; i < 5; i++)
	{
		if (fabs(compare[i] - angles[i]) > 1e-1)
		{
			std::cout << "Mismatch of angle " << i << ", expected " << 
			angles[i] << ", got " << compare[i] << std::endl;
			bad = true;
		}
	}

	return bad;
}
