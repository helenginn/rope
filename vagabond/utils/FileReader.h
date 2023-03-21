//
//  FileReader.h
//  GameDriver
//
//  Created by Helen Ginn on 21/05/2014.
//  Copyright (c) 2014 Helen Ginn. All rights reserved.
//

#ifndef __FileReader__
#define __FileReader__

#include "os.h"
#ifdef OS_UNIX
#include <glob.h> // glob(), globfree()
#else
#ifdef OS_WINDOWS
#include <fileapi.h>
#endif
#endif

#include <stdexcept>
#include <cstring>
#include <sstream>

#include <iostream>
#include <string>
#include <vector>
#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

std::string get_file_contents(std::string filename);

std::vector<std::string> split(const std::string &s, char delim);
bool file_exists(const std::string& name);
bool is_directory(const std::string &name);
void escape_filename(std::string &file);

std::string getPath(std::string whole);
std::string getFilename(std::string filename);

/** removes file extension and path */
std::string getBaseFilename(std::string filename);
std::string getExtension(std::string filename);

/** removes file extension */
std::string getBaseFilenameWithPath(std::string filename);
std::string findNextFilename(std::string file);

inline std::string i_to_str(int val)
{
	return std::to_string(val);
}

std::string f_to_str(double val, int precision);

std::string findNewFolder(std::string prefix = "refine_");

bool is_str_alphabetical(const std::string &str);

/* Random string things */

void trim(std::string& str);
void to_lower(std::string &str);
void to_upper(std::string &str);

/** remove Windows characters */
void defenestrate(std::string &str);
void debom(std::string &str);
void remove_quotes(std::string &str);

inline void pad(std::string &str, char ch, size_t length)
{
	if (str.length() >= length)
	{
		return;
	}

	std::string padding = std::string(length - str.length(), ch);
	str = padding + str;
}


inline void urlencode(std::string &s)
{
    static const char lookup[]= "0123456789abcdef";
    std::stringstream e;
    for(int i=0, ix=s.length(); i<ix; i++)
    {
        const char& c = s[i];
        if ( (48 <= c && c <= 57) ||//0-9
             (65 <= c && c <= 90) ||//abc...xyz
             (97 <= c && c <= 122) || //ABC...XYZ
             (c=='-' || c=='_' || c=='.' || c=='~')
        )
        {
            e << c;
        }
        else
        {
            e << '%';
            e << lookup[ (c&0xF0)>>4 ];
            e << lookup[ (c&0x0F) ];
        }
    }
    s = e.str();
}

void print_cc_diff(std::ostream *_stream, double diff, int limit);

#ifdef OS_UNIX
inline std::vector<std::string> glob_pattern(const std::string& pattern) 
{
	using namespace std;

	// glob struct resides on the stack
	glob_t glob_result;
	memset(&glob_result, 0, sizeof(glob_result));

	// do the glob operation
	int return_value = glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
	if (return_value != 0) 
	{
		globfree(&glob_result);

		if (return_value == GLOB_NOMATCH)
		{
			return std::vector<std::string>();
		}

		stringstream ss;
		ss << "glob() failed with return_value " << return_value << endl;
		throw std::runtime_error(ss.str());
	}

	// collect all the filenames into a std::list<std::string>
	vector<string> filenames;

	for (size_t i = 0; i < glob_result.gl_pathc; i++)
	{
		filenames.push_back(string(glob_result.gl_pathv[i]));
	}

	// cleanup
	globfree(&glob_result);

	// done
	return filenames;
}
#else
#ifdef OS_WINDOWS
inline std::vector<std::string> glob_pattern(const std::string& pattern)
{
    return std::vector<std::string>();
}
#endif
#endif

class FileReader
{

public:
	static void makeDirectoryIfNeeded(std::string _dir)
	{
		DIR *dir = opendir(_dir.c_str());

		if (dir)
		{
			closedir(dir);
		}
		else if (ENOENT == errno)
		{
#ifdef OS_UNIX
            mkdir(_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#else
#ifdef OS_WINDOWS
            mkdir(_dir.c_str());
#endif
#endif
		}
	}

	static void setOutputDirectory(std::string _dir)
	{
		outputDir = _dir;

		makeDirectoryIfNeeded(outputDir);
	}

	static std::string addOutputDirectory(std::string filename,
	                                      std::string subdir = "")
	{
		if (!outputDir.length())
		{
			return filename;
		}

		if (outputDir[0] == '/')
		{
			return outputDir + "/" + filename;
		}
		
		if (subdir.length())
		{
			makeDirectoryIfNeeded("./" + outputDir + "/" + subdir);
			subdir += "/";
		}

		std::string fullPath = "./" + outputDir + "/" + subdir + filename;
		return fullPath;
	}

private:
	static std::string outputDir;

};

#endif /* defined(__GameDriver__FileReader__) */
