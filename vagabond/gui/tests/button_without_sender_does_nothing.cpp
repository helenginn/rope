#include <vagabond/gui/elements/Button.h>

int main()
{
	Button *button = new Button(NULL);
	button->click();
	delete button;
	return 0;
}
