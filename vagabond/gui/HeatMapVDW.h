#ifndef __vagabond__HeatMapVDW__
#define __vagabond__HeatMapVDW__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/core/PathGroup.h>

class MatrixBox;
class MatrixPlot;

class HeatMapVDW : public Scene
{
public:
    HeatMapVDW(Scene *prev, const std::vector<PathGroup> &paths);

    virtual void setup();

    void resetMatrix();
    void scaleMatrix(std::vector<float> dataVector);
    void printMatrixToTerminal();

private:
	std::vector<std::string> _squareNames;
	std::vector<std::string> _rowNames;
	std::vector<std::string> _colNames;

	Eigen::MatrixXf _squareData;
	Eigen::MatrixXf _rectData;

	MatrixPlot *_squarePlot{};
	MatrixPlot *_rectPlot{};

	MatrixBox *_squareBox{};
	MatrixBox *_rectBox{};
};

#endif
