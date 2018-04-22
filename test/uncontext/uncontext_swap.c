#include <stdio.h>  
#include <ucontext.h>  
#include <unistd.h>

int num_f = 0;
int num_g = 0;
int num_main = 0;
ucontext_t context_f;
ucontext_t context_g;
ucontext_t context_main;

void func()
{
	printf("func() begin\n");
	swapcontext(&context_f, &context_g);
	printf("func() end\n");
}

void gunc()
{
	printf("gunc() begin\n");
	swapcontext(&context_g, &context_f);
	printf("gunc() end\n");
}

int main(int argc, const char *argv[]){

	char stack_f[1024*128];
    char stack_g[1024*128];

    getcontext(&context_f);
    num_f++;
    printf("--> f:%d\n", num_f);
    context_f.uc_stack.ss_sp		= &stack_f;
    context_f.uc_stack.ss_size 		= sizeof(stack_f);
    context_f.uc_link				= &context_g;
    makecontext(&context_f, func, 0);

    getcontext(&context_g);
    num_g++;
    printf("--> g:%d\n", num_g);
    context_g.uc_stack.ss_sp		= &stack_g;
    context_g.uc_stack.ss_size 		= sizeof(stack_g);
    context_g.uc_link				= &context_main;
    makecontext(&context_g, gunc, 0);

    swapcontext(&context_main, &context_f);
    
    return 0;  
}