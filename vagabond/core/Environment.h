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

#ifndef __vagabond__Environment__
#define __vagabond__Environment__

class FileManager;

class Environment
{
public:
	Environment();

	static FileManager *fileManager()
	{
		return _environment._fileManager;
	}

	static Environment env()
	{
		return _environment;
	}

	static Environment environment()
	{
		return _environment;
	}
	
	void save();
	void loadDefault();
private:
	FileManager *_fileManager;

	static Environment _environment;
};

#endif
