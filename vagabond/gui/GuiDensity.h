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

#ifndef __vagabond__GuiDensity__
#define __vagabond__GuiDensity__

#include <vagabond/gui/elements/CullablePrimitives.h>
#include <vagabond/core/grids/OriginGrid.h>
#include <fftw3.h>

class AtomMap;
class ArbitraryMap;
class AtomGroup;

namespace MC
{
	struct mcMesh;
};

class GuiDensity : public CullablePrimitives
{
public:
	GuiDensity();

	virtual void render(SnowGL *gl);

	void sampleFromOtherMap(OriginGrid<fftwf_complex> *ref, 
	                        OriginGrid<fftwf_complex> *map,
	                        bool diff = false);
	void populateFromMap(OriginGrid<fftwf_complex> *map);

	void setReferenceDensity(OriginGrid<fftwf_complex> *ref)
	{
		_ref = ref;
	}
	
	void setAtoms(AtomGroup *atoms)
	{
		_atoms = atoms;
	}
	
	void setTracking(bool track)
	{
		_tracking = track ? 1 : 0;
	}
	
	void setStep(float step)
	{
		_step = step;
	}
	
	void setThreshold(float threshold)
	{
		_threshold = threshold;
	}
	
	OriginGrid<fftwf_complex> *const &map() const
	{
		return _map;
	}

	void tear(AtomMap *map);
	void fromAtomMap(AtomMap *map, bool differences);
	virtual void extraUniforms();
private:
	void objectFromMesh(MC::mcMesh &mesh, bool diff);
	AtomGroup *_atoms = nullptr;
	OriginGrid<fftwf_complex> *_ref = nullptr;
	OriginGrid<fftwf_complex> *_map = nullptr;
	OriginGrid<fftwf_complex> *_tear = nullptr;

	float _threshold = 1;
	float _step = -1;
	int _slice = 0;
	int _tracking = 1;
	bool _hasUC = true;
};

#endif
