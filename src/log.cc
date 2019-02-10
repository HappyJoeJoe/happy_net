#include "log.h"

void log::level_log_raw(const char* msg)
{
	FILE *fp;

	fp = fopen(log_path, "a");
	if(!fp) return;

	fprintf(fp,"%s", msg);

	fflush(fp);
	fclose(fp);
}

void log::level_log(int level, const char* fmt, ...)
{
	va_list ap;
    char msg[LOG_MAX_LEN];
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    level_log_raw(msg);
}