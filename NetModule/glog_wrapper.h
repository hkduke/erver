#ifndef GLOG_WRAPPER_H
#define GLOG_WRAPPER_H

#ifdef _WIN32
#define GLOG_NO_ABBREVIATED_SEVERITIES
#define GOOGLE_GLOG_DLL_DECL
#endif // _WIN32

#include <string>
#include <glog/logging.h>
#ifdef _WIN32  
#include <io.h>                      //C (Windows)    access  
#else  
#include <unistd.h>                  //C (Linux)      access     
#endif  

template<typename T>
std::string packString(const T &value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}
template<typename T, typename... Args>
std::string packString(const T &value, Args... args) {
	std::stringstream ss;
	ss << value << packString(args...);
	return ss.str();
}

const std::string MKDIR = "mkdir -p ";

enum LOG_WRAPPER_ERROR
{
	HAVE_INITED_ERROR = -1,
};

//glog wrapper class
class GlogWrapper
{
public:
    //Constructor
    GlogWrapper(const char* log_dir, const char* app_name)
        : m_inited(false)
    {
		google::InitGoogleLogging(app_name);

        initGlog(log_dir, app_name);
    }

    //Destructor
    ~GlogWrapper()
    {
		if (!m_inited)
		{
			return;
		}

        google::ShutdownGoogleLogging();
    }

	int initGlog(const char* log_dir, const char* app_name)
	{
		if(m_inited)
		{
			return HAVE_INITED_ERROR;
		}

		//Make dir
		if(strlen(log_dir) > 0)
		{
#ifdef _WIN32  
			if (_access(log_dir, 0) != 0)
#else
			if (access(log_dir, 0) != 0)
#endif
			{
				int nRet = system((MKDIR + log_dir).c_str());
				printf("The value returned was: %d.\n", nRet);
			}

		}

		//Init log setting
		FLAGS_logtostderr = false;                                      //Default not to log to stderr

		if(strlen(log_dir) > 0)
		{
			FLAGS_log_dir = (NULL == log_dir) ? "unknow" : log_dir;         //Log dir
		}

		FLAGS_colorlogtostderr = true;                                  //Color log to stderr
		FLAGS_stderrthreshold = google::GLOG_INFO;                     //Defalt log FATAL level to stderr

		FLAGS_logbufsecs = 30;                                          //Default log buffer seconds = 30s
		FLAGS_max_log_size = 500;                                       //Max log file size = 500M
		FLAGS_stop_logging_if_full_disk = true;                         //Stop logging id disk is full

		m_inited = true;

		return 0;
	}

    //Set log buffer seconds
    void SetLogBufferSeconds(const int time)
    {
        FLAGS_logbufsecs = time;
    }
	
	void setLogDir(const char* log_dir)
	{
		//Make dir
		system((MKDIR + log_dir).c_str());
		FLAGS_log_dir = (NULL == log_dir) ? "unknow" : log_dir;         //Log dir
	}

    //Set only log to screen
    void SetOnlyLogScreen(const bool log_screen)
    {
        FLAGS_logtostderr = true;
    }

    //Set stderr logging level
    void SetLogScreenLevel(const google::LogSeverity level)
    {
        FLAGS_stderrthreshold = level;
    }
    void SetLogFileLevel(const google::LogSeverity level)
    {
        FLAGS_minloglevel = level;
    }

    //Set log destination
    void SetLogDestination(const google::LogSeverity level, const char* prefix)
    {
        google::SetLogDestination(level, prefix);
    }

protected:
    //Forbid default constructor
    GlogWrapper()
	{
	}

protected:
	bool m_inited;
};

#endif
