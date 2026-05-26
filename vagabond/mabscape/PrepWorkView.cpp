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

#include "AntibodyOrderingView.h"
#include "ContactPoint.h"
#include "PrepWorkView.h"
#include "Mesh.h"
#include "Mab.h"
#include <chrono>
#include <vagabond/utils/DoJob.h>
#include <vagabond/gui/elements/TextButton.h>

PrepWorkView::PrepWorkView(Scene *prev, Mab &mab) 
: Scene(prev), Display(prev), _mab(mab)
{
	_farSlab = 80;
	_slabbing = true;
	shiftToCentre({}, 80);

}

void PrepWorkView::setup()
{
	addTitle("Preparing workspace");

	DoJob *doJob = new DoJob([this]() 
	                         { prepareAntigens(_mab.antigens); }, true);
	
	auto displayResults = [this]()
	{
		for (auto &tidy : _tidy)
		{
			tidy();
		}

		float top = 0.3;
		Text *summary = new Text("Completed " + 
		                         std::to_string(_messages.size()) + " jobs:");
		summary->setLeft(0.2, top);
		addObject(summary);

		for (const std::string &msg : _messages)
		{
			top += 0.06;
			Text *summary = new Text(msg);
			summary->setLeft(0.26, top);
			addObject(summary);
		}
		
		auto show_orders = [this]()
		{
			AntibodyOrderingView *aov = new AntibodyOrderingView(this, _mab);
			aov->show();
		};

		TextButton *tb = new TextButton("Next", this);
		tb->setRight(0.8, 0.8);
		tb->setReturnJob(show_orders);
		addObject(tb);
	};

	DoJob([this, displayResults, doJob]() 
	{
		doJob->join();
		
		addMainThreadJob(displayResults);
	});
	
}

void PrepWorkView::prepareAntigens(Antigens &antigens)
{
	for (Antigen &antigen : _mab.antigens)
	{
		setInformation("Meshing up antigen " + antigen.title);
		Mesh *mesh = antigen.mesh();

		addObject(mesh);
		shiftToCentre(mesh->centroid(), 0);

//		mesh->refine();
		_messages.push_back("Refined mesh for antigen " + antigen.title);

		for (Fiducial &fid : _mab.fiducials)
		{
			if (fid.antigen == antigen.title)
			{
				fid.model.load();
				DisplayUnit *unit = new DisplayUnit(this);
				unit->loadAtoms(fid.model.currentAtoms());
				addMainThreadJob
				([this, unit]()
				 {
					unit->displayAtoms();
					unit->startWatch();
					addDisplayUnit(unit);
				 });

				ContactPoint contact(fid, _mab.antigens);
				contact.findMapping();

				int total = contact.transforms().size();
				_messages.push_back("Mapped " + std::to_string(total) + 
				" orientation" + (total == 1 ? "" : "s") + 
				" for antibody " + fid.name);
				
				for (const glm::mat4x4 &mat : contact.transforms())
				{
					contact.applyTransform(mat);
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				}

				_tidy.push_back
				([unit, &fid, this]()
				 {
					removeDisplayUnit(unit);
					fid.model.unload();
				});
			}
		}

		_tidy.push_back
		([this, mesh]()
		 {
			removeObject(mesh);
		});
	}
}
