
/*
 * $Id: CommCalls.h,v 1.1 2008/02/12 22:58:29 rousskov Exp $
 */

#ifndef SQUID_COMMCALLS_H
#define SQUID_COMMCALLS_H

#include "comm.h"
#include "ConnectionDetail.h"
#include "AsyncCall.h"
#include "AsyncJobCalls.h"

/* CommCalls implement AsyncCall interface for comm_* callbacks.
 * The classes cover two call dialer kinds:
 *     - A C-style call using a function pointer (depricated);
 *     - A C++-style call to an AsyncJob child.
 * and three comm_* callback kinds:
 *     - accept (IOACB),
 *     - connect (CNCB),
 *     - I/O (IOCB).
 */

 /*
  * TODO: When there are no function-pointer-based callbacks left, all
  * this complexity can be removed. Jobs that need comm services will just
  * implement CommReader, CommWriter, etc. interfaces and receive calls
  * using general (not comm-specific) AsyncCall code. For now, we have to
  * allow the caller to create a callback that comm can modify to set
  * parameters, which is not trivial when the caller type/kind is not
  * known to comm and there are many kinds of parameters.
  */


/* Comm*CbParams classes below handle callback parameters */

// Maintains parameters common to all comm callbacks
class CommCommonCbParams {
public:
    CommCommonCbParams(void *aData);
    CommCommonCbParams(const CommCommonCbParams &params);
    ~CommCommonCbParams();

    void print(std::ostream &os) const;

public:
    void *data; // cbdata-protected
    int fd;
    int xerrno;
    comm_err_t flag;

private:
    // should not be needed and not yet implemented
    CommCommonCbParams &operator =(const CommCommonCbParams &params); 
};

// accept parameters
class CommAcceptCbParams: public CommCommonCbParams {
public:
    CommAcceptCbParams(void *aData);

    void print(std::ostream &os) const;

public:
    ConnectionDetail details;
    int nfd; // TODO: rename to fdNew or somesuch
};

// connect parameters
class CommConnectCbParams: public CommCommonCbParams {
public:
    CommConnectCbParams(void *aData);
};

// read/write (I/O) parameters
class CommIoCbParams: public CommCommonCbParams {
public:
    CommIoCbParams(void *aData);

    void print(std::ostream &os) const;

public:
    char *buf;
    size_t size;
};

// close parameters
class CommCloseCbParams: public CommCommonCbParams {
public:
    CommCloseCbParams(void *aData);
};

class CommTimeoutCbParams: public  CommCommonCbParams {
public:
    CommTimeoutCbParams(void *aData);
};

// Interface to expose comm callback parameters of all comm dialers.
// GetCommParams() uses this interface to access comm parameters.
template <class Params_>
class CommDialerParamsT {
public:
    typedef Params_ Params;
    CommDialerParamsT(const Params &io): params(io) {}

public:
    Params params;
};

// Get comm params of an async comm call
template <class Params>
Params &GetCommParams(AsyncCall::Pointer &call) { 
	typedef CommDialerParamsT<Params> DialerParams;
    DialerParams *dp = dynamic_cast<DialerParams*>(call->getDialer());
    assert(dp);
    return dp->params;
}


// All job dialers with comm parameters are merged into one since they
// all have exactly one callback argument and differ in Params type only
template <class C, class Params_>
class CommCbMemFunT: public JobDialer, public CommDialerParamsT<Params_>
{
public:
    typedef Params_ Params;
    typedef void (C::*Method)(const Params &io);

    CommCbMemFunT(C *obj, Method meth): JobDialer(obj),
        CommDialerParamsT<Params>(obj), object(obj), method(meth) {}

    virtual void print(std::ostream &os) const {
        os << '('; this->params.print(os); os << ')'; }

public:
	C *object;
    Method method;

protected:
    virtual void doDial() { (object->*method)(this->params); }
};


// accept (IOACB) dialer
class CommAcceptCbPtrFun: public CallDialer,
    public CommDialerParamsT<CommAcceptCbParams>
{
public:
    typedef CommAcceptCbParams Params;

    CommAcceptCbPtrFun(IOACB *aHandler, const CommAcceptCbParams &aParams);
    void dial();

    virtual void print(std::ostream &os) const;

public:
    IOACB *handler;
};

// connect (CNCB) dialer
class CommConnectCbPtrFun: public CallDialer,
    public CommDialerParamsT<CommConnectCbParams>
{
public:
    typedef CommConnectCbParams Params;

    CommConnectCbPtrFun(CNCB *aHandler, const Params &aParams);
    void dial();

    virtual void print(std::ostream &os) const;

public:
    CNCB *handler;
};


// read/write (IOCB) dialer
class CommIoCbPtrFun: public CallDialer,
    public CommDialerParamsT<CommIoCbParams>
{
public:
    typedef CommIoCbParams Params;

    CommIoCbPtrFun(IOCB *aHandler, const Params &aParams);
    void dial();

    virtual void print(std::ostream &os) const;

public:
    IOCB *handler;
};


// close (PF) dialer
class CommCloseCbPtrFun: public CallDialer,
    public CommDialerParamsT<CommCloseCbParams>
{
public:
    typedef CommCloseCbParams Params;

    CommCloseCbPtrFun(PF *aHandler, const Params &aParams);
    void dial();

    virtual void print(std::ostream &os) const;

public:
    PF *handler;
};

class CommTimeoutCbPtrFun:public CallDialer,
    public CommDialerParamsT<CommTimeoutCbParams>
{
public:
    typedef CommTimeoutCbParams Params;

    CommTimeoutCbPtrFun(PF *aHandler, const Params &aParams);
    void dial();

    virtual void print(std::ostream &os) const;

public:
    PF *handler;
};

// AsyncCall to comm handlers implemented as global functions.
// The dialer is one of the Comm*CbPtrFunT above
// TODO: Get rid of this class by moving canFire() to canDial() method
// of dialers.
template <class Dialer>
class CommCbFunPtrCallT: public AsyncCall {
public:
    typedef typename Dialer::Params Params;

    inline CommCbFunPtrCallT(int debugSection, int debugLevel,
        const char *callName, const Dialer &aDialer);

    virtual CallDialer* getDialer() { return &dialer; }

public:
    Dialer dialer;

protected:
    inline virtual bool canFire();
    inline virtual void fire();
};

// Conveninece wrapper: It is often easier to call a templated function than
// to create a templated class.
template <class Dialer>
inline
CommCbFunPtrCallT<Dialer> *commCbCall(int debugSection, int debugLevel,
    const char *callName, const Dialer &dialer)
{
    return new CommCbFunPtrCallT<Dialer>(debugSection, debugLevel, callName,
        dialer);
}

/* inlined implementation of templated methods */

/* CommCbFunPtrCallT */

template <class Dialer>
CommCbFunPtrCallT<Dialer>::CommCbFunPtrCallT(int debugSection, int debugLevel,
    const char *callName, const Dialer &aDialer): 
        AsyncCall(debugSection, debugLevel, callName),
        dialer(aDialer)
{
}


template <class Dialer>
bool
CommCbFunPtrCallT<Dialer>::canFire()
{
    if (!AsyncCall::canFire())
        return false;

    if (!cbdataReferenceValid(dialer.params.data))
        return cancel("callee gone");

    return true;
}

template <class Dialer>
void
CommCbFunPtrCallT<Dialer>::fire()
{
    dialer.dial();
}

#endif /* SQUID_COMMCALLS_H */