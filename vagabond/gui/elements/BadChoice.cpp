// Copyright (C) 2021 Helen Ginn

#include "BadChoice.h"
#include "TextButton.h"
#include "Text.h"

#define DEFAULT_WIDTH 0.6

BadChoice::BadChoice(Scene *scene, std::string details) :
Modal(scene, DEFAULT_WIDTH, 0.4)
{
	Text *text = new Text(details);
	text->setCentre(0.5, 0.5);
	text->resize(1.0);
	addObject(text);
	
	squeezeToWidth(text);

	{
		TextButton *button = new TextButton("OK", this);
		button->setReturnTag("OK");
		button->setCentre(0.5, 0.65);
		addObject(button);
	}

	setDismissible(false);
}

void BadChoice::buttonPressed(std::string tag, Button *button)
{
	if (tag == "OK")
	{
		hide();
	}
}

