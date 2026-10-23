#include "ComparisonView.h"
#include "ComparisonDetailView.h"
#include "ComparisonScatterView.h"
#include "AgreementMapView.h"
#include <vagabond/gui/elements/TextButton.h>
#include <iomanip>
#include <sstream>

namespace
{
	std::string fixed(double val, int precision = 2)
	{
		if (!std::isfinite(val))
		{
			return "undefined";
		}
		std::ostringstream ss; 
		ss << std::fixed << std::setprecision(precision) << val;
		return ss.str();
	}
}

ComparisonView::ComparisonView(Scene *prev, const ComparisonStats &stats) : ListView(prev), _stats(stats)
{

}

void ComparisonView::setup()
{
	addTitle("Comparison");

	Text *first = new Text("matrix1: " + _stats.label1);
	first->resize(0.7);
	first->setLeft(0.2, 0.19);
	addObject(first);

	Text *second = new Text("matrix2: " + _stats.label2);
	second->resize(0.7);
	second->setLeft(0.2, 0.23);
	addObject(second);

	ListView::setup();
}

size_t ComparisonView::lineCount()
{
	return EntryCount;
}

Renderable *ComparisonView::getLine(int i)
{
	std::string text;
	switch ((Entry)i)
	{
		case Correlation:
			text = "Correlation: r = " + fixed(_stats.pearson) + ", rho = " + fixed(_stats.spearman);
			break;

		case Common:
			text = "Residue in common: " + std::to_string(_stats.common.size());
			break;

		case ScatterPlot:
			text = "Scatter plot (matrix1 vs matrix2)";
			break;
		case AgreementMap:
			text = "Agreement map (residue x residue)";
			break;

		default: break;
	}

	TextButton *button = new TextButton(text, this);
	button->setReturnTag("entry_" + std::to_string(i));
	return button;

}

void ComparisonView::buttonPressed(std::string tag, Button *button)
{
	std::string end = Button::tagEnd(tag, "entry_");
	if (end.length())
	{
		Scene *view = nullptr;
		switch ((Entry)atoi(end.c_str()))
		{
			case Correlation:
				view = new ComparisonDetailView(this, _stats, ComparisonDetailView::Correlation);
			break;
			
			case Common:
				view = new ComparisonDetailView(this, _stats, ComparisonDetailView::Common);
			break;
			
			case ScatterPlot:
				view = new ComparisonScatterView(this, _stats);
			break;

			case AgreementMap:
				view = new AgreementMapView(this, _stats);
			break;

			default:
			break;
		}

		if (view)
		{
			view->show();
		}
		return;
	}

	ListView::buttonPressed(tag, button);
}


































