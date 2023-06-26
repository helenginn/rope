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

#ifndef __vagabond__MappingToMatrix__
#define __vagabond__MappingToMatrix__

#include <vagabond/utils/svd/PCA.h>
template <typename Type> class Mapped;

class MappingToMatrix
{
public:
	MappingToMatrix(Mapped<float> &mapped);
	
	void fraction_to_cart(std::vector<float> &vals);
	void insertScore(float score, std::vector<float> &point);
	
	std::vector<std::vector<float>> carts_for_triangle(int idx, int steps = -1);

	PCA::Matrix &matrix() 
	{
		return _matrix;
	}

	void calculate();
	void normalise();
private:
	float simpleValue(float x, float y);

	void normalise(double &val);
	void loop(float(MappingToMatrix::*get_value)(float, float));

	Mapped<float> &_mapped;
	PCA::Matrix _matrix{};
	PCA::Matrix _unnorm{};

	std::vector<float> _min, _max;
	std::map<int, double *> _ticket2Mat;
	double _mean = 0;
	double _stdev = 1;
	int _steps = 100;
};

#endif
