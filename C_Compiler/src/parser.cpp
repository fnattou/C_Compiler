#pragma once
#include "parser.h"

Parser::Node* Parser::PushBackNode(Node n) {
	mNodeTbl.push_back(n);
	return &mNodeTbl.at(mNodeTbl.size() - 1);
}

Parser::nodeType Parser::GetNodeType(const Token& token) const {
	if (token.isReserved('+')) {
		return nodeType::Add;
	}
	else if (token.isReserved('-')) {
		return nodeType::Sub;
	}
	else if (token.isReserved('*')) {
		return nodeType::Mul;
	}
	else if (token.isReserved('/')) {
		return nodeType::Div;
	}
	else if (token.isReserved('>')) {
		return nodeType::Gt;
	}
	else if (token.isReserved('<')) {
		return nodeType::Lt;
	}
	else if (token.isReserved("<=")) {
		return nodeType::Le;
	}
	else if (token.isReserved(">=")) {
		return nodeType::Ge;
	}
	else if (token.isReserved("==")) {
		return nodeType::Eq;
	}
	else if (token.isReserved("!=")) {
		return nodeType::Ne;
	}
	else if (token.isReserved("if")) {
		return nodeType::If_;
	}
	else if (token.isReserved("for")) {
		return nodeType::For_;
	}
	else if (token.isReserved("while")) {
		return nodeType::While_;
	}
	else if (token.isReturn()) {
		return nodeType::Return;
	}
	else if (token.isReserved('{')) {
		return nodeType::Block;
	}
	return nodeType::None;
}

void Parser::Program() {
	while (mCurrentPos < mTokenTbl.size()) {
		Node* node = Statement();
		mRootNodeTbl.push_back(node);
	}
}

Parser::Node* Parser::Statement() {
	Node* node;
	const auto expectAndNext = [&](char c) { getCurTk().expect(c); ++mCurrentPos; };
	switch ( auto type = GetNodeType(getCurTk()) ) {
	case nodeType::Return:
		++mCurrentPos;
		node = PushBackNode({ .type = type, .lhs = Expr() });
		expectAndNext(';');
		break;

	// if ( lhs ) middle  else ( rhs )
	case nodeType::If_: {
		++mCurrentPos;
		Node n{ .type = type };
		expectAndNext('(');
		n.lhs = Expr();
		expectAndNext(')');
		n.middle = Statement();
		if (mCurrentPos < mTokenTbl.size() && getCurTk().isReserved("else")) {
			++mCurrentPos;
			n.rhs = Statement();
		}
		node = PushBackNode(n);
		break;
	}

	// for ( lhs ; middle ; rhs ) { statement }
	case nodeType::For_: {
		++mCurrentPos;
		Node n{ .type = type };
		const auto readExprIf = [&](Node*& n, char c) {
			if (!mTokenTbl[mCurrentPos + 1].isReserved(c)) {
				n = Expr();
			}
			expectAndNext(c);
			};
		expectAndNext('(');
		readExprIf(n.lhs, ';');
		readExprIf(n.middle, ';');
		readExprIf(n.rhs, ')');
		node = PushBackNode(n);
		break;
	}
	case nodeType::While_:
		++mCurrentPos;
		expectAndNext('(');
		node = PushBackNode({ .type = type, .lhs = Expr() });
		expectAndNext(')');
		node->rhs = Statement();
		break;
	case nodeType::Block: {
		++mCurrentPos;
		Node n{ .type = type };
		while (!getCurTk().isReserved('}')) {
			n.innerBlockNodeTbl.push_back(Statement());
		}
		expectAndNext('}');
		node = PushBackNode(n);
	}
	default:
		node = Expr();
		expectAndNext(';');
		break;
	}
	return node;
}

Parser::Node* Parser::Expr() {
	return Assign();
}

Parser::Node* Parser::Assign() {
	Node* node = Equality();
	if (getCurTk().isReserved('=')) {
		++mCurrentPos;
		node = PushBackNode(Node{ nodeType::Assign,  node, Assign() });
	}
	return node;
}

Parser::Node* Parser::Equality() {
	Node* node = Relational();
	while (!isEndOfState()) {
		switch (auto type = GetNodeType(getCurTk())) {
		case nodeType::Eq:
		case nodeType::Ne:
			++mCurrentPos;
			node = PushBackNode(Node{ type, node, Relational() });
			continue;
		default:
			return node;
		}
	}
	return node;
}

Parser::Node* Parser::Relational() {
	Node* node = Add();
	while (!isEndOfState()) {
		switch (auto type = GetNodeType(getCurTk())) {
		case nodeType::Lt:
		case nodeType::Le:
		case nodeType::Gt:
		case nodeType::Ge:
			++mCurrentPos;
			node = PushBackNode(Node{ type, node, Add() });
			continue;
		default:
			return node;
		}
	}
	return node;
}

Parser::Node* Parser::Add() {
	Node* node = Mul();
	while (!isEndOfState()) {
		switch (auto type = GetNodeType(getCurTk())) {
		case nodeType::Add:
		case nodeType::Sub:
			++mCurrentPos;
			node = PushBackNode(Node{ type, node, Mul() });
			continue;
		default:
			return node;
		}
	}
	return node;
}
 
Parser::Node* Parser::Mul() {
	Node* node = Unary();
	while (!isEndOfState()) {
		switch (auto type = GetNodeType(getCurTk())) {
		case nodeType::Mul:
		case nodeType::Div:
			++mCurrentPos;
			node = PushBackNode(Node{ type, node, Unary() });
			continue;
		default:
			return node;
		}
	}
	return node;
}

Parser::Node* Parser::Unary() {
	switch (GetNodeType(getCurTk())) {
	case nodeType::Add:
		++mCurrentPos;
		return Primaly();
	case nodeType::Sub:   //単項-のときには0－Numに変換する
		++mCurrentPos;
		{
		Node* node = PushBackNode(Node{ nodeType::Num, nullptr, nullptr, 0 });
		return PushBackNode(Node{ nodeType::Sub, node, Primaly() });
		}
	default:
		return Primaly();
	}
}

Parser::Node* Parser::Primaly() {
	//次のトークンが"("なら、"(" expr ")"のはず	
	Token& t = mTokenTbl[mCurrentPos++];
	if (t.isReserved('(')) {
		Node* node = Expr();
		mTokenTbl[mCurrentPos++].expect(')');
		return node;
	}
	else if (t.isIdent()) {
		if (mLValMap.contains(t.mStr)) {
			return PushBackNode(Node{ .type = nodeType::LocalVal, .offset =  mLValMap.at(t.mStr)});
		}
		else {
			const int ofs = (mLValMap.size() + 1) * 8;
			mLValMap.emplace(t.mStr, ofs);
			return PushBackNode(Node{ .type = nodeType::LocalVal, .offset = ofs });
		}
	}

	// そうでなければ数値のはず
	return PushBackNode(Node{ .type = nodeType::Num, .val = t.expectNumber()});
}


void Parser::Parse(vector<Token>& tokenTbl) {
	mTokenTbl = tokenTbl;
	//Fix me : ポインタでlhs, rhsを保存しているため、サイズ拡張時にmoveが発生して壊れる。
	//回避策として多めにとっておく
	mNodeTbl.reserve(tokenTbl.size() * 10);
	if (mTokenTbl.size() > 0) {
		Program();
	}
}

