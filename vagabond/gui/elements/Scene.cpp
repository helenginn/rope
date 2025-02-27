// Copyright (C) 2021 Helen Ginn

#include "Window.h"
#include "Modal.h"
#include "IndexResponder.h"
#include "TextButton.h"
#include "Scene.h"
#include "AskYesNo.h"
#include "Image.h"
#include <vagabond/utils/gl_import.h>
#include <iostream>
#include <SDL2/SDL.h>
#include <time.h>

#include <cstring>

std::string Scene::_defaultBg = "assets/images/paper.jpg";

Scene::Scene(Scene *prev) : SnowGL()
{
	_mouseDown = false;
	_previous = prev;

	setBackground();
}

Scene::~Scene()
{
	
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
	_viewChanged = true;
}

void Scene::reloadBackground()
{
	removeObject(_background);
	Window::setDelete(_background);
	_background = nullptr;

	Image *r = new Image(_defaultBg);
	r->Box::makeQuad();
	addObjectToFront(r);
	_background = r;

	if (_previous != nullptr)
	{
		_previous->reloadBackground();
	}
}

void Scene::setBackground()
{
	Image *r = new Image(_defaultBg);
	r->Box::makeQuad();
	addObject(r);
	_background = r;
}

void Scene::setModal(Modal *modal)
{
	if (!_modal)
	{
		_modal = modal;
		_left = false;
		_right = false;
	}
}

void Scene::removeModal()
{
	_removeModal = _modal;
	_modal = nullptr;
	resetMouseKeyboard();
	_mouseDown = false;
	_moving = false;
	_left = false;
}

void Scene::doThings()
{
	if (_mustRefresh)
	{
		refresh();
		_mustRefresh = false;
		viewChanged();
	}
}

void Scene::render()
{
	SnowGL::render();

	if (_modal != nullptr)
	{
		_modal->render(this);
	}
	
	if (_removeModal != nullptr)
	{
		delete _removeModal;
		_removeModal = nullptr;
	}
}

void Scene::setCentrePixels(Renderable *r, int x, int y)
{
	double xf = 2 * (double)x / width() - 1;
	double yf = 2 * (double)y / height() - 1;

	r->setPosition(glm::vec3(xf, -yf, 0));
}

void Scene::convertToPixels(float *x, float *y)
{
	*x = (*x + 1) / 2 * width();
	*y = (1 - *y) / 2 * height();
}

void Scene::convertToGLCoords(float *x, float *y)
{
	*x = 2 * (float)*x / width() - 1;
	*y = 1 - 2 * (float)*y / height();
}

void Scene::convertToGLCoords(double *x, double *y)
{
	*x = 2 * (double)*x / width() - 1;
	*y = 1 - 2 * (double)*y / height();
}

std::vector<Renderable *> &Scene::pertinentObjects()
{
	if (_modal != nullptr)
	{
		return _modal->objects();
	}

	return objects();
}

void Scene::mouseReleaseEvent(double x, double y, SDL_MouseButtonEvent button)
{
	if (_expired) return;
	
	if (_dragged && _moving)
	{
		_dragged->undrag();
	}
	else if (_dragged)
	{
		_dragged->click(button.button == SDL_BUTTON_LEFT);
	}

	_dragged = nullptr;
	convertToGLCoords(&x, &y);

	if (_modal != nullptr && _chosen == nullptr)
	{
		double z = -FLT_MAX;
		bool hit = _modal->intersectsRay(x, y, &z);

		if (!hit && _left)
		{
			_modal->dismiss();
		}
	}

	_left = button.button == SDL_BUTTON_LEFT;

	if (hasIndexedObjects() > 0 && _modal == nullptr && _chosen == nullptr
	    && !_moving)
	{
		checkIndexBuffer(x, y, false, true, _left);
	}

	_mouseDown = false;
	_moving = false;
}

void Scene::mousePressEvent(double x, double y, SDL_MouseButtonEvent button)
{
	if (_expired) return;

	_lastX = x;
	_lastY = y;
	convertToGLCoords(&x, &y);
	_moving = false;
	_mouseDown = true;

	Renderable *chosen = findObject(x, y);
	_left = (button.button == SDL_BUTTON_LEFT);
	_chosen = chosen;

	if (chosen != nullptr)
	{
		if (chosen->isDraggable())
		{
			std::cout  << "Draggable: " << chosen->name() << std::endl;
			_dragged = chosen;
		}
		else
		{
			// warning: clicking the chosen object may result in its destruction
			chosen->click(_left);
			// don't use the pointer afterwards
		}

	}
}

void Scene::mouseMoveEvent(double x, double y)
{
	if (_expired) return;

	double tx = x;
	double ty = y;
	_moveX = tx;
	_moveY = ty;
	convertToGLCoords(&tx, &ty);
	_moving = true;

	clearHighlights();
	Renderable *chosen = findObject(tx, ty);

	SDL_Cursor *cursor = nullptr;
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

	if (_mouseDown && _dragged != nullptr && _dragged->isDraggable())
	{
		_dragged->drag(tx, ty);
	}
	
	swapCursor(cursor);
	
	if (hasIndexedObjects() > 0 && _modal == nullptr)
	{
		checkIndexBuffer(tx, ty, true, arrow, true);
	}
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
	if (_previous == nullptr)
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
	if (_titleText != nullptr)
	{
		removeObject(_titleText);
		delete _titleText;
		_titleText = nullptr;
	}

	Text *text = new Text(title, Font::Thick);
	text->setCentre(0.5, 0.1);
	addObject(text);
	_titleText = text;
}

void Scene::askToQuit()
{
	if (!_modal)
	{
		AskYesNo *ayn = new AskYesNo(this, "Are you sure you want to quit?",
				"quit", this);
		ayn->setDismissible(true);
		setModal(ayn);
		viewChanged();
	}
	else
	{
		_modal->dismiss();
		viewChanged();
	}
}

void Scene::buttonPressed(std::string tag, Button *button)
{
	if (tag == "back")
	{
		_previous->showSimple();
		Window::setDelete(this);
	}
	else if (tag == "yes_quit")
	{
		exit(0);
	}
	if (tag == "remove_info" && _info != nullptr)
	{
		removeObject(_info);
		delete _info;
		_info = nullptr;
	}
}

void Scene::back(int num)
{
	_previous->showSimple();
	Window::setDelete(this);
	
	/* recursively remove scenes */
	if (num > 0)
	{
		_previous->back(num - 1);

		if (num == 1)
		{
			/* might be called anyway */
			_previous->refresh();
		}
	}
	
	_expired = true;
}

void Scene::interpretControlKey(SDL_Keycode pressed, bool dir)
{
	if (pressed == SDLK_LCTRL)
	{
		_controlPressed = dir;
	}

	if (pressed == SDLK_LGUI || pressed == SDLK_LALT)
	{
		_altPressed = dir;
	}

	if (pressed == SDLK_LSHIFT)
	{
		_shiftPressed = dir;
	}
}

void Scene::keyPressEvent(SDL_Keycode pressed)
{
	interpretControlKey(pressed, true);

	if (!keyResponder())
	{
		if (_modal)
		{
			_modal->doAccessibilityThings(pressed, _shiftPressed);
		}
		else
		{
			doAccessibilityThings(pressed, _shiftPressed);
		}
		return;
	}

	keyResponder()->keyPressed(pressed);
}


void Scene::setInformation(std::string str)
{
	if (_info != nullptr)
	{
		removeObject(_info);
		Window::setDelete(_info);
		_info = nullptr;
	}

	if (str.length())
	{
		_info = new TextButton(str, this, true);
		_info->setReturnTag("remove_info");
		_info->resize(0.6);
		_info->setCentre(0.5, 0.16);
		addObject(_info);
	}

}

void Scene::keyReleaseEvent(SDL_Keycode pressed)
{
	interpretControlKey(pressed, false);
}
