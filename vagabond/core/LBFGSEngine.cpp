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

#include "LBFGSEngine.h"

LBFGSEngine::LBFGSEngine(RunsEngine *ref) : Engine(ref)
{

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
	
	float eval = FLT_MAX;
	sendJob(vals);
	Engine::findBestResult(&eval);

	Engine::grabGradients(g);

	clearResults();

	return eval;
}

void LBFGSEngine::run()
{
	if (n() <= 0)
	{
		throw std::runtime_error("Nonsensical dimensions for LBFGSEngine");
	}

	clearResults();

	std::vector<float> empty = std::vector<float>(n(), 0);
	sendJob(empty);

	getResults();
	float begin = FLT_MAX;
	findBestResult(&begin);

	lbfgs_parameter_t param;
	
	lbfgs_parameter_init(&param);
	param.epsilon = 1e-14;
	param.max_iterations = 10;
}
