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
template <typename Type> class Mappable;

class Cartographer : public Responder<SpecificNetwork>,
public HasResponder<Responder<Cartographer>>
{
public:
	typedef std::vector<std::vector<float>> Points;
	typedef std::pair<Parameter *, Mapped<float> *> ParamMap;
	typedef std::vector<Parameter *> ParameterGroup;
	typedef std::vector<ParameterGroup> ParameterGroups;
	typedef std::vector<ParamMap> ParamMapGroup;
	typedef std::vector<ParamMapGroup> ParamMapGroups;

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
	
	enum ScoreMode
	{
		Simple,
		DisplayMatrix,
		AssessSplits,
		DistanceScore,
	};

	void checkTriangles(ScoreMode mode = Simple);
	void bootstrapToTriangle();
	int bestStartingPoint();
	static void run(Cartographer *cg);
protected:
	virtual void sendObject(std::string tag, void *object);
private:
	float scoreForTriangle(int idx, ScoreMode mode = Simple);
	float scoreForPoints(const Cartographer::Points &points, 
	                     std::function<float(float)> score_func);
	void displayDistances();
	void divideDistances(const size_t &num_points);
	void assessSplits(int face_idx);
	void preparePoints(int idx);
	Mappable<float> *bootstrapFace(std::vector<int> &pidxs);
	
	float scoreForPoints(const Cartographer::Points &points, 
	                     Cartographer::ScoreMode options);

	float processScores(const Cartographer::Points &points, 
	                    bool grab_positions,
	                    std::function<float(float)> process);


	void flipPoints();
	void flipPoint(int pidx);
	void flipPointsFor(Mappable<float> *face, const std::vector<int> &points);
	void permute(std::vector<Mapped<float> *> &maps_only, Points all_points,
	             int pidx);
	void permute(std::vector<Mapped<float> *> &maps, 
	             std::function<float()> score, int pidx);

	float distanceScore(float total);
	std::vector<std::pair<Parameter *, Mapped<float> *>> torsionMapsFor(int pidx);
	Points cartesiansForFace(Mappable<float> *face);
	Mappable<float> *extendFace(std::vector<int> &pidxs, int &tidx);
	std::function<float(float)> limitedDistanceScore();
	ParamMapGroups splitParameters(const ParamMapGroup &list);

	void setPoints(std::vector<Mapped<float> *> &maps,
	               const std::vector<float> &values, int pidx);
	void getPoints(std::vector<Mapped<float> *> &maps,
	               std::vector<float> &values, int pidx);

	std::function<float(float)> score_func(Cartographer::ScoreMode options);

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
