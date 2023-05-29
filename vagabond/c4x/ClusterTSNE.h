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

#ifndef __vagabond__ClusterTSNE__
#define __vagabond__ClusterTSNE__

#include "Cluster.h"

class ClusterTSNE 
{
public:
	ClusterTSNE(const PCA::Matrix &distances, const PCA::Matrix *start,
	            int dims = 3);
	~ClusterTSNE();

	void cluster();
	
	size_t pointCount() const
	{
		return _result.rows;
	}
	
	bool givesTorsionAngles()
	{
		return false;
	}
	
	bool canMapVectors()
	{
		return false;
	}
	
	glm::vec3 pointForDisplay(int i);
	
	std::vector<float> point(int i);

	std::vector<float> mapAnglesToVector(std::vector<Angular> 
	                                             &angles)
	{
		return std::vector<float>();
	}

	void chooseBestAxes(std::vector<float> &vals)
	{
		return;
	}
	
	void setDisplayableDimensions(int dims)
	{
		_dims = dims;
	}

	size_t displayableDimensions()
	{
		return _dims;
	}
	
	PCA::Matrix result();
	void normaliseResults(float scale = 1);
private:
	PCA::Matrix probabilityMatrix(int i, float sigma);
	PCA::Matrix probabilityMatrix(PCA::Matrix &sigmas);
	float findSigma(int row, float perp);
	float incrementResult(float &scale, float &learning);
	
	/* returns row=1 col=n vector of sigmas */
	PCA::Matrix findSigmas(float target);
	float startingSigma();
	float perplexity(PCA::Matrix &m);

	PCA::Matrix gradients(PCA::Matrix &prob);
	float qDistanceValue(int i, int j);
	PCA::Matrix qMatrix();

	void prepareResults(float s);
	float averagePQDiff();

	PCA::Matrix _distances{};
	PCA::Matrix _ps{};
	PCA::Matrix _lastResult{};
	PCA::Matrix _result{};
	PCA::Matrix _tmp{};
	
	int _dims = 3;
};

#endif
