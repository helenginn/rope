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
#include "../VagWindow.h"
#include <curl/curl.h>
#include <string>
#include <cstring>
#include <fstream>

int main(int argc, char **argv)
{
	curl_global_init(CURL_GLOBAL_ALL);
	
	bool runmode = (argc > 1 ? (strcmp(argv[1], "run") == 0) : false);
	bool recordmode = (argc > 1 ? (strcmp(argv[1], "record") == 0) : false);

	std::string file = "";
	if (runmode || recordmode)
	{
		if (runmode) std::cout << "Run mode" << std::endl;
		if (recordmode) std::cout << "Record mode" << std::endl;
		file = (argc > 2 ? argv[2] : "");
	}

	CheckList check(runmode ? file : "");

	VagWindow window;
	window.setup(1, argv);
	window.setRecordFile(recordmode ? file : "");

	while (window.tick())
	{
		bool done = check.next();
		if (done)
		{
			window.setRecordFile(file);
		}
	}

	curl_global_cleanup();

	return 0;
}
