// Copyright (C) 2021 Helen Ginn

#ifndef __practical__TextButton__
#define __practical__TextButton__

#include "Text.h"
#include "Button.h"

class TextButton : public Text, public Button
{
public:
	TextButton(std::string text, ButtonResponder *sender, bool delay = false,
           Font::Type type = Font::Thin) :
	Text(text, type, delay), Button(sender) {};

private:

};

#endif
