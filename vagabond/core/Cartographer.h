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
#include <atomic>
#include <map>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/core/Responder.h>
#include "ScoreMap.h"

class Atom;
class Entity;
class Instance;
class Network;
class Parameter;
class ProblemPrep;
class SpecificNetwork;
class MappingToMatrix;
template <typename Type> class Mapped;
template <typename Type> class Mappable;

class Cartographer : public Responder<SpecificNetwork>,
public HasResponder<Responder<Cartographer>>
{
public:
	typedef std::vector<std::vector<float>> Points;

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
	
	void stopASAP()
	{
		_stop = true;
	}
	
	void skipCurrentJob()
	{
		_skip = true;
	}

	void makeMapping();
	void setup();

	static void flip(Cartographer *cg);
	static void assess(Cartographer *cg);
private:
	void checkTriangles(ScoreMap::Mode mode = ScoreMap::Basic);
	void bootstrapToTriangle();
	int bestStartingPoint(std::vector<int> &ruled_out);

	float scoreForTriangle(int idx, ScoreMap::Mode mode = ScoreMap::Basic);
	void preparePoints(int idx);
	Mappable<float> *bootstrapFace(std::vector<int> &pidxs);
	
	float scoreForPoints(const Cartographer::Points &points, 
	                     ScoreMap::Mode options);
	float scoreWithScorer(const Points &points, ScoreMap scorer);
	ScoreMap basicScorer(ScoreMap::Mode options);

	void flipPoints();
	void flipPoint(int pidx);
	void flipPointsFor(Mappable<float> *face, const std::vector<int> &points);
	void flipGroup(Mappable<float> *face, int g, int pidx);
	void permute(std::vector<Parameter *> &params, 
	             std::function<float()> score, int pidx);

	std::vector<std::pair<Parameter *, Mapped<float> *>> torsionMapsFor(int pidx);
	Points cartesiansForFace(Mappable<float> *face, int paramCount);
	Mappable<float> *extendFace(std::vector<int> &pidxs, int &tidx);

	void setPoints(std::vector<Mapped<float> *> &maps,
	               const std::vector<float> &values, int pidx);
	void getPoints(std::vector<Mapped<float> *> &maps,
	               std::vector<float> &values, int pidx);

	SpecificNetwork *_specified = nullptr;
	Network *_network = nullptr;
	std::vector<Instance *> _instances;
	Entity *_entity = nullptr;
	Mapped<float> *_mapped = nullptr;
	MappingToMatrix *_mat2Map = nullptr;
	PCA::Matrix _distMat{};
	
	struct PointsInTriangle
	{
		Points points;
	};
	
	std::map<int, PointsInTriangle> _pointsInTriangles;
	ProblemPrep *_prepwork = nullptr;
	
	std::vector<Atom *> _atoms;
	int _received = 0;
	std::atomic<bool> _stop{false};
	std::atomic<bool> _skip{false};
};

#endif
