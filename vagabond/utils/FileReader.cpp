//
//  FileReader.cpp
//  GameDriver
//
//  Created by Helen Ginn on 21/05/2014.
//  Copyright (c) 2014 Helen Ginn. All rights reserved.
//

#include <filesystem>
#include <system_error>
#include <stdexcept>

#include "os.h"
#ifdef OS_WINDOWS
    // windows.h needs to be imported first to prevent issues with compilers
    #include <windows.h>     // GetLastError, ERROR_ALREADY_EXISTS
    #include <fileapi.h>     // CreateDirectoryA, GetFileAttributesA, DWORD, INVALID_FILE_ATTRIBUTES, ...
    #include <direct.h>
    #include <sys/stat.h>
    #include <sys/types.h>
#else
#ifdef OS_UNIX
    #include <glob.h>        // glob, glob_t, globfree
    #include <dirent.h>      // opendir, readdir, closedir, DIR
    #include <unistd.h>
    #include <sys/stat.h>    // mkdir, mode_t, S_IRWXU et al.
    #include <sys/types.h>
    #include <cstring>
#endif
#endif
#include "FileReader.h"
#include <fstream>
#include <sstream>
#include <cerrno>
#include <cmath>

#include <iomanip>
#include <algorithm>

/**
 * Check if a file exists (cross-platform compatible)
 *
 * @param filename The name of the file to check
 * @returns true if the file exists, false otherwise
 * */
bool FileReader::file_exists(const std::string& filename)
{
    const std::filesystem::path filePath(filename);
    return std::filesystem::exists(filePath);
}

/**
 * Get the path from a full filename
 * E.g. Returns "/path/to/" from "/path/to/file.txt"
 *
 * @param filename The full path to the file
 * @return The path to the file, including trailing slash, or an empty string if no path exists
 */
std::string FileReader::getPath(const std::string& filename)
{
    std::filesystem::path filePath(filename);
    std::filesystem::path parentPath = filePath.parent_path();
    if (parentPath.empty())
    {
        return "";
    }
    return parentPath.string() + std::string(1, std::filesystem::path::preferred_separator);
}

/**
 * Get the filename from a path (including extension)
 * E.g. Returns "file.txt" from "/path/to/file.txt"
 *
 * @param filename The full path to the file
 * @return The filename with extension
 */
std::string FileReader::getFilename(const std::string& filename)
{
    const std::filesystem::path filePath(filename);
    return filePath.filename().string();
}

/**
 * Get the base filename (without extension) from a filename,
 * aka the file stem.
 * E.g. Returns "file" from "/path/to/file.txt"
 *
 * @param filename The full path to the file
 * @return The base filename without extension
 */
std::string FileReader::getBaseFilename(const std::string& filename)
{
    const std::filesystem::path filePath(filename);
    return filePath.stem().string();
}

/**
 * Get the file extension from a filename
 * E.g. Returns "txt" from "/path/to/file.txt"
 *
 * @param filename The full path to the file
 * @return The file extension (without the dot), or an empty string if no extension exists
 */
std::string FileReader::getExtension(const std::string& filename)
{
    const std::filesystem::path filePath(filename);
    const std::string ext = filePath.extension().string();
    return ext.length() > 1 ? ext.substr(1) : "";
}

/**
 * Get the base filename (without extension) from a filename,
 * but keep the path.
 * E.g. Returns "/path/to/file" from "/path/to/file.txt"
 *
 * @param filename The full path to the file
 * @return The base filename with path, without extension
 * */
std::string FileReader::getBaseFilenameWithPath(const std::string& filename)
{
    const std::filesystem::path filePath(filename);
    return (filePath.parent_path() / filePath.stem()).string();
}

/**
 * Find the next available filename by prepending a number to the base filename.
 * E.g. If "file.txt" is provided, it will check for "0_file.txt", "1_file.txt", etc.
 * and return the first filename that does not exist.
 *
 * @param filename The filename to check against
 * @return A new filename that does not exist yet
 */
std::string FileReader::findNextFilename(const std::string& filename)
{
    std::filesystem::path filePath(filename);
    int count = 0;
    while (true)
    {
        std::filesystem::path trial = filePath.parent_path() / (i_to_str(count) + "_" + filePath.filename().string());

        if (std::filesystem::exists(trial))
        {
            count++;
            continue;
        }
        return trial.string();
    }
}

/**
 * Create a directory if it does not already exist.
 * Throws an exception if the path exists but is not a directory,
 * or if the directory creation fails.
 * On Unix systems, it also sets the permissions to 775 (rwxrwxr-x).
 *
 * @param _dir The directory path to create
 */
void FileReader::makeDirectoryIfNeeded(std::string _dir)
{
    std::filesystem::path dir = std::filesystem::path(_dir);

    // Check if the path is empty
    if (dir.empty())
    {
        return;
    }

    // Normalize path
    std::filesystem::path normalized = dir.lexically_normal();

    // Reject absolute paths
    if (normalized.is_absolute())
    {
        throw std::runtime_error(
            "If you are going to add a path, please don't use an absolute path. "
            "I don't want to be responsible for ruining your filesystem.\n" +
            normalized.string()
        );
    }

    // Reject paths with dots
    if (normalized.string().find('.') != std::string::npos)
    {
        throw std::runtime_error(
            "If you are going to add a path, please don't use full stops (periods). "
            "It is dangerous for your filesystem.\n" +
            normalized.string()
        );
    }

    // Reject paths with more than one subdirectory
    //  Example: "foo" or "foo/bar" are allowed, but "foo/bar/baz" is not.
    std::filesystem::path relative = normalized.relative_path();
    if (std::distance(relative.begin(), relative.end()) > 2)
    {
        throw std::runtime_error(
            "Please no more than one subdirectory down. "
            "I cannot cope with such complexities.\n"
            + normalized.string()
        );
    }

    if (std::filesystem::exists(dir))
    {
        if (std::filesystem::is_directory(dir))
        {
            return; // Directory already exists
        }
        throw std::runtime_error("Path already exists but is not a directory: " + dir.string());
    }

    // Create directory and capture any errors
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec)
    {
        throw std::runtime_error("Failed to create directory: " + dir.string() +
            "\nError: " + ec.message());
    }

    #ifdef OS_UNIX
    // Update permissions on UNIX systems to 775 (rwxrwxr-x)
    std::filesystem::perms perms =
        std::filesystem::perms::owner_all |
        std::filesystem::perms::group_all |
        std::filesystem::perms::others_read |
        std::filesystem::perms::others_exec;

    std::error_code permsEc;
    std::filesystem::permissions(dir, perms, std::filesystem::perm_options::replace, permsEc);
    if (permsEc)
    {
        throw std::runtime_error("Failed to update permissions for directory: " + dir.string() +
            "\nError: " + permsEc.message());
    }
    #endif
}

/**
 * Read the entire contents of a file into a string.
 * Throws an exception if the file cannot be opened or read.
 *
 * @param filename The name of the file to read
 * @return The contents of the file as a string
 */
std::string FileReader::get_file_contents(std::string filename)
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

    throw(std::runtime_error(errString));
}

std::vector<std::string> FileReader::glob_pattern(const std::string &pattern)
{
#ifdef OS_UNIX
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
#else
#ifdef OS_WINDOWS
    std::vector<std::string> results;

    // Split pattern into directory part and wildcard part
    std::string dir;
    std::string mask;

    std::size_t pos = pattern.find_last_of("\\/");
    if (pos == std::string::npos)
    {
        dir = ".";
        mask = pattern;
    }
    else
    {
        dir  = pattern.substr(0, pos);
        mask = pattern.substr(pos + 1);
    }

    std::string searchPath = dir + "\\" + mask;

    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        // No match or error; for *glob*\(\) parity, just return empty on "no match"
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            return results;
        }
        else
        {
            std::ostringstream ss;
            ss << "FindFirstFileA failed for pattern \"" << pattern
               << "\". GetLastError=" << GetLastError();
            throw std::runtime_error(ss.str());
        }
    }

    // Enumerate files that match the pattern
    do
    {
        const char *name = ffd.cFileName;
        // Skip "." and ".."
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;

        std::string fullPath = name;
        results.push_back(fullPath);
    }
    while (FindNextFileA(hFind, &ffd) != 0);

    FindClose(hFind);
    return results;

#endif
#endif
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) 
{
	if (s.length() == 0)
	{
		elems.push_back(s);
		return elems;
	}

	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim))
	{
		elems.push_back(item);
	}
	
	if (s.back() == delim)
	{
		elems.push_back("");
	}
	return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

void debom(std::string &name)
{
	if (name.size() >= 3)
	{
		if (name[0] == (char)0xef && name[1] == (char)0xbb 
		    && name[2] == (char)0xbf)
		{
			for (size_t i = 0; i < 3; i++)
			{
				name.erase(name.begin());
			}
		}
	}
}

void remove_quotes(std::string &str)
{
	if (str[0] == '\"' && str.back() == '\"')
	{
		str.erase(str.begin());
		str.pop_back();
	}
}

void defenestrate(std::string &str)
{
	std::ostringstream ss;
	
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] != '\r')
		{
			ss << str[i];
		}
	}

	str = ss.str();
}

void trim(std::string &str)
{
	trim(str, ' ');
}

void trim(std::string &str, char trim_it)
{
	std::string::size_type pos = str.find_last_not_of(trim_it);
	if(pos != std::string::npos)
	{
		str.erase(pos + 1);
		pos = str.find_first_not_of(trim_it);
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

		if (!FileReader::file_exists(test))
		{
			return test;
		}
		
		count++;
	}
}

bool is_str_alphabetical(const std::string &str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		if ((str[i] < 'a' || str[i] > 'z') && (str[i] < 'A' || str[i] > 'Z')
		    && str[i] != ' ')
		{
			return false;
		}
	}

	return true;
}

void escape_filename(std::string &file)
{
	for (size_t i = 0; i < file.size(); i++)
	{
		if (file[i] == ' ')
		{
			file.insert(i, "\\");
			i++;
		}
	}
}

int count_chars(const std::string &s, const char &ch)
{
	int count = 0;

	for (int i = 0; i < s.size(); i++)
	{
		if (s[i] == ch)
		{
			count++;
		}
	}

	return count;
}

