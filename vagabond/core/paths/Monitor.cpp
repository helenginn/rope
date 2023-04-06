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

#include "Monitor.h"
#include <iostream>

Monitor::Monitor(PathFinder *pf, std::vector<Instance *> list)
{
	_pf = pf;
	_instances = list;
	setupMatrix(&_matrix, _instances.size(), _instances.size());
	setup();
}

void Monitor::setup()
{
	for (Instance *i : _instances)
	{
		for (Instance *j : _instances)
		{
			if (i == j)
			{
				continue;
			}
			
			_results[i][j] = RouteResults{};
		}
	}

}

PCA::Matrix &Monitor::matrix()
{
	for (int i = 0; i < _instances.size(); i++)
	{
		for (int j = 0; j < _instances.size(); j++)
		{
			if (i == j)
			{
				continue;
			}
			
			Instance *inst = _instances[i];
			Instance *jnst = _instances[j];
			
			RouteResults &rr = _results[inst][jnst];
			if (rr.valid)
			{
				float val = 1 - rr.linearity;
				_matrix[i][j] = val;
			}
			else if (rr.passes == 0)
			{
				_matrix[i][j] = 2;
			}
			else
			{
				_matrix[i][j] = NAN;
			}
		}
	}
	
	return _matrix;
}

void Monitor::addValidation(Instance *first, Instance *second, 
                            bool valid, float linearity)
{
	_results[first][second].passes++;
	_results[first][second].valid = valid;
	_results[first][second].linearity = linearity;
	
	matrix();
	clusterInBackground();
}

void Monitor::clusterInBackground()
{
	if (_worker && _running)
	{
		return;
	}
	else if (_worker && !_running)
	{
		_worker->join();
		delete _worker;
		_worker = nullptr;
	}

	return;
	_running = true;
	_worker = new std::thread(Monitor::cluster, this);
}

void Monitor::cluster(Monitor *m)
{
	PCA::Matrix *last = nullptr;
	if (m->_lastResult.rows > 0)
	{
		last = &m->_lastResult;
	}

	ClusterTSNE *tsne = new ClusterTSNE(m->_matrix, last);
	tsne->cluster();
	m->_lastResult = tsne->result();
	tsne->normaliseResults(6);

	m->_running = false;
	
	m->_mutex.lock();
	m->_tsne = tsne;
	m->_mutex.unlock();
}

ClusterTSNE *Monitor::availableTSNE()
{
	if (!_mutex.try_lock())
	{
		return nullptr;
	}

	if (!_tsne)
	{
		_mutex.unlock();
		return nullptr;
	}

	ClusterTSNE *tsne = _tsne;
	_tsne = nullptr;
	_mutex.unlock();

	return tsne;
}

void Monitor::updatePath(Instance *first, Instance *second, Path &path)
{
	_paths.push_back(path);
	RouteResults &rr = _results[first][second];
	rr.path = &_paths.back();
}

Path *Monitor::existingPath(Instance *first, Instance *second)
{
	RouteResults &rr = _results[first][second];
	return rr.path;
}
