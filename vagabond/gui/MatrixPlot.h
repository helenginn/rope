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

#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/gui/elements/Image.h>
#include <functional>
#include <mutex>
#include <vector>

class ColourLegend;
class GLView;

class MatrixPlot : public Image
{
public:
	MatrixPlot(Eigen::MatrixXf &mat, std::mutex &mutex);
	MatrixPlot(Eigen::MatrixXf &mat);

	// PCA::Matrix compatibility for callers not yet migrated to
	// Eigen::MatrixXf directly - remove once every caller in the repo has
	// moved over and PCA::Matrix itself can be expunged.
	MatrixPlot(PCA::Matrix &mat, std::mutex &mutex);
	MatrixPlot(PCA::Matrix &mat);

	virtual void update();
	virtual bool mouseOver();
	virtual void render(GLView *sender);

	void setHoverJob(const std::function<void(float, float)> &job)
	{
		_hoverJob = job;
		setSelectable(true);
	}

	// full-matrix read/write regardless of which backing storage this
	// instance holds - used by MatrixBox to permute/reorder for display
	// without needing to know whether this plot is Eigen- or
	// PCA::Matrix-backed.
	Eigen::MatrixXf toEigen() const;
	void dropFromEigen(const Eigen::MatrixXf &m);

	ColourLegend *const legend() const
	{
		return _legend;
	}
private:
	glm::vec4 colourForValue(float val);
	bool checkDimensions();
	void rebuildPixels();
	void setup();

	int matRows() const;
	int matCols() const;
	float valueAt(int i, int j) const;

	Eigen::MatrixXf *_eigenMat = nullptr;

	// back-compat only, see constructor comment above.
	PCA::Matrix *_pcaMat = nullptr;

	std::mutex *_mutex{};

	float _xProp = 1;
	float _yProp = 1;

	int _rows = 0;
	int _cols = 0;

	// _pixels/_textureDirty are guarded by the inherited _buffLock -
	// rebuildPixels() can run on whatever thread calls update() (e.g. a
	// worker thread driving a live-updating plot), but the actual GL
	// texture upload only ever happens from render() (see there for why).
	std::vector<unsigned char> _pixels;
	bool _textureDirty = false;

	std::function<void(float, float)> _hoverJob;
	ColourLegend *_legend{};
};

#endif
