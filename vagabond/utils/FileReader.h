//
//  FileReader.h
//  GameDriver
//
//  Created by Helen Ginn on 21/05/2014.
//  Copyright (c) 2014 Helen Ginn. All rights reserved.
//

#ifndef __FileReader__
#define __FileReader__

#include <filesystem>
#include <sstream>

#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> split(const std::string &s, char delim);
void escape_filename(std::string &file);

inline std::string i_to_str(int val)
{
	return std::to_string(val);
}

std::string f_to_str(double val, int precision);

std::string findNewFolder(std::string prefix = "refine_");

bool is_str_alphabetical(const std::string &str);

/* Random string things */

void trim(std::string& str);
void trim(std::string& str, char trim_it);
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

class FileReader
{

public:
    static bool file_exists(const std::string& filename);
    static std::string getPath(const std::string& filename);
    static std::string getFilename(const std::string& filename);
    static std::string getBaseFilename(const std::string& filename);
    static std::string getExtension(const std::string& filename);
    static std::string getBaseFilenameWithPath(const std::string& filename);
    static std::string findNextFilename(const std::string& filename);
	static void makeDirectoryIfNeeded(std::string _dir);
    static std::string get_file_contents(std::string filename);
    static std::vector<std::string> glob_pattern(const std::string &pattern);

};

#endif /* defined(__GameDriver__FileReader__) */
