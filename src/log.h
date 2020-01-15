#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdarg.h>

#define LOG_MAX_LEN 1024
#define default_log_path "ser.log"


/* 日志等级 */
enum log_level
{
	kLOG_DEBUG      = 1,
	kLOG_INFO       = 2,
	kLOG_WARN       = 3,
	kLOG_ERR        = 4,
};

class log
{
public:
	log() {}
	~log() {}

	void level_log_raw(const char* msg);
	
	void level_log(int level, const char* fmt, ...);

	void set_log_path(const char* path)
	{
		log_path = const_cast<char*>(path);
	}

private:
	char* log_path;
};

#endif