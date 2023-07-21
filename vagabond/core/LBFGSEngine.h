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

#ifndef __vagabond__LBFGSEngine__
#define __vagabond__LBFGSEngine__

#include "Engine.h"
#include "lbfgs/lbfgs.h"

class LBFGSEngine : public Engine
{
public:
	LBFGSEngine(RunsEngine *ref);
	virtual ~LBFGSEngine() {};

protected:
	virtual void run();

private:
	static lbfgsfloatval_t evaluate(void *engine, const lbfgsfloatval_t *x, 
	                                lbfgsfloatval_t *g, const int n,
	                                const lbfgsfloatval_t step)
	{
		return static_cast<LBFGSEngine *>(engine)->evaluate(x, g, n, step);
	}

	static int progress( void *instance, const lbfgsfloatval_t *x,
	                    const lbfgsfloatval_t *g, 
	                    const lbfgsfloatval_t fx,
	                    const lbfgsfloatval_t xnorm,
	                    const lbfgsfloatval_t gnorm,
	                    const lbfgsfloatval_t step, int n, int k, int ls);

	lbfgsfloatval_t evaluate(const lbfgsfloatval_t *x, lbfgsfloatval_t *g,
	                         const int n, const lbfgsfloatval_t step);

};

#endif
