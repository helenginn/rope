// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __vagabond__HasBondSequenceCustomisation__
#define __vagabond__HasBondSequenceCustomisation__

class HasBondSequenceCustomisation
{
public:
	/** resulting atom distribution should be superimposed on the
	 * initial atom positions from the PDB */
	void setSuperpose(bool superpose)
	{
		_superpose = superpose;
	}
	
	/** grapher should follow the main chain and avoid exploring 
	 * 	branches like disulphides */
	void setInSequence(bool sequence)
	{
		_inSequence = sequence;
	}

	void setTotalSamples(int total)
	{
		_totalSamples = total;
	}
	
	bool skipSections()
	{
		return _skipSections;
	}
	
	void prepareToSkipSections(bool skipSections)
	{
		_skipSections = skipSections;
	}

	/** set whether BondSequences should include hydrogen atoms.
	 *  Ignored after setup() is called. */
	void setIgnoreHydrogens(bool ignore)
	{
		_ignoreHydrogens = ignore;
	}

	/** ignored after setup() is called.
	 * 	@param max must be explicitly specified as non-zero positive */
	void setMaxSimultaneousThreads(size_t max)
	{
		_maxThreads = max;
	}
	
	void setThreads(size_t threads)
	{
		_threads = threads;
	}
	
	void setMaximumLoopCount(size_t loops)
	{
		_loopCount = loops;
	}
	
	void setMaximumJointCount(int max)
	{
		_jointLimit = max;
	}
	
	void transferProperties(HasBondSequenceCustomisation *other)
	{
		other->_ignoreHydrogens = _ignoreHydrogens;
		other->_skipSections = _skipSections;
		other->_totalSamples = _totalSamples;
		other->_inSequence = _inSequence;
		other->_maxThreads = _maxThreads;
		other->_jointLimit = _jointLimit;
		other->_superpose = _superpose;
		other->_loopCount = _loopCount;
		other->_threads = _threads;
	}
	
protected:
	bool _ignoreHydrogens = false;
	bool _skipSections = false;
	bool _inSequence = true;
	bool _superpose = true;
	size_t _loopCount = 1;
	int _jointLimit = -1;

	size_t _totalSamples = 0;
	size_t _maxThreads = 1;
	size_t _threads = 1;
};

#endif
