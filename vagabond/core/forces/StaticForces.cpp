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

#include "Rod.h"
#include "Particle.h"
#include "StaticForces.h"

#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/Eigen/Sparse>
#include <vagabond/utils/Eigen/SparseQR>
#include <vagabond/utils/maths.h>
#include <iostream>
#include <fstream>
#include <time.h>

StaticForces::StaticForces()
{

}

void StaticForces::calculateUnknowns(const std::map<ForceCoordinate, int> 
                                     &index_map)
{
	int rows = _rods.size() * 2 * 3; // relationships per rod
	int cols = index_map.size();

	std::cout << std::endl;
	std::cout << "No. relationships: " << rows << std::endl;
	std::cout << "Degrees of freedom: " << cols << std::endl;
	
	time_t start = time(NULL);

	typedef Eigen::SparseMatrix<float,Eigen::ColMajor> SpMat;

	Eigen::VectorXf targets(rows);
	SpMat weights(rows, cols);
	weights.setZero();

	auto make_insertion_event = [](SpMat &mat, int row)
	{
		return [row, &mat](int idx, float value)
		{
			mat.insert(row, idx) = value;
		};
	};

	int n = 0;
	for (Rod *rod : _rods)
	{
		// adding each coordinate for force equilibrium
		for (int i = 0; i < 3; i++)
		{
			auto insert = make_insertion_event(weights, n);
			rod->forcesEquation(index_map, i, insert, targets(n));
			n++;
		}

		// adding each coordinate for torque + couple equilibrium
		for (int i = 0; i < 3; i++)
		{
			auto insert = make_insertion_event(weights, n);
			rod->torquesEquation(index_map, i, insert, targets(n));
			n++;
		}
	}
	
	std::cout << "==== targets ==== " << std::endl;
	std::cout << targets << std::endl;

	std::cout << "==== weights ==== " << std::endl;
	std::cout << weights << std::endl;
	std::cout << std::endl;

	weights.makeCompressed();
	Eigen::SparseQR<SpMat, Eigen::COLAMDOrdering<int>> qr(weights);
	Eigen::VectorXf results = qr.solve(targets);

//	Eigen::VectorXf results = weights.colPivHouseholderQr().solve(targets);
	
	/*
	std::cout << "==== results ==== " << std::endl;
	std::cout << results << std::endl;
	std::cout << std::endl;
	*/

	Eigen::MatrixXf estimates = weights * results;
	Eigen::MatrixXf display(targets.rows(), 2);
	display(Eigen::all, 0) = targets;
	display(Eigen::all, 1) = estimates;

	std::cout << "==== targets vs estimates ==== " << std::endl;
	
	std::ofstream ffile;
	ffile.open("forces.csv");
	ffile << "target, estimate" << std::endl;
	std::cout << display << std::endl;
	ffile << display << std::endl;
	std::cout << std::endl;
	ffile.close();

	Eigen::MatrixXf diff = estimates - targets;
	
	float f = 0; float g = 0;
	for (int i = 0; i < diff.rows(); i++)
	{
		f += diff(i, 0) * diff(i, 0);
		g += targets(i, 0) * targets(i, 0);
	}
	f /= (float)diff.rows();
	f = sqrt(f);
	g /= (float)diff.rows();
	g = sqrt(g);

	std::cout << "==== remaining errors ==== " << std::endl;
	std::cout << " --> diffs: " << f << std::endl;
	std::cout << " --> targets: " << g << std::endl;
	std::cout << " --> percentage error: " << f / g * 100.f << "%" << std::endl;
	std::cout << std::endl;

	std::map<AbstractForce *, glm::vec3> combined;

	for (auto it = index_map.begin(); it != index_map.end(); it++)
	{
		AbstractForce *force = it->first.first;
		// can assign directly to the magnitude function.
		if (force->status() == AbstractForce::StatusKnownDirOnly)
		{
			float magnitude = results(it->second);
			force->setStatus(AbstractForce::StatusCalculatedDirOnly);
			force->setMagGetter([magnitude]() { return magnitude; });
		}
		else
		{
			int coord = it->first.second;
			combined[force][coord] = results(it->second);
		}
	}

	for (auto it = combined.begin(); it != combined.end(); it++)
	{
		AbstractForce *force = it->first;
		glm::vec3 unit = glm::normalize(it->second);
		float magnitude = glm::length(it->second);

		force->setStatus(AbstractForce::StatusCalculated);
		force->setMagGetter([magnitude]() { return magnitude; });
		force->setUnitGetter([unit]() { return unit; });
		
//		std::cout << it->first << "\t" << it->second << std::endl;
	}
	
	std::ofstream file;
	file.open("stress_strain.csv");
	file << "stress, strain, desc" << std::endl;
	CorrelData data = empty_CD();
	for (Rod *rod : _rods)
	{
		if (rod->category() > 0)
		{
			rod->calculatedAsTension(file, &data);
		}
	}
	float eval = evaluate_CD(data);
	std::cout << std::endl;
	std::cout << "Correlation of bond lengths: " << eval << std::endl;
	std::cout << "Rod stress-strains written to file." << std::endl;
	file.close();
	
	std::ofstream dfile;
	dfile.open("torsion_electromagnetic.csv");
	dfile << "torsion" << std::endl;
	float dots = 0;
	for (Particle *p : _particles)
	{
		bool found;
		float dot =
		p->dotReactionForceAgainst(AbstractForce::ReasonElectrostaticContact,
		                           true, found);
		if (found)
		{
			dfile << dot << " " << p->desc() << std::endl;
		}
	}

	time_t end = time(NULL);
	int seconds = end - start;
	dfile.close();

	std::cout << std::endl;
	std::cout << "Time taken: " << seconds << " s " <<  std::endl;
}

