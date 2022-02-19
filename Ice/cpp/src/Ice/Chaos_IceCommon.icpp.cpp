// FileName         : Chaos_IceCommon.icpp.cpp
// Date of Creation : 2017-10-24
// Description      : 
// Author		    : GuanChao
//____________________________________________________________________
#include <IceUtil/FileUtil.h>
#include <IceUtil/DisableWarnings.h>
#include "Chaos_IceCommon.hpp"
#include <time.h>
#include <fstream>
#include <stdarg.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <time.h>
#ifdef   OS_WIN
#include "WinBase.h"
#else
#include <sys/time.h>
#include <sys/statfs.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>
#define MAX_PATH PATH_MAX
#define DIR_DELIM '/'
#define _tcsnicmp  strncmp
#endif

using namespace std;
namespace CRS
{
    std::string          g_logNetType;
    unsigned int         g_logMaxSize = 0;
	vector<std::string>          g_FilterIps;
    int         g_logLevel = 0;

    static string GetExeDirPath(string& exeName, string& exeMainName)
    {
    	char pathBuf[_V_(MAX_PATH_LEN, 1024)];

#ifdef OS_WIN
    	int fullPathLen = GetModuleFileNameA(NULL, pathBuf, sizeof(pathBuf));
#else
    	int fullPathLen = readlink("/proc/self/exe", pathBuf, sizeof(pathBuf));
#endif // OS
    	if (fullPathLen <= 0)
    	{
    		printf("Can't get exe path!\n");
    		fflush(stdout);
    		return string();
    	}
    	pathBuf[fullPathLen] = '\0';

    	// read to pathBuf end

    	char* pEnd = (char*)(&(pathBuf[fullPathLen]));
    	const char* pLimit = (const char*)pathBuf;

    	while ((*pEnd) != char('\\')
    		&& (*pEnd) != char('/')
    		&& pEnd > pLimit)
    	{
    		pEnd--;
    	}

    	if (pEnd <= pLimit)
    	{// there haven't \ or / in path
    		return string();
    	}

    	pEnd++;

    	exeName.assign(pEnd);

    	unsigned int thePos = exeName.rfind('.');
    	if (thePos < 0 || std::string::npos == thePos)
    	{// if there isn't '.' in filename, cut last character for different
    		thePos = exeName.length() - 1;
    	}
    	
    	exeMainName = exeName.substr(0, thePos);

    	*pEnd = '\0';

    	return string(pathBuf);
    }
    
    RDS_UINT64 SegBaseGetThreadID()
    {
#ifdef _WIN32
    	return ::GetCurrentThreadId();
#elif  __ANDROID__
    	return pthread_self();
#else
    	//修复Arm/Android下获取线程ID有误
    	return syscall(SYS_gettid);
#endif
    }
    std::string GetCurProcessName()
	{
		std::string strapp;
#ifdef _WIN32
		char szPath[MAX_PATH * 2] = { 0 };
		//GetCurrentDirectoryA(MAX_PATH, szPath);
		GetModuleFileNameA(NULL, szPath, MAX_PATH);
		strapp = szPath;
		//strapp = strapp.substr(strapp.find_last_of('\\'));

        strapp = strapp.substr(strapp.rfind("\\") + 1);
		strapp = strapp.substr(0, strapp.rfind("."));	
#else
		char path[PATH_MAX]={0};
		char processname[1024]={0};
		char * processdir=path;
		size_t len=sizeof(path);
		char* path_end;
		if (readlink("/proc/self/exe", path, len) <= 0)
			return "";
		path_end = strrchr(processdir, '/');
		if (path_end == NULL)
			return "";
		++path_end;
		strcpy(processname, path_end);
		*path_end = '\0';
		strapp = processname;
#endif
		return strapp;
	}
#ifndef _WIN32
typedef struct _SYSTEMTIME {
	sg_int16_t wYear;
	sg_int16_t wMonth;
	sg_int16_t wDayOfWeek;
	sg_int16_t wDay;
	sg_int16_t wHour;
	sg_int16_t wMinute;
	sg_int16_t wSecond;
	sg_int16_t wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
	void GetLocalTime(SYSTEMTIME *st)
	{
		if(NULL == st )
			return;
		struct timeval t;
		gettimeofday(&t,NULL);
		struct tm ptm;
		localtime_r(&t.tv_sec, &ptm);

		st->wYear =ptm.tm_year +1900;
		st->wMonth =ptm.tm_mon +1;
		st->wDay =ptm.tm_mday;
		st->wHour =ptm.tm_hour;
		st->wMinute =ptm.tm_min;
		st->wDayOfWeek =ptm.tm_wday;
		st->wSecond =ptm.tm_sec;
		st->wMilliseconds = t.tv_usec/1000;
    }
#endif
bool isDirExists(const string& dirName)
{
#ifdef WIN32
    DWORD ftyp = GetFileAttributesA(dirName.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
    {
        return true;
    }
#else
    struct stat st;
    int iret = lstat(dirName.c_str(), &st);
    if (iret < 0)
    {
        return false;
    }

    if (S_ISDIR(st.st_mode))
    {
        return true;
    }
#endif

    return false;
}
#define MAX_DIR_LEN 256
static bool Create(std::string strDirname)
{
	char DirName[MAX_DIR_LEN] = {'\0'};
#ifdef _WIN32
	char dirchar = '\\';
	std::string dirstring = "\\";
#else
	char dirchar = '/';
	std::string dirstring = "/";
#endif
	strncpy(DirName, strDirname.c_str(), strDirname.length() >= MAX_DIR_LEN ? MAX_DIR_LEN - 1 : strDirname.length());
	int len = strlen(DirName);
	if (DirName[len - 1] != '/' && DirName[len - 1] != '\\')
	{
		strncat(DirName, dirstring.c_str(), dirstring.length());
	}
	len = strlen(DirName);
	for (int i = 1; i < len; i++)
	{
		if (DirName[i] == '/' || DirName[i] == '\\')
		{
			DirName[i] = 0;
#ifndef _WIN32
			if (access(DirName, 0) != 0)
			{
				if(mkdir(DirName,0755) ==-1)
				{
					return false;
				}
			}
#else
			CreateDirectoryA(DirName, NULL);
#endif
			DirName[i] = dirchar;
		}
	}
	return true;
}
#ifndef _WIN32
    int _vscprintf(const char* format, va_list pargs){
        int retval;
        va_list argcopy;
        va_copy(argcopy, pargs);
        retval = vsnprintf(NULL, 0, format, argcopy);
        va_end(argcopy);
        return retval;
    }
#endif
    CSystemLog	CSystemLog::inc;
	CSystemLog&	CSystemLog::GetInstance()
	{
		return inc;
	}
    CSystemLog::CSystemLog()
    {
    	_bstart=false;
    }

    CSystemLog::~CSystemLog()
    {
    }

    const char * lelvelname[] =
    {
    	"system",
    	"error",
    	"warning",
    	"info",
    	"debug",
    	"low_debug",
    };
#define LOGBUFSIZE 1*1024

#define LOG_FILE_LINE_SIZE (256)
    void CSystemLog::WriteLog(std::string mode, LOG_LEVEL level, const char* file_name, int line_no, const char* format, ...)
    {
        char *buf = NULL;
        try
        {
            buf = new char[LOGBUFSIZE];
        }
        catch (std::bad_alloc& ex)
    	{
            std::string strLog;
            char  tmbuf[64] = { 0 };
#ifdef WIN32
    		sprintf_s(tmbuf, sizeof(tmbuf),"[CSystemLog::WriteLog] WriteLog catch exception :%s.\n", ex.what());
#else
            sprintf(tmbuf, "[CSystemLog::WriteLog] WriteLog catch exception :%s.\n", ex.what());
#endif
            strLog = tmbuf;
            CLocalLogOutputStream::GetInstance().PutLogToQueue(strLog);
    		return;
    	}
        if (!buf)
        {
            return ;
        }
    	if (_bstart)
    	{
    		std::string strLog;
    		memset(buf, 0, LOGBUFSIZE);
            va_list argList;
    		va_start(argList, format);
            int iLogContentLen = _vscprintf(format, argList);
            //printf("====iLogContentLen:%d===============\n", iLogContentLen);
            if (iLogContentLen >= LOGBUFSIZE)
            {
                std::string strLog;
                char  tmbuf[64] = { 0 };
#ifdef WIN32
        		sprintf_s(tmbuf, sizeof(tmbuf),"[CSystemLog::WriteLog] Log Content too Large.\n");
#else
                sprintf(tmbuf, "[CSystemLog::WriteLog] Log Content too Large.\n");
#endif
                strLog = tmbuf;
                CLocalLogOutputStream::GetInstance().PutLogToQueue(strLog);
                delete[] buf;
        		return;
            }
            else
            {
#ifdef WIN32
        		int nRet = _vsnprintf(buf, LOGBUFSIZE-1, format, argList);
#else
        		int nRet = vsnprintf(buf, LOGBUFSIZE-1, format, argList);
#endif                
                if (nRet >= LOGBUFSIZE || nRet == -1)
        		{
#ifdef WIN32
        			sprintf_s(buf, sizeof(buf), "Log Content too Large ");
#else
                    sprintf(buf, "Log Content too Large ");
#endif
        		}
            }
    		va_end(argList);
            
			SYSTEMTIME	tm = { 0 };
			GetLocalTime(&tm);
			char  tmbuf[100] = { 0 };
			level = level > CSystemLog::LOG_HIDE ? CSystemLog::LOG_HIDE : level;
#ifdef WIN32
			sprintf_s(tmbuf, sizeof(tmbuf), "[%04d-%02d-%02d %02d:%02d:%02d %03d][%s][%s]", 
                tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds, mode.c_str(), lelvelname[level]);
#else
            sprintf(tmbuf, "[%04d-%02d-%02d %02d:%02d:%02d %03d][%s][%s]", 
                tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds, mode.c_str(), lelvelname[level]);
#endif
			strLog = tmbuf;

			std::string strapp = GetCurProcessName();
			strapp = strapp.substr(strapp.rfind("\\") + 1);
			strapp = strapp.substr(0, strapp.rfind("."));	
			
			char  buffer[64] = { 0 };
#ifdef WIN32
			sprintf_s(buffer, sizeof(buffer), "[%s][%06x]", strapp.c_str(), SegBaseGetThreadID());
#else
            sprintf(buffer, "[%s][%06x]", strapp.c_str(), (unsigned int)SegBaseGetThreadID());
#endif
			strLog += buffer;
            
            char file_line[LOG_FILE_LINE_SIZE + 1] = { 0 }; 
#ifdef WIN32
            _snprintf(file_line, LOG_FILE_LINE_SIZE, "[%s:%d]", file_name, line_no);
#else
            snprintf(file_line, LOG_FILE_LINE_SIZE, "[%s:%d]", file_name, line_no);
#endif
            strLog += file_line;
			strLog += buf;
#ifdef WIN32
			strLog += "\r\n";
#else
            strLog += "\n";
#endif
            if (level <= g_logLevel)
            {
                CLocalLogOutputStream::GetInstance().PutLogToQueue(strLog);
            }
    	}
        else
        {
            //printf("error: _bstart is false.\n");
        }
    	delete[] buf;
    }
    void CSystemLog::Init()
    {
    	if(!_bstart)
    	{
    		_bstart =true;
    	}
    }
    void CSystemLog::Destroy()
    {
    	if(_bstart)
    	{
    		_bstart	=false;
    	}
    }
    CLocalLogOutputStream	CLocalLogOutputStream::ins;
    CLocalLogOutputStream&	CLocalLogOutputStream::GetInstance()
    {
    	return ins;
    }
    CLocalLogOutputStream::CLocalLogOutputStream()
    {
    	m_uiMaxSize = 10; //CID:152970
    	m_bRunning = false; 
    }      
    CLocalLogOutputStream::~CLocalLogOutputStream()
    {
        Stop();
    }   
    string CLocalLogOutputStream::GetCurrentTime()
    {
    	time_t tt = time(NULL);
    	struct tm now_t;
#ifdef _WIN32
    	localtime_s(&now_t, &tt);
#else
    	localtime_r(&tt, &now_t);
#endif
    	char szTime[64] = { '\0' };
    	strftime(szTime, sizeof(szTime), "%Y-%m-%d %H %M %S", &now_t);

    	std::string strTime = szTime;
    	return strTime;
    }
	const int MAXLOGBUFFER = 1024 * 1024 * 3;
    void CLocalLogOutputStream::PutLogToQueue(const string & strLog)
    {
        Lock sync(*this);
        m_strLog += strLog;
        if ((int)m_strLog.size() > MAXLOGBUFFER)
        {
            string str;
            m_vecLogList.push_back(str);
			m_vecLogList.back().swap(m_strLog);
        }
        notifyAll();
    }
    void CLocalLogOutputStream::run()
    {
        while (m_bRunning)
        {
            vector<string> strVecLogList;
            {
                Lock sync(*this);
                if (!m_vecLogList.empty())
                {
                    strVecLogList.swap(m_vecLogList);
                }
                if (!m_strLog.empty())
                {
                    std::string str;
    				strVecLogList.push_back(str);
    				strVecLogList.back().swap(m_strLog);
                }
                if (strVecLogList.empty())
                {
                    wait();
                }
            }

            for (unsigned int i = 0; i < strVecLogList.size(); i++)
            {
                WriteLogToFile(strVecLogList[i]);
            }

        }
        LOG(CSystemLog::LOG_SYSTEM, "[CLocalLogOutputStream::run] thread exit.");
    }
    void CLocalLogOutputStream::WriteLogToFile(const string& strLog)
    {
        if (!m_bRunning)
        {
            return;
        }
        if (!m_strLocLogPath.empty() && !isDirExists(m_strLocLogPath))
        {
            Create(m_strLocLogPath);
        }
    	static bool flag = false;
        std::string strapp = GetCurProcessName();
    	if (!flag)
    	{
    		std::string strTime = GetCurrentTime();
    		m_strLocalLogFile = m_strLogFilePrefix + "_" + g_logNetType + "_" + strapp + "_" + strTime + ".LOG";
            printf("========WriteLogToFile m_strLocalLogFile:%s.\n", m_strLocalLogFile.c_str());
    		flag = true;
    	}

    	static unsigned long fileSize = 0;
    	fileSize += strLog.size();

    	if (fileSize > m_uiMaxSize*1024*1024)
    	{
    	    //zip log 
    	    try
            {
                std::string strLogFile = m_strLocalLogFile;
                std::string strZipFile = strLogFile.substr(0, strLogFile.rfind(".")) + ".zip";
#ifdef _WIN32
                std::string strFileName = strLogFile.substr(strLogFile.rfind("\\") + 1);
#else
                std::string strFileName = strLogFile.substr(strLogFile.rfind("/") + 1);
#endif
                if (m_LogZipFunc)
                {
                    bool bRet = m_LogZipFunc(strZipFile, strLogFile, strFileName, 1);
                    if (!bRet)
                    {
                        LOG(CSystemLog::LOG_ERROR, "[CLocalLogOutputStream::WriteLogToFile] Failed to compress file:[%s].", strZipFile.c_str());
                    }
                }
                else
                {
                    LOG(CSystemLog::LOG_ERROR, "[CLocalLogOutputStream::WriteLogToFile] Failed to compress file:[%s], LogZipFunc is NULL.", strZipFile.c_str());
                }
            }
            catch (...)
            {
                LOG(CSystemLog::LOG_ERROR, "[CLocalLogOutputStream::WriteLogToFile] Failed to compress file:[%s].", m_strLocalLogFile.c_str());
            }
    		std::string strTime = GetCurrentTime();
    		m_strLocalLogFile = m_strLogFilePrefix + "_" + g_logNetType + "_" + strapp + "_" + strTime + ".LOG";
    		fileSize = 0;
    	}

    	std::ofstream ofs;
        ofs.open(m_strLocalLogFile.c_str(), std::ios::out | std::ios::binary | std::ios::app);
        if (ofs.is_open())
        {
            ofs << strLog.c_str();
    	    ofs.close();
        }
        else
    	{
            printf("Failed to open file:%s.\n", m_strLocalLogFile.c_str());
    	}
    }
    void CLocalLogOutputStream::InitLOSLog(std::string strLogPath, std::string strLogFile, int iMaxSize)
    {
        if (!m_bRunning)
            m_bRunning = true;
        else
            return;
        
        m_strLogFilePrefix = strLogFile;
        m_uiMaxSize = iMaxSize;
        m_strLocLogPath = strLogPath;

        this->start();

        //load dll
#ifdef _WIN32
        void* hlib = LoadLibrary("LogZip.dll");
        if (NULL != hlib)
        {
            m_LogZipFunc = (LogZipFunc)::GetProcAddress((HMODULE)hlib, "ZipCompressFile");
            if (NULL != m_LogZipFunc)
            {
                LOG(CSystemLog::LOG_SYSTEM, "[CLocalLogOutputStream::InitLOSLog] Load LogZip.dll Success.");
            }
            else
            {
                LOG(CSystemLog::LOG_ERROR, "[CLocalLogOutputStream::InitLOSLog] Failed to Load LogZip.dll.");
                FreeLibrary((HINSTANCE)hlib);
            }
        }
        else
        {
            UINT nerror = GetLastError();
            LOG(CSystemLog::LOG_ERROR, "[CLocalLogOutputStream::InitLOSLog] Failed to Load LogZip.dll, error:%d.", nerror);
        }
#else
		int flags = RTLD_NOW | RTLD_GLOBAL;// RTLD_LAZY | RTLD_GLOBAL;
		void* hlib = dlopen("libLogZip.so", flags);
		if (NULL != hlib)
		{
			m_LogZipFunc = (LogZipFunc)::dlsym(hlib, "ZipCompressFile");
			if (NULL != m_LogZipFunc)
			{
				printf(" Load LogZip.so Success.");
                LOG(CSystemLog::LOG_SYSTEM, "[CLocalLogOutputStream::InitLOSLog] Load LogZip.dll Success.");
			}
			else
			{
				const char * pszError = dlerror();
				printf(" Failed to dlsym LogZip.so. error[%s]", pszError);
                LOG(CSystemLog::LOG_ERROR, "[CLocalLogOutputStream::InitLOSLog] Failed to Load LogZip.dll[%s].",
                    pszError);
			}
		}
		else
		{
			const char * pszError = dlerror();
			printf("[PUCLoggerServerI::Start] Failed to dlopen LogZip.so. error[%s]", pszError);
            LOG(CSystemLog::LOG_ERROR, "[CLocalLogOutputStream::InitLOSLog] Failed to Load LogZip.dll, error:%s.", 
                pszError);
		}
#endif
        return;
    }
    void CLocalLogOutputStream::Stop()
    {
        m_bRunning = false;
    }
    void InitChaosLogFile(const std::string& prefix, const std::string& strLogPath)
    {
        static bool bLogRunning = false;
        if (!bLogRunning)
        {
            bLogRunning = true;
        }
        else
        {
            LOG(CSystemLog::LOG_SYSTEM, "[InitChaosLogFile] Log has been initialed.");
            return;
        }
        string strLocalLogPath = strLogPath;
        string strLogPathWithPrefix;
        if (!strLocalLogPath.empty())
        {
#ifdef OS_WIN
            strLocalLogPath += "\\";
#else
            strLocalLogPath += "/";
#endif
            strLocalLogPath += "IceLog";
            Create(strLocalLogPath);
#ifdef OS_WIN
            strLogPathWithPrefix = strLocalLogPath + "\\" + prefix;
#else
            strLogPathWithPrefix = strLocalLogPath + "/" + prefix;
#endif
        }
    	else
        {
            std::string exeName;
        	std::string exeMainName;
        	std::string startPathName = GetExeDirPath(exeName, exeMainName);

        	std::string crDirName = startPathName;
            crDirName += "IceLog";
            Create(crDirName);
            strLocalLogPath = crDirName;
#ifdef OS_WIN
            strLogPathWithPrefix = crDirName + "\\" + prefix;
#else
            strLogPathWithPrefix = crDirName + "/" + prefix;
#endif
        }   
        printf("========InitChaosLogFile strLocalLogPath:%s, strLogPathWithPrefix:%s.\n", strLocalLogPath.c_str(),
            strLogPathWithPrefix.c_str());
        CSystemLog::GetInstance().Init();
        CLocalLogOutputStream::GetInstance().InitLOSLog(strLocalLogPath, strLogPathWithPrefix, g_logMaxSize); 

    	LOG(CSystemLog::LOG_SYSTEM, "Project Build Time:%s %s.", __DATE__,__TIME__);

    	LOG(CSystemLog::LOG_SYSTEM, "ChaosLogFile for %s Init Complete.", prefix.c_str());
    }

	bool Check_SaveIP(std::string strIP)
	{
		LOG(CRS::CSystemLog::LOG_SYSTEM, "Check_SaveIP() check IP: %s", strIP.c_str());
		bool ret = false;
		//check IP valid
		string tempIP = strIP;
		string subs;
		int PointCount = 0;
		int iIP;
		while (tempIP.find('.') != std::string::npos)
		{
			PointCount++;
			subs = tempIP.substr(0, tempIP.find('.'));
			if (subs.length() > 3)
			{
				LOG(CRS::CSystemLog::LOG_ERROR, "Check_SaveIP() IP[%s] is invalid", strIP.c_str());
				return false;
			}
			iIP = atoi(subs.c_str());//transfer failed return 0
			if (iIP < 0 || iIP >255 || (iIP == 0 && strcmp(subs.c_str(), "0") != 0) || (subs.length() != 1 && subs[0] == '0'))
			{
				LOG(CRS::CSystemLog::LOG_ERROR, "Check_SaveIP() IP[%s] is invalid", strIP.c_str());
				return false;
			}
			if (PointCount == 1 && (iIP > 233 || iIP < 1))//1<= first Section <= 233
			{
				cout << "Check_SaveIP() IP[" << strIP.c_str() << "] is invalid" << endl;
				return false;
			}
			tempIP = tempIP.substr(tempIP.find('.') + 1);
		}
		if (PointCount != 3)
		{
			LOG(CRS::CSystemLog::LOG_ERROR, "Check_SaveIP() IP[%s] format is wrong", strIP.c_str());
			return false;
		}

		//check is or not Broadcast IP
		while (strIP.find_last_of('.') != std::string::npos)
		{
			if (strIP.substr(strIP.find_last_of('.') + 1) != "255")
			{
				break;
			}
			ret = true;
			strIP = strIP.substr(0, strIP.find_last_of('.'));
		}
		if (ret)
		{
			LOG(CRS::CSystemLog::LOG_SYSTEM, "Check_SaveIP() save IP substring: %s", strIP.c_str());
		}
		CRS::g_FilterIps.push_back(strIP);
		return ret;
	}

	void SplitIPString(std::string srcString, std::string splitChar)
	{
		if (srcString == "")
		{
			LOG(CRS::CSystemLog::LOG_SYSTEM, "SplitIPString() Unpublish IP is not Set");
			return;
		}
		vector<string> retVec;
		int pos = 0;
		int posTemp = pos;
		int stPos = 0;	//截取pos

		pos = srcString.find(splitChar, pos);
		//只有一个IP
		if ((pos == -1) && srcString.size() > 0)
		{
			Check_SaveIP(srcString);
			return;
		}

		while (pos != -1)
		{
			stPos = (posTemp == 0 ? 0 : posTemp + 1);
			std::string temp = srcString.substr(stPos, (pos - stPos));
			Check_SaveIP(temp);
			posTemp = pos;
			pos = srcString.find(splitChar, pos + 1);
		}

		//不以";"结尾
		if (posTemp < (int)srcString.size() - 1)
		{
			std::string temp = srcString.substr(posTemp + 1);
			Check_SaveIP(temp);
		}
	}
}

