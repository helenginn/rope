#ifndef __vagabond__HeatMapVDW__
#define __vagabond__HeatMapVDW__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/PathGroup.h>
#include <vagabond/utils/svd/PCA.h>

class MatrixBox;
class MatrixPlot;

class HeatMapVDW : public Scene
{
public:
    HeatMapVDW(Scene *prev, const std::vector<PathGroup> &paths);

    virtual void setup();
private:
	std::vector<std::string> _squareNames;
	std::vector<std::string> _rowNames;
	std::vector<std::string> _colNames;

	PCA::Matrix _squareData;
	PCA::Matrix _rectData;

	MatrixPlot *_squarePlot{};
	MatrixPlot *_rectPlot{};

	MatrixBox *_squareBox{};
	MatrixBox *_rectBox{};
};

#endif
