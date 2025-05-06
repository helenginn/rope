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

#ifndef __vagabond__TableView__
#define __vagabond__TableView__

#include <vagabond/gui/elements/ListView.h>
#include <vagabond/gui/ExportsCSV.h>

class Box;
class TabulatedData;

class TableView : public ListView, public ExportsCSV
{
public:
	TableView(Scene *prev, TabulatedData *data, const std::string &title);

	virtual void setup();
	virtual void refresh();

	virtual size_t lineCount();
	virtual Renderable *getLine(int i);

	virtual float leftMargin()
	{
		return 0.1;
	}

	virtual size_t unitsPerPage();

	virtual void buttonPressed(std::string tag, Button *button = nullptr);
	
	TabulatedData *const &data() const
	{
		return _data;
	}

	float resizing();
	void addPlusSign();
private:

	Box *makeTextBoxes(TableView *view, std::vector<std::string> strings,
	                   TabulatedData *data);

	virtual void supplyCSV(std::string indicator = "");
	void displayHeaders();
	void addMenu();
	
	struct MakeGraph
	{
		void operator()(TableView *tv);

		std::string dependent;
		std::string independent;
	};
	
	MakeGraph _makeGraph{};

	TabulatedData *_data;

	std::string _title;
};

#endif
