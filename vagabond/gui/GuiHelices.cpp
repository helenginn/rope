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

#include "GuiHelices.h"
#include <vagabond/core/AtomGroup.h>
#include <vagabond/utils/maths.h>
#include <vagabond/core/BondLength.h>

GuiHelices::GuiHelices(GuiAtom *parent) : GuiRepresentation(parent)
{
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/pencil_shading.fsh");
	std::string filename = "assets/images/pencil_shading.png";
	setImage(filename, true);

}

GuiHelices::~GuiHelices()
{

}

void GuiHelices::watchAtom(Atom *atom)
{
	if (atom->atomName() == "N")
	{
		_ns.push_back(atom);
	}
	else if (atom->atomName() == "O")
	{
		_os.push_back(atom);
	}
}

void GuiHelices::watchAtomGroup(AtomGroup *ag)
{
	for (Atom *atom : ag->atomVector())
	{
		watchAtom(atom);
	}
	
	populateHelices();
}

std::vector<GuiHelices::NOPair> GuiHelices::sweepCandidates()
{
	std::vector<NOPair> noPairs;
	float sql_max = 4.0 * 4.0;
	
	for (size_t i = 0; i < _ns.size(); i++)
	{
		Atom *n = _ns[i];
		const glm::vec3 &npos = n->initialPosition();

		for (size_t j = 0; j < _os.size(); j++)
		{
			Atom *o = _os[j];
			
			if (n->residueNumber() == o->residueNumber())
			{
				continue;
			}

			const glm::vec3 &opos = o->initialPosition();
			glm::vec3 diff = opos - npos;
			float sql = glm::dot(diff, diff);
			
			if (sql > sql_max)
			{
				continue;
			}
			
			NOPair pair{n, o};
			noPairs.push_back(pair);
		}
	}

	return noPairs;
}

void GuiHelices::pruneCandidates(std::vector<NOPair> &pairs)
{
	std::vector<NOPair> filtered;
	for (NOPair &pair : pairs)
	{
		int separation = pair.n->bondsBetween(pair.o, 15);
		
		if (separation == 11)
		{
			filtered.push_back(pair);
		}
	}

	pairs = filtered;
}

void GuiHelices::extendRun(Run &run, std::vector<NOPair> &pairs)
{
	for (size_t i = 0; i < pairs.size(); i++)
	{
		int to_begin = run.front().n->bondsBetween(pairs[i].n, 5);
		int to_end = run.back().n->bondsBetween(pairs[i].n, 5);
		
		if (to_begin < 0 && to_end < 0)
		{
			continue;
		}

		if (to_begin > 0)
		{
			run.insert(run.begin(), pairs[i]);
		}
		else if (to_end > 0)
		{
			run.push_back(pairs[i]);
		}

		pairs.erase(pairs.begin() + i);
		i--;
	}
}

std::vector<GuiHelices::Run> 
GuiHelices::findNitrogenRuns(std::vector<NOPair> &pairs)
{
	std::vector<std::vector<NOPair> > runs;

	for (int i = 0; i < (int)pairs.size() - 1; i++)
	{
		for (int j = i + 1; j < (int)pairs.size(); j++)
		{
			int separation = pairs[i].n->bondsBetween(pairs[j].n, 5);

			if (separation < 0)
			{
				continue;
			}
			
			Run run;
			
			NOPair first = pairs[j];
			pairs.erase(pairs.begin() + j);

			NOPair second = pairs[i];
			pairs.erase(pairs.begin() + i);
			
			run.push_back(second);
			run.push_back(first);
			
			extendRun(run, pairs);
			runs.push_back(run);
			i = 0; j = -1;
			break;
		}
	}

	return runs;
}

Atom *GuiHelices::jumpResidueOtherName(Atom *atom, std::string other, int jump)
{
	std::set<Atom *> rejected;
	std::vector<BondNum> bnums;
	bnums.push_back({atom, 6});
	rejected.insert(atom);

	while (bnums.size())
	{
		BondNum bn = bnums.back();
		bnums.pop_back();

		if (bn.atom->atomName() == other && 
		    bn.atom->residueNumber() == atom->residueNumber() + jump)
		{
			return bn.atom;
		}

		if (bn.num == 0)
		{
			continue;
		}

		for (size_t i = 0; i < bn.atom->bondLengthCount(); i++)
		{
			BondLength *bl = bn.atom->bondLength(i);
			Atom *other = bl->otherAtom(bn.atom);	

			if (rejected.count(other))
			{
				continue;
			}

			rejected.insert(other);
			BondNum newBn{other, bn.num - 1};
			bnums.push_back(newBn);
		}
	}

	return nullptr;
}

void GuiHelices::ensureDirection(std::vector<Run> &runs, bool asc)
{
	for (Run &run : runs)
	{
		std::sort(run.begin(), run.end());
		if (!asc)
		{
			std::reverse(run.begin(), run.end());
		}
	}

}

std::vector<GuiHelices::Helix> 
GuiHelices::convertToHelices(std::vector<Run> &runs)
{
	std::vector<Helix> results;

	for (Run &run : runs)
	{
		Helix helix;
		helix.run = run;
		
		for (NOPair &pair : run)
		{
			Atom *nitrogen = pair.n;

			Atom *cAlpha = nullptr;
			for (size_t i = 0; i < nitrogen->bondLengthCount(); i++)
			{
				BondLength *bl = nitrogen->bondLength(i);
				Atom *other = bl->otherAtom(nitrogen);	
				if (other->atomName() == "CA")
				{
					cAlpha = other;
					break;
				}
			}

			helix.cAlphas.push_back(cAlpha);
		}
		
		int start = helix.run.size() - 4;
		
		for (size_t i = start; i < start + 3; i++)
		{
			NOPair &pair = helix.run[i];
			Atom *cAlpha = jumpResidueOtherName(pair.o, "CA", 0);
			if (cAlpha)
			{
				helix.run.push_back(helix.run.back());
				helix.cAlphas.push_back(cAlpha);
			}
		}
		
		Atom *last_ca = helix.cAlphas.back();
		Atom *next = last_ca;
		
		for (size_t i = 0; i < 2; i++)
		{
			next = jumpResidueOtherName(next, "CA", -1);
			if (!next)
			{
				break;
			}
			helix.cAlphas.push_back(next);
			helix.run.push_back(helix.run.back());
		}
		
		results.push_back(helix);
	}

	return results;
}

void GuiHelices::populateMapFromHelices()
{
	_cAlphaMap.clear();
	
	int hnum = 0;

	for (Helix &helix : _helices)
	{
		for (Atom *cAlpha : helix.cAlphas)
		{
			if (cAlpha)
			{
				_cAlphaMap[cAlpha] = hnum;
			}
		}
		
		hnum++;
	}
}

void GuiHelices::cAlphasToWireFrame(Helix &h, std::vector<Snow::Vertex> &vs,
                                    std::vector<glm::vec3> &helix_dirs)
{
	for (int i = 0; i < (int)h.cAlphas.size(); i++)
	{
		Atom *ca = h.cAlphas[i];
		
		if (ca)
		{
			glm::vec3 pos = ca->derivedPosition();
			float prop = ca->addedColour();
			glm::vec4 c = _scheme->colour(prop);

			vs.push_back(new_vertex(pos));
			vs.back().color = c;
		}
		else
		{
			vs.push_back(new_vertex(glm::vec3(NAN)));
		}
		
		glm::vec3 ave{};
		for (int j = -2; j <= 1; j++)
		{
			int k = i + j;
			if (k < 0)
			{
				k = 0;
			}

			if (k >= h.run.size())
			{
				k = h.run.size() - 1;
			}

			const glm::vec3 &npos = h.run[k].n->derivedPosition();
			const glm::vec3 &opos = h.run[k].o->derivedPosition();

			glm::vec3 diff = glm::normalize(opos - npos);
			
			ave += diff;
		}

		ave = glm::normalize(ave);
		helix_dirs.push_back(ave);
	}
}

std::vector<glm::vec3> interpolatedHelixDirs(std::vector<glm::vec3> &dirs)
{
	std::vector<glm::vec3> interpolated_dirs;
	
	for (size_t i = 0; i < dirs.size() - 1; i++)
	{
		float step = 1 / (float)POINTS_PER_BEZIER;
		float sum = 0;
		for (size_t j = 0; j < POINTS_PER_BEZIER; j++)
		{
			glm::vec3 calc = dirs[i] * sum;
			int prev = (i == 0 ? 0 : i - 1);
			calc += dirs[prev] * (1 - sum);
			calc = glm::normalize(calc);

			interpolated_dirs.push_back(calc);
			sum += step;
		}
	}

	return interpolated_dirs;
}

const glm::vec2 helix_cross_section[] = 
{
	{-0.2, +1.0},
	{+0.0, +1.2},
	{+0.2, +1.0},
	{+0.4, +0.0},
	{+0.2, -1.0},
	{-0.0, -1.2},
	{-0.2, -1.0},
	{-0.4, -0.0}
};

const glm::vec2 helix_normals[] =
{
	{-0.50, +0.3},
	{-0.0, +1.0},
	{+0.50, +0.3},
	{+1.0, +0.0},
	{+0.50, -0.3},
	{-0.0, -1.0},
	{-0.50, -0.3},
	{-1.0, +0.0}
};

void GuiHelices::makeHelixSlab(Snow::Vertex &origin, Snow::Vertex &previous,
                               glm::vec3 dir,
                               std::vector<Snow::Vertex> &verts,
                               std::vector<GLuint> &indices, float taper)
{
	const int total = 8;
	glm::vec3 cross = glm::vec3(0.f);
	if (previous.pos.x != NAN)
	{
		glm::vec3 diff = origin.pos - previous.pos;
		cross = glm::normalize(glm::cross(diff, dir));
	}
	
	int j = 0;
	for (int i = verts.size() - total; i < verts.size(); i++)
	{
		glm::vec3 norm = helix_normals[j][0] * cross;
		norm += helix_normals[j][1] * dir;
		verts[i].normal += glm::normalize(norm);
		j++;
	}
	
	for (size_t i = 0; i < total; i++)
	{
		glm::vec3 sum = helix_cross_section[i][0] * cross;
		sum += helix_cross_section[i][1] * dir * taper;
		sum += origin.pos;

		Vertex v = new_vertex(sum);

		glm::vec3 norm = helix_normals[i][0] * cross;
		norm += helix_normals[i][1] * dir;
		v.normal = glm::normalize(norm);
		v.color = origin.color;

		verts.push_back(v);
	}

	if (previous.pos.x == previous.pos.x)
	{
		addCylinderIndices(verts, indices, total);
	}
}

void GuiHelices::extraUniforms()
{
	const char *uniform_name = "dot_threshold";
	GLuint u = glGetUniformLocation(_program, uniform_name);
	float dot = -2.0;
	glUniform1f(u, dot);
	checkErrors("rebinding threshold");
}

void GuiHelices::makeHelixSlabs(std::vector<Snow::Vertex> &bez,
                               std::vector<glm::vec3> &dirs,
                               std::vector<Snow::Vertex> &verts,
                               std::vector<GLuint> &indices)
{
	Snow::Vertex nan_vert = new_vertex(glm::vec3(NAN));
	for (size_t i = 0; i < bez.size(); i++)
	{
		float taper = 1.0;
		
		if (i < 10)
		{
			taper = float(i) / 10.;
		}
		else if (i > bez.size() - 11)
		{
			taper = (float)(bez.size() - i) / 10.;
		}
		
		if (i == 0)
		{
			makeHelixSlab(bez[i], nan_vert, dirs[i], verts, indices, taper);
		}
		else
		{
			makeHelixSlab(bez[i], bez[i - 1], dirs[i], verts, indices, taper);
		}
	}

}

void GuiHelices::drawHelix(Helix &h, std::vector<Snow::Vertex> &verts,
                           std::vector<GLuint> &indices)
{
	std::vector<Snow::Vertex> vs;
	std::vector<glm::vec3> helix_dirs;

	cAlphasToWireFrame(h, vs, helix_dirs);

	std::vector<Snow::Vertex> bez;
	bez = wireFrameToBezier(vs, 2);
	
	std::vector<glm::vec3> smooth_dirs = interpolatedHelixDirs(helix_dirs);
	
	makeHelixSlabs(bez, smooth_dirs, verts, indices);
}

void GuiHelices::mergeHelices()
{
	for (size_t i = 1; i < _helices.size(); i++)
	{
		Helix &curr = _helices[i];
		Helix &prev = _helices[i - 1];
		
		Atom *last = prev.cAlphas.back();
		Atom *first = curr.cAlphas.front();
		
		if (!last || !first)
		{
			continue;
		}

		int nbonds = last->bondsBetween(first, 4);
		
		if (nbonds >= 0)
		{
			prev.run.reserve(prev.run.size() + curr.run.size());
			prev.cAlphas.reserve(prev.cAlphas.size() + curr.cAlphas.size());
			
			prev.run.insert(prev.run.end(), curr.run.begin(), curr.run.end());
			prev.cAlphas.insert(prev.cAlphas.end(), 
			                    curr.cAlphas.begin(), curr.cAlphas.end());
			
			_helices.erase(_helices.begin() + i);
			i--;
		}
	}
}

void GuiHelices::purgeHelices()
{
	for (size_t i = 0; i < _helices.size(); i++)
	{
		if (_helices[i].cAlphas.size() <= 4)
		{
			_helices.erase(_helices.begin() + i);
			i--;
		}
	}
}

void GuiHelices::populateHelices()
{
	/* find all possible N...O backbone hydrogen bonds */
	std::vector<NOPair> noPairs = sweepCandidates();
	
	/* get rid of any which are greater than 12 atoms apart */
	pruneCandidates(noPairs);
	
	/* find runs of consecutive nitrogen atoms */
	std::vector<Run > runs = findNitrogenRuns(noPairs);

	ensureDirection(runs, false);
	_helices = convertToHelices(runs);
	mergeHelices();
	purgeHelices();
	populateMapFromHelices();
	
	for (int i = 0; i < _helices.size(); i++)
	{
		Helix &h = _helices[i];

		std::vector<Snow::Vertex> full_verts;
		std::vector<GLuint> full_idxs;
		drawHelix(h, full_verts, full_idxs);
		
		addIndicesToEnd(full_idxs);
		_helixToVertex[i] = _vertices.size();

		_vertices.reserve(_vertices.size() + full_verts.size());
		_vertices.insert(_vertices.end(), full_verts.begin(), full_verts.end());
	}
}

void GuiHelices::updateSinglePosition(Atom *a, glm::vec3 &p)
{
	if (_cAlphaMap.count(a) == 0)
	{
		return;
	}

	int idx = _cAlphaMap[a];
	_helicesToUpdate.insert(idx);
}

void GuiHelices::updateMultiPositions(Atom *a, WithPos &wp)
{
	updateSinglePosition(a, wp.ave);

}

void GuiHelices::finishUpdate()
{
	std::vector<Snow::Vertex> full_verts;
	std::vector<GLuint> full_idxs;
	int update = _helicesToUpdate.size();

	for (int idx : _helicesToUpdate)
	{
		std::vector<Snow::Vertex> full_verts;
		std::vector<GLuint> full_idxs;
		drawHelix(_helices[idx], full_verts, full_idxs);
		int insertion = _helixToVertex[idx];
		
		for (size_t j = 0; j < full_verts.size(); j++)
		{
			Vertex &v = _vertices[insertion + j];
			v.pos = full_verts[j].pos;
			v.color = full_verts[j].color;
			v.normal = glm::normalize(full_verts[j].normal);
		}
	}
	
	_helicesToUpdate.clear();
	calculateNormals();

	forceRender(true, false);
}
