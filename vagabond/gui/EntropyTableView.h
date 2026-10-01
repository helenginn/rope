#ifndef __vagabond__MetadataView__
#define __vagabond__MetadataView__

#include <vagabond/core/Responder.h>
#include <vagabond/core/PathEntropy.h>
#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/elements/ListView.h>

class Entity;

class EntropyTableView : public ListView
{
public:
	EntropyTableView(Scene *prev, Entity *entity, const std::vector<EntropyResults> &entropy);
	~EntropyTableView();

	virtual void setup();   
	virtual void buttonPressed(std::string tag, Button *button = nullptr);

    virtual size_t lineCount();
    virtual Renderable *getLine(int i);
private:
    Entity *_entity = nullptr;
	
    std::vector<EntropyResults> _entropy;
};

#endif
