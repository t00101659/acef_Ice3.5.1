// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/TcpTransceiver.h>
#include <Ice/Connection.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Buffer.h>
#include <Ice/LocalException.h>

#include <IceUtil/DisableWarnings.h>
#include <Ice/Stats.h>

#include "Chaos_IceCommon.hpp"

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace CRS;

NativeInfoPtr
IceInternal::TcpTransceiver::getNativeInfo()
{
    return this;
}

#ifdef ICE_USE_IOCP
AsyncInfo*
IceInternal::TcpTransceiver::getAsyncInfo(SocketOperation status)
{
    switch(status)
    {
    case SocketOperationRead:
        return &_read;
    case SocketOperationWrite:
        return &_write;
    default:
        assert(false);
        return 0;
    }
}
#endif

SocketOperation
IceInternal::TcpTransceiver::initialize(Buffer& readBuffer, Buffer& writeBuffer)
{
	LOG(CSystemLog::LOG_SYSTEM, "[TcpTransceiver::initialize] come into initialize, _state=[%d], remoteaddr=[%s]", 
        _state, addrToString(_addr).c_str());
    try
    {
        if(_state == StateNeedConnect)
        {
            _state = StateConnectPending;
            return SocketOperationConnect;
        }
        else if(_state <= StateConnectPending)
        {
#ifdef ICE_USE_IOCP
            doFinishConnectAsync(_fd, _write);
#else
            doFinishConnect(_fd);
#endif

            _desc = fdToString(_fd, _proxy, _addr, true);

            if(_proxy)
            {
                //
                // Prepare the read & write buffers in advance.
                //
                _proxy->beginWriteConnectRequest(_addr, writeBuffer);
                _proxy->beginReadConnectRequestResponse(readBuffer);

#ifdef ICE_USE_IOCP
                //
                // Return SocketOperationWrite to indicate we need to start a write.
                //
                _state = StateProxyConnectRequest; // Send proxy connect request
                return IceInternal::SocketOperationWrite;
#else
                //
                // Write the proxy connection message.
                //
                if(write(writeBuffer))
                {
                    //
                    // Write completed without blocking.
                    //
                    _proxy->endWriteConnectRequest(writeBuffer);

                    //
                    // Try to read the response.
                    //
                    if(read(readBuffer))
                    {
                        //
                        // Read completed without blocking - fall through.
                        //
                        _proxy->endReadConnectRequestResponse(readBuffer);
                    }
                    else
                    {
                        //
                        // Return SocketOperationRead to indicate we need to complete the read.
                        //
                        _state = StateProxyConnectRequestPending; // Wait for proxy response
                        return SocketOperationRead;
                    }
                }
                else
                {
                    //
                    // Return SocketOperationWrite to indicate we need to complete the write.
                    //
                    _state = StateProxyConnectRequest; // Send proxy connect request
                    return SocketOperationWrite;
                }
#endif
            }

            _state = StateConnected;
        }
        else if(_state == StateProxyConnectRequest)
        {
            //
            // Write completed.
            //
            _proxy->endWriteConnectRequest(writeBuffer);
            _state = StateProxyConnectRequestPending; // Wait for proxy response
            return SocketOperationRead;
        }
        else if(_state == StateProxyConnectRequestPending)
        {
            //
            // Read completed.
            //
            _proxy->endReadConnectRequestResponse(readBuffer);
            _state = StateConnected;
        }
    }
    catch(const Ice::LocalException& ex)
    {
        if(_traceLevels->network >= 2)
        {
            Trace out(_logger, _traceLevels->networkCat);
            out << "failed to establish tcp connection\n" << fdToString(_fd, _proxy, _addr, false) << "\n" << ex;
        }
        LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::initialize] failed to establish tcp connection:%s,error:%s.",
            fdToString(_fd, _proxy, _addr, false).c_str(), ex.what());
        throw;
    }

    assert(_state == StateConnected);
    if(_traceLevels->network >= 1)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "tcp connection established\n" << _desc;
    }
    LOG(CSystemLog::LOG_INFO, "[TcpTransceiver::initialize] tcp connection established:%s.",
        _desc.c_str());
    return SocketOperationNone;
}

void
IceInternal::TcpTransceiver::close()
{
    //
    // If the transceiver is not connected, its description is simply "not connected",
    // which isn't very helpful.
    //
    if(_state == StateConnected && _traceLevels->network >= 1)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "closing tcp connection\n" << toString();
    }
    LOG(CSystemLog::LOG_INFO, "[TcpTransceiver::close] Closing tcp connection:[socket=%d,addr=%s].", 
        _fd, toString().c_str());

    assert(_fd != INVALID_SOCKET);
    try
    {
        closeSocket(_fd);
        LOG(CSystemLog::LOG_INFO, "[TcpTransceiver::close] close socket successfully.");
        _fd = INVALID_SOCKET;
    }
    catch(const SocketException&)
    {
        _fd = INVALID_SOCKET;
        throw;
    }
}

bool
IceInternal::TcpTransceiver::write(Buffer& buf)
{
    //
    // It's impossible for packetSize to be more than an Int.
    //
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
#ifdef ICE_USE_IOCP
    //
    // Limit packet size to avoid performance problems on WIN32
    //
    if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize)
    {
        packetSize = _maxSendPacketSize;
    }
#endif
    while(buf.i != buf.b.end())
    {
        assert(_fd != INVALID_SOCKET);

        ssize_t ret = ::send(_fd, reinterpret_cast<const char*>(&*buf.i), packetSize, 0);
        if(ret == 0)
        {
            LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::write] Connection[%s] lost exception.",
                    _desc.c_str());
            ConnectionLostException ex(__FILE__, __LINE__);
            ex.error = 0;
            throw ex;
        }

        if(ret == SOCKET_ERROR)
        {
            if(interrupted())
            {
                continue;
            }

            if(noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
                continue;
            }

            if(wouldBlock())
            {
                return false;
            }

            if(connectionLost())
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::write] Connection[%s] lost exception:%d.",
                    _desc.c_str(), ex.error);
                throw ex;
            }
            else
            {
                SocketException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::write] Socket[%s] exception:%d.",
                    _desc.c_str(), ex.error);
                throw ex;
            }
        }

        if(_traceLevels->network >= 3)
        {
            Trace out(_logger, _traceLevels->networkCat);
            out << "sent " << ret << " of " << packetSize << " bytes via tcp\n" << toString();
        }

        if(_stats)
        {
            _stats->bytesSent(type(), static_cast<Int>(ret));
        }

        buf.i += ret;

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = static_cast<int>(buf.b.end() - buf.i);
        }
    }

    return true;
}

bool
IceInternal::TcpTransceiver::read(Buffer& buf)
{
    //
    // It's impossible for packetSize to be more than an Int.
    //
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    while(buf.i != buf.b.end())
    {
        assert(_fd != INVALID_SOCKET);
        ssize_t ret = ::recv(_fd, reinterpret_cast<char*>(&*buf.i), packetSize, 0);

        if(ret == 0)
        {
            LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::read] Connection[%s] lost exception.",
                    _desc.c_str());
            ConnectionLostException ex(__FILE__, __LINE__);
            ex.error = 0;
            throw ex;
        }

        if(ret == SOCKET_ERROR)
        {
            if(interrupted())
            {
                continue;
            }

            if(noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
                continue;
            }

            if(wouldBlock())
            {
                return false;
            }

            if(connectionLost())
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::read] Connection[%s] lost exception:%d.",
                    _desc.c_str(), ex.error);
                throw ex;
            }
            else
            {
                SocketException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::read] Socket[%s] exception:%d.",
                    _desc.c_str(), ex.error);
                throw ex;
            }
        }

        if(_traceLevels->network >= 3)
        {
            Trace out(_logger, _traceLevels->networkCat);
            out << "received " << ret << " of " << packetSize << " bytes via tcp\n" << toString();
        }

        if(_stats)
        {
            _stats->bytesReceived(type(), static_cast<Int>(ret));
        }

        buf.i += ret;

        packetSize = static_cast<int>(buf.b.end() - buf.i);
    }
    return true;
}

#ifdef ICE_USE_IOCP
bool
IceInternal::TcpTransceiver::startWrite(Buffer& buf)
{
    if(_state == StateConnectPending)
    {
        Address addr = _proxy ? _proxy->getAddress() : _addr;

		//string tgtIPStr = IceInternal::addrToString(_addr);
		OUTPUT_LOG_A(0, "## ConnectAsync, WriteIO Ptr + ReadIO Ptr -> Address :[W:" << reinterpret_cast<int>(&(_write)) << " + R:" << reinterpret_cast<int>(&(_read)) << "]->[" << IceInternal::addrToString(addr) << "]");

        doConnectAsync(_fd, addr, _write);
        return false;
    }

    assert(!buf.b.empty());
    assert(buf.i != buf.b.end());

    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize)
    {
        packetSize = _maxSendPacketSize;
    }
    assert(packetSize > 0);
    _write.buf.len = packetSize;
    _write.buf.buf = reinterpret_cast<char*>(&*buf.i);
    int err = WSASend(_fd, &_write.buf, 1, &_write.count, 0, &_write, NULL);
    if(err == SOCKET_ERROR)
    {
        if(!wouldBlock())
        {
            if(connectionLost())
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::startWrite] Connection[%s] lost exception:%d.",
                    _desc.c_str(), ex.error);
                throw ex;
            }
            else
            {
                SocketException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::startWrite] Socket[%s] exception.",
                    _desc.c_str());
                throw ex;
            }
        }
    }
    return packetSize == static_cast<int>(buf.b.end() - buf.i);
}

void
IceInternal::TcpTransceiver::finishWrite(Buffer& buf)
{
    if(_state < StateConnected && _state != StateProxyConnectRequest)
    {
        return;
    }

    if(static_cast<int>(_write.count) == SOCKET_ERROR)
    {
        WSASetLastError(_write.error);
        if(connectionLost())
        {
            ConnectionLostException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::finishWrite] Connection[%s] lost exception:%d.",
                    _desc.c_str(), ex.error);
            throw ex;
        }
        else
        {
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::finishWrite] Socket[%s] exception:%d.",
                    _desc.c_str(), ex.error);
            throw ex;
        }
    }

    if(_traceLevels->network >= 3)
    {
        int packetSize = static_cast<int>(buf.b.end() - buf.i);
        if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize)
        {
            packetSize = _maxSendPacketSize;
        }
        Trace out(_logger, _traceLevels->networkCat);

        out << "sent " << _write.count << " of " << packetSize << " bytes via tcp\n" << toString();
    }

    if(_stats)
    {
        _stats->bytesSent(type(), _write.count);
    }

    buf.i += _write.count;
}

void
IceInternal::TcpTransceiver::startRead(Buffer& buf)
{
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    if(_maxReceivePacketSize > 0 && packetSize > _maxReceivePacketSize)
    {
        packetSize = _maxReceivePacketSize;
    }
    assert(!buf.b.empty() && buf.i != buf.b.end());

    _read.buf.len = packetSize;
    _read.buf.buf = reinterpret_cast<char*>(&*buf.i);
    int err = WSARecv(_fd, &_read.buf, 1, &_read.count, &_read.flags, &_read, NULL);

	if (err != 0)
	{
		int lastErr = WSAGetLastError();
		if (lastErr != WSA_IO_PENDING)
		{
			//OUTPUT_LOG_A(1, "IOCP in startRead Recv LastError is not pending:" << err << "  LastError:" << lastErr);
            LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::startRead] IOCP in startRead Recv LastError is not pending:" \
                "%d LastError:%d.", err, lastErr);
		}
	}

    if(err == SOCKET_ERROR)
    {
        if(!wouldBlock())
        {
            if(connectionLost())
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::startRead] Connection[%s] lost exception:%d.",
                    _desc.c_str(), ex.error);
                throw ex;
            }
            else
            {
                SocketException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::startRead] Socket[%s] exception.",
                    _desc.c_str());
                throw ex;
            }
        }
    }
}

void
IceInternal::TcpTransceiver::finishRead(Buffer& buf)
{
	if ( static_cast<int>(_read.count) == SOCKET_ERROR
		|| _read.count <= 0)
	{
		/*
		OUTPUT_LOG_A(0, "!!ConnectLost, WriteIO Ptr + ReadIO Ptr:[W: " << reinterpret_cast<int>(&(_write)) << " + R: " << reinterpret_cast<int>(&(_read)) << "]"
			<< " read.count: " << _read.count << " read.error: " << _read.error << " read.status: " << _read.status
			<< " read.flags: " << _read.flags << " read.Offset: " << _read.Offset << " read.Internal: " << _read.Internal << " LastError:" << WSAGetLastError() );
			*/
	}

    if(static_cast<int>(_read.count) == SOCKET_ERROR)
    {
        WSASetLastError(_read.error);
        if(connectionLost())
        {
            ConnectionLostException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::finishRead] Connection[%s] lost exception:%d.",
                    _desc.c_str(), ex.error);
            throw ex;
        }
        else
        {
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::finishRead] Socket[%s] exception.",
                    _desc.c_str());
            throw ex;
        }
    }
    else if(_read.count == 0)
    {
        ConnectionLostException ex(__FILE__, __LINE__);
        ex.error = 0;
        LOG(CSystemLog::LOG_ERROR, "[TcpTransceiver::finishRead] Connection[%s] lost exception:%d.",
                    _desc.c_str(), ex.error);
        throw ex;
    }

    if(_traceLevels->network >= 3)
    {
        int packetSize = static_cast<int>(buf.b.end() - buf.i);
        if(_maxReceivePacketSize > 0 && packetSize > _maxReceivePacketSize)
        {
            packetSize = _maxReceivePacketSize;
        }
        Trace out(_logger, _traceLevels->networkCat);
        out << "received " << _read.count << " of " << packetSize << " bytes via tcp\n" << toString();
    }

    if(_stats)
    {
        _stats->bytesReceived(type(), static_cast<Int>(_read.count));
    }

    buf.i += _read.count;
}
#endif

string
IceInternal::TcpTransceiver::type() const
{
    return "tcp";
}

string
IceInternal::TcpTransceiver::toString() const
{
    return _desc;
}

Ice::ConnectionInfoPtr
IceInternal::TcpTransceiver::getInfo() const
{
    Ice::TCPConnectionInfoPtr info = new Ice::TCPConnectionInfo();
    fdToAddressAndPort(_fd, info->localAddress, info->localPort, info->remoteAddress, info->remotePort);
    return info;
}

void
IceInternal::TcpTransceiver::checkSendSize(const Buffer& buf, size_t messageSizeMax)
{
    if(buf.b.size() > messageSizeMax)
    {
        LOG(CSystemLog::LOG_ERROR, "[IceInternal::TcpTransceiver::checkSendSize] Requested size:%d" \
            "bytes, maximum allowed is %d bytes (see Ice.MessageSizeMax)." \
            , (int)buf.b.size(), (int)messageSizeMax);
        Ex::throwMemoryLimitException(__FILE__, __LINE__, buf.b.size(), messageSizeMax);
    }
}

IceInternal::TcpTransceiver::TcpTransceiver(const InstancePtr& instance, SOCKET fd, const NetworkProxyPtr& proxy,
                                            const Address& addr) :
    NativeInfo(fd),
    _proxy(proxy),
    _addr(addr),
    _traceLevels(instance->traceLevels()),
    _logger(instance->initializationData().logger),
    _stats(instance->initializationData().stats),
    _state(StateNeedConnect)
#ifdef ICE_USE_IOCP
    , _read(SocketOperationRead),
    _write(SocketOperationWrite)
#endif
{
    setBlock(_fd, false);

    setTcpBufSize(_fd, instance->initializationData().properties, _logger);

#ifdef ICE_USE_IOCP
    //
    // On Windows, limiting the buffer size is important to prevent
    // poor throughput performances when transfering large amount of
    // data. See Microsoft KB article KB823764.
    //
    _maxSendPacketSize = IceInternal::getSendBufferSize(fd) / 2;
    if(_maxSendPacketSize < 512)
    {
        _maxSendPacketSize = 0;
    }

    _maxReceivePacketSize = IceInternal::getRecvBufferSize(fd);
    if(_maxReceivePacketSize < 512)
    {
        _maxReceivePacketSize = 0;
    }
    LOG(CSystemLog::LOG_SYSTEM, "[IceInternal::TcpTransceiver] _maxSendPacketSize=%d,_maxReceivePacketSize=%d.",
        _maxSendPacketSize, _maxReceivePacketSize);
#endif
}

IceInternal::TcpTransceiver::TcpTransceiver(const InstancePtr& instance, SOCKET fd) :
    NativeInfo(fd),
    _addr(Address()),
    _traceLevels(instance->traceLevels()),
    _logger(instance->initializationData().logger),
    _stats(instance->initializationData().stats),
    _state(StateConnected),
    _desc(fdToString(_fd))
#ifdef ICE_USE_IOCP
    , _read(SocketOperationRead),
    _write(SocketOperationWrite)
#endif
{
    setBlock(_fd, false);

    setTcpBufSize(_fd, instance->initializationData().properties, _logger);

#ifdef ICE_USE_IOCP
    //
    // On Windows, limiting the buffer size is important to prevent
    // poor throughput performances when transfering large amount of
    // data. See Microsoft KB article KB823764.
    //
    _maxSendPacketSize = IceInternal::getSendBufferSize(fd) / 2;
    if(_maxSendPacketSize < 512)
    {
        _maxSendPacketSize = 0;
    }

    _maxReceivePacketSize = IceInternal::getRecvBufferSize(fd);
    if(_maxReceivePacketSize < 512)
    {
        _maxReceivePacketSize = 0;
    }
    LOG(CSystemLog::LOG_SYSTEM, "[IceInternal::TcpTransceiver] _maxSendPacketSize=%d,_maxReceivePacketSize=%d,socket:%d,addr=%s.",
        _maxSendPacketSize, _maxReceivePacketSize, _fd, _desc.c_str());
#endif
}

IceInternal::TcpTransceiver::~TcpTransceiver()
{
    assert(_fd == INVALID_SOCKET);
}

void
IceInternal::TcpTransceiver::connect()
{
#ifndef ICE_USE_IOCP
    try
    {
        Address addr = _proxy ? _proxy->getAddress() : _addr;
        if(doConnect(_fd, addr))
        {
            _state = StateConnected;
            _desc = fdToString(_fd, _proxy, _addr, true);
            if(_traceLevels->network >= 1)
            {
                Trace out(_logger, _traceLevels->networkCat);
                out << "tcp connection established\n" << _desc;
            }
        }
        else
        {
            _desc = fdToString(_fd, _proxy, _addr, true);
        }
    }
    catch(...)
    {
        _fd = INVALID_SOCKET;
        throw;
    }
#endif
}
