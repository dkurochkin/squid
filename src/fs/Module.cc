#include "squid.h"
#include "Module.h"
#if defined(HAVE_FS_UFS) || defined(HAVE_FS_AUFS) || defined(HAVE_FS_DISKD)
#include "fs/ufs/StoreFSufs.h"
#include "fs/ufs/ufscommon.h"
#endif

#if HAVE_FS_COSS
#include "fs/coss/StoreFScoss.h"
#endif

#if HAVE_FS_UFS
static StoreFSufs<UFSSwapDir> *UfsInstance;
#endif

#if HAVE_FS_AUFS
static StoreFSufs<UFSSwapDir> *AufsInstance;
#endif


#if HAVE_FS_DISKD
static StoreFSufs<UFSSwapDir> *DiskdInstance;
#endif

#if HAVE_FS_ROCK
#include "fs/rock/RockStoreFileSystem.h"
static Rock::StoreFileSystem *RockInstance = NULL;
#endif


/* TODO: Modify coss code to:
 * (a) remove the StoreFScoss::GetInstance method,
 * (b) declare the StoreFScoss::stats  as static and
 * (c) merge the StoreFScoss::stat() method with the static
 *     StoreFScoss::Stats() */
#if HAVE_FS_COSS
StoreFScoss &CossInstance = StoreFScoss::GetInstance();
#endif


void Fs::Init()
{

#if HAVE_FS_UFS
    UfsInstance = new StoreFSufs<UFSSwapDir>("Blocking", "ufs");
#endif

#if HAVE_FS_AUFS
    AufsInstance = new StoreFSufs<UFSSwapDir>("DiskThreads", "aufs");;
#endif


#if HAVE_FS_DISKD
    DiskdInstance = new StoreFSufs<UFSSwapDir>("DiskDaemon", "diskd");;
#endif

#if HAVE_FS_ROCK
    RockInstance = new Rock::StoreFileSystem();
#endif

}


void Fs::Clean()
{
#if HAVE_FS_UFS
    delete UfsInstance;
#endif

#if HAVE_FS_AUFS
    delete AufsInstance;
#endif


#if HAVE_FS_DISKD
    delete DiskdInstance;
#endif

#if HAVE_FS_ROCK
    delete RockInstance;
#endif

}
