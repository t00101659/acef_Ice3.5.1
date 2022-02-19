// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/ClientBlobject.h>
#include <Glacier2/FilterManager.h>
#include <Glacier2/FilterI.h>
#include <Glacier2/RoutingTable.h>

#include "../Ice/Chaos_IceCommon.hpp"

using namespace std;
using namespace Ice;
using namespace Glacier2;
using namespace CRS;

Glacier2::ClientBlobject::ClientBlobject(const InstancePtr& instance,
                                         const FilterManagerPtr& filters,
                                         const Ice::Context& sslContext,
                                         const RoutingTablePtr& routingTable):
                                         
    Glacier2::Blobject(instance, 0, sslContext),
    _routingTable(routingTable),
    _filters(filters),
    _rejectTraceLevel(_instance->properties()->getPropertyAsInt("Glacier2.Client.Trace.Reject"))
{
}

Glacier2::ClientBlobject::~ClientBlobject()
{
}

void
Glacier2::ClientBlobject::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& amdCB, 
                                           const std::pair<const Byte*, const Byte*>& inParams,
                                           const Current& current)
{
    bool matched = false;
    bool hasFilters = false;
    string rejectedFilters;
 
    if(!_filters->categories()->empty())
    {
        hasFilters = true;
        if(_filters->categories()->match(current.id.category))
        {
            matched = true;
        }
        else if(_rejectTraceLevel >= 1)
        {
            if(rejectedFilters.size() != 0)
            {
                rejectedFilters += ", ";

            }
            rejectedFilters += "category filter";
        }
    }

    if(!_filters->identities()->empty())
    {
        hasFilters = true;
        std::vector<Ice::Identity> vecTmp = _filters->identities()->get(current);
        LOG(CSystemLog::LOG_DEBUG, "[ClientBlobject::ice_invoke_async] [%d].", (int)vecTmp.size());
        if (vecTmp.size() > 0)
        {
            LOG(CSystemLog::LOG_DEBUG, "[ClientBlobject::ice_invoke_async] [%s/%s].", vecTmp[0].name.c_str(), 
                vecTmp[0].category.c_str());
        }
        if(_filters->identities()->match(current.id))
        {
            matched = true;
        }
        else if(_rejectTraceLevel >= 1)
        {
            if(rejectedFilters.size() != 0)
            {
                rejectedFilters += ", ";

            }
            rejectedFilters += "identity filter";
        }
    }

    ObjectPrx proxy = _routingTable->get(current.id);
    if(!proxy)
    {
		Ice::IPConnectionInfoPtr cf = Ice::IPConnectionInfoPtr::dynamicCast(current.con->getInfo());

		OUTPUT_LOG_B(0, "routingTable get NULL proxy," 
			<< " remote:[" << (cf ? cf->remoteAddress : "HOST") << ":" << (cf ? cf->remotePort : -1) << "] "
			<< " local:[" << (cf ? cf->localAddress : "HOST") << ":" << (cf ? cf->localPort : -1) << "] "
			<< " current.con-P:" << current.con.get() << " current.id:" << current.id.name
			<< " table-P:" << _routingTable.get());

        ObjectNotExistException ex(__FILE__, __LINE__);

        //
        // We use a special operation name indicate to the client that
        // the proxy for the Ice object has not been found in our
        // routing table. This can happen if the proxy was evicted
        // from the routing table.
        //
        ex.id = current.id;
        ex.facet = current.facet;
        ex.operation = "ice_add_proxy";
        throw ex;
    }
	else
	{
		// OUTPUT_LOG_B(0, "routingTable get valid proxy[" << proxy->ice_toString() << "], table-P:" << _routingTable.get() << " current.id:" << current.id.name);
	}

    string adapterId = proxy->ice_getAdapterId();

    if(!adapterId.empty() && !_filters->adapterIds()->empty())
    {
        hasFilters = true;
        if(_filters->adapterIds()->match(adapterId))
        {
            matched = true;
        }
        else if(_rejectTraceLevel >= 1)
        {
            if(rejectedFilters.size() != 0)
            {
                rejectedFilters += ", ";

            }
            rejectedFilters += "adapter id filter";
        }
    }

    if(hasFilters && !matched)
    {
        if(_rejectTraceLevel >= 1)
        {
            Trace out(_instance->logger(), "Glacier2");
            out << "rejecting request: " << rejectedFilters << "\n";
            out << "identity: " << _instance->communicator()->identityToString(current.id);
        }
        LOG(CSystemLog::LOG_ERROR, "[ClientBlobject::ice_invoke_async] rejecting request:%s identity:%s.",
            rejectedFilters.c_str(), _instance->communicator()->identityToString(current.id).c_str());
        ObjectNotExistException ex(__FILE__, __LINE__);
        ex.id = current.id;
        throw ex;
    }
    invoke(proxy, amdCB, inParams, current);
}

StringSetPtr 
ClientBlobject::categories()
{
    return _filters->categories();
}

StringSetPtr 
ClientBlobject::adapterIds()
{
    return _filters->adapterIds();
}

IdentitySetPtr
ClientBlobject::identities()
{
    return _filters->identities();
}
