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

#include "MatrixPlot.h"
#include <vagabond/gui/elements/Window.h>
#include <vagabond/utils/maths.h>

MatrixPlot::MatrixPlot(PCA::Matrix &mat, std::mutex &mutex) 
: Image("assets/images/pencil_shading.png"), _mat(mat), _mutex(mutex)
{
	clearVertices();
	setup();
}

glm::vec4 MatrixPlot::colourForValue(float val)
{
	glm::vec4 v{};
	v.w = 1.0;

	if (val == val)
	{
		val_to_cluster4x_colour(val, &v[0], &v[1], &v[2]);

		for (size_t i = 0; i < 3; i++)
		{
			v[i] /= 255.;
		}
	}
	else
	{
		v[0] = 0.5; v[1] = 0.5; v[2] = 0.5;
	}

	return v;
}

void MatrixPlot::update()
{
	checkDimensions();

	std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
	if (lock.try_lock())
	{
		updateColours();
	}
}

void MatrixPlot::updateColours()
{
	for (auto it = _index2Vertex.begin(); it != _index2Vertex.end(); it++)
	{
		float val = _mat.vals[it->first];
		int idx = it->second;
		
		for (size_t i = 0; i < 4; i++)
		{
			_vertices[idx + i].color = colourForValue(val);
		}
	}

	forceRender();
}

void MatrixPlot::prepareSmallVertices()
{
	std::unique_lock<std::mutex> lock(_mutex);
	_index2Vertex.clear();
	int matNum = 0;
	int vertNum = 0;

	for (int i = 0; i < _mat.rows; i++)
	{
		for (int j = 0; j < _mat.cols; j++)
		{
			glm::vec3 pos{};
			pos.x = i * _xProp;
			pos.y = j * _yProp;
			
			for (int m = 0; m < 2; m++)
			{
				for (int n = 0; n < 2; n++)
				{
					glm::vec3 base = pos;
					base.x += m * _xProp;
					base.y += n * _yProp;
					glm::vec3 tmp = base;
					tmp.x -= 0.5; tmp.y -= 0.5;
					Snow::Vertex &v = addVertex(tmp);
					
					v.tex.x = 1 - base.x;
					v.tex.y = 1 - base.y;
					
					float val = _mat[i][j];
					v.color = colourForValue(val);
					vertNum++;
				}
			}
			
			addIndices(-4, -3, -2);
			addIndices(-3, -2, -1);
			
			_index2Vertex[matNum] = vertNum - 4;
			matNum++;
		}
	}

	forceRender(true, true);
}

void MatrixPlot::setup()
{
	_cols = _mat.cols;
	_rows = _mat.rows;
	
	_xProp = 1 / (float)_rows;
	_yProp = 1 / (float)_cols;
	
	if (_cols > _rows)
	{
		_xProp *= _rows / (float)_cols;
	}
	else if (_rows > _cols)
	{
		_yProp *= _cols / (float)_rows;
	}
	
	prepareSmallVertices();

	rescale(Window::aspect(), 1.);
}

void MatrixPlot::checkDimensions()
{
	if (_cols == _mat.cols && _rows == _mat.rows)
	{
		return;
	}

	clearVertices();
	setup();
	realign();
	
	float scale = resizeScale();
	resize(scale);
	setResizeScale(scale);
}
