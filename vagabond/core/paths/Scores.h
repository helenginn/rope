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

#ifndef __vagabond__Scores__
#define __vagabond__Scores__

#include "ResidueId.h"
#include "../engine/ElementTypes.h"
#include <mutex>

// helpful rant:
// The spaghetti code is really upsetting. I built this lovely control system which splits jobs arbitrarily on different threads, and somehow the control mechanism is just a giant IF statement maze now. I can split the scores up by residue ID but not by subunit. I’ve got a class called “ByResidueResult” and obviously I will need a “ByInstanceResult”, but I should make it templated for the future (“ResultBy<Custom>” where Custom can be whatever I like). In fact, “DetermineCustom” should be an actual function figuring out what “Custom” is. Then I can return e.g. instance ID or residue ID or whatever I like elsewhere in the code.

// that's a bit weird, Past Helen. Just have one Custom option which covers both chains & residue ranges. Your ByResidueResult is not going to work when you have multiple chains anyway.

struct ActivationEnergy;

struct ScoreBucket
{
	std::string chain{};
	ResidueId minRes{};
	ResidueId maxRes{};
	
	int as_num() const
	{
		return minRes.as_num();
	}
	
	bool operator<(const ScoreBucket &other) const;
	bool operator==(const ScoreBucket &other) const;

	bool operator!=(const ScoreBucket &other) const
	{
		return !(*this == other);
	}
	
	bool single() const;
	ScoreBucket()
	{
		chain = {};
		minRes = {};
		maxRes = {};
	}

	ScoreBucket(std::string ch, int m, int M)
	{
		chain = ch;
		minRes = m;
		maxRes = M;
	}

	ScoreBucket(Atom *atom);
	ScoreBucket(int res)
	{
		chain = "";
		minRes = res;
		maxRes = res;
	}
	bool fully_contains(const ScoreBucket &other) const;
	
	friend std::ostream &operator<<(std::ostream &ss, 
	                                const ScoreBucket &sb);

};

inline std::ostream &operator<<(std::ostream &ss, 
                                const ScoreBucket &sb)
{
	ss << sb.chain << " (" << sb.minRes << "-" << sb.maxRes << ")";
	return ss;
}

template <class Custom>
struct SingleResult
{
	Custom id{};
	float score = 0;
	float highest = 0;
};

inline std::ostream &operator<<(std::ostream &ss, 
                                const SingleResult<ScoreBucket> &sr)
{
	ss << sr.id << ":" << sr.score;
	return ss;
}

template <class Custom>
struct ResultBy
{
	std::map<Custom, float> scores;
	std::map<Custom, float> activations;
	std::mutex *mutex = new std::mutex();
	int ticket;
	int n{};
	
	void destroy()
	{
		delete mutex;
		delete this;
	}

	void operator=(const SingleResult<Custom> &srr)
	{
		std::unique_lock<std::mutex> lock(*mutex);
		n++;
		scores[srr.id] += srr.score;
		if (activations[srr.id] < srr.score)
		{
			activations[srr.id] = srr.score;
		}
	}
};


#endif
