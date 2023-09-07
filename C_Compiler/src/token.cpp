#include "token.h"
#include <string>
#include <stdarg.h>
namespace {
	void error(const char* fmt...) {
		va_list ap;
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, "\n");
		exit(1);
	}
}

int Token::expectNumber() const {
	if (mType != Token::TokenType::Num) {
		error("���ł͂���܂���");
	}
	return mVal;
}

bool Token::expect(char op) const{
	if (mType != Token::TokenType::Reserved || mStr[0] != op) {
		error("'%c'�ł͂���܂���");
	}
	return true;
}
