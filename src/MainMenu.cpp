// Copyright (C) 2021 Helen Ginn

#include "MainMenu.h"
#include "TextButton.h"
#include <iostream>

MainMenu::MainMenu() : Scene(), ButtonResponder()
{

}

MainMenu::~MainMenu()
{
	deleteObjects();
}

void MainMenu::setup()
{
	_proj = glm::mat4(1.);
	clearObjects();
	addObject(_background);

	{
		Text *text = new Text("Vagabond");
		std::cout << glm::to_string(text->centroid()) << std::endl;
		setCentre(text, 0.5, 0.1);
		addObject(text);
	}
}

void MainMenu::buttonPressed(std::string tag, Button *button)
{

}
