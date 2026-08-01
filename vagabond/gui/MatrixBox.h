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
#include <vagabond/utils/OpSet.h>
#include <set>

class MatrixPlot;
class TextButton;
class Scene;

class MatrixBox : public Box
{
public:
	// scene is optional - only needed for the shift-drag-to-select-rows
	// feature (shiftPressed() lives on Scene/GLView, not on MatrixBox
	// itself). Leave it null for a plain reorderable matrix with no
	// selection support.
	MatrixBox(MatrixPlot *mp, const std::vector<std::string> &rowNames,
	          const std::vector<std::string> &colNames,
	          bool reorder = false, Scene *scene = nullptr);

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

	// original row indices currently selected via shift-drag on the plot
	// (see the Scene* constructor parameter). Empty if nothing is
	// selected, or if this MatrixBox was built without a Scene.
	const OpSet<int> &selectedRows() const
	{
		return _selectedRows;
	}
private:
	MatrixPlot *_plot{};
	Scene *_scene{};

	std::function<void(bool)> _enableButtons;
	auto create_order_function(int coord);

	// shift-drag-to-select-rows, driven directly off _plot rather than the
	// row labels (labels stop being individually distinguishable once
	// there are too many rows to fit legibly - see selectedRows()).
	auto create_plot_select_function();

	// converts a GL-space y coordinate (as delivered to a drag callback)
	// into a display row index, by matching _plot's own even row division
	// (see MatrixPlot::rebuildPixels()) rather than the row labels' own
	// placement, which - independently of this feature - can drift out of
	// step with the actual rendered rows (see draw()).
	int displayRowAtY(double glY) const;

	// like displayRowAtY(), but for block-drag's "how many rows precede
	// the drop point" question - ranges over [0, _rowPerm.size()]
	// (N+1 boundaries around N rows), not [0, N-1], so the block can be
	// dropped above the very top row (displayRowAtY() clamps to N-1,
	// which can only ever place a block immediately below the top row,
	// never above it).
	int insertionBoundaryAtY(double glY) const;

	// display row the current shift-drag started from, or -1 between
	// gestures - see create_plot_select_function().
	int _selectAnchorRow = -1;

	// rebuilds the translucent highlight quads from _selectedRows against
	// the *current* _rowPerm - called whenever the selection changes and
	// whenever refreshDisplay() runs, since a reorder can move an already
	// selected row to a new display position.
	void updateSelectionHighlight();

	// original row index a given row label button was created for - fixed
	// for that button's lifetime (see draw()), independent of how _rowPerm
	// has since changed via dragging.
	std::map<TextButton *, int> _rowOriginalIndex;

	// active only while dragging a row label that belongs to a selection
	// of more than one row - moves the whole selection together instead
	// of the ordinary single-row swap path in create_order_function().
	struct BlockDrag
	{
		bool active = false;
		TextButton *handle = nullptr;
		// other selected buttons' y offset from the handle at drag start,
		// so the whole block visually moves as one rigid group.
		std::map<TextButton *, float> offsetFromHandle;
		// insertion index _rowPerm was last spliced at - re-splicing only
		// when this changes avoids redundant refreshDisplay()/reposition
		// work on every mouse-move event.
		int lastInsertAt = -1;
	};
	BlockDrag _blockDrag;

	void beginBlockDrag(TextButton *handle);
	void updateBlockDrag(double y);
	void finishBlockDrag();

	// splices _selectedRows out of _rowPerm and reinserts them as a
	// contiguous block such that `insertAt` unselected rows precede them -
	// shared by the live in-drag reorder and the final drop.
	void applyBlockMove(int insertAt);

	// moves every row button (or just the unselected ones, mid-drag - the
	// selected ones are still under direct cursor control until drop) to
	// the slot _rowPerm currently says it belongs at.
	void snapRowButtons(bool includeSelected);

	OpSet<int> _selectedRows;
	std::vector<Box *> _highlightQuads;

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
