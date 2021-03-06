/*
 * $Id$
 *
 * DEBUG: section 54    Interprocess Communication
 *
 */

#include "config.h"
#include "base/TextException.h"
#include "base/RunnersRegistry.h"
#include "ipc/mem/PagePool.h"
#include "ipc/mem/Pages.h"
#include "structs.h"
#include "SwapDir.h"

// Uses a single PagePool instance, for now.
// Eventually, we may have pools dedicated to memory caching, disk I/O, etc.

// TODO: make pool id more unique so it does not conflict with other Squids?
static const char *PagePoolId = "squid-page-pool";
static Ipc::Mem::PagePool *ThePagePool = 0;

// TODO: make configurable to avoid waste when mem-cached objects are small/big
size_t
Ipc::Mem::PageSize()
{
    return 32*1024;
}

bool
Ipc::Mem::GetPage(const PageId::Purpose purpose, PageId &page)
{
    return ThePagePool && PagesAvailable(purpose) > 0 ?
           ThePagePool->get(purpose, page) : false;
}

void
Ipc::Mem::PutPage(PageId &page)
{
    Must(ThePagePool);
    ThePagePool->put(page);
}

char *
Ipc::Mem::PagePointer(const PageId &page)
{
    Must(ThePagePool);
    return ThePagePool->pagePointer(page);
}

size_t
Ipc::Mem::PageLimit()
{
    size_t limit = 0;
    for (int i = 0; i < PageId::maxPurpose; ++i)
        limit += PageLimit(i);
    return limit;
}

size_t
Ipc::Mem::PageLimit(const int purpose)
{
    switch (purpose) {
    case PageId::cachePage:
        return Config.memMaxSize > 0 ? Config.memMaxSize / PageSize() : 0;
    case PageId::ioPage:
        // XXX: this should be independent from memory cache pages
        return PageLimit(PageId::cachePage)/2;
    default:
        Must(false);
    }
    return 0;
}

size_t
Ipc::Mem::PageLevel()
{
    return ThePagePool ? ThePagePool->level() : 0;
}

size_t
Ipc::Mem::PageLevel(const int purpose)
{
    return ThePagePool ? ThePagePool->level(purpose) : 0;
}

/// initializes shared memory pages
class SharedMemPagesRr: public Ipc::Mem::RegisteredRunner
{
public:
    /* RegisteredRunner API */
    SharedMemPagesRr(): owner(NULL) {}
    virtual void run(const RunnerRegistry &);
    virtual void create(const RunnerRegistry &);
    virtual void open(const RunnerRegistry &);
    virtual ~SharedMemPagesRr();

private:
    Ipc::Mem::PagePool::Owner *owner;
};

RunnerRegistrationEntry(rrAfterConfig, SharedMemPagesRr);


void
SharedMemPagesRr::run(const RunnerRegistry &r)
{
    if (!UsingSmp())
        return;

    // When cache_dirs start using shared memory pages, they would
    // need to communicate their needs to us somehow.
    if (Config.memMaxSize <= 0)
        return;

    if (Ipc::Mem::PageLimit() <= 0) {
        if (IamMasterProcess()) {
            debugs(54, DBG_IMPORTANT, "WARNING: mem-cache size is too small ("
                   << (Config.memMaxSize / 1024.0) << " KB), should be >= " <<
                   (Ipc::Mem::PageSize() / 1024.0) << " KB");
        }
        return;
    }

    Ipc::Mem::RegisteredRunner::run(r);
}

void
SharedMemPagesRr::create(const RunnerRegistry &)
{
    Must(!owner);
    owner = Ipc::Mem::PagePool::Init(PagePoolId, Ipc::Mem::PageLimit(),
                                     Ipc::Mem::PageSize());
}

void
SharedMemPagesRr::open(const RunnerRegistry &)
{
    Must(!ThePagePool);
    ThePagePool = new Ipc::Mem::PagePool(PagePoolId);
}

SharedMemPagesRr::~SharedMemPagesRr()
{
    if (!UsingSmp())
        return;

    delete ThePagePool;
    ThePagePool = NULL;
    delete owner;
}
