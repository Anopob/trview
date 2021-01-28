#include "trlevel.h"
#include "Level.h"
#include "LevelLoader.h"

namespace trlevel
{
    std::unique_ptr<ILevel> load_level(const std::string& filename)
    {
        LevelLoader loader { filename };
        return loader.to_level ();
    }
}