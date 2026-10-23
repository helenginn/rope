#ifndef __vagabond__MatrixCache__
#define __vagabond__MatrixCache__

#include <mutex>
#include <string>
#include <vector>
#include "ResidueId.h"

// a residye-by-residue matrix, copied out of the view that made it 
struct SlotMatrix 
{
	std::string label; // view title
	std::vector<ResidueId> ids; // one per row (and column)
	std::vector<double> vals; // ids.size x idz.size
};

// two matrices chosen for comparison (matrix1 and matrix2)
class MatrixCache {
public:
	enum Slot { Matrix1 = 0 , Matrix2 = 1};

	static MatrixCache &instance()
	{
		return slots;
	}

	// put m in the first empty slot, return slot, or -1 if both are full
	int fillNext(const SlotMatrix &m);
	void clear(Slot s);
	bool isFull(Slot s) const;
	bool bothFull() const; 

	// a copy, so the caller can use it after the slot is cleared
	SlotMatrix get(Slot s) const;

private:
	MatrixCache() {};
	MatrixCache(const MatrixCache &) = delete;
	MatrixCache &operator=(const MatrixCache &) = delete;

	static MatrixCache slots;
	mutable std::mutex _mutex;
	SlotMatrix _matrices[2];
	bool _full[2] = {false, false};
};

#endif




















