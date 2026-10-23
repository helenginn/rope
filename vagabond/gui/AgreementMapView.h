#ifndef __vagabond__AgreementMapView__
#define __vagabond__AgreementMapView__

#include <mutex>
#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/ExportsCSV.h>
#include <vagabond/core/CompareMatrices.h>
#include <vagabond/utils/svd/PCA.h>

class MatrixPlot;

// residue x residue map of z1 * z2: red where the matrices agree, blue where not
class AgreementMapView : public Scene, public ExportsCSV
{
public:
	AgreementMapView(Scene *prev, const ComparisonStats &stats);
	virtual ~AgreementMapView();

	virtual void setup() override;
	virtual void buttonPressed(std::string tag, Button *button = nullptr) override;
	virtual void mousePressEvent(double x, double y,
	                             SDL_MouseButtonEvent button) override;
protected:
	virtual void supplyCSV(std::string indicator = "") override;
private:
	void makeMenu();
	void showPairAt(double x, double y);

	const ComparisonStats &_stats;
	PCA::Matrix _colours{};   // agreement mapped onto the colour scale
	std::mutex _mutex;        // MatrixPlot expects one
	MatrixPlot *_plot = nullptr;
};

#endif