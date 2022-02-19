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
// Generated from file `UserAccountMapper.ice'
//
// Warning: do not edit this file.
//
// </auto-generated>
//

#ifndef ICE_GRID_API_EXPORTS
#   define ICE_GRID_API_EXPORTS
#endif
#include <IceGrid/UserAccountMapper.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectFactory.h>
#include <Ice/BasicStream.h>
#include <Ice/Object.h>
#include <Ice/SliceChecksums.h>
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

const ::std::string __IceGrid__UserAccountMapper__getUserAccount_name = "getUserAccount";

}

namespace
{

const char* __IceGrid__UserAccountNotFoundException_name = "IceGrid::UserAccountNotFoundException";

struct __F__IceGrid__UserAccountNotFoundException : public ::IceInternal::UserExceptionFactory
{
    virtual void
    createAndThrow(const ::std::string&)
    {
        throw ::IceGrid::UserAccountNotFoundException();
    }
};

class __F__IceGrid__UserAccountNotFoundException__Init
{
public:

    __F__IceGrid__UserAccountNotFoundException__Init()
    {
        ::IceInternal::factoryTable->addExceptionFactory("::IceGrid::UserAccountNotFoundException", new __F__IceGrid__UserAccountNotFoundException);
    }

    ~__F__IceGrid__UserAccountNotFoundException__Init()
    {
        ::IceInternal::factoryTable->removeExceptionFactory("::IceGrid::UserAccountNotFoundException");
    }
};

const __F__IceGrid__UserAccountNotFoundException__Init __F__IceGrid__UserAccountNotFoundException__i;

}

IceGrid::UserAccountNotFoundException::~UserAccountNotFoundException() throw()
{
}

::std::string
IceGrid::UserAccountNotFoundException::ice_name() const
{
    return __IceGrid__UserAccountNotFoundException_name;
}

IceGrid::UserAccountNotFoundException*
IceGrid::UserAccountNotFoundException::ice_clone() const
{
    return new UserAccountNotFoundException(*this);
}

void
IceGrid::UserAccountNotFoundException::ice_throw() const
{
    throw *this;
}

void
IceGrid::UserAccountNotFoundException::__writeImpl(::IceInternal::BasicStream* __os) const
{
    __os->startWriteSlice("::IceGrid::UserAccountNotFoundException", -1, true);
    __os->endWriteSlice();
}

void
IceGrid::UserAccountNotFoundException::__readImpl(::IceInternal::BasicStream* __is)
{
    __is->startReadSlice();
    __is->endReadSlice();
}

namespace Ice
{
}
#ifdef __SUNPRO_CC
class ICE_DECLSPEC_EXPORT IceProxy::IceGrid::UserAccountMapper;
#endif
ICE_DECLSPEC_EXPORT ::IceProxy::Ice::Object* ::IceProxy::IceGrid::upCast(::IceProxy::IceGrid::UserAccountMapper* p) { return p; }

void
::IceProxy::IceGrid::__read(::IceInternal::BasicStream* __is, ::IceInternal::ProxyHandle< ::IceProxy::IceGrid::UserAccountMapper>& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::IceGrid::UserAccountMapper;
        v->__copyFrom(proxy);
    }
}

::std::string
IceProxy::IceGrid::UserAccountMapper::getUserAccount(const ::std::string& user, const ::Ice::Context* __ctx)
{
    ::IceInternal::InvocationObserver __observer(this, __IceGrid__UserAccountMapper__getUserAccount_name, __ctx);
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__UserAccountMapper__getUserAccount_name);
            __delBase = __getDelegate(false);
            ::IceDelegate::IceGrid::UserAccountMapper* __del = dynamic_cast< ::IceDelegate::IceGrid::UserAccountMapper*>(__delBase.get());
            return __del->getUserAccount(user, __ctx, __observer);
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
IceProxy::IceGrid::UserAccountMapper::begin_getUserAccount(const ::std::string& user, const ::Ice::Context* __ctx, const ::IceInternal::CallbackBasePtr& __del, const ::Ice::LocalObjectPtr& __cookie)
{
    __checkAsyncTwowayOnly(__IceGrid__UserAccountMapper__getUserAccount_name);
    ::IceInternal::OutgoingAsyncPtr __result = new ::IceInternal::OutgoingAsync(this, __IceGrid__UserAccountMapper__getUserAccount_name, __del, __cookie);
    try
    {
        __result->__prepare(__IceGrid__UserAccountMapper__getUserAccount_name, ::Ice::Normal, __ctx);
        ::IceInternal::BasicStream* __os = __result->__startWriteParams(::Ice::DefaultFormat);
        __os->write(user);
        __result->__endWriteParams();
        __result->__send(true);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __result->__exceptionAsync(__ex);
    }
    return __result;
}

::std::string
IceProxy::IceGrid::UserAccountMapper::end_getUserAccount(const ::Ice::AsyncResultPtr& __result)
{
    ::Ice::AsyncResult::__check(__result, this, __IceGrid__UserAccountMapper__getUserAccount_name);
    ::std::string __ret;
    bool __ok = __result->__wait();
    try
    {
        if(!__ok)
        {
            try
            {
                __result->__throwUserException();
            }
            catch(const ::IceGrid::UserAccountNotFoundException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceInternal::BasicStream* __is = __result->__startReadParams();
        __is->read(__ret);
        __result->__endReadParams();
        return __ret;
    }
    catch(const ::Ice::LocalException& ex)
    {
        __result->__getObserver().failed(ex.ice_name());
        throw;
    }
}

const ::std::string&
IceProxy::IceGrid::UserAccountMapper::ice_staticId()
{
    return ::IceGrid::UserAccountMapper::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::IceGrid::UserAccountMapper::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::IceGrid::UserAccountMapper);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::IceGrid::UserAccountMapper::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::IceGrid::UserAccountMapper);
}

::IceProxy::Ice::Object*
IceProxy::IceGrid::UserAccountMapper::__newInstance() const
{
    return new UserAccountMapper;
}

::std::string
IceDelegateM::IceGrid::UserAccountMapper::getUserAccount(const ::std::string& user, const ::Ice::Context* __context, ::IceInternal::InvocationObserver& __observer)
{
    ::IceInternal::Outgoing __og(__handler.get(), __IceGrid__UserAccountMapper__getUserAccount_name, ::Ice::Normal, __context, __observer);
    try
    {
        ::IceInternal::BasicStream* __os = __og.startWriteParams(::Ice::DefaultFormat);
        __os->write(user);
        __og.endWriteParams();
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    ::std::string __ret;
    try
    {
        if(!__ok)
        {
            try
            {
                __og.throwUserException();
            }
            catch(const ::IceGrid::UserAccountNotFoundException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                ::Ice::UnknownUserException __uue(__FILE__, __LINE__, __ex.ice_name());
                throw __uue;
            }
        }
        ::IceInternal::BasicStream* __is = __og.startReadParams();
        __is->read(__ret);
        __og.endReadParams();
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::std::string
IceDelegateD::IceGrid::UserAccountMapper::getUserAccount(const ::std::string& user, const ::Ice::Context* __context, ::IceInternal::InvocationObserver&)
{
    class _DirectI : public ::IceInternal::Direct
    {
    public:

        _DirectI(::std::string& __result, const ::std::string& __p_user, const ::Ice::Current& __current) : 
            ::IceInternal::Direct(__current),
            _result(__result),
            _m_user(__p_user)
        {
        }
        
        virtual ::Ice::DispatchStatus
        run(::Ice::Object* object)
        {
            ::IceGrid::UserAccountMapper* servant = dynamic_cast< ::IceGrid::UserAccountMapper*>(object);
            if(!servant)
            {
                throw ::Ice::OperationNotExistException(__FILE__, __LINE__, _current.id, _current.facet, _current.operation);
            }
            try
            {
                _result = servant->getUserAccount(_m_user, _current);
                return ::Ice::DispatchOK;
            }
            catch(const ::Ice::UserException& __ex)
            {
                setUserException(__ex);
                return ::Ice::DispatchUserException;
            }
        }
        
    private:
        
        ::std::string& _result;
        const ::std::string& _m_user;
    };
    
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__UserAccountMapper__getUserAccount_name, ::Ice::Normal, __context);
    ::std::string __result;
    try
    {
        _DirectI __direct(__result, user, __current);
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
    catch(const ::IceGrid::UserAccountNotFoundException&)
    {
        throw;
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
    return __result;
}

ICE_DECLSPEC_EXPORT ::Ice::Object* IceGrid::upCast(::IceGrid::UserAccountMapper* p) { return p; }

namespace
{
const ::std::string __IceGrid__UserAccountMapper_ids[2] =
{
    "::Ice::Object",
    "::IceGrid::UserAccountMapper"
};

}

bool
IceGrid::UserAccountMapper::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__IceGrid__UserAccountMapper_ids, __IceGrid__UserAccountMapper_ids + 2, _s);
}

::std::vector< ::std::string>
IceGrid::UserAccountMapper::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__IceGrid__UserAccountMapper_ids[0], &__IceGrid__UserAccountMapper_ids[2]);
}

const ::std::string&
IceGrid::UserAccountMapper::ice_id(const ::Ice::Current&) const
{
    return __IceGrid__UserAccountMapper_ids[1];
}

const ::std::string&
IceGrid::UserAccountMapper::ice_staticId()
{
    return __IceGrid__UserAccountMapper_ids[1];
}

::Ice::DispatchStatus
IceGrid::UserAccountMapper::___getUserAccount(::IceInternal::Incoming& __inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.startReadParams();
    ::std::string user;
    __is->read(user);
    __inS.endReadParams();
    try
    {
        ::std::string __ret = getUserAccount(user, __current);
        ::IceInternal::BasicStream* __os = __inS.__startWriteParams(::Ice::DefaultFormat);
        __os->write(__ret);
        __inS.__endWriteParams(true);
        return ::Ice::DispatchOK;
    }
    catch(const ::IceGrid::UserAccountNotFoundException& __ex)
    {
        __inS.__writeUserException(__ex, ::Ice::DefaultFormat);
    }
    return ::Ice::DispatchUserException;
}

namespace
{
const ::std::string __IceGrid__UserAccountMapper_all[] =
{
    "getUserAccount",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping"
};

}

::Ice::DispatchStatus
IceGrid::UserAccountMapper::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< const ::std::string*, const ::std::string*> r = ::std::equal_range(__IceGrid__UserAccountMapper_all, __IceGrid__UserAccountMapper_all + 5, current.operation);
    if(r.first == r.second)
    {
        throw ::Ice::OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }

    switch(r.first - __IceGrid__UserAccountMapper_all)
    {
        case 0:
        {
            return ___getUserAccount(in, current);
        }
        case 1:
        {
            return ___ice_id(in, current);
        }
        case 2:
        {
            return ___ice_ids(in, current);
        }
        case 3:
        {
            return ___ice_isA(in, current);
        }
        case 4:
        {
            return ___ice_ping(in, current);
        }
    }

    assert(false);
    throw ::Ice::OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
}

void
IceGrid::UserAccountMapper::__writeImpl(::IceInternal::BasicStream* __os) const
{
    __os->startWriteSlice(ice_staticId(), -1, true);
    __os->endWriteSlice();
}

void
IceGrid::UserAccountMapper::__readImpl(::IceInternal::BasicStream* __is)
{
    __is->startReadSlice();
    __is->endReadSlice();
}

void ICE_DECLSPEC_EXPORT 
IceGrid::__patch(UserAccountMapperPtr& handle, const ::Ice::ObjectPtr& v)
{
    handle = ::IceGrid::UserAccountMapperPtr::dynamicCast(v);
    if(v && !handle)
    {
        IceInternal::Ex::throwUOE(::IceGrid::UserAccountMapper::ice_staticId(), v);
    }
}

namespace
{

const char* __sliceChecksums[] =
{
    "::IceGrid::UserAccountMapper", "779fd561878e199444e04cdebaf9ffd4",
    "::IceGrid::UserAccountNotFoundException", "fe2dc4d87f21b9b2cf6f1339d1666281",
    0
};
const IceInternal::SliceChecksumInit __sliceChecksumInit(__sliceChecksums);

}