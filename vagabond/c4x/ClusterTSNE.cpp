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

#include "ClusterTSNE.h"
#include "ClusterSVD.h"

#include <iostream>

ClusterTSNE::ClusterTSNE(const PCA::Matrix &distances, const PCA::Matrix *start,
                         int dims)
{
	setDisplayableDimensions(dims);
	_distances = distances;
	
	prepareResults(1);

	for (size_t i = 0; i < this->_result.rows; i++)
	{
		for (size_t j = 0; j < this->_result.cols && j < dims; j++)
		{
			if (start)
			{
				this->_result[i][j] = (*start)[i][j];
			}
			else
			{
				this->_result[i][j] = rand() / (float)RAND_MAX;
			}
		}
	
		this->normaliseResults(6);
	}

	copyMatrix(_lastResult, this->_result);
}

ClusterTSNE::~ClusterTSNE()
{
	freeMatrix(&_result);
	freeMatrix(&_lastResult);
	freeMatrix(&_distances);
	freeMatrix(&_ps);
	freeMatrix(&_tmp);
}

void normalise_row(PCA::Matrix &m, int row)
{
	double add = 0;
	for (size_t j = 0; j < m.cols; j++)
	{
		if (m[row][j] == m[row][j])
		{
			add += m[row][j];
		}
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
		if (m.vals[j] == m.vals[j])
		{
			add += m.vals[j];
		}
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

PCA::Matrix ClusterTSNE::probabilityMatrix(int i, float s)
{
	PCA::Matrix &dm = _distances;
	PCA::Matrix prob;
	setupMatrix(&prob, 1, dm.cols);

	if (s != s)
	{
		return prob;
	}

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

PCA::Matrix ClusterTSNE::probabilityMatrix(PCA::Matrix &sigmas)
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

float ClusterTSNE::perplexity(PCA::Matrix &m)
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
			
			if (m[i][j] <= 1e-6 || m[i][j] != m[i][j])
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

float ClusterTSNE::startingSigma()
{
	double sum = 0;
	double count = 0;
	PCA::Matrix &dm = _distances;

	for (size_t i = 0; i < dm.rows * dm.cols; i++)
	{
		if (dm.vals[i] == dm.vals[i])
		{
			sum += dm.vals[i];
			count++;
		}
	}

	sum /= count;

	return sum / 10;
}


float ClusterTSNE::findSigma(int row, float target)
{
	float s = startingSigma();
	if (s != s)
	{
		return 1;
	}

	float step = s / 2;
	float last_dir = 0;
	float guess = s;
	int count = 0;
	
	while (step > s / 1000 && count < 100)
	{
		PCA::Matrix prob = probabilityMatrix(row, guess);
		float measured = perplexity(prob);
		freeMatrix(&prob);
		
		float dir = (measured - target > 0 ? -1 : 1);
		
		if (dir * last_dir <= -0.5)
		{
			step /= 2;
		}
		
		if (fabs(measured - target) < 0.5)
		{
			break;
		}

		guess += dir * step;

		last_dir = dir;
		count++;
	}
	
	return guess;
}

PCA::Matrix ClusterTSNE::findSigmas(float target)
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

float ClusterTSNE::qDistanceValue(int i, int j)
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

PCA::Matrix ClusterTSNE::qMatrix()
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
			if (nom == nom)
			{
				prob[i][j] = nom;
			}
		}
		
		normalise_row(prob, i);
	}

	return prob;
}

void ClusterTSNE::prepareResults(float s)
{
	setupMatrix(&this->_result, _distances.rows, displayableDimensions());
	setupMatrix(&_lastResult, _distances.rows, displayableDimensions());
	setupMatrix(&_tmp, _distances.rows, displayableDimensions());
}

// eq 5 from paper vandermaaten08a
PCA::Matrix ClusterTSNE::gradients(PCA::Matrix &ps)
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

float ClusterTSNE::averagePQDiff()
{
	PCA::Matrix qs = qMatrix();

	float sum = 0;
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

float ClusterTSNE::incrementResult(float &scale, float &learning)
{
	copyMatrix(_tmp, this->_result);

	PCA::Matrix grads = gradients(_ps);

	for (size_t i = 0; i < this->_result.rows; i++)
	{
		for (size_t j = 0; j < this->_result.cols; j++)
		{
			float momentum = scale * (_tmp[i][j] - _lastResult[i][j]);
			float contribution = learning * grads[i][j];

			float update = _tmp[i][j] + contribution + momentum;
			this->_result[i][j] = update;
		}
	}
	
	copyMatrix(_lastResult, _tmp);
	freeMatrix(&grads);
	
	float val = averagePQDiff();
	return val;
}

void ClusterTSNE::normaliseResults(float scale)
{
	float sum = 0;
	for (size_t i = 0; i < _result.rows; i++)
	{
		for (size_t j = 0; j < displayableDimensions(); j++)
		{
			sum += _result[i][j] * _result[i][j];
		}
	}

	float mag = sqrt(sum);
	
	for (size_t i = 0; i < _result.rows * _result.cols; i++)
	{
		_result.vals[i] /= mag;
		_result.vals[i] *= scale;
	}
}


void ClusterTSNE::cluster()
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
	float learning = 50;
	std::vector<float> results;
	int count = 0;
	const int period = 20;
	
	float first = -1;
	float last = -1;

	while (true)
	{
		float score = incrementResult(scale, learning);
		if (first < 0)
		{
			first = score;
		}

		last = score;
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
	
	std::cout << "Target: from " << first << " to " << last << std::endl;
	
	this->normaliseResults(4);

	freeMatrix(&_ps);
}

std::vector<float> ClusterTSNE::point(int j)
{
	std::vector<float> ret(_dims);

	for (int i = 0; i < _dims; i++)
	{
		ret[i] = _result[j][i];
	}
	
	return ret;
}

glm::vec3 ClusterTSNE::pointForDisplay(int j)
{
	glm::vec3 v{};
	
	for (size_t i = 0; i < 3 && i < _result.cols; i++)
	{
		v[i] = _result[j][i];
	}

	return v;
}

PCA::Matrix ClusterTSNE::result()
{
	PCA::Matrix m;
	setupMatrix(&m, _result.rows, _result.cols);
	copyMatrix(m, _result);
	return m;
}
