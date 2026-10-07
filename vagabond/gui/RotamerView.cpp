//
// Created by romain on 24/04/2026.
//

#include "RotamerView.h"
#include "vagabond/gui/elements/Slider.h"
#include <vagabond/core/Model.h>
#include <../core/rotamers/Rotamers.h>
#include <vagabond/core/rotamers/RotamerModifier.h>
#include <vagabond/core/ModelManager.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/core/files/CsvFile.h>
#include <fstream>
#include <regex>

#include "MatrixPlot.h"

RotamerView::RotamerView(Scene *prev, Instance *instMain, Instance *instSec, std::string mainChain, std::string secChain)
:  Scene(prev), Display(prev), _instMain(instMain), _instSec(instSec)
{
    _instMain->load();
    _instSec->load();
    _modifier = new RotamerModifier(_instMain, _instSec,mainChain, secChain);
}

RotamerView::~RotamerView()
{
    delete _modifier;
    _instMain->unload();
    _instSec->unload();
    std::cout << "model unloaded\n";
}
void RotamerView::setup()
{
    addTitle ("Rotamer View");
    {
        TextButton *t = new TextButton("hide/show collisionBoxes" , this);
        t->setRight(0.9, 0.9);
        t->setReturnTag("collision");
        addObject(t);
    }
    {
        TextButton *t = new TextButton("analysis test" , this);
        t->setRight(0.9, 0.7);
        t->setReturnTag("analysis");
        addObject(t);
    }
    auto viewMod = [this]()
    {
        viewModel();
    };
    {
        TextButton *t = new TextButton("ViewModel", this);
        t->setRight(0.2, 0.7);
        t->setReturnJob(viewMod);
        addObject(t);
    }
    auto libTest = [this]()
    {
        std::map<std::string,Eigen::MatrixXf> matArray = _modifier->proximityMatrix();
    	_plot = new MatrixPlot(matArray["ARG"]);
    	addObject(_plot);

    };
    {
        TextButton *t = new TextButton("libTest", this);
        t->setRight(0.2, 0.9);
        t->setReturnJob(libTest);
        addObject(t);
    }
    {
        _line = new Line();
        _line2 = new Line();
        _line3 = new Line();
        _line4 = new Line();
        _line5 = new Line();
        _line6 = new Line();
        _line7 = new Line();
        _para = new Parallelepiped();

        addObject(_line);
        addObject(_line2);
        addObject(_line3);
        addObject(_line4);
        addObject(_line5);
        addObject(_line6);
        addObject(_line7);
        addObject(_para);
        drawAxis();
    }
    viewModel();
    setupSlider();
}
void RotamerView::drawAxis()
{
    _line4->clearVertices();
    _line5->clearVertices();
    _line6->clearVertices();
    std::vector<glm::vec3> axis {_modifier->drawAxis()};
    _line4->addPoint(glm::vec3(5,0,0));
    _line4->addPoint(axis[0]+glm::vec3(5.f,0.f,0.f));
    _line4->forceRender();
    _line5->addPoint(glm::vec3(5,0,0));
    _line5->addPoint(axis[1]+glm::vec3(5.f,0.f,0.f));
    _line5->setColour(0.8,0.1,0.1);
    _line5->forceRender();
    _line6->addPoint(glm::vec3(5,0,0));
    _line6->addPoint(axis[2]+glm::vec3(5.f,0.f,0.f));
    _line6->setColour(0.1,0.1,0.8);
    _line6->forceRender();
}
void RotamerView::drawChainAxis()
{
    _line2->clearVertices();
    std::vector<glm::vec3> axePoints = _modifier->drawChainAxis();
    _line2->addPoint(axePoints[0]);
    _line2->addPoint(axePoints[1]);
    _line2->addPoint(axePoints[2], false);
    _line2->addPoint(axePoints[3]);
    _line2->setColour(0.2, 0.2, 0.9);
    _line2->forceRender();
}
void RotamerView::buttonPressed(std::string tag, Button *button)
{
    if (tag == "analysis") // coordinates : X = along static structure axis, Y: left-right Z: - = moving away the other structure
    {
        std::vector<glm::vec3> tests  {_modifier->RandStartPos(100)};
        std::string fileName = "vectors_list.csv";
        std::string csvContent {};
        for (auto pos : tests)
        {
            csvContent += std::to_string(pos.x) + "," + std::to_string(pos.y) + "," + std::to_string(pos.z) + '\n';
        }
        // std::ifstream file;
        // file.open(fileName);
        // if (!file.is_open())
        // {
        //     throw std::runtime_error("Could not open rotamer the vectors file");
        // }
        // std::string line {};
        // std::vector<glm::vec3> tests {};
        // while (getline(file, line))
        // {
        //     std::istringstream iss(line);
        //     std::string lineStream;
        //     std::vector<float> xyz {};
        //     glm::vec3 readPos {};
        //     while (getline(iss, lineStream, ','))
        //     {
        //         xyz.push_back(std::stof(lineStream)); // convert to double
        //     }
        //     readPos.x = xyz[0];
        //     readPos.y = xyz[1];
        //     readPos.z = xyz[2];
        //     tests.push_back(readPos);
        // }
        std::ofstream file;
        file.open(fileName);
        if (file.is_open())
        {
            file << csvContent;
            file.close();
        }
        _modifier->analysisPipeline(50, tests, 3);
        _line3->clearVertices();
        for (auto pos : tests)
        {
            _line3->addPoint(glm::vec3(0.f,0.f,0.f), false);
            _line3->addPoint(pos);
        }
        _line3->forceRender();
    }
    if (tag == "collision") // saving current structure
    {
        if (_collision)
        {
            _para->clearVertices();
            _collision = false;
        }
        else
        {
            _collision = true;
            drawChainAxis();
            // setupCollision();
        	/*
            std::ifstream file;
			std::vector<std::string> filenames{};
			glm::vec3 memoryPos{0.f};
	        {
		        for (auto const &entry: std::filesystem::directory_iterator("."))
		        {
		        	if (std::regex_match(entry.path().filename().string(), std::regex(".*_hedgehog\\.csv")))
		        	{
		        		file.open(entry.path().filename().string());
		        		if (file.is_open())
		        		{
		        			std::string line{};
		        			std::vector<glm::vec4> tests{};
		        			glm::vec4 readPos{};
		        			glm::vec3 average{};
		        			float vecNum{0};
		        			float minVal = {FLT_MAX};
		        			float maxVal = {-FLT_MAX};
		        			std::string chain{};
		        			std::string chainSec{};
		        			getline(file, chain, ',');
		        			if (chain == _instMain->currentAtoms()->chosenAnchor()->chain())
		        			{
		        				std::vector<float> coordinates;
		        				std::string numStr{};
		        				glm::vec3 axis1{};
		        				glm::vec3 axis2{};
		        				for (int x = 0; x < 6; x++)
		        				{
		        					getline(file, numStr, ',');
		        					coordinates.push_back(std::stof(numStr));
		        				}
		        				axis1.x = coordinates[0];
		        				axis1.y = coordinates[1];
		        				axis1.z = coordinates[2];
		        				axis2.x = coordinates[3];
		        				axis2.y = coordinates[4];
		        				axis2.z = coordinates[5];
		        				getline(file, chainSec, ',');
		        				getline(file, numStr, '\n');

		        				while (getline(file, line))
		        				{
		        					std::istringstream iss(line);
		        					std::string lineStream;
		        					std::vector<float> xyz{};
		        					while (getline(iss, lineStream, ','))
		        					{
		        						xyz.push_back(std::stof(lineStream)); // convert to float
		        					}
		        					readPos.x = xyz[0];
		        					readPos.y = xyz[1];
		        					readPos.z = xyz[2];
		        					readPos.w = xyz[3];
		        					if (xyz[3] < minVal)
		        						minVal = xyz[3];
		        					else if (xyz[3] > maxVal)
		        						maxVal = xyz[3];
		        					tests.push_back(readPos);
		        					average += glm::vec3(readPos);
		        					vecNum += 1;
		        				}
		        				average /= glm::vec3(vecNum);
		        				glm::vec3 startPos{};
		        				std::cout << '\t' << chain[0] << std::endl;
		        				int counter{0};
		        				glm::vec3 axisStart{};
		        				glm::vec3 axisEnd{};
		        				glm::vec3 axisSecStart{};
		        				glm::vec3 axisSecEnd{};
		        				Atom *endAtom{};
		        				bool firstSecChain{true};
		        				std::cout << "INSIDE" << std::endl;
		        				for (auto atoms: _instMain->currentAtoms()->atomVector())
		        				{
		        					if (atoms->chain()[0] == chain[0] && atoms->isMainChain())
		        					{
		        						if (counter == 0)
		        							axisStart = atoms->derivedPosition();
		        						counter++;
		        						if (counter == 3)
		        						{
		        							std::cout << entry.path().filename().string() << std::endl;
		        							startPos = atoms->derivedPosition();
		        							std::cout << "start Atom chain == " << atoms->chain() << std::endl;

		        							if (std::regex_match(entry.path().filename().string(), std::regex("iter2.*")))
		        								startPos.x += 10;
		        							if (std::regex_match(entry.path().filename().string(), std::regex("iter3.*")))
		        								startPos.x += 20;
		        							// std::cout << atoms->atomName() << std::endl;
		        						}
		        						axisEnd = atoms->derivedPosition();
		        						endAtom = atoms;
		        					}
		        					if (atoms->chain()[0] == chainSec[0] && atoms->isMainChain())
		        					{
		        						if (firstSecChain)
		        						{
		        							axisSecStart = atoms->derivedPosition();
		        							firstSecChain = false;
		        						}
		        						axisSecEnd = atoms->derivedPosition();
		        					}
		        				}
		        				std::cout << "End Atom chain == " << endAtom->chain() << std::endl;

		        				// if (memoryPos == glm::vec3(0.f))
		        				// {
		        				// 	startPos = glm::length(axisEnd-axisSecStart) < glm::length(axisEnd-axisSecEnd) ? axisEnd-(axisEnd-axisSecStart)/glm::vec3(4) : axisEnd-(axisEnd-axisSecEnd)/glm::vec3(4);
		        				// 	memoryPos = startPos;
		        				// }
		        				// else if (glm::length(axisStart-memoryPos) < glm::length(axisEnd-memoryPos))
		        				// {
		        				// 	startPos = glm::length(axisStart-axisSecStart) < glm::length(axisStart-axisSecEnd) ? axisStart-(axisStart-axisSecStart)/glm::vec3(4) : axisStart-(axisStart-axisSecEnd)/glm::vec3(4);
		        				// 	memoryPos = startPos;
		        				// }
		        				// else
		        				// {
		        				// 	startPos = glm::length(axisEnd-axisSecStart) < glm::length(axisEnd-axisSecEnd) ? axisEnd-(axisEnd-axisSecStart)/glm::vec3(4) : axisEnd-(axisEnd-axisSecEnd)/glm::vec3(4);
		        				// 	memoryPos = startPos;
		        				// }
		        				startPos = glm::vec3(_modifier->_transform*glm::vec4((axis1 + (axis2 - axis1) / glm::vec3(2) + average / glm::vec3(1.5)),1.f));
		        				if (std::regex_match(entry.path().filename().string(), std::regex("iter2.*")))
		        					startPos.x += 10;
		        				if (std::regex_match(entry.path().filename().string(), std::regex("iter3.*")))
		        					startPos.x += 20;
		        				//if (endAtom->chain()[0] == 'A')
		        				// {
		        				// 	Line *axis = new Line;
		        				// 	d->addObject(axis);
		        				// 	axis->addPoint(axis1);
		        				// 	axis->addPoint(axis2);
		        				// 	axis->forceRender();
		        				// }
		        				glm::vec3 max(1.f, 0.f, 0.f);
		        				glm::vec3 min(0.f, 0.f, 1.f);
		        				float capped = minVal + (maxVal - minVal) / 16;
		        				for (auto const &vectors: tests)
		        				{
		        					Parallelepiped *para1 = new Parallelepiped(true, true);
		        					addObject(para1);
		        					para1->addTrueParallelepiped(startPos, glm::vec3(_modifier->_transform*(vectors / glm::vec4(4))), 0.4, 0.9);
		        					{
		        						float pos = vectors.w - minVal;
		        						glm::vec3 colour{};
		        						if (pos <= capped - minVal)
		        							colour = (pos / capped) * max + (1 - pos / capped) * min;
		        						else
		        							colour = max;
		        						para1->setColour(colour.x, colour.y, colour.z);
		        					}
		        					para1->setAlpha(0.6f);
		        					para1->forceRender();
		        				}
		        			}
		        			file.close();
		        		}
		        	}
		        }
	        }*/
        }

        drawAxis();
    }
    Scene::buttonPressed(tag, button);
}

void RotamerView::viewModel()
{
    {
        DisplayUnit *unit = new DisplayUnit(this);
        AtomGroup *atoms {_instMain->currentAtoms()};
        atoms->add(_instSec->currentAtoms());
        unit->loadAtoms(atoms);
        setupCollision();
        unit->displayAtoms(false, false);
        unit->setMultiBondMode(true);
        unit->startWatch();
        addDisplayUnit(unit);
    }
}

void RotamerView::rotaList()
{
    RotamerLibrary();
}

void RotamerView::finishedDragging(std::string tag, double x, double y)
{
    if (tag == "X")
    {
        _modifier->move(x,RotamerModifier::MoveX);
    }
    if (tag == "Y")
    {
        _modifier->move(x,RotamerModifier::MoveY);
    }
    if (_collision)
    {
        std::vector<std::pair<glm::vec3,glm::vec3>> drawing {};
        drawing = _modifier->getVertices();
        _para->clearVertices();
        for (auto pair : drawing)
        {
            _para->addParallelepiped(pair.first,pair.second);
        }
        _para->setAlpha(1.0f);
        _para-> forceRender();
    }
    drawChainAxis();
}

void RotamerView::setupSlider()
{
    removeObject(_rangeSlider);
    delete _rangeSlider;
    Slider *s = new Slider();

    s->setDragResponder(this);
    s->resize(0.5);
    s->setup("Rotamer selection", _min, _max, _step);
    s->setStart(0.5, 0);
    s->setCentre(0.5, 0.85);
    s->setReturnTag("X");
    _rangeSlider = s;
    addObject(s);

    removeObject(_rangeSlider2);
    delete _rangeSlider2;
    Slider *s2 = new Slider();
    s2->setVertical(true);
    s2->setDragResponder(this);
    s2->resize(0.5);
    s2->setup("", _min, _max, _step);
    s2->setStart(0, 0.5);
    s2->setCentre(0.2, 0.6);
    s2->setReturnTag("Y");

    _rangeSlider2 = s2;
    addObject(s2);
}

void RotamerView::setupCollision()
{
    if (_collision)
    {
        std::vector<std::pair<glm::vec3,glm::vec3>> drawing {};
        drawing = _modifier->getVertices();
        _para->clearVertices();
        for (auto pair : drawing)
        {
            _para->addParallelepiped(pair.first,pair.second);
        }
        _para->setAlpha(1.0f);
        _para-> forceRender();
    }
}