// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __vagabond__VagWindow__
#define __vagabond__VagWindow__

#include <vagabond/gui/elements/Window.h>
#include <vagabond/core/Responder.h>
#include <vagabond/core/Progressor.h>
#include <atomic>
#include <functional>
#include "MainMenu.h"

class Dictator;
class ProgressBar;

class VagWindow : public Window, public Responder<Progressor>
//, public ProgressViewResponder
{
public:
	VagWindow();
	virtual void setup(int argc, char **argv);
	void setup_special();
	virtual void mainThreadActivities();

	static VagWindow *window()
	{
		return static_cast<VagWindow *>(_myWindow);
	}

	static Dictator *dictator()
	{
		return _dictator;
	}

	static void addJob(std::string str);

	void prepareProgressView();

	bool hasProgressBar()
	{
		return (_bar.ptr != nullptr);
	}

	virtual void extraWindowFlags(unsigned int &flags)
	{
		flags += SDL_WINDOW_RESIZABLE;
	}

	// caller and cancelJob are both optional, purely additive - existing
	// callers that only pass ticks/text keep working identically. When
	// caller is supplied, this registers directly on it (bypassing
	// Environment's progressResponder() entirely - see sendObject()) so
	// the bar can never miss a tick/done that arrives before it exists.
	// When cancelJob is supplied, an "x" button appears next to the bar;
	// clicking it only invokes cancelJob - aborting whatever the tracked
	// job is actually doing is the caller's own responsibility.
	void requestProgressBar(int ticks, std::string text,
	                        Progressor *caller = nullptr,
	                        const std::function<void()> &cancelJob = nullptr);
	void requestProgressBarRemoval();

	// Responder<Progressor> - see sendObject() for why this exists
	// alongside Environment's progressResponder() rather than replacing it.
	virtual void sendObject(std::string tag, void *object);
private:
	void prepareProgressBar(int ticks, std::string text);
	void removeProgressBar();
	// sender is the raw Progressor* pointer VALUE that sent this event
	// (see sendObject()'s comment - never dereferenced, may already be
	// deleted) - compared against _bar.caller so a straggler tick/done
	// from an already-superseded request can never be misapplied to
	// (or buffered for) a different, unrelated request's bar. See
	// sendObject() for why this matters: a search's finishTicker() can
	// arrive after its own bar already finished naturally via ticks
	// alone, and that "done" has nothing to do with whichever request
	// creates the next bar.
	void handleProgressEvent(std::string tag, void *sender);

	static Dictator *_dictator;
	MainMenu *_menu = nullptr;
	struct BarDetails
	{
		int ticks = 0;
		std::string text = "";
		ProgressBar *ptr = nullptr;
		Progressor *caller = nullptr;
		std::function<void()> cancelJob;

		// set by handleProgressEvent() when a tick/done arrives before
		// ptr exists yet, from whichever Progressor caller currently is
		// - prepareProgressBar() applies these the moment it creates the
		// bar, instead of the event being lost. Only ever set for events
		// matching the current caller (see handleProgressEvent()), so a
		// straggler from an already-superseded request can never end up
		// buffered here for an unrelated later request to inherit.
		int pendingTicks = 0;
		bool pendingDone = false;
	};

	BarDetails _bar;

};

#endif
