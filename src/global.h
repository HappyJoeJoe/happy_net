#ifndef __GLOBAL_H__
#define __GLOBAL_H__

class log* get_log_instance();

#define debug_log(fmt, ...) get_log_instance()->level_log(kLOG_DEBUG, fmt, ##__VA_ARGS__)
#define info_log(fmt, ...)  get_log_instance()->level_log(kLOG_INFO,  fmt, ##__VA_ARGS__)
#define warn_log(fmt, ...)  get_log_instance()->level_log(kLOG_WARN,  fmt, ##__VA_ARGS__)
#define err_log(fmt, ...)   get_log_instance()->level_log(kLOG_ERR,   fmt, ##__VA_ARGS__)

#include "log.h"

#endif