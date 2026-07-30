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

	/** rotateRoundCentre() alone shears this image, because makeQuad()
	 * already bakes a non-uniform aspect/texture stretch into its
	 * vertices (see makeQuad()) - this conjugates the rotation by that
	 * same stretch so the image's true proportions survive the turn. */
	void rotateAspectCorrected(float degrees,
	                           glm::vec3 axis = glm::vec3(0, 0, -1));

	void setQuickSwitch(const std::vector<std::string> &filenames);
	virtual void makeQuad();
protected:
	int _w{};
	int _h{};
	
	std::map<std::string, GLuint> _switches;
};

#endif
