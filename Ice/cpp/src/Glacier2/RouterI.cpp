// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Random.h>

#include <Glacier2/FilterManager.h>
#include <Glacier2/RoutingTable.h>
#include <Glacier2/RouterI.h>
#include <Glacier2/Session.h>
#include <fstream>
#include "../Ice/Chaos_IceCommon.hpp"

using namespace std;
using namespace Ice;
using namespace Glacier2;
using namespace CRS;

  void WriteLog(std::string & strLog)
 {
	return;
		std::fstream _fs;
		strLog +="\r\n";
		_fs.open("d:\\glacier2.log", std::ios_base::out | std::ios_base::binary | std::ios_base::app);
				if (_fs.is_open())
				{
					//std::string strLog="erase category:"+category;
					_fs.seekp(0, std::ios_base::end);
					_fs.write((char*)strLog.c_str(), strLog.length()*sizeof(char));
					_fs.close();
				}
 }
Glacier2::RouterI::RouterI(const InstancePtr& instance, const ConnectionPtr& connection, const string& userId, 
                           const SessionPrx& session, const Identity& controlId, const FilterManagerPtr& filters,
                           const Ice::Context& context) :
    _instance(instance),
    _routingTable(new RoutingTable(_instance->communicator(), _instance->proxyVerifier())),
    _clientBlobject(new ClientBlobject(_instance, filters, context, _routingTable)),
    _clientBlobjectBuffered(_instance->clientRequestQueueThread()),
    _serverBlobjectBuffered(_instance->serverRequestQueueThread()),
    _connection(connection),
    _userId(userId),
    _session(session),
    _controlId(controlId),
    _context(context),
    _timestamp(IceUtil::Time::now(IceUtil::Time::Monotonic))
{

	Ice::IPConnectionInfoPtr cf = Ice::IPConnectionInfoPtr::dynamicCast(connection->getInfo());

	OUTPUT_LOG_B(0, "new RouterI,"
		<< " remote:[" << (cf ? cf->remoteAddress : "HOST") << ":" << (cf ? cf->remotePort : -1) << "] "
		<< " local:[" << (cf ? cf->localAddress : "HOST") << ":" << (cf ? cf->localPort : -1) << "] "
		<< " current.con-P:" << connection.get()
		<< " table-P:" << _routingTable.get());

    LOG(CSystemLog::LOG_SYSTEM, "[RouterI::RouterI] new RouterI remote:%s:%d, local:%s:%d,connection:%s.",
        cf->remoteAddress.c_str(), cf->remotePort, cf->localAddress.c_str(), cf->localPort, connection->toString().c_str());

    //
    // If Glacier2 will be used with pre 3.2 clients, then the client proxy must be set.
    // Otherwise getClientProxy just needs to return a nil proxy.
    //
    if(_instance->properties()->getPropertyAsInt("Glacier2.ReturnClientProxy") > 0)
    {
        const_cast<Ice::ObjectPrx&>(_clientProxy) = 
            _instance->clientObjectAdapter()->createProxy(_instance->communicator()->stringToIdentity("dummy"));
    }

    if(_instance->serverObjectAdapter())
    {
        ObjectPrx& serverProxy = const_cast<ObjectPrx&>(_serverProxy);
        Identity ident;
        ident.name = "dummy";
       /* ident.category.resize(20);
        char buf[20];
        IceUtilInternal::generateRandom(buf, static_cast<int>(sizeof(buf)));
        for(unsigned int i = 0; i < sizeof(buf); ++i)
        {
            const unsigned char c = static_cast<unsigned char>(buf[i]); // A value between 0-255
            ident.category[i] = 33 + c % (127-33); // We use ASCII 33-126 (from ! to ~, w/o space).
        }*/
		ident.category=userId;
        serverProxy = _instance->serverObjectAdapter()->createProxy(ident);

        ServerBlobjectPtr& serverBlobject = const_cast<ServerBlobjectPtr&>(_serverBlobject);
        serverBlobject = new ServerBlobject(_instance, _connection);
    }

    if(_instance->getObserver())
    {
        updateObserver(_instance->getObserver());
    }
}

Glacier2::RouterI::~RouterI()
{
}

void
Glacier2::RouterI::updateObserver(const Glacier2::Instrumentation::RouterObserverPtr& observer)
{
    // Can only be called with the SessionRouterI mutex locked

    _observer = _routingTable->updateObserver(observer, _userId, _connection);
    _clientBlobject->updateObserver(_observer);
    if(_serverBlobject)
    {
        _serverBlobject->updateObserver(_observer);
    }
}

void
Glacier2::RouterI::destroy(const Callback_Session_destroyPtr& asyncCB)
{
    LOG(CSystemLog::LOG_DEBUG, "[Glacier2::RouterI::destroy] destroy router session proxy:%s.",
        _session->ice_toString().c_str());
    if(_session)
    {
        if(_instance->serverObjectAdapter())
        {
            try
            {
                //
                // Remove the session control object.
                //
                _instance->serverObjectAdapter()->remove(_controlId);
            }
            catch(const NotRegisteredException&)
            {
            }
            catch(const ObjectAdapterDeactivatedException&)
            {
                //
                // Expected if the router has been shutdown.
                //
            }
        }

        if(_context.size() > 0)
        {
            _session->begin_destroy(_context, asyncCB);
        }
        else
        {
            _session->begin_destroy(asyncCB);
        }
    }

    _clientBlobject->destroy();
    if(_serverBlobject)
    {
        _serverBlobject->destroy();
    }

    _routingTable->destroy();
}

ObjectPrx
Glacier2::RouterI::getClientProxy(const Current&) const
{
    // No mutex lock necessary, _clientProxy is immutable and is never destroyed.
    return _clientProxy;
}

ObjectPrx
Glacier2::RouterI::getServerProxy(const Current&) const
{
    // No mutex lock necessary, _serverProxy is immutable and is never destroyed.
    return _serverProxy;
}

void
Glacier2::RouterI::addProxy(const ObjectPrx& proxy, const Current& current)
{
   std::string strLog ="addProxy :[" +proxy->ice_toString() +"] this[" + toString()+"]";
   WriteLog(strLog);
    LOG(CSystemLog::LOG_DEBUG, "[RouterI::addProxy] addProxy:%s ,this:%s.", proxy->ice_toString().c_str(),
        toString().c_str());
    ObjectProxySeq proxies;
    proxies.push_back(proxy);
    addProxies(proxies, current);
}

ObjectProxySeq
Glacier2::RouterI::addProxies(const ObjectProxySeq& proxies, const Current& current)
{
    return _routingTable->add(proxies, current);
}

string
Glacier2::RouterI::getCategoryForClient(const Ice::Current&) const
{
    assert(false); // Must not be called in this router implementation.
    return 0;
}

void
Glacier2::RouterI::createSession_async(const AMD_Router_createSessionPtr&, const std::string&, const std::string&,
                                       const Current&)
{
    assert(false); // Must not be called in this router implementation.
}

void
Glacier2::RouterI::createSessionFromSecureConnection_async(const AMD_Router_createSessionFromSecureConnectionPtr&,
                                                           const Current&)
{
    assert(false); // Must not be called in this router implementation.
}

void
Glacier2::RouterI::refreshSession_async(const AMD_Router_refreshSessionPtr&, const ::Ice::Current&)
{
    assert(false); // Must not be called in this router implementation.
}

void
Glacier2::RouterI::destroySession(const Current&)
{
    assert(false); // Must not be called in this router implementation.
}

Ice::Long
Glacier2::RouterI::getSessionTimeout(const Current&) const
{
    assert(false); // Must not be called in this router implementation.
    return 0;
}

Ice::Int
Glacier2::RouterI::getACMTimeout(const Current&) const
{
    assert(false); // Must not be called in this router implementation.
    return 0;
}

ClientBlobjectPtr
Glacier2::RouterI::getClientBlobject() const
{
    // Can only be called with the SessionRouterI mutex locked
    if(!_clientBlobjectBuffered && _observer)
    {
        _observer->forwarded(true);
    }
    return _clientBlobject;
}

ServerBlobjectPtr
Glacier2::RouterI::getServerBlobject() const
{
    // Can only be called with the SessionRouterI mutex locked
    if(!_serverBlobjectBuffered && _observer)
    {
        _observer->forwarded(false);
    }
    return _serverBlobject;
}

SessionPrx
Glacier2::RouterI::getSession() const
{
    return _session; // No mutex lock necessary, _session is immutable.
}

IceUtil::Time
Glacier2::RouterI::getTimestamp() const
{
    // Can only be called with the SessionRouterI mutex locked
    return _timestamp;
}

void
Glacier2::RouterI::updateTimestamp() const
{
    // Can only be called with the SessionRouterI mutex locked
    _timestamp = IceUtil::Time::now(IceUtil::Time::Monotonic);
}

string
Glacier2::RouterI::toString() const
{
    ostringstream out;

    out << "id = " << _userId << '\n';
    if(_serverProxy)
    {
        out << "category = " << _serverProxy->ice_getIdentity().category << '\n';
    }
    out << _connection->toString();

    return out.str();
}
