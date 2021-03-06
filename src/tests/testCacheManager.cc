#define SQUID_UNIT_TEST 1

#include "squid.h"
#include <cppunit/TestAssert.h>
#include "mgr/Action.h"

#include "Mem.h"
#include "testCacheManager.h"
#include "CacheManager.h"
#include "Store.h"


CPPUNIT_TEST_SUITE_REGISTRATION( testCacheManager );

/* init memory pools */

void testCacheManager::setUp()
{
    Mem::Init();
}

/*
 * Test creating a CacheManager
 */
void
testCacheManager::testCreate()
{
    CacheManager::GetInstance(); //it's a singleton..
}

/* an action to register */
static void
dummy_action(StoreEntry * sentry)
{
    sentry->flags=1;
}

/*
 * registering an action makes it findable.
 */
void
testCacheManager::testRegister()
{
    CacheManager *manager=CacheManager::GetInstance();
    CPPUNIT_ASSERT(manager != NULL);

    manager->registerProfile("sample", "my sample", &dummy_action, false, false);
    Mgr::Action::Pointer action = manager->createNamedAction("sample");
    CPPUNIT_ASSERT(action != NULL);

    const Mgr::ActionProfile::Pointer profile = action->command().profile;
    CPPUNIT_ASSERT(profile != NULL);
    CPPUNIT_ASSERT(profile->creator != NULL);
    CPPUNIT_ASSERT_EQUAL(false, profile->isPwReq);
    CPPUNIT_ASSERT_EQUAL(false, profile->isAtomic);
    CPPUNIT_ASSERT_EQUAL(String("sample"), String(action->name()));

    StoreEntry *sentry=new StoreEntry();
    sentry->flags=0x25; //arbitrary test value
    action->run(sentry, false);
    CPPUNIT_ASSERT_EQUAL(1,(int)sentry->flags);
}
