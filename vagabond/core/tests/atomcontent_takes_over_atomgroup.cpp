#include <vagabond/core/AtomContent.h>

int main()
{
	AtomGroup *grp = new AtomGroup();
	Atom a, b, c;
	
	*grp += &a;
	*grp += &b;
	*grp += &c;
	
	AtomContent *cnt = new AtomContent(*grp);
	
	delete grp;

	return !(cnt->size() == 3);
}
