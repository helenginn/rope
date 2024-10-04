// Copyright (C) 2021 Helen Ginn

#ifndef __practical__AskToSave__
#define __practical__AskToSave__

#include "Modal.h"
#include "ButtonResponder.h"


class AskToSave : public Modal
{
public:
	AskToSave(Scene *scene, std::string text, std::string tag = "", 
	         ButtonResponder *sender = nullptr);
	
	~AskToSave();
	
	void addJob(const std::string &answer,
	            const std::function<void()> &job)
	{
		_jobs[answer] = job;
	}

	virtual void buttonPressed(std::string tag, Button *button);
private:
	ButtonResponder *_sender;
	std::string _tag;

	std::map<std::string, std::function<void()>> _jobs;
};

#endif


