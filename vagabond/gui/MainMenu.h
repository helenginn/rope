// Copyright (C) 2021 Helen Ginn

#ifndef __practical__MainMenu__
#define __practical__MainMenu__

#include "Scene.h"

class MainMenu : public Scene
{
public:
	MainMenu();
	
	virtual ~MainMenu();

	virtual void setup();
	virtual void buttonPressed(std::string tag, Button *button = NULL);
private:

};

#endif
