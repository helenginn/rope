#include <vagabond/core/Environment.h>
#include <vagabond/core/FileManager.h>
#include "../FileView.h"

int main()
{
	FileView *fv = new FileView(nullptr);
	delete fv;

	FileManager *manager = Environment::fileManager();
	
	manager->acceptFile("file", true);

	return 0;
}
