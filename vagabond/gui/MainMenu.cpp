// Copyright (C) 2021 Helen Ginn

#include "GlobalOptions.h"
#include "ProgressView.h"
#include "MetadataView.h"
#include "DatasetMenu.h"
#include "SandboxView.h"
#include "EntityMenu.h"
#include "ModelMenu.h"
#include "MainMenu.h"
#include "FileView.h"

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/Menu.h>

#include <vagabond/core/Environment.h>
#include <vagabond/utils/version.h>

#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
EM_JS(void, upload_json, (), { document.getElementById('input').click() });
#endif



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

	addTitle("RoPE");

	{
		TextButton *text = new TextButton("Menu", this);
		text->setReturnTag("file_menu");
		text->setRight(0.9, 0.1);
		addObject(text);
	}

	{
		ImageButton *button = new ImageButton("assets/images/files.png", this);
		button->resize(0.2);
		button->setReturnTag("files");
		button->setCentre(0.2, 0.3);
		addObject(button);

		Text *text = new Text("View loaded files");
		text->setCentre(0.2, 0.45);
		addObject(text);
	}

	{
		ImageButton *button = new ImageButton("assets/images/cartoon.png", this);
		button->resize(0.25);
		button->setReturnTag("proteins");
		button->setCentre(0.5, 0.3);
		addObject(button);

		Text *text = new Text("Protein entities");
		text->setCentre(0.5, 0.45);
		addObject(text);
	}

	{
		ImageButton *button = new ImageButton("assets/images/model.png", this);
		button->resize(0.25);
		button->setReturnTag("models");
		button->setCentre(0.8, 0.3);
		addObject(button);

		Text *text = new Text("Models");
		text->setCentre(0.8, 0.45);
		addObject(text);
	}

	{
		ImageButton *button = new ImageButton("assets/images/misc_data.png", this);
		button->resize(0.25);
		button->setReturnTag("metadata");
		button->setCentre(0.2, 0.65);
		addObject(button);

		Text *text = new Text("Metadata");
		text->setCentre(0.2, 0.80);
		addObject(text);
	}

#ifdef VERSION_SANDBOX
	{
		ImageButton *button = new ImageButton("assets/images/tube_1.5ml.png", this);
		button->resize(0.15);
		button->setReturnTag("sandbox");
		button->setCentre(0.8, 0.65);
		addObject(button);

		Text *text = new Text("Sandbox");
		text->setCentre(0.8, 0.80);
		addObject(text);
	}
#endif
}

void MainMenu::doThings()
{
	if (_checkFiles)
	{
		checkForJson();
	}
}

void MainMenu::respond()
{
	_checkFiles = true;
}

void MainMenu::checkForJson()
{
	_checkFiles = false;

	FileManager *fm = Environment::fileManager();
	fm->setFilterType(File::Json);
	int i = fm->filteredCount() - 1;
	std::string newFile = fm->filtered(i);
	File::Type type = File::typeUnknown(newFile);

	Environment::fileManager()->Progressor::setResponder(nullptr);

	if (type & File::Json)
	{
		ProgressView *pg = new ProgressView(this);
		pg->attachToEnvironment();
		pg->show();

		std::cout << ":)" << std::endl;
		Environment::environment().load(newFile);
	}
}

void MainMenu::buttonPressed(std::string tag, Button *button)
{
	std::cout << tag << std::endl;
	if (tag == "files")
	{
		FileView *fileview = new FileView(this);
		fileview->show();
	}
	else if (tag == "file_menu")
	{
		glm::vec2 c = button->xy();
		Menu *m = new Menu(this, this, "file");
#ifdef __EMSCRIPTEN__
		m->addOption("Load", "load");
#endif
		m->addOption("Save", "save");
		m->addOption("Options", "options");
		m->setup(c.x, c.y);
		setModal(m);
	}
	else if (tag == "file_save")
	{
		Environment::environment().save();
	}
	else if (tag == "file_load")
	{
#ifdef __EMSCRIPTEN__
	Environment::fileManager()->HasResponder<Responder<FileManager>>::setResponder(this);
	upload_json();
#endif
	}
	else if (tag == "file_options")
	{
		GlobalOptions *go = new GlobalOptions(this);
		go->show();
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
	else if (tag == "metadata")
	{
		Metadata *md = Environment::metadata();
		MetadataView *mv = new MetadataView(this, md);
		mv->show();
	}
	else if (tag == "datasets")
	{
		DatasetMenu *menu = new DatasetMenu(this);
		menu->show();
	}
	else if (tag == "sandbox")
	{
		SandboxView *sb = new SandboxView(this);
		sb->show();
	}

	Scene::buttonPressed(tag, button);
}
