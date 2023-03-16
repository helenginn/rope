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

#ifndef __vagabond__VisualPreferences__
#define __vagabond__VisualPreferences__

#include "ResidueId.h"

#include <json/json.hpp>
using nlohmann::json;

class Atom;
class Instance;

class VisualPreferences
{
public:
	VisualPreferences();
	
	static VisualPreferences ballStickOnly()
	{
		VisualPreferences vp;
		vp._cAlphaTrace = false;
		return vp;
	}

	friend void to_json(json &j, const VisualPreferences &value);
	friend void from_json(const json &j, VisualPreferences &value);
	
	const bool &cAlphaTrace() const
	{
		return _cAlphaTrace;
	}
	
	void setCAlphaTrace(bool trace)
	{
		_cAlphaTrace = trace;
	}

	const bool &ballAndStick() const
	{
		return _ballAndStick;
	}
	
	void setBallAndStick(bool bas)
	{
		_ballAndStick = bas;
	}
	
	const std::vector<ResidueId> &displayBallSticks() const
	{
		return _displayBallSticks;
	}
	
	void addBallStickResidue(const ResidueId &id)
	{
		_displayBallSticks.push_back(id);
	}
	
	void removeBallStickResidue(int i)
	{
		_displayBallSticks.erase(_displayBallSticks.begin() + i);
	}
	
	std::vector<Atom *> selectBallStickAtoms(std::vector<Atom *> &av, 
	                                         Instance *inst = nullptr);
private:
	bool isBallStickAtom(const Atom *a, Instance *inst = nullptr);

	bool _cAlphaTrace = true;
	bool _ballAndStick = true;

	std::vector<ResidueId> _displayBallSticks;
};

inline void to_json(json &j, const VisualPreferences &value)
{
	j["calpha"] = value._cAlphaTrace;
	j["ball_and_stick"] = value._ballAndStick;
	std::vector<ResidueId> residues = value._displayBallSticks;
	j["bas_residues"] = residues;

}

inline void from_json(const json &j, VisualPreferences &value)
{
	value._cAlphaTrace = j["calpha"];
	value._ballAndStick = j["ball_and_stick"];
	std::vector<ResidueId> dbs = j["bas_residues"];
	value._displayBallSticks = dbs;
}

#endif
