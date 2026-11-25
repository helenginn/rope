//
// Created by romain on 20/04/2026.
//

#include "RotamerMenu.h"
#include "MatrixPlot.h"
#include "elements/ListView.h"
#include "vagabond/utils/Eigen/src/Core/Matrix.h"

#include <string>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/Line.h>
#include <vagabond/gui/DisplayUnit.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/rotamers/RotamerModifier.h>
#include <vagabond/core/Model.h>
#include <vagabond/core/ModelManager.h>

#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cmath>

RotamerMenu::RotamerMenu(Scene *prev, bool test, Instance *instMain, Instance *instSec)
: ListView(prev), _test(test), _instMain(instMain), _instSec(instSec)
{
    setup();
    std::cout << "yeepi" << std::endl;
}

RotamerMenu::~RotamerMenu()
{
    _instMain->unload();
    _instSec->unload();
}

void RotamerMenu::setup()
{
    addTitle("zboui");
    loadMatrices();
    ListView::setup();
}

void RotamerMenu::buttonPressed(std::string tag, Button *button)
{
    ListView::buttonPressed(tag, button);
    //showBackButton();
}

size_t RotamerMenu::lineCount()
{
    return _matrixList.size();
}

Renderable *RotamerMenu::getLine(int i)
{
    Box *box = new Box();
    MatrixRotamer &m = _matrixList[i];

    auto showmatrix = [this, m]()
    {
        openCombinedView(m);
    };
    {
        TextButton *t = new TextButton(m.name + " (" + m.chain + ")", this);
        t->setReturnTag("matrix_" + std::to_string(i));
        t->setRight(0.3, 0.0);
        t->setReturnJob(showmatrix);
        box->addObject(t);
    }

    {
        std::string info = std::to_string(m.matrix.rows()) + "x" + std::to_string(m.matrix.cols());
        TextButton *dim = new TextButton(info, this);
        dim->setRight(0.5, 0.0);
        box->addObject(dim);
    }

    return box;
}

void RotamerMenu::finishedDragging(std::string tag, double x, double y)
{
}
void RotamerMenu::loadMatrices()
{
    _matrixList.clear();
    std::string path{"run/rotaPaths"};
    if (_test)
        path = "run/rotaPathsTests";
    namespace fs = std::filesystem;
    if (!fs::exists(path))
        return;

    for (auto const &entry : fs::directory_iterator(path))
    {
        std::string fname = entry.path().filename().string();
        bool isApaths = fname.size() > 18 && fname.substr(fname.size() - 18) == "availRotApaths.csv";
        bool isBpaths = fname.size() > 18 && fname.substr(fname.size() - 18) == "availRotBpaths.csv";
        if (!isApaths && !isBpaths)
            continue;

        MatrixRotamer m{};
        m.chain = (fname.find("availRotApaths") != std::string::npos) ? "A" : "B";
        m.name = fname.substr(fname.length() - 23, 5);
        if (fname[4] == '3')
        {
            m.name += "_iter1_";
            if (_test)
            {
                m.name += fname.substr(11, 2);
            }
        }
        else if (fname[4] == '2')
        {
            m.name += "_iter2_";
            if (_test)
            {
                m.name += fname.substr(11, 2);
            }
        }
        else
        {
            m.name += "_iterFINAL_";
            if (_test)
            {
                m.name += fname.substr(11, 2);
            }
        }
        std::ifstream file(entry.path());
        if (!file.is_open())
            continue;

        std::string line;
        std::getline(file, line);
        {
            std::istringstream iss(line);
            std::string token;
            std::vector<float> xyz;
            while (std::getline(iss, token, ','))
                xyz.push_back(std::stof(token));
            if (xyz.size() >= 3)
            {
                m.startPos = glm::vec3(xyz[0], xyz[1], xyz[2]);
            }
        }

        std::getline(file, line);
        {
            std::istringstream iss(line);
            std::string token;
            std::getline(iss, token, ',');
            while (std::getline(iss, token, ','))
                m.labels.push_back(token);
        }

        int rows = 0;
        int maxCols = 0;
        std::vector<std::vector<int>> data;
        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string token;
            std::getline(iss, token, ',');
            std::vector<int> row;
            while (std::getline(iss, token, ','))
                row.push_back(std::stoi(token));
            if ((int)row.size() > maxCols)
                maxCols = row.size();
            data.push_back(std::move(row));
            // for (auto &value : data[rows])
            //     std::cout << std::to_string(value) << ", ";
            // std::cout << std::endl;
            rows++;
        }
        file.close();

        m.matrix = Eigen::MatrixXi::Zero(rows, maxCols);
        for (int r = 0; r < rows; r++)
            for (int c = 0; c < (int)data[r].size(); c++)
                m.matrix(r, c) = data[r][c];

        _matrixList.push_back(std::move(m));
    }

    std::sort(_matrixList.begin(), _matrixList.end(),
              [](const MatrixRotamer &a, const MatrixRotamer &b)
              { return a.name < b.name; });
}

void RotamerMenu::openCombinedView(const MatrixRotamer &m)
{
    if (_combinedView)
        _combinedView = nullptr;
    _combinedView = new CombinedView(this, _instMain, _instSec, m);
    _combinedView->show();
}

CombinedView::CombinedView(Scene *prev, Instance *instMain, Instance *instSec, MatrixRotamer matRot) : Scene(prev), Display(prev), _instMain(instMain), _instSec(instSec), _matrix(matRot)
{
    _matrixRows = _matrix.matrix.rows();
    std::string mainChain = _instMain->currentAtoms()->atomVector()[0]->chain();
    std::string secChain = _instSec->currentAtoms()->atomVector()[0]->chain();
    _modifier = new RotamerModifier(_instMain, _instSec, mainChain, secChain);
    // _instMain->load();
    // _instSec->load();
    view();
}

CombinedView::~CombinedView()
{
    std::cout << "[DEBUG] COMBINEDVIEW DESTROYED MUHAHAHA" << std::endl;
    _instMain->unload();
    _instSec->unload();
    delete _modifier;
}

void CombinedView::buttonPressed(std::string tag, Button *button)
{
    Scene::buttonPressed(tag, button);
}
void CombinedView::view()
{
    DisplayUnit *unit = new DisplayUnit(this);
    AtomGroup *atoms{_instMain->currentAtoms()};
    atoms->add(_instSec->currentAtoms());
    for (auto *a : atoms->atomVector())
    {
        a->setInitialPosition(a->derivedPosition());
    }
    unit->loadAtoms(atoms);
    unit->displayAtoms(false, false);
    unit->setMultiBondMode(true);
    unit->startWatch();
    addDisplayUnit(unit);
}

void CombinedView::setup()
{
    addTitle(_matrix.name + " (" + _matrix.chain + ")");
    {
        float mag = glm::length(_matrix.startPos);
        if (mag < 1e-6f)
            mag = 1.0f;
        _slider = new Slider();
        _slider->setDragResponder(this);
        _slider->resize(0.3);
        _slider->setup("Path to folded structure", 0.0, (double)mag, mag / _matrix.matrix.col(1).size(), false);
        std::cout << "[DEBUG] Number of steps = " << _matrix.matrix.col(1).size() << std::endl;
        _slider->setStart(1, 0);
        _slider->setCentre(0.5, 0.90);
        _slider->setReturnTag("sliderMoved");
        addObject(_slider);
    }
    {
        Eigen::MatrixXf mat = _matrix.matrix.cast<float>();
        _plot = new MatrixPlot(mat);
        _plot->rescale(1.5f, 10.f);
        _plot->setCentre(0.75, 0.5);
        addObject(_plot);
    }
    {
        _indicator = new Line(false);
        _indicator->setAlpha(0.8);
        _indicator->setColour(1.0, 0.0, 0.0);
        addObject(_indicator);
    }
    updateIndicator(0.0);
}

void CombinedView::finishedDragging(std::string tag, double x, double y)
{
    if (tag != "sliderMoved")
        return;

    float mag = glm::length(_matrix.startPos);
    if (mag < 1e-6f)
        return;

    float distance = (float)x;
    float delta = distance - _prevDistance;
    _prevDistance = distance;

    if (_modifier && std::abs(delta) > 1e-6f)
        _modifier->moveDirection(_matrix.startPos, delta);

    updateIndicator(distance);
}

void CombinedView::updateIndicator(float distance)
{
    if (!_indicator)
        return;

    float mag = glm::length(_matrix.startPos);
    if (mag < 1e-6f || _matrixRows < 2)
        return;

    int row = (int)std::round((float)((distance / mag) * (_matrixRows - 1)));
    row = std::max(0, std::min(row, _matrixRows - 1));

    glm::vec3 minCorner, maxCorner;
    _plot->bounds(minCorner, maxCorner);

    float y = maxCorner.y - ((float)row / (_matrixRows - 1)) * (maxCorner.y - minCorner.y);
    float xMin = minCorner.x;
    float xMax = maxCorner.x;

    _indicator->clearVertices();
    _indicator->addPoint(glm::vec3(xMin - 0.02, y, 0.f));
    _indicator->addPoint(glm::vec3(xMax + 0.02, y, 0.f));
    _indicator->forceRender();
}
