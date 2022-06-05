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

#ifndef __vagabond__ClusterTSNE__cpp__
#define __vagabond__ClusterTSNE__cpp__

#include "ClusterTSNE.h"
#include "ClusterSVD.h"

template <class DG>
ClusterTSNE<DG>::ClusterTSNE(DG &dg) : Cluster<DG>(dg)
{
	ClusterSVD<DG> svd(dg);
	svd.cluster();
	_distances = svd.distanceMatrix();
	
	prepareResults(1);

	for (size_t i = 0; i < this->_result.rows; i++)
	{
		for (size_t j = 0; j < this->_result.cols && j < 3; j++)
		{
			glm::vec3 v = svd.point(i);
			this->_result[i][j] = v[j];
		}
	}
	
	this->normaliseResults(100);

	copyMatrix(_lastResult, this->_result);
}

template <class DG>
ClusterTSNE<DG>::~ClusterTSNE()
{

}

void normalise_row(PCA::Matrix &m, int row)
{
	double add = 0;
	for (size_t j = 0; j < m.cols; j++)
	{
		add += m[row][j];
	}

	for (size_t j = 0; j < m.cols; j++)
	{
		m[row][j] /= add;

		if (m[row][j] != m[row][j] || !isfinite(m[row][j]))
		{
			m[row][j] = 0;
		}
	}
}

void normalise_matrix(PCA::Matrix &m)
{
	double add = 0;
	for (size_t j = 0; j < m.cols * m.rows; j++)
	{
		add += m.vals[j];
	}

	for (size_t j = 0; j < m.cols * m.rows; j++)
	{
		m.vals[j] /= add;

		if (m.vals[j] != m.vals[j] || !isfinite(m.vals[j]))
		{
			m.vals[j] = 0;
		}
	}
}

template <class DG>
PCA::Matrix ClusterTSNE<DG>::probabilityMatrix(int i, float s)
{
	PCA::Matrix &dm = _distances;
	PCA::Matrix prob;
	setupMatrix(&prob, 1, dm.cols);

	for (size_t j = 0; j < dm.cols; j++)
	{
		prob[0][j] = 0;

		if (i == j)
		{
			continue;
		}

		double d = dm[i][j];
		if (d != d || !isfinite(d))
		{
			continue;
		}

		double p = exp(-d * d / (2 * s * s));
		prob[0][j] = p;
	}

	normalise_matrix(prob);

	return prob;
}

template <class DG>
PCA::Matrix ClusterTSNE<DG>::probabilityMatrix(PCA::Matrix &sigmas)
{
	PCA::Matrix &dm = _distances;

	PCA::Matrix prob;
	setupMatrix(&prob, dm.rows, dm.cols);
	
	for (size_t i = 0; i < dm.rows; i++)
	{
		for (size_t j = 0; j < dm.cols; j++)
		{
			prob[i][j] = 0;

			if (i == j)
			{
				continue;
			}

			double d = dm[i][j];
			if (d != d || !isfinite(d))
			{
				continue;
			}

			double p = exp(-d * d / (2 * sigmas[0][i] * sigmas[0][i]));

			prob[i][j] = p;
		}

		normalise_row(prob, i);
	}
	
	return prob;
}

template <class DG>
float ClusterTSNE<DG>::perplexity(PCA::Matrix &m)
{
	float entropy = 0;

	for (size_t i = 0; i < m.rows; i++)
	{
		for (size_t j = 0; j < m.cols; j++)
		{
			if (i == j)
			{
				continue;
			}
			
			if (m[i][j] <= 1e-6)
			{
				continue;
			}
			
			float part = m[i][j] * log(m[i][j]) / log(2);
			entropy += part;
		}
	}
	
	double perp = pow(2, -entropy);
	
	return perp;
}

template <class DG>
float ClusterTSNE<DG>::startingSigma()
{
	double sum = 0;
	PCA::Matrix &dm = _distances;

	for (size_t i = 0; i < dm.rows * dm.cols; i++)
	{
		sum += dm.vals[i];
	}

	sum /= (float)(dm.rows * dm.cols);

	return sum / 10;
}


template <class DG>
float ClusterTSNE<DG>::findSigma(int row, float target)
{
	float s = startingSigma();
	float step = s / 2;
	float last_dir = 0;
	float guess = s;
	
	while (step > s / 1000)
	{
		PCA::Matrix prob = probabilityMatrix(row, guess);
		float measured = perplexity(prob);

		float dir = (measured - target > 0 ? -1 : 1);
		
		if (dir * last_dir <= -0.5)
		{
			step /= 2;
		}

		guess += dir * step;

		last_dir = dir;
		freeMatrix(&prob);
	}
	
	return guess;
}

template <class DG>
PCA::Matrix ClusterTSNE<DG>::findSigmas(float target)
{
	PCA::Matrix sigmas;
	setupMatrix(&sigmas, 1, _distances.cols);

	for (size_t i = 0; i < _distances.rows; i++)
	{
		float s = findSigma(i, target);
		sigmas[0][i] = s;
	}

	return sigmas;
}

template <class DG>
float ClusterTSNE<DG>::qDistanceValue(int i, int j)
{
	float distsq = 0;
	for (size_t k = 0; k < displayableDimensions(); k++)
	{
		float comp = this->_result[i][k] - this->_result[j][k];
		distsq += comp * comp;
	}

	float nom = 1 / (1 + distsq);
	return nom;
}

template <class DG>
PCA::Matrix ClusterTSNE<DG>::qMatrix()
{
	PCA::Matrix prob;
	setupMatrix(&prob, _distances.rows, _distances.cols);
	
	if (this->_result.cols != displayableDimensions())
	{
		throw std::runtime_error("Result columns not equal to dimensions");
	}
	
	for (size_t i = 0; i < _distances.rows; i++)
	{
		for (size_t j = 0; j < _distances.cols; j++)
		{
			if (i == j)
			{
				continue;
			}

			float nom = qDistanceValue(i, j);
			prob[i][j] = nom;
		}
		
		normalise_row(prob, i);
	}

	return prob;
}

template <class DG>
void ClusterTSNE<DG>::prepareResults(float s)
{
	setupMatrix(&this->_result, _distances.rows, displayableDimensions());
	setupMatrix(&_lastResult, _distances.rows, displayableDimensions());
	setupMatrix(&_tmp, _distances.rows, displayableDimensions());

	for (size_t i = 0; i < this->_result.rows; i++)
	{
		for (size_t j = 0; j < this->_result.cols; j++)
		{
			this->_result[i][j] = rand() / (double)RAND_MAX - 0.5;
			this->_result[i][j] *= s;
		}
	}

	copyMatrix(_lastResult, this->_result);
}

// eq 5 from paper vandermaaten08a
template <class DG>
PCA::Matrix ClusterTSNE<DG>::gradients(PCA::Matrix &ps)
{
	PCA::Matrix grads;
	setupMatrix(&grads, _distances.rows, displayableDimensions());

	PCA::Matrix qs = qMatrix();

	for (size_t i = 0; i < grads.rows; i++)
	{
		for (size_t k = 0; k < displayableDimensions(); k++)
		{
			grads[i][k] = 0;
		}

		for (size_t j = 0; j < grads.rows; j++)
		{
			float prob_diff = ps[i][j] - qs[i][j];
			float nom = qDistanceValue(i, j);

			for (size_t k = 0; k < displayableDimensions(); k++)
			{
				float point_diff = this->_result[i][k] - this->_result[j][k];
				
				double grad = -4 * point_diff * nom * prob_diff;
				
				grads[i][k] += grad;
			}
		}
	}

	freeMatrix(&qs);
	
	return grads;
}

template <class DG>
float ClusterTSNE<DG>::averagePQDiff()
{
	PCA::Matrix qs = qMatrix();

	float sum = 0;
	float count = 0;
	for (size_t i = 0; i < qs.rows; i++)
	{
		for (size_t j = 0; j < qs.cols; j++)
		{
			float add = fabs(qs[i][j] - _ps[i][j]);
			sum += add;
		}
	}
	
	return sum;
}

template <class DG>
float ClusterTSNE<DG>::incrementResult(float &scale, float &learning)
{
	copyMatrix(_tmp, this->_result);

	PCA::Matrix grads = gradients(_ps);

	for (size_t i = 0; i < this->_result.rows; i++)
	{
		for (size_t j = 0; j < this->_result.cols; j++)
		{
			float current = _tmp[i][j];
			float momentum = scale * (_tmp[i][j] - _lastResult[i][j]);
			float contribution = learning * grads[i][j];

			float update = _tmp[i][j] + contribution + momentum;
			this->_result[i][j] = update;
		}
	}
	
	copyMatrix(_lastResult, _tmp);
	freeMatrix(&grads);
	
	float val = averagePQDiff();
	std::cout << "Target: " << val << std::endl;
	return val;
}


template <class DG>
void ClusterTSNE<DG>::cluster()
{
	float perp = 30;
	
	if (_distances.rows < perp * 2)
	{
		perp = _distances.rows / 2;
	}
	
	PCA::Matrix sigmas = findSigmas(perp);
	_ps = probabilityMatrix(sigmas);
	freeMatrix(&sigmas);
	
	float scale = 0.5;
	float learning = 200;
	std::vector<float> results;
	int count = 0;
	const int period = 20;

	while (true)
	{
		float score = incrementResult(scale, learning);
		results.push_back(score);
		count++;
		
		if (count > 500)
		{
			float good = 0;
			for (int i = -period; i < 0; i++)
			{
				int idx = results.size() + i - 1;
				
				if (results[idx] > results[idx + 1])
				{
					good++;
				}
			}

			if (good < period * 0.5)
			{
				break;
			}
		}
	}
	
	this->normaliseResults(4);

	freeMatrix(&_ps);
}

#endif
