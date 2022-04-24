// Copyright (C) 2021 Helen Ginn

#include "Window.h"
#include "Modal.h"
#include "TextButton.h"
#include "Scene.h"
#include "Image.h"
#include <GLES3/gl3.h>
#include <iostream>
#include <SDL2/SDL.h>
#include <time.h>

#include <cstring>

Scene::Scene(Scene *prev) : SnowGL()
{
	if (Window::hasContext())
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	_background = NULL;
	_modal = NULL;
	_mouseDown = false;
	_dragged = NULL;
	_removeModal = NULL;
	
	_back = NULL;
	_previous = prev;

	setBackground();
	showBackButton();
}

void Scene::setBackground()
{
	Image *r = new Image("assets/images/paper.jpg");
	r->Box::makeQuad();
	addObject(r);
	_background = r;
}

void Scene::removeModal()
{
	_removeModal = _modal;
	_modal = NULL;
}

void Scene::render()
{
	SnowGL::render();

	if (_modal != NULL)
	{
		_modal->render(this);
	}
	
	if (_removeModal != NULL)
	{
		delete _removeModal;
		_removeModal = NULL;
	}
}

void Scene::setCentrePixels(Renderable *r, int x, int y)
{
	double xf = 2 * (double)x / width() - 1;
	double yf = 2 * (double)y / height() - 1;

	r->setPosition(glm::vec3(xf, -yf, 0));
}

void Scene::convertToGLCoords(double *x, double *y)
{
	*x = 2 * (double)*x / width() - 1;
	*y = 1 - 2 * (double)*y / height();
}

std::vector<Renderable *> &Scene::pertinentObjects()
{
	if (_modal != NULL)
	{
		return _modal->objects();
	}

	return objects();
}

void Scene::mouseReleaseEvent(double x, double y, SDL_MouseButtonEvent button)
{
	_mouseDown = false;
	_dragged = NULL;
}

void Scene::mousePressEvent(double x, double y, SDL_MouseButtonEvent button)
{
	convertToGLCoords(&x, &y);
	Renderable *chosen = findObject(x, y);
	if (chosen != NULL)
	{
		chosen->click();
	}
	
	if (_modal != NULL && chosen == NULL)
	{
		double z = -FLT_MAX;
		bool hit = _modal->intersectsPolygon(x, y, &z);

		if (!hit)
		{
			_modal->dismiss();
		}
	}
	
	_mouseDown = true;
}

void Scene::mouseMoveEvent(double x, double y)
{
	convertToGLCoords(&x, &y);

	clearHighlights();
	Renderable *chosen = findObject(x, y);

	SDL_Cursor *cursor; 
	if (chosen != NULL && chosen->mouseOver())
	{
		cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	}
	else
	{
		cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	}
	
	if (_mouseDown && chosen && chosen->isDraggable())
	{
		if (_dragged == NULL)
		{
			_dragged = chosen;
		}
	}

	if (_dragged != NULL)
	{
		_dragged->drag(x, y);
	}

	SDL_SetCursor(cursor);
}


void Scene::showSimple()
{
	Window::reloadScene(this);
}

void Scene::show()
{
	Window::setCurrentScene(this);
}

char *Scene::dataToChar(void *data, int nbytes)
{
	char *tmp = new char[nbytes + 1];
	memcpy(tmp, data, nbytes * sizeof(char));
	tmp[nbytes] = '\0';

	return tmp;
}

void Scene::showBackButton()
{
	if (_previous == NULL)
	{
		return;
	}

	TextButton *button = new TextButton("Back", this);
	button->setReturnTag("back");
	button->resize(0.6);
	button->setLeft(0.02, 0.06);
	addObject(button);
	_back = button;
}

void Scene::addTitle(std::string title)
{
	Text *text = new Text(title);
	text->setCentre(0.5, 0.1);
	addObject(text);
}

void Scene::buttonPressed(std::string tag, Button *button)
{
	if (tag == "back")
	{
		_previous->showSimple();
		Window::setDelete(this);
	}
}

void Scene::back(int num)
{
	_previous->showSimple();
	Window::setDelete(this);
	
	if (num > 0)
	{
		_previous->back(num - 1);

		if (num == 1)
		{
			_previous->refresh();
		}
	}
}

void Scene::keyPressEvent(SDL_Keycode pressed)
{
	if (_keyResponder == nullptr)
	{
		return;
	}

	_keyResponder->keyPressed(pressed);
}
