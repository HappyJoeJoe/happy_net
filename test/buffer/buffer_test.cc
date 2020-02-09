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

    cout << "reverse: " 
         << b.peek_reserve() 
         << " obj:" 
         << sizeof(obj)
         << endl;

	b.append_string(str);

    cout << "reverse: " 
         << b.peek_reserve() 
         << " len:"
         << len
         << endl;

	obj tmp;
	b.get<obj>(tmp);

    cout << "reverse: " 
         << b.peek_reserve() 
         << endl;

	cout << "age:"    << tmp.age    << " "
	     << "str:"    << tmp.c      << " "
	     << "height:" << tmp.height << " "
	     << "ptr:"    << tmp.ptr    << " "
	     << endl;

    char buf[len+1];
    b.get_string(len, buf);
    cout << "buf:[" << buf << "]" << endl;

    cout << "reverse: " 
         << b.peek_reserve() 
         << endl;

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

    cout << "reverse: " 
         << bb.peek_reserve() 
         << endl;

    char c_a;
    char c_b;
    char c_c;
    char c_d;
    bb.get<char>(c_a);
    bb.get<char>(c_b);
    bb.get<char>(c_c);
    bb.get<char>(c_d);

    cout << "reverse: " 
         << bb.peek_reserve() 
         << endl;

    cout << "a:[" << c_a << "] " 
         << "b:[" << c_b << "] " 
         << "c:[" << c_c << "] " 
         << "d:[" << c_d << "] " 
         << endl;

    const char* str1 = "123456789";
    size_t len2 = strlen(str1);
    bb.append_string(str1);

    cout << "reverse: " 
         << bb.peek_reserve() 
         << endl;

    //4 表示'e', 'f', 'g', 'h'四个字节
    char buf1[4 + len2 + 1];
    buf1[4 + len2] = '\0';
    bb.get_string(4 + len2, buf1);
    cout << "buf1:[" << buf1 << "]" << endl;
    cout << "buf1[" << 4 + len2 << "]:" << buf1[4 + len2] << endl;

    cout << "reverse: " 
         << bb.peek_reserve() 
         << endl;

	return 0;
}