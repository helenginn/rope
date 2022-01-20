// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Scene__
#define __practical__Scene__

#include "SnowGL.h"

class Renderable;
class Modal;

class Scene : public SnowGL
{
public:
	Scene();
	
	void setModal(Modal *modal)
	{
		_modal = modal;
	}
	
	void removeModal();

	virtual void setup() = 0;
	virtual void refresh() {};
	void setBackground();
	void show();
	virtual void showSimple();
	virtual void render();

	void mouseMoveEvent(double x, double y);
	void mousePressEvent(double x, double y);
	void mouseReleaseEvent(double x, double y);

	void setCentre(Renderable *r, double x, double y);
	void setLeft(Renderable *r, double x, double y);
	void setRight(Renderable *r, double x, double y);

	virtual void back(int num) {};
protected:
	virtual std::vector<Renderable *> &pertinentObjects();
	void convertToGLCoords(double *x, double *y);
	void setCentrePixels(Renderable *r, int x, int y);

	static char *dataToChar(void *data, int nbytes);

	Renderable *_dragged;

	Renderable *_background;
	Modal *_modal;
	Modal *_removeModal;
	bool _mouseDown;
};

#endif
