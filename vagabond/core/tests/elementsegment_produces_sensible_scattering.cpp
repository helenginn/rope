#include "../ElementSegment.h"
#include <iostream>

int main()
{
	ElementSegment seg;
	seg.setDimensions(6, 6, 6);
	seg.setElement("N");
	
	int nx, ny, nz;
	seg.limits(nx, ny, nz);

	float last = 0;
	bool ok = true;

	for (int i = -nx; i < nx; i++)
	{
		VoxelElement &ve = seg.element(i, 0, 0);
		float res = seg.resolution(i, 0, 0);
		std::cout << "Res: " << res << ", scatter: " << ve.scatter << std::endl;
		
		if (i < 0 && ve.scatter < last)
		{
			std::cout << "elemental scattering for voxel x=" << i << 
			" did not increase from neighbour" << std::endl;
			ok = false;
		}
		else if (i > 0 && ve.scatter > last)
		{
			std::cout << "elemental scattering for voxel x=" << i << 
			" did not decrease from neighbour" << std::endl;
			ok = false;
		}
		
		last = ve.scatter;
	}
	
	return ok ? 0 : 1;
}
