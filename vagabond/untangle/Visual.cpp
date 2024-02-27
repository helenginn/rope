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

#include <vagabond/gui/RenderLine.h>

#include <vagabond/gui/elements/FloatingText.h>
#include <vagabond/core/BondLength.h>
#include <vagabond/core/Atom.h>
#include "TangledBond.h"
#include "Untangle.h"
#include "Visual.h"
#include "Points.h"

Visual::Visual(Untangle *untangle)
{
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/axes.vsh");
	setFragmentShaderFile("assets/shaders/axes.fsh");
	setImage("assets/images/bond.png");
	
	_colours["A"] = glm::vec4(0, 0, 0.9, 0);
	_colours["B"] = glm::vec4(0.5, 0.4, 0, 0);
	_colours[""] = glm::vec4(0.5, 0.5, 0.5, 0);

	_untangle = untangle;
	setup();
	
	_scoreText = new Text("Score: ");
	_scoreText->setLeft(0.1, 0.1);
	addObject(_scoreText);
}

void Visual::addBond(const glm::vec3 &v, const glm::vec3 &w, float score,
                     const glm::vec4 &cl, const glm::vec4 &cr)
{
	std::vector<Vertex> vs = vertices_for_vector(v, w);

	for (Vertex &vert : vs)
	{
		vert.color[3] = score - 1;
	}
	
	vs[0].color += cl;
	vs[1].color += cr;
	vs[2].color += cl;
	vs[3].color += cr;

	_vertices.reserve(_vertices.size() + vs.size());
	_vertices.insert(_vertices.end(), vs.begin(), vs.end());
	add_indices_for_renderable(this);
}

float Visual::updateBond(Bonded *bonded)
{
	Atom *left = bonded->a;
	Atom *right = bonded->b;

	glm::vec3 v = left->conformerPositions()[bonded->p].pos.ave;
	glm::vec3 w = right->conformerPositions()[bonded->q].pos.ave;
	glm::vec3 dir = v - w;

	float score = bonded->tb->simple_score(bonded->p, bonded->q);
	
	for (size_t i = 0; i < 4; i++)
	{
		glm::vec3 &pos = (i % 2 == 0 ? v : w);
		size_t j = i + bonded->idx;

		_vertices[j].pos = pos;
		_vertices[j].normal = dir;
		if (bonded->p == bonded->q)
		{
			_vertices[j].color = glm::vec4(0.f);
		}
		else
		{
			std::string conf = (i % 2 == 0 ? bonded->p : bonded->q);
			_vertices[j].color = colourFor(conf);
		}

		_vertices[j].color[3] = score;
	}
	
	return score;
}

void Visual::updateScore()
{
	float total = _untangle->biasedScore();

	auto fix_text = [total, this]()
	{
		_scoreText->setText("Score: " + std::to_string(total));
	};
	
	addMainThreadJob(fix_text);
}

void Visual::updateBonds()
{
	for (Bonded &bonded : _bonded)
	{
		updateBond(&bonded);
	}

	forceRender(true, true);
	updateScore();
}

void Visual::addBonded(Atom *left, Atom *right, TangledBond *tb,
                       const std::string &p, const std::string &q)
{
	_bonded.push_back({tb, left, right, p, q, _vertices.size()});
	_mapping[left].push_back(&_bonded.back());
	_mapping[right].push_back(&_bonded.back());
}

void Visual::addBond(TangledBond &bond)
{
	BondLength &bl = bond;
	
	Atom *left = bl.atom(0);
	Atom *right = bl.atom(1);
	
	if (left->elementSymbol() == "H" || right->elementSymbol() == "H")
	{
		return;
	}

	OpSet<std::string> lefts = left->conformerList();
	OpSet<std::string> rights = right->conformerList();

	typedef std::pair<std::string, std::string> ConfPair;

	OpSet<ConfPair> pairs = lefts.pairs_with(rights);
	
	for (const std::string &first : lefts)
	{
		for (const std::string &second : rights)
		{
			glm::vec3 v = left->conformerPositions()[first].pos.ave;
			glm::vec3 w = right->conformerPositions()[second].pos.ave;
			
			if (glm::length(v) < 1e-6 || glm::length(w) < 1e-6)
			{
				continue;
			}
			
			float score = bond.simple_score(first, second);
			addBonded(left, right, &bond, first, second);

			if (first[0] == 0 || second[0] == 0 || first == second)
			{
				addBond(v, w, score);
			}
			else
			{
				std::cout << "'" << first << "' '" << second << "'" <<std::endl;
				addBond(v, w, score, _colours[first], _colours[second]);
			}
		}
	}
}

void Visual::labelAtom(Atom *atom)
{
	if (_text)
	{
		removeObject(_text);
		delete _text;
	}

	FloatingText *text = new FloatingText(atom->desc(), 20, 2.0);
	text->setPosition(atom->initialPosition());
	addObject(text);
	_text = text;
}

void Visual::focusOn(int resi)
{
	Atom *atom = _untangle->atomFor(resi);
	labelAtom(atom);
	glm::vec3 pos = atom->initialPosition();
	
	if (_gl)
	{
		_gl->shiftToCentre(pos, 0);
	}
}

void Visual::setupPoints()
{
	Points *points = new Points(this);

	for (Atom *atom : _untangle->atoms())
	{
		if (atom->elementSymbol() != "H")
		{
			points->addAtom(atom);
		}
	}

	_points = points;
}

void Visual::setup()
{
	for (TangledBond &bond : _untangle->bonds())
	{
		addBond(bond);
	}
	
	setupPoints();
	updateBonds();
	_untangle->backgroundUntangle({});
}

void Visual::render(SnowGL *gl)
{
//	glEnable(GL_DEPTH_TEST);
	Renderable::render(gl);
//	glDisable(GL_DEPTH_TEST);
}

