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
#include <algorithm>
#include <vector>
#include "svdcmp.h"
#include "matrix.h"
#include <iostream>
#include <stdexcept>

using namespace PCA;

Matrix::Matrix(const Eigen::MatrixXf &matrix)
{
	setupMatrix(this, matrix.rows(), matrix.cols());
	for (int i = 0; i < rows * cols; i++)
	{
		vals[i] = matrix.data()[i];
	}
}

void PCA::setupMatrix(Matrix *mat, int rows, int cols)
{
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
//		throw std::runtime_error("columns specified as less than number of rows");
		rows = cols;
	}
	if (cols == 0)
	{
		cols = rows;
	}

	setupMatrix(&cc->u, rows, cols);
	setupMatrix(&cc->v, cols, cols);
	cc->w = (double *)calloc(cols, sizeof(double));
	cc->N = (double *)calloc(cols, sizeof(double));
}

void PCA::zeroMatrix(Matrix *mat)
{
	memset(mat->vals, '\0', sizeof(mat->vals[0]) * mat->rows * mat->cols);
}

void PCA::copyMatrix(Matrix &dest, const Matrix &source)
{
	if (dest.rows == 0 && dest.cols == 0)
	{
		setupMatrix(&dest, source.rows, source.cols);
	}
	if (dest.rows < source.rows || dest.cols < source.cols)
	{
		throw std::runtime_error("Destination rows/cols not compatible with "
		                         "source matrix.");
	}
	
	if (dest.rows == source.rows && dest.cols == source.cols)
	{
		memcpy(dest.vals, source.vals, 
		       sizeof(double) * source.rows * source.cols);
		return;
	}
	
	for (size_t i = 0; i < source.cols; i++)
	{
		for (size_t j = 0; j < source.rows; j++)
		{
			dest[i][j] = source[i][j];
		}
	}
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
	                     (mat) cc->v.ptrs, (vect *) &(cc->N));

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
	int x = cc->u.rows; // e.g. 1904
	int y = cc->u.cols; // e.g. 13
	
	bool success = runSVD(cc);
	
	for (size_t i = 0; i < x; i++)
	{
		for (size_t j = 0; j < y; j++)
		{
			cc->u[i][j] /= cc->w[j];
			if (cc->w[j] < 1e-6)
			{
				cc->u[i][j] = 0;
			}
		}
	}

	Matrix tmp;
	setupMatrix(&tmp, x, y);
	
	for (size_t i = 0; i < x; i++)
	{
		for (size_t j = 0; j < y; j++)
		{
			for (size_t k = 0; k < y; k++)
			{
				double add = cc->v[j][k] * cc->u[i][k];
				tmp.ptrs[i][j] += add;
			}
		}
	}
	
	for (size_t i = 0; i < x * y; i++)
	{
		cc->u.vals[i] = tmp.vals[i];
	}
	
	freeMatrix(&tmp);
	return success;
}

void PCA::freeMatrix(Matrix *m)
{
	if (m == nullptr)
	{
		return;
	}
	
	free(m->vals);
	free(m->ptrs);
	m->vals = nullptr;
	m->ptrs = nullptr;
	m->rows = 0;
	m->cols = 0;
}

void PCA::freeSVD(SVD *cc)
{
	freeMatrix(&cc->u);
	freeMatrix(&cc->v);

	if (cc->w != nullptr)
	{
		free(cc->w);
	}
	cc->w = nullptr;

	if (cc->N != nullptr)
	{
		free(cc->N);
	}
	cc->N = nullptr;
}

PCA::Matrix PCA::distancesFrom(Matrix &m)
{
	Matrix dists;
	setupMatrix(&dists, m.rows, m.rows);

	for (size_t i = 0; i < dists.rows; i++)
	{
		for (size_t j = 0; j < dists.cols; j++)
		{
			double sq = 0;
			for (size_t k = 0; k < m.cols; k++)
			{
				if (m[i][k] != m[i][k] || m[j][k] != m[j][k])
				{
					continue;
				}

				float add = (m[i][k] - m[j][k]) * (m[i][k] - m[j][k]);
				sq += add;
			}
			
			dists[i][j] = sqrt(sq);
		}
	}
	
	return dists;
}

PCA::Matrix PCA::transpose(Matrix *other)
{
	PCA::Matrix tmp;
	setupMatrix(&tmp, other->cols, other->rows);

	for (size_t i = 0; i < tmp.rows; i++)
	{
		for (size_t j = 0; j < tmp.cols; j++)
		{
			tmp[i][j] = (*other)[j][i];
		}
	}
	
	return tmp;
}

void PCA::multMatrices(const Matrix &first, const Matrix &second, 
                       Matrix &result)
{
	std::string str = "(" + std::to_string(first.rows) + " x " + 
	std::to_string(first.cols) + ")";
	str += " and (" + (std::to_string(second.rows) + " x " + 
	                   std::to_string(second.cols)) + ")";
	str += " to fit (" + (std::to_string(result.rows) + " x " + 
	                      std::to_string(result.cols)) + ")";
	
	if (first.cols != second.rows)
	{
		throw std::runtime_error("Trying to multiply incompatible matrices:" 
		                         + str);
	}
	if (result.cols > 0 && (result.rows != first.rows || 
	                        result.cols != second.cols))
	{
		throw std::runtime_error("Pre-allocated matrix has improper dimensions: "
		                         + str);
	}
	else if (result.cols == 0)
	{
		setupMatrix(&result, first.rows, second.cols);
	}
	else
	{
		zeroMatrix(&result); // prepare for battle
	}

	for (size_t j = 0; j < first.rows; j++)
	{
		for (size_t i = 0; i < second.cols; i++)
		{
			for (size_t k = 0; k < first.cols; k++)
			{
				result[j][i] += first[j][k] * second[k][i];
			}
		}
	}
}

