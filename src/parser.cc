#define OP 256
#define ID 257
#define NUM 258
#define REAL 259

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "type.h"
#include "parser.h"

using namespace Boost::Internal;

bool isDigit(char c) {
    if (c >= '0' && c <= '9')
        return true;
    return false;
}

bool isLetter(char c) {
    if (c >= 'a' && c <= 'z')
        return true;
    if (c >= 'A' && c <= 'Z') 
        return true;
    return false;
}

class Token {
public:
    int tag = -1;
    std::string lexeme = "";
    int value1 = 0;
    float value2 = 0;
    Token(int _tag) {
        tag = _tag;
    }

    Token(int _tag, std::string _lexeme) {
        tag = _tag;
        lexeme = _lexeme;
    }

    Token(int _tag, int _value) {
        tag = _tag;
        value1 = _value;
    }

    Token(int _tag, float _value) {
        tag = _tag;
        value2 = _value;
    }

    Token() {}
    std::string getLex() {
        return lexeme;
    }

    int getInt() {
        return value1;
    }

    int getFloat() {
        return value2;
    }
};

class Lexer {
    char peek;
    int record;
    std::string buffer;
public :
    Lexer(std::string _buffer) {
        buffer = _buffer;
        peek = ' ';
        record = 0;
    }

    Lexer() {}

    void readch() {
        peek = buffer[record];
        record++;
    }

    bool readch(char c) {
        readch();
        if (peek != c)
            return false;
        peek = ' ';
        return true;
    }

    Token scan() {
        while (true) {
            if (peek == ' ' || peek == '\t' || peek == '\r') {
                readch();
                continue;
            }
            else {
                break;
            }
        }

        if (peek == '/') {
            if (readch('/')) {
                return Token(OP);
            }
            else {
                return Token('/');
            }
        }

        if (isDigit(peek)) {
            int v = 0;
            do {
                v = 10 * v + ((int)peek - '0');
                readch();
            } while (isDigit(peek));

            if (peek != '.') {
                return Token(NUM, v);
            }

            float x = v;
            float d = 10;
            while (true) {
                readch();
                if (!isDigit(peek))
                    break;

                x += ((int)peek - '0') / d;
                d *= 10;
            }
            return Token(REAL,x);
        }

        if ((peek == '_') || isLetter(peek)) {
            std::string name  = "";
            do {
                name += peek;
                readch();
            } while (isDigit(peek) || isLetter(peek) || (peek == '_'));

            return Token(ID, name);
        }

        Token tok = Token(peek);
        peek = ' ';

        return tok;
    }
};


std::vector<Stmt> P(Type index_type, Type data_type);
Stmt S(Type index_type, Type data_type);
Expr LHS(Type index_type, Type data_type);
Expr RHS(Type index_type, Type data_type);
Expr term(Type index_type, Type data_type);
Expr TSRef(Type index_type, Type data_type);
std::vector<size_t> CList(Type index_type, Type data_type);
std::vector<Expr> AList(Type index_type, Type data_type, std::vector<size_t> dom);
Expr IdExpr(Type index_type, Type data_type, int dom_size);

Lexer lexer;
Token look;
//loop_index
std::vector<Expr> loop_index;
std::vector<std::string> index_name;
std::vector<int> index_dom;
//ins & outs
std::vector<Expr> ins;
std::vector<Expr> outs;
std::vector<std::string> in_name;
std::vector<std::string> out_name;
bool in_flag[100];
int in_record[100];
bool out_flag[100];
int out_record[100];

Group parser(std::string str, Type index_type, Type data_type, 
    std::vector<std::string> in_str, std::vector<std::string> out_str, std::string kernel_name) {
    lexer = Lexer(str);
    look = lexer.scan();
    in_name = in_str;
    out_name = out_str;
    for (unsigned int i = 0; i < in_name.size(); i++) {
        in_flag[i] = false;
    }
    for (unsigned int i = 0; i < out_name.size(); i++) {
        out_flag[i] = false;
    }
    ins.clear();
    outs.clear();

    std::vector<Stmt> loop_nests = P(index_type, data_type);

    std::vector<Expr> ins_final;
    std::vector<Expr> outs_final;
    for (unsigned int i = 0; i < in_name.size(); i++) {
        ins_final.push_back(ins[in_record[i]]);
    }
    for(unsigned int i = 0; i < out_name.size(); i++) {
        outs_final.push_back(outs[out_record[i]]);
    }
    Group ret = Kernel::make(kernel_name, ins_final, outs_final, loop_nests, KernelType::CPU);
    return ret;
}

Token match(int _tag) {
    if (look.tag == _tag) {
        Token tok = look;
        look = lexer.scan();
        return tok;
    }
    else {
        std::cout << "error!" << std::endl;
        return Token(-1);
    }
}

//P ::= S*
std::vector<Stmt> P(Type index_type, Type data_type){
    std::vector<Stmt> stmts;
    while (look.tag == ID){
        loop_index.clear();
        index_name.clear();
        index_dom.clear();
        Stmt st = S(index_type, data_type);
        Stmt loop_nest = LoopNest::make(loop_index, {st});
        stmts.push_back(loop_nest);
        match(';');
    }
    return stmts;
}

//S ::= LHS = RHS ;
Stmt S(Type index_type, Type data_type) {
    Expr expr_left = LHS(index_type, data_type);
    match('=');
    Expr expr_right = RHS(index_type, data_type);
    Stmt stmt = Move::make(expr_left, expr_right, MoveType::MemToMem);
    return stmt;
} 

//LHS ::= TRef
Expr LHS(Type index_type, Type data_type) {
    return TSRef(index_type, data_type);
}

Expr handle(Expr expr, Type index_type, Type data_type) {
    if (look.tag == '+') {
        look = lexer.scan();
        Expr i = RHS(index_type, data_type);
        Expr ret = Binary::make(data_type, BinaryOpType::Add, expr, i);
        return ret;
    }
    else if (look.tag == '-') {
        look = lexer.scan();
        Expr i = RHS(index_type, data_type);
        Expr ret = Binary::make(data_type, BinaryOpType::Sub, expr, i);
        return ret;
    }
    else if (look.tag == '*') {
        look = lexer.scan();
        Expr i = term(index_type, data_type);
        Expr ret = Binary::make(data_type, BinaryOpType::Mul, expr, i);
        return ret;
    }
    else if (look.tag == '/') {
        look = lexer.scan();
        Expr i = term(index_type, data_type);
        Expr ret = Binary::make(data_type, BinaryOpType::Div, expr, i);
        return ret;
    }
    else if (look.tag == '%') {
        look = lexer.scan();
        Expr i = term(index_type, data_type);
        Expr ret = Binary::make(data_type, BinaryOpType::Mod, expr, i);
        return ret;
    }
    else {
        look = lexer.scan();
        Expr i = term(index_type, data_type);
        Expr ret = Binary::make(data_type, BinaryOpType::Div, expr, i);
        return ret;
    }
}


//RHS ::= RHS + term | RHS - term | term
Expr RHS(Type index_type, Type data_type){
    Expr expr = term(index_type, data_type);

    if (look.tag == '+' || look.tag == '-') {
        return handle(expr, index_type, data_type);
    }
    return expr;
    
}  

//term ::= term * factor | term / factor | term % factor | term // factor | factor
//factor ::= TRef | SRef | Const | (RHS)
Expr term(Type index_type, Type data_type) {
    if (look.tag == NUM) {
        Token tok = match(NUM);
        Expr expr = Expr(tok.getInt());
        if (look.tag == '*' || look.tag == '/' || look.tag == '%' || look.tag == OP)
            return handle(expr, index_type, data_type);
        return expr;
    }
    else if (look.tag == REAL) {
        Token tok = match(REAL);
        Expr expr = Expr(tok.getFloat());
        if (look.tag == '*' || look.tag == '/' || look.tag == '%' || look.tag == OP)
            return handle(expr, index_type, data_type);
        return expr;
    }
    else if (look.tag == '(') {
        look = lexer.scan();
        Expr expr = RHS(index_type, data_type);
        match(')');
        if (look.tag == '*' || look.tag == '/' || look.tag == '%' || look.tag == OP) {
            return handle(expr, index_type, data_type);
        }
        return expr;
    }
    else {
        Expr expr = TSRef(index_type,data_type);
        if (look.tag == '*' || look.tag == '/' || look.tag == '%' || look.tag == OP) {
            return handle(expr, index_type, data_type);
        }
        return expr;
    }
}


//TRef ::= Id < CList > [ AList ]
Expr TSRef(Type index_type, Type data_type) {
    std::string name = "";
    std::vector<size_t> dom;
    std::vector<Expr> index;
    
    Token tok = match(ID);
    match('<');
    dom = CList(index_type, data_type);
    match('>');
    if (look.tag == '[') {
        match('[');
        index = AList(index_type, data_type, dom);
        match(']');
        Expr expr = Var::make(data_type, tok.getLex(), index, dom);
        for (unsigned int i = 0; i < in_name.size(); i++) {
            if ((tok.getLex() == in_name[i]) && !in_flag[i]) {
                in_flag[i] = true;
                ins.push_back(expr);
                in_record[i] = ins.size() - 1;
                break;
            }
        }
        for (unsigned int i = 0; i < out_name.size(); i++) {
            if ((tok.getLex() == out_name[i]) && !out_flag[i]) {
                out_flag[i] = true;
                outs.push_back(expr);
                out_record[i] = outs.size() - 1;
                break;
            }
        }
        return expr;
    }
    else {
        index.push_back(Expr(0));
        Expr expr = Var::make(data_type, tok.getLex(), index, dom);
        for (unsigned int i = 0; i < in_name.size(); i++) {
            if ((tok.getLex() == in_name[i]) && !in_flag[i]) {
                in_flag[i] = true;
                ins.push_back(expr);
                in_record[i] = ins.size() - 1;
                break;
            }
        }
        for (unsigned int i = 0; i < out_name.size(); i++) {
            if ((tok.getLex() == out_name[i]) && !out_flag[i]) {
                out_flag[i] = true;
                outs.push_back(expr);
                out_record[i] = outs.size() - 1;
                break;
            }
        }
        return expr;
    }
}

//CList ::= CList , IntV | IntV
std::vector<size_t> CList(Type index_type, Type data_type) {
    std::vector<size_t> ret;
    Token tok = match(NUM);
    ret.push_back(tok.getInt());
    while (look.tag == ',') {
        look = lexer.scan();
        tok = match(NUM);
        ret.push_back(tok.getInt());
    }
    return ret;
}

//AList ::= AList , IdExpr | IdExpr
std::vector<Expr> AList(Type index_type, Type data_type, std::vector<size_t> dom) {
    int count = 0;
    std::vector<Expr> ret;
    Expr expr = IdExpr(index_type, data_type, dom[count++]);
    ret.push_back(expr);

    while (look.tag == ',') {
        look = lexer.scan();
        Expr expr_1 = IdExpr(index_type, data_type, dom[count++]);
        ret.push_back(expr_1);
    }
    return ret;
}

//IdExpr ::= Id | IdExpr + IdExpr | IdExpr + IntV | IdExpr * IntV | IdExpr // IntV | IdExpr % IntV
Expr IdExpr(Type index_type, Type data_type, int dom_size) {
    Token tok = match(ID);
    Expr dom_i = Dom::make(index_type, 0, dom_size);
    Expr i = Index::make(index_type, tok.getLex(), dom_i, IndexType::Spatial);
    bool flag = false;
    for (unsigned int j = 0; j < index_name.size(); j++) {
        if (index_name[j] == tok.getLex()) {
            flag = true;
            if (dom_size < index_dom[j]) {
                index_dom[j] = dom_size;
                loop_index[j] = i;
            }
            break;
        }
    }   
    if (!flag) {
        index_name.push_back(tok.getLex());
        index_dom.push_back(dom_size);
        loop_index.push_back(i);
    }
    

    if (look.tag == OP) { //"//" means Or?
        look = lexer.scan();
        tok = match(NUM);
        Expr ret = Binary::make(index_type, BinaryOpType::Div, i, tok.getInt());
        return ret;
    }
    else if (look.tag == '*') {
        look = lexer.scan();
        tok = match(NUM);
        Expr ret = Binary::make(index_type, BinaryOpType::Mul, i, tok.getInt());
        return ret;
    }
    else if (look.tag == '%') {
        look = lexer.scan();
        tok = match(NUM);
        Expr ret = Binary::make(index_type, BinaryOpType::Mod, i, tok.getInt());
        return ret;
    }
    else if (look.tag == '+') {
        look = lexer.scan();
        if (look.tag == NUM) {
            tok = match(NUM);
            Expr ret = Binary::make(index_type, BinaryOpType::Add, i, tok.getInt());
            return ret;
        }
        else {
            // type????????
            Expr j = IdExpr(index_type, data_type, dom_size);
            Expr ret = Binary::make(index_type, BinaryOpType::Add, i, j);
            return ret;
        }
    }
    else if (look.tag == '-') {
        look = lexer.scan();
        if (look.tag == NUM) {
            tok = match(NUM);
            Expr ret = Binary::make(index_type, BinaryOpType::Sub, i, tok.getInt());
            return ret;
        }
        else {
            // type????????
            Expr j = IdExpr(index_type, data_type, dom_size);
            Expr ret = Binary::make(index_type, BinaryOpType::Sub, i, j);
            return ret;
        }
    }
    else
        return i;
}