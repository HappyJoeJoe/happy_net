#include "../../src/buffer.h"
#include <iostream>

using namespace std;

struct obj
{
	int age;
	char c;
	long height;
};

int main(int argc, char const *argv[])
{
	struct obj o;
	o.age = 20;
	o.c = 'a';
	o.height = 200000;

	const char* str = "I'm very happy";
	size_t len = strlen(str);

	buffer b;
	b.append<struct obj>(o);
	b.append_string(str);

	struct obj tmp;
	b.get<struct obj>(tmp);

	cout << "age:"    << tmp.age << " "
	     << "str:"    << tmp.c   << " "
	     << "height:" << tmp.height << endl;

    char buf[len+1];
    b.get_string(len, buf);
    cout << "buf:[" << buf << "]" << endl;

	return 0;
}