// Copyright (C) 2021 Helen Ginn

#ifndef __practical__TextEntry__
#define __practical__TextEntry__

#include "Text.h"
#include "Button.h"
#include "KeyResponder.h"

class TextEntry : public Text, public Button, public KeyResponder
{
public:
	TextEntry(std::string text, ButtonResponder *sender) :
	Text(text), Button(sender) {};

	virtual void keyPressed(char key);
	virtual void keyPressed(SDL_Keycode other);

	virtual void click();
	virtual void finish();
	bool validateKey(char key);
private:
	void showInsert();

	std::string _scratch;
	bool _active = false;
};

#endif
