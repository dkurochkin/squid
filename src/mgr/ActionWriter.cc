/*
 * $Id$
 *
 * DEBUG: section 16    Cache Manager API
 *
 */

#include "config.h"
#include "base/TextException.h"
#include "comm/Connection.h"
#include "mgr/ActionWriter.h"
#include "Store.h"


CBDATA_NAMESPACED_CLASS_INIT(Mgr, ActionWriter);

Mgr::ActionWriter::ActionWriter(const Action::Pointer &anAction, const Comm::ConnectionPointer &conn):
        StoreToCommWriter(conn, anAction->createStoreEntry()),
        action(anAction)
{
    debugs(16, 5, HERE << conn << " action: " << action);
}

void
Mgr::ActionWriter::start()
{
    debugs(16, 5, HERE);
    Must(action != NULL);

    StoreToCommWriter::start();
    action->fillEntry(entry, false);
}
