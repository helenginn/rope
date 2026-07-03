// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If n ot, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __vagabond__FlexibilityController__
#define __vagabond__FlexibilityController__


#include <vagabond/gui/Display.h>
#include <vagabond/gui/elements/DragResponder.h>
#include <vagabond/core/Flexibility.h>
#include <vagabond/core/FlexibilityTypes.h>
#include <memory>

class Flexibility;
class FlexibilityView;
class Instance;
class HBondManager;

class FlexibilityController: public Display, public DragResponder
{
public:
	FlexibilityController(FlexibilityView *view, Instance *instance, Flexibility *flex);
	void showMenu(Button *button);
	bool handleButton(const std::string &tag, Button *button);
    void reset();
    // void checkHBondSelection();
    void callAddHBonds(const std::vector<HBondManager::HBondPair> &donorAcceptorPairs); 
    void callPrepareResources()
    {
        _flex->prepareResources();
    }
    void callSubmitJobAndRetrieve(float weight)
    {
        _flex->submitJobAndRetrieve(weight);
    }
    virtual void finishedDragging(std::string tag, double x, double y);
    const std::vector<HBondEntity>& handleHBondTicks()
    {
        if (!_flex)
        {
            static std::vector<HBondEntity> empty;
            std::cerr << "[ERROR] Flexibility not initialized!\n";
            return empty;
        }
        return _flex->getHBonds();
    }
    const std::vector<VdWBondEntity>& handleVdWTicks()
    {
        if (!_flex)
        {
            static std::vector<VdWBondEntity> empty;
            std::cerr << "[ERROR] Flexibility not initialized!\n";
            return empty;
        }
        return _flex->getVdWBonds();
    }
    

protected:
	void handleClearHBonds();
    void handleSaveState();
    void handleExportPDB(Button *button);
    void handleSelectedHBonds(Button *button);
    void handleBFactorCloud();
    void handleRangeMin(Button* button);
    void handleRangeMax(Button* button);
    void handleSaveSamples();
    void handleNumSamples(Button* button);
    void handleDistMatrix();
    void handleColumnIdx(Button* button);




private:
	FlexibilityView *_view = nullptr;
    Flexibility *_flex = nullptr;
    Instance *_instance = nullptr;
    std::string _tag;

    bool _first = true;
    bool _selectFlag = false;
    float _minRange  = 0.0f;
    float _maxRange  = 0.0f;
    float _currentWeight = 0.0f;

    std::vector<HBondManager::HBondPair> _hBondPairs;

};
#endif