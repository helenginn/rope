
#ifndef __vagabond__GuiAtom__
#define __vagabond__GuiAtom__

#include <vagabond/gui/elements/Renderable.h>
#include <atomic>
#include <vagabond/core/Atom.h>

class Icosahedron;
class GuiBond;
class AtomGroup;

class GuiAtom : public Renderable
{
public:
	GuiAtom();
	~GuiAtom();
	
	void watchAtom(Atom *a);
	void watchAtoms(AtomGroup *as);
	
	size_t verticesPerAtom();
	void checkAtoms();
	void startBackgroundWatch();

	void setMulti(bool m);
	void stop();

	virtual void render(SnowGL *gl);
private:
	size_t indicesPerAtom();
	void setPosition(glm::vec3 position);
	void colourByElement(std::string ele);
	bool checkAtom(Atom *a);
	void updateSinglePosition(Atom *a, glm::vec3 &p);
	void updateMultiPositions(Atom *a, Atom::WithPos &wp);
	static void backgroundWatch(GuiAtom *what);

	Icosahedron *_template;
	GuiBond *_bonds;

	std::vector<Atom *> _atoms;
	std::map<Atom *, int> _atomIndex;
	std::map<Atom *, glm::vec3> _atomPos;
	
	std::thread *_watch = nullptr;
	std::atomic<bool> _finish;
	
	bool _multi = true;
};

#endif
