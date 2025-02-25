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

#ifndef __vagabond__Visual__
#define __vagabond__Visual__

#include <list>
#include <vagabond/gui/elements/SimplePolygon.h>

class FloatingText;
class TangledBond;
class BondLength;
class Untangle;
class Points;
class Atom;
class Text;

class Visual : public SimplePolygon
{
public:
	Visual(Untangle *untangle);

	void addBond(TangledBond &bond);
	void addBond(const glm::vec3 &v, const glm::vec3 &w, float score,
	             const glm::vec4 &cl = {}, const glm::vec4 &cr = {});
	
	virtual void render(SnowGL *gl);
	void setup();
	void updateBonds();
	void updateScore();
	
	void undo();
	void redo();

	Points *points()
	{
		return _points;
	}
	
	std::function<void(Atom *, const std::string &, float)> updateBadness();
	
	glm::vec4 colourFor(const std::string &conf) 
	{
		return _colours[conf];
	}

	// taking into account the other conformer in the pair
	glm::vec4 colourForConfs(const std::string &conf1, 
	                         const std::string &conf2,
	                         const std::string &chosen);
	
	Untangle *const &untangle() const
	{
		return _untangle;
	}

	void labelAtom(Atom *atom);
	void focusOn(int resi);
	virtual void extraUniforms();
	void findDisulphides();
	
	void setShowDirt(bool dirt)
	{
		_showDirt = dirt;
	}

	void clearBadness();
private:
	void setupPoints();
	void addBonded(Atom *left, Atom *right, TangledBond *tb,
	               const std::string &p, const std::string &q);

	Untangle *_untangle = nullptr;

	std::map<std::string, glm::vec4> _colours;
	Points *_points = nullptr;
	
	struct Bonded
	{
		TangledBond *tb;
		Atom *a; 
		Atom *b;

		std::string p;
		std::string q;
		
		bool isIdentity()
		{
			return p == q;
		}
		
		size_t idx;
	};
	
	std::list<Bonded> _bonded;

	int _showDirt = 0;
	float biasedScore();
	float updateBond(Bonded *bonded);
	FloatingText *_text = nullptr;
	Text *_scoreText = nullptr;
	Text *_clashText = nullptr;
	
	std::map<TangledBond *, Text *> _disulphides;

	std::map<Atom *, std::vector<Bonded *>> _mapping;
};

#endif
