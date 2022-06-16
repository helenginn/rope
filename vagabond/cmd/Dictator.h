// breathalyser
// Copyright (C) 2019 Helen Ginn
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

#ifndef __breathalyser__dictator__
#define __breathalyser__dictator__

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <map>

class CmdWorker;

class Dictator
{
public:
	Dictator();

	void setup();
	
	void setArgs(std::vector<std::string> args)
	{
		_args = args;
	}
	
	void addArg(std::string arg)
	{
		_args.push_back(arg);

		std::cout << "Added an argument, notifying" << std::endl;
		_cv.notify_one();
	}
	
	static std::string valueForKey(std::string key)
	{
		return _properties[key];
	}
	
	static void setValueForKey(std::string key, std::string value)
	{
		_properties[key] = value;
	}

	void start();
	bool nextJob();
	
	void workerLock()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_cv.wait(lock);
	}

	void finish();
protected:
	bool checkForFile(std::string &first, std::string &last);
	bool sanityCheck(std::string &first, std::string &last);
	void processRequest(std::string &first, std::string &last);
	void loadFiles(std::string &last);

	static std::map<std::string, std::string> _properties;
	static std::map<std::string, std::string> _commands;
private:
	void processNextArg(std::string arg);
	void makeCommands();

	std::condition_variable _cv;
	std::mutex _mutex;
	std::thread *_thread;
	CmdWorker *_worker;
	std::vector<std::string> _args;
	int _currentJob;
	bool _loadMode;
};

#endif
