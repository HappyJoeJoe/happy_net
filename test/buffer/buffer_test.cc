#include "../../src/buffer.h"
#include <iostream>

using namespace std;

typedef struct obj
{
	int age;
	char c;
	long height;
	char* ptr;
} obj;

int main(int argc, char const *argv[])
{
	obj o;
	o.age = 20;
	o.c = 'a';
	o.height = 200000;
	o.ptr = const_cast<char*>("hello world!");

	const char* str = "I'm very happy";
	size_t len = strlen(str);

	buffer b;
	b.append<obj>(o);
	b.append_string(str);

	obj tmp;
	b.get<obj>(tmp);
	cout << "age:"    << tmp.age    << " "
	     << "str:"    << tmp.c      << " "
	     << "height:" << tmp.height << " "
	     << "ptr:"    << tmp.ptr    << " "
	     << endl;

    char buf[len+1];
    b.get_string(len, buf);
    cout << "buf:[" << buf << "]" << endl;

    cout << "------------------------------------" << endl;

    buffer bb;
    bb.append<char>('a');
    bb.append<char>('b');
    bb.append<char>('c');
    bb.append<char>('d');
    bb.append<char>('e');
    bb.append<char>('f');
    bb.append<char>('g');
    bb.append<char>('h');

    char c_a;
    char c_b;
    char c_c;
    char c_d;
    bb.get<char>(c_a);
    bb.get<char>(c_b);
    bb.get<char>(c_c);
    bb.get<char>(c_d);

    cout << "a:[" << c_a << "] " 
         << "b:[" << c_b << "] " 
         << "c:[" << c_c << "] " 
         << "d:[" << c_d << "] " 
         << endl;

    const char* str1 = "123456789";
    size_t len2 = strlen(str1);
    bb.append_string(str1);

    //4 表示'e', 'f', 'g', 'h'四个字节
    char buf1[4 + len2 + 1];
    bb.get_string(4 + len2, buf1);
    cout << "buf1:[" << buf1 << "]" << endl;


	return 0;
}