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

#ifndef __vagabond__MultiEngine__
#define __vagabond__MultiEngine__

#include "engine/ExpectantPhore.h"
#include "SimplexEngine.h"
#include "LBFGSEngine.h"
#include "ResidueId.h"
#include <vagabond/utils/Vec3s.h>
#include <queue>
#include <atomic>

#include "MultiEngineBase.h"

template <class Key, class EngineType> class MultiEngine;

template <class Key, class EngineType>
class RunsMultiEngine
{
public:
	virtual std::map<Key, float> 
	getMultiResult(const std::vector<float> &all, 
	               MultiEngineBase *caller) = 0;

	virtual void finishedKey(const Key &key) {};

};

template <class Key, class EngineType>
class MultiEngine : public RunsEngine, public MultiEngineBase
{
public:
	MultiEngine(RunsMultiEngine<Key, EngineType> *caller, int param_num)
	{
		_num = param_num;
		_ref = caller;
	}
	
	~MultiEngine()
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
		if (_getGradientVector)
		{
			_gradients.resize(_num);
		}

		for (auto it = crucial_info.begin(); it != crucial_info.end(); it++)
		{
			if (it->second.size() == 0)
			{
				continue;
			}

			_key2Info[it->first] += it->second;
			for (int idx : it->second)
			{
				if (idx > _num - 1)
				{
					_num = idx + 1;
				}
			}

			if (!_key2Info.at(it->first).engine)
			{
				const std::vector<int> &idxs = it->second;
				int size = idxs.size();
				EngineType *engine = nullptr;
				engine = new EngineType(this, [size]() { return size; } );

				engine->setGetScoreGetTicket
				([this, engine](int *job_id)
				 {
					return getResult(job_id, engine);
				});
				engine->setSendJobGetTicket
				([this, engine](const std::vector<float> &all)
				 {
					return sendJob(all, engine);
				});
				
				if (_getGradientVector)
				{
					engine->setGetGradientVector
					([this, size, idxs]()
					 {
						std::vector<float> gs(size);
						int i = 0;
						for (const int &idx : idxs)
						{
							gs[i] = _gradients[idx];
							i++;
						}
						return gs;
					 });
				}

				engine->setMaxRuns(_maxRuns);
				engine->setStepSize(_step);
				_key2Info.at(it->first).engine = engine;
				_engine2Key[engine] = it->first;
				_key2Info.at(it->first).expected.expect_one();
			}
		}
	}

	template <typename Func>
	void do_on_each_engine(const Func &func)
	{
		for (auto it = _key2Info.begin(); it != _key2Info.end(); it++)
		{
			if (it->second.engine)
			{
				func(it->second.engine);
			}
		}
	}
	
	void setStepSize(float step)
	{
		_step = step;
	}
	
	void cleanup()
	{
		clearTasks();

		for (auto it = _key2Info.begin(); it != _key2Info.end(); it++)
		{
			it->second.cleanup();
		}
		
		_received = 0;
		_done = 0;
	}

	virtual void run()
	{
		cleanup();

		std::unique_lock<std::mutex> lock(_result);
		std::vector<Task<void *, void *> *> firsts;
		auto start_engine = [this, &firsts](EngineType *engine)
		{
			auto *first = engine->taskedRun(this);
			firsts.push_back(first);
		};

		do_on_each_engine(start_engine);

		auto *mother_task = new Task<void *, void *>([](void *)
		                                             { return nullptr; },
		"mother task");
		for (auto *first : firsts)
		{
			mother_task->must_complete_before(first);
		}

		addImmediateTask(mother_task);
		_cv.wait(lock);

		prepareSendWholeJob();
	}
	
	void checkIfSubmissionComplete()
	{
		while (_received + _done >= _key2Info.size())
		{
			if (_done == _key2Info.size())
			{
				std::unique_lock<std::mutex> lock(_result);
				_cv.notify_one();
				break;
			}

			prepareSendWholeJob();
		}
	}

	void prepareSendWholeJob()
	{
		std::vector<float> everything(_num);
		
		int new_receive = 0;
		for (auto it = _key2Info.begin(); it != _key2Info.end(); it++)
		{
			Info &info = it->second;
			Floats *fs = nullptr;
			
			if (info.jobs.size() > 0)
			{
				fs = &info.jobs.front();
			}
			if (info.done)
			{
				fs = &info.best;
			}

			if (!fs || fs->size() == 0)
			{
				continue;
			}
			
			insert_into_main(everything, info, *fs);

			if (info.jobs.size() > 0)
			{
				info.jobs.pop();

				if (info.jobs.size() > 0)
				{
					new_receive++;
				}
			}
		}
		
		_received = new_receive;
		
		if (_setParameters)
		{
			_setParameters(everything);
		}
		
		// needs to be able to return lots of [results per key]
		std::map<Key, float> map = _ref->getMultiResult(everything, this);

		// prepare gradients now before the mini-engines are notified
		if (_getGradientVector)
		{
			_gradients = _getGradientVector();
		}
		
		for (auto it = map.begin(); it != map.end(); it++)
		{
			if (_key2Info.count(it->first) == 0)
			{
				continue;
			}

			Info &info = _key2Info.at(it->first);
			if (info.done)
			{
				continue;
			}

			double score = it->second;

			std::unique_lock<std::mutex> lock(info.expected.mutex());
			int ticket = info.tickets.front();
			info.tickets.pop();
			setScoreForTicket(ticket, score);
			info.for_collection.push(ticket);
			info.expected.signal_one();
		}
	}

	virtual float getResult(int *job_id, Engine *sender)
	{
		Key key = _engine2Key.at(sender);
		Info &info = _key2Info.at(key);
		info.expected.wait();
		std::unique_lock<std::mutex> lock(info.expected.mutex());
		int ticket = info.for_collection.front();
		info.for_collection.pop();
		float score = scoreForTicket(ticket);
		*job_id = ticket;
		
		return score;
	}
	
	virtual int sendJob(const std::vector<float> &all, 
	                    Engine *sender)
	{
		std::unique_lock<std::mutex> lock(_access);

		Key &key = _engine2Key.at(sender);
		Info &info = _key2Info.at(key);

		if (info.jobs.size() == 0)
		{
			_received++;
		}
		
		int ticket = -1;
		{
			std::unique_lock<std::mutex> lock(info.expected.mutex());
			info.expected.expect_one_already_locked();
			ticket = getNextTicket();
			info.jobs.push({all});
			info.tickets.push(ticket);
		}
		
		checkIfSubmissionComplete();
		
		return ticket;
	}
	
	virtual void declareDone(Engine *sender, const std::vector<float> &best)
	{
		std::unique_lock<std::mutex> lock(_access);
		_done++;
		Key &key = _engine2Key.at(sender);
		_ref->finishedKey(key);
		Info &info = _key2Info.at(key);
		info.best = best;
		info.done = true;
		checkIfSubmissionComplete();
	}
	
	std::map<Key, float> blueprint()
	{
		std::map<Key, float> bp;
		for (auto it = _key2Info.begin(); it != _key2Info.end(); it++)
		{
			const Key &key = it->first;
			bp[key] = 0;
		}
		return bp;
	}

	typedef std::function<void(const std::vector<float> &)> SetParameters;
	typedef std::function<std::vector<float>()> GetGradientVector;

	void setGetGradientVector(const GetGradientVector &job)
	{
		_getGradientVector = job;
	}
	
	void setSetParameters(const SetParameters &job)
	{
		_setParameters = job;
	}
private:
	std::map<Engine *, Key> _engine2Key;

	struct Info
	{
		void operator+=(const std::vector<int> &indices)
		{
			idxs = indices;
		}
		
		void cleanup()
		{
			done = false;
			jobs = {};
			best = {};
			tickets = {};
			for_collection = {};
			expected.reset();
			expected.expect_one();
		}

		bool done = false;
		EngineType *engine = nullptr;
		std::vector<int> idxs;
		std::queue<Floats> jobs;
		Floats best;
		std::queue<int> tickets;
		std::queue<int> for_collection;
		ExpectantPhore expected;
	};

	void insert_into_main(std::vector<float> &everything, 
	                      const MultiEngine<Key, EngineType>::Info 
	                      &info,
	                      const std::vector<float> &fs)
	{
		for (int i = 0; i < info.idxs.size() && i < fs.size(); i++)
		{
			everything[info.idxs[i]] = fs[i];
		}
	}
	

	std::mutex _access;
	std::map<Key, Info> _key2Info;
	int _received = 0;
	int _done = 0;
	int _num = 0;
	int _maxRuns = 10;
	float _step = 1;
	std::condition_variable _cv;
	std::mutex _result;
	RunsMultiEngine<Key, EngineType> *_ref;
	GetGradientVector _getGradientVector{};
	SetParameters _setParameters{};
	std::vector<float> _gradients{};
};

template <class Key>
class MultiSimplex : public MultiEngine<Key, SimplexEngine>
{
public:
	MultiSimplex(RunsMultiEngine<Key, SimplexEngine> *caller, int param_num)
	: MultiEngine<Key, SimplexEngine>(caller, param_num)
	{

	}

};

template <class Key>
class MultiLBFGS : public MultiEngine<Key, LBFGSEngine>
{
public:
	MultiLBFGS(RunsMultiEngine<Key, LBFGSEngine> *caller, int param_num)
	: MultiEngine<Key, LBFGSEngine>(caller, param_num)
	{

	}

};


#endif
