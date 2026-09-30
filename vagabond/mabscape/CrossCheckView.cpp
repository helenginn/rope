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

		// the same sums as for all the antibodies, for this one alone
		std::vector<double> abSumsq(results.size(), 0);
		std::vector<size_t> abCounts(results.size(), 0);

		for (size_t p = 0; p < clusters.points.size(); p++)
		{
			size_t r = clusters.resultOf[p];
			glm::vec3 diff = clusters.points[p] - 
			clusters.centres[clusters.member[p]];
			sumsq[r] += glm::dot(diff, diff);
			counts[r]++;
			abSumsq[r] += glm::dot(diff, diff);
			abCounts[r]++;
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
			// and its RMSD as for the overall one: per result, the root mean
			// square distance of its positions from their cluster centres,
			// then the mean and spread over the results
			std::vector<float> abRmsds;
			for (size_t r = 0; r < results.size(); r++)
			{
				if (abCounts[r])
				{
					abRmsds.push_back(sqrt(abSumsq[r] / abCounts[r]));
				}
			}

			float mean, sd;
			meanAndSd(abRmsds, mean, sd);

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

			stats.freed.push_back({pl.name, best, mean, sd, elements});
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
	size_t rows = 1 + 2 + 1 + 1 + 1 + (freed ? 1 + freed : 0) + 1;
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

	// a line with values in boxes of their own, in columns to the right
	auto line3 = [this, &y, scale](const std::string &label, 
	                               const std::string &first,
	                               const std::string &second,
	                               const std::string &third)
	{
		Text *l = new Text(label);
		l->resize(0.6 * scale);
		l->setLeft(0.15, y);
		addObject(l);

		const std::pair<const std::string *, float> columns[] =
		{{&first, 0.5f}, {&second, 0.68f}, {&third, 0.85f}};

		for (const auto &column : columns)
		{
			if (column.first->length())
			{
				Text *v = new Text(*column.first);
				v->resize(0.6 * scale);
				v->setRight(column.second, y);
				addObject(v);
			}
		}

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

		// each one: how far its average position is from its known one, its
		// RMSD (as above) and how much data there is for it
		line3("", "distance", "rmsd", "data");

		for (const auto &entry : _stats.freed)
		{
			line3("   " + entry.name, 
			      withError(entry.distance, 0, false),
			      withError(entry.rmsd, entry.rmsdSd, _stats.results > 1),
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
