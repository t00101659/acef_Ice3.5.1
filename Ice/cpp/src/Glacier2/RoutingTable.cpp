// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/RoutingTable.h>
#include <Glacier2/Instrumentation.h>
#include <fstream>
#include "../Ice/Chaos_IceCommon.hpp"

using namespace std;
using namespace Ice;
using namespace Glacier2;
using namespace CRS;

  void WriteLog(std::string & strLog);

Glacier2::RoutingTable::RoutingTable(const CommunicatorPtr& communicator, const ProxyVerifierPtr& verifier) :
    _communicator(communicator),
    _traceLevel(_communicator->getProperties()->getPropertyAsInt("Glacier2.Trace.RoutingTable")),
    _maxSize(_communicator->getProperties()->getPropertyAsIntWithDefault("Glacier2.RoutingTable.MaxSize", 1000)),
    _verifier(verifier)
{
}

void
Glacier2::RoutingTable::destroy()
{
    IceUtil::Mutex::Lock sync(*this);
    if(_observer)
    {
        _observer->routingTableSize(-static_cast<Ice::Int>(_map.size()));
    }
    _observer.detach();
}

Glacier2::Instrumentation::SessionObserverPtr
Glacier2::RoutingTable::updateObserver(const Glacier2::Instrumentation::RouterObserverPtr& obsv,
                                       const string& userId,
                                       const Ice::ConnectionPtr& connection)
{
    IceUtil::Mutex::Lock sync(*this);
    _observer.attach(obsv->getSessionObserver(userId, connection, static_cast<Ice::Int>(_map.size()), _observer.get()));
    return _observer.get();
}

ObjectProxySeq
Glacier2::RoutingTable::add(const ObjectProxySeq& unfiltered, const Current& current)
{
	Ice::IPConnectionInfoPtr cf = Ice::IPConnectionInfoPtr::dynamicCast(current.con->getInfo());

	/*OUTPUT_LOG_B(0, "RoutingTable add proxy begin,"
		   << " remote:[" << (cf? cf->remoteAddress: "HOST") << ":" << (cf? cf->remotePort: -1) << "] "
		   << " local:[" << (cf? cf->localAddress: "HOST") << ":" << (cf? cf->localPort: -1) << "] "
		   << " current.con-P:" << current.con.get() << " current.id:" << current.id.name 
		   << " table-P:" << this );*/

    //LOG(CSystemLog::LOG_DEBUG, "[RoutingTable::add] add proxy from remote:%s:%d, local:%s:%d,connection:%s.",
    //    cf->remoteAddress.c_str(), cf->remotePort, cf->localAddress.c_str(), cf->localPort, 
    //    _communicator->identityToString(current.id).c_str());
    IceUtil::Mutex::Lock sync(*this);

    size_t sz = _map.size();

    //
    // We 'pre-scan' the list, applying our validation rules. The
    // ensures that our state is not modified if this operation results
    // in a rejection.
    //
    ObjectProxySeq proxies; 
    for(ObjectProxySeq::const_iterator prx = unfiltered.begin(); prx != unfiltered.end(); ++prx)
    {
        if(!*prx) // We ignore null proxies.
        {
            continue; 
        }

        if(!_verifier->verify(*prx))
        {
            current.con->close(true);
			
			//OUTPUT_LOG_B(0, "Throw not exist exception when add proxy[" << (*prx)->ice_toString() << "]");
            LOG(CSystemLog::LOG_ERROR, "[RoutingTable::add] Failed to verify proxy:%s.", (*prx)->ice_toString().c_str());
            
            throw ObjectNotExistException(__FILE__, __LINE__);
        }
        ObjectPrx proxy = (*prx)->ice_twoway()->ice_secure(false)->ice_facet(""); // We add proxies in default form.
        proxies.push_back(proxy);

		//OUTPUT_LOG_B(0, "add proxy to proxies size = " << proxies.size() << ", proxy:[" << proxy->ice_toString() << "]");
        //LOG(CSystemLog::LOG_DEBUG, "[RoutingTable::add] proxies size :%d, new proxy added:%s.", proxies.size(),
        //    proxy->ice_toString().c_str());
        //CALL_STACK_OUTPUT;
    }

    ObjectProxySeq evictedProxies;
    for(ObjectProxySeq::const_iterator prx = proxies.begin(); prx != proxies.end(); ++prx)
    {
        ObjectPrx proxy = *prx;
        EvictorMap::iterator p = _map.find(proxy->ice_getIdentity());
        
        if(p == _map.end())
        {
            if(_traceLevel == 1 || _traceLevel >= 3)
            {
                Trace out(_communicator->getLogger(), "Glacier2");
                out << "adding proxy to routing table:\n" << _communicator->proxyToString(proxy);
            }
            //LOG(CSystemLog::LOG_DEBUG, "[RoutingTable::add] add proxy:%s to router table.", 
            //    _communicator->proxyToString(proxy).c_str());
			//OUTPUT_LOG_B(0, "Adding proxy[" << _communicator->proxyToString(proxy) << "] to routing table-P:[" << this << "]");
            
            EvictorEntryPtr entry = new EvictorEntry;
            p = _map.insert(_map.begin(), pair<const Identity, EvictorEntryPtr>(proxy->ice_getIdentity(), entry));
            EvictorQueue::iterator q = _queue.insert(_queue.end(), p);
            entry->proxy = proxy;
            entry->pos = q;
        }
        else
        {
            if(_traceLevel == 1 || _traceLevel >= 3)
            {
                Trace out(_communicator->getLogger(), "Glacier2");
                out << "proxy already in routing table:\n" << _communicator->proxyToString(proxy);
            }

			//OUTPUT_LOG_B(0, "Duplicate! proxy[" << _communicator->proxyToString(proxy) << "] already in routing table-P:[" << this << "]");
            LOG(CSystemLog::LOG_ERROR, "[RoutingTable::add] proxy already in routing table:%s.", 
                _communicator->proxyToString(proxy).c_str());
            EvictorEntryPtr entry = p->second;
            _queue.erase(entry->pos);
            EvictorQueue::iterator q = _queue.insert(_queue.end(), p);
            entry->pos = q;
        }
        
        while(static_cast<int>(_map.size()) > _maxSize)
        {
            p = _queue.front();
            
            if(_traceLevel >= 2)
            {
                Trace out(_communicator->getLogger(), "Glacier2");
                out << "evicting proxy from routing table:\n" << _communicator->proxyToString(p->second->proxy);
            }

			//OUTPUT_LOG_B(0, "_map.size()=" << _map.size() << "! Evicting proxy[" << _communicator->proxyToString(p->second->proxy) << "] from routing table-P:[" << this << "]");
            LOG(CSystemLog::LOG_ERROR, "[RoutingTable::add] evicting proxy from routing table:%s.",
                _communicator->proxyToString(p->second->proxy).c_str());
            evictedProxies.push_back(p->second->proxy);

            _map.erase(p);
            _queue.pop_front();
        }
    }

    if(_observer)
    {
        _observer->routingTableSize(static_cast<Ice::Int>(_map.size()) - static_cast<Ice::Int>(sz));
    }

    return evictedProxies;
}

ObjectPrx
Glacier2::RoutingTable::get(const Identity& ident)
{
    if(ident.name.empty())
    {
        return 0;
    }

    IceUtil::Mutex::Lock sync(*this);

    EvictorMap::iterator p = _map.find(ident);

    if(p == _map.end())
    {
        return 0;
    }
    else
    {
        EvictorEntryPtr entry = p->second;
        _queue.erase(entry->pos);
        EvictorQueue::iterator q = _queue.insert(_queue.end(), p);
        entry->pos = q;

        //LOG(CSystemLog::LOG_DEBUG, "[RoutingTable::get] get proxy:%s, which Identity:%s:%s.",
        //    entry->proxy->ice_toString().c_str(), ident.name.c_str(), ident.category.c_str());

        return entry->proxy;
    }
}
