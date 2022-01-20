// Copyright (C) 2021 Helen Ginn

#include "Window.h"
#include "Modal.h"
#include "Scene.h"
#include "Image.h"
#include <GLES3/gl3.h>
#include <iostream>
#include <SDL2/SDL.h>
#include <time.h>

#include <cstring>

Scene::Scene() : SnowGL()
{
//	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	_background = NULL;
	_modal = NULL;
	_mouseDown = false;
	_dragged = NULL;
	_removeModal = NULL;
	
	setBackground();
}

void Scene::setBackground()
{
	_proj = glm::mat4(1.);
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

void Scene::setLeft(Renderable *r, double x, double y)
{
	double xf = 2 * x - 1;
	double yf = 2 * y - 1;
	
	xf += r->maximalWidth() / 2;

	r->setPosition(glm::vec3(xf, -yf, 0));
}

void Scene::setRight(Renderable *r, double x, double y)
{
	double xf = 2 * x - 1;
	double yf = 2 * y - 1;
	
	xf -= r->maximalWidth() / 2;

	r->setPosition(glm::vec3(xf, -yf, 0));
}

void Scene::setCentre(Renderable *r, double x, double y)
{
	double xf = 2 * x - 1;
	double yf = 2 * y - 1;

	r->setPosition(glm::vec3(xf, -yf, 0));
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

void Scene::mouseReleaseEvent(double x, double y)
{
	_mouseDown = false;
	_dragged = NULL;
}

void Scene::mousePressEvent(double x, double y)
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
