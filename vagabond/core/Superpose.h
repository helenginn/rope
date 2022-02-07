// vagabond
// Copyright (C) 2019 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __vagabond__Superpose__
#define __vagabond__Superpose__

#include <vector>
#include "../utils/glm_import.h"
#include "../utils/svd/PCA.h"

class Superpose
{
public:
	Superpose();
	
	void forceSameHand(bool hand)
	{
		_sameHand = hand;
	}
	
	struct PosPair
	{
		glm::vec3 p;
		glm::vec3 q;
		
		glm::vec3 &operator[](int i)
		{
			if (i == 0) return p;
			if (i == 1) return q;
			throw std::runtime_error("Invalid index called of PosPair");
		}
	};

	void addPositionPair(glm::vec3 &p, glm::vec3 &q);
	void addPositionPairs(std::vector<PosPair> &pairs);

	void superpose();
	
	/** returns best mat4x4 matrix to map q vector list onto p */
	const glm::mat4x4 &transformation()
	{
		return _transformation;
	}
	
	glm::mat3x3 rotation()
	{
		return _rotation;
	}
	
	glm::vec3 translation()
	{
		return _translation;
	}
private:
	void subtractPositions(const glm::vec3 &pm, const glm::vec3 &qm);
	void getAveragePositions(glm::vec3 &pm, glm::vec3 &qm);
	void populateSVD(PCA::SVD &svd);
	glm::mat3x3 getRotation(PCA::SVD &svd);
	void createTransformation(glm::vec3 &subtract, glm::mat3x3 &rot, 
	                          glm::vec3 &add);

	std::vector<PosPair> _pairs;
	bool _sameHand = false;

	glm::mat4x4 _transformation;
	glm::mat3x3 _rotation;
	glm::vec3 _translation;
};

#endif
