// vagabond
// Copyright (C) 2019 Helen Ginn
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

#include "SimplexEngine.h"
#include "engine/Task.h"
#include <float.h>
#include <algorithm>
#include <iostream>
#include "MultiSimplex.h"

SimplexEngine::SimplexEngine(RunsEngine *ref) : Engine(ref)
{
	_finish = false;
}

void SimplexEngine::reorderVertices()
{
	std::sort(_points.begin(), _points.end(), std::less<TestPoint &>());
	
}

void SimplexEngine::resetVertex(TestPoint &point)
{
	point.vertex.resize(n());
	for (size_t i = 0; i < point.vertex.size(); i++)
	{
		point.vertex[i] = 0;
	}
	point.eval = FLT_MAX;
	point.tickets.clear();
}

void SimplexEngine::allocateResources()
{
	_points.resize(n() + 1);

	for (size_t i = 0; i < n() + 1; i++)
	{
		resetVertex(_points[i]);
	}

	resetVertex(_centroid);
}

void SimplexEngine::findCentroid()
{
	resetVertex(_centroid);
	float c = 0;

	for (size_t j = 0; j < n(); j++)
	{
		for (size_t i = 0; i < n(); i++)
		{
			_centroid.vertex[i] += _points[j].vertex[i];
		}
		c++;
	}

	for (size_t i = 0; i < n(); i++)
	{
		_centroid.vertex[i] /= (float)c;
	}
}

void SimplexEngine::printPoint(SPoint &point)
{
	for (size_t i = 0; i < point.size(); i++)
	{
		std::cout << point[i] << " ";
	}
}

void SimplexEngine::printSimplex()
{
	std::cout << "==== SIMPLEX ====" << std::endl;
	for (int i = 0; i < pointCount(); i++)
	{
		if (i == 0)
		{
			std::cout << "Best:   ";
		}
		else if (i == pointCount() - 1)
		{
			std::cout << "Worst:  ";
		}
		else
		{
			std::cout << "Point " << i << ": ";
		}

		printPoint(_points[i].vertex);
		std::cout << " ==> " << _points[i].eval << std::endl;
	}
	std::cout << std::endl;
}

bool SimplexEngine::reachedConvergence()
{
	return (_count > _maxRuns || _shrinkCount >= 10 || _finish);
}

SimplexEngine::TestPoint &SimplexEngine::worst()
{
	return _points[_points.size() - 1];
}

SimplexEngine::TestPoint &SimplexEngine::second_worst()
{
	int idx = _points.size() - 2;
	if (idx < 0) idx = 0;
	return _points[idx];
}

void SimplexEngine::requestReflection()
{
	reorderVertices();
//	printSimplex();
	Engine::clearResults();

	findCentroid();
	SPoint trial = scaleThrough(worst().vertex, _centroid.vertex, -1);
	_reflected = trial;
	sendJob(trial);
}

void SimplexEngine::replaceReflection()
{
	worst().vertex = _reflected;
	worst().eval = _lastEval;
}

void SimplexEngine::submitExpansion()
{
	SPoint expanded = scaleThrough(_reflected, _centroid.vertex, -2);
	sendJob(expanded);
	_expanded = expanded;
}

void SimplexEngine::handleExpansion()
{
	getOneResult();
	float next = findBestScore();
	clearResults();

	worst().vertex = (next < _lastEval ? _expanded : _reflected);
	worst().eval = (next < _lastEval ? next : _lastEval);
}

void SimplexEngine::submitContraction(float eval, float worst_score, 
                                      float second_worst_score)
{
	if (eval > second_worst_score && eval < worst_score)
	{
		_contracted = scaleThrough(_reflected, _centroid.vertex, 0.5);
		_compare = eval;
	}
	else 
	{
		_contracted = scaleThrough(worst().vertex, _centroid.vertex, 0.5);
		_compare = worst_score;
	}

	sendJob(_contracted);
}

void SimplexEngine::handleContraction()
{
	float next = FLT_MAX;
	getResults();
	clearResults();

	if (next < _compare)
	{
		worst().vertex = _contracted;
		worst().eval = next;
	}
	else
	{
		submitShrink();
		handleJobs();
	}

}

void SimplexEngine::cycle()
{
	_count = 0;
	_shrinkCount = 0;

	while (true)
	{
		if (reachedConvergence())
		{
			break;
		}

		requestReflection();

		float worst_score = worst().eval;
		float second_worst_score = second_worst().eval;

		TestPoint &best = _points[0];
		float best_score = best.eval;

		getResults();
		_lastEval = Engine::findBestScore();
		clearResults();
		_count++;
		
		if (_lastEval > best_score && _lastEval < second_worst_score)
		{
			replaceReflection();
		}
		else if (_lastEval < best_score)
		{
			submitExpansion();
			handleExpansion();
		}
		else 
		{
			submitContraction(_lastEval, worst_score, second_worst_score);
			handleContraction();
		}
	}
}

void SimplexEngine::collateResults()
{
	std::unique_lock<std::mutex> lock(_mutex);
	for (auto it = _scores.begin(); it != _scores.end(); it++)
	{
		int ticket = it->first;

		for (size_t i = 0; i < _points.size(); i++)
		{
			if (_points[i].tickets.count(ticket) > 0)
			{
				_points[i].eval = it->second.score;
				_points[i].vertex = it->second.vals;
			}
		}
	}

}

void SimplexEngine::handleJobs()
{
	getResults();
	collateResults();
	clearResults();
}

void SimplexEngine::submitShrink()
{
	_shrinkCount++;
	for (size_t i = 0; i < _points.size(); i++)
	{
		SPoint trial = scaleThrough(_points[i].vertex, _centroid.vertex, 0.8);
		int ticket = sendJob(trial);
		_points[i].tickets[ticket] = trial;
	}
}

void SimplexEngine::preRun()
{
	n() = ref()->parameterCount(this);

	if (n() <= 0)
	{
		throw std::runtime_error("Nonsensical dimensions for SimplexEngine");
	}

	if (_maxJobs <= 0)
	{
		throw std::runtime_error("Nonsensical maximum job count per vertex"
		                         "for SimplexEngine");
	}
	
	if (_steps.size() == 0)
	{
		_steps = std::vector<float>(n(), _step);
	}

	clearResults();

	std::vector<float> empty = std::vector<float>(n(), 0);
	sendJob(empty);
}

void SimplexEngine::run()
{
	preRun();
	
	getResults();

	allocateResources();
	sendStartingJobs();
	handleJobs();
	cycle();
	
	sendJob(bestResult());
	getResults();
}

void SimplexEngine::sendStartingJobs()
{
	for (size_t i = 0; i < pointCount(); i++)
	{
		SPoint trial;
		trial.resize(n());

		for (size_t j = 0; j < n(); j++)
		{
			float val = (i == n() - 1 - j) ? _steps[j] : 0;
			trial[j] = val;
		}
		
		int ticket = sendJob(trial);
		
		if (ticket < 0)
		{
			throw(std::runtime_error("::sendJob has not been implemented "
			                              " in SimplexEngine superclass."));
		}
		
		_points[i].tickets[ticket] = trial;
	}
}

SimplexEngine::SPoint SimplexEngine::scaleThrough(SPoint &p, SPoint &q, float k)
{
	SPoint ret = SPoint(n(), 0.);

	for (size_t i = 0; i < n(); i++)
	{
		ret[i] = q[i] + k * (p[i] - q[i]);
	}
	
	return ret;
}

const SimplexEngine::SPoint &SimplexEngine::bestPoint() const
{
	return _points[0].vertex;
}

void SimplexEngine::finish()
{
	_finish = true;
}

void SimplexEngine::handleShrink(MultiEngine *ms, Task<void *, void *> *&first)
{
	Task<void *, void *> *before = nullptr;
	for (int i = 0; i < pointCount(); i++)
	{
		auto job = [this](void *) -> void *
		{
			getOneResult();
			return nullptr;
		};

		auto *next = new Task<void *, void *>(job, "getting job", 
		                                      ms->hanging());
		if (i == 0)
		{
			first = next;
			before = next;
		}
		else
		{
			before->must_complete_before(next);
			before = next;
		}
	}
	
	auto finalise = [this, ms](void *) -> void *
	{
		collateResults();
		clearResults();
		auto *restart =
		new Task<void *, void *>(_cycle, "start new cycle");
		ms->addImmediateTask(restart);
		return nullptr;
	};

	auto *after = new Task<void *, void *>(finalise, "finalising", 
	                                       ms->immediate());
	before->must_complete_before(after);
}

Task<void *, void *> *
SimplexEngine::taskedHandleJobs(Task<void *, void *> *before,
                                MultiEngine *ms)
{
	for (int i = 0; i < pointCount(); i++)
	{
		int total = pointCount();
		auto job = [this, i, total](void *) -> void *
		{
			getOneResult();
			return nullptr;
		};

		auto *next = new Task<void *, void *>(job, "getting job", 
		                                      ms->hanging());
		before->must_complete_before(next);
		before = next;
	}
	
	auto finalise = [this](void *) -> void *
	{
		collateResults();
		clearResults();
		return nullptr;
	};

	auto *after = new Task<void *, void *>(finalise, "finalising", 
	                                       ms->immediate());
	before->must_complete_before(after);
	return after;
}

void SimplexEngine::prepareCycle(MultiEngine *ms)
{	
	_declare_done = [this, ms](void *)
	{
		ms->declareDone(this, bestResult());
		_points.clear();
		_bestResult.clear();
		_bestScore = FLT_MAX;
		return nullptr;
	};

	_handle_expand  = [this, ms](void *) -> void *
	{
		handleExpansion();
		auto *restart =
		new Task<void *, void *>(_cycle, "start new cycle");
		ms->addImmediateTask(restart);
		return nullptr;
	};

	_handle_contract  = [this, ms](void *) -> void *
	{
		getOneResult();
		float next = findBestScore();
		clearResults();

		if (next < _compare)
		{
			worst().vertex = _contracted;
			worst().eval = next;
			auto *restart =
			new Task<void *, void *>(_cycle, "start new cycle");
			ms->addImmediateTask(restart);
		}
		else
		{
			Task<void *, void *> *first = nullptr;
			handleShrink(ms, first);
			ms->addHangingTask(first);
			submitShrink();
			// finish up here
		}

		return nullptr;
	};

	_decide_what_to_do = [this, ms]
	(void *) -> void *
	{
		getOneResult();
		_lastEval = findBestScore();
//		std::cout << "Eval: " << _lastEval << std::endl;
		clearResults(); // in reflected.
		_count++;

		if (_lastEval > _trio.best && _lastEval < _trio.second_worst)
		{
			replaceReflection();
			auto *restart = new Task<void *, void *>
			(_cycle, "start new cycle");
			ms->addImmediateTask(restart);
		}
		else if (_lastEval < _trio.best)
		{
			ms->addHangingTask(new Task<void *, void *>
			                   (_handle_expand,
			                   "handle expansion"));
			submitExpansion();
		}
		else 
		{
			ms->addHangingTask(new Task<void *, void *>
			                   (_handle_contract,
			                   "handle contraction"));

			submitContraction(_lastEval, _trio.worst, 
			                  _trio.second_worst);
		}

		return nullptr;
	};


	_cycle = [this, ms]
	(void *) -> void *
	{
		if (reachedConvergence())
		{
			ms->addImmediateTask(new Task<void *, void *>(_declare_done,
			                                              "declare done"));
		}
		else
		{
			float w = worst().eval;
			float sw = second_worst().eval;
			float b = _points[0].eval;

			_trio = {w, sw, b};

			ms->addHangingTask(new Task<void *, void *>(_decide_what_to_do,
			                                            "decision time"));

			requestReflection();
		}

		return nullptr;
	};
}

void SimplexEngine::taskedCycle(Task<void *, void *> *before, MultiEngine *ms)
{
	auto check_convergence = 
	new Task<void *, void *>(_cycle, "start new cycle",
	                         ms->immediate());

	before->must_complete_before(check_convergence);
}

Task<void *, void *> *SimplexEngine::taskedRun(MultiEngine *ms)
{
	_count = 0;
	_shrinkCount = 0;
	n() = ref()->parameterCount(this);
	allocateResources();
	prepareCycle(ms);

	auto *prerun = new Task<void *, void *>
	([this](void *)
	 {
		preRun();
		return nullptr;
	},
	 "pre-run");

	auto *handle_baseline = new Task<void *, void *>
	([this](void *)
	 {
		getOneResult();
		allocateResources();
		sendStartingJobs();
		return nullptr;
	},
	"handle_baseline");
	
	auto *handleJobs = taskedHandleJobs(handle_baseline, ms);
	taskedCycle(handleJobs, ms);
	
	ms->addHangingTask(handle_baseline);

	return prerun;
	
}

