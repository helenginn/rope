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

#include <set>
#include <iostream>
#include "Readings.h"
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/Eigen/Sparse>
#include <vagabond/utils/Eigen/SparseQR>

std::map<Reflection::HKL, int> Readings::indexing()
{
	std::set<Reflection::HKL> set;
	for (auto it = this->begin(); it != this->end(); it++)
	{
		std::vector<Locality> &all = it->second;
		for (const Locality &loc : all)
		{
			for (auto jt = loc.begin(); jt != loc.end(); jt++)
			{
				set.insert(jt->first);
			}
		}
		set.insert(it->first);
	}

	std::map<Reflection::HKL, int> indexed;
	int n = 0;
	
	for (const Reflection::HKL &hkl : set)
	{
		indexed[hkl] = n;
		n++;
	}

	return indexed;
}

void Readings::operator+=(const Locality &locality)
{
	(*this)[locality.hkl()].push_back(locality);
}

size_t Readings::localitySize()
{
	size_t tot = 0;
	for (auto it = this->begin(); it != this->end(); it++)
	{
		std::vector<Locality> &all = it->second;
		tot += all.size();
	}
	return tot;
}

//typedef Eigen::SparseMatrix<float,Eigen::RowMajor> SpMat;
typedef Eigen::SparseMatrix<float,Eigen::ColMajor> SpMat;

std::map<Reflection::HKL, Info> Readings::operator()(const Reflection::HKL &ref)
{
	std::vector<Locality> &all = (*this)[ref];
	
	for (const Locality &loc : all)
	{
		float ref_i = loc.at(ref).intensity;
		for (auto it = loc.begin(); it != loc.end(); it++)
		{
			if (it->first == ref)
			{
				continue;
			}
			
			if (sq_dist_from(ref, it->first) <= 1.01)
			{
				float ref_j = it->second.intensity;
				std::cout << it->first << " " << ref_i << " " << ref_j << std::endl;
			}

		}
	}
	
	return {};
}

std::map<Reflection::HKL, Info> Readings::operator()()
{
	std::map<Reflection::HKL, int> indexed = indexing();

	SpMat data(indexed.size(), localitySize());
	data.setZero();

	Eigen::VectorXf results(localitySize());
	
	int n = 0;
	for (auto it = this->begin(); it != this->end(); it++)
	{
		std::vector<Locality> &all = it->second;
		if (all.size() > 1)
		{
		std::cout << "No. localities for " << it->first << ": " << all.size() << std::endl;
		}
		for (const Locality &loc : all)
		{
			float ave = loc.sum() / (float)loc.size();
			float weights = 0;
			for (auto it = loc.begin(); it != loc.end(); it++)
			{
				int idx = indexed.at(it->first);
				float weight = it->second.intensity;
				weights += weight;
				data.insert(idx, n) = weight;
			}

			results(n) = ave * weights;
			n++;
		}
	}

	data.makeCompressed();
	Eigen::SparseQR<SpMat, Eigen::COLAMDOrdering<int>> qr(data.transpose());
	Eigen::VectorXf sol = qr.solve(results.transpose());

	std::map<Reflection::HKL, Info> ret;
	std::cout << "Indexed size: " << indexed.size() << std::endl;
	for (auto it = indexed.begin(); it != indexed.end(); it++)
	{
		ret[it->first] = Info{sol(it->second), 1};
		std::cout << it->first << " " << sol(it->second) << std::endl;
	}
	std::cout << "Returning " << ret.size() << " reflections." << std::endl;
	
	return ret;
}
