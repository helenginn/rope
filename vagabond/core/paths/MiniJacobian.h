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

#ifndef __vagabond__MiniJacobian__
#define __vagabond__MiniJacobian__

#include <vagabond/utils/Eigen/Dense>
#include <mutex>
#include <iostream>

struct MiniJacobian
{
	// rows are parameters, columns are constraints
	MiniJacobian(int r, int c) : _matrix(r, c)
	{
		mutex = new std::mutex();
	}

	MiniJacobian()
	{
		mutex = new std::mutex();
	}
	
	Eigen::MatrixXf &matrix()
	{
		return _matrix;
	}
	
	const Eigen::MatrixXf &matrix() const
	{
		return _matrix;
	}
	
	auto &operator()(int r, int c)
	{
		return _matrix(r, c);
	}
	
	MiniJacobian &operator=(const MiniJacobian &other)
	{
		std::unique_lock<std::mutex> lock(*mutex);
		if (_matrix.rows() == 0 || _matrix.cols() == 0)
		{
			_matrix = Eigen::MatrixXf(other.matrix().rows(), 
			                          other.matrix().cols());
			_matrix.setZero();
		}
		_matrix += other.matrix();
		return *this;
	}
	
	void destroy()
	{
		delete mutex;
	}

	int ticket = 0;
	Eigen::MatrixXf _matrix;
	std::mutex *mutex = nullptr;
};



#endif
