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

#include "UntangleWindow.h"
#include <vagabond/utils/gl_import.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <curl/curl.h>
#endif

float test = 0;

int main (int argc, char **argv)
{
	if (argc == 1)
	{
		std::cout << "Please supply ligand CIF files followed by "
		"your PDB file as command line arguments." << std::endl;
		exit(0);
	}

	UntangleWindow window;
	window.setup(argc, argv);

	while (window.tick())
	{

	}

	return 0;
}
