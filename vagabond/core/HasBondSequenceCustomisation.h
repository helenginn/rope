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
	void setSuperpose(bool superpose)
	{
		_superpose = superpose;
	}

	void setTotalSamples(int total)
	{
		_totalSamples = total;
		
		_mode = (total == 1 ? SingleSample : MultiSample);
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
	
	void transferProperties(HasBondSequenceCustomisation *other)
	{
		other->_ignoreHydrogens = _ignoreHydrogens;
		other->_superpose = _superpose;
		other->_skipSections = _skipSections;
		other->_totalSamples = _totalSamples;
		other->_maxThreads = _maxThreads;
		other->_threads = _threads;
		other->_mode = _mode;
	}
protected:
	bool _ignoreHydrogens = false;
	bool _skipSections = false;
	bool _superpose = true;

	size_t _totalSamples = 0;
	size_t _maxThreads = 0;
	size_t _threads = 0;

	enum SampleMode
	{
		SingleSample,
		MultiSample,
	};

	SampleMode _mode = MultiSample;
};

#endif