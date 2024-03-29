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
// Generated from file `Metrics.ice'
//
// Warning: do not edit this file.
//
// </auto-generated>
//

#ifndef ICE_STORM_LIB_API_EXPORTS
#   define ICE_STORM_LIB_API_EXPORTS
#endif
#include <IceStorm/Metrics.h>
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

}
#ifdef __SUNPRO_CC
class ICE_DECLSPEC_EXPORT IceProxy::IceMX::TopicMetrics;
#endif
ICE_DECLSPEC_EXPORT ::IceProxy::Ice::Object* ::IceProxy::IceMX::upCast(::IceProxy::IceMX::TopicMetrics* p) { return p; }

void
::IceProxy::IceMX::__read(::IceInternal::BasicStream* __is, ::IceInternal::ProxyHandle< ::IceProxy::IceMX::TopicMetrics>& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::IceMX::TopicMetrics;
        v->__copyFrom(proxy);
    }
}

const ::std::string&
IceProxy::IceMX::TopicMetrics::ice_staticId()
{
    return ::IceMX::TopicMetrics::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::IceMX::TopicMetrics::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::IceMX::TopicMetrics);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::IceMX::TopicMetrics::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::IceMX::TopicMetrics);
}

::IceProxy::Ice::Object*
IceProxy::IceMX::TopicMetrics::__newInstance() const
{
    return new TopicMetrics;
}
#ifdef __SUNPRO_CC
class ICE_DECLSPEC_EXPORT IceProxy::IceMX::SubscriberMetrics;
#endif
ICE_DECLSPEC_EXPORT ::IceProxy::Ice::Object* ::IceProxy::IceMX::upCast(::IceProxy::IceMX::SubscriberMetrics* p) { return p; }

void
::IceProxy::IceMX::__read(::IceInternal::BasicStream* __is, ::IceInternal::ProxyHandle< ::IceProxy::IceMX::SubscriberMetrics>& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::IceMX::SubscriberMetrics;
        v->__copyFrom(proxy);
    }
}

const ::std::string&
IceProxy::IceMX::SubscriberMetrics::ice_staticId()
{
    return ::IceMX::SubscriberMetrics::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::IceMX::SubscriberMetrics::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::IceMX::SubscriberMetrics);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::IceMX::SubscriberMetrics::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::IceMX::SubscriberMetrics);
}

::IceProxy::Ice::Object*
IceProxy::IceMX::SubscriberMetrics::__newInstance() const
{
    return new SubscriberMetrics;
}

ICE_DECLSPEC_EXPORT ::Ice::Object* IceMX::upCast(::IceMX::TopicMetrics* p) { return p; }
::Ice::ObjectPtr
IceMX::TopicMetrics::ice_clone() const
{
    ::Ice::Object* __p = new TopicMetrics(*this);
    return __p;
}

namespace
{
const ::std::string __IceMX__TopicMetrics_ids[3] =
{
    "::Ice::Object",
    "::IceMX::Metrics",
    "::IceMX::TopicMetrics"
};

}

bool
IceMX::TopicMetrics::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__IceMX__TopicMetrics_ids, __IceMX__TopicMetrics_ids + 3, _s);
}

::std::vector< ::std::string>
IceMX::TopicMetrics::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__IceMX__TopicMetrics_ids[0], &__IceMX__TopicMetrics_ids[3]);
}

const ::std::string&
IceMX::TopicMetrics::ice_id(const ::Ice::Current&) const
{
    return __IceMX__TopicMetrics_ids[2];
}

const ::std::string&
IceMX::TopicMetrics::ice_staticId()
{
    return __IceMX__TopicMetrics_ids[2];
}

void
IceMX::TopicMetrics::__writeImpl(::IceInternal::BasicStream* __os) const
{
    __os->startWriteSlice(ice_staticId(), -1, false);
    __os->write(published);
    __os->write(forwarded);
    __os->endWriteSlice();
    ::IceMX::Metrics::__writeImpl(__os);
}

void
IceMX::TopicMetrics::__readImpl(::IceInternal::BasicStream* __is)
{
    __is->startReadSlice();
    __is->read(published);
    __is->read(forwarded);
    __is->endReadSlice();
    ::IceMX::Metrics::__readImpl(__is);
}

namespace
{

class __F__IceMX__TopicMetrics : public ::Ice::ObjectFactory
{
public:
#ifndef NDEBUG
virtual ::Ice::ObjectPtr
    create(const ::std::string& type)
#else
virtual ::Ice::ObjectPtr
    create(const ::std::string&)
#endif
    {
        assert(type == ::IceMX::TopicMetrics::ice_staticId());
        return new ::IceMX::TopicMetrics;
    }

    virtual void
    destroy()
    {
    }
};
const ::Ice::ObjectFactoryPtr __F__IceMX__TopicMetrics_Ptr = new __F__IceMX__TopicMetrics;

class __F__IceMX__TopicMetrics__Init
{
public:

    __F__IceMX__TopicMetrics__Init()
    {
        ::IceInternal::factoryTable->addObjectFactory(::IceMX::TopicMetrics::ice_staticId(), __F__IceMX__TopicMetrics_Ptr);
    }

    ~__F__IceMX__TopicMetrics__Init()
    {
        ::IceInternal::factoryTable->removeObjectFactory(::IceMX::TopicMetrics::ice_staticId());
    }
};

const __F__IceMX__TopicMetrics__Init __F__IceMX__TopicMetrics__i;

}

const ::Ice::ObjectFactoryPtr&
IceMX::TopicMetrics::ice_factory()
{
    return __F__IceMX__TopicMetrics_Ptr;
}

void ICE_DECLSPEC_EXPORT 
IceMX::__patch(TopicMetricsPtr& handle, const ::Ice::ObjectPtr& v)
{
    handle = ::IceMX::TopicMetricsPtr::dynamicCast(v);
    if(v && !handle)
    {
        IceInternal::Ex::throwUOE(::IceMX::TopicMetrics::ice_staticId(), v);
    }
}

ICE_DECLSPEC_EXPORT ::Ice::Object* IceMX::upCast(::IceMX::SubscriberMetrics* p) { return p; }
::Ice::ObjectPtr
IceMX::SubscriberMetrics::ice_clone() const
{
    ::Ice::Object* __p = new SubscriberMetrics(*this);
    return __p;
}

namespace
{
const ::std::string __IceMX__SubscriberMetrics_ids[3] =
{
    "::Ice::Object",
    "::IceMX::Metrics",
    "::IceMX::SubscriberMetrics"
};

}

bool
IceMX::SubscriberMetrics::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__IceMX__SubscriberMetrics_ids, __IceMX__SubscriberMetrics_ids + 3, _s);
}

::std::vector< ::std::string>
IceMX::SubscriberMetrics::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__IceMX__SubscriberMetrics_ids[0], &__IceMX__SubscriberMetrics_ids[3]);
}

const ::std::string&
IceMX::SubscriberMetrics::ice_id(const ::Ice::Current&) const
{
    return __IceMX__SubscriberMetrics_ids[2];
}

const ::std::string&
IceMX::SubscriberMetrics::ice_staticId()
{
    return __IceMX__SubscriberMetrics_ids[2];
}

void
IceMX::SubscriberMetrics::__writeImpl(::IceInternal::BasicStream* __os) const
{
    __os->startWriteSlice(ice_staticId(), -1, false);
    __os->write(queued);
    __os->write(outstanding);
    __os->write(delivered);
    __os->endWriteSlice();
    ::IceMX::Metrics::__writeImpl(__os);
}

void
IceMX::SubscriberMetrics::__readImpl(::IceInternal::BasicStream* __is)
{
    __is->startReadSlice();
    __is->read(queued);
    __is->read(outstanding);
    __is->read(delivered);
    __is->endReadSlice();
    ::IceMX::Metrics::__readImpl(__is);
}

namespace
{

class __F__IceMX__SubscriberMetrics : public ::Ice::ObjectFactory
{
public:
#ifndef NDEBUG
virtual ::Ice::ObjectPtr
    create(const ::std::string& type)
#else
virtual ::Ice::ObjectPtr
    create(const ::std::string&)
#endif
    {
        assert(type == ::IceMX::SubscriberMetrics::ice_staticId());
        return new ::IceMX::SubscriberMetrics;
    }

    virtual void
    destroy()
    {
    }
};
const ::Ice::ObjectFactoryPtr __F__IceMX__SubscriberMetrics_Ptr = new __F__IceMX__SubscriberMetrics;

class __F__IceMX__SubscriberMetrics__Init
{
public:

    __F__IceMX__SubscriberMetrics__Init()
    {
        ::IceInternal::factoryTable->addObjectFactory(::IceMX::SubscriberMetrics::ice_staticId(), __F__IceMX__SubscriberMetrics_Ptr);
    }

    ~__F__IceMX__SubscriberMetrics__Init()
    {
        ::IceInternal::factoryTable->removeObjectFactory(::IceMX::SubscriberMetrics::ice_staticId());
    }
};

const __F__IceMX__SubscriberMetrics__Init __F__IceMX__SubscriberMetrics__i;

}

const ::Ice::ObjectFactoryPtr&
IceMX::SubscriberMetrics::ice_factory()
{
    return __F__IceMX__SubscriberMetrics_Ptr;
}

void ICE_DECLSPEC_EXPORT 
IceMX::__patch(SubscriberMetricsPtr& handle, const ::Ice::ObjectPtr& v)
{
    handle = ::IceMX::SubscriberMetricsPtr::dynamicCast(v);
    if(v && !handle)
    {
        IceInternal::Ex::throwUOE(::IceMX::SubscriberMetrics::ice_staticId(), v);
    }
}

namespace
{

const char* __sliceChecksums[] =
{
    "::IceMX::SubscriberMetrics", "ab5eddbb2d0449f94b4808b6cf92552",
    "::IceMX::TopicMetrics", "afc516f773371c41f4f612d9e9629c",
    0
};
const IceInternal::SliceChecksumInit __sliceChecksumInit(__sliceChecksums);

}
