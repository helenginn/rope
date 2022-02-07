#include "../FileReader.h"

int main()
{
	std::string filename = "/path/to/location/file.name.ext";
	std::string ret = getBaseFilenameWithPath(filename);

	bool same = (ret == "/path/to/location/file.name");
	return !same;
}

