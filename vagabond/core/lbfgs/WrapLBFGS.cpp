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

#include <math.h>
#include "WrapLBFGS.h"
#include "LBFGSEngine.h"
#include "MultiEngineBase.h"
#include "arithmetic_ansi.h"

WrapLBFGS::WrapLBFGS(int _n, lbfgsfloatval_t *_x, 
                     lbfgsfloatval_t *_ptr_fx,
                     lbfgs_send_t _proc_send,
                     lbfgs_receive_t _proc_receive,
                     void *_instance,
                     lbfgs_parameter_t *_param)
: n(_n), x(_x), ptr_fx(_ptr_fx), proc_send(_proc_send),
proc_receive(_proc_receive), instance(_instance)
{
	/* set parameters to their default values if passed null. */
	(!_param) ? (param = _defparam) : (param = *_param);

	cd.n = n;
	cd.instance = instance;

	errorChecking();
	allocation();
}

WrapLBFGS::~WrapLBFGS()
{
    /* Return the final value of the objective function. */
	if (ptr_fx != NULL)
	{
		*ptr_fx = fx;
	}

	vecfree(pf);

	int m = param.m;

	/* Free memory blocks used by this function. */
	if (lm != NULL)
	{
		for (int i = 0; i < m; ++i)
		{
			vecfree(lm[i].s);
			vecfree(lm[i].y);
		}
		vecfree(lm);
	}

	vecfree(pg);
	vecfree(w);
	vecfree(d);
	vecfree(gp);
	vecfree(g);
	vecfree(xp);
}

void WrapLBFGS::allocation()
{
	xp = (lbfgsfloatval_t*)vecalloc(n * sizeof(lbfgsfloatval_t));
	g = (lbfgsfloatval_t*)vecalloc(n * sizeof(lbfgsfloatval_t));
	gp = (lbfgsfloatval_t*)vecalloc(n * sizeof(lbfgsfloatval_t));
	d = (lbfgsfloatval_t*)vecalloc(n * sizeof(lbfgsfloatval_t));
	w = (lbfgsfloatval_t*)vecalloc(n * sizeof(lbfgsfloatval_t));
	if (xp == NULL || g == NULL || gp == NULL || 
	    d == NULL || w == NULL)
	{
		throw LBFGSERR_OUTOFMEMORY;
	}

	// missing implementation for non-default line searches.
	
	int m = param.m;

	/* Allocate limited memory storage. */
	lm = (iteration_data_t*)vecalloc(m * sizeof(iteration_data_t));
	if (lm == NULL)
	{
		throw LBFGSERR_OUTOFMEMORY;
	}

	/* Initialize the limited memory. */
	for (int i = 0; i < m; ++i) {
		it = &lm[i];
		it->alpha = 0;
		it->ys = 0;
		it->s = 
		(lbfgsfloatval_t*)vecalloc(n * sizeof(lbfgsfloatval_t));
		it->y = 
		(lbfgsfloatval_t*)vecalloc(n * sizeof(lbfgsfloatval_t));

		if (it->s == NULL || it->y == NULL)
		{
			throw LBFGSERR_OUTOFMEMORY;
		}
	}

	/* Allocate an array for storing previous values of the objective function. */
	if (0 < param.past) {
		pf = (lbfgsfloatval_t*)vecalloc(param.past * 
		                                sizeof(lbfgsfloatval_t));
	}
}

void WrapLBFGS::errorChecking()
{
	if (n <= 0) 
	{
		throw LBFGSERR_INVALID_N;
	}

	if (param.epsilon < 0.)
	{
		throw LBFGSERR_INVALID_EPSILON;
	}
	if (param.past < 0)
	{
		throw LBFGSERR_INVALID_TESTPERIOD;
	}
	if (param.delta < 0.)
	{
		throw LBFGSERR_INVALID_DELTA;
	}
	if (param.min_step < 0.)
	{
		throw LBFGSERR_INVALID_MINSTEP;
	}
	if (param.max_step < param.min_step)
	{
		throw LBFGSERR_INVALID_MAXSTEP;
	}
	if (param.ftol < 0.)
	{
		throw LBFGSERR_INVALID_FTOL;
	}
	if (param.linesearch == LBFGS_LINESEARCH_BACKTRACKING_WOLFE ||
	    param.linesearch == LBFGS_LINESEARCH_BACKTRACKING_STRONG_WOLFE)
	{
		if (param.wolfe <= param.ftol || 1. <= param.wolfe)
		{
			throw LBFGSERR_INVALID_WOLFE;
		}
	}
	if (param.gtol < 0.)
	{
		throw LBFGSERR_INVALID_GTOL;
	}
	if (param.xtol < 0.)
	{
		throw LBFGSERR_INVALID_XTOL;
	}
	if (param.max_linesearch <= 0)
	{
		throw LBFGSERR_INVALID_MAXLINESEARCH;
	}
	if (param.orthantwise_c < 0.)
	{
		throw LBFGSERR_INVALID_ORTHANTWISE;
	}
	if (param.orthantwise_start < 0 || n < param.orthantwise_start)
	{
		throw LBFGSERR_INVALID_ORTHANTWISE_START;
	}
	if (param.orthantwise_end < 0)
	{
		param.orthantwise_end = n;
	}
	if (n < param.orthantwise_end)
	{
		throw LBFGSERR_INVALID_ORTHANTWISE_END;
	}
	// missing implementation for non-default line searches.
}

int WrapLBFGS::run()
{
	try
	{
		initialise();
		handleFirstResult();
		computeInitialStep();
		macrocycle();
	}
	catch (const int &err)
	{
		return err;
	}

	return LBFGS_SUCCESS;
}

void WrapLBFGS::initialise()
{
	sendJob(0.f);
}

void WrapLBFGS::sendCurrent()
{
	sendJob(step);
}

void WrapLBFGS::sendJob(float one_step)
{
	proc_send(cd.instance, x, g, cd.n, one_step);
}

void WrapLBFGS::receiveJob()
{
	fx = proc_receive(cd.instance, x, g);
}

void WrapLBFGS::handleFirstResult()
{
	receiveJob();

	/* Store the initial value of the objective function. */
	if (pf != NULL) {
		pf[0] = fx;
	}
}

void WrapLBFGS::computeInitialStep()
{
	/*
	Compute the direction;
	we assume the initial hessian matrix H_0 as the identity matrix.
	*/
	if (param.orthantwise_c == 0.) {
		vecncpy(d, g, n);
	} else {
		vecncpy(d, pg, n);
	}

	/*
	Make sure that the initial variables are not a minimizer.
	*/
	vec2norm(&xnorm, x, n);
	if (param.orthantwise_c == 0.) {
		vec2norm(&gnorm, g, n);
	} else {
		vec2norm(&gnorm, pg, n);
	}
	if (xnorm < 1.0) xnorm = 1.0;
	if (gnorm / xnorm <= param.epsilon)
	{
		throw LBFGS_ALREADY_MINIMIZED;
	}

	/* Compute the initial step:
	step = 1.0 / sqrt(vecdot(d, d, n))
	*/
	vec2norminv(&step, d, n);
}

void WrapLBFGS::macrocycle()
{
	for (;;) {
		/* Store the current position and gradient vectors. */
		macrocyclePrepare();

		/* Search for an optimal step. */
		ls = lineSearch();

		macrocycleHandle();
	}
}

int WrapLBFGS::lineSearch()
{
	lineSearchPrepare();

	int ret = lineSearchCycle();

    return ret;
}

int WrapLBFGS::lineSearchCycle()
{
	for (;;)
	{
		lineSearchCyclePrepare();

        /* Evaluate the function and gradient values. */
		sendCurrent();
		receiveJob();

		int ret = lineSearchCycleHandle();

		if (ret != LBFGS_SUCCESS)
		{
			return ret;
		}
    }

    return LBFGSERR_LOGICERROR;
}

void WrapLBFGS::macrocyclePrepare()
{
	veccpy(xp, x, n);
	veccpy(gp, g, n);

}

void WrapLBFGS::macrocycleHandle()
{
	int m = param.m;
	if (ls < 0) {
		/* Revert to the previous point. */
		veccpy(x, xp, n);
		veccpy(g, gp, n);
		throw ls;
	}

	/* Compute x and g norms. */
	vec2norm(&xnorm, x, n);
	vec2norm(&gnorm, g, n);

	/* Don't report the progress. */

	/* Convergence test.
The criterion is given by the following formula:
	|g(x)| / \max(1, |x|) < \epsilon
	*/
	if (xnorm < 1.0) xnorm = 1.0;
	if (gnorm / xnorm <= param.epsilon) {
		/* Convergence. */
		throw LBFGS_SUCCESS;
	}

	/*
	Test for stopping criterion.
The criterion is given by the following formula:
	(f(past_x) - f(x)) / f(x) < \delta
	*/

	if (pf != NULL) {
		/* We don't test the stopping criterion while k < past. */
		if (param.past <= k) {
			/* Compute the relative improvement from the past. */
			rate = (pf[k % param.past] - fx) / fx;

			/* The stopping criterion. */
			if (rate < param.delta) {
				throw LBFGS_STOP;
			}
		}

		/* Store the current value of the objective function. */
		pf[k % param.past] = fx;
	}

	if (param.max_iterations != 0 && 
	    param.max_iterations < k+1)
	{
		/* Maximum number of iterations. */
		throw LBFGSERR_MAXIMUMITERATION;
	}

	/* Update vectors s and y:
	s_{k+1} = x_{k+1} - x_{k} = \step * d_{k}.
	y_{k+1} = g_{k+1} - g_{k}.
	*/
	it = &lm[end];
	vecdiff(it->s, x, xp, n);
	vecdiff(it->y, g, gp, n);

	/*
Compute scalars ys and yy:
	ys = y^t \cdot s = 1 / \rho.
	yy = y^t \cdot y.
	Notice that yy is used for scaling the hessian matrix 
	H_0 (Cholesky factor).
	*/
	vecdot(&ys, it->y, it->s, n);
	vecdot(&yy, it->y, it->y, n);
	it->ys = ys;

	/*
	Recursive formula to compute dir = -(H \cdot g).
This is described in page 779 of:
	Jorge Nocedal.
	Updating Quasi-Newton Matrices with Limited Storage.
	Mathematics of Computation, Vol. 35, No. 151,
	pp. 773--782, 1980.
	*/
	bound = (m <= k) ? m : k;
	++k;
	end = (end + 1) % m;

	/* Compute the steepest direction. */
	if (param.orthantwise_c == 0.) {
		/* Compute the negative of gradients. */
		vecncpy(d, g, n);
	} else {
		vecncpy(d, pg, n);
	}

	j = end;
	for (int i = 0;i < bound;++i) {
		j = (j + m - 1) % m;    /* if (--j == -1) j = m-1; */
		it = &lm[j];
		/* \alpha_{j} = \rho_{j} s^{t}_{j} \cdot q_{k+1}. */
		vecdot(&it->alpha, it->s, d, n);
		it->alpha /= it->ys;
		/* q_{i} = q_{i+1} - \alpha_{i} y_{i}. */
		vecadd(d, it->y, -it->alpha, n);
	}

	vecscale(d, ys / yy, n);

	for (int i = 0;i < bound;++i) {
		it = &lm[j];
		/* \beta_{j} = \rho_{j} y^t_{j} \cdot \gamma_{i}. */
		vecdot(&beta, it->y, d, n);
		beta /= it->ys;
		/* \gamma_{i+1} = \gamma_{i} + (\alpha_{j} - \beta_{j}) s_{j}. */
		vecadd(d, it->s, it->alpha - beta, n);
		j = (j + 1) % m;        /* if (++j == m) j = 0; */
	}

	/* Now the search direction d is ready. 
	We try step = 1 first.
	*/
	step = 1.0;
}

#define min2(a, b)      ((a) <= (b) ? (a) : (b))
#define max2(a, b)      ((a) >= (b) ? (a) : (b))
#define max3(a, b, c)   max2(max2((a), (b)), (c));

void WrapLBFGS::lineSearchPrepare()
{
	/* Check the input parameters for errors. */
	if (step <= 0.)
	{
		throw LBFGSERR_INVALIDPARAMETERS;
	}

	/* Compute the initial gradient in the search direction. */
	vecdot(&dginit, g, d, n);

	/* Make sure that s points to a descent direction. */
	if (0 < dginit) {
		throw LBFGSERR_INCREASEGRADIENT;
	}

	/* Initialize local variables. */
	brackt = 0;
	stage1 = 1;
	finit = fx;
	dgtest = param.ftol * dginit;
	width = param.max_step - param.min_step;
	prev_width = 2.0 * width;

	/*
	The variables stx, lsfx, dgx contain the values of 
	the step, function, and directional derivative at the 
	best step.  The variables sty, fy, dgy contain the 
	value of the step, function, and derivative at the 
	other endpoint of the interval of uncertainty.
	The variables stp, f, dg contain the values of the step,
	function, and derivative at the current step.
	*/
	stx = sty = 0.;
	lsfx = fy = finit;
	dgx = dgy = dginit;
}


void WrapLBFGS::lineSearchCyclePrepare()
{
	/*
	Set the minimum and maximum steps to correspond to the
	present interval of uncertainty.
	*/
	if (brackt) {
		stmin = min2(stx, sty);
		stmax = max2(stx, sty);
	} else {
		stmin = stx;
		stmax = step + 4.0 * (step - stx);
	}

	/* Clip the step in the range of [stpmin, stpmax]. */
	if (step < param.min_step) step = param.min_step;
	if (param.max_step < step) step = param.max_step;

	/*
	If an unusual termination is to occur then let
	stp be the lowest point obtained so far.
	*/
	if ((brackt && ((step <= stmin || stmax <= step) || param.max_linesearch <= count + 1 || uinfo != 0)) || (brackt && (stmax - stmin <= param.xtol * stmax))) {
		step = stx;
	}

	/*
	Compute the current value of x:
	x <- x + (step) * s.
	*/
	veccpy(x, xp, n);
	vecadd(x, d, step, n);
}

int WrapLBFGS::lineSearchCycleHandle()
{
	vecdot(&dg, g, d, n);
	ftest1 = finit + step * dgtest;
	++count;

	/* Test for errors and convergence. */
	if (brackt && ((step <= stmin || stmax <= step) || uinfo != 0)) {
		/* Rounding errors prevent further progress. */
		return LBFGSERR_ROUNDING_ERROR;
	}
	if (step == param.max_step && fx <= ftest1 && dg <= dgtest) {
		/* The step is the maximum value. */
		return LBFGSERR_MAXIMUMSTEP;
	}
	if (step == param.min_step && (ftest1 < fx || dgtest <= dg)) {
		/* The step is the minimum value. */
		return LBFGSERR_MINIMUMSTEP;
	}
	if (brackt && (stmax - stmin) <= param.xtol * stmax) {
		/* Relative width of the interval of uncertainty is at most xtol. */
		return LBFGSERR_WIDTHTOOSMALL;
	}
	if (param.max_linesearch <= count) {
		/* Maximum number of iteration. */
		return LBFGSERR_MAXIMUMLINESEARCH;
	}
	if (fx <= ftest1 && fabs(dg) <= param.gtol * (-dginit)) {
		/* The sufficient decrease condition and the directional derivative condition hold. */
		return count;
	}

	/*
	In the first stage we seek a step for which the modified
	function has a nonpositive value and nonnegative derivative.
	*/
	if (stage1 && fx <= ftest1 && min2(param.ftol, param.gtol) * dginit <= dg) {
		stage1 = 0;
	}

	/*
	A modified function is used to predict the step only if
	we have not obtained a step for which the modified
	function has a nonpositive function value and nonnegative
	derivative, and if a lower function value has been
	obtained but the decrease is not sufficient.
	*/
	if (stage1 && ftest1 < fx && fx <= lsfx) {
		/* Define the modified function and derivative values. */
		fm = fx - step * dgtest;
		fxm = lsfx - stx * dgtest;
		fym = fy - sty * dgtest;
		dgm = dg - dgtest;
		dgxm = dgx - dgtest;
		dgym = dgy - dgtest;

		/*
		Call update_trial_interval() to update the interval of
		uncertainty and to compute the new step.
		*/
		uinfo = update_trial_interval(
		                              &stx, &fxm, &dgxm,
		                              &sty, &fym, &dgym,
		                              step, &fm, &dgm,
		                              stmin, stmax, &brackt
		                              );

		/* Reset the function and gradient values for f. */
		lsfx = fxm + stx * dgtest;
		fy = fym + sty * dgtest;
		dgx = dgxm + dgtest;
		dgy = dgym + dgtest;
	} else {
		/*
		Call update_trial_interval() to update the interval of
		uncertainty and to compute the new step.
		*/
		uinfo = update_trial_interval(
		                              &stx, &lsfx, &dgx,
		                              &sty, &fy, &dgy,
		                              step, &fx, &dg,
		                              stmin, stmax, &brackt
		                              );
	}

	/*
	Force a sufficient decrease in the interval of uncertainty.
	*/
	if (brackt) {
		if (0.66 * prev_width <= fabs(sty - stx)) {
			step = stx + 0.5 * (sty - stx);
		}
		prev_width = width;
		width = fabs(sty - stx);
	}

	return LBFGS_SUCCESS;
}


/**
 * Define the local variables for computing minimizers.
 */
#define USES_MINIMIZER \
lbfgsfloatval_t a, d, gamma, theta, p, q, r, s;

/**
 * Find a minimizer of an interpolated cubic function.
 *  @param  cm      The minimizer of the interpolated cubic.
 *  @param  u       The value of one point, u.
 *  @param  fu      The value of f(u).
 *  @param  du      The value of f'(u).
 *  @param  v       The value of another point, v.
 *  @param  fv      The value of f(v).
 *  @param  du      The value of f'(v).
 */
#define CUBIC_MINIMIZER(cm, u, fu, du, v, fv, dv) \
    d = (v) - (u); \
    theta = ((fu) - (fv)) * 3 / d + (du) + (dv); \
    p = fabs(theta); \
    q = fabs(du); \
    r = fabs(dv); \
    s = max3(p, q, r); \
    /* gamma = s*sqrt((theta/s)**2 - (du/s) * (dv/s)) */ \
    a = theta / s; \
    gamma = s * sqrt(a * a - ((du) / s) * ((dv) / s)); \
    if ((v) < (u)) gamma = -gamma; \
    p = gamma - (du) + theta; \
    q = gamma - (du) + gamma + (dv); \
    r = p / q; \
    (cm) = (u) + r * d;

/**
 * Find a minimizer of an interpolated cubic function.
 *  @param  cm      The minimizer of the interpolated cubic.
 *  @param  u       The value of one point, u.
 *  @param  fu      The value of f(u).
 *  @param  du      The value of f'(u).
 *  @param  v       The value of another point, v.
 *  @param  fv      The value of f(v).
 *  @param  du      The value of f'(v).
 *  @param  xmin    The maximum value.
 *  @param  xmin    The minimum value.
 */
#define CUBIC_MINIMIZER2(cm, u, fu, du, v, fv, dv, xmin, xmax) \
    d = (v) - (u); \
    theta = ((fu) - (fv)) * 3 / d + (du) + (dv); \
    p = fabs(theta); \
    q = fabs(du); \
    r = fabs(dv); \
    s = max3(p, q, r); \
    /* gamma = s*sqrt((theta/s)**2 - (du/s) * (dv/s)) */ \
    a = theta / s; \
    gamma = s * sqrt(max2(0, a * a - ((du) / s) * ((dv) / s))); \
    if ((u) < (v)) gamma = -gamma; \
    p = gamma - (dv) + theta; \
    q = gamma - (dv) + gamma + (du); \
    r = p / q; \
    if (r < 0. && gamma != 0.) { \
        (cm) = (v) - r * d; \
    } else if (a < 0) { \
        (cm) = (xmax); \
    } else { \
        (cm) = (xmin); \
    }

/**
 * Find a minimizer of an interpolated quadratic function.
 *  @param  qm      The minimizer of the interpolated quadratic.
 *  @param  u       The value of one point, u.
 *  @param  fu      The value of f(u).
 *  @param  du      The value of f'(u).
 *  @param  v       The value of another point, v.
 *  @param  fv      The value of f(v).
 */
#define QUARD_MINIMIZER(qm, u, fu, du, v, fv) \
    a = (v) - (u); \
    (qm) = (u) + (du) / (((fu) - (fv)) / a + (du)) / 2 * a;

/**
 * Find a minimizer of an interpolated quadratic function.
 *  @param  qm      The minimizer of the interpolated quadratic.
 *  @param  u       The value of one point, u.
 *  @param  du      The value of f'(u).
 *  @param  v       The value of another point, v.
 *  @param  dv      The value of f'(v).
 */
#define QUARD_MINIMIZER2(qm, u, du, v, dv) \
    a = (u) - (v); \
    (qm) = (v) + (dv) / ((dv) - (du)) * a;

/**
 * Update a safeguarded trial value and interval for line search.
 *
 *  The parameter x represents the step with the least function value.
 *  The parameter t represents the current step. This function assumes
 *  that the derivative at the point of x in the direction of the step.
 *  If the bracket is set to true, the minimizer has been bracketed in
 *  an interval of uncertainty with endpoints between x and y.
 *
 *  @param  x       The pointer to the value of one endpoint.
 *  @param  fx      The pointer to the value of f(x).
 *  @param  dx      The pointer to the value of f'(x).
 *  @param  y       The pointer to the value of another endpoint.
 *  @param  fy      The pointer to the value of f(y).
 *  @param  dy      The pointer to the value of f'(y).
 *  @param  t       The pointer to the value of the trial value, t.
 *  @param  ft      The pointer to the value of f(t).
 *  @param  dt      The pointer to the value of f'(t).
 *  @param  tmin    The minimum value for the trial value, t.
 *  @param  tmax    The maximum value for the trial value, t.
 *  @param  brackt  The pointer to the predicate if the trial value is
 *                  bracketed.
 *  @retval int     Status value. Zero indicates a normal termination.
 *  
 *  @see
 *      Jorge J. More and David J. Thuente. Line search algorithm with
 *      guaranteed sufficient decrease. ACM Transactions on Mathematical
 *      Software (TOMS), Vol 20, No 3, pp. 286-307, 1994.
 */
int WrapLBFGS::update_trial_interval(
    lbfgsfloatval_t *x,
    lbfgsfloatval_t *fx,
    lbfgsfloatval_t *dx,
    lbfgsfloatval_t *y,
    lbfgsfloatval_t *fy,
    lbfgsfloatval_t *dy,
    lbfgsfloatval_t &t,
    lbfgsfloatval_t *ft,
    lbfgsfloatval_t *dt,
    const lbfgsfloatval_t tmin,
    const lbfgsfloatval_t tmax,
    int *brackt
    )
{
    int bound;
    int dsign = fsigndiff(dt, dx);
    lbfgsfloatval_t mc; /* minimizer of an interpolated cubic. */
    lbfgsfloatval_t mq; /* minimizer of an interpolated quadratic. */
    lbfgsfloatval_t newt;   /* new trial value. */
    USES_MINIMIZER;     /* for CUBIC_MINIMIZER and QUARD_MINIMIZER. */

    /* Check the input parameters for errors. */
    if (*brackt) {
        if (t <= min2(*x, *y) || max2(*x, *y) <= t) {
            /* The trival value t is out of the interval. */
            return LBFGSERR_OUTOFINTERVAL;
        }
        if (0. <= *dx * (t - *x)) {
            /* The function must decrease from x. */
            return LBFGSERR_INCREASEGRADIENT;
        }
        if (tmax < tmin) {
            /* Incorrect tmin and tmax specified. */
            return LBFGSERR_INCORRECT_TMINMAX;
        }
    }

    /*
        Trial value selection.
     */
    if (*fx < *ft) {
        /*
            Case 1: a higher function value.
            The minimum is brackt. If the cubic minimizer is closer
            to x than the quadratic one, the cubic one is taken, else
            the average of the minimizers is taken.
         */
        *brackt = 1;
        bound = 1;
        CUBIC_MINIMIZER(mc, *x, *fx, *dx, t, *ft, *dt);
        QUARD_MINIMIZER(mq, *x, *fx, *dx, t, *ft);
        if (fabs(mc - *x) < fabs(mq - *x)) {
            newt = mc;
        } else {
            newt = mc + 0.5 * (mq - mc);
        }
    } else if (dsign) {
        /*
            Case 2: a lower function value and derivatives of
            opposite sign. The minimum is brackt. If the cubic
            minimizer is closer to x than the quadratic (secant) one,
            the cubic one is taken, else the quadratic one is taken.
         */
        *brackt = 1;
        bound = 0;
        CUBIC_MINIMIZER(mc, *x, *fx, *dx, t, *ft, *dt);
        QUARD_MINIMIZER2(mq, *x, *dx, t, *dt);
        if (fabs(mc - t) > fabs(mq - t)) {
            newt = mc;
        } else {
            newt = mq;
        }
    } else if (fabs(*dt) < fabs(*dx)) {
        /*
            Case 3: a lower function value, derivatives of the
            same sign, and the magnitude of the derivative decreases.
            The cubic minimizer is only used if the cubic tends to
            infinity in the direction of the minimizer or if the minimum
            of the cubic is beyond t. Otherwise the cubic minimizer is
            defined to be either tmin or tmax. The quadratic (secant)
            minimizer is also computed and if the minimum is brackt
            then the the minimizer closest to x is taken, else the one
            farthest away is taken.
         */
        bound = 1;
        CUBIC_MINIMIZER2(mc, *x, *fx, *dx, t, *ft, *dt, tmin, tmax);
        QUARD_MINIMIZER2(mq, *x, *dx, t, *dt);
        if (*brackt) {
            if (fabs(t - mc) < fabs(t - mq)) {
                newt = mc;
            } else {
                newt = mq;
            }
        } else {
            if (fabs(t - mc) > fabs(t - mq)) {
                newt = mc;
            } else {
                newt = mq;
            }
        }
    } else {
        /*
            Case 4: a lower function value, derivatives of the
            same sign, and the magnitude of the derivative does
            not decrease. If the minimum is not brackt, the step
            is either tmin or tmax, else the cubic minimizer is taken.
         */
        bound = 0;
        if (*brackt) {
            CUBIC_MINIMIZER(newt, t, *ft, *dt, *y, *fy, *dy);
        } else if (*x < t) {
            newt = tmax;
        } else {
            newt = tmin;
        }
    }

    /*
        Update the interval of uncertainty. This update does not
        depend on the new step or the case analysis above.

        - Case a: if f(x) < f(t),
            x <- x, y <- t.
        - Case b: if f(t) <= f(x) && f'(t)*f'(x) > 0,
            x <- t, y <- y.
        - Case c: if f(t) <= f(x) && f'(t)*f'(x) < 0, 
            x <- t, y <- x.
     */
    if (*fx < *ft) {
        /* Case a */
        *y = t;
        *fy = *ft;
        *dy = *dt;
    } else {
        /* Case c */
        if (dsign) {
            *y = *x;
            *fy = *fx;
            *dy = *dx;
        }
        /* Cases b and c */
        *x = t;
        *fx = *ft;
        *dx = *dt;
    }

    /* Clip the new trial value in [tmin, tmax]. */
    if (tmax < newt) newt = tmax;
    if (newt < tmin) newt = tmin;

    /*
        Redefine the new trial value if it is close to the upper bound
        of the interval.
     */
    if (*brackt && bound) {
        mq = *x + 0.66 * (*y - *x);
        if (*x < *y) {
            if (mq < newt) newt = mq;
        } else {
            if (newt < mq) newt = mq;
        }
    }

    /* Return the new trial value. */
    t = newt;
    return 0;
}

// returns the "pre-run" task, but can also set up all other tasks
Task<void *, void *> *WrapLBFGS::taskedRun(MultiEngineBase *ms,
                                           LBFGSEngine *engine)
{
	_declare_done = [engine, ms](void *)
	{
		ms->declareDone(engine, engine->vals());
		engine->reset();
		return nullptr;
	};

	auto reset = [this, ms]()
	{
		auto receive = 
		new Task<void *, void *>(_receiveLineSearch,
		                         "receive line search");

		ms->addHangingTask(receive);
		
		sendCurrent();
	};

	_receiveLineSearch = [this, reset, ms](void *) -> void *
	{
		receiveJob();

		ls = lineSearchCycleHandle();

		if (ls < 0) // this was an error
		{
			ms->addImmediateTask
			(new Task<void *, void *>(_declare_done,
			                          "declare done"));
		}
		else if (ls != LBFGS_SUCCESS) // we stopped line searching
		{
			try
			{
				macrocycleHandle();
				macrocyclePrepare();
				lineSearchPrepare();
				lineSearchCyclePrepare();
			}
			catch (...)
			{
				ms->addImmediateTask
				(new Task<void *, void *>(_declare_done,
				                          "declare done"));
				return nullptr;
			}

			reset();
		}
		else // we must search again
		{
			lineSearchCyclePrepare();
			reset();
		}
		return nullptr;
	};

	return new Task<void *, void *>(_receiveLineSearch,
	                                "first receive line search");
}
