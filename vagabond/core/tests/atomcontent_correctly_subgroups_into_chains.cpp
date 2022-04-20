#include <vagabond/core/AtomContent.h>
#include <vagabond/core/Chain.h>
#include <iostream>

int main()
{
	AtomGroup *grp = new AtomGroup();
	Atom a, b, c;
	a.setChain("A");
	b.setChain("A");
	c.setChain("B");
	
	*grp += &a;
	*grp += &b;
	*grp += &c;
	
	AtomContent *cnt = new AtomContent(*grp);
	delete grp;

	int cnum = cnt->chainCount();
	
	if (cnum != 2)
	{
		std::cout << "Chain count: " << cnum << std::endl;
		return 1;
	}
	
	for (size_t i = 0; i < cnt->chainCount(); i++)
	{
		int num = cnt->chain(i)->size();
		std::string id = cnt->chain(i)->id();
		std::cout << "Chain " << id << " has num " << num << std::endl;
		
		if (id == "A" && num != 2)
		{
			return 1;
		}
		else if (id == "B" && num != 1)
		{
			return 1;
		}
	}

	return 0;
}
