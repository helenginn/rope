#ifndef __vagabond__Cif2Geometry__
#define __vagabond__Cif2Geometry__

#include <string>

#include <gemmi/cif.hpp>

class Atom;

class Cif2Geometry
{
public:
	Cif2Geometry(std::string filename);
	
	void pullOutCode(std::string code)
	{
		_code = code;
	}

	void parse();
	
	size_t atomCount()
	{
		return _atoms.size();
	}
	
	Atom *atom(int i)
	{
		return _atoms[i];
	}
private:
	void processLoop(gemmi::cif::Loop &loop);

	std::string _filename;
	std::string _code;

	std::vector<Atom *> _atoms;
};

#endif
