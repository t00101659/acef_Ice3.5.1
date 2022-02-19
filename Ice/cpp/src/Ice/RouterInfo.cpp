// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/RouterInfo.h>
#include <Ice/Router.h>
#include <Ice/LocalException.h>
#include <Ice/Connection.h> // For ice_connection()->timeout().
#include <Ice/Functional.h>
#include <Ice/Reference.h>
#include <Ice/EndpointI.h>
#include "Chaos_IceCommon.hpp"

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace CRS;

IceUtil::Shared* IceInternal::upCast(RouterManager* p) { return p; }
IceUtil::Shared* IceInternal::upCast(RouterInfo* p) { return p; }

IceInternal::RouterManager::RouterManager() :
    _tableHint(_table.end())
{
}

void
IceInternal::RouterManager::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    for_each(_table.begin(), _table.end(), Ice::secondVoidMemFun<const RouterPrx, RouterInfo>(&RouterInfo::destroy));

    _table.clear();
    _tableHint = _table.end();
}

RouterInfoPtr
IceInternal::RouterManager::get(const RouterPrx& rtr)
{
    if(!rtr)
    {
        return 0;
    }

    RouterPrx router = RouterPrx::uncheckedCast(rtr->ice_router(0)); // The router cannot be routed.

    IceUtil::Mutex::Lock sync(*this);

    map<RouterPrx, RouterInfoPtr>::iterator p = _table.end();
    
    if(_tableHint != _table.end())
    {
        if(_tableHint->first == router)
        {
            p = _tableHint;
        }
    }
    
    if(p == _table.end())
    {
        p = _table.find(router);
    }

    if(p == _table.end())
    {
        _tableHint = _table.insert(_tableHint, pair<const RouterPrx, RouterInfoPtr>(router, new RouterInfo(router)));
    }
    else
    {
        _tableHint = p;
    }

    return _tableHint->second;
}

RouterInfoPtr
IceInternal::RouterManager::erase(const RouterPrx& rtr)
{
    RouterInfoPtr info;
    if(rtr)
    {
        RouterPrx router = RouterPrx::uncheckedCast(rtr->ice_router(0)); // The router cannot be routed.
        IceUtil::Mutex::Lock sync(*this);

        map<RouterPrx, RouterInfoPtr>::iterator p = _table.end();
        if(_tableHint != _table.end() && _tableHint->first == router)
        {
            p = _tableHint;
            _tableHint = _table.end();
        }
        
        if(p == _table.end())
        {
            p = _table.find(router);
        }
        
        if(p != _table.end())
        {
            info = p->second;
            _table.erase(p);
        }
    }

    return info;
}

IceInternal::RouterInfo::RouterInfo(const RouterPrx& router) :
    _router(router)
{
    assert(_router);
}

void
IceInternal::RouterInfo::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    _clientEndpoints.clear();
    _serverEndpoints.clear();
    _adapter = 0;

	OUTPUT_LOG_B(0, "destroy routerinfo, before _identities.clear size =" << _identities.size()  << " routerinfo ptr:" << this);

    _identities.clear();
}

bool
IceInternal::RouterInfo::operator==(const RouterInfo& rhs) const
{
    return _router == rhs._router;
}

bool
IceInternal::RouterInfo::operator!=(const RouterInfo& rhs) const
{
    return _router != rhs._router;
}

bool
IceInternal::RouterInfo::operator<(const RouterInfo& rhs) const
{
    return _router < rhs._router;
}

vector<EndpointIPtr>
IceInternal::RouterInfo::getClientEndpoints()
{
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!_clientEndpoints.empty())
        {
            return _clientEndpoints;
        }
    }

    return setClientEndpoints(_router->getClientProxy());
}

void
IceInternal::RouterInfo::getClientProxyResponse(const Ice::ObjectPrx& proxy, const GetClientEndpointsCallbackPtr& callback)
{
    callback->setEndpoints(setClientEndpoints(proxy));
}

void
IceInternal::RouterInfo::getClientProxyException(const Ice::Exception& ex, const GetClientEndpointsCallbackPtr& callback)
{
    if(dynamic_cast<const Ice::CollocationOptimizationException*>(&ex))
    {
        try
        {
            callback->setEndpoints(getClientEndpoints());
        }
        catch(const Ice::LocalException& e)
        {
            callback->setException(e);
        }
    }
    else
    {
        callback->setException(dynamic_cast<const Ice::LocalException&>(ex));
    }
}

void
IceInternal::RouterInfo::getClientEndpoints(const GetClientEndpointsCallbackPtr& callback)
{
    vector<EndpointIPtr> clientEndpoints;
    {
        IceUtil::Mutex::Lock sync(*this);
        clientEndpoints = _clientEndpoints;
    }

    if(!clientEndpoints.empty())
    {
        callback->setEndpoints(clientEndpoints);
        return;
    }

    _router->begin_getClientProxy(newCallback_Router_getClientProxy(this, 
                                                                    &RouterInfo::getClientProxyResponse, 
                                                                    &RouterInfo::getClientProxyException),
                                  callback);
}

vector<EndpointIPtr>
IceInternal::RouterInfo::getServerEndpoints()
{
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!_serverEndpoints.empty())
        {
            return _serverEndpoints;
        }
    }

    return setServerEndpoints(_router->getServerProxy());
}

void
IceInternal::RouterInfo::addProxy(const ObjectPrx& proxy)
{
	OUTPUT_LOG_B(0, "addProxy begin, proxy:[" << proxy->ice_toString() << "]");

    assert(proxy); // Must not be called for null proxies.

    {
        IceUtil::Mutex::Lock sync(*this);
        if(_identities.find(proxy->ice_getIdentity()) != _identities.end())
        {
            //
            // Only add the proxy to the router if it's not already in our local map.
            //

			OUTPUT_LOG_B(0, "the router is already in local map, proxy:[" << proxy->ice_toString() << "]");

            return;
        }
    }

    ObjectProxySeq proxies;
    proxies.push_back(proxy);

	OUTPUT_LOG_B(0, "addAndEvictProxies begin, proxy:[" << proxy->ice_toString() << "]");

    addAndEvictProxies(proxy, _router->addProxies(proxies));
}

void
IceInternal::RouterInfo::addProxyResponse(const Ice::ObjectProxySeq& proxies, const AddProxyCookiePtr& cookie)
{
    addAndEvictProxies(cookie->proxy(), proxies);
    cookie->cb()->addedProxy();
}

void
IceInternal::RouterInfo::addProxyException(const Ice::Exception& ex, const AddProxyCookiePtr& cookie)
{
    if(dynamic_cast<const Ice::CollocationOptimizationException*>(&ex))
    {
        try
        {
            addProxy(cookie->proxy());
            cookie->cb()->addedProxy();
        }
        catch(const Ice::LocalException& e)
        {
            cookie->cb()->setException(e);
        }
    }
    else
    {
        cookie->cb()->setException(dynamic_cast<const Ice::LocalException&>(ex));
    }
}

bool
IceInternal::RouterInfo::addProxy(const Ice::ObjectPrx& proxy, const AddProxyCallbackPtr& callback)
{
	OUTPUT_LOG_B(0, "AC addProxy begin, proxy:[" << proxy->ice_toString() << "]");

    assert(proxy);
    {
        IceUtil::Mutex::Lock sync(*this);
        if(_identities.find(proxy->ice_getIdentity()) != _identities.end())
        {
			OUTPUT_LOG_B(0, "AC the router is already in local map, proxy:[" << proxy->ice_toString() << "]");

            //
            // Only add the proxy to the router if it's not already in our local map.
            //
            return true;
        }
    }


    Ice::ObjectProxySeq proxies;
    proxies.push_back(proxy);
    AddProxyCookiePtr cookie = new AddProxyCookie(callback, proxy);

	OUTPUT_LOG_B(0, "begin_addProxies, proxy:[" << proxy->ice_toString() << "]");

    _router->begin_addProxies(proxies,
                              newCallback_Router_addProxies(this, 
                                                            &RouterInfo::addProxyResponse, 
                                                            &RouterInfo::addProxyException), 
                              cookie);
    LOG(CSystemLog::LOG_DEBUG, "[RouterInfo::addProxy] begin_addProxies, proxy:%s,router:%s.",
        proxy->ice_toString().c_str(), _router->ice_toString().c_str());
    return false;
}

void
IceInternal::RouterInfo::setAdapter(const ObjectAdapterPtr& adapter)
{
    IceUtil::Mutex::Lock sync(*this);
    _adapter = adapter;
}

ObjectAdapterPtr
IceInternal::RouterInfo::getAdapter() const
{
    IceUtil::Mutex::Lock sync(*this);
    return _adapter;
}

void
IceInternal::RouterInfo::clearCache(const ReferencePtr& ref)
{
    IceUtil::Mutex::Lock sync(*this);

	OUTPUT_LOG_B(0, "clearCache before _identities.erase size =" << _identities.size() << ", ref id:" << ref->getIdentity().name << " routerinfo ptr:" << this);
    _identities.erase(ref->getIdentity());
	OUTPUT_LOG_B(0, "clearCache after _identities.erase size =" << _identities.size() << ", ref id:" << ref->getIdentity().name << " routerinfo ptr:" << this);
}

vector<EndpointIPtr>
IceInternal::RouterInfo::setClientEndpoints(const Ice::ObjectPrx& proxy)
{
    IceUtil::Mutex::Lock sync(*this);
    if(_clientEndpoints.empty())
    {
        if(!proxy)
        {
            //
            // If getClientProxy() return nil, use router endpoints.
            //
            _clientEndpoints = _router->__reference()->getEndpoints();
            std::string toString;
            char szBuff[16];
            if (_clientEndpoints.size() > 0)
            {
                for (size_t i = 0; i < _clientEndpoints.size(); i++)
                {
                    EndpointI* pEp = _clientEndpoints[i].get();
                    toString += pEp->GetHost();
                    sprintf(szBuff, "%d;", pEp->GetPort());
                    toString += szBuff;
                }
            }
            LOG(CSystemLog::LOG_DEBUG, "[RouterInfo::setClientEndpoints] router proxy:%s,router endpoint:%s.",
                _router->ice_toString().c_str(), toString.c_str());
        }
        else
        {
            Ice::ObjectPrx clientProxy = proxy->ice_router(0); // The client proxy cannot be routed.

            //
            // In order to avoid creating a new connection to the router,
            // we must use the same timeout as the already existing
            // connection.
            //
            try
            {
                clientProxy = clientProxy->ice_timeout(_router->ice_getConnection()->timeout());
            }
            catch(const Ice::CollocationOptimizationException&)
            {
                // Ignore - collocated router
            }

            _clientEndpoints = clientProxy->__reference()->getEndpoints();
        }
    }
    return _clientEndpoints;
}


vector<EndpointIPtr>
IceInternal::RouterInfo::setServerEndpoints(const Ice::ObjectPrx& /*serverProxy*/)
{
    IceUtil::Mutex::Lock sync(*this);
    if(_serverEndpoints.empty()) // Lazy initialization.
    {
        ObjectPrx serverProxy = _router->getServerProxy();
        if(!serverProxy)
        {
            throw NoEndpointException(__FILE__, __LINE__);
        }

        serverProxy = serverProxy->ice_router(0); // The server proxy cannot be routed.

        _serverEndpoints = serverProxy->__reference()->getEndpoints();
    }
    return _serverEndpoints;
}

void
IceInternal::RouterInfo::addAndEvictProxies(const Ice::ObjectPrx& proxy, const Ice::ObjectProxySeq& evictedProxies)
{
    IceUtil::Mutex::Lock sync(*this);

    //
    // Check if the proxy hasn't already been evicted by a concurrent addProxies call. 
    // If it's the case, don't add it to our local map.
    //
    multiset<Identity>::iterator p = _evictedIdentities.find(proxy->ice_getIdentity());
    if(p != _evictedIdentities.end())
    {
        _evictedIdentities.erase(p);
    }
    else
    {
        //
        // If we successfully added the proxy to the router,
        // we add it to our local map.
        //
		OUTPUT_LOG_B(0, "before _identities.insert size =" << _identities.size() << ", proxy id:" << proxy->ice_getIdentity().name << " routerinfo ptr:" << this);

        _identities.insert(proxy->ice_getIdentity());

		OUTPUT_LOG_B(0, "after _identities.insert size =" << _identities.size() << ", proxy id:" << proxy->ice_getIdentity().name << " routerinfo ptr:" << this);
    }
    
    //
    // We also must remove whatever proxies the router evicted.
    //
    for(Ice::ObjectProxySeq::const_iterator q = evictedProxies.begin(); q != evictedProxies.end(); ++q)
    {
		OUTPUT_LOG_B(0, "before _identities.erase size =" << _identities.size() << ", proxy id:" << (*q)->ice_getIdentity().name << " routerinfo ptr:" << this);

        if(_identities.erase((*q)->ice_getIdentity()) == 0)
        {
            //
            // It's possible for the proxy to not have been
            // added yet in the local map if two threads
            // concurrently call addProxies.
            //
            _evictedIdentities.insert((*q)->ice_getIdentity());
        }

		OUTPUT_LOG_B(0, "after _identities.erase size =" << _identities.size() << ", proxy id:" << (*q)->ice_getIdentity().name << " routerinfo ptr:" << this);
    }
}
