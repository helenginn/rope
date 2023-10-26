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

#include <vagabond/core/engine/Task.h>
#include "Result.h"
#include "PdbFile.h"
#include "AtomGroup.h"
#include "BondSequenceHandler.h"
#include "BondSequence.h"
#include <string>
#include <iostream>

auto make_fixed_float(const float num)
{
	return [num](int) -> float
	{ 
		std::cout << "Making a " << num << std::endl;
		return num;
	};
}

namespace tt = boost::test_tools;
BOOST_AUTO_TEST_CASE(tasks_follow_well)
{
	return;
	auto to_words = [](float f) -> std::string
	{
		std::cout << "turning " << f << " into words" << std::endl;
		return std::to_string(f);
	};
	
	struct strings
	{
		strings &operator=(std::string str)
		{
			found.push_back(str);
			return *this;
		}
		std::vector<std::string> found;
	};

	auto speak = [](strings str) -> void *
	{
		std::cout << "say it: ";
		std::cout << str.found[0] << " " << str.found[1] << std::endl;
		return nullptr;
	};
	
	Task<float, std::string> *convert_three{}, *convert_five{};
	Tasks tasks;
	{
		auto *make_three = new Task<int, float>(make_fixed_float(3), 0);
		convert_three = new Task<float, std::string>(to_words, 0);
		make_three->follow_with(convert_three);

		tasks += {convert_three, make_three};
	}

	{
		auto *make_five = new Task<int, float>(make_fixed_float(5), 1);
		convert_five = new Task<float, std::string>(to_words, 1);

		make_five->follow_with(convert_five);

		tasks += {make_five, convert_five};
	}

	auto *speak_out_loud = new Task<strings, void *>(speak, 2);
	convert_three->follow_with(speak_out_loud);
	convert_five->follow_with(speak_out_loud);
	tasks += {speak_out_loud};

	tasks.run(2);

	
	BOOST_TEST(true);
}

BOOST_AUTO_TEST_CASE(tasks_with_calculator)
{
	std::string path = "/Users/vgx18549/pdbs/stromelysin/1hfs-no-lig.pdb";
	PdbFile geom = PdbFile(path);
	geom.parse();
	AtomGroup *hexane = geom.atoms();

	BondSequenceHandler handler;
	handler.setMaxSimultaneousThreads(1);
	handler.setTotalSamples(1);
	handler.setSuperpose(true);
	handler.setIgnoreHydrogens(true);
	handler.addAnchorExtension(hexane->chosenAnchor());

	handler.setup();
	handler.prepareSequences();
	
	std::vector<float> params(handler.sequence(0)->torsionBasis()->parameterCount());

	struct Job
	{
		int ticket;
		BondSequence *sequence;
	};
	
	int ticket = 1;
	Task<Ticket, Ticket> *final_hook;
	Task<Ticket, void *> *let_sequence_go;

	CalcFlags flags = CalcFlags(DoTorsions | DoPositions | DoSuperpose);
	Tasks *tasks = handler.calculate(ticket, flags, params, 
	                                 &final_hook, &let_sequence_go);

	final_hook->follow_with(let_sequence_go);

	tasks->run(2);

	BOOST_TEST(true);
}
