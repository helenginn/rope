// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Scene__
#define __practical__Scene__

#include "SnowGL.h"
#include "SDL2/SDL.h"
#include "ButtonResponder.h"

class Renderable;
class Button;
class Modal;

class Scene : public SnowGL, public ButtonResponder
{
public:
	Scene(Scene *prev = nullptr);
	
	void setModal(Modal *modal)
	{
		_modal = modal;
	}
	
	void setBackScene(Scene *prev)
	{
		_previous = prev;
	}
	
	void removeModal();

	void preSetup();
	virtual void setup() = 0;
	virtual void refresh() {};
	void setBackground();
	void addTitle(std::string);
	void setFutureTitle(std::string title)
	{
		_title = title;
	}

	void show();
	void queueToShow();
	void showBackButton();
	virtual void showSimple();
	virtual void render();

	virtual void mouseMoveEvent(double x, double y);
	virtual void mousePressEvent(double x, double y, SDL_MouseButtonEvent button);
	virtual void mouseReleaseEvent(double x, double y, SDL_MouseButtonEvent button);

	virtual void keyReleaseEvent(SDL_Keycode pressed) {};
	virtual void keyPressEvent(SDL_Keycode pressed);

	virtual void back(int num = 0);
	void buttonPressed(std::string tag, Button *button);
protected:
	virtual std::vector<Renderable *> &pertinentObjects();
	void convertToGLCoords(double *x, double *y);
	void setCentrePixels(Renderable *r, int x, int y);

	static char *dataToChar(void *data, int nbytes);
	
	std::string _title;

	Renderable *_dragged;

	Renderable *_background;
	Modal *_modal;
	Modal *_removeModal;
	Scene *_previous;
	Button *_back;

	bool _mouseDown;
};

#endif
