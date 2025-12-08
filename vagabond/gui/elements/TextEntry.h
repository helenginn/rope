// Copyright (C) 2021 Helen Ginn

#ifndef __practical__TextEntry__
#define __practical__TextEntry__

#include "Text.h"
#include "Button.h"
#include "KeyResponder.h"
#include <vagabond/core/Responder.h>
#include <functional>

class Scene;

class TextEntry : public Text, public Button, public KeyResponder,
public HasResponder<Responder<TextEntry>>
{
public:
	TextEntry(std::string text, ButtonResponder *sender, 
	          ButtonResponder *scene = nullptr) :
	Text(text), Button(sender)
	{
		_responder = scene;
		if (_responder == nullptr)
		{
			_responder = sender;
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
	
	void allowMultiLine(bool allow)
	{
		_multiLine = allow;
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
	
	void setStretchFunction(const std::function<void(TextEntry *)> &stretch)
	{
		_stretch = stretch;
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
	ButtonResponder *_responder = nullptr;
	std::function<void(TextEntry *)> _stretch;
	bool _multiLine{false};
};

#endif
