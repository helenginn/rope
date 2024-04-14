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

#include "CheckList.h"
#include "../elements/Window.h"
#include "../elements/Scene.h"
#include <string>
#include <vagabond/utils/FileReader.h>
#include <SDL2/SDL.h>

CheckList::CheckList(const std::string &filename)
{
	if (filename.length())
	{
		try
		{
			std::string contents = get_file_contents(filename);
			_progress.commands = split(contents, '\n');
			std::cout << "Total commands: " << _progress.total() << std::endl;
		}
		catch (std::runtime_error &err)
		{
			std::cout << err.what() << std::endl;
			exit(0);
		}
	}
	else
	{
		_done = true;
	}
}

void CheckList::pushKey(const std::vector<std::string> &bits)
{
	if (bits.size() < 3) return;
	
	bool down = (bits[1] == "down");
	bool up = (bits[1] == "up");
	bool move = (bits[1] == "move");
	
	SDL_Event event;
	if (down) event.type = SDL_KEYDOWN;
	if (up) event.type = SDL_KEYUP;
	
	int key = atoi(bits[2].c_str());
	event.key.keysym.sym = SDL_Keycode(key);

	SDL_PushEvent(&event);
}

void CheckList::pushClick(const std::vector<std::string> &bits)
{
	if (bits.size() < 5) return;
	
	bool down = (bits[1] == "down");
	bool up = (bits[1] == "up");
	bool move = (bits[1] == "move");
	bool left = (bits[2] == "left");
	bool right = (bits[2] == "right");

	float x = atof(bits[3].c_str());
	float y = atof(bits[4].c_str());
	Window::currentScene()->convertToPixels(&x, &y);
	std::cout << "Pushing event with: " << x << " " << y << std::endl;
	
	SDL_Event event;
	if (down) event.type = SDL_MOUSEBUTTONDOWN;
	if (up) event.type = SDL_MOUSEBUTTONUP;
	if (move) event.type = SDL_MOUSEMOTION;
	if (left) event.button.button = SDL_BUTTON_LEFT;
	if (right) event.button.button = SDL_BUTTON_RIGHT;
	event.motion.x = x;
	event.motion.y = y;

	SDL_PushEvent(&event);
}

void CheckList::processCommand(const std::string &line)
{
	std::cout << "EXEC: " << line << std::endl;
	std::vector<std::string> bits = split(line, ' ');
	
	if (bits[0] == "wait")
	{
		int wait = _waitCount;
		if (bits.size() > 1)
		{
			wait = atoi(bits[1].c_str());
		}
		_progress.countdown += wait;
	}

	if (bits[0] == "click")
	{
		pushClick(bits);
	}
	else if (bits[0] == "key")
	{
		pushKey(bits);
	}
}

bool CheckList::next()
{
	if (_done)
	{
		return false;
	}

	if (_progress.countdown > 0)
	{
		_progress.countdown--;
		return false;
	}

	_progress.number++;
	if (_progress.number >= _progress.total())
	{
		_done = true;
		std::cout << "---- done" << std::endl;
		return true;
	}
	
	processCommand(_progress.commands[_progress.number]);
	return false;
}
