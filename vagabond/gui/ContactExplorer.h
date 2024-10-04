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

#ifndef __vagabond__ContactExplorer__
#define __vagabond__ContactExplorer__

#include <mutex>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/core/paths/Contacts.h>
#include <vagabond/core/TabulatedData.h>
#include <vagabond/gui/elements/Scene.h>

class MatrixPlot;

class ContactExplorer : public Scene
{
public:
	ContactExplorer(Scene *prev, Contacts &contact);

	void mousePressEvent(double x, double y, SDL_MouseButtonEvent button);
	virtual void setup();
private:
	Contacts _contacts;

	PCA::Matrix _matrix{};
	MatrixPlot *_plot;
	std::mutex _mutex;
	TabulatedData *_data = nullptr;
	int _start{};
	int _end{};
};

#endif
