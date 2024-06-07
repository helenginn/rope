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

#include "ApplyScaleBFactor.h"
#include "AddTogether.h"
#include "Babinet.h"
#include "grids/Diffraction.h"
#include "files/MtzFile.h"

Babinet::Babinet(Diffraction *model)
{
	_model = model;
}

Babinet::~Babinet()
{
	
}

void Babinet::prepareSolvent()
{
	_solvent = new Diffraction(_model);
}

void Babinet::applyKB(float k, float b)
{
	ApplyScaleBFactor apply(_solvent);
	apply(k, b);
}

void Babinet::addProtein()
{
	/*
	AddTogether add(_model, _solvent);
	
	Diffraction *result = add();
	delete _solvent;
	_solvent = result;
	return;
	*/

	auto add_protein = [this](long idx)
	{
		float amp = _model->element(idx).amplitude();
		float ph = _model->element(idx).phase();

		_solvent->element(idx).addAmplitudePhase(amp, ph);
	};
	
	_solvent->do_op_on_each_1d_index(add_protein);

}

Diffraction *Babinet::operator()()
{
	prepareSolvent();
	applyKB(_k, _b);

	addProtein();

	return _solvent;
}
