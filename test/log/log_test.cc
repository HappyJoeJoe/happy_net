#include "../../src/log.h"

using namespace std;

int main(int argc, char* argv[])
{
	log g_log;
	g_log.level_log(kLOG_INFO, "%d\n", 123);
	return 0;
}