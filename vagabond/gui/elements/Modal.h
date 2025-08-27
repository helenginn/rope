// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Modal__
#define __practical__Modal__

#include "Box.h"
#include <vector>
#include "ButtonResponder.h"

class Scene;

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
};

#endif

