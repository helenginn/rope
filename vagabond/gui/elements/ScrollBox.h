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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __vagabond__ScrollBox__
#define __vagabond__ScrollBox__

#include <vagabond/gui/elements/Box.h>
#include <vagabond/gui/elements/DragResponder.h>

class Slider;

class ScrollBox : public Box, public DragResponder,
				  public Responder<Box>
{
public:
	ScrollBox();
	
	void setContent(Box *box);
	void setBounds(glm::vec4 frac);

	virtual void finishedDragging(std::string tag, double x, double y);

protected:
	virtual void respond();
	virtual void extraUniforms();
private:
	bool hasSlider()
	{
		return _slider;
	}

	float glMaxHeight();
	void heightChanged();
	float glBoundHeight();
	float screenMidHeight();
	float screenBoundRight();
	float screenBoundHeight();
	void updateScrollOffset(float offset);


	void deleteSlider();
	void drawSlider();
	void addSliderIfNeeded();

	glm::vec4 _screenBounds = {0, 0, 1, 1};
	glm::vec4 _glBounds = {-1, -1, 1, 1};

	Box *_content = nullptr;
	Slider *_slider = nullptr;
	
	float _scrollOffset = 0;
};

#endif
