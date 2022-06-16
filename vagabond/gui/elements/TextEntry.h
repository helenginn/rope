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
	
	enum Validation
	{
		Numeric,
		Id,
		None
	};
	
	void setValidationType(Validation v)
	{
		_validation = v;
	}

	virtual void keyPressed(char key);
	virtual void keyPressed(SDL_Keycode other);
	
	std::string scratch()
	{
		return _scratch;
	}

	virtual void click();
	virtual void finish();
	bool validateKey(char key);
	
	float as_num() const;
private:
	void showInsert();

	std::string _scratch;
	bool _active = false;
	Validation _validation = None;
};

#endif
