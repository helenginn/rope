#ifndef __practical__Image__
#define __practical__Image__

#include <string>
#include "Box.h"

class Image : virtual public Box
{
public:
	Image(std::string filename);
	Image(int texid);
	
	void changeImage(std::string filename);

	void setQuickSwitch(const std::vector<std::string> &filenames);
	virtual void makeQuad();
protected:
	int _w{};
	int _h{};
	
	std::map<std::string, GLuint> _switches;
};

#endif
