#ifndef __vagabond__ComparisonScatterView__
#define __vagabond__ComparisonScatterView__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/ExportsCSV.h>
#include <vagabond/core/CompareMatrices.h>

class ComparisonScatterView : public Scene, public ExportsCSV
{
public: 
	ComparisonScatterView(Scene *prev, const ComparisonStats &stats);
	virtual void setup();
	virtual void buttonPressed(std::string tag, Button *button = nullptr);

protected: 
	virtual void supplyCSV(std::string indicator = "");


private:
	void makeMenu();
	std::string helpText();
	const ComparisonStats &_stats;
};

#endif