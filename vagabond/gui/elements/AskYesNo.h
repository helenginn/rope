// Copyright (C) 2021 Helen Ginn

#ifndef __practical__AskYesNo__
#define __practical__AskYesNo__

#include "Modal.h"
#include "ButtonResponder.h"

/** AskYesNo: simple yes/no dialogue with a question and a tag {tag} for
 * later identification.
 * the responder will be returned a tag: "yes_{tag}" or "no_{tag}" depending
 * on the user's answer */

class AskYesNo : public Modal
{
public:
	AskYesNo(Scene *scene, std::string text, std::string tag = "", 
	         ButtonResponder *sender = nullptr);
	
	~AskYesNo();
	
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


