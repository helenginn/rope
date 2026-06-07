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

#include "ViewCorrelations.h"

#include <vagabond/utils/Eigen/Core>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/utils/FloydWarshall.h>
#include <vagabond/utils/DoJob.h>
#include <fstream>
#include <vagabond/core/protonic/Clique.h>
#include <vagabond/core/protonic/Correlative.h>
#include <vagabond/core/protonic/CertainStates.h>
#include <vagabond/gui/elements/list/LineGroup.h>
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/MatrixPlot.h>
#include <vagabond/gui/CommunicationChoice.h>
#include <vagabond/gui/CommunicationAnalysis.h>
#include <vagabond/gui/elements/ScrollBox.h>
#include <vagabond/gui/elements/Window.h>

using Eigen::seqN;

ViewCorrelations::ViewCorrelations(Scene *prev, Clique *clique)
: Scene(prev), _clique(clique)
{

}

void ViewCorrelations::setup()
{
	addTitle("Sub-network correlations");

	makeList();
}

void ViewCorrelations::makeList()
{
	_clique->setSelectJob({});
	
	for (Clique &clique : _clique->subdivisions())
	{
		auto view_subnetwork = [this, &clique](bool left)
		{
			if (left)
			{
				viewSubnetwork(clique);
			}
		};

		clique.setSelectJob(view_subnetwork);
	}

	_clique->setSelectJob([this](bool left) { if (left) viewAll(); });
	std::cout << ("Subgroups of " + 
	                        std::to_string(_clique->probes().size())
	                        + " nodes") << std::endl;

	LineGroup *lg = new LineGroup(_clique, this);
	lg->setLeft(-0.04, 0.2);
	std::cout << "Clique count: " << _clique->itemCount() << std::endl;

	ScrollBox *sb = new ScrollBox();
	sb->setContent(lg);
	sb->setBounds(glm::vec4(0.15, 0.0, 0.9, 0.28));
	addObject(sb);

	lg->refreshGroups();
	sb->addSliderIfNeeded();
}

void ViewCorrelations::viewAll()
{
	float all_ave = 0;
	OpSet<ProbeTypePair> all = 
	Correlative::probeTypePairs(_clique->subdivisions(), all_ave);

	_correlative = new Correlative(all, all_ave, false);
	Correlative &correl = *_correlative;

	auto process_clique = [&correl](const Clique &clique)
	{
		if (!clique.states()) return;
		const CertainStates &states = *clique.states();
		correl.addStates(states);
	};
	
	for (Clique &clique : _clique->subdivisions())
	{
		process_clique(clique);
	}

	Eigen::MatrixXf overall = correl.acquireMatrix();
	
	deleteTemps();
	_matrix = PCA::Matrix(overall);
	MatrixPlot *mp = new MatrixPlot(_matrix, _mutex);
	
	auto lookup = correl.matrixLookup();
	auto display_lookup = [this, lookup](float x, float y)
	{
		std::string info = lookup(x, y);
		setInformation(info);
	};

	mp->setHoverJob(display_lookup);
	glm::mat3x3 rot = glm::mat3x3(1.f);
	rot[1] *= -1;
	mp->rotateRoundCentre(rot);
	Renderable::Alignment align = Renderable::Alignment
	(Renderable::Alignment::Left | Renderable::Alignment::Top);
	mp->setArbitrary(0.4, 0.25, align);
	addTempObject(mp);
	
	auto choose_groups = [this]()
	{
		CommunicationChoice *cc = new CommunicationChoice(this, _clique);
		cc->show();
	};
	
	auto comm_analysis = [this, choose_groups, overall]()
	{
		int num = _clique->allCommsNames().size();
		if (num <= 1)
		{
			AskYesNo *ayn = 
			new AskYesNo(this, "Need to define at least two communication "\
			             "groups before analysis. Choose them now?", "", this);

			ayn->addJob("yes", choose_groups);
			setModal(ayn);
		}
		else
		{
			CommunicationAnalysis *ca = 
			new CommunicationAnalysis(this, _clique, 
			                          overall, _correlative->insertions());
			ca->show();
		}
	};
	
	auto fill_gaps = [this, mp, &overall]()
	{
		auto combine = [](float x, float y)
		{
			return x * y;
		};
		
		setInformation("Deriving intermediate correlations");

		FloydWarshall fw(overall, combine, true);
		fw.addDisplayMatrix(_matrix, _mutex, [mp]() { mp->update(); });
		fw.run();
		setInformation("Finished deriving intermediates");
		std::cout << "Done filling gaps" << std::endl;
	};
	
	TextButton *comm = new TextButton("Communication analysis", this);
	comm->setCentre(0.55, 0.9);
	comm->setReturnJob(comm_analysis);
	addTempObject(comm);

	new DoJob(fill_gaps);

}

void ViewCorrelations::viewSubnetwork(Clique &clique)
{
	setInformation(clique.name());
	deleteTemps();
	
	if (!clique.states()) return;
	const CertainStates &states = *clique.states();

	int np = states.ptps().size();
	float ydim = std::min(0.05, 0.5 / (float)np);
	float xdim = ydim * 0.6;

	auto make_align_to_grid = [xdim, ydim, this]()
	{
		float offx = 0.5;
		float offy = 0.4;

		return [this, xdim, ydim, offx, offy](Renderable *r, int m, int n)
		{
			Renderable::Alignment align = 
			Renderable::Alignment
			(Renderable::Alignment::Centre | Renderable::Alignment::Middle);

			if (n == -1)
			{
				glm::mat4x4 base = glm::mat3x3(1.f);
				base[1] /= Window::aspect();
				glm::mat3x3 rot;
				rot = glm::mat3x3(glm::rotate(base, (float)deg2rad(-45),
				                              glm::vec3(0., 0., -1.)));
				r->rotateRoundCentre(rot);
				align = Renderable::Alignment
				(Renderable::Alignment::Left | Renderable::Alignment::Bottom);
			}
			else if (m == -1)
			{
				align = Renderable::Alignment(Renderable::Alignment::Right);
			}
			else
			{
				glm::mat3x3 rot = glm::mat3x3(1.f);
				rot[1] *= -1;
				r->rotateRoundCentre(rot);

			}
			m++;
			n++;

			r->setArbitrary(offx + m * xdim, offy + n * ydim, align);
			addTempObject(r);
		};
	};
	
	auto align_to_grid = make_align_to_grid();
	
	std::cout << "Number of probes: " << states.ptps().size() << std::endl;
	
	for (int p = 0; p < states.ptps().size(); p++)
	{
		std::string desc = states.ptps()[p].first->desc();
		std::string type = states.ptps()[p].second == 
		hnet::Types::BondType ? " bonding" : " exists";
		
		for (int i = 0; i < 2; i++)
		{
			Text *t = new Text(desc + type);
			t->resize(0.3);
			align_to_grid(t, (i == 0 ? -1 : p), (i == 0 ? p : -1));
		}
	}
	
	for (int m = 0; m < states.ptps().size(); m++)
	{
		const ProbeTypePair &left = states.ptps()[m];
		for (int n = 0; n < states.ptps().size(); n++)
		{
			const ProbeTypePair &right = states.ptps()[n];
			float ave = states.average_score();
			ProbeCorrelation corr = states.correlate(left, right, ave, true);
			
			PCA::Matrix pca = PCA::Matrix(corr.mat);
			MatrixPlot *mp = new MatrixPlot(pca);
			mp->resize(xdim * 3);
			if (corr.mat.rows() == 2 && corr.mat.cols() == 2)
			{
				mp->resize(0.667);
			}
			align_to_grid(mp, m, n);
		}
	}
}

