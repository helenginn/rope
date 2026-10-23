#ifndef __vagabond__ComparisonDetailView__
#define __vagabond__ComparisonDetailView__

#include <vagabond/gui/elements/ListView.h>
#include <vagabond/core/CompareMatrices.h>

class ComparisonDetailView : public ListView
{
public:
	enum Kind 
	{
		Correlation,
		Common
	};

	ComparisonDetailView(Scene *prev, const ComparisonStats &stats, Kind kind);
	virtual void setup();
	virtual size_t lineCount();
	virtual Renderable *getLine(int i);

private: 
	void makeMenu();
	std::string helpText();
	
	std::vector<std::string> _lines;
	Kind _kind;
	const ComparisonStats &_stats;
};

#endif