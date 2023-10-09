#pragma once
#include "parser.h"
#include <iostream>

Parser::nodeType Parser::GetNodeType(const Token& token) const {
	if (token.isReturn()) {
		return nodeType::Return;
	}

	static const std::unordered_map<string_view, nodeType>  map = {
		{"+", nodeType::Add}, {"-", nodeType::Sub}, {"*", nodeType::Mul},
		{"/", nodeType::Div}, {">", nodeType::Gt}, {"<", nodeType::Lt},
		{"<=", nodeType::Le}, {">=", nodeType::Ge}, {"==", nodeType::Eq},
		{"!=", nodeType::Ne}, {"if", nodeType::If_}, {"for", nodeType::For_},
		{"while", nodeType::While_}, {"{", nodeType::Block}, {"&", nodeType::Addr},
		{"sizeof", nodeType::Sizeof}
	};
	for (const auto& pair : map) {
		if (token.isReserved(pair.first)) {
			return pair.second;
		}
	}
	return nodeType::None;
}

void Parser::Program() {
	while (mCurrentPos < mTokenTbl.size()) {
			mRootNodeTbl.push_back(Function());
	}
}

Parser::Node* Parser::Function() {
	//返り値の型
	ValTypeInfo* typeInfoPtr = ReadValueType();
	if (!typeInfoPtr) {
		std::cerr << "関数の返り値の型が宣言されていません" << std::endl;
		assert(0);
	}

	Node n{ .type = nodeType::DeclareFunc }; 
	mFuncInfoTbl.emplace( getCurTk().mStr, 
		FuncInfo{ 
			.name = getCurTk().mStr ,
			.returnValTypeInfoPtr = typeInfoPtr
		});
	mCurrentFuncInfoPtr = &mFuncInfoTbl[getCurTk().mStr];
	n.funcInfoPtr = mCurrentFuncInfoPtr;

	//関数名
	if (!getCurTk().isIdent()) {
		std::cerr << "関数宣言から始まっていません" << std::endl;
		assert(0);
	}
	++mCurrentPos;

	//引数部分
	expectAndNext('(');
	while (!nextIfIsReserved(")")) {
		const ValTypeInfo* infoPtr = ReadValueType();
		assert(infoPtr != nullptr);
		const int ofs = mCurrentFuncInfoPtr->pushBackToValMap(
			getCurTk().mStr, infoPtr->getByteSize(), infoPtr
			);
		auto& p = mNodeTbl.emplace_back(Node{
			.type = nodeType::LocalVal, .offset = ofs, .valTypeInfoPtr = infoPtr
			});
		mCurrentFuncInfoPtr->argumentNodeTbl.push_back(&p);
		++mCurrentPos;
	}

	//関数の本体
	expectAndNext('{');
	while (!nextIfIsReserved("}")) {
		Node* node = Statement();
		n.innerBlockNodeTbl.push_back(node);
	}
	return &mNodeTbl.emplace_back(n);
}

Parser::Node* Parser::Statement() {
	Node* node;
	switch ( auto type = GetNodeType(getCurTk()) ) {
	case nodeType::Return:
		++mCurrentPos;
		node = &mNodeTbl.emplace_back(Node{ .type = type, .lhs = Expr() });
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
		if (nextIfIsReserved("else")) {
			n.rhs = Statement();
		}
		node = &mNodeTbl.emplace_back(n);
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
		node = &mNodeTbl.emplace_back(n);
		break;
	}
	case nodeType::While_:
		++mCurrentPos;
		expectAndNext('(');
		node = &mNodeTbl.emplace_back(Node{ .type = type, .lhs = Expr() });
		expectAndNext(')');
		node->rhs = Statement();
		break;
	case nodeType::Block: {
		++mCurrentPos;
		Node n{ .type = type };
		while (!nextIfIsReserved("}")) {
			n.innerBlockNodeTbl.push_back(Statement());
		}
		node = &mNodeTbl.emplace_back(n);
		break;
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
	if (nextIfIsReserved("=")) {
		node = &mNodeTbl.emplace_back(Node{ nodeType::Assign,  node, Assign() });
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
			node = &mNodeTbl.emplace_back(Node{ type, node, Relational() });
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
			node = &mNodeTbl.emplace_back(Node{ type, node, Add() });
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
			node = &mNodeTbl.emplace_back(Node{ type, node, Mul() });
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
			node = &mNodeTbl.emplace_back(Node{ type, node, Unary() });
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
	case nodeType::Sizeof:
		++mCurrentPos;
		{
			//SizeOf内の式を調査し、もととなる型を探す
			Node* node = Unary();
			while (node->type != nodeType::Num && node->type != nodeType::LocalVal) {
				node = node->lhs ? node->lhs : node->rhs;
			}

			//型のサイズを数値として返す
			int size = 0;
			if (node->type == nodeType::LocalVal) {
				size = node->valTypeInfoPtr->getByteSize();
			}
			else if (node->type == nodeType::Num) {
				size = 4;
			}
			else {
				std::cerr << "Sizeof演算子内で型が見つかりません" << std::endl;
				assert(0);
			}
			return &mNodeTbl.emplace_back(Node{ .type = nodeType::Num, .val = size});
		}
	case nodeType::Sub:   //単項-のときには0－Numに変換する
		++mCurrentPos;
		{
		Node* node = &mNodeTbl.emplace_back(Node{ .type = nodeType::Num, .val = 0 });
		return &mNodeTbl.emplace_back(Node{ nodeType::Sub, node, Primaly() });
		}
	case nodeType::Addr:
		++mCurrentPos;
		return &mNodeTbl.emplace_back(Node{ .type = nodeType::Addr, .rhs = Unary() });
	case nodeType::Mul: //getnodeTypeは'*'をMulとして返す, ここではderef
		++mCurrentPos;
		return &mNodeTbl.emplace_back(Node{ .type = nodeType::Deref, .rhs = Unary() });
	default:
		return Primaly();
	}
}

Parser::Node* Parser::Primaly() {
	Token t = mTokenTbl[mCurrentPos];
	//次のトークンが"("なら、"(" expr ")"のはず	
	if (t.isReserved('(')) {
		mCurrentPos++;
		Node* node = Expr();
		mTokenTbl[mCurrentPos++].expect(')');
		return node;
	}

	//新しい変数の宣言
	if (auto* typeInfoPtr = ReadValueType(); typeInfoPtr) {
		//変数名を読む
		t = getCurTk(); ++mCurrentPos;
		//登録された既存の変数に同名のものはないはず
		assert(!mCurrentFuncInfoPtr->lValOfsMap.contains(t.mStr));
		assert(!mCurrentFuncInfoPtr->lValTypeMap.contains(t.mStr));
		//新しく変数を登録
		const int ofs = mCurrentFuncInfoPtr->pushBackToValMap(
			t.mStr, typeInfoPtr->getByteSize(), typeInfoPtr
			);

		return &mNodeTbl.emplace_back(Node{ 
			.type = nodeType::LocalVal, 
			.offset = ofs , 
			.valTypeInfoPtr = typeInfoPtr
			});
	}

	if (t.isIdent()) {
		mCurrentPos++;
		//"(" が続くなら関数呼び出し
		if (nextIfIsReserved("(")) {
			assert(mFuncInfoTbl.contains(t.mStr));
			Node n{ .type = nodeType::CallFunc , .funcInfoPtr = &mFuncInfoTbl[t.mStr]};

			//実引数部分の読み込み
			while (!nextIfIsReserved(")")) {
				n.argumentNodeTbl.push_back(Expr());
				nextIfIsReserved(",");
			}
			return &mNodeTbl.emplace_back(n);
		}

		//宣言された変数の呼び出し
		assert(mCurrentFuncInfoPtr->lValOfsMap.contains(t.mStr));
		assert(mCurrentFuncInfoPtr->lValTypeMap.contains(t.mStr));
		return &mNodeTbl.emplace_back(Node{
				.type = nodeType::LocalVal,
				.offset = mCurrentFuncInfoPtr->lValOfsMap.at(t.mStr),
				.valTypeInfoPtr = mCurrentFuncInfoPtr->lValTypeMap.at(t.mStr)
				});
	}

	// そうでなければ数値のはず
	mCurrentPos++;
	return &mNodeTbl.emplace_back(Node{ .type = nodeType::Num, .val = t.expectNumber()});
}


Parser::ValTypeInfo* Parser::ReadValueType(){
	if (!getCurTk().isReserved("int")) {
		return nullptr;
	}
	++mCurrentPos;
	using enum ValTypeInfo::ValType;
	ValTypeInfo* retPtr = &mTypeInfoTbl.emplace_back(Int, nullptr);
	while (nextIfIsReserved("*")) {
		retPtr = &mTypeInfoTbl.emplace_back(Ptr, retPtr);
	}
	return retPtr;
}


void Parser::Parse(vector<Token>& tokenTbl) {
	mTokenTbl = tokenTbl;
	//Fix me : lhs, rhsなどの参照先をポインタで保存しているため
	// サイズ拡張時にポインタが変更されるためダングリング参照になる。
	//回避策として多めにとっておく
	mNodeTbl.reserve(tokenTbl.size() * 10);
	mRootNodeTbl.reserve(tokenTbl.size());
	mTypeInfoTbl.reserve(tokenTbl.size());
	if (mTokenTbl.size() > 0) {
		Program();
	}
}

