#include "ComparisonDetailView.h" 
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/TextButton.h>
#include <iomanip>
#include <sstream>

namespace
{
	std::string fixed(double val, int precision = 3)
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

ComparisonDetailView::ComparisonDetailView(Scene *prev, const ComparisonStats &stats, Kind kind) : ListView(prev), _kind(kind), _stats(stats)
{

}

std::string ComparisonDetailView::helpText()
{
	switch (_kind)
	{
		case Correlation:
			return "r (Pearson): are the two patterns proportional?\n"
  				"1 = same shape, 0 = unrelated, -1 = opposite.\n"
				"rho (Spearman): are the pairs in the same order?\n"
				"It is robust to outliers: if rho is much larger\n"
				"than r, a few residue pairs distort r.";
		case Common:
			return "Residues present in both matrices, matched by\n"
				"residue number (chain letters may differ).\n"
				"Only these residues are compared.";

		default: 
			return "";

	}
}

void ComparisonDetailView::makeMenu()
{
	TextButton *text = new TextButton("Menu", this);
	text->setRight(0.95, 0.1);
	addObject(text);

	text->setReturnJob([this, text]()
	{
		Menu *m = new Menu(this);
		m->addOption("Help", [this]()
		{
			setModal(new BadChoice(this, helpText()));
		});
		m->setup(text);
		setModal(m);
	});
}

void ComparisonDetailView::setup()
{
	_lines.clear();

	switch (_kind)
	{
		case Correlation:
		{
			addTitle("Correlation");
			_lines.push_back("Pearson r = " + fixed(_stats.pearson));
			_lines.push_back("Spearman rho = " + fixed(_stats.spearman));
			_lines.push_back("from " + std::to_string(_stats.pairs.size()) + " residue pairs");
		}
		break;

		case Common:
		{
			addTitle("Residue in common");
			std::string line;
			for (size_t i = 0; i < _stats.common.size(); i++)
			{
				line += _stats.common[i].str() + " ";
				if (i % 10 ==9)
				{
					_lines.push_back(line);
					line = "";
				}
			}
			if (line.length())
			{
				_lines.push_back(line);
			}
		}
		break;


		deault: 
		break;
	}		

	if (_lines.empty())
	{
		_lines.push_back("(none)");
	}

	ListView::setup();
}


size_t ComparisonDetailView::lineCount()
{
	return _lines.size();
}

Renderable *ComparisonDetailView::getLine(int i)
{
	Text *text = new Text(_lines[i]);
	return text;
}


























