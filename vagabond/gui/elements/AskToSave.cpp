// Copyright (C) 2021 Helen Ginn

#include "AskToSave.h"
#include "TextButton.h"
#include "Modal.h"
#include <vagabond/core/Environment.h>

AskToSave::AskToSave(Scene *scene, std::string text, std::string tag,
                   ButtonResponder *sender) : 
Modal(scene, 0.6, 0.4)
{
	Text *t = new Text(text);
	t->setCentre(0.5, 0.45);
	addObject(t);

	squeezeToWidth(t);

	addThreeButtons("Cancel", "cancel", "No", "no", "Yes", "yes");
	
	_sender = sender;
	_tag = tag;

	setDismissible(false);
}

void AskToSave::buttonPressed(std::string tag, Button *button)
{
	// if (tag == "do not save")
	// {
	// 	exit(0);
	// }
	if (_jobs.count(tag))
	{
		hide();
		_jobs[tag]();
		exit(0); 
	}
	else
	{
		hide();
		if (_sender)
		{
			std::cout << tag <<std::endl;
			std::cout << _tag <<std::endl;
			std::cout <<tag + "_" + _tag<<std::endl;
			std::string tagCompine = tag + "_" + _tag;
			_sender->buttonPressed(tag + "_" + _tag);   
			if (tagCompine == "no_savequit")
			{
				exit(0);
			}   
			else if (tagCompine == "yes_savequit")
			{
				Environment::environment().save();
				exit(0);
			}
			else if (tagCompine == "cancel_savequit");
			{
				setDismissible(true);
			}

		}

	}

}
// void AstToSave::EnableSaving()
// {

// }

AskToSave::~AskToSave()
{

}
