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

#include "FilterConformer.h"

#include "engine/Tasks.h"
#include "engine/Task.h"

#include "BondCalculator.h"
#include "BondSequence.h"
#include "AtomBlock.h"
#include "StructureModification.h"
#include "Conformer.h"
#include "Loopy.h"

FilterConformer::FilterConformer(Loopy *loopy)
: _resources(loopy->_resources), _loopy(loopy)
{

}

int FilterConformer::submitJob(const std::vector<float> &vals) const
{
	int ticket = 1;

	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;
	
	/* get easy references to resources */
	BondCalculator *const &calculator = _resources.calculator;
	BondSequenceHandler *const &sequences = _resources.sequences;

	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->actOfSubmission(ticket);
	Task<BondSequence *, Deviation> *dev = nullptr;

	Flag::Calc calc;
	calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions | Flag::DoSuperpose);
	Flag::Extract extract = Flag::Extract(Flag::AtomVector | Flag::Deviation);

	/* calculation of torsion angle-derived and target-derived
	 * atom positions */
	sequences->calculate(calc, vals, &first_hook, &final_hook);
	sequences->extract(extract, submit_result, final_hook, &dev);
	
	if (_clash)
	{
		const ClashFunction &clash = _clash;

		auto calc_clash = [clash](BondSequence *seq) -> Deviation
		{
			const std::vector<AtomBlock> &blocks = seq->blocks();
			float score = clash(blocks);
			return {score};
		};

		dev->changeTodo(calc_clash);
	}
	
	_resources.tasks->addTask(first_hook);

	return ticket;
}

bool FilterConformer::operator()(Conformer *active, float proximity_limit) const
{
	submitJob(active->values());

	Result *r = _resources.calculator->acquireObject();
	r->transplantPositions();

	float clash = r->deviation;
	bool ok = acceptable(clash, proximity_limit);

	std::cout << _tag << ": " << clash;
	
	if (!ok)
	{
		std::cout << "- rejected";
		
		if (_message.length())
		{
			std::cout << ": " << _message << std::endl;
		}
	}
	else
	{
		_loopy->copyPositions(_tag);
	}

	std::cout << std::endl;
	
	return ok;
}

