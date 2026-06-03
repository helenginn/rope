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

#include <cmath>
#include <algorithm>
#include <iostream>
#include <vagabond/utils/OpSet.h>
#include "MatrixBox.h"
#include <vagabond/gui/MatrixPlot.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/gui/elements/Scene.h>

MatrixBox::MatrixBox(MatrixPlot *mp, const std::vector<std::string> &rowNames,
                     const std::vector<std::string> &colNames,
                     bool reorder, Scene *scene)
: _plot(mp), _scene(scene), _rowNames(rowNames), _colNames(colNames)
{
	_identical = (&rowNames == &colNames);

	// capture before anything else touches _plot's matrix - this is the
	// one copy that never gets permuted.
	_original = _plot->toEigen();

	// MatrixPlot renders row index along +y in raw OpenGL space, and +y is
	// up - so a row list in natural order renders bottom-up. Compensate
	// here, once, as the starting permutation rather than a separate
	// reversal disconnected from the data: _rowPerm begins reversed so
	// row labels (and the matrix rows behind them) both render top-down
	// together. Columns run along +x, which needs no such flip.
	_rowPerm.resize(_rowNames.size());
	for (size_t i = 0; i < _rowPerm.size(); i++)
	{
		_rowPerm[i] = _rowPerm.size() - 1 - i;
	}

	_colPerm.resize(_colNames.size());
	for (size_t i = 0; i < _colPerm.size(); i++)
	{
		_colPerm[i] = i;
	}

	refreshDisplay();

	_plot->setCentre(0.5, 0.5);
	addObject(_plot);

	if (reorder)
	{
		guessReordering();
	}
	draw();
}

void MatrixBox::refreshDisplay()
{
	Eigen::MatrixXf display(_rowPerm.size(), _colPerm.size());
	for (size_t i = 0; i < _rowPerm.size(); i++)
	{
		for (size_t j = 0; j < _colPerm.size(); j++)
		{
			display(i, j) = _original(_rowPerm[i], _colPerm[j]);
		}
	}

	_plot->dropFromEigen(display);
	_plot->update();

	// a reorder can move an already-selected row to a new display
	// position - keep the highlight following it rather than leaving it
	// pointing at whatever now occupies the old slot.
	updateSelectionHighlight();
}

void MatrixBox::syncCoupledPerm(bool rowsChanged)
{
	std::vector<int> &source = rowsChanged ? _rowPerm : _colPerm;
	std::vector<int> &target = rowsChanged ? _colPerm : _rowPerm;
	target = source;
	std::reverse(target.begin(), target.end());
}

float MatrixBox::valueAtOriginal(int origRow, int origCol) const
{
	return _original(origRow, origCol);
}

void MatrixBox::setValueAtOriginal(int origRow, int origCol, float value)
{
	_original(origRow, origCol) = value;
	refreshDisplay();
}

void MatrixBox::setOriginalMatrix(const Eigen::MatrixXf &matrix)
{
	if (matrix.rows() != _original.rows() || matrix.cols() != _original.cols())
	{
		std::cout << "MatrixBox::setOriginalMatrix: expected " <<
		_original.rows() << "x" << _original.cols() << ", got " <<
		matrix.rows() << "x" << matrix.cols() << " - ignoring" << std::endl;
		return;
	}

	_original = matrix;
	refreshDisplay();
}

typedef std::pair<float, TextButton *> OrderedButton;
auto MatrixBox::create_order_function(int coord)
{
	return [this, coord]()
	{
		std::set<OrderedButton> ordered;
		for (auto &pair : _info)
		{
			if (pair.second.coord == coord)
			{
				ordered.insert({pair.second.hypothetical[coord],
				               pair.first});
			}
		}
		return ordered;
	};
};

int MatrixBox::displayRowAtY(double glY) const
{
	if (_rowPerm.empty())
	{
		return 0;
	}

	glm::vec3 minC, maxC;
	_plot->bounds(minC, maxC);

	if (maxC.y == minC.y)
	{
		return 0;
	}

	// _plot's own rendered rows are an even division of its true extent
	// (see MatrixPlot::rebuildPixels()) - matching that directly, rather
	// than going via the row label buttons' positions, sidesteps whatever
	// the labels' own placement formula happens to be (see draw()).
	// _rowPerm[0] is the *bottom* row, not the top - confirmed by the
	// original single-row drag path (create_order_function() sorts
	// buttons ascending by y, and that sorted order is what gets composed
	// straight into _rowPerm) - fraction 0 at the plot's bottom edge.
	float frac = ((float)glY - minC.y) / (maxC.y - minC.y);
	int row = (int)std::floor(frac * (float)_rowPerm.size());
	row = std::max(0, std::min((int)_rowPerm.size() - 1, row));
	return row;
}

int MatrixBox::insertionBoundaryAtY(double glY) const
{
	if (_rowPerm.empty())
	{
		return 0;
	}

	glm::vec3 minC, maxC;
	_plot->bounds(minC, maxC);

	if (maxC.y == minC.y)
	{
		return 0;
	}

	// same bottom-is-0 orientation as displayRowAtY(), but rounded to the
	// nearest boundary *line* between rows (0..N inclusive) rather than
	// floored to a row band (0..N-1) - see the comment in MatrixBox.h.
	float frac = ((float)glY - minC.y) / (maxC.y - minC.y);
	int boundary = (int)std::round(frac * (float)_rowPerm.size());
	boundary = std::max(0, std::min((int)_rowPerm.size(), boundary));
	return boundary;
}

void MatrixBox::updateSelectionHighlight()
{
	for (Box *quad : _highlightQuads)
	{
		removeObject(quad);
		delete quad;
	}
	_highlightQuads.clear();

	if (_selectedRows.size() == 0)
	{
		return;
	}

	// invert _rowPerm (original index -> display position) so a selection
	// stored by original index - stable across reorders - can be drawn at
	// wherever those rows currently sit on screen.
	std::vector<int> displayPosForOriginal(_rowPerm.size());
	for (size_t i = 0; i < _rowPerm.size(); i++)
	{
		displayPosForOriginal[_rowPerm[i]] = (int)i;
	}

	std::vector<bool> selectedAtDisplay(_rowPerm.size(), false);
	for (int orig : _selectedRows)
	{
		selectedAtDisplay[displayPosForOriginal[orig]] = true;
	}

	// _plot's own rendered rows are an even division of its true extent
	// (see MatrixPlot::rebuildPixels() and displayRowAtY()) - matching
	// that directly keeps the highlight pinned to the actual heatmap
	// rows regardless of the row labels' own placement.
	glm::vec3 minC, maxC;
	_plot->bounds(minC, maxC);
	float halfWidth = fabs(maxC.x - minC.x) / 2.f;
	float centreX = (maxC.x + minC.x) / 2.f;
	float rowSpan = (maxC.y - minC.y) / (float)_rowPerm.size();

	// merge consecutive selected display rows into a single quad rather
	// than one per row.
	size_t i = 0;
	while (i < selectedAtDisplay.size())
	{
		if (!selectedAtDisplay[i])
		{
			i++;
			continue;
		}

		size_t runStart = i;
		while (i < selectedAtDisplay.size() && selectedAtDisplay[i])
		{
			i++;
		}
		size_t runEnd = i - 1;

		// display position 0 is the bottom row - see displayRowAtY().
		float bottomY = minC.y + (float)runStart * rowSpan;
		float topY = minC.y + (float)(runEnd + 1) * rowSpan;
		float centreY = (topY + bottomY) / 2.f;
		float halfHeight = fabs(topY - bottomY) / 2.f;

		Box *quad = new Box();
		quad->makeQuad();
		quad->rescale(halfWidth, halfHeight);
		quad->setPosition(glm::vec3(centreX, centreY, minC.z + 0.05f));
		quad->setColour(1.00, 1.00, 0.0);
		quad->setAlpha(0.45);
		addObject(quad);
		_highlightQuads.push_back(quad);
	}
}

auto MatrixBox::create_plot_select_function()
{
	return [this](double x, double y, bool lastOne)
	{
		if (lastOne)
		{
			// x/y are not meaningful on this call (see Renderable::undrag())
			// - whatever the last real move event computed already stands.
			_selectAnchorRow = -1;
			return;
		}

		if (!_scene || !_scene->shiftPressed())
		{
			return;
		}

		int row = displayRowAtY(y);

		if (_selectAnchorRow < 0)
		{
			_selectAnchorRow = row;
		}

		int lo = std::min(_selectAnchorRow, row);
		int hi = std::max(_selectAnchorRow, row);

		_selectedRows.clear();
		for (int i = lo; i <= hi; i++)
		{
			_selectedRows.insert(_rowPerm[i]);
		}

		updateSelectionHighlight();
	};
}

void MatrixBox::beginBlockDrag(TextButton *handle)
{
	_blockDrag.active = true;
	_blockDrag.handle = handle;
	_blockDrag.offsetFromHandle.clear();
	_blockDrag.lastInsertAt = -1;

	float handleY = handle->centroid().y;

	for (auto &pair : _rowOriginalIndex)
	{
		TextButton *other = pair.first;
		if (other == handle || !_selectedRows.count(pair.second))
		{
			continue;
		}

		_blockDrag.offsetFromHandle[other] = other->centroid().y - handleY;
	}
}

void MatrixBox::applyBlockMove(int insertAt)
{
	// selected original rows, in their current (pre-move) display order -
	// preserves their relative order within the moved block.
	std::vector<int> selectedInOrder;
	std::vector<int> remaining;
	for (int orig : _rowPerm)
	{
		if (_selectedRows.count(orig))
		{
			selectedInOrder.push_back(orig);
		}
		else
		{
			remaining.push_back(orig);
		}
	}

	insertAt = std::max(0, std::min(insertAt, (int)remaining.size()));

	std::vector<int> updated;
	updated.reserve(_rowPerm.size());
	updated.insert(updated.end(), remaining.begin(), remaining.begin() + insertAt);
	updated.insert(updated.end(), selectedInOrder.begin(), selectedInOrder.end());
	updated.insert(updated.end(), remaining.begin() + insertAt, remaining.end());

	_rowPerm = updated;

	if (_identical)
	{
		syncCoupledPerm(true);
	}

	refreshDisplay();
}

void MatrixBox::snapRowButtons(bool includeSelected)
{
	// signed y-delta per slot increase and the y for slot 0 - slot 0 is
	// the *bottom* row, not the top (see displayRowAtY()), matching
	// _plot's own even row division rather than the row labels' own
	// placement.
	glm::vec3 minC, maxC;
	_plot->bounds(minC, maxC);
	size_t n = std::max((size_t)1, _rowPerm.size());
	float rowHeight = fabs(maxC.y - minC.y) / (float)n;
	float slot0Y = minC.y + rowHeight / 2.f;

	// _identical: a row's coupled column label (see draw()'s _couples,
	// matched by shared name) needs to track _colPerm too, since
	// syncCoupledPerm() reorders the underlying data columns right along
	// with the rows - the original single-row drag path does this via
	// steal_coordinate_from_other(), which block-drag has no equivalent
	// of, so it has to happen here instead or the column labels are left
	// pointing at the wrong data columns after any row block-move.
	float colSpan = 0.f;
	float slot0X = 0.f;
	if (_identical && _colPerm.size() > 0)
	{
		colSpan = fabs(maxC.x - minC.x) / (float)_colPerm.size();
		slot0X = minC.x + colSpan / 2.f;
	}

	for (auto &pair : _rowOriginalIndex)
	{
		TextButton *button = pair.first;
		int orig = pair.second;

		// still under direct cursor control mid-drag - see
		// updateBlockDrag() - but its coupled column below is
		// repositioned regardless, since that one isn't being dragged.
		if (includeSelected || !_selectedRows.count(orig))
		{
			auto it = std::find(_rowPerm.begin(), _rowPerm.end(), orig);
			int displayPos = (int)std::distance(_rowPerm.begin(), it);

			glm::vec3 pos = button->centroid();
			pos.y = slot0Y + (float)displayPos * rowHeight;
			button->setPosition(pos);
		}

		if (_identical)
		{
			auto coupleIt = _couples.find(button);
			if (coupleIt != _couples.end() && coupleIt->second)
			{
				auto colIt = std::find(_colPerm.begin(), _colPerm.end(), orig);
				int colPos = (int)std::distance(_colPerm.begin(), colIt);

				glm::vec3 cpos = coupleIt->second->centroid();
				cpos.x = slot0X + (float)colPos * colSpan;
				coupleIt->second->setPosition(cpos);
			}
		}
	}
}

void MatrixBox::updateBlockDrag(double y)
{
	glm::vec3 pos = _blockDrag.handle->centroid();
	pos.y = y;
	_blockDrag.handle->setPosition(pos);

	for (auto &pair : _blockDrag.offsetFromHandle)
	{
		glm::vec3 opos = pair.first->centroid();
		opos.y = y + pair.second;
		pair.first->setPosition(opos);
	}

	// re-splice _rowPerm live as the block crosses other rows, same as
	// the single-row drag path does - only when the target slot actually
	// changes, to avoid redundant refreshDisplay()/reposition work on
	// every mouse-move event.
	int targetBoundary = insertionBoundaryAtY(y);

	int insertAt = 0;
	for (int p = 0; p < targetBoundary && p < (int)_rowPerm.size(); p++)
	{
		if (!_selectedRows.count(_rowPerm[p]))
		{
			insertAt++;
		}
	}

	if (insertAt != _blockDrag.lastInsertAt)
	{
		applyBlockMove(insertAt);
		snapRowButtons(false);
		updateSelectionHighlight();
		_blockDrag.lastInsertAt = insertAt;
	}
}

void MatrixBox::finishBlockDrag()
{
	// _rowPerm is already up to date from the live re-splicing in
	// updateBlockDrag() - just settle every button (including the
	// selected ones, still under direct cursor control until now) onto
	// its final slot.
	snapRowButtons(true);

	_blockDrag = BlockDrag();

	updateSelectionHighlight();
}

void MatrixBox::draw()
{

	float width = _plot->maximalWidth() / 2.f;
	float height = _plot->maximalHeight() / 2.f;
	float xstep = width / (float)_colNames.size();
	float ystep = -height / (float)_rowNames.size();
	float x = -width / 2 + xstep / 2;
	float y = +height / 2 + ystep / 2;

	if (_scene)
	{
		// setDragFunction() alone only makes _plot draggable - hit-testing
		// in HasRenderables::findObject() gates on isSelectable() first
		// (Image/MatrixPlot isn't a Button, so unlike the row/col labels
		// it isn't selectable by default), so without this the plot is
		// never even found by a mouse press and nothing fires at all.
		_plot->setSelectable(true);
		_plot->setDragFunction(create_plot_select_function());
		_plot->setClickJob([this]()
		{
			if (_scene->shiftPressed())
			{
				_selectedRows.clear();
				updateSelectionHighlight();
				_scene->viewChanged();
			}
		});
	}

	auto drag_button = [this](TextButton *tb, int coord)
	{
		_info[tb] = {};
		_info[tb].coord = coord;
		auto create_order = create_order_function(coord);
		
		auto swap_required = [](const std::set<OrderedButton> &orig,
		                   const std::set<OrderedButton> &update,
		                   TextButton *search) -> std::pair<int, int>
		{
			auto it = orig.begin();
			int c = 0;
			int n = -1;
			int m = -1;
			
			for (auto &pair : update)
			{
				TextButton *current = pair.second;
				TextButton *old = it->second;
				
				if (current == search) { n = c; }
				if (old == search) { m = c; }

				it++; c++;
			}

			return {m, n};
		};
		
		auto save_current_pos_info = [this]()
		{
			// prepare all other positions
			for (auto &pair : _info)
			{
				glm::vec3 orig = pair.first->centroid();
				pair.second.start = orig;
				pair.second.hypothetical = orig;
			}
		};
		
		auto set_coordinate = [this]
		(TextButton *tb, const glm::vec3 &pos, int coord)
		{
			glm::vec3 curr = _info[tb].start;
			curr[coord] = pos[coord];
			tb->setPosition(curr);
		};

		auto steal_coordinate_from_other = [this, set_coordinate]
		(TextButton *curr, TextButton *old)
		{
			set_coordinate(curr, _info[old].start, _info[old].coord);
			if (_identical)
			{
				TextButton *pCurr = _couples[curr];
				TextButton *pOld = _couples[old];

				if (pCurr && pOld)
				{
					set_coordinate(pCurr, _info[pOld].start, _info[pOld].coord);
				}
			}
		};

		return [this, tb, create_order, swap_required, coord,
		        steal_coordinate_from_other, save_current_pos_info]
		(double x, double y, bool lastOne)
		{
			Status &status = _info[tb];

			// dragging a row that's part of a multi-row selection moves
			// the whole selection together instead of swapping individual
			// rows - see beginBlockDrag()/updateBlockDrag()/
			// finishBlockDrag(). Columns are unaffected (rows-only, see
			// MatrixBox.h).
			if (coord == 1 && !_blockDrag.active && !status.dragging
			    && _selectedRows.size() > 1
			    && _rowOriginalIndex.count(tb)
			    && _selectedRows.count(_rowOriginalIndex.at(tb)))
			{
				beginBlockDrag(tb);
			}

			if (_blockDrag.active && _blockDrag.handle == tb)
			{
				if (lastOne)
				{
					finishBlockDrag();
				}
				else
				{
					updateBlockDrag(y);
				}
				return;
			}

			if (!status.dragging)
			{
				save_current_pos_info();

				_order = create_order();
				status.dragging = true;
			}
			else if (lastOne)
			{
				tb->setPosition(status.start);
				_info[tb].dragging = false;
			}
			else
			{
				float xf = x;
				float yf = y;

				tb->setPosition({xf, yf, 0});
				status.hypothetical = {xf, yf, 0};
			}
			
			auto tmp = create_order();
			auto it = _order.begin();
			bool changed = false;
			
			std::vector<int> order; order.reserve(tmp.size());
			for (auto &pair : tmp)
			{
				TextButton *current = pair.second;
				TextButton *old = it->second;
				if (current != old)
				{
					steal_coordinate_from_other(current, old);
					changed = true;
				}

				std::pair<int, int> swap = swap_required(_order, tmp, current);
				order.push_back(swap.first);
				it++;
			}

			if (changed)
			{
				// order[newPos] = oldPos, over display positions for this
				// coord only. Compose it onto whichever permutation vector
				// this drag directly controls.
				auto compose = [](std::vector<int> &perm,
				                  const std::vector<int> &order)
				{
					std::vector<int> updated(perm.size());
					for (size_t i = 0; i < order.size(); i++)
					{
						updated[i] = perm[order[i]];
					}
					perm = updated;
				};

				if (coord == 1) // buttons on y -> row swaps
				{
					compose(_rowPerm, order);
				}
				else if (coord == 0)
				{
					compose(_colPerm, order);
				}

				// _identical: composing the same positional order into
				// both independently only stays correct while they share
				// a baseline - re-derive the other one from whichever
				// just changed instead, to keep rows and columns in
				// lock-step (see syncCoupledPerm()).
				if (_identical)
				{
					syncCoupledPerm(coord == 1);
				}

				refreshDisplay();

				save_current_pos_info();
				_order = tmp;
			}
			
		};
	};
	
	Renderable::Alignment row_align = Renderable::Alignment::Right;
	Renderable::Alignment col_align = Renderable::Alignment
	(Renderable::Alignment::Centre | Renderable::Alignment::Bottom);
	
	// buttons keep a fixed name for their whole lifetime once created here
	// - dragging only ever moves a button's position, never its text. The
	// baseline flip (and any pre-drag reordering from guessReordering())
	// is therefore expressed entirely by which original name gets placed
	// at which starting display position, via _rowPerm/_colPerm.
	std::vector<TextButton *> rowButtons(_rowNames.size());
	for (size_t i = 0; i < _rowNames.size(); i++)
	{
		const std::string &first = _rowNames[_rowPerm[i]];
		TextButton *t = new TextButton(first);
		t->resize(0.3);
		t->setArbitrary(-width / 2, y, row_align);
		t->setDragFunction(drag_button(t, 1));
		addObject(t);
		_rowOriginalIndex[t] = _rowPerm[i];
		y += ystep;
		rowButtons[i] = t;
	}

	std::vector<TextButton *> colButtons(_colNames.size());
	for (size_t j = 0; j < _colNames.size(); j++)
	{
		const std::string &first = _colNames[_colPerm[j]];
		TextButton *t = new TextButton(first);
		t->resize(0.3);
		t->setDragFunction(drag_button(t, 0));
		t->setArbitrary(x, -height / 2, col_align);
		glm::mat4x4 base = glm::mat3x3(1.f);
		glm::mat3x3 rot;
		rot = glm::mat3x3(glm::rotate(base, (float)deg2rad(-90),
		                              glm::vec3(0., 0., -1.)));
		rot[1] *= 0.8;
		rot[0] /= 0.8;
		t->rotateRoundCentre(rot);
		addObject(t);
		x += xstep;
		colButtons[j] = t;
	}

	if (_identical)
	{
		// pair by matching label, not by position in either list - robust
		// regardless of which axis carries the baseline flip, and the
		// direct fix for rows/columns not staying in step with each other.
		for (TextButton *rowButton : rowButtons)
		{
			for (TextButton *colButton : colButtons)
			{
				if (rowButton->text() == colButton->text())
				{
					_couples[rowButton] = colButton;
					_couples[colButton] = rowButton;
					break;
				}
			}
		}
	}
}

void removeRow(Eigen::MatrixXf& matrix, unsigned int rowToRemove)
{
	unsigned int numRows = matrix.rows() - 1;
	unsigned int numCols = matrix.cols();

	if (rowToRemove < numRows)
	{
		matrix.block(rowToRemove,0,numRows-rowToRemove,numCols) 
		= matrix.block(rowToRemove+1,0,numRows-rowToRemove,numCols);

	}
	matrix.conservativeResize(numRows,numCols);
}

void removeColumn(Eigen::MatrixXf& matrix, unsigned int colToRemove)
{
	unsigned int numRows = matrix.rows();
	unsigned int numCols = matrix.cols() - 1;

	if (colToRemove < numCols)
	{
		matrix.block(0,colToRemove,numRows,numCols-colToRemove) 
		= matrix.block(0,colToRemove+1,numRows,numCols-colToRemove);
	}

	matrix.conservativeResize(numRows,numCols);
}

void MatrixBox::guessReordering()
{
	if (!_identical)
	{
		return;
	}

	// operates on the current display matrix (i.e. already reflecting
	// _rowPerm/_colPerm as they stand right now, including the baseline
	// flip) - logicalOrder accumulates the reordering relative to *that*
	// starting arrangement, then gets composed onto _rowPerm/_colPerm
	// below, rather than being written to _plot's matrix directly.
	Eigen::MatrixXf A = _plot->toEigen();
	OpSet<int> row_list;
	for (int i = 0; i < A.rows(); i++)
	{
		row_list += i;
	}

	std::vector<int> logicalOrder(A.rows());
	for (int i = 0; i < A.rows(); i++)
	{
		logicalOrder[i] = i;
	}

	auto insertion = []<typename Obj>(std::vector<Obj> &list, int i, int j)
	{
		Obj tmp = list[i];
		list.erase(list.begin() + i);
		if (j > i) j--;
		list.insert(list.begin() + j, tmp);
	};

	auto insert_rows = [&](int i, int j)
	{
		std::vector<int> reorder = {row_list.begin(), row_list.end()};
		insertion(reorder, i, j);

		insertion(logicalOrder, i, j);

		Eigen::VectorXi perm = Eigen::Map<Eigen::VectorXi>(reorder.data(),
		                                                   reorder.size());

		Eigen::PermutationMatrix<Eigen::Dynamic, Eigen::Dynamic> P(perm);
		Eigen::MatrixXf pA = P.transpose() * A * P;
		A = pA;
	};
	
	auto get_dot = [&](int i, int j)
	{
		Eigen::VectorXf myRow = A.row(i);
		Eigen::VectorXf other = A.row(j);

		float dot = 0;
		for (int k = 0; k < myRow.size(); k++)
		{
			if (myRow(k) == myRow(k) && other(k) == other(k))
			{
				dot += myRow(k) * other(k);
			}
		}

		dot /= (float)myRow.size();
		return dot;
	};

	auto round = [&]()
	{
		for (int i = 0; i < A.rows(); i++)
		{
			float bcc = 0; float bi = -1;

			for (int j = 0; j < A.rows(); j++)
			{
				if (i == j)
				{
					continue;
				}
				
				float dot = get_dot(i, j);

				if (dot > bcc)
				{
					bcc = dot; bi = j;
				}
			}

			if (i - bi == 1 || bi - i == 1 || bi == -1)
			{
				continue;
			}
			/*
			else if (bi == 0)
			{
				insert_rows(i, bi + 1);
				return;
			}
			*/
			else if (bi == A.rows() - 1)
			{
				insert_rows(i, bi - 1);
			}
			else
			{
				int left = bi;
				int right = bi + 1;
				
				int ldot = get_dot(i, left);
				int rdot = get_dot(i, right);

				int target = (ldot > rdot ? left : right);
				insert_rows(i, target);
			}
		}
	};
	
	for (int i = 0; i < 5000; i++)
	{
		round();
	}

	auto compose = [](std::vector<int> &perm, const std::vector<int> &order)
	{
		std::vector<int> updated(perm.size());
		for (size_t i = 0; i < order.size(); i++)
		{
			updated[i] = perm[order[i]];
		}
		perm = updated;
	};

	compose(_rowPerm, logicalOrder);
	syncCoupledPerm(true);

	refreshDisplay();
}

std::vector<std::string> MatrixBox::rowNames()
{
	std::vector<std::string> texts(_rowPerm.size());
	for (size_t i = 0; i < _rowPerm.size(); i++)
	{
		texts[i] = _rowNames[_rowPerm[i]];
	}
	return texts;
}

std::vector<std::string> MatrixBox::colNames()
{
	std::vector<std::string> texts(_colPerm.size());
	for (size_t i = 0; i < _colPerm.size(); i++)
	{
		texts[i] = _colNames[_colPerm[i]];
	}
	return texts;
}

