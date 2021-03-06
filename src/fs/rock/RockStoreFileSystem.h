#ifndef SQUID_FS_ROCK_FS_H
#define SQUID_FS_ROCK_FS_H

#include "StoreFileSystem.h"

namespace Rock
{

/// \ingroup Rock, FileSystems
class StoreFileSystem: public ::StoreFileSystem
{

public:
    static void Stats(StoreEntry * sentry);

    StoreFileSystem();
    virtual ~StoreFileSystem();

    virtual char const *type() const;
    virtual SwapDir *createSwapDir();
    virtual void done();
    virtual void registerWithCacheManager();
    virtual void setup();

private:
    //static Stats Stats_;

    StoreFileSystem(const StoreFileSystem &); // not implemented
    StoreFileSystem &operator=(const StoreFileSystem &); // not implemented
};

} // namespace Rock

#endif /* SQUID_FS_ROCK_FS_H */
