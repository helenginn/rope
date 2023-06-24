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

	virtual void render(SnowGL *gl);

	void hide();
	
	void setDismissible(double dismiss)
	{
		_dismissable = dismiss;
	}
	
	void addTwoButtons(std::string left, std::string ltag,
	                   std::string right, std::string rtag);

	void dismiss();
	
	void squeezeToWidth(Renderable *r);
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

