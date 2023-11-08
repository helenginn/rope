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

#include "files/FastaFile.h"
#include <fstream>

FastaFile::FastaFile(std::string filename) : File(filename)
{

}

File::Type FastaFile::cursoryLook()
{
	std::string path = toFilename(_filename);

	std::ifstream f;
	f.open(path);
	
	if (!f.is_open())
	{
		std::cout << "Could not open file: " << _filename << std::endl;
		return Nothing;
	}

	int seqs = 0;
	std::string head, body;
	while (f.good())
	{
		getline(f, head);
		getline(f, body);
		
		if (!f.good())
		{
			break;
		}

		defenestrate(head);
		debom(head);

		if (head.length() > 0 && head[0] == '>' && body.length())
		{
			seqs++;
		}
	}
	
	f.close();
	
	return (seqs > 0 ? Sequence : Nothing);
}

void FastaFile::parse()
{
	std::string path = toFilename(_filename);

	std::ifstream f;
	f.open(path);
	
	if (!f.is_open())
	{
		std::cout << "Could not open file: " << _filename << std::endl;
		return;
	}

	std::string full_head, body;
	while (f.good())
	{
		getline(f, full_head);
		getline(f, body);
		
		if (!f.good())
		{
			break;
		}
		defenestrate(full_head);
		debom(full_head);

		if (full_head.length())
		{
			std::string head = std::string(&full_head[1]);
			_sequences[head] = body;
		}
	}
	
	f.close();
}
