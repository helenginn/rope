#include "ComparisonScatterView.h"
#include "Graph.h"
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/TextButton.h>
#include <algorithm>
#include <sstream>

namespace
{
	const std::string HelpText = "One dot per residue pair (i, j):\n"
	"x = its value in matrix1, y = its value in matrix2.\n"
	"A narrow, tilted cloud means the matrices agree.\n"
	"Horizontal or vertical streaks are single residues\n"
	"that behave differently in one of the matrices.";
}

ComparisonScatterView::ComparisonScatterView(Scene *prev, const ComparisonStats &stats)
: Scene(prev), _stats(stats)
{
	setExportHandler(this);
}

void ComparisonScatterView::setup()
{
	addTitle("Scatter plot");
	makeMenu();

	Graph *graph = new Graph();
	graph->style = Graph::StyleScatter;

	double min1 = INFINITY, max1 = -INFINITY;
	double min2 = INFINITY, max2 = -INFINITY;
	for (const ComparisonStats::Pair &p : _stats.pairs)
	{
		graph->addPoint(0, p.value1, p.value2);
		min1 = std::min(min1, p.value1); max1 = std::max(max1, p.value1);
		min2 = std::min(min2, p.value2); max2 = std::max(max2, p.value2);
	}

	if (!(max1 > min1))
	{
		graph->setRange('x', (std::isfinite(min1) ? min1 : 0) - 1, (std::isfinite(max1) ? max1 : 0) + 1);
	}
	if (!(max2 > min2))
	{
		graph->setRange('y', (std::isfinite(min2) ? min2 : 0) - 1, (std::isfinite(max2) ? max2 : 0) + 1);
	}

	graph->setAxisLabel('x', "matrix1");
	graph->setAxisLabel('y', "matrix2");
	graph->setup(0.5, 0.5);
	graph->addToGraphPosition(0.5, 0.5);
	addObject(graph);

	Text *first = new Text("matrix1: " + _stats.label1);
	first->resize(0.6);
	first->setLeft(0.05, 0.9);
	addObject(first);

	Text *second = new Text("matrix2: " + _stats.label2);
	second->resize(0.6);
	second->setLeft(0.05, 0.94);
	addObject(second);
}

void ComparisonScatterView::makeMenu()
{
	TextButton *text = new TextButton("Menu", this);
	text->setRight(0.95, 0.1);
	addObject(text);

	text->setReturnJob([this, text]()
	{
		Menu *m = new Menu(this);
		m->addOption("Save as CSV", [this]()
		{
			ExportsCSV::buttonPressed("export");
		});
		m->addOption("Help", [this]()
		{
			setModal(new BadChoice(this, HelpText));
		});
		m->setup(text);
		setModal(m);
	});
}

void ComparisonScatterView::supplyCSV(std::string indicator)
{
	std::ostringstream ss;
	ss << "residue_i, residue_j, matrix1, matrix2\n";
	for (const ComparisonStats::Pair &p : _stats.pairs)
	{
		ss << p.i << "," << p.j << "," << p.value1 << "," << p.value2 << "\n";
	}
	_csv = ss.str();
}

void ComparisonScatterView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "export_csv")
	{
		ExportsCSV::buttonPressed(tag, button);
		return;
	}
	Scene::buttonPressed(tag, button);
}














