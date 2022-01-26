#include "GuiAtom.h"
#include "matrix_functions.h"
#include "Icosahedron.h"
#include "Atom.h"
#include "AtomGroup.h"
#include "GuiBond.h"
#include <iostream>

GuiAtom::GuiAtom() : Renderable()
{
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/lighting.fsh");
	
	_template = new Icosahedron();
	_template->triangulate();
	_template->setColour(0.5, 0.5, 0.5);
	_template->resize(0.3);
	
	_bonds = new GuiBond();
}

void GuiAtom::colourByElement(std::string ele)
{
	glm::vec4 colour = glm::vec4(0.5, 0.5, 0.5, 1.);
	if (ele == "O")
	{
		colour = glm::vec4(1.0, 0.2, 0.2, 1.);
	}
	if (ele == "S")
	{
		colour = glm::vec4(1.0, 1.0, 0.2, 1.);
	}
	if (ele == "H")
	{
		colour = glm::vec4(0.8, 0.8, 0.8, 1.);
	}
	if (ele == "N")
	{
		colour = glm::vec4(0.2, 0.2, 1.0, 1.);
	}

	_template->setColour(colour.x, colour.y, colour.z);
}

void GuiAtom::setPosition(glm::vec3 position)
{
	if (!is_glm_vec_sane(position))
	{
		throw std::runtime_error("position contains nan or vec values");
	}

	_template->setPosition(position);
}

size_t GuiAtom::verticesPerAtom()
{
	return _template->vertexCount();
}

void GuiAtom::render(SnowGL *gl)
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	Renderable::render(gl);
	_bonds->render(gl);

	glDisable(GL_DEPTH_TEST);
}

void GuiAtom::watchAtom(Atom *a)
{
	_atoms.push_back(a);
	long index = _vertices.size();

	colourByElement(a->elementSymbol());
	setPosition(glm::vec3(0.));

	appendObject(_template);

	_atomIndex[a] = index;
	
	checkAtom(a);
}

void GuiAtom::watchAtoms(AtomGroup *a)
{
	for (size_t i = 0; i < a->size(); i++)
	{
		watchAtom((*a)[i]);
	}
	
	_bonds->watchBonds(a);
}

void GuiAtom::checkAtom(Atom *a)
{
	glm::vec3 p;
	if (a->positionChanged() && a->fishPosition(&p))
	{
		int idx = _atomIndex[a];
		glm::vec3 last = _atomPos[a];

		glm::vec3 diff = p - last;

		if (!is_glm_vec_sane(diff))
		{
			throw std::runtime_error("position contains nan or vec values");
		}

		int end = idx + verticesPerAtom(); 
		for (size_t j = idx; j < end; j++)
		{
			_vertices[j].pos += diff;
		}

		_bonds->updateAtom(a, p);
		_atomPos[a] = p;
	}
}

void GuiAtom::checkAtoms()
{
	for (size_t i = 0; i < _atoms.size(); i++)
	{
		Atom *a = _atoms[i];
		checkAtom(a);
	}

	setupVBOBuffers();
	_bonds->setupVBOBuffers();
}
