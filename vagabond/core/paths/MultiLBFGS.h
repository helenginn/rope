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

#ifndef __vagabond__MultiLBFGS__
#define __vagabond__MultiLBFGS__

#include "MultiEngine.h"

template <class Key>
class MultiLBFGS;

template <class Key>
class RunsMultiLBFGS
{
public:
	virtual std::map<Key, float> 
	getMultiLBFGSResult(const std::vector<float> &all, 
	                    MultiLBFGS<Key> *caller) = 0;

	virtual void finishedKey(const Key &key) {};

};

template <class Key>
class MultiLBFGS : public RunsEngine, public MultiEngine
{
public:
	MultiLBFGS(RunsMultiLBFGS<Key> *runs, int count)
	{
		_count = count;
		_ref = runs;
	}
	
	~MultiLBFGS()
	{
		do_on_each_engine([](Engine *engine) { delete engine; });
	}
	
	void setMaxRuns(int mr)
	{
		_maxRuns = mr;
	}

	// which key is sensitive to which parameters
	void supplyInfo(const std::map<Key, std::vector<int>> &crucial_info)
	{
		for (auto it = crucial_info.begin(); it != crucial_info.end(); it++)
		{
			if (it->second.size() == 0)
			{
				continue;
			}

			_key2Info[it->first] += it->second;
			for (int idx : it->second)
			{
				if (idx > _count - 1)
				{
					_count = idx + 1;
				}
			}

			if (!_key2Info.at(it->first).engine)
			{
				SimplexEngine *engine = new SimplexEngine(this);
				engine->setMaxRuns(_maxRuns);
				engine->setStepSize(_step);
				_key2Info.at(it->first).engine = engine;
				_engine2Key[engine] = it->first;
				_key2Info.at(it->first).expected.expect_one();
			}
		}
	}
private:
	int _count = 0;

};

#endif
