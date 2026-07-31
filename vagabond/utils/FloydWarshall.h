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

#ifndef __vagabond__FloydWarshall__
#define __vagabond__FloydWarshall__

#include <functional>
#include <mutex>
#include "Eigen/Dense"

class FloydWarshall
{
public:
	typedef std::function<float(float, float)> CombineWeight;

	FloydWarshall(Eigen::MatrixXf &sqMat, const CombineWeight &cw,
	              bool maximise = false);
	
	typedef std::function<void()> VoidFunction;
	
	void addDisplayMatrix(Eigen::MatrixXf &mat, std::mutex &mutex,
	                      const VoidFunction &update)
	{
		_mat = &mat;
		_mutex = &mutex;
		_update = update;
	}
	
	void addJobAfterDone(const VoidFunction &done)
	{
		_done = done;
	}

	// called once per outer-loop (k) iteration - i.e. _sqMat.rows() times
	// in total - so a caller can drive a progress bar without run() itself
	// needing to know anything about GUI/Progressor machinery.
	void addTickJob(const VoidFunction &tick)
	{
		_tick = tick;
	}

	void run();
private:
	Eigen::MatrixXf &_sqMat;
	CombineWeight _combineWeight{};
	bool _maximise{};

	Eigen::MatrixXf *_mat{};
	std::mutex *_mutex{};
	VoidFunction _update{};
	VoidFunction _done{};
	VoidFunction _tick{};
};

#endif
