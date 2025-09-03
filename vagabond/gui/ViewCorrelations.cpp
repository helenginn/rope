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
#include <vagabond/utils/svd/PCA.h>
#include <fstream>
#include <vagabond/core/protonic/Clique.h>
#include <vagabond/gui/elements/list/LineGroup.h>
#include <vagabond/gui/MatrixPlot.h>
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
	_clique->setHandleBeingChosen({});
	
	for (Clique &clique : _clique->subdivisions())
	{
		auto view_subnetwork = [this, &clique](bool left)
		{
			if (left)
			{
				viewSubnetwork(clique);
			}
		};

		clique.setHandleBeingChosen(view_subnetwork);
	}

	_clique->setHandleBeingChosen([this](bool left) { if (left) viewAll(); });

	LineGroup *lg = new LineGroup(_clique, this);
	lg->setLeft(-0.04, 0.2);
	std::cout << "Clique count: " << _clique->itemCount() << std::endl;

	ScrollBox *sb = new ScrollBox();
	sb->setContent(lg);
	sb->setBounds(glm::vec4(0.15, 0.0, 0.9, 0.35));
	addObject(sb);

	lg->refreshGroups();
	sb->addSliderIfNeeded();
}

void ViewCorrelations::viewAll()
{
	OpSet<ProbeTypePair> all;

	for (Clique &clique : _clique->subdivisions())
	{
		const std::vector<ProbeResult> &results = clique.results();
		std::vector<ProbeTypePair> active_probes = 
		probes(results);
		all += active_probes;
	}
	
	std::map<ProbeTypePair, std::pair<int, int>> insertions;

	int accumulative = 0;
	for (const ProbeTypePair &ptp : all)
	{
		int dim = dim_for_type(ptp.second);
		insertions[ptp] = {accumulative, dim};
		accumulative += dim;
	}
	
	_overall = MatrixXf(accumulative, accumulative);
	_overall.setZero();

	auto process_clique = [this, &insertions](const Clique &clique)
	{
		const std::vector<ProbeResult> &results = clique.results();
		std::vector<ProbeTypePair> active_probes = 
		probes(results);

		for (const ProbeTypePair &left : active_probes)
		{
			int x = insertions[left].first;
			int m = insertions[left].second;

			for (const ProbeTypePair &right : active_probes)
			{
				int y = insertions[right].first;
				int n = insertions[right].second;
				ProbeCorrelation corr = correlate(results, left, 
				                                  right, false);
				
				_overall(seqN(x, m), seqN(y, n)) += corr.mat;
			}
		}
	};
	
	for (Clique &clique : _clique->subdivisions())
	{
		process_clique(clique);
	}

	for (int i = 0; i < accumulative; i++)
	{
		float max = _overall(i, i);
		float mult = max > 1e-6 ? 1 / max : 0;
		_overall(i, Eigen::all) *= mult;
	}
	
	deleteTemps();
	PCA::Matrix pca = PCA::Matrix(_overall);
	MatrixPlot *mp = new MatrixPlot(pca);
	glm::mat3x3 rot = glm::mat3x3(1.f);
	rot[1] *= -1;
	mp->rotateRoundCentre(rot);
	Renderable::Alignment align = Renderable::Alignment
	(Renderable::Alignment::Left | Renderable::Alignment::Top);
	mp->setArbitrary(0.4, 0.25, align);
	addTempObject(mp);
	
	std::ofstream file;
	file.open("hbond_matrix.csv");
	
	auto to_str = [](int m, int i)
	{
		if (m == 2)
		{
			switch (i)
			{
				case 0: return "present";
				case 1: return "present";
				default: break;
			}
		}
		else 
		{
			switch (i)
			{
				case 0: return "missing";
				case 1: return "donor";
				case 2: return "acceptor";
				default: break;
			}
		}
		return "";
	};
	
	file << "left, right, value" << std::endl;
	for (const ProbeTypePair &left : all)
	{
		int m = insertions[left].second;
		std::string mstr = (m == 2 ? "exists" : "bonding");

		for (const ProbeTypePair &right : all)
		{
			int n = insertions[right].second;

			std::string nstr = (n == 2 ? "exists" : "bonding");
			for (int i = 0; i < m; i++)
			{
				std::string istr = to_str(m, i);
				std::string lstr = (left.first->desc() + " (" + 
				                    mstr + ", " + istr + ")");
				std::replace(lstr.begin(), lstr.end(), ',', ':');

				for (int j = 0; j < n; j++)
				{
					std::string jstr = to_str(n, j);
					std::string rstr = (right.first->desc() + " (" + 
					                    nstr + ", " + jstr + ")");
					std::replace(rstr.begin(), rstr.end(), ',', ':');

					file << lstr << "," << rstr << ",";
					int x = insertions[left].first + i;
					int y = insertions[right].first + j;
					float v = _overall(x, y);
					file << v << std::endl;
				}
			}
		}
	}

	file.close();
	

}

void ViewCorrelations::viewSubnetwork(Clique &clique)
{
	setInformation(clique.name());
	deleteTemps();
	
	const std::vector<ProbeResult> &results = clique.results();
	std::vector<ProbeTypePair> active_probes = 
	probes(results);

	int np = active_probes.size();
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
	
	std::cout << "Number of probes: " << active_probes.size() << std::endl;
	
	for (int p = 0; p < active_probes.size(); p++)
	{
		std::string desc = active_probes[p].first->desc();
		std::string type = active_probes[p].second == 
		hnet::Types::BondType ? " bonding" : " exists";
		
		for (int i = 0; i < 2; i++)
		{
			Text *t = new Text(desc + type);
			t->resize(0.3);
			align_to_grid(t, (i == 0 ? -1 : p), (i == 0 ? p : -1));
		}
	}
	
	for (int m = 0; m < active_probes.size(); m++)
	{
		for (int n = 0; n < active_probes.size(); n++)
		{
			ProbeCorrelation corr = correlate(results, active_probes[m], 
			                                  active_probes[n], true);
			
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

