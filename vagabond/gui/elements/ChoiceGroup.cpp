// Copyright (C) 2021 Helen Ginn

#include "ChoiceGroup.h"
#include "Choice.h"
#include "Scene.h"
#include <iostream>

ChoiceGroup::ChoiceGroup(Scene *scene, ButtonResponder *responder) : Box()
{
	_scene = scene;
	_responder = responder;
	setName("Choice");
}

ChoiceImage *ChoiceGroup::addImage(std::string filename, std::string tag)
{
	ChoiceImage *image = new ChoiceImage(filename, _scene, this);
	image->setReturnTag(tag);
	addChoice(image);
	return image;
}

void ChoiceGroup::addChoice(Choice *choice)
{
	_choices.push_back(choice);
	addObject(choice);
}

ChoiceText *ChoiceGroup::addText(std::string text, std::string tag)
{
	if (tag.length() == 0)
	{
		tag = text;
	}

	setName(name() + " " + text);

	ChoiceText *choice = new ChoiceText(text, _scene, this);
	choice->setReturnTag(tag);
	addChoice(choice);
	return choice;
}

void ChoiceGroup::arrange(double resize, double ctx, double cty,
                          double xspan, double yspan)
{
	double count = choiceCount();
	double xinit = 0 - xspan / 2;
	double xstep = xspan / count;
	xinit += xstep / 2;

	double yinit = 0 - yspan / 2;
	double ystep = yspan / count;
	yinit += ystep / 2;

	for (double i = 0; i < _choices.size(); i++)
	{
		double x = xinit + i * xstep;
		double y = yinit + i * ystep;

		choice(i)->resize(resize);
		choice(i)->setCentre(x, y);
	}
	
	setCentre(ctx, cty);
}

void ChoiceGroup::buttonPressed(std::string tag, Button *button)
{
	for (size_t i = 0; i < choiceCount(); i++)
	{
		choice(i)->unclick();
	}
	
	_tag = tag;
	
	if (_responder)
	{
		_responder->buttonPressed(tag, button);
	}
}

void ChoiceGroup::setInert(bool inert)
{
	for (size_t i = 0; i < choiceCount(); i++)
	{
		choice(i)->setInert(inert);
	}
}

void ChoiceGroup::takeChoice(Choice *c)
{
	c->setSender(this);
	addChoice(c);

}
