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

#ifndef __vagabond__AbWatch__
#define __vagabond__AbWatch__

#include <vagabond/gui/elements/SimplePolygon.h>
#include <vagabond/gui/elements/IndexResponder.h>

class AbWatch : public IndexResponder
{
public:
	AbWatch(const SimplePolygon &other, const unsigned int &version_track, 
	        std::mutex *mut, const glm::vec3 &pos, const std::string &name);

	void interacted(int idx, bool hover, bool left);

	virtual size_t requestedIndices()
	{
		return _vertices.size();
	}
	
	const std::string &name() const
	{
		return _name;
	}

	void highlight();
	virtual void reindex();
	virtual void render(GLView *gl);
private:
	unsigned int _currVersion = 0;
	const unsigned int &_versionTrack;
	std::mutex *_mut{};
	const glm::vec3 &_pos;
	const std::string _name;
	glm::vec3 _last;

	void doThings();
};

#endif


