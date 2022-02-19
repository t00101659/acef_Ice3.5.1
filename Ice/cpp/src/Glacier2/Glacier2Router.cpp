// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/UUID.h>
#include <IceUtil/Options.h>
#include <IceUtil/FileUtil.h>
#include <Ice/Service.h>
#include <Glacier2/Instance.h>
#include <Glacier2/RouterI.h>
#include <Glacier2/Session.h>
#include <Glacier2/SessionRouterI.h>
#include <Glacier2/CryptPermissionsVerifierI.h>
#include "../Ice/Chaos_IceCommon.hpp"

using namespace std;
using namespace Ice;
using namespace Glacier2;
using namespace CRS;

namespace Glacier2
{

class RouterService : public Service
{
public:

    RouterService();

protected:

    virtual bool start(int, char*[], int&);
    virtual bool stop();
    virtual CommunicatorPtr initializeCommunicator(int&, char*[], const InitializationData&);

private:

    void usage(const std::string&);

    InstancePtr _instance;
    SessionRouterIPtr _sessionRouter;
};

class FinderI : public Ice::RouterFinder
{
public:
    
    FinderI(const Glacier2::RouterPrx& router) : _router(router)
    {
    }
    
    virtual Ice::RouterPrx
    getRouter(const Ice::Current&)
    {
        return _router;
    }

private:

    const Glacier2::RouterPrx _router;
};
class NullPermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    bool checkPermissions(const string&, const string&, string&, const Current&) const
    {
        return true;
    }
};

class NullSSLPermissionsVerifierI : public Glacier2::SSLPermissionsVerifier
{
public:

    virtual bool
    authorize(const Glacier2::SSLInfo&, std::string&, const Ice::Current&) const
    {
        return true;
    }
};

};

Glacier2::RouterService::RouterService()
{
}
/*
class ChatSessionI : public Glacier2::Session, public IceUtil::Mutex
{
public:

	ChatSessionI(const std::string&);

	virtual void ice_ping(const Ice::Current&) const;
	virtual void destroy(const Ice::Current&);

private:

	const std::string _userId;
};

ChatSessionI::ChatSessionI(const string& userId) :
_userId(userId)
{
}

void
ChatSessionI::ice_ping(const Ice::Current&) const
{
}


void
ChatSessionI::destroy(const Ice::Current& current)
{
	Lock sync(*this);
	current.adapter->remove(current.id);
}

class DummyPermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

	virtual bool
		checkPermissions(const string& userId, const string& password, string&, const Ice::Current&) const
	{
			cout << "verified user `" << userId << "' with password `" << password << "'" << endl;
			return true;
		}
};

class ChatSessionManagerI : public Glacier2::SessionManager
{
public:

	virtual Glacier2::SessionPrx
		create(const string& userId, const Glacier2::SessionControlPrx&, const Ice::Current& current)
        {
        	return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(new ChatSessionI(userId)));
        }
};*/

bool
Glacier2::RouterService::start(int argc, char* argv[], int& status)
{
    bool nowarn;
    LOG(CSystemLog::LOG_SYSTEM, "[RouterService::start] Enter RouterService::start.");
    
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "nowarn");

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

    if(!args.empty())
    {
        cerr << argv[0] << ": too many arguments" << endl;
        usage(argv[0]);
        return false;
    }

    PropertiesPtr properties = communicator()->getProperties();

    //
    // Initialize the client object adapter.
    //
    const string clientEndpointsProperty = "Glacier2.Client.Endpoints";
    if(properties->getProperty(clientEndpointsProperty).empty())
    {
        error("property `" + clientEndpointsProperty + "' is not set");
        LOG(CSystemLog::LOG_ERROR, "[RouterService::start] property %s is not set.",
            clientEndpointsProperty.c_str());
        return false;
    }
    
    if(properties->getPropertyAsInt("Glacier2.SessionTimeout") > 0 &&
       properties->getProperty("Glacier2.Client.ACM.Timeout").empty())
    {
        ostringstream os;
        os << properties->getPropertyAsInt("Glacier2.SessionTimeout");
        properties->setProperty("Glacier2.Client.ACM.Timeout", os.str());
    }

    if(properties->getProperty("Glacier2.Client.ACM.Close").empty())
    {
        properties->setProperty("Glacier2.Client.ACM.Close", "4"); // Forcefull close on invocation and idle.
    }

    ObjectAdapterPtr clientAdapter = communicator()->createObjectAdapter("Glacier2.Client");


    //
    // Initialize the server object adapter only if server endpoints
    // are defined.
    //
    const string serverEndpointsProperty = "Glacier2.Server.Endpoints";
    ObjectAdapterPtr serverAdapter;
    if(!properties->getProperty(serverEndpointsProperty).empty())
    {
        serverAdapter = communicator()->createObjectAdapter("Glacier2.Server");
    }

    string instanceName = properties->getPropertyWithDefault("Glacier2.InstanceName", "Glacier2");

    //
    // We need a separate object adapter for any collocated
    // permissions verifiers. We can't use the client adapter.
    //
    properties->setProperty("Glacier2Internal.Verifiers.AdapterId", IceUtil::generateUUID());
    ObjectAdapterPtr verifierAdapter = communicator()->createObjectAdapter("Glacier2Internal.Verifiers");
    verifierAdapter->setLocator(0);

    //
    // Check for a permissions verifier or a password file.
    //
    string verifierProperty = "Glacier2.PermissionsVerifier";
    string verifierPropertyValue = properties->getProperty(verifierProperty);
    string passwordsProperty = properties->getProperty("Glacier2.CryptPasswords");
    PermissionsVerifierPrx verifier;
    if(!verifierPropertyValue.empty())
    {
        Identity nullPermVerifId;
        nullPermVerifId.category = instanceName;
        nullPermVerifId.name = "NullPermissionsVerifier";

        ObjectPrx obj;
        try
        {
            try
            {
                obj = communicator()->propertyToProxy(verifierProperty);
            }
            catch(const Ice::ProxyParseException&)
            {
                //
                // Check if the property is just the identity of the null permissions verifier
                // (the identity might contain spaces which would prevent it to be parsed as a
                // proxy).
                //
                if(communicator()->stringToIdentity(verifierPropertyValue) == nullPermVerifId)
                {
                    obj = communicator()->stringToProxy("\"" + verifierPropertyValue + "\"");
                }
            }

            if(!obj)
            {
                error("permissions verifier `" + verifierPropertyValue + "' is invalid");
                LOG(CSystemLog::LOG_ERROR, "[RouterService::start] permissions verifier %s is invalid.",
                    verifierPropertyValue.c_str());
                return false;
            }
        }
        catch(const std::exception& ex)
        {
            ServiceError err(this);
            err << "permissions verifier `" << verifierPropertyValue + "' is invalid:\n" << ex;
            LOG(CSystemLog::LOG_ERROR, "[RouterService::start] permissions verifier %s is invalid:%s.",
                    verifierPropertyValue.c_str(), ex.what());
            return false;
        }

        if(obj->ice_getIdentity() == nullPermVerifId)
        {
            verifier = PermissionsVerifierPrx::uncheckedCast(
                verifierAdapter->add(new NullPermissionsVerifierI(), nullPermVerifId)->ice_collocationOptimized(true));
        }
        else
        {
            try
            {
                verifier = PermissionsVerifierPrx::checkedCast(obj);
                if(!verifier)
                {
                    error("permissions verifier `" + verifierPropertyValue + "' is invalid");
                    LOG(CSystemLog::LOG_ERROR, "[RouterService::start] permissions verifier %s is invalid.",
                    verifierPropertyValue.c_str());
                    return false;
                }
            }
            catch(const Ice::Exception& ex)
            {
                if(!nowarn)
                {
                    ServiceWarning warn(this);
                    warn << "unable to contact permissions verifier `" << verifierPropertyValue << "'\n" << ex;
                }
                verifier = PermissionsVerifierPrx::uncheckedCast(obj);
            }
        }
    }
    else if(!passwordsProperty.empty())
    {
        //
        // No nativeToUTF8 conversion necessary here, since no string
        // converter is installed by Glacier2 the string is UTF-8.
        //
        IceUtilInternal::ifstream passwordFile(passwordsProperty);

        if(!passwordFile)
        {
            string err = strerror(errno);
            error("cannot open `" + passwordsProperty + "' for reading: " + err);
            LOG(CSystemLog::LOG_ERROR, "[RouterService::start] cannot open %s for reading:%s.",
                    passwordsProperty.c_str(), err.c_str());
            return false;
        }

        map<string, string> passwords;

        while(true)
        {
            string userId;
            passwordFile >> userId;
            if(!passwordFile)
            {
                break;
            }

            string password;
            passwordFile >> password;
            if(!passwordFile)
            {
                break;
            }

            assert(!userId.empty());
            assert(!password.empty());
            passwords.insert(make_pair(userId, password));
        }

        PermissionsVerifierPtr verifierImpl = new CryptPermissionsVerifierI(passwords);

        verifier = PermissionsVerifierPrx::uncheckedCast(verifierAdapter->addWithUUID(verifierImpl));
    }

    //
    // Get the session manager if specified.
    //
    string sessionManagerProperty = "Glacier2.SessionManager";
    string sessionManagerPropertyValue = properties->getProperty(sessionManagerProperty);
    /*
	if (sessionManagerPropertyValue.empty())
	{
		::Ice::EndpointSeq endpts = clientAdapter->getEndpoints();
		for (size_t i = 0; i < endpts.size(); i++)
		{
			endpts[i]->toString();
			sessionManagerPropertyValue = "PucSessionManager:" + endpts[i]->toString();// tcp - h localhost - p 4064";
			//sessionManagerPropertyValue.replace(sessionManagerPropertyValue.find("0.0.0.0"), "localhost");
			if (sessionManagerPropertyValue.find("0.0.0.0") != std::string::npos)
			{
				std::string s = sessionManagerPropertyValue.substr(0, sessionManagerPropertyValue.find("0.0.0.0"));
				s +="localhost" + sessionManagerPropertyValue.substr(sessionManagerPropertyValue.find("0.0.0.0") + strlen("0.0.0.0"));
				sessionManagerPropertyValue = s;
			}
			std::cout << "galcier2router::" << sessionManagerPropertyValue << std::endl;
		}
	}
	if (sessionManagerPropertyValue.empty())
	{
		sessionManagerPropertyValue = "PucSessionManager:tcp - h localhost - p 4064";
	}*/
	std::cout << "galcier2router::1" << sessionManagerPropertyValue<< std::endl;
    SessionManagerPrx sessionManager;
    if(!sessionManagerPropertyValue.empty())
    {
        ObjectPrx obj;
        try
        {
			obj = communicator()->stringToProxy(sessionManagerPropertyValue);
        }
        catch(const std::exception& ex)
        {
            ServiceError err(this);
            err << "session manager `" << sessionManagerPropertyValue 
                << "' is invalid\n:" << ex.what();
            LOG(CSystemLog::LOG_ERROR, "[RouterService::start] session manager %s is invalid,ex:%s.",
                sessionManagerPropertyValue.c_str(), ex.what());
            return false;
        }
		std::cout << "galcier2router::2" << sessionManagerPropertyValue << std::endl;
        LOG(CSystemLog::LOG_SYSTEM, "[RouterService::start] galcier2router::2 %s.",
            sessionManagerPropertyValue.c_str());
        try
        {
            sessionManager = SessionManagerPrx::checkedCast(obj);
            if(!sessionManager)
            {
                error("session manager `" + sessionManagerPropertyValue + "' is invalid");
                LOG(CSystemLog::LOG_ERROR, "session manager %s is invalid.", sessionManagerPropertyValue.c_str());
                return false;
            }
        }
        catch(const std::exception& ex)
        {
            if(!nowarn)
            {
                ServiceWarning warn(this);
                warn << "unable to contact session manager `" << sessionManagerPropertyValue << "'\n"
                     << ex;
            }
            LOG(CSystemLog::LOG_ERROR, "unable to contact session manager:%s,ex:%s.", 
                sessionManagerPropertyValue.c_str(), ex.what());
            sessionManager = SessionManagerPrx::uncheckedCast(obj);
        }
		std::cout << "galcier2router::3" << sessionManagerPropertyValue << std::endl;
        sessionManager = 
            SessionManagerPrx::uncheckedCast(sessionManager->ice_connectionCached(false)->ice_locatorCacheTimeout(
                properties->getPropertyAsIntWithDefault("Glacier2.SessionManager.LocatorCacheTimeout", 600)));
    }
	std::cout << "galcier2router::4" << sessionManagerPropertyValue << std::endl;
    LOG(CSystemLog::LOG_SYSTEM, "[RouterService::start] galcier2router::4 %s.",
            sessionManagerPropertyValue.c_str());
    //
    // Check for an SSL permissions verifier.
    //
    string sslVerifierProperty = "Glacier2.SSLPermissionsVerifier";
    string sslVerifierPropertyValue = properties->getProperty(sslVerifierProperty);
    SSLPermissionsVerifierPrx sslVerifier;
    if(!sslVerifierPropertyValue.empty())
    {
        Identity nullSSLPermVerifId;
        nullSSLPermVerifId.category = instanceName;
        nullSSLPermVerifId.name = "NullSSLPermissionsVerifier";

        ObjectPrx obj;
        try
        {
            try
            {
                obj = communicator()->propertyToProxy(sslVerifierProperty);
            }
            catch(const Ice::ProxyParseException&)
            {
                //
                // Check if the property is just the identity of the null permissions verifier
                // (the identity might contain spaces which would prevent it to be parsed as a
                // proxy).
                //
                if(communicator()->stringToIdentity(sslVerifierPropertyValue) == nullSSLPermVerifId)
                {
                    obj = communicator()->stringToProxy("\"" + sslVerifierPropertyValue + "\"");
                }
            }

            if(!obj)
            {
                error("ssl permissions verifier `" + verifierPropertyValue + "' is invalid");
                LOG(CSystemLog::LOG_ERROR, "[RouterService::start] ssl permissions verifier %s is invalid.",
                    verifierPropertyValue.c_str());
                return false;
            }
        }
        catch(const std::exception& ex)
        {
            ServiceError err(this);
            err << "ssl permissions verifier `" << sslVerifierPropertyValue << "' is invalid:\n"
                << ex;
            LOG(CSystemLog::LOG_ERROR, "[RouterService::start] ssl permissions verifier %s is invalid:%s.",
                    sslVerifierPropertyValue.c_str(), ex.what());
            return false;
        }

        if(obj->ice_getIdentity() == nullSSLPermVerifId)
        {

            sslVerifier = SSLPermissionsVerifierPrx::uncheckedCast(
                verifierAdapter->add(new NullSSLPermissionsVerifierI(), nullSSLPermVerifId)->
                        ice_collocationOptimized(true));
        }
        else
        {
            try
            {
                sslVerifier = SSLPermissionsVerifierPrx::checkedCast(obj);
                if(!sslVerifier)
                {
                    error("ssl permissions verifier `" + sslVerifierPropertyValue + "' is invalid");
                    LOG(CSystemLog::LOG_ERROR, "[RouterService::start] ssl permissions verifier %s is invalid.",
                    sslVerifierPropertyValue.c_str());
                    return false;
                }
            }
            catch(const Ice::Exception& ex)
            {
                if(!nowarn)
                {
                    ServiceWarning warn(this);
                    warn << "unable to contact ssl permissions verifier `" << sslVerifierPropertyValue << "'\n"
                         << ex;
                }
                sslVerifier = SSLPermissionsVerifierPrx::uncheckedCast(obj);
            }
        }
    }
	
    //
    // Get the SSL session manager if specified.
    //
    string sslSessionManagerProperty = "Glacier2.SSLSessionManager";
    string sslSessionManagerPropertyValue = properties->getProperty(sslSessionManagerProperty);
    SSLSessionManagerPrx sslSessionManager;
    if(!sslSessionManagerPropertyValue.empty())
    {
        ObjectPrx obj;
        try
        {
            obj = communicator()->propertyToProxy(sslSessionManagerProperty);
        }
        catch(const std::exception& ex)
        {
            ServiceError err(this);
            err << "ssl session manager `" << sslSessionManagerPropertyValue << "' is invalid:\n" << ex;
            LOG(CSystemLog::LOG_ERROR, "[RouterService::start] ssl session manager %s is invalid:%s.",
                    sslSessionManagerPropertyValue.c_str(), ex.what());
            return false;
        }
        try
        {
            sslSessionManager = SSLSessionManagerPrx::checkedCast(obj);
            if(!sslSessionManager)
            {
                error("ssl session manager `" + sslSessionManagerPropertyValue + "' is invalid");
                LOG(CSystemLog::LOG_ERROR, "[RouterService::start] ssl session manager %s is invalid.",
                    sslSessionManagerPropertyValue.c_str());
                return false;
            }
        }
        catch(const Ice::Exception& ex)
        {
            if(!nowarn)
            {
                ServiceWarning warn(this);
                warn << "unable to contact ssl session manager `" << sslSessionManagerPropertyValue 
                     << "'\n" << ex;
            }
            sslSessionManager = SSLSessionManagerPrx::uncheckedCast(obj);
        }
        sslSessionManager = 
            SSLSessionManagerPrx::uncheckedCast(sslSessionManager->ice_connectionCached(false)->ice_locatorCacheTimeout(
                properties->getPropertyAsIntWithDefault("Glacier2.SSLSessionManager.LocatorCacheTimeout", 600)));
    }

    if(!verifier && !sslVerifier)
    {
        error("Glacier2 requires a permissions verifier or password file");
        LOG(CSystemLog::LOG_ERROR, "[RouterService::start] Glacier2 requires a permissions verifier or password file.");
        return false;
    }

    //
    // Create the instance object.
    //
    try
    {
        _instance = new Instance(communicator(), clientAdapter, serverAdapter);
    }
    catch(const Ice::InitializationException& ex)
    {
        error("Glacier2 initialization failed:\n" + ex.reason);
        LOG(CSystemLog::LOG_ERROR, "[RouterService::start] Glacier2 initialization failed:%s.", ex.reason.c_str());
        return false;
    }
    /*
	Glacier2::PermissionsVerifierPtr dpv = new DummyPermissionsVerifierI;
	clientAdapter->add(dpv, communicator()->stringToIdentity("PucSessionVerifier"));
	Glacier2::SessionManagerPtr csm = new ChatSessionManagerI;
	clientAdapter->add(csm, communicator()->stringToIdentity("PucSessionManager"));
    */
    //
    // Create the session router. The session router registers itself
    // and all required servant locators, so no registration has to be
    // done here.
    //
    _sessionRouter = new SessionRouterI(_instance, verifier, sessionManager, sslVerifier, sslSessionManager);

    //
    // Th session router is used directly as servant for the main
    // Glacier2 router Ice object.
    //
    Identity routerId;
    routerId.category = _instance->properties()->getPropertyWithDefault("Glacier2.InstanceName", "Glacier2");
    routerId.name = "router";
    Glacier2::RouterPrx routerPrx = Glacier2::RouterPrx::uncheckedCast(clientAdapter->add(_sessionRouter, routerId));

    //
    // Add the Ice router finder object to allow retrieving the router
    // proxy with just the endpoint information of the router.
    //
    Identity finderId;
    finderId.category = "Ice";
    finderId.name = "RouterFinder";
    clientAdapter->add(new FinderI(routerPrx), finderId);
    
    if(_instance->getObserver())
    {
        _instance->getObserver()->setObserverUpdater(_sessionRouter);
    }

    //
    // Everything ok, let's go.
    //
    try
    {
        clientAdapter->activate();
        if(serverAdapter)
        {
            serverAdapter->activate();
        }
    }
    catch(const std::exception& ex)
    {
        {
            ServiceError err(this);
            err << "caught exception activating object adapters\n" << ex;
            LOG(CSystemLog::LOG_ERROR, "[RouterService::start] caught exception activating object adapters:%s.", 
                ex.what());
        }

        stop();
        return false;
    }
    LOG(CSystemLog::LOG_SYSTEM, "[RouterService::start] Leave RouterService::start.");
    return true;
}

bool
Glacier2::RouterService::stop()
{
    if(_sessionRouter)
    {
        _sessionRouter->destroy();
        _sessionRouter = 0;
    }

    if(_instance)
    {
        if(_instance->getObserver())
        {
            _instance->getObserver()->setObserverUpdater(0);
        }
        _instance->destroy();
        _instance = 0;
    }
    return true;
}

CommunicatorPtr
Glacier2::RouterService::initializeCommunicator(int& argc, char* argv[], 
                                                const InitializationData& initializationData)
{
    InitializationData initData = initializationData;
    initData.properties = createProperties(argc, argv, initializationData.properties);
 
    //
    // Make sure that Glacier2 doesn't use a router.
    //
    initData.properties->setProperty("Ice.Default.Router", "");
    
    //
    // Active connection management is permitted with Glacier2. For
    // the client object adapter, the ACM timeout is set to the
    // session timeout to ensure client connections are not closed
    // prematurely,
    //
    //initData.properties->setProperty("Ice.ACM.Client", "0");
    //initData.properties->setProperty("Ice.ACM.Server", "0");

    //
    // We do not need to set Ice.RetryIntervals to -1, i.e., we do
    // not have to disable connection retry. It is safe for
    // Glacier2 to retry outgoing connections to servers. Retry
    // for incoming connections from clients must be disabled in
    // the clients.
    //

    return Service::initializeCommunicator(argc, argv, initData);
}

void
Glacier2::RouterService::usage(const string& appName)
{
    string options =
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "--nowarn             Suppress warnings.";
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

#ifdef _WIN32

int
wmain(int argc, wchar_t* argv[])

#else

int
main(int argc, char* argv[])

#endif
{
    Glacier2::RouterService svc;
    return svc.main(argc, argv);
}
