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

#ifndef __vagabond__Conformer__
#define __vagabond__Conformer__

#include <vector>
#include <string>
#include <functional>
#include "RunsEngine.h"
#include "HasMetadata.h"
#include <vagabond/c4x/Angular.h>

struct Loop;
struct ResidueId;
class Atom;
class Parameter;

enum ParameterType
{
	Invalid,
	Ramachandran,
	SideChain,
	PeptideBond,
	Adjustment
};

enum LoopStage
{
	Rough,
	Adjusting,
	Sidechains
};

inline std::ostream &operator<<(std::ostream &ss, const LoopStage &stage)
{
	switch (stage)
	{
		case Rough:
		ss << std::string("Rough");
		break;

		case Adjusting:
		ss << std::string("Adjusting");
		break;

		case Sidechains:
		ss << std::string("Sidechains");
		break;

		default:
		break;
	}

	return ss;
}

inline std::ostream &operator<<(std::ostream &ss, const ParameterType &type)
{
	switch (type)
	{
		case Invalid:
		ss << std::string("Invalid");
		break;

		case Ramachandran:
		ss << std::string("Ramachandran");
		break;

		case SideChain:
		ss << std::string("SideChain");
		break;

		case Adjustment:
		ss << std::string("Adjustment");
		break;

		case PeptideBond:
		ss << std::string("PeptideBond");
		break;
		
		default:
		break;
	}

	return ss;
}

class Conformer : public HasMetadata
{
public:
	Conformer(Loop *loop = nullptr);
	virtual const Metadata::KeyValues metadata(Metadata *source = nullptr) const;
	
	void setCount(int count)
	{
		_count = count;
	}
	
	operator bool() const
	{
		return _loop;
	}
	
	Loop *const &loop() const
	{
		return _loop;
	}
	
	void addParameter(Parameter *param, int idx);
	
	size_t validCount();
	void setValues(const std::vector<float> &values);
	std::vector<float> values();
	std::vector<Angular> angles();
	std::vector<ResidueTorsion> headers();
	size_t parameterCount();
	
	typedef std::function<float(Atom *)> WeightForAtom;
	const WeightForAtom &weights() const
	{
		return _weights;
	}
	
	void setWeights(const WeightForAtom &weights)
	{
		_weights = weights;
	}

	void setGetterSetters(const LoopStage &stage, std::vector<float> &steps);

	void randomise(const LoopStage &stage);
	void report();
	
	void setRmsd(float &rmsd)
	{
		_rmsd = rmsd;
	}
	
	const float &rmsd() const
	{
		return _rmsd;
	}
	
	virtual const std::string id() const
	{
		return "conf_" + std::to_string(_count);
	}
	
	virtual bool displayable() const
	{
		return true;
	}
private:
	struct ParamDetails
	{
		ParamDetails(Parameter *param, int idx, Loop *loop);

		Parameter *param;
		int index;
		float step;
		float value;
		ParameterType type;
	};

	std::vector<ParamDetails> _details;
	std::vector<float> _values;

	WeightForAtom _weights;

	Loop *_loop = nullptr;

	Getter _getter{};
	Setter _setter{};
	
	float _rmsd = FLT_MAX;
	int _count = 0;
};

#endif
