/*
 * $Id$
 *
 * DEBUG: section 16    Cache Manager API
 *
 */

#include "config.h"
#include "base/TextException.h"
#include "ipc/Messages.h"
#include "ipc/TypedMsgHdr.h"
#include "mgr/IoAction.h"
#include "SquidMath.h"
#include "Store.h"


extern void GetIoStats(Mgr::IoActionData& stats);
extern void DumpIoStats(Mgr::IoActionData& stats, StoreEntry* sentry);

Mgr::IoActionData::IoActionData()
{
    xmemset(this, 0, sizeof(*this));
}

Mgr::IoActionData&
Mgr::IoActionData::operator += (const IoActionData& stats)
{
    http_reads += stats.http_reads;
    for (int i = 0; i < _iostats::histSize; ++i)
        http_read_hist[i] += stats.http_read_hist[i];
    ftp_reads += stats.ftp_reads;
    for (int i = 0; i < _iostats::histSize; ++i)
        ftp_read_hist[i] += stats.ftp_read_hist[i];
    gopher_reads += stats.gopher_reads;
    for (int i = 0; i < _iostats::histSize; ++i)
        gopher_read_hist[i] += stats.gopher_read_hist[i];

    return *this;
}

Mgr::IoAction::Pointer
Mgr::IoAction::Create(const CommandPointer &cmd)
{
    return new IoAction(cmd);
}

Mgr::IoAction::IoAction(const CommandPointer &cmd):
        Action(cmd), data()
{
    debugs(16, 5, HERE);
}

void
Mgr::IoAction::add(const Action& action)
{
    debugs(16, 5, HERE);
    data += dynamic_cast<const IoAction&>(action).data;
}

void
Mgr::IoAction::collect()
{
    GetIoStats(data);
}

void
Mgr::IoAction::dump(StoreEntry* entry)
{
    debugs(16, 5, HERE);
    Must(entry != NULL);
    DumpIoStats(data, entry);
}

void
Mgr::IoAction::pack(Ipc::TypedMsgHdr& msg) const
{
    msg.setType(Ipc::mtCacheMgrResponse);
    msg.putPod(data);
}

void
Mgr::IoAction::unpack(const Ipc::TypedMsgHdr& msg)
{
    msg.checkType(Ipc::mtCacheMgrResponse);
    msg.getPod(data);
}
