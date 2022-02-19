// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ACM.h>
#include <Ice/ConnectionI.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Instance.h>
#include "Chaos_IceCommon.hpp"

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace CRS;

IceUtil::Shared* IceInternal::upCast(ACMMonitor* p) { return p; }
IceUtil::Shared* IceInternal::upCast(FactoryACMMonitor* p) { return p; }

IceInternal::ACMConfig::ACMConfig(bool server) :
    timeout(IceUtil::Time::seconds(60)),
    heartbeat(Ice::HeartbeatOnInvocation),
    close(server ? Ice::CloseOnInvocation : Ice::CloseOnInvocationAndIdle)
{
}

IceInternal::ACMConfig::ACMConfig(const Ice::PropertiesPtr& p,
                                  const Ice::LoggerPtr& l,
                                  const string& prefix,
                                  const ACMConfig& dflt)
{
    string timeoutProperty;
    if((prefix == "Ice.ACM.Client" || prefix == "Ice.ACM.Server") && p->getProperty(prefix + ".Timeout").empty())
    {
        timeoutProperty = prefix; // Deprecated property.
    }
    else
    {
        timeoutProperty = prefix + ".Timeout";
    };

    this->timeout = IceUtil::Time::seconds(p->getPropertyAsIntWithDefault(timeoutProperty,
                                                                          static_cast<int>(dflt.timeout.toSeconds())));
    LOG(CSystemLog::LOG_SYSTEM, "ACMConfig:%s:%d.", timeoutProperty.c_str(), static_cast<int>(this->timeout.toSeconds()));
    
    int hb = p->getPropertyAsIntWithDefault(prefix + ".Heartbeat", dflt.heartbeat);
    if(hb >= Ice::HeartbeatOff && hb <= Ice::HeartbeatAlways)
    {
        this->heartbeat = static_cast<Ice::ACMHeartbeat>(hb);
    }
    else
    {
        l->warning("invalid value for property `" + prefix + ".Heartbeat" + "', default value will be used instead");
        this->heartbeat = dflt.heartbeat;
        LOG(CSystemLog::LOG_WARNING, "invalid value for property %s..Heartbeat , default value will be used instead.",
            prefix.c_str());
    }
    LOG(CSystemLog::LOG_SYSTEM, "ACMConfig:Heartbeat:%d.", hb);

    int cl = p->getPropertyAsIntWithDefault(prefix + ".Close", dflt.close);
    if(cl >= Ice::CloseOff && cl <= Ice::CloseOnIdleForceful)
    {
        this->close = static_cast<Ice::ACMClose>(cl);
    }
    else
    {
        l->warning("invalid value for property `" + prefix + ".Close" + "', default value will be used instead");
        this->close = dflt.close;
        LOG(CSystemLog::LOG_WARNING, "invalid value for property %s..Close , default value will be used instead.",
            prefix.c_str());
    }
    LOG(CSystemLog::LOG_SYSTEM, "ACMConfig:Close:%d.", cl);
}

IceInternal::FactoryACMMonitor::FactoryACMMonitor(const InstancePtr& instance, const ACMConfig& config) :
    _instance(instance), _config(config)
{
}

IceInternal::FactoryACMMonitor::~FactoryACMMonitor()
{
    assert(!_instance);
    assert(_connections.empty());
    assert(_changes.empty());
    assert(_reapedConnections.empty());
}

void
IceInternal::FactoryACMMonitor::destroy()
{
    Lock sync(*this);
    if(!_instance)
    {
        //
        // Ensure all the connections have been cleared, it's important to wait here
        // to prevent the timer destruction in IceInternal::Instance::destroy.
        //
        while(!_connections.empty())
        {
            wait();
        }
        return;
    }

    //
    // Cancel the scheduled timer task and schedule it again now to clear the
    // connection set from the timer thread.
    //
    if(!_connections.empty())
    {
        _instance->timer()->cancel(this);
        _instance->timer()->schedule(this, IceUtil::Time());
    }

    _instance = 0;
    _changes.clear();

    //
    // Wait for the connection set to be cleared by the timer thread.
    //
    while(!_connections.empty())
    {
        wait();
    }
}

void
IceInternal::FactoryACMMonitor::add(const ConnectionIPtr& connection)
{
    if(_config.timeout == IceUtil::Time())
    {
        return;
    }

    Lock sync(*this);
    if(_connections.empty())
    {
        _connections.insert(connection);
        _instance->timer()->scheduleRepeated(this, _config.timeout / 2);
    }
    else
    {
        _changes.push_back(make_pair(connection, true));
    }
    LOG(CSystemLog::LOG_DEBUG, "[FactoryACMMonitor::add] add connection:%s,timeout:%d,close:%d,heartbeat:%d.", 
        connection->toString().c_str(), static_cast<int>(_config.timeout.toSeconds()), _config.close, _config.heartbeat);
}

void
IceInternal::FactoryACMMonitor::remove(const ConnectionIPtr& connection)
{
    if(_config.timeout == IceUtil::Time())
    {
        return;
    }

    Lock sync(*this);
    assert(_instance);
    _changes.push_back(make_pair(connection, false));
    LOG(CSystemLog::LOG_SYSTEM, "[FactoryACMMonitor::remove] connection:%s.",
        connection->toString().c_str());
}

void
IceInternal::FactoryACMMonitor::reap(const ConnectionIPtr& connection)
{
    Lock sync(*this);
    LOG(CSystemLog::LOG_DEBUG, "[FactoryACMMonitor::reap] Reap closed connection:%s.", connection->toString().c_str());
    _reapedConnections.push_back(connection);
}

ACMMonitorPtr
IceInternal::FactoryACMMonitor::acm(const IceUtil::Optional<int>& timeout,
                                    const IceUtil::Optional<Ice::ACMClose>& close,
                                    const IceUtil::Optional<Ice::ACMHeartbeat>& heartbeat)
{
    Lock sync(*this);
    assert(_instance);

    ACMConfig config(_config);
    if(timeout)
    {
        config.timeout = IceUtil::Time::seconds(*timeout);
    }
    if(close)
    {
        config.close = *close;
    }
    if(heartbeat)
    {
        config.heartbeat = *heartbeat;
    }
    LOG(CSystemLog::LOG_SYSTEM, "[FactoryACMMonitor::acm] acm connection timeout:%d,close:%d,heartbeat:%d.",
        static_cast<int>(config.timeout.toSeconds()), config.close, config.heartbeat);
    return new ConnectionACMMonitor(this, _instance->timer(), config);
}

Ice::ACM
IceInternal::FactoryACMMonitor::getACM()
{
    Ice::ACM acm;
    acm.timeout = static_cast<int>(_config.timeout.toSeconds());
    acm.close = _config.close;
    acm.heartbeat = _config.heartbeat;
    return acm;
}

void
IceInternal::FactoryACMMonitor::swapReapedConnections(vector<ConnectionIPtr>& connections)
{
    Lock sync(*this);
    _reapedConnections.swap(connections);
}

void
IceInternal::FactoryACMMonitor::runTimerTask()
{
    {
        Lock sync(*this);
        if(!_instance)
        {
            _connections.clear();
            notifyAll();
            return;
        }

        for(vector<pair<ConnectionIPtr, bool> >::const_iterator p = _changes.begin(); p != _changes.end(); ++p)
        {
            if(p->second)
            {
                _connections.insert(p->first);
            }
            else
            {
                _connections.erase(p->first);
            }
        }
        _changes.clear();

        if(_connections.empty())
        {
            _instance->timer()->cancel(this);
            return;
        }
    }

    //
    // Monitor connections outside the thread synchronization, so
    // that connections can be added or removed during monitoring.
    //
    IceUtil::Time now = IceUtil::Time::now(IceUtil::Time::Monotonic);
    for(set<ConnectionIPtr>::const_iterator p = _connections.begin(); p != _connections.end(); ++p)
    {
        try
        {
            (*p)->monitor(now, _config);
        }
        catch(const exception& ex)
        {
            handleException(ex);
        }
        catch(...)
        {
            handleException();
        }
    }
}

void
FactoryACMMonitor::handleException(const exception& ex)
{
    Lock sync(*this);
    if(!_instance)
    {
        return;
    }

    Error out(_instance->initializationData().logger);
    out << "exception in connection monitor:\n" << ex.what();
    LOG(CSystemLog::LOG_ERROR, "exception in connection monitor:%s.", ex.what());
}

void
FactoryACMMonitor::handleException()
{
    Lock sync(*this);
    if(!_instance)
    {
        return;
    }

    Error out(_instance->initializationData().logger);
    out << "unknown exception in connection monitor";
    LOG(CSystemLog::LOG_ERROR, "unknown exception in connection monitor.");
}

IceInternal::ConnectionACMMonitor::ConnectionACMMonitor(const FactoryACMMonitorPtr& parent,
                                                        const IceUtil::TimerPtr& timer,
                                                        const ACMConfig& config) :
    _parent(parent), _timer(timer), _config(config)
{
}

IceInternal::ConnectionACMMonitor::~ConnectionACMMonitor()
{
    assert(!_connection);
}

void
IceInternal::ConnectionACMMonitor::add(const ConnectionIPtr& connection)
{
    Lock sync(*this);
    assert(!_connection && connection);
    _connection = connection;
    if(_config.timeout != IceUtil::Time())
    {
        _timer->scheduleRepeated(this, _config.timeout / 2);
    }
    LOG(CSystemLog::LOG_SYSTEM, "[ConnectionACMMonitor::add] add connection:%s timeout:%d,close:%d,heartbeat:%d.",
        connection->toString().c_str(), static_cast<int>(_config.timeout.toSeconds()), _config.close, _config.heartbeat);
}

void
IceInternal::ConnectionACMMonitor::remove(const ConnectionIPtr& connection)
{
    Lock sync(*this);
    assert(_connection == connection);
    if(_config.timeout != IceUtil::Time())
    {
        _timer->cancel(this);
    }
    _connection = 0;
}

void
IceInternal::ConnectionACMMonitor::reap(const ConnectionIPtr& connection)
{
    _parent->reap(connection);
}

ACMMonitorPtr
IceInternal::ConnectionACMMonitor::acm(const IceUtil::Optional<int>& timeout,
                                       const IceUtil::Optional<Ice::ACMClose>& close,
                                       const IceUtil::Optional<Ice::ACMHeartbeat>& heartbeat)
{
    return _parent->acm(timeout, close, heartbeat);
}

Ice::ACM
IceInternal::ConnectionACMMonitor::getACM()
{
    Ice::ACM acm;
    acm.timeout = static_cast<int>(_config.timeout.toSeconds());
    acm.close = _config.close;
    acm.heartbeat = _config.heartbeat;
    return acm;
}

void
IceInternal::ConnectionACMMonitor::runTimerTask()
{
    Ice::ConnectionIPtr connection;
    {
        Lock sync(*this);
        if(!_connection)
        {
            return;
        }
        connection = _connection;
    }

    try
    {
        connection->monitor(IceUtil::Time::now(IceUtil::Time::Monotonic), _config);
    }
    catch(const exception& ex)
    {
        _parent->handleException(ex);
    }
    catch(...)
    {
        _parent->handleException();
    }
}
