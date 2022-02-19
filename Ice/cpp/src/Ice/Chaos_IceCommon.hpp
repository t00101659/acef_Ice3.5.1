// FileName         : Chaos_IceCommon.hpp
// Date of Creation : 2017-08-28
// Description      : 
// Author		    : GuanChao
//____________________________________________________________________

#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_WINDOWS) // CHECK OS

#define	OS_WIN

#if defined(WIN32) || defined(_WIN32)
#define OS_WIN32
#define OS_32BIT
#elif defined(WIN64) || defined(_WIN64)
#define OS_WIN64
#define OS_64BIT
#endif

#elif defined(__ANDROID__)

#else

#define	OS_LINUX // assume linux

#ifndef __WORDSIZE
#include <bits/wordsize.h>
#endif // !__WORDSIZE
#ifndef __WORDSIZE
#error "Unknown system, only support Windows/Linux."
#endif // !__WORDSIZE

#if (__WORDSIZE == 32)
#define OS_LINUX32
#define OS_32BIT
#elif (__WORDSIZE == 64)
#define OS_LINUX64
#define OS_64BIT
#else
#error "Support 32 or 64 bit system only."
#endif

#endif //  CHECK OS

#ifdef		OS_WIN
#include <mutex>
#include <fstream>
#include <vector>
#endif


// base data type define:
typedef     signed char             Sint8;
typedef     signed short            Sint16;
typedef     unsigned char           Uint8;
typedef     unsigned short          Uint16;
typedef     signed int				Sint32;
typedef     unsigned int			Uint32;

#define		_V_(_n_, _v_)		(_v_)

using namespace::std;
namespace CRS
{
#define RDS_UINT32		unsigned int
#define RDS_INT32		int 
#define	RDS_BYTE		unsigned char

#define RDS_UINT64		unsigned long long
#define RDS_INT64		signed long long

#define sg_uint32_t unsigned int
#define sg_uint16_t unsigned short
#define sg_uint8_t unsigned char
#define sg_int32_t int
#define sg_int16_t short
#define sg_int8_t char
#define sg_bool_t bool
#define sg_void_t void
#define sg_ulong_t unsigned long
#define sg_long_t long
#define sg_handle_t void *

typedef bool (*LogZipFunc)(const std::string zipFilePath, const std::string srcFilePath, const std::string srcFileName, const int level);

extern std::string          g_logNetType;
extern unsigned int         g_logMaxSize;
extern int                  g_logLevel;
class ICE_API CSystemLog
{
public:
	enum LOG_LEVEL{LOG_SYSTEM = 0,LOG_ERROR,LOG_WARNING,LOG_INFO,LOG_DEBUG,LOG_HIDE};
    static CSystemLog	inc;
	static CSystemLog&	GetInstance();
	virtual ~CSystemLog(void);
#ifdef WIN32
    void WriteLog(std::string mode, LOG_LEVEL level, const char* file_name, int line_no, const char* format, ...);
#else
    void WriteLog(std::string mode, LOG_LEVEL level, const char* file_name, int line_no, const char* format, ...) __attribute__((format(printf, 6, 7)));
#endif
	void Init();
	void Destroy();
protected:
	CSystemLog(void);
	bool		_bstart;
};
class CLocalLogOutputStream : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:
	static CLocalLogOutputStream ins;
	static CLocalLogOutputStream& GetInstance();
	CLocalLogOutputStream();
	~CLocalLogOutputStream();    
    virtual void run();
    void WriteLogToFile(const string& strLog);
    string GetCurrentTime();
    void PutLogToQueue(const string& strLog);
    void InitLOSLog(std::string strLogPath, std::string strLogFile, int iMaxSize);
    void Stop();

private:
	RDS_UINT32 m_uiMaxSize;
    string m_strLocalLogFile;
    string m_strLogFilePrefix;
    bool m_bRunning;
    string m_strLocLogPath;
    vector<string> m_vecLogList;

    string m_strLog;

    LogZipFunc m_LogZipFunc;
};
	extern void		InitChaosLogFile(const std::string& prefix, const std::string& strLogPath);
	extern void		SplitIPString(std::string strCfgIP, std::string splitChar);
	extern bool		Check_SaveIP(std::string strIP);
	extern vector<string>          g_FilterIps;


}

// flag 1: include file name and line
#define			INNER_OUTPUT_LOG(_flag_, _str_)			

#define LOG(level, stringFormat, ...) CRS::CSystemLog::GetInstance().WriteLog("Ice", level, __FILE__, __LINE__, stringFormat, ##__VA_ARGS__)



#define		OUTPUT_LOG_NOR		INNER_OUTPUT_LOG

#define		NO_LOG(_flag_, _str_)	

// phase output log
#define		OUTPUT_LOG_A			NO_LOG

#define		OUTPUT_LOG_B			NO_LOG

#define		OUTPUT_LOG_C			INNER_OUTPUT_LOG

