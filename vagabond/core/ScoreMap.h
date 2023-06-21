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

#ifndef __vagabond__ScoreMap__
#define __vagabond__ScoreMap__

#include <vagabond/core/Responder.h>
#include <vagabond/core/CompareDistances.h>

class Atom;
class SpecificNetwork;
template <typename Type> class Mapped;

typedef std::vector<std::vector<float>> Points;

class ScoreMap : public Responder<SpecificNetwork>
{
public:
	ScoreMap(Mapped<float> *mapped, SpecificNetwork *specified);
	~ScoreMap();

	typedef std::function<bool(Atom *const &atom)> AtomFilter;

	void setIndividualHandler(std::function<void(float, std::vector<float> &)> f)
	{
		_eachHandler = f;
	}

	float scoreForPoints(const Points &points);
	
	void setFilters(AtomFilter &left, AtomFilter &right);
	
	bool hasMatrix();
	
	std::vector<Atom *> atoms();
	
	PCA::Matrix matrix();
	
	enum Mode
	{
		Unset,
		Basic,
		Display,
		Distance,
		AssessSplits,
	};

	void setMode(Mode mode)
	{
		_mode = mode;
	}
protected:
	virtual void sendObject(std::string tag, void *object);
private:
	float processPoints(const Points &points, bool make_aps);

	float submitJobs(const Points &points, bool make_aps);

	Mapped<float> *_mapped = nullptr;
	SpecificNetwork *_specified = nullptr;

	CompareDistances _comparer;

	std::function<void(float, std::vector<float> &)> _eachHandler;
	
	bool modeNeedsAPS(const Mode &mode)
	{
		return (mode == Distance || mode == AssessSplits || mode == Display);
	}
	
	Mode _mode = Unset;
};

#endif
