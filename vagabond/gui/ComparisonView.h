#ifndef __vagabond__ComparisonView__
#define __vagabond__ComparisonView__

#include <vagabond/gui/elements/ListView.h>
#include <vagabond/core/CompareMatrices.h>

class ComparisonView : public ListView
{
public:
	enum Entry 
	{
		Correlation, 
		Common,
		ScatterPlot, 
		AgreementMap, 
		EntryCount
	};

	ComparisonView(Scene *prev, const ComparisonStats &stats);

	virtual void setup();
	virtual size_t lineCount();
	virtual Renderable *getLine(int i);
	virtual void buttonPressed(std::string tag, Button *button = nullptr);
private:
	ComparisonStats _stats;
};

#endif