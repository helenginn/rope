#ifndef __vagabond__FlexibilityCache__
#define __vagabond__FlexibilityCache__

#include <map>

class Instance;
class Flexibility;

class FlexibilityCache
{
public:
    FlexibilityCache() {};
    static FlexibilityCache& instance()
    {
        return cache;
    }

    void store(Instance *inst, Flexibility *flex)
    {
        _cache[inst] = flex;
    }

    Flexibility *get(Instance *inst)
    {
        if (_cache.count(inst))
        {
            return _cache[inst];
        }
        return nullptr;
    }

    void remove(Instance *inst)
    {
        _cache.erase(inst);
    }

private:
    static FlexibilityCache cache;
    FlexibilityCache(const FlexibilityCache&) = delete;
    FlexibilityCache& operator=(const FlexibilityCache&) = delete;

    std::map<Instance*, Flexibility*> _cache;
};

#endif


