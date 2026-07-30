// Copyright (C) 2021 Helen Ginn

#include "InfoModal.h"
#include "TextButton.h"
#include "Text.h"

#define DEFAULT_WIDTH 0.6

InfoModal::InfoModal(Scene *scene, std::string details) :
Modal(scene, DEFAULT_WIDTH, 0.4)
{
	setStacked(true);

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

void InfoModal::buttonPressed(std::string tag, Button *button)
{
	if (tag == "OK")
	{
		hide();
	}
}
