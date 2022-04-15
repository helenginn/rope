#include "GuiAtom.h"
#include "Icosahedron.h"
#include "GuiBond.h"

#include <vagabond/core/matrix_functions.h>
#include <vagabond/core/Atom.h>
#include <vagabond/core/AtomGroup.h>

#include <iostream>
#include <thread>

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
	_finish = false;
}

GuiAtom::~GuiAtom()
{
	if (_watch != nullptr)
	{
		_finish = true;
		_watch->join();
	}
}

void GuiAtom::setMulti(bool m)
{
	_multi = m;
	_bonds->setMulti(m);
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
	if (ele == "P")
	{
		colour = glm::vec4(1.0, 0.2, 1.0, 1.);
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

size_t GuiAtom::indicesPerAtom()
{
	return _template->indexCount();
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
	_vertices.reserve(verticesPerAtom() * a->size());
	_indices.reserve(indicesPerAtom() * a->size());

	for (size_t i = 0; i < a->size(); i++)
	{
		watchAtom((*a)[i]);
	}
	
	_bonds->watchBonds(a);
}

void GuiAtom::updateMultiPositions(Atom *a, Atom::WithPos &wp)
{
	_bonds->updateAtoms(a, wp);
}

void GuiAtom::updateSinglePosition(Atom *a, glm::vec3 &p)
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

	_atomPos[a] = p;
}

bool GuiAtom::checkAtom(Atom *a)
{
	if (!_multi)
	{
		glm::vec3 p;
		if (a->positionChanged() && a->fishPosition(&p))
		{
			updateSinglePosition(a, p);
			_bonds->updateAtom(a, p);
			return true;
		}
	}
	else
	{
		Atom::WithPos wp;
		if (a->positionChanged() && a->fishPositions(&wp))
		{
			updateSinglePosition(a, wp.ave);
			updateMultiPositions(a, wp);
			return true;
		}
	}
	
	return false;
}

void GuiAtom::checkAtoms()
{
	bool changed = false;
	int pre = _bonds->indexCount();

	for (size_t i = 0; i < _atoms.size(); i++)
	{
		Atom *a = _atoms[i];
		try
		{
			changed |= checkAtom(a);
		}
		catch (std::runtime_error err)
		{
			std::cout << "Error drawing atom! " << err.what() << ", ";
			std::cout << "atom: " << a->atomName() <<  std::endl;
			return;
		}
	}
	
	int post = _bonds->indexCount();

	if (changed)
	{
		forceRender();
		_bonds->forceRender();
	}
}

void GuiAtom::backgroundWatch(GuiAtom *what)
{
	while (!what->_finish)
	{
		what->checkAtoms();
		std::chrono::duration<double, std::milli> time_span;
		time_span = std::chrono::milliseconds(100);
		std::this_thread::sleep_for(time_span);
	}
}

void GuiAtom::startBackgroundWatch()
{
	_watch = new std::thread(&GuiAtom::backgroundWatch, this);
}
