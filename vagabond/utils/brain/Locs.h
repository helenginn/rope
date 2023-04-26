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

#ifndef __vagabond__Locs__
#define __vagabond__Locs__

#include <ostream>
#include <iostream>
#include <cstring>

enum FunctionType
{
	FPureLinear,
	FLogSigmoid,
};

enum LearningType
{
	ConstantAlpha,
	VariableAlpha,
};

struct MatrixLoc;

struct VectorLoc
{
	float *ptr;
	size_t size;

	float &operator[](const int i) const
	{
		return ptr[i];
	}

	friend std::ostream &operator<<(std::ostream &ss, const VectorLoc &vl)
	{
		ss << "{";
		for (size_t i = 0; i < vl.size; i++)
		{
			ss << vl.ptr[i];
			if (i < vl.size - 1)
			{
				ss << ", ";
			}
		}
		ss << "}";
		return ss;
	}
	
	VectorLoc &operator*=(const float &constant)
	{
		for (size_t i = 0; i < size; i++)
		{
			ptr[i] *= constant;
		}
		
		return *this;
	}
	
	static void dot_vector(const VectorLoc &input, VectorLoc &output)
	{
		for (size_t i = 0; i < output.size; i++)
		{
			output[i] *= input[i];
		}
	}
	
	static void subtract_vector(const VectorLoc &input, VectorLoc &output)
	{
		for (size_t i = 0; i < output.size; i++)
		{
			output[i] -= input[i];
		}
	}
	
	static void add_vector(const VectorLoc &input, VectorLoc &output)
	{
		for (size_t i = 0; i < output.size; i++)
		{
			output[i] += input[i];
		}
	}
	
	static void copy_vector(const VectorLoc &input, VectorLoc &output)
	{
		memcpy(output.ptr, input.ptr, sizeof(float) * output.size);
	}
};

struct MatrixLoc
{
	float **ptr;
	size_t m;
	size_t n;

	size_t size() const
	{
		return m * n;
	}

	// m = row, n = column
	void populate(int m_, int n_, float *start)
	{
		m = m_; n = n_;
		ptr = (float **)malloc(m * sizeof(float *));
		for (size_t i = 0; i < m; i++)
		{
			ptr[i] = start + (i * n);
		}
	}

	float *operator[](const int i) const
	{
		return ptr[i];
	}

	friend std::ostream &operator<<(std::ostream &ss, const MatrixLoc &ml)
	{
		for (size_t i = 0; i < ml.m; i++)
		{
			ss << (i == 0 ? "{{" : " {");
			for (size_t j = 0; j < ml.n; j++)
			{
				ss << ml.ptr[i][j];
				if (j < ml.n - 1)
				{
					ss << ", ";
				}
			}
			ss << "}";
		}
		ss << "}";
		return ss;
	}
	
	MatrixLoc &operator*=(const float &constant)
	{
		for (size_t i = 0; i < size(); i++)
		{
			ptr[0][i] *= constant;
		}
		
		return *this;
	}

	static void multiply_transpose(const MatrixLoc &mat, 
	                               const VectorLoc &input,
	                               VectorLoc &output)
	{
		for (size_t i = 0; i < mat.n; i++)
		{
			output[i] = 0;
			for (size_t j = 0; j < mat.m; j++)
			{
				output[i] += mat[j][i] * input[j];
			}
		}
	}

	static void subtract_matrix(const MatrixLoc &input, MatrixLoc &output)
	{
		for (size_t i = 0; i < output.size(); i++)
		{
			output[0][i] -= input[0][i];
		}
	}
	

	static void multiply(const MatrixLoc &mat, const VectorLoc &input,
	                VectorLoc &output)
	{
		for (size_t j = 0; j < mat.m; j++)
		{
			output[j] = 0;
			for (size_t i = 0; i < mat.n; i++)
			{
				output[j] += mat[j][i] * input[i];
			}
		}
	}
	
	static void vec_by_transpose_vec(const VectorLoc &first, VectorLoc &second,
	                                 const MatrixLoc &output)
	{
		for (size_t i = 0; i < output.m; i++)
		{
			for (size_t j = 0; j < output.n; j++)
			{
				output[i][j] = first[i] * second[j];
			}
		}
	}
};


#endif
