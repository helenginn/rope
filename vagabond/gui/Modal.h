// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Modal__
#define __practical__Modal__

#include "Box.h"
#include <vector>

class Scene;

class Modal : public Box
{
public:
	Modal(Scene *scene, double width, double height);
	
	~Modal();

	virtual void render(SnowGL *gl);

	void hide();
	void setCentre(Renderable *r, double fx, double fy);
	void setLeft(Renderable *r, double fx, double fy);
	void setRight(Renderable *r, double fx, double fy);
	
	void setDismissible(double dismiss)
	{
		_dismissable = dismiss;
	}

	void dismiss();
protected:
	Scene *_scene;
private:
	void conv_coords(double *fx, double *fy);

	Box *_darker;

	double _width;
	double _height;
	
	bool _dismissable;
};

#endif

