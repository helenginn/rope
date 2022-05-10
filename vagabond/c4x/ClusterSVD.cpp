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

#ifndef __vagabond__ClusterSVD__cpp__
#define __vagabond__ClusterSVD__cpp__

#include "Cluster.h"
#include <iostream>

template <class DG>
ClusterSVD<DG>::ClusterSVD(DG &dg) : Cluster<DG>(dg)
{

}

template <class DG>
ClusterSVD<DG>::~ClusterSVD()
{
	freeSVD(&_svd);
}

template <class DG>
PCA::Matrix ClusterSVD<DG>::matrix()
{
	if (_type == PCA::Distance)
	{
		return this->_dg.distanceMatrix();
	}
	else 
	{
		return this->_dg.correlationMatrix();
	}
	
}

template <class DG>
void ClusterSVD<DG>::cluster()
{
	PCA::Matrix mat = matrix();

	setupSVD(&_svd, mat.rows, mat.cols);
	setupMatrix(&this->_result, mat.rows, mat.cols);

	copyMatrix(_svd.u, mat);
	runSVD(&_svd);
	reorderSVD(&_svd);

	copyMatrix(this->_result, _svd.u);
}

#endif
