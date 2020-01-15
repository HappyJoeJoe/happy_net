#include "global.h"

class log* get_log_instance()
{
	static class log* l = nullptr;
	if (l == nullptr)
	{
		l = new log();
		l->set_log_path(default_log_path);
	}
	
	return l;
}