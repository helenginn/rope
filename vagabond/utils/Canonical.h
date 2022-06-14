// helencore
// Copyright (C) 2019 Helen Ginn
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

#ifndef __helencore__Canonical__
#define __helencore__Canonical__

#include <vector>
#include "svd/PCA.h"

class Canonical
{
public:
	Canonical(int m, int n);
	
	void sizeHint(int n);
	void addVecs(std::vector<double> &ms, std::vector<double> &ns);
	void run();
	double correlation();

	~Canonical();
private:
	void transformVectors(std::vector<double> &vals, int total,
	                      int chosen, PCA::Matrix &basis);
	int _nSamples;
	int _m;
	int _n;
	int _d;
	
	PCA::SVD _mmCC, _nnCC;
	PCA::Matrix _mBasis, _nBasis;
	PCA::Matrix _u, _v;
	bool _run;
	
	std::vector<double> _mVecs;
	std::vector<double> _nVecs;
};

#endif
