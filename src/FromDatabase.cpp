// Copyright (C) 2021 Helen Ginn

#include "FromDatabase.h"
#include "FileReader.h"

FromDatabase::FromDatabase(std::string load)
{

}

void FromDatabase::populateFromString(std::string str)
{
	std::vector<std::string> properties = split(str, '&');

	for (size_t i = 0; i < properties.size(); i++)
	{
		std::vector<std::string> bits = split(properties[i], '=');
		
		if (bits.size() < 2)
		{
			continue;
		}
		
		setValue(bits[0], bits[1]);
	}
}

