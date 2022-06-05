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

#ifndef __vagabond__PCA__
#define __vagabond__PCA__

namespace PCA
{
	enum MatrixType
	{
		Distance,
		Correlation
	};

	struct Matrix
	{
		double *vals = nullptr;
		double **ptrs = nullptr;
		int rows = 0;
		int cols = 0;
		
		double *operator[](const int i)
		{
			return ptrs[i];
		}
	};

	struct SVD
	{
		Matrix u{};
		Matrix v{};
		double *w = nullptr;
	};

	struct OrderW
	{
		double w;
		int idx;
	};

	/** allocates internal structure for MxN dimensional matrix.
	 * 	@param *mat pointer to Matrix structure.
	 * 	@param rows number of rows in matrix (slow axis)
	 * 	@param cols number of columns, less than or equal to rows, or 0 for
	 * 	default equal to rows (fast axis) */
	void setupMatrix(Matrix *mat, int rows, int cols = 0);
	void setupSVD(SVD *cc, int rows, int cols = 0);
	void zeroMatrix(Matrix *mat);
	void printMatrix(Matrix *mat);
	void multMatrix(Matrix &mat, double *vector);
	void copyMatrix(Matrix &dest, Matrix &source);
	void reorderSVD(SVD *cc);
	bool invertSVD(SVD *cc);
	void freeMatrix(Matrix *m);
	void freeSVD(SVD *cc);

	Matrix distancesFrom(Matrix &m);

	bool runSVD(SVD *cc);
	bool order_by_w(const OrderW &a, const OrderW &b);

}

#endif
