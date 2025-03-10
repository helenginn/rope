// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Scene__
#define __practical__Scene__

#include <functional>
#include "SnowGL.h"
#include "SDL2/SDL.h"
#include "ButtonResponder.h"

class IndexResponder;
class TextButton;
class Renderable;
class Button;
class Modal;
class Text;

class Scene : public SnowGL, public ButtonResponder
{
public:
	Scene(Scene *prev = nullptr);
	virtual ~Scene();
	
	
	void setBackScene(Scene *prev)
	{
		_previous = prev;
	}
	
	Scene *const previous() const
	{
		return _previous;
	}
	
	void setModal(Modal *modal);
	void removeModal();

	void preSetup();
	virtual void setup() = 0;
	virtual void refresh() {};
	void refreshNextRender()
	{
		_mustRefresh = true;
	}

	void askToQuit();
	void setBackground();
	void addTitle(std::string);
	void setFutureTitle(std::string title)
	{
		_title = title;
	}

	void show();
	void queueToShow();
	void showBackButton();
	void hideBackButton();
	virtual void showSimple();
	virtual void render();
	virtual void doThings();

	virtual void mouseMoveEvent(double x, double y);
	virtual void mousePressEvent(double x, double y, SDL_MouseButtonEvent button);
	virtual void mouseReleaseEvent(double x, double y, SDL_MouseButtonEvent button);

	virtual void keyReleaseEvent(SDL_Keycode pressed);
	virtual void keyPressEvent(SDL_Keycode pressed);

	virtual void back(int num = 0);
	void setBackJob(const std::function<void()> &backJob)
	{
		_backJob = backJob;
	}
	void buttonPressed(std::string tag, Button *button);
	
	virtual const bool hasIndexedObjects() const
	{
		return false;
	}
	
	void setInformation(std::string str);

	void convertToPixels(float *x, float *y);
	void convertToGLCoords(float *x, float *y);
	
	const bool &mouseDown() const
	{
		return _mouseDown;
	}
protected:
	void reloadBackground();
	virtual std::vector<Renderable *> &pertinentObjects();
	void convertToGLCoords(double *x, double *y);
	virtual void checkIndexBuffer(double x, double y, 
	                              bool hover, bool arrow, bool left) {};
	void setCentrePixels(Renderable *r, int x, int y);
	void swapCursor(SDL_Cursor *newCursor);
	void interpretControlKey(SDL_Keycode pressed, bool dir);
	
	std::string _title;

	Renderable *_background = nullptr;
	Modal *_modal = nullptr;
	Modal *_removeModal = nullptr;
	Scene *_previous = nullptr;
	Button *_back = nullptr;
	TextButton *_info = nullptr;
	Text *_titleText = nullptr;
	
	std::function<void()> _backJob{};
	
	SDL_Cursor *_cursor = nullptr;

	int _lastIdx;

	bool _moving = false;
	bool _expired = false;
	bool _mouseDown = false;
	bool _mustRefresh = false;
	static std::string _defaultBg;
};

#endif
