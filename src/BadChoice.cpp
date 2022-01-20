// Copyright (C) 2021 Helen Ginn

#include "BadChoice.h"
#include "TextButton.h"
#include "Text.h"

BadChoice::BadChoice(Scene *scene, std::string details) :
Modal(scene, 0.6, 0.4)
{
	Text *text = new Text(details);
	setLeft(text, 0.1, 0.5);
	addObject(text);


	{
		TextButton *button = new TextButton("OK", this);
		button->setReturnTag("OK");
		setCentre(button, 0.5, 0.9);
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

