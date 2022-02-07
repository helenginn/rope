#include "../FileReader.h"

int main()
{
	std::string filename = "/path/to/location/file.name.ext";
	std::string ret = getBaseFilename(filename);

	bool same = (ret == "file.name");
	return !same;
}


