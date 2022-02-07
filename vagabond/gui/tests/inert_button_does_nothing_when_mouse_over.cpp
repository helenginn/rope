#include "../Button.h"
#include <iostream>

int main()
{
	Button *button = new Button(NULL);
	button->setInert(true);
	button->mouseOver();
	
	if (button->isSelected())
	{
		throw(std::runtime_error("Inert button is selected after mouse over"));
	}

	delete button;
	return 0;
}


