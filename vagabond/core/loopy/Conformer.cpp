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

#include "ResidueTorsion.h"
#include "Conformer.h"
#include "Parameter.h"
#include "Instance.h"
#include "Loopy.h"
#include "Atom.h"

Conformer::Conformer(Loop *loop) : HasMetadata(), _loop(loop)
{

}

float step_for_parameter_type(ParameterType type)
{
	switch (type)
	{
		case Ramachandran:
		return 72.f;
		break;

		case SideChain:
		return 72.f;
		break;

		case Adjustment:
		return 72.f;
		break;

		case PeptideBond:
		return 2.f;
		break;
		
		default:
		return 0.f;
		break;
	}
}

ParameterType parameter_type(Parameter *param, Loop *loop)
{
	if (!loop->idNeedsClashCheck(param->residueId()))
	{
		/* don't care about parameters far away from the loop */
		return Invalid;
	}
	else if (param->isConstrained())
	{
		return Invalid;
	}
	else if (!loop->idInLoop(param->residueId())
	         && param->coversMainChain() && !param->isPeptideBond())
	{
		/* parameters neighbouring the loop can be used for small adjustment */
		return Adjustment;
	}
	else if (!loop->idInLoop(param->residueId()))
	{
		/* any other parameters in neighbouring residues are invalid */
		return Invalid;
	}
	else if (param->isPeptideBond())
	{
		return PeptideBond;
	}
	else if (param->coversMainChain())
	{
		return Ramachandran;
	}
	else
	{
		return SideChain;
	}
}

ParameterType Conformer::parameterType(Parameter *param)
{
	return parameter_type(param, _loop);
}

bool type_used_for_stage(const ParameterType &type, const LoopStage &stage)
{
	if (type == Invalid)
	{
		return false;
	}

	if (type == Adjustment && (stage == Rough || stage == Adjusting))
	{
		return true;
	}
	
	if (type == PeptideBond && (stage == Rough || stage == Adjusting))
	{
		return true;
	}
	
	if (type == Ramachandran && (stage == Rough || stage == Adjusting))
	{
		return true;
	}
	
	if (type == SideChain && stage == Sidechains)
	{
		return true;
	}
	
	return false;
}

size_t Conformer::validCount()
{
	size_t n = 0;
	for (ParamDetails &details : _details)
	{
		if (details.type != Invalid)
		{
			n++;
		}
	}

	return n;
}

Conformer::ParamDetails::ParamDetails(Parameter *p, int idx, Loop *loop)
{
	param = p;
	type = parameter_type(p, loop);
	index = idx;
	step = step_for_parameter_type(type);
	
	if (!p->isTorsion())
	{
		step = 2.f;
	}

	value = 0;
}

void Conformer::addParameter(Parameter *param, int idx)
{
	ParamDetails details(param, idx, _loop);
	_details.push_back(details);
}

std::vector<float> Conformer::values()
{
	return _values;
}

size_t Conformer::parameterCount()
{
	std::vector<float> tmp;
	_getter(tmp);

	return tmp.size();
}

void Conformer::setValues(const std::vector<float> &values)
{
	_setter(values);
}

void Conformer::randomise(const LoopStage &stage)
{
	if (stage != Rough)
	{
		return;
	}
	
	_values.resize(_details.size());
	_rmsd = FLT_MAX;

	for (ParamDetails &details : _details)
	{
		if (details.type == Ramachandran)
		{
			if (details.param->anAtom()->code() == "PRO")
			{
				details.value = (rand() / (double)RAND_MAX) * 30.f - 15.f;
			}
			else
			{
				details.value = (rand() / (double)RAND_MAX) * 360.f - 180.f;
			}
		}
		else
		{
			details.value = 0;
		}
		
		_values[details.index] = details.value;
	}
}

std::vector<Angular> Conformer::angles()
{
	std::vector<Angular> angs;
	angs.reserve(_values.size());
	
	for (ParamDetails &details : _details)
	{
		if (details.type == Ramachandran || details.type == Adjustment)
		{
			angs.push_back(Angular(_values[details.index]));
		}
	}

	return angs;
}

std::vector<ResidueTorsion> Conformer::headers()
{
	std::vector<ResidueTorsion> rts;

	for (ParamDetails &details : _details)
	{
		if (details.type == Ramachandran || details.type == Adjustment)
		{
			ResidueTorsion rt{};
			rt.setTorsion(TorsionRef(details.param));
			rt.setEntity(loop()->instance->entity());
			rt.setLocalId(details.param->residueId());
			rt.housekeeping();
			rts.push_back(rt);
		}
	}
	
	return rts;
}

void Conformer::setGetterSetters(const LoopStage &stage, std::vector<float> &steps)
{
	steps.clear();

	for (ParamDetails &details : _details)
	{
		if (type_used_for_stage(details.type, stage))
		{
			steps.push_back(details.step);
		}
	}

	_getter = [stage, this](std::vector<float> &values)
	{
		int n = 0;

		for (ParamDetails &details : _details)
		{
			if (type_used_for_stage(details.type, stage))
			{
				values.push_back(details.value);
			}

			n++;
		}
	};
	
	std::vector<float> start;
	_getter(start);

	_setter = [start, stage, this](const std::vector<float> &values)
	{
		int n = 0;
		int i = 0;
		for (ParamDetails &details : _details)
		{
			if (type_used_for_stage(details.type, stage))
			{
				_values[n] = values[i] + start[i];
				i++;
			}

			n++;
		}
	};
	
	_setter(start);
}

void Conformer::report()
{

}

const Metadata::KeyValues Conformer::metadata(Metadata *source) const
{
	Metadata *md = source ? source : Environment::metadata();
	const Metadata::KeyValues *ptr = md->valuesForInstance(id());

	if (ptr != nullptr)
	{
		return *ptr;
	}

	return Metadata::KeyValues{};
}
