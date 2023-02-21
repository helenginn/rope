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

#include <iostream>
#include <fstream>
#include <thread>
#include <vagabond/gui/elements/Text.h>

#include "SandboxView.h"
#include "HyperValue.h"
#include "PdbFile.h"
#include "Sequence.h"
#include "AtomGroup.h"
#include "AtomsFromSequence.h"
#include "CyclicView.h"

SandboxView::SandboxView(Scene *prev) : Scene(prev), Display(prev)
{
	Sequence apa("APA");
	AtomGroup *as = AtomsFromSequence(apa).atoms();
	setAtoms(as);
	as->recalculate();

	_angles = new Text("angles", Font::Thin, true);
	_angles->resize(0.6);
	_angles->setLeft(0.2, 0.5);
	addObject(_angles);

	_lengths = new Text("lengths", Font::Thin, true);
	_lengths->resize(0.6);
	_lengths->setLeft(0.05, 0.5);
	addObject(_lengths);
	
	/*
	for (size_t i = 0; i < as->size(); i++)
	{
		Atom *a = (*as)[i];
		if (a->cyclic())
		{
			addCyclicView(a->cyclic());
		}
	}
	*/
	
//	addCyclicView(nullptr);
}

void SandboxView::addCyclicView(Cyclic *cyclic)
{
	CyclicView *view = new CyclicView(cyclic);
	view->setResponder(this);
	addObject(view);
	_cyclic = view;
}

SandboxView::~SandboxView()
{
	stop();
}

void SandboxView::setup()
{
	addTitle("Proline sandbox");

	if (_cyclic)
	{
		_cyclic->increment();
	}

	doThings();
	
	Display::setup();
	
	if (_atoms)
	{
		PdbFile::writeAtoms(_atoms, "apa.pdb");
		Atom *cg = _atoms->firstAtomWithName("CG");
		HyperValue *hv = cg->hyperValue(1);
		hv->setValue(7.0);
	}
	
	scan();
}

void SandboxView::scan()
{
	Atom *cg = _atoms->firstAtomWithName("CG");
	HyperValue *offset = cg->hyperValue(0);
	HyperValue *amplitude = cg->hyperValue(1);

	std::vector<Atom *> cas = _atoms->atomsWithName("CA");
	Atom *ca = nullptr;

	for (Atom *candidate : cas)
	{
		if (candidate->code() != "PRO")
		{
			continue;
		}
		
		ca = candidate;
		break;
	}
	
	std::map<int, std::map<int, std::pair<float, float> > > rounds;
	std::map<int, std::map<int, std::pair<float, float> > > results;
	
	float golden = (sqrt(5) + 1) / 2;
	float headstart = 0;
	for (float amp = 0; amp < 0.8; amp += 0.0005)
	{
		float prop = amp * amp * 1000 + 10;
		float step = 1 / prop;
		headstart += golden; 

		for (float off = headstart; off < 0.5f + headstart; off += step)
		{
			float val = fmod(off, 0.5);
			offset->setValue(val);
			amplitude->setValue(amp);
			_atoms->recalculate();

			BondTorsion *psi = ca->findBondTorsion("C-N-CA-C");
			float f = psi->measurement(BondTorsion::SourceDerived);
			BondTorsion *x2_ = ca->findBondTorsion("CA-CB-CG-CD");
			float x2 = x2_->measurement(BondTorsion::SourceDerived);
			
			int f_round = (long)lrint(floor(f));
			int x2_round = (long)lrint(floor(x2));
			float rf = f - (float)f_round;
			float rx2 = x2 - (float)x2_round;

			std::pair<float, float> pair(amp, val);
			
			if (rounds[f_round][x2_round].first > -rf)
			{
				rounds[f_round][x2_round].first = -rf;
				results[f_round][x2_round].first = amp;
			}
			
			if (rounds[f_round][x2_round].second > -rx2)
			{
				rounds[f_round][x2_round].second = -rx2;
				results[f_round][x2_round].second = val;
			}
		}
	}
	
	std::ofstream proline;
	proline.open("lookup.txt");
	proline << "psi, x2, amp, offset" << std::endl;

	for (auto it = results.begin(); it != results.end(); it++)
	{
		float f = it->first;
		for (auto jt = it->second.begin(); jt != it->second.end(); jt++)
		{
			float x2 = jt->first;
			float amp = jt->second.first;
			float off = jt->second.second;
			proline << f << ", " << x2 << ", ";
			proline << amp << ", " << off << ", ";
			proline << std::endl;
		}

	}
	
	proline.close();
}

void SandboxView::buttonPressed(std::string tag, Button *button)
{
	Display::buttonPressed(tag, button);
}

void SandboxView::stop()
{
	if (_worker != nullptr)
	{
		_worker->join();
		delete _worker;
		_worker = nullptr;
	}

}

void SandboxView::mouseReleaseEvent(double x, double y,
                                    SDL_MouseButtonEvent button)
{
	Mouse3D::mouseReleaseEvent(x, y, button);

	if (_cyclic && !_moving && _shiftPressed)
	{
		stop();
		updateInfo();
		_worker = new std::thread(&CyclicView::refine, _cyclic);
	}
}

void SandboxView::respond()
{

}

void SandboxView::doThings()
{
	/*
	if (_atoms)
	{
		Atom *cg = _atoms->firstAtomWithName("CG");
		HyperValue *hv = cg->hyperValue(0);

		float val = hv->value();
		val += 0.1;
		hv->setValue(val);

		_atoms->recalculate();

		std::vector<Atom *> cas = _atoms->atomsWithName("CA");
		std::vector<Atom *> cds = _atoms->atomsWithName("CD");
		float f = 0; float x1 = 0; float x2 = 0; float x3 = 0;
		
		for (Atom *cd : cds)
		{
			if (cd->code() != "PRO")
			{
				continue;
			}

			BondTorsion *x3_ = cd->findBondTorsion("N-CD-CG-CB");
			x3 = x3_->measurement(BondTorsion::SourceDerived);
		}
		
		for (Atom *ca : cas)
		{
			if (ca->code() != "PRO")
			{
				continue;
			}

			BondTorsion *x1_ = ca->findBondTorsion("CG-CB-CA-N");
			x1 = x1_->measurement(BondTorsion::SourceDerived);
			BondTorsion *x2_ = ca->findBondTorsion("CA-CB-CG-CD");
			x2 = x2_->measurement(BondTorsion::SourceDerived);
		}

		std::cout << "psi/x1/x2/x3 " << f << " " << x1 << " ";
		std::cout << x2 << " " << x3 << std::endl;
	}
	*/
}

void SandboxView::updateInfo()
{
	std::string report = "angles\n";
	report += _cyclic->angles();
	_angles->setText(report);

	report = "lengths\n";
	report += _cyclic->lengths();
	_lengths->setText(report);

	float lsc = _cyclic->score();
	setInformation("score: " + std::to_string(lsc));

}
