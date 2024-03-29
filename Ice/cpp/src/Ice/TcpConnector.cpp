// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/TcpConnector.h>
#include <Ice/TcpTransceiver.h>
#include <Ice/TcpEndpointI.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Network.h>
#include <Ice/Exception.h>
#include "Chaos_IceCommon.hpp"

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace CRS;

TransceiverPtr
IceInternal::TcpConnector::connect()
{
    if(_traceLevels->network >= 2)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "trying to establish tcp connection to " << toString();
    }
    LOG(CSystemLog::LOG_INFO, "[TcpConnector::connect] Trying to establish tcp connection to %s.",
        toString().c_str());

    SOCKET fd = INVALID_SOCKET;
    try
    {
        fd = createSocket(false, _addr);
        TransceiverPtr transceiver = new TcpTransceiver(_instance, fd, _proxy, _addr);
        dynamic_cast<TcpTransceiver*>(transceiver.get())->connect();
        LOG(CSystemLog::LOG_INFO, "[TcpConnector::connect] Connect to address[socket=%d,addr=%s] successfully.",
            fd, toString().c_str());
        return transceiver;
    }
    catch(const Ice::LocalException& ex)
    {
        if(_traceLevels->network >= 2)
        {
            Trace out(_logger, _traceLevels->networkCat);
            out << "failed to establish tcp connection to " << toString() << "\n" << ex;
        }
        LOG(CSystemLog::LOG_ERROR, "[IceInternal::TcpConnector::connect] Failed to establish tcp connection to %s, error:%s.",
            toString().c_str(), ex.what());
        throw;
    }
}

Short
IceInternal::TcpConnector::type() const
{
    return TCPEndpointType;
}

string
IceInternal::TcpConnector::toString() const
{
    return addrToString(!_proxy ? _addr : _proxy->getAddress());
}

bool
IceInternal::TcpConnector::operator==(const Connector& r) const
{
    const TcpConnector* p = dynamic_cast<const TcpConnector*>(&r);
    if(!p)
    {
        return false;
    }

    if(compareAddress(_addr, p->_addr) != 0)
    {
        return false;
    }
    
    if(_timeout != p->_timeout)
    {
        return false;
    }

    if(_connectionId != p->_connectionId)
    {
        return false;
    }

    return true;
}

bool
IceInternal::TcpConnector::operator!=(const Connector& r) const
{
    return !operator==(r);
}

bool
IceInternal::TcpConnector::operator<(const Connector& r) const
{
    const TcpConnector* p = dynamic_cast<const TcpConnector*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(_timeout < p->_timeout)
    {
        return true;
    }
    else if(p->_timeout < _timeout)
    {
        return false;
    }

    if(_connectionId < p->_connectionId)
    {
        return true;
    }
    else if(p->_connectionId < _connectionId)
    {
        return false;
    }
    return compareAddress(_addr, p->_addr) < 0;
}

IceInternal::TcpConnector::TcpConnector(const InstancePtr& instance, const Address& addr, const NetworkProxyPtr& proxy,
                                        Ice::Int timeout, const string& connectionId) :
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->initializationData().logger),
    _addr(addr),
    _proxy(proxy),
    _timeout(timeout),
    _connectionId(connectionId)
{
}

IceInternal::TcpConnector::~TcpConnector()
{
}
