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

#ifndef __vagabond__PathPlot__
#define __vagabond__PathPlot__

#include "MatrixPlot.h"
#include "Responder.h"
#include <mutex>

class Monitor;
class Image;

class PathPlot : public MatrixPlot, public Responder<Monitor>
{
public:
	PathPlot(Monitor *monitor);

	virtual void sendObject(std::string tag, void *object);
	
protected:
	void makeImage(int idx, glm::vec3 pos, std::string image);
	void clearImage(int idx);
private:
	Monitor *_monitor = nullptr;

	std::mutex _mutex;
	std::map<int, Image *> _images;
};

#endif
