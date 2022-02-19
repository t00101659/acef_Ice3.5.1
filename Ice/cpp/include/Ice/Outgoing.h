// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OUTGOING_H
#define ICE_OUTGOING_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/UniquePtr.h>
#include <Ice/RequestHandlerF.h>
#include <Ice/InstanceF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/ReferenceF.h>
#include <Ice/BasicStream.h>
#include <Ice/Current.h>
#include <Ice/ObserverHelper.h>
#include <Ice/RouterInfoF.h>
#include <Ice/Identity.h>
#include <Ice/LocatorInfoF.h>

namespace Ice
{

class LocalException;

}

namespace IceInternal
{

//
// An exception wrapper, which is used for local exceptions that
// require special retry considerations.
//
class ICE_API LocalExceptionWrapper
{
public:

    LocalExceptionWrapper(const Ice::LocalException&, bool);
    LocalExceptionWrapper(const LocalExceptionWrapper&);

    const Ice::LocalException* get() const;

    //
    // If true, always repeat the request. Don't take retry settings
    // or "at-most-once" guarantees into account.
    //
    // If false, only repeat the request if the retry settings allow
    // to do so, and if "at-most-once" does not need to be guaranteed.
    //
    bool retry() const;

    static void throwWrapper(const ::std::exception&);

private:

    const LocalExceptionWrapper& operator=(const LocalExceptionWrapper&);

    IceUtil::UniquePtr<Ice::LocalException> _ex;
    bool _retry;
};

class ICE_API OutgoingMessageCallback : private IceUtil::noncopyable
{
public:

    virtual ~OutgoingMessageCallback() { }
 
    virtual void sent(bool) = 0;
    virtual void finished(const Ice::LocalException&, bool) = 0;
};
class Outgoing;
class outgoingoperationhandler
{
public:
typedef struct stOutgoingInfo
{
    ::std::string strOperation;
    Ice::Int iRequestId;
    IceUtil::Int64 dwStartTick;
    Ice::Identity Id;
    ::std::string strFacet;
    ::std::string strAdapterId;
    ::std::string strConnectionId;
    IceInternal::LocatorInfoPtr locatorInfoPtr;
    IceInternal::RouterInfoPtr routerInfoPtr;
}TOutgoingInfo;
	void Addoperation(IceInternal::Outgoing* ptr,std::string stroperation)
	{
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        TOutgoingInfo info;
        info.strOperation = stroperation;
        _mapop[ptr] = info;
	}
	void Removeoperation(IceInternal::Outgoing* ptr)
	{
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        _mapop.erase(ptr);
	}
	std::string Getoperation(IceInternal::Outgoing* ptr)
	{
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        return _mapop[ptr].strOperation;  
	}
    void SetRequestId(IceInternal::Outgoing* ptr, Ice::Int requestId)
	{
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            itr->second.iRequestId = requestId;
        }
	}
    Ice::Int GetRequestId(IceInternal::Outgoing* ptr)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            return itr->second.iRequestId;
        }
        else
        {
            return -1;
        }
    }
    void SetStartTick(IceInternal::Outgoing* ptr, IceUtil::Int64 dwStartTick)
	{
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            itr->second.dwStartTick = dwStartTick;
        }
	}
    IceUtil::Int64 GetStartTick(IceInternal::Outgoing* ptr)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            return itr->second.dwStartTick;
        }
        else
        {
            return -1;
        }
    }
    void SetId(IceInternal::Outgoing* ptr, Ice::Identity& Id)
	{
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            itr->second.Id = Id;
        }
	}
    Ice::Identity GetId(IceInternal::Outgoing* ptr)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            return itr->second.Id;
        }
        else
        {
            Ice::Identity Id;
            Id.category = "";
            Id.name = "";
            return Id;
        }
    }
    void SetFacet(IceInternal::Outgoing* ptr, ::std::string strFacet)
	{
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            itr->second.strFacet = strFacet;
        }
	}
    ::std::string GetFacet(IceInternal::Outgoing* ptr)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            return itr->second.strFacet;
        }
        else
        {
            return ::std::string("");
        }
    }
    void SetAdapterId(IceInternal::Outgoing* ptr, ::std::string strAdapterId)
	{
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            itr->second.strAdapterId = strAdapterId;
        }
	}
    ::std::string GetAdapterId(IceInternal::Outgoing* ptr)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            return itr->second.strAdapterId;
        }
        else
        {
            return ::std::string("");
        }
    }
    void SetConnectionId(IceInternal::Outgoing* ptr, ::std::string strConnectionId)
	{
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            itr->second.strConnectionId = strConnectionId;
        }
	}
    ::std::string GetConnectionId(IceInternal::Outgoing* ptr)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            return itr->second.strConnectionId;
        }
        else
        {
            return ::std::string("");
        }
    }
    void SetLocatorInfo(IceInternal::Outgoing* ptr, IceInternal::LocatorInfoPtr locatorInfoPtr)
	{
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            itr->second.locatorInfoPtr = locatorInfoPtr;
        }
	}
    LocatorInfoPtr GetLocatorInfo(IceInternal::Outgoing* ptr)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            return itr->second.locatorInfoPtr;
        }
        else
        {
            return NULL;
        }
    }
    void SetRouterInfo(IceInternal::Outgoing* ptr, IceInternal::RouterInfoPtr routerInfoPtr)
	{
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            itr->second.routerInfoPtr = routerInfoPtr;
        }
	}
    RouterInfoPtr GetrouterInfo(IceInternal::Outgoing* ptr)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        std::map<IceInternal::Outgoing*, TOutgoingInfo>::iterator itr = _mapop.find(ptr);
        if (itr != _mapop.end())
        {
            return itr->second.routerInfoPtr;
        }
        else
        {
            return NULL;
        }
    }
protected:
	std::map<IceInternal::Outgoing*, TOutgoingInfo> _mapop;
	IceUtil::Monitor<IceUtil::Mutex> _monitor;
};
class ICE_API Outgoing : public OutgoingMessageCallback
{
public:

    Outgoing(RequestHandler*, const std::string&, Ice::OperationMode, const Ice::Context*, InvocationObserver&);
    ~Outgoing();

    bool invoke(); // Returns true if ok, false if user exception.
    void abort(const Ice::LocalException&);
    virtual void sent(bool);
    virtual void finished(BasicStream&);
    void finished(const Ice::LocalException&, bool);

    // Inlined for speed optimization.
    BasicStream* os() { return &_os; }
    BasicStream* startReadParams()
    {
        _is.startReadEncaps();
        return &_is;
    }
    void endReadParams()
    {
        _is.endReadEncaps();
    }
    void readEmptyParams()
    {
        _is.skipEmptyEncaps();
    }
    void readParamEncaps(const Ice::Byte*& encaps, Ice::Int& sz)
    {
        _is.readEncaps(encaps, sz);
    }

    BasicStream* startWriteParams(Ice::FormatType format)
    {
        _os.startWriteEncaps(_encoding, format);
        return &_os;
    }
    void endWriteParams()
    {
        _os.endWriteEncaps();
    }
    void writeEmptyParams()
    {
        _os.writeEmptyEncaps(_encoding);
    }
    void writeParamEncaps(const Ice::Byte* encaps, Ice::Int size)
    {
        if(size == 0)
        {
            _os.writeEmptyEncaps(_encoding);
        }
        else
        {
            _os.writeEncaps(encaps, size);
        }
    }

    bool hasResponse() 
    {
        return !_is.b.empty();
    }

    void throwUserException();

    void attachRemoteObserver(const Ice::ConnectionInfoPtr& c, const Ice::EndpointPtr& endpt,
                              Ice::Int requestId, Ice::Int size)
    {
        _remoteObserver.attach(_observer.getRemoteObserver(c, endpt, requestId, size));
    }

private:

    //
    // Optimization. The request handler and the reference may not be
    // deleted while a stack-allocated Outgoing still holds it.
    //
    RequestHandler* _handler;
    IceUtil::UniquePtr<Ice::LocalException> _exception;
    InvocationObserver& _observer;
    ObserverHelperT<Ice::Instrumentation::RemoteObserver> _remoteObserver;

    enum
    {
        StateUnsent,
        StateInProgress,
        StateOK,
        StateUserException,
        StateLocalException,
        StateFailed
    } _state;

    Ice::EncodingVersion _encoding;
    BasicStream _is;
    BasicStream _os;
    bool _sent;

    //
    // NOTE: we use an attribute for the monitor instead of inheriting
    // from the monitor template.  Otherwise, the template would be
    // exported from the DLL on Windows and could cause linker errors
    // because of multiple definition of IceUtil::Monitor<IceUtil::Mutex>, 
    // see bug 1541.
    //
    IceUtil::Monitor<IceUtil::Mutex> _monitor;
};

class BatchOutgoing : public OutgoingMessageCallback
{
public:

    BatchOutgoing(RequestHandler*, InvocationObserver&);
    BatchOutgoing(Ice::ConnectionI*, Instance*, InvocationObserver&);
    
    void invoke();
    
    virtual void sent(bool);
    virtual void finished(const Ice::LocalException&, bool);
    
    BasicStream* os() { return &_os; }

    void attachRemoteObserver(const Ice::ConnectionInfoPtr& connection, const Ice::EndpointPtr& endpt, Ice::Int sz)
    {
        _remoteObserver.attach(_observer.getRemoteObserver(connection, endpt, 0, sz));
    }

private:

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    RequestHandler* _handler;
    Ice::ConnectionI* _connection;
    bool _sent;
    IceUtil::UniquePtr<Ice::LocalException> _exception;

    BasicStream _os;

    InvocationObserver& _observer;
    ObserverHelperT<Ice::Instrumentation::RemoteObserver> _remoteObserver;
};

}

#endif
