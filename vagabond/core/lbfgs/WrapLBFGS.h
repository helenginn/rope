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

#ifndef __vagabond__WrapLBFGS__
#define __vagabond__WrapLBFGS__

#include "lbfgs.h"
#include "../engine/Task.h"

class WrapLBFGS
{
public:
	WrapLBFGS( int n, lbfgsfloatval_t *x, lbfgsfloatval_t *ptr_fx,
	          lbfgs_evaluate_t proc_evaluate,
	          lbfgs_progress_t proc_progress,
	          void *instance,
	          lbfgs_parameter_t *_param);
	~WrapLBFGS();

	int run();
private:
	void errorChecking();
	void allocation();
	void initialise();
	void computeInitialStep();
	void cycle();

	struct tag_callback_data {
		int n;
		void *instance;
		lbfgs_evaluate_t proc_evaluate;
		lbfgs_progress_t proc_progress;
	};
	typedef struct tag_callback_data callback_data_t;

	struct tag_iteration_data {
		lbfgsfloatval_t alpha;
		lbfgsfloatval_t *s;     /* [n] */
		lbfgsfloatval_t *y;     /* [n] */
		lbfgsfloatval_t ys;     /* vecdot(y, s) */
	};
	typedef struct tag_iteration_data iteration_data_t;

	static constexpr lbfgs_parameter_t _defparam = {
		6, 1e-5, 0, 1e-5,
		0, LBFGS_LINESEARCH_DEFAULT, 40,
		1e-20, 1e20, 1e-4, 0.9, 0.9, 1.0e-16,
		0.0, 0, -1,
	};

	// LINE SEARCH PROTOCOLS
	typedef int (*line_search_proc)( int n,
	                                lbfgsfloatval_t *x,
	                                lbfgsfloatval_t *f,
	                                lbfgsfloatval_t *g,
	                                lbfgsfloatval_t *s,
	                                lbfgsfloatval_t *stp,
	                                const lbfgsfloatval_t* xp,
	                                const lbfgsfloatval_t* gp,
	                                lbfgsfloatval_t *wa,
	                                callback_data_t *cd,
	                                const lbfgs_parameter_t *param
	                                );

	int line_search_morethuente( int n,
	                            lbfgsfloatval_t *x,
	                            lbfgsfloatval_t *f,
	                            lbfgsfloatval_t *g,
	                            lbfgsfloatval_t *s,
	                            lbfgsfloatval_t *stp,
	                            const lbfgsfloatval_t* xp,
	                            const lbfgsfloatval_t* gp,
	                            lbfgsfloatval_t *wa,
	                            callback_data_t *cd,
	                            const lbfgs_parameter_t *param
	                            );

	int line_search_cycle(
	                      int n,
	                      lbfgsfloatval_t *x,
	                      lbfgsfloatval_t *f,
	                      lbfgsfloatval_t *g,
	                      lbfgsfloatval_t *s,
	                      lbfgsfloatval_t *stp,
	                      const lbfgsfloatval_t* xp,
	                      const lbfgsfloatval_t* gp,
	                      lbfgsfloatval_t *wa,
	                      callback_data_t *cd,
	                      const lbfgs_parameter_t *param);

	int update_trial_interval( lbfgsfloatval_t *x,
	                          lbfgsfloatval_t *fx,
	                          lbfgsfloatval_t *dx,
	                          lbfgsfloatval_t *y,
	                          lbfgsfloatval_t *fy,
	                          lbfgsfloatval_t *dy,
	                          lbfgsfloatval_t *t,
	                          lbfgsfloatval_t *ft,
	                          lbfgsfloatval_t *dt,
	                          const lbfgsfloatval_t tmin,
	                          const lbfgsfloatval_t tmax,
	                          int *brackt
	                          );

	int n; // number of parameters
	int j = 0; // used in cycle
	int k = 1; // used in cycle
	int bound = 0; // used in cycle
	int end = 0; // used in cycle
	lbfgsfloatval_t *x;
	lbfgsfloatval_t *ptr_fx;
    lbfgs_evaluate_t proc_evaluate;
    lbfgs_progress_t proc_progress;
    void *instance;
    lbfgs_parameter_t param;
	callback_data_t cd;

    lbfgsfloatval_t *xp{};
    lbfgsfloatval_t *g{}, *gp{}, *pg{};
    lbfgsfloatval_t *d{}, *w{}, *pf{};
    iteration_data_t *lm{}, *it{};
    lbfgsfloatval_t ys{}, yy{};
    lbfgsfloatval_t xnorm{}, gnorm{}, beta{};
    lbfgsfloatval_t step{};
    lbfgsfloatval_t fx{};
    lbfgsfloatval_t rate{};

	// used in line search
    int count = 0;
    int brackt, stage1, uinfo = 0;
    lbfgsfloatval_t dg;
    lbfgsfloatval_t stx, lsfx, dgx;
    lbfgsfloatval_t sty, fy, dgy;
    lbfgsfloatval_t fxm, dgxm, fym, dgym, fm, dgm;
    lbfgsfloatval_t finit, ftest1, dginit, dgtest;
    lbfgsfloatval_t width, prev_width;
    lbfgsfloatval_t stmin, stmax;
};

#endif
