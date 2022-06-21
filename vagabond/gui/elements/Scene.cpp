// Copyright (C) 2021 Helen Ginn

#include "Window.h"
#include "Modal.h"
#include "IndexResponder.h"
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
	_background = NULL;
	_modal = NULL;
	_mouseDown = false;
	_dragged = NULL;
	_removeModal = NULL;
	
	_back = NULL;
	_previous = prev;

	setBackground();
}

void Scene::preSetup()
{
	showBackButton();
	
	if (_title.length())
	{
		addTitle(_title);
		_title = "";
	}
	
	setup();
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
	resetMouseKeyboard();
}

void Scene::render()
{
	doThings();

	if (_mustRefresh)
	{
		refresh();
		_mustRefresh = false;
	}

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
	convertToGLCoords(&x, &y);

	if (hasIndexedObjects() > 0)
	{
		bool left = button.button == SDL_BUTTON_LEFT;
		checkIndexBuffer(x, y, false, true, left);
	}
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
		bool hit = _modal->intersectsRay(x, y, &z);

		if (!hit)
		{
			_modal->dismiss();
		}
	}
	
	_mouseDown = true;
}

void Scene::swapCursor(SDL_Cursor *newCursor)
{
	SDL_SetCursor(newCursor);

	if (_cursor != nullptr)
	{
		SDL_FreeCursor(_cursor);
		_cursor = nullptr;
	}

	_cursor = newCursor;
}

void Scene::mouseMoveEvent(double x, double y)
{
	convertToGLCoords(&x, &y);

	clearHighlights();
	Renderable *chosen = findObject(x, y);

	SDL_Cursor *cursor;
	bool arrow = true;
	if (chosen != nullptr && chosen->mouseOver())
	{
		cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
		arrow = false;
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
	
	swapCursor(cursor);
	
	if (hasIndexedObjects() > 0 && _modal == nullptr)
	{
		checkIndexBuffer(x, y, true, arrow, true);
	}
}

void Scene::showSimple()
{
	Window::reloadScene(this);
}

void Scene::queueToShow()
{
	Window::setNextScene(this);
}

void Scene::show()
{
	Window::setCurrentScene(this);
}

void Scene::hideBackButton()
{
	if (_back)
	{
		_back->setDisabled(true);
	}
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
	Text *text = new Text(title, Font::Thick);
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

