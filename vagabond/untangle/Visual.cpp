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
#include <vagabond/utils/FileReader.h>
#include <vagabond/core/BondLength.h>
#include <vagabond/core/Atom.h>
#include "TangledBond.h"
#include "UndoStack.h"
#include "Untangle.h"
#include "Visual.h"
#include "Points.h"

Visual::Visual(Untangle *untangle)
{
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/untangle_axes.vsh");
	setFragmentShaderFile("assets/shaders/untangle_axes.fsh");
	setImage("assets/images/bond.png");
	
	_colours["A"] = glm::vec4(0, 0, 0.9, 0);
	_colours["B"] = glm::vec4(0.5, 0.4, 0, 0);
	_colours[""] = glm::vec4(0.0, 0.0, 0.0, 0);

	_untangle = untangle;
	setup();
	
	_scoreText = new Text("except S-S: ");
	_scoreText->setLeft(0.1, 0.1);
	_scoreText->resize(0.4);
	addObject(_scoreText);

	_clashText = new Text("Clash: ");
	_clashText->setLeft(0.1, 0.14);
	_clashText->resize(0.4);
	addObject(_clashText);
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

glm::vec4 Visual::colourForConfs(const std::string &conf1,
                                 const std::string &conf2,
                                 const std::string &chosen)
{
	if (conf1 == "" || conf2 == "" || conf1 == conf2)
	{
		return _colours[""];
	}

	return _colours[chosen];
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
		std::string chosen = (i % 2 == 0 ? bonded->p : bonded->q);

		_vertices[j].color = colourForConfs(bonded->p, bonded->q, chosen);
		_vertices[j].color[3] = score;
	}
	
	return score;
}

void Visual::updateScore()
{
	float ss = _untangle->biasedScore(false);
	float no_ss = _untangle->biasedScore(true);
	float clash = _untangle->clashScore();

	auto fix_text = [ss, no_ss, clash, this]()
	{
		_scoreText->setText("Not S-S score: " + std::to_string(no_ss));
		_clashText->setText("Clash: " + std::to_string(clash));
		
		for (auto it = _disulphides.begin(); it != _disulphides.end(); it++)
		{
			TangledBond *bond = it->first;
			float score = bond->total_score(1.f);
			std::string str_score = f_to_str(score, 2);
			std::string resi_left = bond->atom(0)->residueId().str();
			std::string resi_right = bond->atom(1)->residueId().str();
			
			Text *text = it->second;
			text->setText(resi_left + "-S " + str_score + " S-" + resi_right);
		}
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

			if (first == "" || second == "" || first == second)
			{
				addBond(v, w, score);
			}
			else
			{
				glm::vec4 lCol = _colours[first];
				glm::vec4 rCol = _colours[second];
				addBond(v, w, score, lCol, rCol);
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
		_text = nullptr;
	}
	
	if (atom == nullptr)
	{
		return;
	}

	FloatingText *text = new FloatingText(atom->desc(), 20, 2.0);
	text->setPosition(atom->initialPosition());
	addObject(text);
	_text = text;
}

bool Visual::focusOn(std::string chain, int resi)
{
	Atom *atom = _untangle->atomFor(chain, resi);
	if (!atom)
	{
		return false;
	}
	labelAtom(atom);
	glm::vec3 pos = atom->initialPosition();
	
	if (_gl)
	{
		_gl->shiftToCentre(pos, 0);
	}
	return true;
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
}

void Visual::render(SnowGL *gl)
{
	Renderable::render(gl);
}

void Visual::undo()
{
	_points->stack()->undo();
}

void Visual::redo()
{
	_points->stack()->redo();
}

void Visual::clearBadness()
{
	for (Vertex &v : _vertices)
	{
		v.extra[0] = 0;
	}
}

std::function<void(Atom *, const std::string &, float)> Visual::updateBadness()
{
	return [this](Atom *atom, const std::string &conf, float badness)
	{
		// we aren't displaying hydrogens so they need to be foisted onto
		// the closest bond
		if (atom->elementSymbol() == "H" && atom->bondLengthCount() == 1)
		{
			atom = atom->connectedAtom(0);
		}

		float bad = (exp(-badness * badness / 2.f) - 1);
		auto bondeds = _mapping[atom];

		for (Bonded *bonded : bondeds)
		{
			if (bonded->p == conf && bonded->q == conf)
			{
				int idx = bonded->idx;
				int offset = (bonded->a == atom) ? 0 : 1;

				for (int i = 0; i <= 2; i += 2)
				{
					Vertex &v = _vertices[idx + offset + i];
					v.extra[0] += bad;
				}
			}
		}
		
		forceRender(true, true);
	};
}

void Visual::extraUniforms()
{
	const char *uniform_name = "show_dirt";
	GLuint u = glGetUniformLocation(_program, uniform_name);
	glUniform1i(u, _showDirt);
}

void Visual::findDisulphides()
{
	float top = 0.2;
	for (Bonded &bonded : _bonded)
	{
		if (_disulphides.count(bonded.tb) > 0)
		{
			continue;
		}

		if (bonded.a->elementSymbol() == "S" &&
		    bonded.b->elementSymbol() == "S")
		{
			Text *text = new Text("disulphide");
			text->setLeft(0.1, top);
			text->resize(0.4);
			addObject(text);

			_disulphides[bonded.tb] = text;
			top += 0.04;
		}
	}

}
