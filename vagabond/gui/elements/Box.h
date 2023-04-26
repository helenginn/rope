// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Box__
#define __practical__Box__

#include "Renderable.h"
#include "DragResponder.h"
#include "ButtonResponder.h"

class Slider;

class Box : public Renderable
{
public:
	Box();
	
	void switchToScrolling(DragResponder *resp);
	void setScrollBox(glm::vec4 offset);
	void updateScrollOffset(float offset);

	virtual void addQuad(double z = 0);

	virtual void addSplitQuad(double z = 0);
	void rescale(double x, double y);
	virtual void makeQuad();
	
	virtual bool mouseOver();
	virtual void unMouseOver();
	virtual void modify(Renderable *r);
	
	void addAltTag(std::string text);
	void updateScrollBox(glm::vec4 bounds);
	void copyScrolling(const Box *other);
protected:
	virtual void extraUniforms();
	void assessBounds();
	void deleteSlider();
	void drawSlider();
	void setToScrollShaders();

	float _scrollOffset = 0;
	glm::vec4 _scrollBox = glm::vec4(0.f);
	glm::vec4 _bounds = glm::vec4(0.f);
	DragResponder *_dragResponder = nullptr;
	Slider *_slider = nullptr;
};

#endif
