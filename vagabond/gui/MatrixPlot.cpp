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
#include <vagabond/gui/elements/GLView.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/utils/maths.h>
#include <vagabond/gui/ColourLegend.h>
#include <vagabond/core/Environment.h>

MatrixPlot::MatrixPlot(Eigen::MatrixXf &mat)
: Image(Environment::matrixBackgroundFilename()), _eigenMat(&mat)
{
	_legend = new ColourLegend(Cluster4x, true, nullptr);
	_legend->disableButtons();

	clearVertices();
	setup();
}

MatrixPlot::MatrixPlot(Eigen::MatrixXf &mat, std::mutex &mutex)
: Image(Environment::matrixBackgroundFilename()), _eigenMat(&mat), _mutex(&mutex)
{
	_legend = new ColourLegend(Cluster4x, true, nullptr);
	_legend->disableButtons();

	clearVertices();
	setup();
}

MatrixPlot::MatrixPlot(PCA::Matrix &mat)
: Image(Environment::matrixBackgroundFilename()), _pcaMat(&mat)
{
	_legend = new ColourLegend(Cluster4x, true, nullptr);
	_legend->disableButtons();

	clearVertices();
	setup();
}


MatrixPlot::MatrixPlot(PCA::Matrix &mat, std::mutex &mutex)
: Image(Environment::matrixBackgroundFilename()), _pcaMat(&mat), _mutex(&mutex)
{
	_legend = new ColourLegend(Cluster4x, true, nullptr);
	_legend->disableButtons();

	clearVertices();
	setup();
}

int MatrixPlot::matRows() const
{
	return _eigenMat ? (int)_eigenMat->rows() : _pcaMat->rows;
}

int MatrixPlot::matCols() const
{
	return _eigenMat ? (int)_eigenMat->cols() : _pcaMat->cols;
}

float MatrixPlot::valueAt(int i, int j) const
{
	return _eigenMat ? (*_eigenMat)(i, j) : (float)(*_pcaMat)[i][j];
}

Eigen::MatrixXf MatrixPlot::toEigen() const
{
	return _eigenMat ? *_eigenMat : _pcaMat->toEigen();
}

void MatrixPlot::dropFromEigen(const Eigen::MatrixXf &m)
{
	if (_eigenMat)
	{
		*_eigenMat = m;
	}
	else
	{
		_pcaMat->dropFromEigen(m);
	}
}

glm::vec4 MatrixPlot::colourForValue(float val)
{
	glm::vec4 v = _legend->colour(val);
	return v;
}

void MatrixPlot::update()
{
	std::unique_lock<std::mutex> lock(_buffLock);
	
	if (!checkDimensions())
	{
		lock.unlock();
	}

	if (_mutex)
	{
		std::unique_lock<std::mutex> datalock(*_mutex, std::defer_lock);
		if (datalock.try_lock())
		{
			updateColours();
		}
	}
	else
	{
		updateColours();
	}
}

void MatrixPlot::updateColours()
{
	for (auto it = _index2Vertex.begin(); it != _index2Vertex.end(); it++)
	{
		int row = it->first / _cols;
		int col = it->first % _cols;
		float val = valueAt(row, col);
		int idx = it->second;

		for (size_t i = 0; i < 4; i++)
		{
			_vertices[idx + i].color = colourForValue(val);
		}
	}

	forceRender(true, false);
	if (_gl)
	{
		_gl->viewChanged();
	}
}

void MatrixPlot::prepareSmallVertices()
{
	_index2Vertex.clear();
	int matNum = 0;
	int vertNum = 0;

	// row index -> vertical (y), column index -> horizontal (x) - matches
	// how row/column labels get placed by callers like MatrixBox (rows
	// down the left, columns across the bottom).
	for (int i = 0; i < _rows; i++)
	{
		for (int j = 0; j < _cols; j++)
		{
			glm::vec3 pos{};
			pos.x = j * _xProp;
			pos.y = i * _yProp;

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

					float val = valueAt(i, j);
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
	_cols = matCols();
	_rows = matRows();
	
	// x spans columns, y spans rows - see prepareSmallVertices().
	_xProp = 1 / (float)_cols;
	_yProp = 1 / (float)_rows;

	if (_rows > _cols)
	{
		_xProp *= _cols / (float)_rows;
	}
	else if (_cols > _rows)
	{
		_yProp *= _rows / (float)_cols;
	}
	
	prepareSmallVertices();

	rescale(Window::aspect(), 1.);
	updateColours();
}

bool MatrixPlot::checkDimensions()
{
	if (_cols == matCols() && _rows == matRows())
	{
		return false;
	}

	clearVertices();
	setup();
	realign();
	
	float scale = resizeScale();
	resize(scale);
	setResizeScale(scale);
	
	return true;
}

bool MatrixPlot::mouseOver()
{
	if (!_hoverJob) return false;

	// _gl (Renderable::_gl) is only set inside render(GLView*) - a
	// freshly-added MatrixPlot can already be spatially hit-testable
	// (Scene::mouseMoveEvent's findObject()) before it has ever actually
	// been rendered once, so this can legitimately still be null here.
	if (!_gl) return false;

	double x, y;
	_gl->getMoveCoords(x, y);
	
	float &minx = _vertices[0].pos.x;
	float &miny = _vertices[0].pos.y;
	float maxx = vertices().back().pos.x;
	float maxy = vertices().back().pos.y;
	
	float cx = (x - minx) / (maxx - minx);
	float cy = (y - miny) / (maxy - miny);
	
	_hoverJob(cx, cy);
	
	return true;
}
