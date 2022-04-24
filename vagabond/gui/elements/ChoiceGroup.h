// Copyright (C) 2021 Helen Ginn


#ifndef __ChoiceGroup__ChoiceGroup__
#define __ChoiceGroup__ChoiceGroup__

#include "Renderable.h"
#include "ButtonResponder.h"

class Choice;
class ChoiceImage;
class ChoiceText;
class Scene;

class ChoiceGroup : public Renderable, public ButtonResponder
{
public:
	ChoiceGroup(Scene *scene);
	
	ChoiceImage *addImage(std::string filename, std::string tag);
	void takeChoice(Choice *choice);
	ChoiceText *addText(std::string text, std::string tag = "");

	void setInert(bool inert);

	void arrange(double resize, double ctx, double cty,
	             double xspan, double yspan);
	
	std::string tag()
	{
		return _tag;
	}

	size_t choiceCount()
	{
		return _choices.size();
	}
	
	Choice *choice(int i)
	{
		return _choices[i];
	}
	
	void addChoice(Choice *choice);
	
	virtual void buttonPressed(std::string tag, Button *button = NULL);
private:
	std::vector<Choice *> _choices;
	Scene *_scene;

	std::string _tag;
};

#endif

