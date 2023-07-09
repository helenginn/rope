#ifndef __vagabond__histogram__
#define __vagabond__histogram__

#include <vector>
#include <climits>
#include <float.h>
#include <climits>

struct Histogram
{
	float min = FLT_MAX;
	float max = -FLT_MAX;
	float step = 1;
	
	int highest = 0;
	int lowest = INT_MAX;

	std::vector<int> freqs;
};

void histogram(const std::vector<float> &vals, Histogram &hist);
void get_histogram_limits(const std::vector<float> &vals, Histogram &hist);

#endif
