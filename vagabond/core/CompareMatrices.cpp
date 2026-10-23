#include "CompareMatrices.h"
#include <algorithm>
#include <fstream>
#include <map>
#include <numeric>
#include <set>

namespace
{
	// residue -> row index, residues that appear twice go into repeated
	std::map<ResidueId, int> indexOf(const SlotMatrix &m, std::set<ResidueId> &repeated)
	{
		std::map<ResidueId, int> index;
		for (size_t i = 0; i < m.ids.size(); i++)
		{
			if (index.count(m.ids[i])) repeated.insert(m.ids[i]);
			else index[m.ids[i]] = i;
		}
		return index;
	}

	double pearson(const std::vector<double> &x, const std::vector<double> &y)
	{
		size_t n = x.size();
		if (n < 2) return NAN;

		double mx = std::accumulate(x.begin(), x.end(), 0.0) / n;
		double my = std::accumulate(y.begin(), y.end(), 0.0) / n;

		double sxx = 0; double syy = 0; double sxy = 0;
		for (size_t k = 0; k < n; k++)
		{
			sxx += (x[k] - mx) * (x[k] - mx);
			syy += (y[k] - my) * (y[k] - my);
			sxy += (x[k] - mx) * (y[k] - my);
		}
		if (sxx <= 0 || syy <= 0) return NAN; 
		return sxy / std::sqrt(sxx * syy);
	}

	// ranks startign at 1, tied values share the average of their ranks
	std::vector<double> ranks(const std::vector<double> &x)
	{
		std::vector<int> order(x.size());
		std::iota(order.begin(), order.end(), 0);
		std::sort(order.begin(), order.end(), [&x](int a, int b)
			{return x[a] < x[b]; });
		
		std::vector<double> r(x.size());
		size_t i = 0;
		while (i < order.size())
		{
			size_t j = i;
			while (j + 1 < order.size() && x[order[j+1]] == x[order[i]]) j++;
			
			for (size_t k = i; k <= j; k++) r[order[k]] = (i + j) / 2.0 + 1;
			i = j + 1;
		}
		return r;
	}

	// for the labels 
	std::string quoted(const std::string &s)
	{
		std::string out = "\"";
		for (char c : s) out += (c == '"') ? std::string("\"\"") : std::string(1, c);
		return out + "\"";
	}
}

CompareMatrices::CompareMatrices(const SlotMatrix &m1, const SlotMatrix &m2): _m1(m1), _m2(m2)
{
	_stats.label1 = m1.label;
	_stats.label2 = m2.label;
}

void CompareMatrices::run()
{
	matchResidues();
	collectPairs();
	calculateScores();
	standardise();
	buildAgreementMap();
}

void CompareMatrices::matchResidues()
{
	std::set<ResidueId> repeated1, repeated2;
	std::map<ResidueId, int> index1 = indexOf(_m1, repeated1);
	std::map<ResidueId, int> index2 = indexOf(_m2, repeated2);

	std::set<ResidueId> all;
	for (auto &p : index1) all.insert(p.first);
	for (auto &p : index2) all.insert(p.first);


	for (const ResidueId &id : all)
	{
		std::string reason;
		if (!index1.count(id)) reason = "not in matrix1";
		else if (!index2.count(id)) reason = "not in matrix2";
		else if (repeated1.count(id)) reason = "residue number repeated in matrix1";
		else if (repeated2.count(id)) reason = "residue number repeated in matrix2";

		if (reason.length())
		{
			_stats.excluded.push_back({id, reason});
			continue;
		}

		_stats.common.push_back(id);
		_rows1.push_back(index1[id]);
		_rows2.push_back(index2[id]);
	}
}

void CompareMatrices::collectPairs()
{
	size_t n1 = _m1.ids.size();
	size_t n2 = _m2.ids.size();
	std::vector<double> x, y;

	for (size_t p = 0; p < _stats.common.size(); p++)
	{
		for (size_t q = p + 1; q < _stats.common.size(); q++)
		{
			double v1 = _m1.vals[_rows1[p] * n1 + _rows1[q]];
			double v2 = _m2.vals[_rows2[p] * n2 + _rows2[q]];
			if (!std::isfinite(v1) || !std::isfinite(v2)) continue;

			_stats.pairs.push_back({_stats.common[p], _stats.common[q], v1, v2});
		}
	}
}

void CompareMatrices::calculateScores()
{
	std::vector<double> x, y;
	for (const ComparisonStats::Pair &p : _stats.pairs)
	{
		x.push_back(p.value1);
		y.push_back(p.value2);
	}
	_stats.pearson = pearson(x,y);
	_stats.spearman = pearson(ranks(x), ranks(y));
}

void CompareMatrices::standardise()
{
	size_t n = _stats.pairs.size();
	if (n == 0) return;

	double mean1 = 0, mean2 = 0;
	for (const ComparisonStats::Pair &p : _stats.pairs)
	{
		mean1 += p.value1;
		mean2 += p.value2;
	}
	mean1 /= n;
	mean2 /= n;

	double var1 = 0, var2 = 0;
	for (const ComparisonStats::Pair &p : _stats.pairs)
	{
		var1 += (p.value1 - mean1) * (p.value1 - mean1);
		var2 += (p.value2 - mean2) * (p.value2 - mean2);
	}
	double sd1 = std::sqrt(var1 / n);
	double sd2 = std::sqrt(var2 / n);

	// constant matric ==> no spread, z-score 0 
	for (ComparisonStats::Pair &p : _stats.pairs)
	{
		p.z1 = (sd1 > 0) ? (p.value1 - mean1) / sd1 : 0;
		p.z2 = (sd2 > 0) ? (p.value2 - mean2) / sd2 : 0;
	} 
}

void CompareMatrices::buildAgreementMap()
{
	size_t n = _stats.common.size();
	_stats.agreement.assign(n * n, NAN);

	std::map<ResidueId, int> position; 
	for (size_t k = 0; k < n; k++)
	{
		position[_stats.common[k]] = k;
	}

	for (const ComparisonStats::Pair &p : _stats.pairs)
	{
		int i = position[p.i];
		int j = position[p.j];
		double v = p.z1 * p.z2;
		_stats.agreement[i * n + j] = v;
		_stats.agreement[j * n + i] = v;

	}
}



bool CompareMatrices::writeCSVs(const std::string &prefix) const
{
    std::ofstream pairs(prefix + "pairs.csv");
    if (!pairs.is_open())
    {
        return false;
    }

    pairs << "residue_i,residue_j,matrix1,matrix2\n";
    for (const ComparisonStats::Pair &p : stats().pairs)
    {
        pairs << p.i << "," << p.j << "," << p.value1 << "," << p.value2 << "\n";
    }

    return true;
}








