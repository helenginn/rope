#include <vagabond/gui/elements/Button.h>
#include <iostream>

int main()
{
	Button *button = new Button(NULL);
	button->mouseOver();
	
	if (!button->isSelected())
	{
		throw(std::runtime_error("Button is not selected after mouse over"));
		return 1;
	}

	button->unMouseOver();
	
	if (button->isSelected())
	{
		throw(std::runtime_error("Button is selected after mouse over"));
		return 1;
	}

	delete button;
	return 0;
}

