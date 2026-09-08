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

#ifndef __vagabond__Graph__
#define __vagabond__Graph__

#include <vagabond/gui/elements/Box.h>
#include <map>
#include <functional>
#include <optional>

class Scatter;
class ThickLine;
class IndexResponseView;
class ColourLegend;

class Graph : public Box
{
public:
	Graph();
	
	enum Style
	{
		StyleLine,
		StyleScatter,
	};
	
	void setIndexResponder(IndexResponseView *irv);
	
	std::string data_label(int i, int j)
	{
		return _data[i][j].label;
	}
	
	Style style = StyleLine;

	// when style == StyleScatter, also draw each series' points joined by
	// a line, in the order they were added by addPoint() - lets a caller
	// show both the individual points and the track/series they belong to
	// at once, without a second, axis-duplicating Graph overlaid on top.
	bool connectPoints = false;

	// pointType indexes assets/images/points.png (0-7: filled circle,
	// filled star, filled triangle, X, circle-with-dot [the default, see
	// DataPoint], outline star, outline triangle, asterisk) - lets a
	// caller mark specific points as visually distinct from the rest
	// (e.g. OccupanciesView highlighting the watched signal residue).
	void addPoint(int series, float x, float y, const std::string &label = "",
	              float alpha = 1.f, int pointType = 4);
	void setRange(char axis, float min, float max);
	
	// width and height of box contents.
	void setup(float width, float height);
	void addToGraphPosition(float cx, float cy);
	
	void setSeriesColour(int series, glm::vec3 colour);

	void setAxisLabel(char axis, std::string name);
	void plotData(float width, float height);

	// debounced - see .cpp for why: a single stray "nothing under the
	// mouse" read right at a point's own antialiased edge shouldn't
	// instantly wipe a label that a following, equally valid mouse-move
	// poll would just recreate.
	void clearLabels();

	// called by Scatter::interacted() whenever it (re)draws a label -
	// cancels whatever run of clearLabels() misses had built up so far.
	void noteLabelShown();
	void clear();

	// optional per-point world-space coordinates (e.g. atom positions),
	// independent of the plotted x/y data itself and purely for
	// hoverColour() below - supplying none leaves points on their default
	// per-series colour/black, unchanged from before this existed.
	// Replaces whichever coordinates were previously set for this series,
	// so it can be resupplied/updated freely; indices should line up with
	// the order addPoint() was called for that series.
	void setSeriesCoordinates(int series, std::vector<glm::vec3> coords);

	// called by Scatter::interacted() on hover - recolours every point
	// across every series by its coordinate's distance from the hovered
	// point's own coordinate (BlackYellow, scaled 0 to the largest distance
	// found). A no-op if the hovered point has no coordinate set.
	// Reverted back to plain black by clearLabels() once the mouse leaves
	// every point (see its own comment).
	//
	// Also computes a "local correlation": the same x/y data addPoint()
	// was given, correlated across only the closest half (by this same
	// distance) of every coordinate-bearing point to the hovered one -
	// the furthest half is discarded outright, not down-weighted - and
	// reports it via setHoverInfoCallback() below. Doesn't touch the
	// colouring above, which still spans every point.
	void hoverColour(int series, int idx);

	// called with the local correlation (see hoverColour()'s own comment)
	// on hover, and std::nullopt once the mouse leaves every point (same
	// moment resetColours() reverts the colouring) - lets a caller (e.g.
	// OccupanciesView) surface it, e.g. via setInformation().
	void setHoverInfoCallback(std::function<void(std::optional<double>)> cb)
	{
		_hoverInfoCallback = cb;
	}
private:
	void resetColours();
	struct DataPoint
	{
		glm::vec2 point;
		std::string label;
		float alpha;
		int pointType = 4;
	};

	void addAxes(float width, float height);
	void addAxisTicks(int axis, float width, float height);
	void addAxisLabels(int axis, float width, float height);
	void addPoints(float width, float height, int series,
	               std::vector<DataPoint> &line);
	void addScatters(float width, float height);
	void addLines(float width, float height);
	void addLine(float width, float height, int series, 
	             std::vector<DataPoint> &line);
	void determineLimits();
	void loadLine(ThickLine *tl, glm::vec3 &start, const glm::vec3 &move,
	              glm::vec3 colour = {0.2, 0.2, 0.2});
	
	std::map<int, std::vector<DataPoint>> _data;
	std::map<int, glm::vec2> _axisRanges;
	
	std::map<int, std::string> _labels;
	std::map<int, glm::vec3> _colours;
	
	std::vector<Scatter *> _scatters;
	// series each entry of _scatters was built from, same order/length -
	// lets hoverColour()/resetColours() map a Scatter back to the
	// coordinates (if any) set for its series.
	std::vector<int> _scatterSeries;

	// see setSeriesCoordinates()/hoverColour() - keyed the same as _data.
	std::map<int, std::vector<glm::vec3>> _coords;

	// lazily created on first hoverColour() call, BlackYellow scheme -
	// see hoverColour()'s own comment.
	ColourLegend *_legend = nullptr;

	// true once hoverColour(hover=true) has actually recoloured a point -
	// lets resetColours() (called on every debounced clearLabels(), i.e.
	// on essentially every mouse move over the graph) skip touching the
	// scatters' colours at all when hover-distance colouring was never
	// engaged, so callers that never supply setSeriesCoordinates() see no
	// change from before this feature existed.
	bool _hoverColoured = false;

	// see setHoverInfoCallback()/hoverColour().
	std::function<void(std::optional<double>)> _hoverInfoCallback;

	// see clearLabels()/noteLabelShown() - counts consecutive clearLabels()
	// calls since the last noteLabelShown(), so a lone stray miss (mouse
	// jitter right at a point's own rendered edge) doesn't instantly hide
	// the label the previous, equally valid poll just showed.
	int _missesSinceShown = 0;
};

#endif
