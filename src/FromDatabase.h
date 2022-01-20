// Copyright (C) 2021 Helen Ginn

#ifndef __tutorial__FromDatabase__
#define __tutorial__FromDatabase__

#include <string>

class FromDatabase
{
public:
	FromDatabase(std::string load);
	virtual ~FromDatabase() {};

	void populateFromString(std::string str);
	
	virtual void setValue(std::string key, std::string value) {};
protected:

};

#endif

