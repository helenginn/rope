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

#include "CrossCheckView.h"
#include "Positions.h"
#include "Mab.h"
#include <vagabond/gui/elements/Text.h>
#include <algorithm>
#include <cmath>
#include <set>
#include <sstream>
#include <iomanip>

static void meanAndSd(const std::vector<float> &values, float &mean, float &sd)
{
	mean = 0; sd = 0;
	if (values.empty())
	{
		return;
	}

	for (float v : values)
	{
		mean += v;
	}
	mean /= values.size();

	if (values.size() > 1)
	{
		float sumsq = 0;
		for (float v : values)
		{
			sumsq += (v - mean) * (v - mean);
		}
		sd = sqrt(sumsq / (values.size() - 1));
	}
}

CrossCheckView::Stats 
CrossCheckView::compute(const Positions &positions, Competition &comp,
                        const std::string &key,
                        const std::vector<PosToComp::Result> &results)
{
	Stats stats;
	stats.results = results.size();

	if (results.empty())
	{
		return stats;
	}

	// the fixed antibodies of this bucket, so that the rest of those with
	// a structure are the ones which were free but have a known answer
	std::set<std::string> fixedHere;
	std::stringstream ss(key);
	std::string name;
	while (std::getline(ss, name, ','))
	{
		size_t start = name.find_first_not_of(' ');
		if (start != std::string::npos)
		{
			fixedHere.insert(name.substr(start));
		}
	}

	std::vector<const std::vector<Positions::Placement> *> all;
	for (const PosToComp::Result &r : results)
	{
		all.push_back(&r.placements);
	}

	std::set<std::string> structural;
	for (const std::string &s : positions.structuralAntibodies())
	{
		structural.insert(s);
	}

	// the data, for how much of it involves each antibody
	std::vector<std::string> order = comp.favoured_ordering;
	Eigen::MatrixXf data = comp.make_plot(order);

	std::vector<double> sumsq(results.size(), 0);
	std::vector<size_t> counts(results.size(), 0);
	std::vector<float> freedDistances;

	for (const Positions::Placement &pl : *all[0])
	{
		Positions::Clusters clusters;
		if (!Positions::cluster(all, pl.name, clusters))
		{
			continue;
		}

		stats.antibodies++;

		for (size_t p = 0; p < clusters.points.size(); p++)
		{
			size_t r = clusters.resultOf[p];
			glm::vec3 diff = clusters.points[p] - 
			clusters.centres[clusters.member[p]];
			sumsq[r] += glm::dot(diff, diff);
			counts[r]++;
		}

		if (!structural.count(pl.name) || fixedHere.count(pl.name))
		{
			continue;
		}

		// its average position against the structure, allowing for the
		// symmetry operator which brings the two together
		std::vector<glm::vec3> known = positions.structuralCopies(pl.name);
		float best = FLT_MAX;
		for (const glm::vec3 &k : known)
		{
			best = std::min(best, glm::length(clusters.centres[0] - k));
		}

		if (best < FLT_MAX)
		{
			// and result by result, each one's nearest copy to the nearest
			// copy of the structure
			std::vector<float> perResult(results.size(), FLT_MAX);
			for (size_t p = 0; p < clusters.points.size(); p++)
			{
				for (const glm::vec3 &k : known)
				{
					float d = glm::length(clusters.points[p] - k);
					float &nearest = perResult[clusters.resultOf[p]];
					nearest = std::min(nearest, d);
				}
			}

			std::vector<float> seen;
			for (float d : perResult)
			{
				if (d < FLT_MAX)
				{
					seen.push_back(d);
				}
			}

			float mean, sd;
			meanAndSd(seen, mean, sd);

			size_t elements = 0;
			auto at = std::find(order.begin(), order.end(), pl.name);
			if (at != order.end())
			{
				int row = at - order.begin();
				for (int col = 0; col < data.cols(); col++)
				{
					float v = data(row, col);
					if (col != row && v == v)
					{
						elements++;
					}
				}
			}

			stats.freed.push_back({pl.name, best, sd, elements});
			freedDistances.push_back(best);
		}
	}

	std::vector<float> rmsds;
	for (size_t r = 0; r < results.size(); r++)
	{
		if (counts[r])
		{
			rmsds.push_back(sqrt(sumsq[r] / counts[r]));
		}
	}

	meanAndSd(rmsds, stats.rmsd, stats.rmsdSd);
	meanAndSd(freedDistances, stats.freedMean, stats.freedSd);
	stats.randomDistance = positions.randomDistance();
	return stats;
}

CrossCheckView::CrossCheckView(Scene *prev, const std::string &title,
                               const Stats &stats)
: Scene(prev), _title(title), _stats(stats)
{

}

static std::string withError(float mean, float sd, bool showSd)
{
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(2) << mean;

	if (showSd)
	{
		ss << " +/- " << sd;
	}

	ss << " A";
	return ss.str();
}

void CrossCheckView::setup()
{
	addTitle("Cross-check");

	// everything is squashed, text as well as spacing, if a long list of
	// fiducials would otherwise run off the bottom of the page
	const float top = 0.2;
	const float bottom = 0.93;
	size_t freed = _stats.freed.size();
	size_t rows = 1 + 2 + 1 + 1 + (freed ? 1 + freed : 1) + 1;
	float needed = rows * 0.06 + 3 * 0.03;
	float scale = std::min(1.f, (bottom - top) / needed);

	float y = top;
	auto line = [this, &y, scale](const std::string &label, 
	                              const std::string &value)
	{
		Text *l = new Text(label);
		l->resize(0.6 * scale);
		l->setLeft(0.15, y);
		addObject(l);

		if (value.length())
		{
			Text *v = new Text(value);
			v->resize(0.6 * scale);
			v->setRight(0.85, y);
			addObject(v);
		}

		y += 0.06 * scale;
	};

	// a line with a second value in a box of its own, further to the left
	auto line2 = [this, &y, scale](const std::string &label, 
	                               const std::string &middle,
	                               const std::string &value)
	{
		Text *l = new Text(label);
		l->resize(0.6 * scale);
		l->setLeft(0.15, y);
		addObject(l);

		Text *m = new Text(middle);
		m->resize(0.6 * scale);
		m->setRight(0.62, y);
		addObject(m);

		Text *v = new Text(value);
		v->resize(0.6 * scale);
		v->setRight(0.85, y);
		addObject(v);

		y += 0.06 * scale;
	};

	auto gap = [&y, scale]()
	{
		y += 0.03 * scale;
	};

	// from what is being averaged, to how well it holds together, to how
	// well it agrees with what is known
	line(_title, "");
	gap();
	line("Results averaged", std::to_string(_stats.results));
	line("Antibodies averaged", std::to_string(_stats.antibodies));
	gap();
	line("Average RMSD of antibody positions", 
	     withError(_stats.rmsd, _stats.rmsdSd, _stats.results > 1));
	gap();

	line("Fiducials left free", std::to_string(_stats.freed.size()));

	if (_stats.freed.size())
	{
		line("Average distance from known free set",
		     withError(_stats.freedMean, _stats.freedSd, 
		               _stats.freed.size() > 1));

		for (const auto &entry : _stats.freed)
		{
			// the distance and its spread over the results, then how much
			// data there is
			line2("   " + entry.name, 
			      withError(entry.distance, entry.sd, _stats.results > 1),
			      std::to_string(entry.elements) + " entries");
		}
	}
	else
	{
		line("Average distance from known free set", "n/a");
	}

	// for scale: how far apart two random positions come out
	line("Random reference (symmetry-aware)", 
	     withError(_stats.randomDistance, 0, false));
}
