// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Modal__
#define __practical__Modal__

#include "Box.h"
#include <vector>
#include <string>
#include "ButtonResponder.h"

class Scene;
class ImageButton;

class Modal : virtual public Box, virtual public ButtonResponder
{
public:
	Modal(Scene *scene, double width, double height);
	Modal(Scene *scene);
	
	~Modal();

	virtual void render(GLView *gl);

	void hide();

	void setDismissible(double dismiss)
	{
		_dismissable = dismiss;
	}

	/** when true, hide() removes only this (topmost) modal instead of the
	 * whole stack underneath it - for a modal meant to be shown stacked on
	 * top of another already-open one (e.g. a help popup over a form).
	 * Off by default, so every existing Modal keeps its current "closing
	 * me closes everything below me too" behaviour. */
	void setStacked(bool stacked)
	{
		_stacked = stacked;
	}

	/** adds a small "?" icon in this modal's top-right corner which pops
	 * up a dismissable InfoModal with this text, stacked on top so it
	 * doesn't close this modal underneath it. Works for any Modal
	 * subclass - position is derived from this modal's own width/height,
	 * not hardcoded to one particular dialog's size. */
	void setHelpText(const std::string &text);

	virtual void buttonPressed(std::string, Button *button) {}
	void addTwoButtons(std::string left, std::string ltag,
	                   std::string right, std::string rtag);

	/* called just before display if you need to rearrange things */
	virtual void refresh() {};
	void dismiss();
	
	void squeezeToWidth(Renderable *r);
protected:
	void makeFreshBox(double width, double height);
	Scene *_scene;
private:
	void conv_coords(double *fx, double *fy);

	Box *_darker = nullptr;

	double _width = 0;
	double _height = 0;
	
	bool _dismissable = true;
	bool _stacked = false;

	std::string _helpText{};
	ImageButton *_help = nullptr;
};

#endif

