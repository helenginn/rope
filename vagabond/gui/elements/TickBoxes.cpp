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

#include "TickBoxes.h"
#include "TextButton.h"
#include "ImageButton.h"

TickBoxes::TickBoxes(Scene *scene, ButtonResponder *responder,
                     const std::string &main_tag) : Box(), Button(responder)
{
	_scene = scene;
	_responder = responder;
	_tag = main_tag;
}

TickBoxes::~TickBoxes()
{

}

void TickBoxes::addOption(const std::string &text, std::string tag,
                          bool ticked)
{
	if (tag.length() == 0)
	{
		tag = text;
	}

	_options.push_back(Option(this, text, tag, ticked));
}

float TickBoxes::resizeNeeded(float width, float height)
{
	float total_width = 0;
	float total_height = 0;
	for (Option &option : _options)
	{
		total_width += option.totalDim(0);
		total_height += option.totalDim(1);
	}
	
	float resize_height = std::min(width / total_width, 1.f);
	float resize_width = std::min(height / total_height, 1.f);
	
	float resize = std::min(resize_width, resize_height);
	return resize;
}

void TickBoxes::arrange(float topleft_x, float topleft_y, 
                        float bottomright_x, float bottomright_y)
{
	float width = bottomright_x - topleft_x;
	float height = bottomright_y - topleft_y;

	float resize = resizeNeeded(width, height);
	
	float step_x = (_vertical ? 0 : width / _options.size());
	float step_y = (_vertical ? height / _options.size() : 0);
	
	float x = 0;
	float y = 0;
	
	for (Option &option : _options)
	{
		option.tickbox->resize(resize);
		option.text->resize(resize);

		option.tickbox->setCentre(x, y - 0.01);
		option.text->setLeft(x + 0.01, y);
		addObject(option.tickbox);
		addObject(option.text);
		
		x += step_x;
		y += step_y;
	}
	
	setArbitrary(topleft_x, topleft_y, Alignment(Left | Top));
}

TickBoxes::Option *TickBoxes::optionForTag(const std::string &tag)
{
	for (Option &option : _options)
	{
		if (option.tag == tag)
		{
			return &option;
		}
	}

	return nullptr;
}

std::string image_for_state(const bool &ticked)
{
	return (ticked ? "assets/images/tickbox_filled.png" :
	        "assets/images/tickbox_empty.png");
}

void TickBoxes::untickAllExcept(const std::string &tag)
{
	if (!_oneOnly)
	{
		return;
	}

	for (Option &option : _options)
	{
		if (option.tag == tag)
		{
			continue;
		}

		std::string empty = image_for_state(false);
		option.ticked = false;
		option.tickbox->changeImage(empty);
	}
}

void TickBoxes::setInert(const std::string &tag, bool inert)
{
	Option *opt = optionForTag(tag);
	if (!opt)
	{
		std::cout << "?? something has gone wrong" << std::endl;
		return;
	}

	opt->tickbox->setInert(inert);
	opt->text->setInert(inert);
}

void TickBoxes::tick(const std::string &tag, bool yes)
{
	untickAllExcept(tag);
	Option *opt = optionForTag(tag);
	if (!opt)
	{
		std::cout << "?? something has gone wrong" << std::endl;
		return;
	}

	opt->tickbox->changeImage(image_for_state(yes));
	opt->ticked = yes;
}

void TickBoxes::toggle(const std::string &tag)
{
	Option *opt = optionForTag(tag);
	if (!opt)
	{
		std::cout << "?? something has gone wrong" << std::endl;
		return;
	}

	std::string new_image = (opt->ticked ? "assets/images/tickbox_empty.png" :
	                         "assets/images/tickbox_filled.png");
	
	opt->ticked = !opt->ticked;
	opt->tickbox->changeImage(image_for_state(opt->ticked));
}

bool TickBoxes::isTicked(const std::string &tag)
{
	Option *opt = optionForTag(tag);
	return (opt && opt->ticked);
}

void TickBoxes::buttonPressed(std::string tag, Button *button)
{
	untickAllExcept(tag);
	toggle(tag);
	
	std::string response_tag = (_tag.length() > 0 ? _tag + "_" + tag : tag);
	
	reaction(response_tag);
}

TickBoxes::Option::Option(TickBoxes *boxes, const std::string &str, 
                          const std::string &new_tag, bool tkt)
{
	tag = new_tag;
	ticked = tkt;

	text = new TextButton(str, boxes);
	text->setReturnTag(tag);
	text->setLeft(0.01, 0.00);

	tickbox = new ImageButton(image_for_state(ticked), boxes);
	tickbox->setCentre(0.00, 0.00);
	tickbox->resize(0.05);
	tickbox->setReturnTag(tag);
}

TickBoxes::Option::~Option()
{
}

float TickBoxes::Option::totalDim(int dim)
{
	double min = FLT_MAX; double max = -FLT_MAX;
	tickbox->maximalDim(&min, &max, dim);

	return max - min;
}
