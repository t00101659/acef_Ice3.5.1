// FileName         : Chaos_ConnectionTest.icpp.cpp
// Date of Creation : 2017-09-13
// Description      : 
// Author		        : GuanChao
//____________________________________________________________________

#include <Ice/Network.h>
#include <Ice/EventHandler.h>
#include <Ice/Selector.h>

#include "Chaos_IceCommon.hpp"

#ifdef   OS_WIN

using namespace std;
using namespace Ice;
using namespace IceInternal;


// CT = connection test

namespace CRS
{

struct CTAsyncInfo : public ::IceInternal::AsyncInfo
{
	CTAsyncInfo() : AsyncInfo(SocketOperationNone), endpointIndex(0)
	{
	}

	int endpointIndex;
};
	

class CTWorkThread : public ::IceUtil::Thread
{
public:
	virtual void run();
};


class CTEventHandler : virtual public ::IceInternal::EventHandler
{
public:
	CTEventHandler(SOCKET s) : _socket(s)
	{

	}

	//
	// Called to start a new asynchronous read or write operation.
	//
	virtual bool startAsync(SocketOperation sop){ return false; }
	virtual bool finishAsync(SocketOperation sop){ return false; }

	//
	// Called when there's a message ready to be processed.
	//
	virtual void message(ThreadPoolCurrent& cur){}

	//
	// Called when the event handler is unregistered.
	//
	virtual void finished(ThreadPoolCurrent& cur){}

	//
	// Get a textual representation of the event handler.
	//
	virtual std::string toString() const{ return "CT"; }

	//
	// Get the native information of the handler, this is used by the selector.
	//
	virtual NativeInfoPtr getNativeInfo()
	{
		Ice::OperationNotExistException ex(__FILE__, __LINE__);
		throw ex;
		return NULL;
	}

	// instead getNativeInfo
	SOCKET GetNativeSocket()
	{
		return _socket;
	}

	// instead the implement getAsyncInfo in NativeInfo
	CTAsyncInfo* GetCTAsyncInfo()
	{
		return &_asyncInfo;
	}

	CTAsyncInfo _asyncInfo;
	const SOCKET _socket;

private:
	CTEventHandler();

};

class CacheAddress
{
public:
	CacheAddress(EndpointI* pEP)
	{
		this->Init(pEP);
	}

	void Init(EndpointI* pEP)
	{
		if (pEP)
		{
			_host = pEP->GetHost();
			_port = pEP->GetPort();
		}
	}

	bool operator < (const CacheAddress& cAddr) const
	{
		if (_port != cAddr._port)
		{
			return _port < cAddr._port;
		}

		return _host < cAddr._host;
	}

	bool operator == (const CacheAddress& cAddr) const
	{
		return (this->_port == cAddr._port) && (this->_host == cAddr._host);
	}

	string		_host;
	int			_port;
};

const static DWORD IOCP_FLAG_CONNECT = 117;

static IceInternal::Selector s_CTSelector;
static CTWorkThread s_CTWorkThread;
static vector<CTEventHandler*> s_CTEvtHdVec;
static set<CacheAddress> s_CTCacheAddrTree;

static volatile bool s_CTInited = false;
static volatile int s_RsltIndex = -1;
//static IceUtil::Monitor<IceUtil::Mutex> s_RsltMonitor;

static void InitializeByCTEventHandler(IceInternal::Selector& selector, CTEventHandler* ctHandler); // instead Selector::initialize
static EventHandler* GetNextHandlerWithCTAsyncInfo(IceInternal::Selector& selector, CTAsyncInfo& ctAsyncInfo, int timeout); // instead Selector::getNextHandler
static bool AddSocketToCTSelectorAndConnectAsync(SOCKET sock, Address tgtAddr, int index);
static void CTSelectorCleanAllSockets();

void CTSelectorInit();
void ResortEndpointsByNetTest(vector<EndpointIPtr>& theEndpoints);
}


static IceUtil::Mutex CTAssginResult_mutex;
void CRS::CTWorkThread::run()
{
	while (1)
	{
		CTAsyncInfo theAsyncInfo;
		try
		{
			EventHandler* pHandler = GetNextHandlerWithCTAsyncInfo(s_CTSelector, theAsyncInfo, -1);
			if (!pHandler)
			{
				continue;
			}
		}
		catch (const SelectorTimeoutException&)
		{
			OUTPUT_LOG_A(1, "WorkThread throw SelectorTimeoutException!!");
            LOG(CSystemLog::LOG_ERROR, "[CTWorkThread::run] WorkThread throw SelectorTimeoutException!!");
			continue;
		}

		// Finished an IOCP operation
		if (IOCP_FLAG_CONNECT == theAsyncInfo.flags && 0 == theAsyncInfo.error)
		{
			//IceUtil::Monitor<IceUtil::Mutex>::Lock monitorAutoLock(s_RsltMonitor);

			IceUtil::Mutex::Lock ARautoLock(CTAssginResult_mutex);

			OUTPUT_LOG_A(1, "CT success. theAsyncInfo.flags: " << theAsyncInfo.flags << " theAsyncInfo.error: " << theAsyncInfo.error);
            LOG(CSystemLog::LOG_DEBUG, "[CTWorkThread::run] CT success. index[%d] theAsyncInfo.flags: %d theAsyncInfo.error:%d.",
                theAsyncInfo.endpointIndex, theAsyncInfo.flags, theAsyncInfo.error);
			if (-1 == s_RsltIndex)
			{
			    // only assgin the first index
				s_RsltIndex = theAsyncInfo.endpointIndex;
			}
			//s_RsltMonitor.notifyAll();

			//CTSelectorCleanAllSockets();
		}
		else
		{
			OUTPUT_LOG_A(1, "CT ignore. theAsyncInfo.flags: " << theAsyncInfo.flags << " theAsyncInfo.error: " << theAsyncInfo.error);
            LOG(CSystemLog::LOG_DEBUG, "[CTWorkThread::run] CT ignore. theAsyncInfo.flags: %d theAsyncInfo.error:%d.",
                theAsyncInfo.flags, theAsyncInfo.error);
		}

	}
}


static IceUtil::Mutex CTSelectorInit_TSmutex;
void CRS::CTSelectorInit()
{
	IceUtil::Mutex::Lock TSautoLock(CTSelectorInit_TSmutex);

	try
	{	
		if (!s_CTInited)
		{   // init IOCP

			s_CTSelector.setup(0);
			// start work thread
			s_CTWorkThread.start();

			s_CTInited = true;

            LOG(CSystemLog::LOG_SYSTEM, "[CRS::CTSelectorInit] CTSelectorInit success.");
		}
	}
	catch (...)
	{
        LOG(CSystemLog::LOG_SYSTEM, "[CRS::CTSelectorInit] CTSelectorInit failed!! LastError: %d.", GetLastError());
	}
}

static void CRS::InitializeByCTEventHandler(IceInternal::Selector& selector, CTEventHandler* ctHandler)
{
	if (!ctHandler)
	{
		Ice::ObjectNotExistException ex(__FILE__, __LINE__);
		throw ex;
	}

	HANDLE socket = reinterpret_cast<HANDLE>(ctHandler->GetNativeSocket());
	if (CreateIoCompletionPort(socket, selector.getIOCPHandle(), reinterpret_cast<ULONG_PTR>(ctHandler), 0) == NULL)
	{
		Ice::SocketException ex(__FILE__, __LINE__);
		ex.error = GetLastError();
		throw ex;
	}
}

static EventHandler* CRS::GetNextHandlerWithCTAsyncInfo(IceInternal::Selector& selector, CTAsyncInfo& ctAsyncInfo, int timeout)
{
	ULONG_PTR key;
	LPOVERLAPPED ol;
	DWORD count;

	if (!GetQueuedCompletionStatus(selector.getIOCPHandle(), &count, &key, &ol, INFINITE))
	{
		ctAsyncInfo.count = SOCKET_ERROR;
		ctAsyncInfo.error = WSAGetLastError();

		OUTPUT_LOG_A(1, "CT GetQueuedCompletionStatus failed!! theAsyncInfo.flags: " << ctAsyncInfo.flags << " theAsyncInfo.error: " << ctAsyncInfo.error << " ol: " << (unsigned int)(ol));
        LOG(CSystemLog::LOG_ERROR, "[CRS::GetNextHandlerWithCTAsyncInfo] CT GetQueuedCompletionStatus failed!! theAsyncInfo.flags:%d,theAsyncInfo.error:%d,ol:%d.",
            ctAsyncInfo.flags, ctAsyncInfo.error, (unsigned int)(ol));
		return NULL;
	}

	if (!ol)
	{
		ctAsyncInfo.count = SOCKET_ERROR;
		ctAsyncInfo.error = WSAGetLastError();

		OUTPUT_LOG_A(1, "CT GetQueuedCompletionStatus return NULL ol!! theAsyncInfo.flags: " << ctAsyncInfo.flags << " theAsyncInfo.error: " << ctAsyncInfo.error << " ol: " << (unsigned int)(ol));

        LOG(CSystemLog::LOG_ERROR, "[CRS::GetNextHandlerWithCTAsyncInfo] CT GetQueuedCompletionStatus return NULL ol!! theAsyncInfo.flags:%d,theAsyncInfo.error:%d,ol:%d.",
            ctAsyncInfo.flags, ctAsyncInfo.error, (unsigned int)(ol));
		return NULL;
	}

	CTAsyncInfo* pInfo = static_cast<CTAsyncInfo*>(ol);
	ctAsyncInfo = *pInfo;
	ctAsyncInfo.error = 0;

	//OUTPUT_LOG_A(1, "GetQueuedCompletionStatus success, theAsyncInfo.flags: " << ctAsyncInfo.flags << " theAsyncInfo.error: " << ctAsyncInfo.error << " ol: " << (unsigned int)(ol));

	return reinterpret_cast<EventHandler*>(key);
}


static bool CRS::AddSocketToCTSelectorAndConnectAsync(SOCKET sock, Address tgtAddr, int index)
{
	CTEventHandler* pET = new(std::nothrow) CTEventHandler(sock);
	if (!pET)
	{
		return false;
	}

	try
	{
		InitializeByCTEventHandler(s_CTSelector, pET);
	}
	catch (...)
	{
	    // add to iocp failed
	    LOG(CSystemLog::LOG_ERROR, "[AddSocketToCTSelectorAndConnectAsync] Failed to CreateIoCompletionPort[addr=%s,index=%d,socket=%d].",
	        addrToString(tgtAddr).c_str(), index, sock);
		delete pET;
		return false;
	}

	s_CTEvtHdVec.push_back(pET);

	//if (!PostQueuedCompletionStatus(_handle, 0, reinterpret_cast<ULONG_PTR>(handler), info))

	CTAsyncInfo* pInfo = pET->GetCTAsyncInfo();
	pInfo->flags = IOCP_FLAG_CONNECT;
	pInfo->endpointIndex = index;

	try
	{
		doConnectAsync(sock, tgtAddr, *pInfo);
	}
	catch (...)
	{
	    // add to iocp failed
	    LOG(CSystemLog::LOG_ERROR, "[AddSocketToCTSelectorAndConnectAsync] Failed to doConnectAsync[addr=%s,index=%d,socket=%d].",
	        addrToString(tgtAddr).c_str(), index, sock);
		delete pET;
		return false;
	}

	return true;
}

static IceUtil::Mutex CTSelectorCleanAllSockets_TSmutex;
void CRS::CTSelectorCleanAllSockets()
{
	IceUtil::Mutex::Lock TSautoLock(CTSelectorCleanAllSockets_TSmutex);

	//vector<CTEventHandler*> dumpVec(s_CTEvtHdVec); // copy for loop

	unsigned int i = 0;
	for (; i < s_CTEvtHdVec.size(); i++)
	{
		CTEventHandler* pTheET = s_CTEvtHdVec[i];

		if (pTheET)
		{
			closeSocketNoThrow(pTheET->_socket);

			// delete pTheET;
		}
	}

	s_CTEvtHdVec.clear();
}

static void GetAddrAndSockVectors(vector<SOCKET>& sockVec, vector<Address>& addrVec, const vector<EndpointIPtr>& theEndpoints)
{
	addrVec.clear();
	sockVec.clear();

	vector<EndpointIPtr>::const_iterator theItem;
	for (theItem = theEndpoints.begin(); theItem != theEndpoints.end(); ++theItem)
	{
		EndpointI* pEP = theItem->get();
		if (TCPEndpointType == (*theItem)->type())
		{
			TcpEndpointI* pEndPt = dynamic_cast<TcpEndpointI*>(pEP);
			if (!pEndPt)
			{
				continue;
			}

			Address theAddr = getAddressForServer(pEndPt->GetHost(), pEndPt->GetPort(), IceInternal::EnableBoth, false);
			SOCKET theSock = IceInternal::createSocket(false, theAddr); // tcp socket

			//if (INVALID_SOCKET != theSock)
			{// add ignore condition, because need equal to count of endpoints
				addrVec.push_back(theAddr);
				sockVec.push_back(theSock);
			}
		}
		else if (UDPEndpointType == (*theItem)->type())
		{
			UdpEndpointI* pEndPt = dynamic_cast<UdpEndpointI*>(pEP);
			if (!pEndPt)
			{
				continue;
			}

			Address theAddr = getAddressForServer(pEndPt->GetHost(), pEndPt->GetPort(), IceInternal::EnableBoth, false);
			SOCKET theSock = IceInternal::createSocket(true, theAddr); // udp socket

			//if (INVALID_SOCKET != theSock)
			{
				addrVec.push_back(theAddr);
				sockVec.push_back(theSock);
			}
		}
	}
}

// for ResortEndpointsByNetTest threadSafe
static IceUtil::Mutex ResortFuncThreadSafeMutex;
void CRS::ResortEndpointsByNetTest(vector<EndpointIPtr>& theEndpoints)
{	
	int times = 5 * 30;
	IceUtil::Mutex::TryLock TSautoLock(ResortFuncThreadSafeMutex);
	while (!TSautoLock.acquired() && times-- > 0)
	{
		IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(200));
		TSautoLock.acquire();
	}
	if (!TSautoLock.acquired())	//1s还没获取到锁就退出用默认端点顺序  改为3s
	{
		LOG(CSystemLog::LOG_DEBUG, "[CRS::ResortEndpointsByNetTest] can't get lock over 3s, return.");
		return;
	}
	//OUTPUT_LOG_A(1, "ResortEndpointsByNetTest. Endpoints count:" << theEndpoints.size() << " boolInit:" << s_CTInited);
	//LOG(CSystemLog::LOG_DEBUG, "[CRS::ResortEndpointsByNetTest] Endpoints count:%d.", theEndpoints.size());


	if (theEndpoints.size() <= 1 || !s_CTInited)
	{// less than 1, or not inited, don't sort
		return;
	}

	int rsltIndex = -1;

	bool bCached = false; // for output log
	{
        // search in connectted cache tree
		unsigned int k;
		for (k = 0; k < theEndpoints.size(); k++)
		{
			CacheAddress findAddr(theEndpoints[k].get());

			if (s_CTCacheAddrTree.find(findAddr) != s_CTCacheAddrTree.end())
			{
			    // found in cache tree
				rsltIndex = k;

				bCached = true;
				//OUTPUT_LOG_A(1, "Pick from cache tree. Index:[" << rsltIndex << "] host:" << findAddr._host << " port:" << findAddr._port);
				//LOG(CSystemLog::LOG_DEBUG, "Pick from cache tree. Index:%d, host:%s,port:%d.",
				//    rsltIndex, findAddr._host.c_str(), findAddr._port);
			}
		}
	}
	if (bCached == false)
	{// print result to log
		int i;
		for (i = 0; i < theEndpoints.size(); i++)
		{
			EndpointI* pEP = theEndpoints[i].get();

			OUTPUT_LOG_A(1, "Original Endpoint[" << i << "]# host:" << pEP->GetHost() << " port:" << pEP->GetPort());
            LOG(CSystemLog::LOG_DEBUG, "[CRS::ResortEndpointsByNetTest] Original Endpoint:index[%d] host:%s, port:%d.",
                i, pEP->GetHost().c_str(), pEP->GetPort());
		}
	}   

	if (rsltIndex < 0)
	{
		vector<Address> addrVec;
		vector<SOCKET>  sockVec;

		GetAddrAndSockVectors(sockVec, addrVec, theEndpoints);

		assert(addrVec.size() == sockVec.size());
		if (addrVec.size() != theEndpoints.size() || sockVec.size() != theEndpoints.size())
		{
		    // some endpoints is not TCP/UDP endpoints, can't match the index in endpoints vector, skip this function
			return;
		}

    	{   
            // thread safe old scope
    		//IceUtil::Mutex::Lock TSautoLock(ResortFuncThreadSafeMutex);
    		//IceUtil::Monitor<IceUtil::Mutex>::Lock monitorAutoLock(s_RsltMonitor);

    		s_RsltIndex = -1;

    		unsigned int idx = 0;
    		for (; idx < sockVec.size() && idx < addrVec.size(); idx++)
    		{
    			if (INVALID_SOCKET != sockVec[idx])
    			{
    			    // not test invalid socket
    				AddSocketToCTSelectorAndConnectAsync(sockVec[idx], addrVec[idx], idx);
    			}
    		}
    		//ResortFuncThreadSafeMutex.unlock();
    		volatile int c = 20;
    		while (-1 == s_RsltIndex && c-- > 0)
    		{
    		    // c * 2 ms
    			IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(200));
    		}
    		//ResortFuncThreadSafeMutex.lock();

    		rsltIndex = s_RsltIndex;

    		CTSelectorCleanAllSockets(); // got result or overtime, close all sockets

    		OUTPUT_LOG_A(1, "ResortEndpoints Selected Index: " << rsltIndex << " Tick Counter:" << c);
            LOG(CSystemLog::LOG_DEBUG, "[CRS::ResortEndpointsByNetTest] ResortEndpoints Selected Index: %d, Tick Counter:%d.",
                rsltIndex, c);

    		if (rsltIndex < 0 || rsltIndex >= (int)(theEndpoints.size()))
    		{
    		    // haven't rslt or over
    			CacheAddress newAddr(theEndpoints[0].get()); // add for prevent always test unreachable EndPoints
    			s_CTCacheAddrTree.insert(newAddr);
    			return;
    		}
    		else
    		{
    		    // connect successed, add to successed cache
    			CacheAddress newAddr(theEndpoints[rsltIndex].get());
    			s_CTCacheAddrTree.insert(newAddr);
    		}
    	}// thread safe old scope

	}// if (rsltIndex < 0)

	{
        // resort, put result index at first
		vector<EndpointIPtr> orgEndpoints(theEndpoints);

		theEndpoints[0] = orgEndpoints[rsltIndex];

		unsigned int i = 1, j = 0;
		while (i < theEndpoints.size() && j < orgEndpoints.size())
		{
			if (rsltIndex == j)
			{
				j++;
				continue;
			}
			theEndpoints[i++] = orgEndpoints[j++];
		}
	}

	if (!bCached)
	{
	    // print result to log
		unsigned int i;
		for (i = 0; i < theEndpoints.size(); i++)
		{
			EndpointI* pEP = theEndpoints[i].get();

			OUTPUT_LOG_A(1, "After ResortEndpointsByNetTest Endpoint[" << i << "]# host:" << pEP->GetHost() << " port:" << pEP->GetPort());
            LOG(CSystemLog::LOG_SYSTEM, "[CRS::ResortEndpointsByNetTest] After ResortEndpointsByNetTest Endpoint[%d]# host:%s port:%d.",
                i, pEP->GetHost().c_str(), pEP->GetPort());
		}
	}
}

#endif  // OS_WIN