// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ObjectAdapterFactory.h>
#include <Ice/ObjectAdapterI.h>
#include <Ice/Object.h>
#include <Ice/LocalException.h>
#include <Ice/Functional.h>
#include <IceUtil/UUID.h>
#include "Chaos_IceCommon.hpp"

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace CRS;

IceUtil::Shared* IceInternal::upCast(ObjectAdapterFactory* p) { return p; }

void
IceInternal::ObjectAdapterFactory::shutdown()
{
    LOG(CSystemLog::LOG_SYSTEM, "[ObjectAdapterFactory::shutdown] Enter shutdown.");
    //CALL_STACK_OUTPUT;    
    list<ObjectAdapterIPtr> adapters;

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        
        //
        // Ignore shutdown requests if the object adapter factory has
        // already been shut down.
        //
        if(!_instance)
        {
            return;
        }
        
        adapters = _adapters;
        
        _instance = 0;
        _communicator = 0;
        
        notifyAll();
    }
    
    //
    // Deactivate outside the thread synchronization, to avoid
    // deadlocks.
    //
    for_each(adapters.begin(), adapters.end(), IceUtil::voidMemFun(&ObjectAdapter::deactivate));
    LOG(CSystemLog::LOG_SYSTEM, "[ObjectAdapterFactory::shutdown] Leave shutdown.");
}

void
IceInternal::ObjectAdapterFactory::waitForShutdown()
{
    list<ObjectAdapterIPtr> adapters;

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        
        //
        // First we wait for the shutdown of the factory itself.
        //
        LOG(CSystemLog::LOG_SYSTEM, "[ObjectAdapterFactory::waitForShutdown] We wait for the shutdown of the factory itself.");
        while(_instance)
        {
            wait();
        }
        LOG(CSystemLog::LOG_SYSTEM, "[ObjectAdapterFactory::waitForShutdown] We are shutdown.");
        adapters = _adapters;
    }

    //
    // Now we wait for deactivation of each object adapter.
    //
    for_each(adapters.begin(), adapters.end(), IceUtil::voidMemFun(&ObjectAdapter::waitForDeactivate));
}

bool
IceInternal::ObjectAdapterFactory::isShutdown() const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    return _instance == 0;
}

void
IceInternal::ObjectAdapterFactory::destroy()
{
    //
    // First wait for shutdown to finish.
    //
    waitForShutdown();

    list<ObjectAdapterIPtr> adapters;

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        adapters = _adapters;
    }

    //
    // Now we destroy each object adapter.
    //
    printf("Now we destroy each object adapter.\n");
    for_each(adapters.begin(), adapters.end(), IceUtil::voidMemFun(&ObjectAdapter::destroy));

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        _adapters.clear();
    }
}

void
IceInternal::ObjectAdapterFactory::updateObservers(void (ObjectAdapterI::*fn)())
{
    list<ObjectAdapterIPtr> adapters;

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        adapters = _adapters;
    }

    for_each(adapters.begin(), adapters.end(), IceUtil::voidMemFun(fn));
}

ObjectAdapterPtr
IceInternal::ObjectAdapterFactory::createObjectAdapter(const string& name, const RouterPrx& router)
{
    LOG(CSystemLog::LOG_INFO, "[IceInternal::ObjectAdapterFactory::createObjectAdapter] Enter createObjectAdapter.");
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    if(!_instance)
    {
        throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    ObjectAdapterIPtr adapter;
    if(name.empty())
    {
        string uuid = IceUtil::generateUUID();
        adapter = new ObjectAdapterI(_instance, _communicator, this, uuid, true);
        adapter->initialize(0);
    }
    else
    {
        if(_adapterNamesInUse.find(name) != _adapterNamesInUse.end())
        {
            throw AlreadyRegisteredException(__FILE__, __LINE__, "object adapter", name);
        }
        adapter = new ObjectAdapterI(_instance, _communicator, this, name, false);
        adapter->initialize(router);
        _adapterNamesInUse.insert(name);
        LOG(CSystemLog::LOG_INFO, "[IceInternal::ObjectAdapterFactory::createObjectAdapter] Create object adapter:%s " \
            "successfully!", name.c_str());
    }

    _adapters.push_back(adapter);
    LOG(CSystemLog::LOG_INFO, "[IceInternal::ObjectAdapterFactory::createObjectAdapter] Leave createObjectAdapter.");
    return adapter;
}

ObjectAdapterPtr
IceInternal::ObjectAdapterFactory::findObjectAdapter(const ObjectPrx& proxy)
{
    list<ObjectAdapterIPtr> adapters;
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        if(!_instance)
        {
            return 0;
        }
        
        adapters = _adapters;
    }

    for(list<ObjectAdapterIPtr>::iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
        try
        {
            if((*p)->isLocal(proxy))
            {
                return *p;
            }
        }
        catch(const ObjectAdapterDeactivatedException&)
        {
            // Ignore.
        }
    }

    return 0;
}

void
IceInternal::ObjectAdapterFactory::removeObjectAdapter(const ObjectAdapterPtr& adapter)
{
    LOG(CSystemLog::LOG_INFO, "[IceInternal::ObjectAdapterFactory::removeObjectAdapter] Enter removeObjectAdapter.");
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    if(!_instance)
    {
        return;
    }

    for(list<ObjectAdapterIPtr>::iterator p = _adapters.begin(); p != _adapters.end(); ++p)
    {
        if(*p == adapter)
        {
            const EndpointSeq eseq = adapter->getPublishedEndpoints();
            for (size_t i = 0; i != eseq.size(); i++)
            {
                LOG(CSystemLog::LOG_INFO, "[IceInternal::ObjectAdapterFactory::removeObjectAdapter] Remove ObjectAdapter Endpoint:%s.", 
                    eseq[i]->toString().c_str());
            }
            _adapters.erase(p);
            LOG(CSystemLog::LOG_INFO, "[IceInternal::ObjectAdapterFactory::removeObjectAdapter] Remove ObjectAdapter %s successfully.",
                adapter->getName().c_str());
            break;
        }
    }
    _adapterNamesInUse.erase(adapter->getName());
    LOG(CSystemLog::LOG_INFO, "[IceInternal::ObjectAdapterFactory::removeObjectAdapter] Leave removeObjectAdapter.");
}

void
IceInternal::ObjectAdapterFactory::flushAsyncBatchRequests(const CommunicatorBatchOutgoingAsyncPtr& outAsync) const
{
    list<ObjectAdapterIPtr> adapters;
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        adapters = _adapters;
    }

    for(list<ObjectAdapterIPtr>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
        (*p)->flushAsyncBatchRequests(outAsync);
    }
}

IceInternal::ObjectAdapterFactory::ObjectAdapterFactory(const InstancePtr& instance,
                                                        const CommunicatorPtr& communicator) :
    _instance(instance),
    _communicator(communicator)
{
}

IceInternal::ObjectAdapterFactory::~ObjectAdapterFactory()
{
    assert(!_instance);
    assert(!_communicator);
    assert(_adapters.empty());
}
