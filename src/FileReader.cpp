//
//  FileReader.cpp
//  GameDriver
//
//  Created by Helen Ginn on 21/05/2014.
//  Copyright (c) 2014 Helen Ginn. All rights reserved.
//

#include "FileReader.h"
#include <fstream>
#include <sstream>
#include <cerrno>
#include <cmath>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <iomanip>
#include <algorithm>

std::string FileReader::outputDir;

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) 
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

bool file_exists(const std::string& name)
{
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

std::string get_file_contents(std::string filename)
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);

	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize((unsigned long)in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}


	std::string errString = "Could not get file contents for file " + std::string(filename);
	std::cout << errString << std::endl;

	throw(errno);
}

std::string defenestrate(std::string str)
{
	std::ostringstream ss;
	
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] != '\r')
		{
			ss << str[i];
		}
	}

	return ss.str();
}

std::string getPath(std::string whole)
{
	size_t pos = whole.rfind("/");
	if(pos == std::string::npos)  //No path.
	{
		return "./";
	}
	else
	{
		return whole.substr(0, pos + 1);
	}
}

std::string getFilename(std::string filename)
{
	size_t pos = filename.rfind("/");
	if(pos == std::string::npos)  //No path.
	return filename;

	return filename.substr(pos + 1, filename.length());
}

std::string getBaseFilenameWithPath(std::string filename)
{
	std::string fName = filename;
	size_t pos = fName.rfind(".");
	if(pos == std::string::npos)  //No extension.
	return fName;

	if(pos == 0)    //. is at the front. Not an extension.
	return fName;

	return fName.substr(0, pos);
}

std::string getExtension(std::string filename)
{
	std::string fName = getFilename(filename);
	size_t pos = fName.rfind(".");
	if (pos == std::string::npos || pos == fName.length() - 1)  //No extension
	{
		return "";
	}

	return fName.substr(pos + 1);
}

std::string getBaseFilename(std::string filename)
{
	std::string fName = getFilename(filename);
	size_t pos = fName.rfind(".");
	if(pos == std::string::npos)  //No extension.
	return fName;

	if(pos == 0)    //. is at the front. Not an extension.
	return fName;

	return fName.substr(0, pos);
}

std::string findNextFilename(std::string file)
{
	std::string path = getPath(file);
	std::string trial = getFilename(file);
	int count = 0;

	while (true)
	{
		std::string test = path + "/" + i_to_str(count) + "_" + trial;

		if (!file_exists(test))
		{
			return test;
		}
		
		count++;
	}
}

void trim(std::string &str)
{
	std::string::size_type pos = str.find_last_not_of(' ');
	if(pos != std::string::npos)
	{
		str.erase(pos + 1);
		pos = str.find_first_not_of(' ');
		if(pos != std::string::npos) str.erase(0, pos);
	}
	else str.erase(str.begin(), str.end());
}

void print_cc_diff(std::ostream *stream, double diff, int limit)
{
	if (diff != diff)
	{
		diff = 0;
	}

	int signs = fabs(diff * 10);
	int dir = (diff < 0);	
	
	if (signs > 1000)
	{
		signs = 1000;
	}

	if (signs > limit && limit > 0)
	{
		signs = limit;
	}

	*stream << " ";
	for (int j = 0; j < signs; j++)
	{
		*stream << (dir ? "+" : "-");
	}

	if (limit > 0)
	{
		for (int j = signs; j < 20; j++)
		{
			*stream << " ";	
		}
	}
}

void to_lower(std::string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

void to_upper(std::string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

std::string i_to_str(int val)
{
	std::ostringstream ss;
	ss << val;
	std::string temp = ss.str();

	return temp;
}

std::string f_to_str(double val, int precision)
{
	std::ostringstream ss;
	if (precision > 0)
	{
		ss << std::fixed << std::setprecision(precision);
	}
	else if (precision < 0)
	{
		ss << std::fixed;
	}

	ss << val;
	std::string temp = ss.str();

	return temp;
}

std::string findNewFolder(std::string prefix)
{
	int count = 1;
	
	while (1)
	{
		std::string test = prefix + i_to_str(count);

		if (!file_exists(test))
		{
			return test;
		}
		
		count++;
	}
}
