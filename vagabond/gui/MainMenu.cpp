// Copyright (C) 2021 Helen Ginn

#include "Display.h"
#include "MainMenu.h"
#include "FileView.h"
#include "TextButton.h"
#include <iostream>

MainMenu::MainMenu() : Scene()
{

}

MainMenu::~MainMenu()
{
	deleteObjects();
}

void MainMenu::setup()
{
	clearObjects();
	addObject(_background);

	{
		Text *text = new Text("Vagabond");
		setCentre(text, 0.5, 0.1);
		addObject(text);
	}

	{
		TextButton *button = new TextButton("View loaded files", this);
		button->setReturnTag("show_files");
		setCentre(button, 0.5, 0.3);
		addObject(button);
	}
}

void MainMenu::buttonPressed(std::string tag, Button *button)
{
	if (tag == "show_files")
	{
		FileView *fileview = new FileView(this);
		fileview->show();
	}
	else if (tag == "model")
	{
		Display *display = new Display(this);
		display->show();
	}

	Scene::buttonPressed(tag, button);
}
