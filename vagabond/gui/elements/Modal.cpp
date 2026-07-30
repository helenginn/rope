// Copyright (C) 2021 Helen Ginn

#include "Modal.h"
#include "Scene.h"
#include "TextButton.h"
#include "ImageButton.h"
#include "InfoModal.h"
#include <iostream>

Modal::Modal(Scene *scene)
{
	_scene = scene;
}

void Modal::makeFreshBox(double width, double height)
{
	clearVertices();
	addQuad(0.9);
	setColour(0.0, 0.0, 0.0);
	rescale(width, height);
	_width = width;
	_height = height;
	forceRender(true, false);
}

Modal::Modal(Scene *scene, double width, double height) : Box()
{
	setImage("assets/images/box.png");

	_darker = new Box();
	_darker->addQuad(0.8);
	_darker->setAlpha(0.5);
	_darker->setFragmentShaderFile("assets/shaders/shade.fsh");
	
	makeFreshBox(width, height);
	
	_dismissable = true;

	_scene = scene;
}

void Modal::render(GLView *gl)
{
	if (_darker)
	{
		_darker->render(gl);
	}

	Box::render(gl);
}

void Modal::conv_coords(double *fx, double *fy)
{
	*fx = (*fx - 0.5) * _width + 0.5;
	*fy = (*fy - 0.5) * _height + 0.5;

}

void Modal::hide()
{
	_scene->removeModals(_stacked);
}

void Modal::dismiss()
{
	if (_dismissable)
	{
		hide();
	}
}

void Modal::setHelpText(const std::string &text)
{
	_helpText = text;

	if (!_help)
	{
		ImageButton *ib = new ImageButton("assets/images/question_mark.png",
		                                  nullptr);
		ib->resize(0.06);
		// this box is centred on screen by default, spanning roughly
		// [0.5 - _width/2, 0.5 + _width/2] x [0.5 - _height/2, 0.5 + _height/2]
		// in the same full-screen fraction space setCentre()/setRight() use
		// (see Renderable::setArbitrary()) - so a fixed inset from those
		// edges lands just inside the top-right corner of any Modal,
		// whatever its own width/height happen to be.
		double right = 0.5 + _width / 2 - 0.02;
		double top = 0.5 - _height / 2 + 0.06;
		ib->setRight(right, top);
		ib->setReturnJob([this]()
		{
			InfoModal *info = new InfoModal(_scene, _helpText);
			_scene->setModal(info, false);
		});
		addObject(ib);
		_help = ib;
	}
}

Modal::~Modal()
{
	if (_darker)
	{
		delete _darker;
		_darker = nullptr;
	}
}

void Modal::addTwoButtons(std::string left, std::string ltag,
                          std::string right, std::string rtag)
{
	{
		TextButton *button = new TextButton(left, this);
		button->setReturnTag(ltag);
		button->setCentre(0.3, 0.65);
		addObject(button);
	}

	{
		TextButton *button = new TextButton(right, this);
		button->setReturnTag(rtag);
		button->setCentre(0.7, 0.65);
		addObject(button);
	}

}

void Modal::squeezeToWidth(Renderable *r)
{
	float width = r->maximalWidth() / 2;
	if (width > _width)
	{
		r->resize(0.9 * _width / width);
	}
}
