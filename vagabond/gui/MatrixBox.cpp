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

#include "MatrixBox.h"
#include <vagabond/gui/MatrixPlot.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Window.h>

MatrixBox::MatrixBox(MatrixPlot *mp, const std::vector<std::string> &rowNames,
                     const std::vector<std::string> &colNames)
: _plot(mp), _rowNames(rowNames), _colNames(colNames)
{
	mp->setCentre(0.0, 0.0);
	addObject(mp);
	_identical = (&rowNames == &colNames);
	std::reverse(_colNames.begin(), _colNames.end());

	float width = mp->maximalWidth() / 2.f;
	float height = mp->maximalHeight() / 2.f;
	float xstep = width / (float)colNames.size();
	float ystep = -height / (float)rowNames.size();
	float x = -width / 2 + xstep / 2;
	float y = +height / 2 + ystep / 2;
	
	auto swap_matrix_row_or_col = [this](int i, int j, int coord)
	{
		Eigen::MatrixXf A = _plot->_mat.toEigen();
		std::cout << "Before: " << std::endl;
		std::cout << A << std::endl;
		
		if (coord == 1 && !_identical)
		{
			A.row(i).swap(A.row(j));
		}
		else if (coord == 0 && !_identical)
		{
			A.col(i).swap(A.col(j));
		}
		else if (_identical)
		{
			A.row(i).swap(A.row(j));
			A.col(i).swap(A.col(j));
		}
		
		std::cout << "After: " << std::endl;
		std::cout << A << std::endl;
		
		if (_plot->_mutex)
		{
			std::unique_lock<std::mutex> lock(*(_plot->_mutex));
			_plot->_mat.dropFromEigen(A);
			_plot->update();
		}
		else
		{
			_plot->_mat.dropFromEigen(A);
			_plot->update();
		}
	};
	
	auto drag_button = [this, swap_matrix_row_or_col](TextButton *tb, int coord)
	{
		_info[tb] = {};
		_info[tb].coord = coord;
		
		typedef std::pair<float, TextButton *> OrderedButton;

		auto create_order = [this, coord]()
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
			glm::vec3 curr = tb->centroid();
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

		return [this, tb, create_order, swap_required, swap_matrix_row_or_col,
		        steal_coordinate_from_other, save_current_pos_info, coord]
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
			int n = 0;
			bool changed = false;
			
			for (auto &pair : tmp)
			{
				TextButton *current = pair.second;
				TextButton *old = it->second;
				if (current != old)
				{
					steal_coordinate_from_other(current, old);
					changed = true;
					std::pair<int, int> swap = swap_required(_order, tmp, 
					                                         current);
					std::cout << "Swap " << swap.first << " to " << 
					swap.second << std::endl;
					if (swap.first > swap.second)
					{
						swap_matrix_row_or_col(swap.first, swap.second, coord);
					}
				}
				it++;
			}

			if (changed)
			{
				save_current_pos_info();
				_order = tmp;
			}
			
		};
	};
	
	Renderable::Alignment row_align = Renderable::Alignment::Right;
	Renderable::Alignment col_align = Renderable::Alignment
	(Renderable::Alignment::Left | Renderable::Alignment::Bottom);
	
	std::vector<std::pair<TextButton *, TextButton *>> pairs;

	for (const std::string &first : _rowNames)
	{
		TextButton *t = new TextButton(first);
		t->resize(0.4);
		t->setArbitrary(-width / 2, y, row_align);
		t->setDragFunction(drag_button(t, 1));
		addObject(t);
		y += ystep;
		if (_identical)
		{
			pairs.push_back({t, nullptr});
		}
	}

	int n = 0;
	for (const std::string &first : _colNames)
	{
		TextButton *t = new TextButton(first);
		t->resize(0.4);
		t->setDragFunction(drag_button(t, 0));
		t->setArbitrary(x, -height / 2, col_align);
		glm::mat4x4 base = glm::mat3x3(1.f);
		base[1] /= Window::aspect();
		glm::mat3x3 rot;
		rot = glm::mat3x3(glm::rotate(base, (float)deg2rad(-45),
		                              glm::vec3(0., 0., -1.)));
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

