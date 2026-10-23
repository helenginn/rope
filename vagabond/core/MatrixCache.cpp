#include "MatrixCache.h"

MatrixCache MatrixCache::slots;

int MatrixCache::fillNext(const SlotMatrix &m)
{
	if (m.vals.size() != m.ids.size() * m.ids.size())
	{
		return -1; // residue list doesn't match
	}

	std::lock_guard<std::mutex> lock(_mutex);
	for (int s = Matrix1; s <= Matrix2; s++)
	{
		if (!_full[s])
		{
			_matrices[s] = m;
			_full[s] = true;
			return s; 
		}
	}
	return -1; 
}

void MatrixCache::clear(Slot s)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_matrices[s] = SlotMatrix{};
	_full[s] = false; 
}

bool MatrixCache::isFull(Slot s) const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _full[s];
}


bool MatrixCache::bothFull() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _full[Matrix1] && _full[Matrix2];
}

SlotMatrix MatrixCache::get(Slot s) const 
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _matrices[s];
	
}











