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

#ifndef __vagabond__ProgressBar__
#define __vagabond__ProgressBar__

#include <vagabond/gui/elements/Image.h>
#include <vagabond/core/Progressor.h>
#include <vagabond/core/Responder.h>

class ProgressBar : public Image,
public Responder<Progressor>
{
public:
	ProgressBar(std::string text = "");
	~ProgressBar();

	void finish();
	void setMaxTicks(int count);
	virtual void extraUniforms();
	virtual void sendObject(std::string tag, void *object);
private:
	int _maxTicks = 0;
	int _ticks = 0;

};

#endif
