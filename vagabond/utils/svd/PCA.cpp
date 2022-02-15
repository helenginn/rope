// vagabond
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

#include "PCA.h"
#include <cmath>
#include <algorithm>
#include <cstring>
#include <vector>
#include "svdcmp.h"
#include "matrix.h"
#include <iostream>
#include <stdexcept>

using namespace PCA;

void PCA::setupMatrix(Matrix *mat, int rows, int cols)
{
	cols = rows;

	mat->vals = (double *)calloc(rows * cols, sizeof(double));
	mat->ptrs = (double **)malloc(sizeof(double *) * rows);
	mat->rows = rows;
	mat->cols = cols;
	
	for (size_t i = 0; i < rows; i++)
	{
		mat->ptrs[i] = &mat->vals[i * cols];
	}
}

void PCA::setupSVD(SVD *cc, int rows, int cols)
{
	if (rows < cols && cols != 0)
	{
		throw std::runtime_error("columns specified as less than number of rows");
	}
	
	cols = rows;

	setupMatrix(&cc->u, rows, cols);
	setupMatrix(&cc->v, cols, cols);
	cc->w = (double *)calloc(cols, sizeof(double));
}

void PCA::multMatrix(Matrix &mat, double *vector)
{
	double *ret = (double *)calloc(mat.cols, sizeof(double));

	for (size_t i = 0; i < mat.cols; i++)
	{
		for (size_t j = 0; j < mat.rows; j++)
		{
			ret[i] += mat.ptrs[j][i] * vector[j];
		}
	}
	
	for (size_t i = 0; i < mat.cols; i++)
	{
		vector[i] = ret[i];
	}

	free(ret);
}

void PCA::printMatrix(Matrix *mat)
{
	for (size_t i = 0; i < mat->rows; i++)
	{
		for (size_t j = 0; j < mat->cols; j++)
		{
			std::cout << mat->ptrs[i][j] << " ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;
}

bool PCA::runSVD(SVD *cc)
{
	svdcmp((mat)cc->u.ptrs, cc->u.rows, cc->u.cols, (vect) cc->w, 
	                     (mat) cc->v.ptrs);

	return true;
}

bool PCA::order_by_w(const OrderW &a, const OrderW &b) 
{
	return a.w > b.w;
}

void PCA::reorderSVD(SVD *cc)
{
	std::vector<OrderW> list;

	for (size_t i = 0; i < cc->u.cols; i++)
	{
		OrderW order;
		order.w = cc->w[i];
		order.idx = i;
		list.push_back(order);
	}
	
	std::sort(list.begin(), list.end(), order_by_w);
	
	SVD tmp;
	setupSVD(&tmp, cc->u.rows, cc->u.cols);
	
	for (size_t j = 0; j < cc->u.cols; j++)
	{
		int old_idx = list[j].idx;
		double new_w = list[j].w;
		tmp.w[j] = new_w;

		for (size_t i = 0; i < cc->u.rows; i++)
		{
			tmp.u.ptrs[i][j] = cc->u.ptrs[i][old_idx];
		}
	}

	for (size_t j = 0; j < cc->u.cols; j++)
	{
		int old_idx = list[j].idx;
		for (size_t i = 0; i < cc->u.cols; i++)
		{
			tmp.v.ptrs[i][j] = cc->v.ptrs[i][old_idx];
		}
	}

	for (size_t j = 0; j < cc->u.cols; j++)
	{
		cc->w[j] = tmp.w[j];
		for (size_t i = 0; i < cc->u.rows; i++)
		{
			cc->u.ptrs[i][j] = tmp.u.ptrs[i][j];
		}
	}
	
	for (size_t i = 0; i < cc->u.cols; i++)
	{
		for (size_t j = 0; j < cc->u.cols; j++)
		{
			cc->v.ptrs[i][j] = tmp.v.ptrs[i][j];
		}
	}
	
	freeSVD(&tmp);
}

bool PCA::invertSVD(SVD *cc)
{
	int x = cc->u.rows;

	bool success = runSVD(cc);

	for (size_t j = 0; j < x; j++)
	{
		for (size_t i = 0; i < x; i++)
		{
			cc->u.ptrs[j][i] /= cc->w[i];
		}
	}

	Matrix tmp;
	setupMatrix(&tmp, x);
	
	for (size_t i = 0; i < x; i++)
	{
		for (size_t j = 0; j < x; j++)
		{
			if (cc->w[i] < 1e-6)
			{
				tmp.ptrs[i][j] = 0;
			}
			else
			{
				for (size_t k = 0; k < x; k++)
				{
					double add = cc->v.ptrs[i][k] * cc->u.ptrs[j][k];
					tmp.ptrs[i][j] += add;
				}
			}
		}
	}
	
	for (size_t i = 0; i < x * x; i++)
	{
		cc->u.vals[i] = tmp.vals[i];
	}
	
	freeMatrix(&tmp);
	return success;
}

void PCA::freeMatrix(Matrix *m)
{
	free(m->vals);
	free(m->ptrs);
	m->vals = nullptr;
	m->ptrs = nullptr;
}

void PCA::freeSVD(SVD *cc)
{
	freeMatrix(&cc->u);
	freeMatrix(&cc->v);
	free(cc->w);
	cc->w = nullptr;
}
