/*
 * $Id$
 *
 * DEBUG: section 54    Interprocess Communication
 *
 */

#include "config.h"
#include "base/Subscription.h"
#include "base/TextException.h"
#include "comm/Connection.h"
#include "ipc/Strand.h"
#include "ipc/StrandCoord.h"
#include "ipc/Messages.h"
#include "ipc/SharedListen.h"
#include "ipc/StrandSearch.h"
#include "ipc/Kids.h"
#include "mgr/Request.h"
#include "mgr/Response.h"
#include "mgr/Forwarder.h"
#include "SwapDir.h" /* XXX: scope boundary violation */
#include "CacheManager.h"
#if USE_DISKIO_IPCIO
#include "DiskIO/IpcIo/IpcIoFile.h" /* XXX: scope boundary violation */
#endif
#if SQUID_SNMP
#include "snmp/Forwarder.h"
#include "snmp/Request.h"
#include "snmp/Response.h"
#endif

CBDATA_NAMESPACED_CLASS_INIT(Ipc, Strand);


Ipc::Strand::Strand():
        Port(MakeAddr(strandAddrPfx, KidIdentifier)),
        isRegistered(false)
{
}

void Ipc::Strand::start()
{
    Port::start();
    registerSelf();
}

void Ipc::Strand::registerSelf()
{
    debugs(54, 6, HERE);
    Must(!isRegistered);

    HereIamMessage ann(StrandCoord(KidIdentifier, getpid()));
    TypedMsgHdr message;
    ann.pack(message);
    SendMessage(coordinatorAddr, message);
    setTimeout(6, "Ipc::Strand::timeoutHandler"); // TODO: make 6 configurable?
}

void Ipc::Strand::receive(const TypedMsgHdr &message)
{
    debugs(54, 6, HERE << message.type());
    switch (message.type()) {

    case mtRegistration:
        handleRegistrationResponse(HereIamMessage(message));
        break;

    case mtSharedListenResponse:
        SharedListenJoined(SharedListenResponse(message));
        break;

#if USE_DISKIO_IPCIO
    case mtStrandSearchResponse:
        IpcIoFile::HandleOpenResponse(StrandSearchResponse(message));
        break;

    case mtIpcIoNotification:
        IpcIoFile::HandleNotification(message);
        break;
#endif /* USE_DISKIO_IPCIO */

    case mtCacheMgrRequest: {
        const Mgr::Request req(message);
        handleCacheMgrRequest(req);
    }
    break;

    case mtCacheMgrResponse: {
        const Mgr::Response resp(message);
        handleCacheMgrResponse(resp);
    }
    break;

#if SQUID_SNMP
    case mtSnmpRequest: {
        const Snmp::Request req(message);
        handleSnmpRequest(req);
    }
    break;

    case mtSnmpResponse: {
        const Snmp::Response resp(message);
        handleSnmpResponse(resp);
    }
    break;
#endif

    default:
        debugs(54, 1, HERE << "Unhandled message type: " << message.type());
        break;
    }
}

void Ipc::Strand::handleRegistrationResponse(const HereIamMessage &msg)
{
    // handle registration response from the coordinator; it could be stale
    if (msg.strand.kidId == KidIdentifier && msg.strand.pid == getpid()) {
        debugs(54, 6, "kid" << KidIdentifier << " registered");
        clearTimeout(); // we are done
    } else {
        // could be an ACK to the registration message of our dead predecessor
        debugs(54, 6, "kid" << KidIdentifier << " is not yet registered");
        // keep listening, with a timeout
    }
}

void Ipc::Strand::handleCacheMgrRequest(const Mgr::Request& request)
{
    Mgr::Action::Pointer action =
        CacheManager::GetInstance()->createRequestedAction(request.params);
    action->respond(request);
}

void Ipc::Strand::handleCacheMgrResponse(const Mgr::Response& response)
{
    Mgr::Forwarder::HandleRemoteAck(response.requestId);
}

#if SQUID_SNMP
void Ipc::Strand::handleSnmpRequest(const Snmp::Request& request)
{
    debugs(54, 6, HERE);
    Snmp::SendResponse(request.requestId, request.pdu);
}

void Ipc::Strand::handleSnmpResponse(const Snmp::Response& response)
{
    debugs(54, 6, HERE);
    Snmp::Forwarder::HandleRemoteAck(response.requestId);
}
#endif

void Ipc::Strand::timedout()
{
    debugs(54, 6, HERE << isRegistered);
    if (!isRegistered)
        fatalf("kid%d registration timed out", KidIdentifier);
}
