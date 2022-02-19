// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
//
// Ice version 3.5.1
//
// <auto-generated>
//
// Generated from file `Process.ice'
//
// Warning: do not edit this file.
//
// </auto-generated>
//

#ifndef ICE_API_EXPORTS
#   define ICE_API_EXPORTS
#endif
#include <Ice/Process.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectFactory.h>
#include <Ice/BasicStream.h>
#include <Ice/Object.h>
#include <IceUtil/Iterator.h>
#include <IceUtil/DisableWarnings.h>

#ifndef ICE_IGNORE_VERSION
#   if ICE_INT_VERSION / 100 != 305
#       error Ice version mismatch!
#   endif
#   if ICE_INT_VERSION % 100 > 50
#       error Beta header file detected
#   endif
#   if ICE_INT_VERSION % 100 < 1
#       error Ice patch level mismatch!
#   endif
#endif

namespace
{

const ::std::string __Ice__Process__shutdown_name = "shutdown";

const ::std::string __Ice__Process__writeMessage_name = "writeMessage";

}
#ifdef __SUNPRO_CC
class ICE_DECLSPEC_EXPORT IceProxy::Ice::Process;
#endif
ICE_DECLSPEC_EXPORT ::IceProxy::Ice::Object* ::IceProxy::Ice::upCast(::IceProxy::Ice::Process* p) { return p; }

void
::IceProxy::Ice::__read(::IceInternal::BasicStream* __is, ::IceInternal::ProxyHandle< ::IceProxy::Ice::Process>& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::Ice::Process;
        v->__copyFrom(proxy);
    }
}

void
IceProxy::Ice::Process::shutdown(const ::Ice::Context* __ctx)
{
    ::IceInternal::InvocationObserver __observer(this, __Ice__Process__shutdown_name, __ctx);
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __delBase = __getDelegate(false);
            ::IceDelegate::Ice::Process* __del = dynamic_cast< ::IceDelegate::Ice::Process*>(__delBase.get());
            __del->shutdown(__ctx, __observer);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex, __observer);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, true, __cnt, __observer);
        }
    }
}

::Ice::AsyncResultPtr
IceProxy::Ice::Process::begin_shutdown(const ::Ice::Context* __ctx, const ::IceInternal::CallbackBasePtr& __del, const ::Ice::LocalObjectPtr& __cookie)
{
    ::IceInternal::OutgoingAsyncPtr __result = new ::IceInternal::OutgoingAsync(this, __Ice__Process__shutdown_name, __del, __cookie);
    try
    {
        __result->__prepare(__Ice__Process__shutdown_name, ::Ice::Normal, __ctx);
        __result->__writeEmptyParams();
        __result->__send(true);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __result->__exceptionAsync(__ex);
    }
    return __result;
}

void
IceProxy::Ice::Process::end_shutdown(const ::Ice::AsyncResultPtr& __result)
{
    __end(__result, __Ice__Process__shutdown_name);
}

void
IceProxy::Ice::Process::writeMessage(const ::std::string& message, ::Ice::Int fd, const ::Ice::Context* __ctx)
{
    ::IceInternal::InvocationObserver __observer(this, __Ice__Process__writeMessage_name, __ctx);
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __delBase = __getDelegate(false);
            ::IceDelegate::Ice::Process* __del = dynamic_cast< ::IceDelegate::Ice::Process*>(__delBase.get());
            __del->writeMessage(message, fd, __ctx, __observer);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex, __observer);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, true, __cnt, __observer);
        }
    }
}

::Ice::AsyncResultPtr
IceProxy::Ice::Process::begin_writeMessage(const ::std::string& message, ::Ice::Int fd, const ::Ice::Context* __ctx, const ::IceInternal::CallbackBasePtr& __del, const ::Ice::LocalObjectPtr& __cookie)
{
    ::IceInternal::OutgoingAsyncPtr __result = new ::IceInternal::OutgoingAsync(this, __Ice__Process__writeMessage_name, __del, __cookie);
    try
    {
        __result->__prepare(__Ice__Process__writeMessage_name, ::Ice::Normal, __ctx);
        ::IceInternal::BasicStream* __os = __result->__startWriteParams(::Ice::DefaultFormat);
        __os->write(message);
        __os->write(fd);
        __result->__endWriteParams();
        __result->__send(true);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __result->__exceptionAsync(__ex);
    }
    return __result;
}

void
IceProxy::Ice::Process::end_writeMessage(const ::Ice::AsyncResultPtr& __result)
{
    __end(__result, __Ice__Process__writeMessage_name);
}

const ::std::string&
IceProxy::Ice::Process::ice_staticId()
{
    return ::Ice::Process::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::Ice::Process::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::Ice::Process);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::Ice::Process::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::Ice::Process);
}

::IceProxy::Ice::Object*
IceProxy::Ice::Process::__newInstance() const
{
    return new Process;
}

void
IceDelegateM::Ice::Process::shutdown(const ::Ice::Context* __context, ::IceInternal::InvocationObserver& __observer)
{
    ::IceInternal::Outgoing __og(__handler.get(), __Ice__Process__shutdown_name, ::Ice::Normal, __context, __observer);
    __og.writeEmptyParams();
    bool __ok = __og.invoke();
    if(__og.hasResponse())
    {
        try
        {
            if(!__ok)
            {
                try
                {
                    __og.throwUserException();
                }
                catch(const ::Ice::UserException& __ex)
                {
                    ::Ice::UnknownUserException __uue(__FILE__, __LINE__, __ex.ice_name());
                    throw __uue;
                }
            }
            __og.readEmptyParams();
        }
        catch(const ::Ice::LocalException& __ex)
        {
            throw ::IceInternal::LocalExceptionWrapper(__ex, false);
        }
    }
}

void
IceDelegateM::Ice::Process::writeMessage(const ::std::string& message, ::Ice::Int fd, const ::Ice::Context* __context, ::IceInternal::InvocationObserver& __observer)
{
    ::IceInternal::Outgoing __og(__handler.get(), __Ice__Process__writeMessage_name, ::Ice::Normal, __context, __observer);
    try
    {
        ::IceInternal::BasicStream* __os = __og.startWriteParams(::Ice::DefaultFormat);
        __os->write(message);
        __os->write(fd);
        __og.endWriteParams();
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    if(__og.hasResponse())
    {
        try
        {
            if(!__ok)
            {
                try
                {
                    __og.throwUserException();
                }
                catch(const ::Ice::UserException& __ex)
                {
                    ::Ice::UnknownUserException __uue(__FILE__, __LINE__, __ex.ice_name());
                    throw __uue;
                }
            }
            __og.readEmptyParams();
        }
        catch(const ::Ice::LocalException& __ex)
        {
            throw ::IceInternal::LocalExceptionWrapper(__ex, false);
        }
    }
}

void
IceDelegateD::Ice::Process::shutdown(const ::Ice::Context* __context, ::IceInternal::InvocationObserver&)
{
    class _DirectI : public ::IceInternal::Direct
    {
    public:

        _DirectI(const ::Ice::Current& __current) : 
            ::IceInternal::Direct(__current)
        {
        }
        
        virtual ::Ice::DispatchStatus
        run(::Ice::Object* object)
        {
            ::Ice::Process* servant = dynamic_cast< ::Ice::Process*>(object);
            if(!servant)
            {
                throw ::Ice::OperationNotExistException(__FILE__, __LINE__, _current.id, _current.facet, _current.operation);
            }
            servant->shutdown(_current);
            return ::Ice::DispatchOK;
        }
        
    private:
        
    };
    
    ::Ice::Current __current;
    __initCurrent(__current, __Ice__Process__shutdown_name, ::Ice::Normal, __context);
    try
    {
        _DirectI __direct(__current);
        try
        {
            __direct.getServant()->__collocDispatch(__direct);
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
    }
    catch(const ::Ice::SystemException&)
    {
        throw;
    }
    catch(const ::IceInternal::LocalExceptionWrapper&)
    {
        throw;
    }
    catch(const ::std::exception& __ex)
    {
        ::IceInternal::LocalExceptionWrapper::throwWrapper(__ex);
    }
    catch(...)
    {
        throw ::IceInternal::LocalExceptionWrapper(::Ice::UnknownException(__FILE__, __LINE__, "unknown c++ exception"), false);
    }
}

void
IceDelegateD::Ice::Process::writeMessage(const ::std::string& message, ::Ice::Int fd, const ::Ice::Context* __context, ::IceInternal::InvocationObserver&)
{
    class _DirectI : public ::IceInternal::Direct
    {
    public:

        _DirectI(const ::std::string& __p_message, ::Ice::Int __p_fd, const ::Ice::Current& __current) : 
            ::IceInternal::Direct(__current),
            _m_message(__p_message),
            _m_fd(__p_fd)
        {
        }
        
        virtual ::Ice::DispatchStatus
        run(::Ice::Object* object)
        {
            ::Ice::Process* servant = dynamic_cast< ::Ice::Process*>(object);
            if(!servant)
            {
                throw ::Ice::OperationNotExistException(__FILE__, __LINE__, _current.id, _current.facet, _current.operation);
            }
            servant->writeMessage(_m_message, _m_fd, _current);
            return ::Ice::DispatchOK;
        }
        
    private:
        
        const ::std::string& _m_message;
        ::Ice::Int _m_fd;
    };
    
    ::Ice::Current __current;
    __initCurrent(__current, __Ice__Process__writeMessage_name, ::Ice::Normal, __context);
    try
    {
        _DirectI __direct(message, fd, __current);
        try
        {
            __direct.getServant()->__collocDispatch(__direct);
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
    }
    catch(const ::Ice::SystemException&)
    {
        throw;
    }
    catch(const ::IceInternal::LocalExceptionWrapper&)
    {
        throw;
    }
    catch(const ::std::exception& __ex)
    {
        ::IceInternal::LocalExceptionWrapper::throwWrapper(__ex);
    }
    catch(...)
    {
        throw ::IceInternal::LocalExceptionWrapper(::Ice::UnknownException(__FILE__, __LINE__, "unknown c++ exception"), false);
    }
}

ICE_DECLSPEC_EXPORT ::Ice::Object* Ice::upCast(::Ice::Process* p) { return p; }

namespace
{
const ::std::string __Ice__Process_ids[2] =
{
    "::Ice::Object",
    "::Ice::Process"
};

}

bool
Ice::Process::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__Ice__Process_ids, __Ice__Process_ids + 2, _s);
}

::std::vector< ::std::string>
Ice::Process::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__Ice__Process_ids[0], &__Ice__Process_ids[2]);
}

const ::std::string&
Ice::Process::ice_id(const ::Ice::Current&) const
{
    return __Ice__Process_ids[1];
}

const ::std::string&
Ice::Process::ice_staticId()
{
    return __Ice__Process_ids[1];
}

::Ice::DispatchStatus
Ice::Process::___shutdown(::IceInternal::Incoming& __inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    __inS.readEmptyParams();
    shutdown(__current);
    __inS.__writeEmptyParams();
    return ::Ice::DispatchOK;
}

::Ice::DispatchStatus
Ice::Process::___writeMessage(::IceInternal::Incoming& __inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.startReadParams();
    ::std::string message;
    ::Ice::Int fd;
    __is->read(message);
    __is->read(fd);
    __inS.endReadParams();
    writeMessage(message, fd, __current);
    __inS.__writeEmptyParams();
    return ::Ice::DispatchOK;
}

namespace
{
const ::std::string __Ice__Process_all[] =
{
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "shutdown",
    "writeMessage"
};

}

::Ice::DispatchStatus
Ice::Process::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< const ::std::string*, const ::std::string*> r = ::std::equal_range(__Ice__Process_all, __Ice__Process_all + 6, current.operation);
    if(r.first == r.second)
    {
        throw ::Ice::OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }

    switch(r.first - __Ice__Process_all)
    {
        case 0:
        {
            return ___ice_id(in, current);
        }
        case 1:
        {
            return ___ice_ids(in, current);
        }
        case 2:
        {
            return ___ice_isA(in, current);
        }
        case 3:
        {
            return ___ice_ping(in, current);
        }
        case 4:
        {
            return ___shutdown(in, current);
        }
        case 5:
        {
            return ___writeMessage(in, current);
        }
    }

    assert(false);
    throw ::Ice::OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
}

void
Ice::Process::__writeImpl(::IceInternal::BasicStream* __os) const
{
    __os->startWriteSlice(ice_staticId(), -1, true);
    __os->endWriteSlice();
}

void
Ice::Process::__readImpl(::IceInternal::BasicStream* __is)
{
    __is->startReadSlice();
    __is->endReadSlice();
}

void ICE_DECLSPEC_EXPORT 
Ice::__patch(ProcessPtr& handle, const ::Ice::ObjectPtr& v)
{
    handle = ::Ice::ProcessPtr::dynamicCast(v);
    if(v && !handle)
    {
        IceInternal::Ex::throwUOE(::Ice::Process::ice_staticId(), v);
    }
}