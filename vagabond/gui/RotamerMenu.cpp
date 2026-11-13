//
// Created by romain on 20/04/2026.
//

#include "RotamerMenu.h"
#include "MatrixPlot.h"
#include "elements/ListView.h"
#include "vagabond/utils/Eigen/src/Core/Matrix.h"
#include "vagabond/utils/svd/PCA.h"

#include <string>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/core/EntityManager.h>

#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

RotamerMenu::RotamerMenu(Scene *prev) : ListView(prev)
{
    setup();
    std::cout << "yeepi" << std::endl;
}

RotamerMenu::~RotamerMenu()
{
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
        Display *viewMat = new Display(this);
        Eigen::MatrixXf mat = m.matrix.cast<float>();
        _plot = new MatrixPlot(mat);
        _plot->rescale(3.f, 15.f);
        viewMat->addObject(_plot);
        viewMat->show();
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

void RotamerMenu::loadMatrices()
{
    _matrixList.clear();
    namespace fs = std::filesystem;
    if (!fs::exists("run"))
        return;

    for (auto const &entry : fs::directory_iterator("run"))
    {
        std::string fname = entry.path().filename().string();
        bool isApaths = fname.size() > 18 && fname.substr(fname.size() - 18) == "availRotApaths.csv";
        bool isBpaths = fname.size() > 18 && fname.substr(fname.size() - 18) == "availRotBpaths.csv";
        if (!isApaths && !isBpaths)
            continue;

        MatrixRotamer m{};
        m.chain = (fname.find("availRotApaths") != std::string::npos) ? "A" : "B";
        m.name = fname.substr(fname.length() - 23, 5);

        std::ifstream file(entry.path());
        if (!file.is_open())
            continue;

        std::string line;
        // std::getline(file, line);

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
            for (auto &value : data[rows])
                std::cout << std::to_string(value) << ", ";
            std::cout << std::endl;
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
