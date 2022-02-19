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
// Generated from file `Instrumentation.ice'
//
// Warning: do not edit this file.
//
// </auto-generated>
//

#ifndef ICE_STORM_SERVICE_API_EXPORTS
#   define ICE_STORM_SERVICE_API_EXPORTS
#endif
#include <IceStorm/Instrumentation.h>
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

namespace
{

}

}

namespace Ice
{
}

ICE_DECLSPEC_EXPORT ::Ice::LocalObject* IceStorm::Instrumentation::upCast(::IceStorm::Instrumentation::TopicObserver* p) { return p; }

ICE_DECLSPEC_EXPORT ::Ice::LocalObject* IceStorm::Instrumentation::upCast(::IceStorm::Instrumentation::SubscriberObserver* p) { return p; }

ICE_DECLSPEC_EXPORT ::Ice::LocalObject* IceStorm::Instrumentation::upCast(::IceStorm::Instrumentation::ObserverUpdater* p) { return p; }

ICE_DECLSPEC_EXPORT ::Ice::LocalObject* IceStorm::Instrumentation::upCast(::IceStorm::Instrumentation::TopicManagerObserver* p) { return p; }
