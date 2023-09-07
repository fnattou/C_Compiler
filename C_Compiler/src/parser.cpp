#pragma once
#include "parser.h"

Parser::Node* Parser::PushBackNode(Node n) {
	mNodeTbl.push_back(n);
	return &mNodeTbl.at(mNodeTbl.size() - 1);
}


Parser::Node* Parser::Expr() {
	Node* node = Mul();
	while (mCurrentPos < mTokenTbl.size()) {
		Token& currentToken = mTokenTbl[mCurrentPos];
		if (currentToken.isOperator('+')) {
			++mCurrentPos;
			node = PushBackNode(Node{ nodeType::Add, node, Mul() });
		}
		else if (currentToken.isOperator('-')) {
			++mCurrentPos;
			node = PushBackNode(Node{ nodeType::Sub, node, Mul() });
		}
		else {
			return node;
		}
	}
	return node;
}

Parser::Node* Parser::Relational() {
	Node* node = Add();
	while (mCurrentPos < mTokenTbl.size()) {

	}
}

Parser::Node* Parser::Add() {

}
 
Parser::Node* Parser::Mul() {
	Node* node = Unary();
	while (mCurrentPos < mTokenTbl.size()) {
		Token& currentToken = mTokenTbl[mCurrentPos];
		if (currentToken.isOperator('*')) {
			++mCurrentPos;
			node = PushBackNode(Node{ nodeType::Mul, node, Unary() });
		}
		else if (currentToken.isOperator('/')) {
			++mCurrentPos;
			node = PushBackNode(Node{ nodeType::Div, node, Unary() });
		}
		else {
			return node;
		}
	}
	return node;
}

Parser::Node* Parser::Primaly() {
	//次のトークンが"("なら、"(" expr ")"のはず	
	Token& t = mTokenTbl[mCurrentPos++];
	if (t.isOperator('(')) {
		Node* node = Expr();
		mTokenTbl[mCurrentPos++].expect(')');
		return node;
	}

	// そうでなければ数値のはず
	return PushBackNode(Node{ nodeType::Num, nullptr, nullptr, t.expectNumber()});
}

Parser::Node* Parser::Unary() {
	Token& t = mTokenTbl[mCurrentPos];
	if (t.isOperator('+')) {
		++mCurrentPos;
		return Primaly();
	}
	if (t.isOperator('-')) {
		++mCurrentPos;
		Node* node = PushBackNode(Node{ nodeType::Num, nullptr, nullptr, 0 });
		return PushBackNode(Node{ nodeType::Sub, node, Primaly() });
	}
	return Primaly();
}

void Parser::Parse(vector<Token>& tokenTbl) {
	mTokenTbl = tokenTbl;
	//Fix me : ポインタでlhs, rhsを保存しているため、サイズ拡張時にmoveが発生して壊れる。
	//回避策として多めにとっておく
	mNodeTbl.reserve(tokenTbl.size() * 10);
	if (mTokenTbl.size() > 0) {
		Expr();
	}
}

