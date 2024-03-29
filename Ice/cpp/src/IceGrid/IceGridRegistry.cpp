// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <Ice/Ice.h>
#include <Ice/Service.h>
#include <IceGrid/RegistryI.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/Util.h>
#include "../Ice/Chaos_IceCommon.hpp"

using namespace std;
using namespace Ice;
using namespace IceGrid;
using namespace CRS;

namespace IceGrid
{

class RegistryService : public Service
{
public:

    RegistryService();
    ~RegistryService();

    virtual bool shutdown();

protected:

    virtual bool start(int, char*[], int&);
    virtual void waitForShutdown();
    virtual bool stop();
    virtual CommunicatorPtr initializeCommunicator(int&, char*[], const InitializationData&);

private:

    void usage(const std::string&);

    RegistryIPtr _registry;
};

} // End of namespace IceGrid

RegistryService::RegistryService()
{
}

RegistryService::~RegistryService()
{
}

bool
RegistryService::shutdown()
{
    assert(_registry);
    _registry->shutdown();
    return true;
}

bool
RegistryService::start(int argc, char* argv[], int& status)
{
    LOG(CSystemLog::LOG_SYSTEM, "[RegistryService::start] Enter start.");
    bool nowarn;
    bool readonly;
    std::string initFromReplica;

    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "nowarn");
    opts.addOpt("", "readonly");
    opts.addOpt("", "initdb-from-replica", IceUtilInternal::Options::NeedArg);
    
    vector<string> args;
    try
    {
        args = opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        error(e.reason);
        usage(argv[0]);
        return false;
    }

    if(opts.isSet("help"))
    {
        usage(argv[0]);
        status = EXIT_SUCCESS;
        return false;
    }
    if(opts.isSet("version"))
    {
        print(ICE_STRING_VERSION);
        status = EXIT_SUCCESS;
        return false;
    }
    nowarn = opts.isSet("nowarn");
    readonly = opts.isSet("readonly");
    if(opts.isSet("initdb-from-replica"))
    {
        initFromReplica = opts.optArg("initdb-from-replica");
    }

    if(!args.empty())
    {
        cerr << argv[0] << ": too many arguments" << endl;
        usage(argv[0]);
        return false;
    }

    Ice::PropertiesPtr properties = communicator()->getProperties();

    //
    // Warn the user that setting Ice.ThreadPool.Server isn't useful.
    //
    if(!nowarn && properties->getPropertyAsIntWithDefault("Ice.ThreadPool.Server.Size", 0) > 0)
    {
        Warning out(communicator()->getLogger());
        out << "setting `Ice.ThreadPool.Server.Size' is not useful, ";
        out << "you should set individual adapter thread pools instead.";
    }


    TraceLevelsPtr traceLevels = new TraceLevels(communicator(), "IceGrid.Registry");
    
    _registry = new RegistryI(communicator(), traceLevels, nowarn, readonly, initFromReplica);
    if(!_registry->start())
    {
        return false;
    }
    LOG(CSystemLog::LOG_SYSTEM, "[RegistryService::start] End start.");
    return true;
}

void
RegistryService::waitForShutdown()
{
    //
    // Wait for the activator shutdown. Once the run method returns
    // all the servers have been deactivated.
    //
    enableInterrupt();
    _registry->waitForShutdown();
    disableInterrupt();
}

bool
RegistryService::stop()
{
    _registry->stop();
    return true;
}

CommunicatorPtr
RegistryService::initializeCommunicator(int& argc, char* argv[], 
                                        const InitializationData& initializationData)
{
    InitializationData initData = initializationData;
    initData.properties = createProperties(argc, argv, initData.properties);
    
    //
    // Make sure that IceGridRegistry doesn't use collocation optimization.
    //
    initData.properties->setProperty("Ice.Default.CollocationOptimized", "0");

    //
    // Default backend database plugin is Freeze if none is specified.
    //
    if(initData.properties->getProperty("Ice.Plugin.DB").empty())
    {
        initData.properties->setProperty("Ice.Plugin.DB", "IceGridFreezeDB:createFreezeDB");
    }

    //
    // Setup the client thread pool size.
    //
    setupThreadPool(initData.properties, "Ice.ThreadPool.Client", 1, 100);

    //
    // Close idle connections
    //
    initData.properties->setProperty("Ice.ACM.Close", "3");


    return Service::initializeCommunicator(argc, argv, initData);
}

void
RegistryService::usage(const string& appName)
{
    string options =
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "--nowarn             Don't print any security warnings.\n"
        "--readonly           Start the master registry in read-only mode.\n"
        "--initdb-from-replica=<replica>\n"
        "                     Initialize the database from the given replica.";
#ifndef _WIN32
    options.append(
        "\n"
        "\n"
        "--daemon             Run as a daemon.\n"
        "--pidfile FILE       Write process ID into FILE.\n"
        "--noclose            Do not close open file descriptors.\n"
        "--nochdir            Do not change the current working directory.\n"
    );
#endif
    print("Usage: " + appName + " [options]\n" + options);
}

 //#include "../Ice/Chaos_IceCommon.icpp.cpp"

#ifdef _WIN32
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")
std::string  inttostring(int value)
{
	char buf[20] = { 0 };
	sprintf(buf, ("%d"), value);
	return buf;
}
LONG WINAPI MyUnhandledExceptionFilter(__in struct _EXCEPTION_POINTERS *ExceptionInfo)
{
    printf("MyUnhandledExceptionFilter\n");
	MINIDUMP_EXCEPTION_INFORMATION INFO;
	INFO.ExceptionPointers = ExceptionInfo;
	INFO.ThreadId = GetCurrentThreadId();
	INFO.ClientPointers = false;
	
	SYSTEMTIME	tm;
	GetLocalTime(&tm);
	std::string strPath = "./Registry_dump" + inttostring(tm.wHour) + inttostring(tm.wMinute) + inttostring(tm.wSecond) + ".dmp";
    
	HANDLE hf = CreateFileA(strPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hf, MiniDumpNormal, ExceptionInfo == NULL ? 0 : &INFO, NULL, NULL);
	CloseHandle(hf);
	return EXCEPTION_CONTINUE_SEARCH;
}
int
wmain(int argc, wchar_t* argv[])

#else

int
main(int argc, char* argv[])

#endif
{
#ifdef _WIN32
    //增加捕获minidump事件
    SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
	SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX | SEM_NOALIGNMENTFAULTEXCEPT);
#endif
    RegistryService svc;
    return svc.main(argc, argv);
}
