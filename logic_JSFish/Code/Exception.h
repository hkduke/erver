#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__
///////////////////////////////////////////////////////////////////////////////////

#include "Prereqs.h"

///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
class base_error : public std::exception 
{
public:
    inline base_error(const std::string& msg, int line, const char *file):msg_(msg), line_(line), file_(file){} 
    virtual ~base_error() throw() {} 

	virtual int line() const throw() { return line_; } 
    virtual const char* file() const throw() { return file_; }  

    virtual const char* what() const throw()    { return msg_.c_str(); } 
    void set(const std::string& msg)            { msg_ = msg; }

private: 
    std::string msg_;
	const int line_;
	const char* file_;
};

class blocker_error : public base_error
{
public:
    blocker_error(const std::string& msg, int line, const char *file) : base_error(msg, line, file) {}
};

class critical_error : public base_error 
{
public:
    critical_error(const std::string& msg, int line, const char *file) : base_error(msg, line, file) {}
};

class major_error : public base_error 
{
public:
    major_error(const std::string& msg, int line, const char *file) : base_error(msg, line, file) {}
};

class normal_error : public base_error
{
public:
    normal_error(const std::string& msg, int line, const char *file) : base_error(msg, line, file) {}
};

class minor_error : public base_error
{
public:
    minor_error(const std::string& msg, int line, const char *file) : base_error(msg, line, file) {}
};

class enhancement_error : public base_error
{
public:
    enhancement_error(const std::string& msg, int line, const char *file) : base_error(msg, line, file) {}
};

#if _DEBUG

#define throw_blocker_error(str) throw( blocker_error(str, __LINE__, __FILE__) )
#define throw_critical_error(str) throw( critical_error(str, __LINE__, __FILE__) )
#define throw_major_error(str) throw( major_error(str, __LINE__, __FILE__) )
#define throw_normal_error(str) throw( normal_error(str, __LINE__, __FILE__) )
#define throw_minor_error(str) throw( minor_error(str, __LINE__, __FILE__) )
#define throw_enhancement_error(str) throw( enhancement_error(str, __LINE__, __FILE__) )

#else

#define throw_blocker_error(str) //throw( blocker_error(str, __LINE__, __FILE__) )
#define throw_critical_error(str) //throw( critical_error(str, __LINE__, __FILE__) )
#define throw_major_error(str) //throw( major_error(str, __LINE__, __FILE__) )
#define throw_normal_error(str) //throw( normal_error(str, __LINE__, __FILE__) )
#define throw_minor_error(str) //throw( minor_error(str, __LINE__, __FILE__) )
#define throw_enhancement_error(str) //throw( enhancement_error(str, __LINE__, __FILE__) )

#endif

///////////////////////////////////////////////////////////////////////////////

#endif