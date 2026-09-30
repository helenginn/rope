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
#include <functional>
#include <mutex>

class AbWatch : public IndexResponder
{
public:
	// where to display this copy of an antibody: fills in the position to
	// show and returns a number which differs from the last call whenever
	// that position (or where it comes from) has changed
	typedef std::function<unsigned int(glm::vec3 &)> Source;

	AbWatch(const SimplePolygon &other, const Source &source, 
	        std::mutex *mut, const std::string &name);

	void interacted(int idx, bool hover, bool left);

	virtual size_t requestedIndices()
	{
		return _vertices.size();
	}
	
	const std::string &name() const
	{
		return _name;
	}

	// the antibody this is a copy of (name() also carries the copy number)
	void setAntibody(const std::string &ab, int copy)
	{
		_antibody = ab;
		_copy = copy;
	}

	// the hover job says whether this copy should go on to highlight itself
	void setHoverJob(const std::function<bool(const std::string &, int)> &job)
	{
		_hoverJob = job;
	}

	void setClickJob(const std::function<void(AbWatch *)> &job)
	{
		_clickJob = job;
	}

	const std::string &antibody() const
	{
		return _antibody;
	}

	int copy() const
	{
		return _copy;
	}

	// all one colour, as a key to something else, until setFixedStyle()
	void showColour(const glm::vec4 &colour);

	// free antibodies are blue and selectable; fixed ones keep their own colour
	void setFixedStyle(bool fixed);

	// highlights this copy without giving it a label
	void highlightQuietly();

	// The radius of the sphere, as made.
	float radius() const
	{
		return _radius;
	}

	// where the sphere is shown relative to the position it stands for, which
	// may depend on where that is (none if this is empty)
	void setDisplace(const std::function<glm::vec3(const glm::vec3 &)> &fn)
	{
		_displace = fn;
		_started = false;
	}

	void highlight();
	virtual void reindex();
	virtual void render(GLView *gl);
private:
	unsigned int _currVersion = 0;
	bool _started = false;
	float _radius{};
	std::function<glm::vec3(const glm::vec3 &)> _displace;
	Source _source;
	std::mutex *_mut{};
	const std::string _name;
	std::string _antibody;
	glm::vec4 _baseColour{};
	int _copy{};
	std::function<bool(const std::string &, int)> _hoverJob;
	std::function<void(AbWatch *)> _clickJob;
	glm::vec3 _last;

	void doThings();
};

#endif


