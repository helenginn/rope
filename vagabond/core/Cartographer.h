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
class Parameter;
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

	float scoreForTriangle(int idx, bool wide = false, 
	                       bool identify_torsions = true);
	void checkTriangles(bool identify_torsions = true);
	static void run(Cartographer *cg);
protected:
	virtual void sendObject(std::string tag, void *object);
private:
	void assessSplits(int face_idx);
	void preparePoints(int idx);
	
	typedef std::vector<std::vector<float>> Points;
	float scoreForPoints(const Points &points, bool identify_torsions);

	void flipTriangles();
	void flipTriangle(int idx);
	bool flipTriangleTorsion(Mapped<float> *map, int idx);
	void flipPoint(Mapped<float> *map, int idx, int num_360s);

	SpecificNetwork *_specified = nullptr;
	Network *_network = nullptr;
	std::vector<Instance *> _instances;
	Entity *_entity = nullptr;
	Mapped<float> *_mapped = nullptr;
	MappingToMatrix *_mat2Map = nullptr;
	PCA::Matrix _distMat{};
	std::mutex _mDist;
	
	struct FaceBits
	{
		Points points;
		Points wider;
		std::vector<Parameter *> problems;
	};
	
	std::map<int, FaceBits> _faceBits;
	
	std::vector<Atom *> _atoms;
	int _received = 0;
};

#endif
