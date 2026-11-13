//
// Created by romain on 20/04/2026.
//

#ifndef __vagabond__RotamerMenu__
#define __vagabond__RotamerMenu__

#include "FileManager.h"
#include "MatrixPlot.h"
#include <vagabond/gui/elements/ListView.h>
#include <vagabond/gui/Display.h>
#include <vagabond/utils/Eigen/Core>

class MatrixRotamer
{
public:
    std::string name{};
    std::string chain{};
    std::vector<std::string> labels{};
    Eigen::MatrixXi matrix{};
};

class RotamerMenu : public ListView
{
public:
    RotamerMenu(Scene *prev);
    virtual ~RotamerMenu();
    void setup();

    virtual size_t lineCount();
    virtual Renderable *getLine(int i);
    void loadMatrices();
    virtual void buttonPressed(std::string tag, Button *button = nullptr);

private:
    std::vector<MatrixRotamer> _matrixList{};
    FileManager *_manager{};
    MatrixPlot *_plot{};
};

#endif
