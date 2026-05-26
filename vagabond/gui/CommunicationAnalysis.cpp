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

#include "CommunicationAnalysis.h"
#include <vagabond/gui/elements/list/LineGroup.h>
#include <vagabond/gui/elements/list/ItemLine.h>
#include <vagabond/gui/elements/ScrollBox.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/gui/MatrixPlot.h>
#include <vagabond/core/protonic/Clique.h>
#include <vagabond/core/protonic/ProbeResult.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/Canonical.h>

using Eigen::seqN;
using Eigen::seq;

CommunicationAnalysis::CommunicationAnalysis(Scene *scene, Clique *clique,
                                             const MatrixXf &mat,
                                             const std::map<ProbeTypePair, 
                                             std::pair<int, int>> &insertions)
: Scene(scene), _clique(clique), _mat(mat), _lookup(insertions)
{

}

void CommunicationAnalysis::prepareGroups()
{
	OpSet<std::string> names = _clique->allCommsNames();
	_parent.setDisplayName("Groups");
	for (const std::string &name : names)
	{
		Item *item = new Item();

		auto job = [this, item, name](bool left)
		{
			if (!left)
			{
				return;
			}
			if (_chosen.count(name))
			{
				_chosen -= name;
				_lg->display(item)->setColour(0.0, 0.0, 0.0);
			}
			else
			{
				_chosen += name;
				_lg->display(item)->setColour(0.4, 0.0, 0.4);
			}
		};

		item->setDisplayName(name);
		item->setSelectable(true);
		item->setSelectJob(job);
		_parent.addItem(item);
	}

	LineGroup *lg = new LineGroup(&_parent, this);
	lg->setLeft(0.1, 0.2);
	_lg = lg;

	ScrollBox *sb = new ScrollBox();
	sb->setContent(lg);
	sb->setBounds(glm::vec4(0.15, 0.0, 0.9, 0.35));
	addObject(sb);

}

float CommunicationAnalysis::compare(const std::string &first,
                                     const std::string &second)
{
	auto check_pair = [this](const ProbeTypePair &ptp, const std::string &check,
	                         OpSet<ProbeTypePair> &which)
	{
		const OpSet<std::string> &descs = _clique->nodeDescsForGroup(check);

		if (descs.count(ptp.first->desc()))
		{
			if (ptp.first->is_bond() && ptp.second == hnet::Types::BondType)
			{
				which += ptp;
			}
			else if (ptp.first->is_atom() && ptp.second == 
			         hnet::Types::ExistenceType)
			{
				which += ptp;
			}
		}
	};
	
	OpSet<ProbeTypePair> lefts{}, rights{};

	for (auto it = _lookup.begin(); it != _lookup.end(); it++)
	{
		const ProbeTypePair &ptp = it->first;
		check_pair(ptp, first, lefts);
		check_pair(ptp, second, rights);
	}
	
	auto add_to_size = [this](int &size)
	{
		return [this, &size](const ProbeTypePair &ptp)
		{
			size += _lookup[ptp].second;
		};
	};
	
	int ln{}, rn{};
	deleteTemps();
	
	auto add_to_matrix = [this](MatrixXf &mat, int &inc, float &drop)
	{
		return [this, &inc, &mat](const ProbeTypePair &ptp)
		{
			int ins = _lookup[ptp].first;
			int dim = _lookup[ptp].second;

			MatrixXf tmp = _wU(Eigen::all, seqN(ins, dim));
			mat(Eigen::all, seqN(inc, dim)) = tmp;

			inc += dim;
		};
	};

	std::for_each(lefts.begin(), lefts.end(), add_to_size(ln));
	std::for_each(rights.begin(), rights.end(), add_to_size(rn));
	
	if (ln == 0 || rn == 0)
	{
		return 0;
	}

	_lMat = MatrixXf(_overOne, ln);
	_lMat.setZero();
	_rMat = MatrixXf(_overOne, rn);
	_rMat.setZero();

	Eigen::MatrixXf cMat = MatrixXf(ln, rn);
	cMat.setZero();
	
	int linc{}, rinc{};
	float drop{};

	std::for_each(lefts.begin(), lefts.end(), add_to_matrix(_lMat, linc, drop));
	std::for_each(rights.begin(), rights.end(), add_to_matrix(_rMat, rinc, drop));
	/*
	std::cout << linc << " and " << drop << std::endl;
	
	std::cout << "Left matrix: " << std::endl;
	std::cout << _lMat << std::endl;
	std::cout << "Right matrix: " << std::endl;
	std::cout << _rMat << std::endl;
	*/
	
	for (int j = 0; j < ln; j++)
	{
		for (int i = 0; i < rn; i++)
		{
			float cc = 0;
			for (int k = 0; k < _overOne; k++)
			{
				cc += _lMat(k, j) * _rMat(k, i);
			}
			cMat(j, i) = cc;
		}
	}

	float sum = 0;
	for (int j = 0; j < ln; j++)
	{
		for (int i = 0; i < ln; i++)
		{
			float diffsq = 0;
//			float lsq = 0;
//			float rsq = 0;
			for (int k = 0; k < rn; k++)
			{
				float contrib = cMat(i, k) - cMat(j, k);
				diffsq += contrib * contrib;
//				lsq += cMat(i, k) * cMat(i, k);
//				rsq += cMat(j, k) * cMat(j, k);
			}
			sum += sqrt(diffsq);
		}
	}
	
	float r = sum;

	auto to_vector = [](MatrixXf &mat, int r)
	{
		std::vector<double> ret(mat.cols());
		for (int i = 0; i < mat.cols(); i++)
		{
			ret[i] = mat(r, i);
		}
		return ret;
	};
	
	Canonical cc(ln, rn);
	cc.sizeHint(_lMat.rows());

	for (int r = 0; r < _lMat.rows(); r++)
	{
		cc.addVecs(to_vector(_lMat, r), to_vector(_rMat, r));
	}
	
	auto get_weight = [this](const int &idx)
	{
		return _w[idx];
	};
	cc.addWeights(get_weight);
	
	float result = 0;
	try
	{
		cc.run();
		result = fabs(cc.correlation());
//		setInformation("Comparing " + first + " to " + second + ", correl: " +
//		               std::to_string(r));
//		std::cout << "Correlation: " << r << std::endl;
	}
	catch (int e)
	{
//		setInformation("Failed correlation");
	}
	return result;
}

void CommunicationAnalysis::svd()
{
	Eigen::JacobiSVD<MatrixXf> svd(_mat, Eigen::ComputeFullU | 
	                               Eigen::ComputeFullV);
	Eigen::MatrixXf u = svd.matrixU().transpose();
	Eigen::VectorXf w = svd.singularValues();
	for (float &f : w)
	{
		f = sqrt(f);
	}
	_w = w;
	_wU = u;
	
	int n = 0;
	for (const float &val : w)
	{
		_wU.row(n) *= val;
		n++;
	}

	n = w.size();
	
	/*
	PCA::Matrix mat = PCA::Matrix(_wU);
	MatrixPlot *mp = new MatrixPlot(mat);
	mp->setCentre(0.75, 0.25);
	addObject(mp);
	*/

	_overOne = n;
}

void CommunicationAnalysis::setup()
{
	addTitle("Communication Analysis");
	
	svd();

	OpSet<std::string> names = _clique->allCommsNames();

	Eigen::MatrixXf mat(names.size(), names.size());
	mat.setZero();

	/*
	auto get_vector = [this](const std::string &name) -> Eigen::VectorXf
	{
		const OpSet<Probe *> &probes = _clique->probes();
		int n = 0;
		auto it = probes.begin();
		while (n < _wU.rows())
		{
			if ((*it)->desc() == name)
			{
				return _wU.row(n);
			}
			n++; it++;
		}
		return Eigen::VectorXf();
	};

	int m = 0;
	for (const std::string &first : names)
	{
		int n = 0;
		for (const std::string &second : names)
		{
			Eigen::VectorXf left = get_vector(first);
			Eigen::VectorXf right = get_vector(second);
			std::vector<float> l = {left.begin(), left.end()};
			std::vector<float> r = {right.begin(), right.end()};

			float cc = correlation(l, r);

			mat(m, n) = cc;
			n++;
		}
		m++;
	}
	*/
	

	int m = 0;
	for (const std::string &first : names)
	{
		int n = 0;
		for (const std::string &second : names)
		{
			float r = compare(first, second);
			mat(m, n) = r;
			n++;
		}
		m++;
	}
	
	Eigen::JacobiSVD<MatrixXf> svd(mat, Eigen::ComputeFullU | 
	                               Eigen::ComputeFullV);
	


	
	Box *box = new Box();

	PCA::Matrix oldmat(mat);
	MatrixPlot *mp = new MatrixPlot(oldmat);
	mp->setCentre(0.0, 0.0);
	box->addObject(mp);
	
	float width = mp->maximalWidth() / 2.f;
	float height = mp->maximalHeight() / 2.f;
	float xstep = width / (float)names.size();
	float ystep = -height / (float)names.size();
	float x = -width / 2 + xstep / 2;
	float y = +height / 2 + ystep / 2;
	
	Renderable::Alignment row_align = Renderable::Alignment::Right;
	Renderable::Alignment col_align = Renderable::Alignment
	(Renderable::Alignment::Left | Renderable::Alignment::Bottom);

	for (const std::string &first : names)
	{
		{
			Text *t = new Text(first);
			t->resize(0.4);
			t->setArbitrary(-width / 2, y, row_align);
			box->addObject(t);
			y += ystep;
		}

		{
			Text *t = new Text(first);
			t->resize(0.4);
			t->setArbitrary(x, -height / 2, col_align);
			glm::mat4x4 base = glm::mat3x3(1.f);
			base[1] /= Window::aspect();
			glm::mat3x3 rot;
			rot = glm::mat3x3(glm::rotate(base, (float)deg2rad(-45),
			                              glm::vec3(0., 0., -1.)));
			t->rotateRoundCentre(rot);
			box->addObject(t);
			x += xstep;
		}
	}

	box->setCentre(0.5, 0.5);
	addObject(box);
}
