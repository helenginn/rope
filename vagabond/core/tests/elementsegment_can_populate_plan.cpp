#include "../ElementSegment.h"
#include "../AtomGroup.h"
#include "../Atom.h"

int main()
{
	AtomGroup test;
	Atom a;
	test += &a;
	
	ElementSegment segment;
	segment.setDimensions(10, 20, 30); // to allocate _data

	FFT<VoxelElement>::PlanDims dim{};
	dim.nx = 10;
	dim.ny = 20;
	dim.nz = 30;
	
	segment.populatePlan(dim);
	
	return 0;
}
