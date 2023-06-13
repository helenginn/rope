// Copyright (C) 2021 Helen Ginn

#ifndef __practical__TextEntry__
#define __practical__TextEntry__

#include "Text.h"
#include "Button.h"
#include "KeyResponder.h"
#include <vagabond/core/Responder.h>

class TextEntry : public Text, public Button, public KeyResponder,
public HasResponder<Responder<TextEntry>>
{
public:
	TextEntry(std::string text, ButtonResponder *sender, 
	          ButtonResponder *scene = nullptr) :
	Text(text), Button(sender)
	{
		_scene = scene;
		if (_scene == nullptr)
		{
			_scene = sender;
		}
	}
	
	enum Validation
	{
		Numeric,
		Alphabetic,
		Id,
		None
	};
	
	void setValidationType(Validation v)
	{
		_validation = v;
	}
	
	void allowCapitals(bool allow)
	{
		_capitals = allow;
	}

	virtual void keyPressed(char key);
	virtual void keyPressed(SDL_Keycode other);
	
	std::string scratch()
	{
		return _scratch;
	}
	
	void setScratch(std::string scratch)
	{
		_scratch = scratch;
	}

	virtual void click(bool left = true);
	virtual void finish();
	bool validateKey(char key);
	
	float as_num() const;
private:
	void showInsert();
	void shiftKey(char &key);

	std::string _scratch;
	bool _active = false;
	bool _capitals = false;
	Validation _validation = None;
	ButtonResponder *_scene = nullptr;
};

#endif
