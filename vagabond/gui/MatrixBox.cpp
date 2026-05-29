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

#include <climits>
#include <vagabond/utils/OpSet.h>
#include "MatrixBox.h"
#include <vagabond/gui/MatrixPlot.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Window.h>

MatrixBox::MatrixBox(MatrixPlot *mp, const std::vector<std::string> &rowNames,
                     const std::vector<std::string> &colNames,
                     bool reorder)
: _plot(mp), _rowNames(rowNames), _colNames(colNames)
{
	_identical = (&rowNames == &colNames);
	std::reverse(_colNames.begin(), _colNames.end());

	_plot->setCentre(0.0, 0.0);
	addObject(_plot);

	if (reorder)
	{
		guessReordering();
	}
	draw();
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

void MatrixBox::draw()
{

	float width = _plot->maximalWidth() / 2.f;
	float height = _plot->maximalHeight() / 2.f;
	float xstep = width / (float)_colNames.size();
	float ystep = -height / (float)_rowNames.size();
	float x = -width / 2 + xstep / 2;
	float y = +height / 2 + ystep / 2;
	
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
				Eigen::MatrixXf A = _plot->_mat.toEigen();
				Eigen::VectorXi perm = Eigen::Map<Eigen::VectorXi>(order.data(), 
				                                                   order.size());

				Eigen::PermutationMatrix<Eigen::Dynamic, Eigen::Dynamic> P(perm);
				Eigen::MatrixXf pA;
				if (_identical)
				{
					pA = P.transpose() * A * P;
				}
				else if (coord == 1) // buttons on y -> row swaps
				{
					std::cout << "Implement non-square matrix permutations pls" << std::endl;
				}
				else if (coord == 0)
				{
					std::cout << "Implement non-square matrix permutations pls" << std::endl;

				}
				A = pA;
				_plot->_mat.dropFromEigen(A);
				_plot->update();

				save_current_pos_info();
				_order = tmp;
			}
			
		};
	};
	
	Renderable::Alignment row_align = Renderable::Alignment::Right;
	Renderable::Alignment col_align = Renderable::Alignment
	(Renderable::Alignment::Centre | Renderable::Alignment::Bottom);
	
	std::vector<std::pair<TextButton *, TextButton *>> pairs;

	for (const std::string &first : _rowNames)
	{
		TextButton *t = new TextButton(first);
		t->resize(0.3);
		t->setArbitrary(-width / 2, y, row_align);
		t->setDragFunction(drag_button(t, 1));
		addObject(t);
		y += ystep;
		if (_identical)
		{
			_indices[pairs.size()] = t;
			pairs.push_back({t, nullptr});
		}
	}

	int n = 0;
	for (const std::string &first : _colNames)
	{
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
		if (_identical)
		{
			pairs[n].second = t; n++;
		}
	}
	
	_couples = std::map<TextButton *, TextButton *>(pairs.begin(), pairs.end());
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

	Eigen::MatrixXf A = _plot->_mat.toEigen();
	OpSet<int> row_list;
	for (int i = 0; i < A.rows(); i++)
	{
		row_list += i;
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
		
		insertion(_rowNames, i, j);
		
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

	_colNames = _rowNames;
	std::reverse(_colNames.begin(), _colNames.end());

	_plot->_mat.dropFromEigen(A);
	_plot->update();
}

std::vector<std::string> MatrixBox::names(int coord)
{
	auto create_order = create_order_function(1);
	std::set<OrderedButton> ordered = create_order();
	
	std::vector<std::string> texts; texts.reserve(ordered.size());
	for (const OrderedButton &button  : ordered)
	{
		texts.push_back(button.second->text());
	}
	return texts;
}

std::vector<std::string> MatrixBox::rowNames()
{
	return names(1);
}

std::vector<std::string> MatrixBox::colNames()
{
	return names(0);
}

