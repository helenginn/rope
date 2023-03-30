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

#include <vagabond/utils/include_boost.h>
#include <cmath>

#define private public // evil but delicious
#define protected public // evil but delicious

#include <vagabond/utils/brain/InputLayer.h>
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(input_layer_wants_forwards_layers)
{
	InputLayer il;
	std::vector<int> requests = il.wantedLayers();

	BOOST_TEST(requests.size() > 0);
	
	for (int i : requests)
	{
		BOOST_TEST(i > 0);
	}
}

BOOST_AUTO_TEST_CASE(input_layer_rejects_unwanted_layer)
{
	InputLayer il;
	
	try
	{
		il.supplyLayer(-1, nullptr);
		BOOST_FAIL("Nonsense layer was not rejected by input layer");
	}
	catch (const std::runtime_error &err)
	{

	}
}

BOOST_AUTO_TEST_CASE(input_layer_rejects_duplicate_index)
{
	InputLayer il;
	InputLayer other;
	
	try
	{
		il.supplyLayer(+1, &other);
		il.supplyLayer(+1, &other);
		BOOST_FAIL("Input layer accepted layer for pre-assigned index");
	}
	catch (const std::runtime_error &err)
	{

	}
}

BOOST_AUTO_TEST_CASE(input_layer_is_happy_with_attached_layer)
{
	InputLayer il;
	InputLayer other;
	
	float tmp;
	il.setStartPtr(&tmp);
	il.supplyLayer(+1, &other);
	il.sanityCheck(); // will throw any problems
}

BOOST_AUTO_TEST_CASE(input_layer_requests_number_of_neurons)
{
	InputLayer il;
	il.setNeuronCount(6);
	size_t req = il.requestedEntries();
	
	BOOST_TEST(req == 6);
}

BOOST_AUTO_TEST_CASE(input_layer_correctly_takes_input)
{
	InputLayer il;
	InputLayer second;

	il.supplyLayer(+1, &second);
	il.setNeuronCount(6);
	float tmp[6];
	float other[] = {1, 2, 3, 4, 5, 6};

	il.setStartPtr(tmp);
	il.enterInput(other);
	
	for (int i = 0; i < 6; i++)
	{
		BOOST_TEST(tmp[i] == other[i], tt::tolerance(1e-6));
	}
}
