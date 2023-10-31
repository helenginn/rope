// vagabond
// Copyright (C) 2019 Helen Ginn
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

#include "BondCalculator.h"
#include "Result.h"
#include "engine/Task.h"

BondCalculator::BondCalculator()
{

}

BondCalculator::~BondCalculator()
{

}

Task<Result, void *> *BondCalculator::submitResult(int ticket)
{
	auto send = [this, ticket](Result result) -> void *
	{
		result.ticket = ticket;
		submitResult(result);
		return nullptr;
	};

	_resultPool.expect_one();
	auto *submit = new Task<Result, void *>(send, "submit result");
	return submit;

}

void BondCalculator::holdHorses()
{
	_resultPool.expect_one();
}

void BondCalculator::releaseHorses()
{
	_resultPool.release_one();
}

Result *BondCalculator::acquireResult()
{
	Result *result = nullptr;
	_resultPool.acquireObjectOrNull(result);
	return result;
}

void BondCalculator::submitResult(const Result &r)
{
	Result *remade = new Result(r);
	_resultPool.pushObject(remade);

	// an extra signal because we trapped this semaphore when submitting
	// the corresponding job.
	_resultPool.one_arrived();
}

void BondCalculator::submitResult(Result *r)
{
	_resultPool.pushObject(r);

	// an extra signal because we trapped this semaphore when submitting
	// the corresponding job.
	_resultPool.one_arrived();
}

