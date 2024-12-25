#ifndef __vagabond__HBondMenu__
#define __vagabond__HBondMenu__

#include <vagabond/gui/elements/ListView.h>
#include <vagabond/core/HBondManager.h>
#include <vagabond/core/HBondData.h>
#include <HBondDataView.h>
#include <vagabond/core/Responder.h>
#include <vagabond/core/files/File.h>
#include <vagabond/gui/elements/Scene.h>
#include <functional>


class HBondDataView;

class HBondMenu : public ListView, public Responder<HBondManager>
{
public:
	HBondMenu(Scene *prev);
	~HBondMenu();
	virtual void refresh();
	virtual void setup();
	virtual size_t lineCount();
	virtual Renderable *getLine(int i);
	virtual void buttonPressed(std::string tag, Button *button = nullptr);
	virtual void respond();
	bool selectMode()
	{
		return _selectMode;
	}
	void setMode(bool mode)
	{	
		_selectMode = mode;
	}
	std::vector<HBondManager::HBondPair> getPairs()
	{
		return _hbondPairs;
	}
	HBondManager *getManager()
	{
		return _manager;
	}
	void setCallBackFunction(std::function<void(std::vector<HBondManager::HBondPair>)> callback)
    {
        _callback = callback;
    }
private:
	std::function<void(std::vector<HBondManager::HBondPair>)> _callback;
	std::vector<HBondManager::HBondPair> _hbondPairs;
	HBondManager *_manager;
	bool _selectMode = false;
	HBondData *_currHBond = nullptr;
};

#endif