// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Modal__
#define __practical__Modal__

#include "Box.h"
#include <vector>

class Scene;

class Modal : virtual public Box
{
public:
	Modal(Scene *scene, double width, double height);
	Modal(Scene *scene);
	
	~Modal();

	virtual void render(SnowGL *gl);

	void hide();
	void setModalCentre(Renderable *r, double fx, double fy);
//	void setLeft(Renderable *r, double fx, double fy);
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

	Box *_darker = nullptr;

	double _width = 0;
	double _height = 0;
	
	bool _dismissable = true;
};

#endif

