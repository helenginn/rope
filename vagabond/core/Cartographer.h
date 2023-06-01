// vagabond
// Copyright (C) 2022 Helen Ginn
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

#ifndef __vagabond__Cartographer__
#define __vagabond__Cartographer__

#include <vector>
#include <map>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/core/Responder.h>

class Atom;
class Entity;
class Instance;
class Network;
class BondCalculator;
class SpecificNetwork;
class MappingToMatrix;
template <typename Type> class Mapped;

class Cartographer : public Responder<SpecificNetwork>,
public HasResponder<Responder<Cartographer>>
{
public:
	Cartographer(Entity *entity, std::vector<Instance *> instances);
	
	SpecificNetwork *specified()
	{
		return _specified;
	}
	
	PCA::Matrix &matrix();
	
	Mapped<float> *mapped()
	{
		return _mapped;
	}
	
	MappingToMatrix *map2Matrix()
	{
		return _mat2Map;
	}

	void makeMapping();
	void setup();

	void scoreForTriangle(int idx);
	void checkTriangles();
	static void run(Cartographer *cg);
protected:
	virtual void sendObject(std::string tag, void *object);
private:
	void assessSplits(int face_idx);
	void preparePoints(int idx);

	SpecificNetwork *_specified = nullptr;
	Network *_network = nullptr;
	std::vector<Instance *> _instances;
	Entity *_entity = nullptr;
	Mapped<float> *_mapped = nullptr;
	MappingToMatrix *_mat2Map = nullptr;
	PCA::Matrix _distMat{};
	std::mutex _mDist;
	
	typedef std::vector<std::vector<float>> Points;
	
	struct FaceBits
	{
		struct BondCalcInt
		{
			BondCalculator *calculator;
			int idx;
		};

		Points points;
		std::vector<BondCalcInt> problems;
		float score;
	};
	
	std::map<int, FaceBits> _faceBits;
	
	std::vector<Atom *> _atoms;
};

#endif
