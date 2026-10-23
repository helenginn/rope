#include "ComparisonResults.h"

ComparisonResults ComparisonResults::results;

void ComparisonResults::set(const ComparisonStats &stats)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_stats = stats;
	_has = true;
}

bool ComparisonResults::has() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _has;
}

ComparisonStats ComparisonResults::get() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _stats;
}
