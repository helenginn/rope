#include "../Window.h"
#include "../FileManager.h"
#include "../FileView.h"

int main()
{
	FileView *fv = new FileView(nullptr);
	delete fv;

	FileManager *manager = Window::fileManager();
	
	manager->acceptFile("file", true);

	return 0;
}
