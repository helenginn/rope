//
//  maths.cpp
//  vagabond
//
//  Created by Helen Ginn on 22/07/2017.
//  Copyright (c) 2017 Strubi. All rights reserved.
//

#include "maths.h"
#include <cmath>
#include <vector>
#include <iostream>

double standard_deviation(std::vector<double> &values) 
{
	double squaredSum = 0;
	double weightSqSum = 0;
	double ave = mean(values);

	for (int i = 0; i < values.size(); i++)
	{
		double value = values[i];

		if (value != value || value == FLT_MAX)
		continue;

		squaredSum += pow(ave - value, 2);

		double weight = 1;

		weightSqSum += weight;
	}

	double stdev = sqrt(squaredSum / weightSqSum);

	return stdev;
}

double r_factor(std::vector<double> &set1, std::vector<double> &set2)
{
	double numerator = 0;
	double denominator = 0;

	for (int i = 0; i < set1.size(); i++)
	{
		if (set1[i] == set1[i] && set2[i] == set2[i])
		{
			double amp_x = (set1[i]);
			double amp_y = (set2[i]);

			numerator += fabs(amp_y - amp_x);
			denominator += fabs(amp_x);
		}
	}

	double rfactor = numerator / denominator;

	return rfactor;
}

double correlation(std::vector<double> &vec1, std::vector<double> &vec2,
                   double cutoff, std::vector<double> *weights)
{
	double sum_x = 0;
	double sum_y = 0;
	double sum_weight = 0;

	if (!vec1.size() || !vec2.size())
	{
		return 0;
	}

	for (int i = 0; i < vec1.size(); i++)
	{
		double weight = 1;
		
		if (weights != NULL)
		{
			weight = (*weights)[i];
		}

		if (vec1[i] != vec1[i] || vec2[i] != vec2[i] || weight != weight)
		{
			continue;
		}

		if (vec2[i] <= cutoff)
		{
			continue;
		}

		sum_x += vec1[i] * weight;
		sum_y += vec2[i] * weight;

		sum_weight += weight;
	}

	double mean_x = sum_x / sum_weight;
	double mean_y = sum_y / sum_weight;

	if (mean_x != mean_x || mean_y != mean_y)
	return 0;

	double sum_x_y_minus_mean_x_y = 0;
	double sum_x_minus_mean_x_sq = 0;
	double sum_y_minus_mean_y_sq = 0;

	for (int i = 0; i < vec1.size(); i++)
	{
		double weight = 1;
		
		if (weights != NULL)
		{
			weight = (*weights)[i];
		}

		if (vec1[i] != vec1[i] || vec2[i] != vec2[i] || weight != weight)
		{
			continue;
		}

		if (vec2[i] <= cutoff)
		{
			continue;
		}

		double addition = (vec1[i] - mean_x) * (vec2[i] - mean_y);
		sum_x_y_minus_mean_x_y += addition * weight;

		addition = pow(vec1[i] - mean_x, 2);
		sum_x_minus_mean_x_sq += addition * weight;

		addition = pow(vec2[i] - mean_y, 2);
		sum_y_minus_mean_y_sq += addition * weight;
	}

	sum_x_y_minus_mean_x_y /= sum_weight;
	sum_x_minus_mean_x_sq /= sum_weight;
	sum_y_minus_mean_y_sq /= sum_weight;

	double r = sum_x_y_minus_mean_x_y
	/ (sqrt(sum_x_minus_mean_x_sq) * sqrt(sum_y_minus_mean_y_sq));

	return r;
}

/* Produces in real space */
void generateResolutionBins(float minD, float maxD,
                            int binCount, std::vector<float> &bins)
{
	bins.reserve(binCount);
	double minRadius = (minD == 0) ? 0 : 1 / minD;
	double maxRadius = 1 / maxD;

	if (maxD <= 0)
	{
		std::cout << "Warning: maximum resolution set to 0. Ignoring.";
		return;
	}

	double maxVolume = pow(maxRadius, 3);
	double minVolume = pow(minRadius, 3);
	double totalVolume = maxVolume - minVolume;

	double eachVolume = totalVolume / binCount;

	double r1 = minRadius;
	double r2 = 0;

	bins.push_back(1 / r1);

	for (int i = 0; i < binCount; i++)
	{
		double r2_cubed = pow(r1, 3) + eachVolume;
		r2 = cbrt(r2_cubed);

		bins.push_back(1 / r2);

		r1 = r2;
	}
}

void regression_line(std::vector<double> xs, std::vector<double> ys,
                     double *intercept, double *gradient, int max)
{
	double sigma_x = 0;
	double sigma_y = 0;
	double sigma_x_y = 0;
	double sigma_x_2 = 0;
	double weight_n = 0;

	for (int i=0; i < xs.size(); i++)
	{
		double x = xs[i];
		double y = ys[i];
		double weight = 1;

		sigma_x += x * weight;
		sigma_y += y * weight;
		sigma_x_y += x * y * weight;
		sigma_x_2 += x * x * weight;
		weight_n += weight;
		
		if (max > 0 && i > max)
		{
			break;
		}
	}

	double mean_x = sigma_x / weight_n;
	double mean_y = sigma_y / weight_n;

	double sxy = sigma_x_y - sigma_x * sigma_y / weight_n;
	double sxx = sigma_x_2 - pow(sigma_x, 2) / weight_n;

	*gradient = sxy / sxx;
	*intercept = mean_y - *gradient * mean_x;
}

void hsv_to_rgb(float &H, float &S, float &V)
{
	if(H > 360 || H < 0 || S > 100 || S < 0 || V > 100 || V < 0)
	{
		return;
	}

	float s = S / 100;
	float v = V / 100;
	float C = s * v;
	float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
	float m = v - C;
	float r,g,b;

	if (H >= 0 && H < 60)
	{
		r = C,g = X,b = 0;
	}
	else if (H >= 60 && H < 120)
	{
		r = X,g = C,b = 0;
	}
	else if (H >= 120 && H < 180)
	{
		r = 0,g = C,b = X;
	}
	else if (H >= 180 && H < 240)
	{
		r = 0,g = X,b = C;
	}
	else if (H >= 240 && H < 300)
	{
		r = X,g = 0,b = C;
	}
	else
	{
		r = C,g = 0,b = X;
	}

	int R = (r + m)*255;
	int G = (g + m)*255;
	int B = (b + m)*255;
	
	H = R / 255.;
	S = G / 255.;
	V = B / 255.;
}

void rgb_to_hsv(float &r, float &g, float &b)
{
	float h, s, v;
	float min, max, delta;

	min = r < g ? r : g;
	min = min  < b ? min  : b;

	max = r > g ? r : g;
	max = max  > b ? max  : b;

	v = max;                                // v
	delta = max - min;
	if (delta < 0.00001)
	{
		r = 0;
		g = 0; // undefined, maybe nan?
		return;
	}
	if( max > 0.0 ) 
	{ 
		s = (delta / max);                  // s
	} 
	else 
	{
		// if max is 0, then r = g = b = 0
		// s = 0, h is undefined
		g = 0.0;
		r = NAN;                            // its now undefined
		return;
	}
	if( r >= max )
	{
		h = ( g - b ) / delta;        // between yellow & magenta
	}
	else if( g >= max )
	{
		h = 2.0 + ( b - r ) / delta;  // between cyan & yellow
	}
	else
	{
		h = 4.0 + ( r - g ) / delta;  // between magenta & cyan
		h *= 60.0;                              // degrees
	}

	if( h < 0.0 )
	{
		h += 360.0;
	}

	r = h;
	g = s;
	b = v;
}
