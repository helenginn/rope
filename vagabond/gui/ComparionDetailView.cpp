#include "ComparisonDetailView.h" 
#include <vagabond/gui/elements/Text.h>
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

void ComparisonDetailView::setup()
{
	_lines.clear();

	switch (_kind)
	{
		case Correlation:
			addTitle("Correlation");
			_lines.push_back("Pearson r = " + fixed(_stats.pearson));
			_lines.push_back("Spearman rho = " + fixed(_stats.spearman));
			_lines.push_back("from " + std::to_string(_stats.pairs.size()) + " residue pairs");
			_lines.push_back("");
			_lines.push_back("r: are the two patterns proportional?");
			_lines.push_back("rho: are the pairs in the same order? (robust to outliers)");
	}		break;
	
}