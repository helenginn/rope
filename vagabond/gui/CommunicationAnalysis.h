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

#ifndef __vagabond__CommunicationAnalysis__
#define __vagabond__CommunicationAnalysis__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/core/protonic/ProbeResult.h>
#include <vagabond/core/Item.h>

using Eigen::MatrixXf;
using Eigen::VectorXf;

class Clique;
class LineGroup;

class CommunicationAnalysis : public Scene
{
public:
	CommunicationAnalysis(Scene *scene, Clique *clique, const MatrixXf &mat,
	                      const std::map<ProbeTypePair,
	                      std::pair<int, int>> &insertions);
	~CommunicationAnalysis();

	virtual void setup();
private:
	void prepareGroups();
	float compare(const std::string &first, const std::string &second);

	void svd();

	Clique *_clique{};

	MatrixXf _mat;
	MatrixXf _wU;
	VectorXf _w;
	int _overOne{};
	
	MatrixXf _lMat{};
	MatrixXf _rMat{};
	PCA::Matrix _cc;

	// diagnostic counters, reported once as a summary at the end of
	// setup() rather than per-pair - see compare().
	int _emptyPairs{};
	int _ranPairs{};

	std::map<ProbeTypePair, std::pair<int, int>> _lookup;
	
	LineGroup *_lg{};
	Item _parent{};
	OpSet<std::string> _chosen;
};

#endif
