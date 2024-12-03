#ifndef __vagabond__histogram__
#define __vagabond__histogram__

#include <vector>
#include <climits>
#include <float.h>
#include <climits>

template <typename T>
struct Distribution
{
	float min = FLT_MAX;
	float max = -FLT_MAX;
	float step = 1;
	
	float mid_bin(int i)
	{
		return min + (i + 0.5) * step;
	}
	
	int highest = 0;
	int lowest = INT_MAX;

	std::vector<T> freqs;
};

typedef Distribution<int> Histogram;

void histogram(const std::vector<float> &vals, Histogram &hist);
void get_histogram_limits(const std::vector<float> &vals, Histogram &hist);

#endif
