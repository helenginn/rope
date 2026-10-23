#ifndef __vagabond__CompareMatrixOptions__
#define __vagabond__CompareMatrixOptions__

#include <functional>
#include <vagabond/core/MatrixCache.h>

class Menu; 
class Scene; 

// Adds "Compare this matrix (matrix1/matrix2)" and 
// when filled, "Clear matrix1/matrix2" to a menu. 
// getMatrix is only called when the user picks "Compare"

void addCompareOptions(Menu *menu, Scene *scene, 
					   const std::function<SlotMatrix()> &getMatrix);

#endif