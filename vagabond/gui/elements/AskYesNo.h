// Copyright (C) 2021 Helen Ginn

#ifndef __practical__AskYesNo__
#define __practical__AskYesNo__

#include "Modal.h"
#include "ButtonResponder.h"

class AskYesNo : public Modal, public ButtonResponder
{
public:
	AskYesNo(Scene *scene, std::string text, std::string tag, 
	         ButtonResponder *sender);
	
	~AskYesNo();

	virtual void buttonPressed(std::string tag, Button *button);
private:
	ButtonResponder *_sender;
	std::string _tag;

};

#endif


