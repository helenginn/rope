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

#ifndef __vagabond__OriginGrid__cpp__
#define __vagabond__OriginGrid__cpp__

#include "OriginGrid.h"

/* 11-point interpolation - attempted transcription from Dave's function
 * from GAP */
template <class T>
float OriginGrid<T>::interpolate(glm::vec3 real)
{
	real2Voxel(real);
	glm::vec3 &vox000 = real;

	/* vox000 has integer and real components */
	this->collapse(vox000.x, vox000.y, vox000.z);
	
	/* Pick out just the real components - this is faster
	 * than modf */
	glm::vec3 uvw = glm::vec3(vox000.x - floor(vox000.x),
	                          vox000.y - floor(vox000.y),
	                          vox000.z - floor(vox000.z));

	/* Extra refers to the additional index to be fished
	 * for 11-point interpolation. We already get 0 and 1. */
	int extra[3] = {-1, -1, -1};
	int central[3] = {0, 0, 0};
	int next[3] = {1, 1, 1};
	
	/* If uvw components are greater than 0.5, then flip them 
	 * make the extra index one ahead and reverse the order */
	for (int i = 0; i < 3; i++)
	{
		if (*(&uvw.x + i) > 0.5)
		{
			extra[i] = 2;
			central[i] = 1;
			next[i] = 0;
			*(&uvw.x + i) = 1 - *(&uvw.x + i);
		}
	}

	int vox000x = vox000.x + central[0];
	int vox000y = vox000.y + central[1];
	int vox000z = vox000.z + central[2];
	int vox000xm = vox000.x + next[0];
	int vox000ym = vox000.y + next[1];
	int vox000zm = vox000.z + next[2];
	int vox000xn = vox000.x + extra[0];
	int vox000yn = vox000.y + extra[1];
	int vox000zn = vox000.z + extra[2];

	this->collapse(vox000x, vox000y, vox000z);
	this->collapse(vox000xm, vox000ym, vox000zm);
	this->collapse(vox000xn, vox000yn, vox000zn);

	vox000y  *= this->nx();
	vox000ym *= this->nx();
	vox000yn *= this->nx();
	vox000z  *= this->nx() * this->ny();
	vox000zm *= this->nx() * this->ny();
	vox000zn *= this->nx() * this->ny();

	long idx000 = vox000x + vox000y + vox000z;
	long idx100 = vox000xm + vox000y + vox000z;
	long idx010 = vox000x + vox000ym + vox000z;
	long idx110 = vox000xm + vox000ym + vox000z;
	long idx001 = vox000x + vox000y + vox000zm;
	long idx101 = vox000xm + vox000y + vox000zm;
	long idx011 = vox000x + vox000ym + vox000zm;
	long idx111 = vox000xm + vox000ym + vox000zm;
	
	long idxn00 = vox000xn + vox000y + vox000z;
	long idx0n0 = vox000x + vox000yn + vox000z;
	long idx00n = vox000x + vox000y + vox000zn;
	
	float u = uvw.x;
	float v = uvw.y;
	float w = uvw.z;
	
	float p000 = this->elementValue(idx000);
	float p001 = this->elementValue(idx001);
	float p010 = this->elementValue(idx010);
	float p011 = this->elementValue(idx011);
	float p100 = this->elementValue(idx100);
	float p101 = this->elementValue(idx101);
	float p110 = this->elementValue(idx110);
	float p111 = this->elementValue(idx111);
	
	float a = p100 - p000;
	float b = p010 - p000;
	float c = p110 - p010;
	float d = p101 - p001;
	
	float pn00 = this->elementValue(idxn00);
	float p0n0 = this->elementValue(idx0n0);
	float p00n = this->elementValue(idx00n);

	float p8value = p000+u*(a+w*(-a+d)+v*((c-a)+w*( a-c-d-p011+p111)))
	+ v*(b+w*(-p001+p011-b))+w*(-p000+p001);
	
	float mod = (p000 - 0.5 * p100 - 0.5 * pn00) * (u - u * u);
	mod += (p000 - 0.5 * p010 - 0.5 * p0n0) * (v - v * v);
	mod += (p000 - 0.5 * p001 - 0.5 * p00n) * (w - w * w);
	
	float p11value = p8value + 0.4 * mod;

	return p11value;
}

#endif
