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

#ifndef __vagabond__Comparable__
#define __vagabond__Comparable__

/** \class Comparable group of atoms corresponding to a molecule or known ligand
 * which can be compared with one another or interfaces identified, etc. */

#include <vagabond/utils/glm_json.h>
using nlohmann::json;

class Interface;
class AtomGroup;
class Molecule;
class Model;
class Atom;

class Comparable
{
public:
	Comparable();
	virtual ~Comparable() {};

	virtual AtomGroup *currentAtoms() = 0;
	
	Model *const model();

	void setModel(Model *model)
	{
		_model = model;
	}
	
	const std::string &model_id() const
	{
		return _model_id;
	}
	
	virtual const std::string id() const { return ""; }
	
	virtual std::string desc() const { return ""; }
	
	/** generate a simple interface between this comparable and its model
	 *  with a tight cutoff distance */
	Interface *interfaceWithModel();

	void addToInterface(Interface *face, double max, bool derived);
	virtual Atom *equivalentForAtom(Model *other, std::string desc)
	{
		return nullptr;
	}
	
	virtual std::map<Atom *, Atom *> mapAtoms(Molecule *other)
	{
		return std::map<Atom *, Atom *>();
	}
	
	friend void to_json(json &j, const Molecule &value);
	friend void from_json(const json &j, Molecule &value);
protected:
	std::string _model_id;
	Model *_model = nullptr;

	AtomGroup *_currentAtoms = nullptr;
	AtomGroup *_motherAtoms = nullptr;
};

#endif
