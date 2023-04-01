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
#include <algorithm>

#define private public // evil but delicious
#define protected public // evil but delicious

#include <vagabond/utils/brain/InputLayer.h>
#include <vagabond/utils/brain/HiddenLayer.h>
#include <vagabond/utils/brain/OutputLayer.h>
#include <vagabond/utils/brain/NeuralNet.h>
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
	il.supplyLayer(+1, &other);
	il.setStartPtr(&tmp);
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

BOOST_AUTO_TEST_CASE(hidden_layer_requests_appropriate_entries)
{
	InputLayer il;
	OutputLayer hl;

	il.supplyLayer(+1, &hl);
	hl.supplyLayer(-1, &il);
	il.setNeuronCount(3);
	hl.setNeuronCount(5);

	size_t target = 3*5 + 5; // weights, bias,
	target += 5 + 5 + 5 + 5 + 5; // sum, evaluation of function, gradient,
	//sensitivities, targets.
	
	size_t found = hl.requestedEntries();
	
	BOOST_TEST(found == target);
}

BOOST_AUTO_TEST_CASE(hidden_layer_randomises_weights)
{
	InputLayer il;
	OutputLayer hl;

	il.supplyLayer(+1, &hl);
	hl.supplyLayer(-1, &il);
	il.setNeuronCount(2);
	hl.setNeuronCount(3);

	size_t found = hl.requestedEntries();
	float *tmp = (float *)malloc(found * sizeof(float));
	for (size_t i = 0; i < found; i++)
	{
		tmp[i] = 1;
	}

	hl.setStartPtr(tmp);
	
	for (size_t i = 0; i < 2 * 3; i++)
	{
		BOOST_TEST(tmp[i] < 0.6);
	}
}

BOOST_AUTO_TEST_CASE(hidden_layer_multiplies_weights)
{
	InputLayer il;
	OutputLayer hl;

	il.supplyLayer(+1, &hl);
	hl.supplyLayer(-1, &il);
	il.setNeuronCount(2);
	hl.setNeuronCount(3);

	size_t il_reqs = il.requestedEntries();
	size_t hl_reqs = hl.requestedEntries();
	size_t total = il_reqs + hl_reqs;

	float *tmp = (float *)calloc(total, sizeof(float));
	il.setStartPtr(tmp);
	float *inc = tmp + il_reqs;
	hl.setStartPtr(inc);
	
	for (size_t i = 0; i < 2 * 3; i++) // reset weights to known values
	{
		inc[i] = 0.1;
	}
	
	float fake[] = {1, 1};
	float *ptr = &fake[0];

	il.enterInput(ptr);
	
	hl.run();
	
	VectorLoc v = hl.outputLayerInfo();
	
	for (size_t i = 0; i < v.size; i++)
	{
		BOOST_TEST(v[i] == 0.2, tt::tolerance(1e-6));
	}
}

// https://hagan.okstate.edu/NNDesign.pdf chapter 11-15
NeuralNet prepareNetworkFromTextbook()
{
	NeuralNet net{};
	net.inputLayer().setNeuronCount(1);
	net.outputLayer().setNeuronCount(1);

	{
		std::vector<float> weights = {+0.09, -0.17};
		std::vector<float> biases = {+0.48};
		net.outputLayer().setDefaultWeights(weights);
		net.outputLayer().setDefaultBiases(biases);
	}
	
	HiddenLayer hl{};
	hl.setNeuronCount(2);
	hl.setFunctionType(FLogSigmoid);
	
	{
		std::vector<float> weights = {-0.27, -0.41};
		std::vector<float> biases = {-0.48, -0.13};
		hl.setDefaultWeights(weights);
		hl.setDefaultBiases(biases);
	}

	net.addLayer(hl);
	
	net.connect();

	return net;
}

BOOST_AUTO_TEST_CASE(worked_example_from_textbook)
{
	NeuralNet net = prepareNetworkFromTextbook();

	float input[] = {1};
	float output[] = {1.707};
	float *iPtr = &input[0];
	float *oPtr = &output[0];
	
	float expected = 0.446282;

	net.inputLayer().enterInput(iPtr);
	net.outputLayer().expectOutput(oPtr);
	net.forwardRun();
	net.backwardRun();
	net.learnRun();
	
	const VectorLoc &v = net.outputLayer().outputLayerInfo();
	
	BOOST_TEST(v[0] == expected, tt::tolerance(1e-6));

	net.forwardRun();
	BOOST_TEST(v[0] == 0.759284854, tt::tolerance(1e-6));
}

BOOST_AUTO_TEST_CASE(neural_net_without_training_returns_same_result)
{
	float p = 0.5;
	float t = 1.0;

	NeuralNet net = prepareNetworkFromTextbook();
	
	net.setInputOutput(&p, &t);
	net.forwardRun();
	
	const VectorLoc &v = net.outputLayer().outputLayerInfo();
	
	float before = v[0];
	net.forwardRun();
	float after = v[0];
	
	BOOST_TEST(before == after, tt::tolerance(1e-6));
}

BOOST_AUTO_TEST_CASE(train_worked_example_on_data)
{
	std::vector<std::pair<float, float> > points;

	for (int i = -8; i < +8; i++)
	{
		float p = (float)i / 4.;
		float t = 1 + sin(M_PI / 4 * p);
		points.push_back(std::make_pair(p, t));
	}

	std::random_shuffle(points.begin(), points.end());

	NeuralNet net = prepareNetworkFromTextbook();
	
	for (size_t i = 0; i < 100; i++)
	{
		for (auto pair : points)
		{
			net.setInputOutput(&pair.first, &pair.second);
			net.runAndLearn();
		}

		std::random_shuffle(points.begin(), points.end());
	}
	
	const VectorLoc &v = net.outputLayer().outputLayerInfo();
	for (int i = -8; i < +8; i++)
	{
		float p = (float)i / 4.;
		float t = 1 + sin(M_PI / 4 * p);
		net.setInputOutput(&p, &t);
		net.forwardRun();
		
		float e = fabs(v[0] - t);

		BOOST_TEST(e < 0.1);
	}
	
}
