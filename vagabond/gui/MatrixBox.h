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

#ifndef __vagabond__MatrixBox__
#define __vagabond__MatrixBox__

#include <vagabond/gui/elements/Box.h>
#include <vagabond/utils/Eigen/Dense>
#include <set>

class MatrixPlot;
class TextButton;

class MatrixBox : public Box
{
public:
	MatrixBox(MatrixPlot *mp, const std::vector<std::string> &rowNames,
	          const std::vector<std::string> &colNames,
	          bool reorder = false);

	void guessReordering();
	void draw();

	std::vector<std::string> rowNames();
	std::vector<std::string> colNames();

	// _original never changes except through these - read/write a value
	// by its stable, original row/column index regardless of how the
	// display has since been dragged around. See refreshDisplay().
	float valueAtOriginal(int origRow, int origCol) const;
	void setValueAtOriginal(int origRow, int origCol, float value);

	// bulk equivalents for updating every value at once (e.g. a freshly
	// recomputed correlation matrix) - a loop of setValueAtOriginal()
	// calls would work but re-derives and re-pushes the whole display
	// matrix on every single call. matrix must be the same shape as
	// whatever MatrixBox was originally constructed with.
	const Eigen::MatrixXf &originalMatrix() const
	{
		return _original;
	}
	void setOriginalMatrix(const Eigen::MatrixXf &matrix);
private:
	MatrixPlot *_plot{};

	std::function<void(bool)> _enableButtons;
	auto create_order_function(int coord);

	// re-derives the displayed matrix as
	// display(i, j) = _original(_rowPerm[i], _colPerm[j]) and pushes it
	// into _plot's storage - the single place that keeps labels and
	// rendered data from being able to drift apart, since every drag and
	// every original-index write goes through here.
	void refreshDisplay();

	// _identical only: _colPerm must always equal _rowPerm reversed (the
	// same item sits at row position i and column position N-1-i - see
	// the constructor for why rows start reversed and columns do not).
	// Re-derives the other permutation from whichever one just changed,
	// rather than composing the same positional order into both
	// independently, which only stays correct as long as they have not
	// yet diverged from that shared baseline.
	void syncCoupledPerm(bool rowsChanged);

	struct Status
	{
		bool dragging = false;
		glm::vec3 start = {};
		glm::vec3 hypothetical = {};
		int coord = -1;
	};

	std::set<std::pair<float, TextButton *>> _order;
	std::map<TextButton *, Status> _info;
	bool _redraw = false;
	bool _identical = false;

	std::vector<std::string> _rowNames;
	std::vector<std::string> _colNames;
	std::map<TextButton *, TextButton *> _couples;

	// display position -> original index. _rowPerm starts as a reversal
	// (not identity) to compensate for MatrixPlot rendering in raw OpenGL
	// space (+y is up) while row labels read top-down - see MatrixBox.cpp.
	std::vector<int> _rowPerm;
	std::vector<int> _colPerm;

	// captured once from _plot's matrix at construction, before any
	// baseline flip or reordering, and never mutated again directly -
	// only ever read through refreshDisplay()/valueAtOriginal(), and
	// written through setValueAtOriginal().
	Eigen::MatrixXf _original;
};

#endif
