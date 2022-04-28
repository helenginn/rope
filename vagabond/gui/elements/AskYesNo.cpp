// Copyright (C) 2021 Helen Ginn

#include "AskYesNo.h"
#include "TextButton.h"
#include "Text.h"

AskYesNo::AskYesNo(Scene *scene, std::string text, std::string tag,
                   ButtonResponder *sender) : 
Modal(scene, 0.6, 0.4)
{
	Text *t = new Text(text);
	t->setCentre(0.5, 0.45);
	addObject(t);

	{
		TextButton *button = new TextButton("No", this);
		button->setReturnTag("no");
		button->setCentre(0.3, 0.65);
		addObject(button);
	}

	{
		TextButton *button = new TextButton("Yes", this);
		button->setReturnTag("yes");
		button->setCentre(0.7, 0.65);
		addObject(button);
	}
	
	_sender = sender;
	_tag = tag;

	setDismissible(false);
}

void AskYesNo::buttonPressed(std::string tag, Button *button)
{
	_sender->buttonPressed(tag + "_" + _tag);

	hide();
}

AskYesNo::~AskYesNo()
{

}
