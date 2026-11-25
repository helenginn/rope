//
// Created by romain on 20/04/2026.
//

#ifndef __vagabond__RotamerMenu__
#define __vagabond__RotamerMenu__

#include "FileManager.h"
#include "MatrixPlot.h"
#include <vagabond/gui/elements/ListView.h>
#include <vagabond/gui/Display.h>
#include <vagabond/gui/elements/DragResponder.h>
#include <vagabond/utils/Eigen/Core>
#include <vagabond/utils/gl_import.h>

class RotamerModifier;
class Instance;
class Slider;
class Line;

class MatrixRotamer
{
public:
    std::string name{};
    std::string chain{};
    std::vector<std::string> labels{};
    Eigen::MatrixXi matrix{};
    glm::vec3 startPos{};
};

class CombinedView : public Display, public DragResponder
{
public:
    CombinedView(Scene *prev, Instance *instMain, Instance *instSec, MatrixRotamer matRot);
    virtual ~CombinedView();
    void setup();
    void view();
    void preSetup();
    void buttonPressed(std::string tag, Button *button);

    virtual void finishedDragging(std::string tag, double x, double y);

private:
    Instance *_instMain{};
    Instance *_instSec{};
    RotamerModifier *_modifier{};
    MatrixRotamer _matrix{};
    void updateIndicator(float distance);

    std::vector<MatrixRotamer> _matrixList{};
    FileManager *_manager{};
    MatrixPlot *_plot{};
    bool _test{false};

    Display *_combinedView{};
    Line *_indicator{};
    Slider *_slider{};
    int _matrixRows{};
    float _prevDistance{0.f};
};

class RotamerMenu : public ListView, public DragResponder
{
public:
    RotamerMenu(Scene *prev, bool test = false, Instance *instMain = nullptr, Instance *instSec = nullptr);
    virtual ~RotamerMenu();
    void setup();

    virtual void finishedDragging(std::string tag, double x, double y);
    virtual size_t lineCount();
    virtual Renderable *getLine(int i);
    void loadMatrices();
    virtual void buttonPressed(std::string tag, Button *button = nullptr);

private:
    void openCombinedView(const MatrixRotamer &m);

    std::vector<MatrixRotamer> _matrixList{};
    FileManager *_manager{};
    MatrixPlot *_plot{};
    bool _test{false};

    CombinedView *_combinedView{};
    RotamerModifier *_modifier{};
    Instance *_instMain{};
    Instance *_instSec{};
    Line *_indicator{};
    Slider *_slider{};
    glm::vec3 _startPos{};
    int _matrixRows{};
    float _prevDistance{0.f};
};

#endif
