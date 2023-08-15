#pragma once
#include <string>
using std::string;
class Compiler
{
public:
	Compiler(){}
	~Compiler(){}
	string Compile(string src);
};
