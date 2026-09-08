// vagabond
// Copyright (C) 2026 Helen Ginn
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __vagabond__OccupancyComparisonView__
#define __vagabond__OccupancyComparisonView__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/Responder.h>
#include <vagabond/core/TorsionData.h>

#include <memory>
#include <atomic>

class Entity;
class Metadata;
class Residue;
class RotamerOccupancy;
class ChooseHeader;
class TextButton;
class ImageButton;
class SequenceSlider;

class OccupancyComparisonView : public Scene, public Responder<ChooseHeader>
{
public:
	OccupancyComparisonView(Scene *prev, Entity *entity);
	~OccupancyComparisonView();

	virtual void setup();
	virtual void buttonPressed(std::string tag, Button *button = nullptr);
	virtual void sendObject(std::string header, void *object);
private:
	void addHeaderButton();
	void addStubButtons();
	void refreshHeaderButton();
	void startMeasurement();
	void showPerResidue();

	Entity *_entity = nullptr;
	Metadata *_md = nullptr;
	TorsionData _group = TorsionData(0);
	std::shared_ptr<RotamerOccupancy> _rota;

	std::string _header;
	TextButton *_headerButton = nullptr;
	TextButton *_perResidueButton = nullptr;
	ImageButton *_perResidueArrow = nullptr;
	TextButton *_pairwiseButton = nullptr;
	ImageButton *_pairwiseArrow = nullptr;

	SequenceSlider *_slider = nullptr;
	Residue *_selectedResidue = nullptr;

	std::shared_ptr<std::atomic<bool>> _cancelled;
};

#endif
