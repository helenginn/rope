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

#include "Babinet.h"
#include "Diffraction.h"
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
	auto b_factor = [this, k, b](int i, int j, int l)
	{
		long idx = _solvent->index(i, j, l);
		float res = _solvent->resolution(i, j, l);
		float amp = _model->element(idx).amplitude();
		
		float b_val = exp(-b / (4 * res * res));
		if (idx == 0) b_val = 1;
		float mult = k * b_val;
		amp = amp * mult * -1;

		_solvent->element(idx).setAmplitude(amp);
	};
	
	_solvent->do_op_on_centred_index(b_factor);
}

void Babinet::addProtein()
{
	auto add_protein = [this](long idx)
	{
		float amp = _model->element(idx).amplitude();
		float my_amp = _solvent->element(idx).amplitude();
		float ph = _model->element(idx).phase();

		_solvent->element(idx).addAmplitudePhase(amp, ph);
	};
	
	_solvent->do_op_on_each_1d_index(add_protein);

}

Diffraction *Babinet::operator()()
{
	prepareSolvent();
	applyKB(_k, _b);

	/*
	MtzFile file("");
	file.setMap(_solvent);
	file.write_to_file("only.mtz", 1.5);
	*/

	addProtein();

	return _solvent;
}
