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

#ifndef __vagabond__TickBoxes__
#define __vagabond__TickBoxes__

#include "Button.h"
#include "ButtonResponder.h"

class Scene;
class TextButton;
class ImageButton;

class TickBoxes : public Button, public ButtonResponder
{
public:
	TickBoxes(Scene *scene, ButtonResponder *responder, 
	          const std::string &main_tag = "");
	
	virtual ~TickBoxes();
	
	void setOneOnly(const bool &only)
	{
		_oneOnly = only;
	}

	void setVertical(const bool &vert)
	{
		_vertical = vert;
	}

	const std::string &tag() const
	{
		return _tag;
	}
	
	void setInert(const std::string &tag, bool inert = true);
	void tick(const std::string &tag, bool yes = true);
	void toggle(const std::string &tag);
	
	bool isTicked(const std::string &tag);
	
	void addOption(const std::string &text, const std::function<void()> &job,
	               bool ticked = false);
	
	void addOption(const std::string &text, std::string tag = "",
	               bool ticked = false);
	
	void arrange(float topleft_x, float topleft_y, 
	             float bottomright_x, float bottomright_y);

	virtual void buttonPressed(std::string tag, Button *button);
private:
	ButtonResponder *_responder = nullptr;
	Scene *_scene = nullptr;

	void untickAllExcept(const std::string &tag);
	float resizeNeeded(float width, float height);
	
	struct Option
	{
		std::string str;
		std::string tag;
		TextButton *text = nullptr;
		ImageButton *tickbox = nullptr;
		TickBoxes *parent = nullptr;
		std::function<void()> job{};
		bool ticked = false;
		
		float totalDim(int dim);
		
		void set(TickBoxes *boxes, const std::string &str, 
		         const std::string &tag, const std::function<void()> &job, 
		         bool tkt);
		Option(TickBoxes *parent, const std::string &text, 
		       const std::function<void()> &job, bool ticked);
		Option(TickBoxes *parent, const std::string &text, 
		       const std::string &tag, bool ticked);
		~Option();
	};
	
	Option *optionForTag(const std::string &tag);

	std::vector<Option> _options;
	std::string _tag;
	
	bool _oneOnly = true;
	bool _vertical = false;
};

#endif
