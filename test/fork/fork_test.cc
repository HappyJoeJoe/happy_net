#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int main()
{
	for (int i = 0; i < 4; ++i)
	{
		fork();

		// printf("-\n"); /* 6个 -, 带换行 */
		// printf("-");   /* 8个 - */
	}

	// wait(NULL);
	// wait(NULL);

	return 0;
}