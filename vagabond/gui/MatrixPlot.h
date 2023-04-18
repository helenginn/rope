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

#ifndef __vagabond__MatrixPlot__
#define __vagabond__MatrixPlot__

#include <vagabond/utils/svd/PCA.h>
#include <vagabond/gui/elements/Image.h>
#include <mutex>

class MatrixPlot : public Image
{
public:
	MatrixPlot(PCA::Matrix &mat, std::mutex &mutex);

	virtual void update();

	const PCA::Matrix &mat() const
	{
		return _mat;
	}
protected:
	std::map<int, int> _index2Vertex;
private:
	glm::vec4 colourForValue(float val);
	void prepareSmallVertices();
	void updateColours();
	void setup();

	PCA::Matrix &_mat;
	std::mutex &_mutex;

	float _xProp = 1;
	float _yProp = 1;
};

#endif
