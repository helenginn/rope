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
	
	/** 90 for left-facing, -90 for right-facing */
	static ImageButton *arrow(float deg, ButtonResponder *sender)
	{
		ImageButton *b = new ImageButton("assets/images/arrow.png", sender);
		b->rescale(0.1, 0.03);
		glm::mat3x3 rot;
		rot = glm::mat3x3(glm::rotate(glm::mat4(1.), (float)deg2rad(deg),
		                              glm::vec3(0., 0., -1.)));
		b->rotateRoundCentre(rot);
		return b;
	}

	void arrowInDir(float deg_cw)
	{

	}
private:

};

#endif

