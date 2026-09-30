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
#include "Playground.h"
#include "ContactPoint.h"
#include "AlignmentView.h"
#include "PrepWorkView.h"
#include "Mesh.h"
#include "Mab.h"
#include <chrono>
#include <vagabond/utils/DoJob.h>
#include <vagabond/gui/elements/TextButton.h>

PrepWorkView::PrepWorkView(Scene *prev, Mab &mab) 
: Scene(prev), Display(prev), _mab(mab), _prev(prev)
{
	_farSlab = 80;
	_slabbing = true;
	shiftToCentre({}, 80);

}

void PrepWorkView::setup()
{
	addTitle("Preparing workspace");

	// an exception escaping a worker thread would end the whole program
	DoJob *doJob = new DoJob([this]() 
	{
		try
		{
			prepareAntigens(_mab.antigens);
		}
		catch (const std::exception &err)
		{
			std::cout << "Preparation failed: " << err.what() << std::endl;
			PrepMessage message = {std::string("Preparation stopped: ") + 
			                       err.what(), false};
			message.failure = true;
			_messages.push_back(message);
		}
	}, true);
	
	auto displayResults = [this]()
	{
		for (auto &tidy : _tidy)
		{
			tidy();
		}

		// keep what was worked out, so it is there next time
		_mab.save();

		// a scene of its own, a page at a time: the list can be long. It
		// goes back to where this one did, since the work here is done.
		Mab *mab = &_mab;
		auto next = [mab](Scene *scene)
		{
			// reordered before (and kept)? then straight on to the playground;
			// the ordering can still be changed from its menu
			bool ordered = true;
			for (const Competition &comp : mab->competitions)
			{
				ordered &= (comp.favoured_ordering.size() > 0);
			}

			if (ordered && mab->competitions.size())
			{
				Playground *playground = new Playground(scene, *mab);
				playground->show();
				return;
			}

			AntibodyOrderingView *aov = new AntibodyOrderingView(scene, *mab);
			aov->show();
		};

		PrepResultsView *prv = new PrepResultsView(_prev, _messages, next);
		prv->show();
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

		mesh->refine();
		_messages.push_back({"Refined mesh for antigen " + antigen.title, false});

		for (Fiducial &fid : _mab.fiducials)
		{
			setInformation("Aligning " + fid.name);
			if (fid.antigen == antigen.title)
			{
				DisplayUnit *unit = nullptr;
				bool loaded = false;
				try
				{
					// worked out before, from the same models? then no need to
					// load them or go through it all again
					ContactPoint *contact = new ContactPoint(fid, _mab.antigens, 
					                                         false);
					bool saved = contact->restore(fid.alignment);

					if (!saved)
					{
						delete contact;

						fid.model.load();
						loaded = true;
						unit = new DisplayUnit(this);
						unit->loadAtoms(fid.model.currentAtoms());
						addMainThreadJob
						([this, unit]()
						 {
							unit->displayAtoms();
							unit->startWatch();
							addDisplayUnit(unit);
						 });

						contact = new ContactPoint(fid, _mab.antigens);
						contact->findMapping();
						fid.alignment = contact->alignmentJson();
					}

					fid.contact = contact;

					int total = contact->transforms().size();
					_messages.push_back({"Mapped " + std::to_string(total) + 
					" orientation" + (total == 1 ? "" : "s") + 
					" for antibody " + fid.name + (saved ? " (saved)" : ""), 
					false});

					// a poor alignment of any of them
					for (const ContactPoint::Warning &warning : contact->warnings())
					{
						PrepMessage message = {warning.text, true};
						Antigen *ag = &antigen; Fiducial *fd = &fid;
						glm::mat4x4 tr = warning.transform;
						std::string text = warning.text;
						message.show = [ag, fd, tr, text](Scene *scene)
						{
							AlignmentView *av = new AlignmentView(scene, *ag, *fd,
							                                      tr, text);
							av->show();
						};
						_messages.push_back(message);
					}

					if (saved)
					{
						for (const glm::mat4x4 &mat : contact->transforms())
						{
							antigen.sym.add_if_new(mat);
						}
						continue;
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(200));
				
					for (const glm::mat4x4 &mat : contact->transforms())
					{
						contact->applyTransform(mat);
						std::this_thread::sleep_for(std::chrono::milliseconds(200));
						antigen.sym.add_if_new(mat);
					}

					_tidy.push_back
					([unit, &fid, this]()
					 {
						removeDisplayUnit(unit);
						fid.model.unload();
					});
				}
				catch (const std::exception &err)
				{
					// this antibody cannot be aligned: say so, carry on with
					// the others. Without a contact it is not a fixed one.
					std::cout << "Could not align " << fid.name << ": " 
					<< err.what() << std::endl;
					fid.contact = nullptr;

					// and what was shown for it goes with the others
					if (unit || loaded)
					{
						_tidy.push_back
						([unit, loaded, &fid, this]()
						 {
							if (unit)
							{
								removeDisplayUnit(unit);
							}

							if (loaded)
							{
								fid.model.unload();
							}
						});
					}

					PrepMessage message = {"Could not align antibody " + 
					                       fid.name + ": " + err.what(), false};
					message.failure = true;
					_messages.push_back(message);
				}
			}
		}

		antigen.sym.recalibrateToFirst();
		int total = antigen.sym.transforms().size();
		_messages.push_back({"Collected " + std::to_string(total) + 
		                     " orientation" + (total == 1 ? "" : "s") + 
		                     " for antigen " + antigen.title, false});

		for (const glm::mat4x4 &mat : antigen.sym.transforms())
		{
			std::cout << mat << std::endl << std::endl;
		}

		_tidy.push_back
		([this, mesh]()
		 {
			removeObject(mesh);
		});
	}
}
