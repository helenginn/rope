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
private:
	MatrixPlot *_plot{};

	std::function<void(bool)> _enableButtons;

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
	std::map<int, TextButton *> _indices;
};

#endif
