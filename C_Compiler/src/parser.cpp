#pragma once
#include "parser.h"

Parser::Node* Parser::PushBackNode(Node n) {
	mNodeTbl.push_back(n);
	return &mNodeTbl.at(mNodeTbl.size() - 1);
}

Parser::nodeType Parser::GetNodeType(Token& token) {
	if (token.isOperator('+')) {
		return nodeType::Add;
	}
	else if (token.isOperator('-')) {
		return nodeType::Sub;
	}
	else if (token.isOperator('*')) {
		return nodeType::Mul;
	}
	else if (token.isOperator('/')) {
		return nodeType::Div;
	}
	else if (token.isOperator('>')) {
		return nodeType::Lt;
	}
	else if (token.isOperator('<')) {
		return nodeType::Gt;
	}
	else if (token.isOperator("<=")) {
		return nodeType::Le;
	}
	else if (token.isOperator(">=")) {
		return nodeType::Ge;
	}
	else if (token.isOperator("==")) {
		return nodeType::Eq;
	}
	else if (token.isOperator("!=")) {
		return nodeType::Ne;
	}
	return nodeType::None;
}


Parser::Node* Parser::Expr() {
	Node* node = Relational();
	while (mCurrentPos < mTokenTbl.size()) {
		switch (auto type = GetNodeType(mTokenTbl[mCurrentPos])) {
			case nodeType::Eq:
			case nodeType::Ne:
				++mCurrentPos;
				node = PushBackNode(Node{ type, node, Relational() });
			default:
				return node;
		}
	}
	return node;
}

Parser::Node* Parser::Relational() {
	Node* node = Add();
	while (mCurrentPos < mTokenTbl.size()) {
		switch (auto type = GetNodeType(mTokenTbl[mCurrentPos])) {
		case nodeType::Lt:
		case nodeType::Le:
		case nodeType::Gt:
		case nodeType::Ge:
			++mCurrentPos;
			node = PushBackNode(Node{ type, node, Add() });
		default:
			return node;
		}
	}
	return node;
}

Parser::Node* Parser::Add() {
	Node* node = Mul();
	while (mCurrentPos < mTokenTbl.size()) {
		switch (auto type = GetNodeType(mTokenTbl[mCurrentPos])) {
		case nodeType::Add:
		case nodeType::Sub:
			++mCurrentPos;
			node = PushBackNode(Node{ type, node, Mul() });
		default:
			return node;
		}
	}
	return node;
}
 
Parser::Node* Parser::Mul() {
	Node* node = Unary();
	while (mCurrentPos < mTokenTbl.size()) {
		switch (auto type = GetNodeType(mTokenTbl[mCurrentPos])) {
		case nodeType::Mul:
		case nodeType::Div:
			++mCurrentPos;
			node = PushBackNode(Node{ type, node, Unary() });
		default:
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
	switch (GetNodeType(mTokenTbl[mCurrentPos])) {
	case nodeType::Add:
		++mCurrentPos;
		return Primaly();
	case nodeType::Sub:   //単項-のときには0−Numに変換する
		++mCurrentPos;
		{
		Node* node = PushBackNode(Node{ nodeType::Num, nullptr, nullptr, 0 });
		return PushBackNode(Node{ nodeType::Sub, node, Primaly() });
		}
	default:
		return Primaly();
	}
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

