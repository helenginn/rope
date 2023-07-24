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

#ifndef __vagabond__AltersNetwork__
#define __vagabond__AltersNetwork__

#include "Responder.h"
#include <functional>
#include <atomic>

class Parameter;
class SpecificNetwork;
template <typename Type> class Mapped;

class AltersNetwork : public HasResponder<Responder<AltersNetwork>>
{
public:
	AltersNetwork(SpecificNetwork *sn, Mapped<float> *mapped,
	std::atomic<bool> &stop, std::atomic<bool> &skip);

	void bindPointValues(int pidx, std::vector<Parameter *> &params);
	void bindPointGradients(const std::vector<int> &idxs, 
	                        std::vector<Parameter *> &params);
protected:
	Mapped<float> *_mapped = nullptr;
	SpecificNetwork *_specified = nullptr;

	typedef std::function<void(std::vector<float> &values)> Getter;
	typedef std::function<void(const std::vector<float> &values)> Setter;

	Getter _getPoints;
	Setter _setPoints;

	std::atomic<bool> &_stop;
	std::atomic<bool> &_skip;
};

#endif
