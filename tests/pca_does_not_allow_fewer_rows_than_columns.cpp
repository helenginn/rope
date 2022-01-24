#include "../src/svd/PCA.h"
#include <stdexcept>

using namespace PCA;

int main()
{
	SVD m, n;
	
	try
	{
		setupSVD(&m, 2, 3);
		return 1;
	}
	catch (std::runtime_error err)
	{

	}

	setupSVD(&n, 3, 2);
	freeSVD(&n);

	return 0;
}
