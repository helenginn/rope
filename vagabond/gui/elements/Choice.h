// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Choice__
#define __practical__Choice__

#include "Button.h"
#include "Text.h"
#include "Image.h"
#include "Scene.h"
#include <string>

class Choice : public Button
{
public:
	Choice(ButtonResponder *sender, Scene *scene);
	
	virtual ~Choice();

	virtual void click();
	void unclick();
	virtual void render(SnowGL *gl);
private:
	void makeTick();
	Scene *_scene;
	Image *_tick;
	bool _ticked;

};

class ChoiceImage : public Image, public Choice
{
public:
	ChoiceImage(std::string filename, Scene *scene,
	            ButtonResponder *sender = NULL)
	: Image(filename),
	Choice(sender, scene) {};

};

class ChoiceText : public Text, public Choice
{
public:
	ChoiceText(std::string text, Scene *scene,
	           ButtonResponder *sender = NULL)
	: Text(text),
	Choice(sender, scene) {};

};

#endif

