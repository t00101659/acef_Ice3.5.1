// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/ReapThread.h>
#include "../Ice/Chaos_IceCommon.hpp"

using namespace std;
using namespace IceGrid;
using namespace CRS;

namespace
{

class ConnectionCallbackI : public Ice::ConnectionCallback
{
public:

    ConnectionCallbackI(const ReapThreadPtr& reaper) : _reaper(reaper)
    {
    }
    
    virtual void
    heartbeat(const Ice::ConnectionPtr& con)
    {
        //LOG(CSystemLog::LOG_DEBUG, "[ConnectionCallbackI:heartbeat] heartbeat with the connection[%s].",
        //        con->toString().c_str());
        _reaper->connectionHeartbeat(con);
    }

    virtual void
    closed(const Ice::ConnectionPtr& con)
    {
        LOG(CSystemLog::LOG_ERROR, "[ConnectionCallbackI:closed] closed with the connection[%s].",
                con->toString().c_str());
        _reaper->connectionClosed(con);
    }

private:

    const ReapThreadPtr _reaper;
};

}

ReapThread::ReapThread() :
    IceUtil::Thread("Icegrid reaper thread"),
    _callback(new ConnectionCallbackI(this)),
    _terminated(false)
{
}

void
ReapThread::run()
{
    vector<ReapableItem> reap;
    while(true)
    {
        {
            Lock sync(*this);
            if(_terminated)
            {
                break;
            }

            calcWakeInterval();

            //
            // If the wake interval is zero then we wait forever.
            //
            if(_wakeInterval == IceUtil::Time())
            {
                wait();
            }
            else
            {
                timedWait(_wakeInterval);
            }
            
            if(_terminated)
            {
                break;
            }

            list<ReapableItem>::iterator p = _sessions.begin();
            while(p != _sessions.end())
            {
                try
                {
                    if(p->timeout == IceUtil::Time())
                    {
                        p->item->timestamp(); // This should throw if the reapable is destroyed.
                        ++p;
                        continue;
                    }
                    else if((IceUtil::Time::now(IceUtil::Time::Monotonic) - p->item->timestamp()) > p->timeout)
                    {
                        LOG(CSystemLog::LOG_ERROR, "[ReapThread::run] connection:%s is timeout for [%d]secs more than %d.",
                            p->connection->toString().c_str(), 
                            static_cast<int>((IceUtil::Time::now(IceUtil::Time::Monotonic) - p->item->timestamp()).toSeconds()),
                            static_cast<int>(p->timeout.toSeconds()));
                        reap.push_back(*p);
                    }
                    else
                    {
                        ++p;
                        continue;
                    }
                }
                catch(const Ice::ObjectNotExistException&)
                {
                }

                //
                // Remove the reapable
                //
                if(p->connection)
                {
                    map<Ice::ConnectionPtr, set<ReapablePtr> >::iterator q = _connections.find(p->connection);
                    if(q != _connections.end())
                    {
                        q->second.erase(p->item);
                        if(q->second.empty())
                        {
                            LOG(CSystemLog::LOG_ERROR, "[ReapThread::run] connection[%s] heartbeat is timeout:[%d]secs.",
                                p->connection->toString().c_str(), static_cast<int>(p->timeout.toSeconds()));
                            p->connection->setCallback(0);
                            _connections.erase(q);
                        }
                    }
                }
                p = _sessions.erase(p);
            }
        }

        //LOG(CSystemLog::LOG_DEBUG, "[ReapThread::run] reap size:%d", reap.size());
        for(vector<ReapableItem>::const_iterator p = reap.begin(); p != reap.end(); ++p)
        {
            LOG(CSystemLog::LOG_ERROR, "[ReapThread::run] destroy session with conn:%s",
                p->connection->toString().c_str());
            p->item->destroy(false);
        }
        reap.clear();
    }
}

void
ReapThread::terminate()
{
    list<ReapableItem> reap;
    {
        Lock sync(*this);
        if(_terminated)
        {
            assert(_sessions.empty());
            return;
        }
        _terminated = true;
        notify();
        reap.swap(_sessions);

        for(map<Ice::ConnectionPtr, set<ReapablePtr> >::iterator p = _connections.begin(); p != _connections.end(); ++p)
        {
            LOG(CSystemLog::LOG_DEBUG, "terminate connection:%s.", 
                p->first->toString().c_str());
            p->first->setCallback(0);
        }
        _connections.clear();
        _callback = 0;
    }

    for(list<ReapableItem>::iterator p = reap.begin(); p != reap.end(); ++p)
    {
        p->item->destroy(true);
    }
}

void
ReapThread::add(const ReapablePtr& reapable, int timeout, const Ice::ConnectionPtr& connection)
{
    Lock sync(*this);
    if(_terminated)
    {
        return;
    }

    //
    // NOTE: registering a reapable with a null timeout is allowed. The reapable is reaped
    // only when the reaper thread is shutdown.
    //

    //
    // 10 seconds is the minimum permissable timeout.
    //
    if(timeout > 0 && timeout < 10)
    {
        timeout = 10;
    }

    ReapableItem item;
    item.item = reapable;
    item.connection = connection;
    item.timeout = timeout == 0 ? IceUtil::Time() : IceUtil::Time::seconds(timeout);
    _sessions.push_back(item);

    if(connection)
    {
        map<Ice::ConnectionPtr, set<ReapablePtr> >::iterator p = _connections.find(connection);
        if(p == _connections.end())
        {
            p = _connections.insert(make_pair(connection, set<ReapablePtr>())).first;
            connection->setCallback(_callback);
            LOG(CSystemLog::LOG_DEBUG, "[ReapThread::add] set heartbeat callback to connection:%s,timeout:%d.",
                connection->toString().c_str(), timeout);
        }
        p->second.insert(reapable);
    }

    if(timeout > 0)
    {
        //
        // If there is a new minimum wake interval then wake the reaping
        // thread.
        //
        if(calcWakeInterval())
        {
            notify();
        }
        
        //
        // Since we just added a new session with a non null timeout there
        // must be a non-zero wakeInterval.
        //
        assert(_wakeInterval != IceUtil::Time());
    }
}

void 
ReapThread::connectionHeartbeat(const Ice::ConnectionPtr& con)
{
    Lock sync(*this);
    map<Ice::ConnectionPtr, set<ReapablePtr> >::const_iterator p = _connections.find(con);
    if(p == _connections.end())
    {
        LOG(CSystemLog::LOG_ERROR, "connectionHeartbeat Failed to find connection:%s.", 
            con->toString().c_str());
        con->setCallback(0);
        return;
    }

    for(set<ReapablePtr>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
    {
        (*q)->heartbeat();
    }
}

void
ReapThread::connectionClosed(const Ice::ConnectionPtr& con)
{
    Lock sync(*this);
    map<Ice::ConnectionPtr, set<ReapablePtr> >::iterator p = _connections.find(con);
    if(p == _connections.end())
    {
        LOG(CSystemLog::LOG_ERROR, "connectionClosed Failed to find connection:%s.", 
            con->toString().c_str());
        con->setCallback(0);
        return;
    }

    for(set<ReapablePtr>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
    {
        (*q)->destroy(false);
    }
    _connections.erase(p);
}
//
// Returns true if the calculated wake interval is less than the current wake 
// interval (or if the original wake interval was "forever").
//
bool
ReapThread::calcWakeInterval()
{
    // Re-calculate minimum timeout
    IceUtil::Time oldWakeInterval = _wakeInterval;
    IceUtil::Time minimum;
    bool first = true;
    for(list<ReapableItem>::const_iterator p = _sessions.begin(); p != _sessions.end(); ++p)
    {
        if(p->timeout != IceUtil::Time() && (first || p->timeout < minimum))
        {
            minimum = p->timeout;
            first = false;
        }
    }

    _wakeInterval = minimum;
    return oldWakeInterval == IceUtil::Time() || minimum < oldWakeInterval;
}
