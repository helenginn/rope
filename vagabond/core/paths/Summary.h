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

#ifndef __vagabond__Summary__
#define __vagabond__Summary__

#include <string>
#include <vagabond/core/Responder.h>
#include <vagabond/gui/elements/SimplePolygon.h>

class PathTask;
class Text;
class Image;

class Summary : public SimplePolygon, public HasResponder<Responder<Summary *> >
{
public:
	Summary(PathTask *top);

	std::string text();
	
	void setup();
	void finishPathFinding();
private:
	PathTask *_top = nullptr;

	struct TickLine
	{
		Text *text;
		Image *running;
		Image *tick;
		Image *cross;
		
		void disableAll();
	};

	Image *prepareImage(std::string image, float top);
	TickLine makeLine(float top, std::string words);
	
	TickLine _pathFinding;
	TickLine _warping;
};

#endif
