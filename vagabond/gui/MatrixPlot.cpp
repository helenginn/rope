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

	setup();
}

MatrixPlot::MatrixPlot(Eigen::MatrixXf &mat, std::mutex &mutex)
: Image(Environment::matrixBackgroundFilename()), _eigenMat(&mat), _mutex(&mutex)
{
	_legend = new ColourLegend(Cluster4x, true, nullptr);
	_legend->disableButtons();

	setup();
}

MatrixPlot::MatrixPlot(PCA::Matrix &mat)
: Image(Environment::matrixBackgroundFilename()), _pcaMat(&mat)
{
	_legend = new ColourLegend(Cluster4x, true, nullptr);
	_legend->disableButtons();

	setup();
}


MatrixPlot::MatrixPlot(PCA::Matrix &mat, std::mutex &mutex)
: Image(Environment::matrixBackgroundFilename()), _pcaMat(&mat), _mutex(&mutex)
{
	_legend = new ColourLegend(Cluster4x, true, nullptr);
	_legend->disableButtons();

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

	checkDimensions();

	if (_mutex)
	{
		std::unique_lock<std::mutex> datalock(*_mutex, std::defer_lock);
		if (datalock.try_lock())
		{
			rebuildPixels();
		}
	}
	else
	{
		rebuildPixels();
	}
}

void MatrixPlot::rebuildPixels()
{
	// _buffLock is already held by update() - rebuildPixels() only ever
	// touches CPU-side data (this buffer, not GL state), so it is safe
	// to call from whichever thread called update() (e.g. a worker
	// thread driving a live-updating plot). The actual texture upload
	// only ever happens from render() - see there for why.
	_pixels.resize((size_t)_rows * _cols * 4);

	for (int i = 0; i < _rows; i++)
	{
		for (int j = 0; j < _cols; j++)
		{
			glm::vec4 colour = colourForValue(valueAt(i, j));

			// row 0 needs to land in the texture's LAST row: Box::addQuad()
			// maps world y=-1 (bottom, where the old per-cell grid put row
			// 0 - pos.y = i * _yProp increases with i) to texcoord v=1,
			// i.e. the far end of the buffer, not the start of it.
			size_t bufRow = (size_t)(_rows - 1 - i);
			size_t idx = (bufRow * _cols + j) * 4;
			_pixels[idx + 0] = (unsigned char)(colour.r * 255.f);
			_pixels[idx + 1] = (unsigned char)(colour.g * 255.f);
			_pixels[idx + 2] = (unsigned char)(colour.b * 255.f);
			_pixels[idx + 3] = (unsigned char)(colour.a * 255.f);
		}
	}

	_textureDirty = true;

	if (_gl)
	{
		_gl->viewChanged();
	}
}

void MatrixPlot::render(GLView *sender)
{
	{
		std::unique_lock<std::mutex> lock(_buffLock);
		if (_textureDirty)
		{
			changeImage(_pixels.data(), _cols, _rows);
			_textureDirty = false;
		}
	}

	Image::render(sender);
}

void MatrixPlot::setup()
{
	_cols = matCols();
	_rows = matRows();

	// x spans columns, y spans rows - fit the matrix's true row:col
	// aspect ratio into a unit box while keeping cells square, same
	// arithmetic this class has always used (previously applied per
	// cell, now applied once to the single quad below).
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

	// Box::makeQuad() spans -1..+1 (size 2) in both axes, so halving the
	// target span gets the same absolute cols*_xProp x rows*_yProp size
	// the old per-cell geometry naturally produced, before the same
	// separate window-aspect correction the old code applied via its own
	// rescale(Window::aspect(), 1.) call after building geometry.
	Box::makeQuad();
	rescale(_xProp * _cols / 2., _yProp * _rows / 2.);
	rescale(Window::aspect(), 1.);

	rebuildPixels();
}

bool MatrixPlot::checkDimensions()
{
	if (_cols == matCols() && _rows == matRows())
	{
		return false;
	}

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
