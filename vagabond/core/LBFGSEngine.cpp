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

#include "MultiEngineBase.h"
#include "LBFGSEngine.h"
#include "lbfgs/WrapLBFGS.h"
#include "engine/Task.h"
#include <stdio.h>
#include <stdexcept>

LBFGSEngine::LBFGSEngine(RunsEngine *ref, 
                         const std::function<int()> &paramCount)
: Engine(ref, paramCount)
{

}

LBFGSEngine::~LBFGSEngine()
{
	delete _wrapped;
}

int LBFGSEngine::progress(void *instance, const lbfgsfloatval_t *x,
                          const lbfgsfloatval_t *g, 
                          const lbfgsfloatval_t fx,
                          const lbfgsfloatval_t xnorm,
                          const lbfgsfloatval_t gnorm,
                          const lbfgsfloatval_t step, int n, int k, int ls)
{
    printf("Iteration %d:\n", k);
    printf("  fx = %f, x[0] = %f, x[1] = %f\n", fx, x[0], x[1]);
    printf("  xnorm = %f, gnorm = %f, step = %f\n", xnorm, gnorm, step);
    printf("\n");

    return 0;
}

std::vector<float> vec_from_lbfgs(const lbfgsfloatval_t *x, const int n)
{
	std::vector<float> ret(n);
	for (size_t i = 0; i < n; i++)
	{
		ret[i] = x[i];
	}
	return ret;
}

lbfgsfloatval_t LBFGSEngine::evaluate(const lbfgsfloatval_t *x,
                                      lbfgsfloatval_t *g,
                                      const int n,
                                      const lbfgsfloatval_t step)
{
	std::vector<float> vals = vec_from_lbfgs(x, n);
	for (float &f : vals)
	{
		f *= _step;
	}
	
	sendJob(vals);
	getOneResult();
	float eval = findBestScore();

	Engine::grabGradients(g, x);

	clearResults();

	return eval;
}

void LBFGSEngine::send(const lbfgsfloatval_t *x, lbfgsfloatval_t *g,
                      const int n, const lbfgsfloatval_t step)
{
	std::vector<float> vals = vec_from_lbfgs(x, n);
	for (float &f : vals)
	{
		f *= _step;
	}
	
	sendJob(vals);
}

float LBFGSEngine::receive(const lbfgsfloatval_t *x, lbfgsfloatval_t *g)
{
	getOneResult();
	float eval = findBestScore();
	Engine::grabGradients(g, x);
	clearResults();
	return eval;
}

void LBFGSEngine::preRun()
{
	n() = _paramCount();

	if (n() <= 0)
	{
		throw std::runtime_error("Nonsensical dimensions for LBFGSEngine");
	}

	clearResults();

	std::vector<float> empty = std::vector<float>(n(), 0);
	sendJob(empty);

}

void LBFGSEngine::reset()
{
	_vals.clear();
	_score = FLT_MAX;
}

void LBFGSEngine::run()
{
	preRun();
	getOneResult();

	lbfgs_parameter_t param;
	
	lbfgs_parameter_init(&param);
	param.max_iterations = 10;
	param.max_linesearch = 10;
	
	float endScore = 0;
	std::vector<float> vals(n());

	lbfgs(vals.size(), &vals[0], &endScore, &LBFGSEngine::evaluate,
	      nullptr, this, &param);
}

// returns the "pre-run" task, but can also set up all other tasks
Task<void *, void *> *LBFGSEngine::taskedRun(MultiEngineBase *ms)
{
	n() = _paramCount();
	_vals = std::vector<float>(n(), 0);

	lbfgs_parameter_t param;
	lbfgs_parameter_init(&param);
	param.max_iterations = 10;
	param.max_linesearch = 10;
	
	auto send_to_me = [this](void *instance, const lbfgsfloatval_t *x,
	                         lbfgsfloatval_t *g, const int n,
	                         const lbfgsfloatval_t step)
	{
		this->send(x, g, n, step);
		return 0;
	};
	
	auto receive_by_me = [this](void *instance, const lbfgsfloatval_t *x,
	                            lbfgsfloatval_t *g)
	{
		return this->receive(x, g);
	};

	_wrapped = new WrapLBFGS(_vals.size(), &_vals[0], &_score, 
	                         send_to_me, receive_by_me, 
	                         this, &param);

	auto *prerun = new Task<void *, void *>
	([this](void *)
	 {
		_wrapped->initialise();
		return nullptr;
	},
	 "pre-run");

	// this can start because it's going to hang on the "getOneResult"
	// until something comes back anyway (when preRun() is called)
	auto *handle_baseline = new Task<void *, void *>
	([this](void *)
	 {
		_wrapped->handleFirstResult();
		_wrapped->computeInitialStep();
		_wrapped->macrocyclePrepare();
		_wrapped->lineSearchPrepare();
		_wrapped->lineSearchCyclePrepare();
		_wrapped->sendCurrent();
		return nullptr;
	},
	"handle_baseline");
	
	auto *handleJobs = _wrapped->taskedRun(ms, this);
	handle_baseline->must_complete_before(handleJobs);

	ms->addHangingTask(handle_baseline); 

	return prerun; // the starting gun is returned to MultiEngine
	
}

