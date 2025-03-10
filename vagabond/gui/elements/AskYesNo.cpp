// Copyright (C) 2021 Helen Ginn

#include "AskYesNo.h"
#include "TextButton.h"

AskYesNo::AskYesNo(Scene *scene, std::string text, std::string tag,
                   ButtonResponder *sender) : 
Modal(scene, 0.6, 0.4)
{
	Text *t = new Text(text);
	t->setCentre(0.5, 0.45);
	addObject(t);

	squeezeToWidth(t);

	addTwoButtons("No", "no", "Yes", "yes");
	
	_sender = sender;
	_tag = tag;

	setDismissible(false);
}

void AskYesNo::buttonPressed(std::string tag, Button *button)
{
	if (_jobs.count(tag))
	{
		hide();
		_jobs[tag]();
	}
	else
	{
		hide();
		if (_sender)
		{
			_sender->buttonPressed(tag + "_" + _tag);
		}
	}
}

AskYesNo::~AskYesNo()
{

}
