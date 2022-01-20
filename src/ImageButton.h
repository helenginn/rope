// Copyright (C) 2021 Helen Ginn

#ifndef __practical__ImageButton__
#define __practical__ImageButton__

#include "Image.h"
#include "Button.h"

class ImageButton : public Image, public Button
{
public:
	ImageButton(std::string filename, ButtonResponder *sender) :
	Image(filename), Button(sender) {};

private:

};

#endif

