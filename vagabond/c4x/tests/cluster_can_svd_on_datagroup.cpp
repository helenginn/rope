#include "../ClusterSVD.h"
#include <iostream>

int main()
{
	size_t length = 3;
	DataGroup<float> dg(length);
	
	dg.addArray("zero", {-4, 0, 4});
	dg.addArray("one", {2, 0, -2});
	dg.addArray("two", {1, 0, -1});
	dg.addArray("three", {-2, 0, 2});
	dg.normalise();
	
	ClusterSVD<DataGroup<float> > cx(dg);
	cx.cluster();

	return 0;
}


