// Copyright (C) 2021 Helen Ginn

#include "DatasetMenu.h"
#include "EntityMenu.h"
#include "ModelMenu.h"
#include "MainMenu.h"
#include "FileView.h"
#include "TextButton.h"
#include "ImageButton.h"
#include <vagabond/core/Environment.h>
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

	addTitle("Vagabond sandbox");

#ifndef __EMSCRIPTEN__
	{
		TextButton *text = new TextButton("Save", this);
		text->setReturnTag("save");
		text->setRight(0.9, 0.1);
		addObject(text);
	}
#endif

	{
		ImageButton *button = new ImageButton("assets/images/files.png", this);
		button->resize(0.2);
		button->setReturnTag("files");
		button->setCentre(0.2, 0.3);
		addObject(button);

		Text *text = new Text("View loaded files");
		text->setCentre(0.2, 0.5);
		addObject(text);
	}

	{
		ImageButton *button = new ImageButton("assets/images/cartoon.png", this);
		button->resize(0.25);
		button->setReturnTag("proteins");
		button->setCentre(0.5, 0.3);
		addObject(button);

		Text *text = new Text("Protein entities");
		text->setCentre(0.5, 0.5);
		addObject(text);
	}

	{
		ImageButton *button = new ImageButton("assets/images/model.png", this);
		button->resize(0.25);
		button->setReturnTag("models");
		button->setCentre(0.8, 0.3);
		addObject(button);

		Text *text = new Text("Models");
		text->setCentre(0.8, 0.5);
		addObject(text);
	}

	{
		ImageButton *button = new ImageButton("assets/images/data_model.png", this);
		button->resize(0.3);
		button->setReturnTag("datasets");
		button->setCentre(0.2, 0.6);
		addObject(button);

		Text *text = new Text("Dataset preparation");
		text->setCentre(0.2, 0.8);
		addObject(text);
	}
}

void MainMenu::buttonPressed(std::string tag, Button *button)
{
	if (tag == "files")
	{
		FileView *fileview = new FileView(this);
		fileview->show();
	}
	else if (tag == "save")
	{
		Environment::environment().save();
	}
	else if (tag == "proteins")
	{
		EntityMenu *menu = new EntityMenu(this);
		menu->show();
	}
	else if (tag == "models")
	{
		ModelMenu *menu = new ModelMenu(this);
		menu->show();
	}
	else if (tag == "datasets")
	{
		DatasetMenu *menu = new DatasetMenu(this);
		menu->show();
	}

	Scene::buttonPressed(tag, button);
}
