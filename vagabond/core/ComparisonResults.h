#ifndef __vagabond__ComparisonResults__
#define __vagabond__ComparisonResults__

#include <mutex>
#include "CompareMatrices.h"


class ComparisonResults
{
public:
	static ComparisonResults &instance()
	{
		return results;
	}

	void set(const ComparisonStats &stats);
	bool has() const;
	// keep a a copy so a view keeps its resutls even if a new 
	// comparison finishes
	ComparisonStats get() const; 
private:
	ComparisonResults() {};
	ComparisonResults(const ComparisonResults &) = delete;
	ComparisonResults &operator=(const ComparisonResults &) = delete;

	static ComparisonResults results;
	mutable std::mutex _mutex;
	ComparisonStats _stats;
	bool _has = false;
};

#endif