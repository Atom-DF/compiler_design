//
// Created by Atom Filippi on 22/10/2019.
//

//===----------------------------------------------------------------------===//
// Recursive Descent Parser - Function call for each production
//===----------------------------------------------------------------------===//

#include <vector>
#include <queue>
#include <memory>
#include <typeinfo>

#include "parser.h"
#include "../mccomp.h"
#include "../ast/ast.h"
#include "../exceptions/exceptions.h"

using namespace std;
using namespace llvm;

// TODO add epsilon checking everywhere lol

// parse ident
unique_ptr<IdentASTnode> parser_ident() {
    if (CurTok.type == IDENT) {
        auto temp = CurTok;
        getNextToken();
        return make_unique<IdentASTnode>(temp);
    }
}

// VAR_TYPE -> int | float | bool
unique_ptr<Var_TypeASTnode> parser_var_type() {
    if (CurTok.type == FLOAT_TOK or CurTok.type == BOOL_TOK or CurTok.type == INT_TOK) {
        auto temp = CurTok;
        getNextToken();
        return make_unique<Var_TypeASTnode>(temp);
    }
}

// TYPE_SPEC -> void | VAR_TYPE
unique_ptr<TypeASTnode> parser_fun_type() {
    if (CurTok.type == VOID_TOK) {
        auto temp = CurTok;
        getNextToken();
        return make_unique<Fun_TypeASTnode>(temp);
    }
    return parser_var_type();
}

// PARAM -> VAR_TYPE ident
unique_ptr<ParamASTnode> parser_param() {
    unique_ptr<TypeASTnode> type;
    unique_ptr<IdentASTnode> ident;

    type = move(parser_var_type());

    ident = move(parser_ident());

    return make_unique<ParamASTnode>(move(type), move(ident));
}

// PARAM_LIST_ -> comma PARAM PARAM_LIST_ | espilon
vector<unique_ptr<ParamASTnode>> parser_param_list_() {
    vector<unique_ptr<ParamASTnode>> param_list;

    if (CurTok.type == RPAR)
        return vector<unique_ptr<ParamASTnode>>();

    if (CurTok.type == COMMA)
        getNextToken();

    auto temp = move(parser_param());

    param_list = move(parser_param_list_());
    param_list.push_back(move(temp));

    return param_list;
}

// PARAM_LIST -> PARAM PARAM_LIST_
unique_ptr<ParamsASTnode> parser_param_list() {
    vector<unique_ptr<ParamASTnode>> param_list;

    auto temp = move(parser_param());
    param_list = parser_param_list_();
    param_list.push_back(move(temp));

    return make_unique<ParamsASTnode>(move(param_list));
}

// PARAMS -> PARAM_LIST | void | epsilon
unique_ptr<ParamsASTnode> parser_params() {
    if (CurTok.type == VOID_TOK) {
        getNextToken();
        return make_unique<ParamsASTnode>(false);
    }
    if (CurTok.type == RPAR)
        return make_unique<ParamsASTnode>(true);
    return parser_param_list();
}

// EXTERN -> extern TYPE_SPEC ident ( PARAMS ) ;
unique_ptr<ExternASTnode> parser_extern() {
    unique_ptr<TypeASTnode> type;
    unique_ptr<IdentASTnode> ident;
    unique_ptr<ParamsASTnode> params;

//    token should already be extern
    getNextToken();

    type = move(parser_fun_type());
    ident = move(parser_ident());

    if (CurTok.type == LPAR)
        getNextToken();

    params = move(parser_params());

    if (CurTok.type == RPAR)
        getNextToken();

    if (CurTok.type == SC)
        getNextToken();

    return make_unique<ExternASTnode>(move(type), move(ident), move(params));
}

// TODO not reccursive
// EXTERN_LIST' -> EXTERN EXTERN_LIST' | epsilon
vector<unique_ptr<ExternASTnode>> parser_extern_list_() {
    vector<unique_ptr<ExternASTnode>> extern_list = vector<unique_ptr<ExternASTnode>>();

    while (CurTok.type == EXTERN)
        extern_list.push_back(move(parser_extern()));

    return extern_list;
}

// EXTERN_LIST -> EXTERN EXTERN_LIST'
unique_ptr<Extern_listASTnode> parser_extern_list() {
    vector<unique_ptr<ExternASTnode>> extern_list;

    auto temp = move(parser_extern());

    extern_list = move(parser_extern_list_());

    extern_list.push_back(move(temp));

    return make_unique<Extern_listASTnode>(move(extern_list));
}

// ARGS_LIST_ -> comma EXPR ARG_LIST2 | epsilon
vector<unique_ptr<ExprASTnode>> parser_args_list_() {
    if (CurTok.type == COMMA) {
        getNextToken();
        auto expr = parser_expr();
        auto list = parser_args_list_();
        list.push_back(move(expr));
        return list;
    }

    return vector<unique_ptr<ExprASTnode>>();
}

// ARGS_LIST -> EXPR ARG_LIST_
vector<unique_ptr<ExprASTnode>> parser_args_list() {
    auto expr = parser_expr();
    auto list = parser_args_list_();
    list.push_back(move(expr));
    return list;
}

// ARGS -> ARG_LIST | epsilon
vector<unique_ptr<ExprASTnode>> parser_args() {
    if (CurTok.type == RPAR)
        return vector<unique_ptr<ExprASTnode>>();
    return parser_args_list();
}

// RVAL_FUN -> ( ARGS ) | epseilon
vector<unique_ptr<ExprASTnode>> parser_rval_fun() {
    if (CurTok.type == LPAR) {
        getNextToken();
        auto args = parser_args();
        if (CurTok.type == RPAR)
            getNextToken();
        return args;
    }

    return vector<unique_ptr<ExprASTnode>>();
}

// RVAL_NOT -> ( EXPR ) | ident RVAL_FUN | INT_LIT | FLOAT_LIT | BOOL_LIT
unique_ptr<ExprASTnode> parser_rval_not() {
    if (CurTok.type == LPAR) {
        getNextToken();
        auto expr = parser_expr();
        if (CurTok.type == RPAR)
            getNextToken();
        return move(expr);
    }

    if (CurTok.type == INT_LIT) {
        auto value = IntVal;
        auto tok = CurTok;
        getNextToken();
        return make_unique<IntASTnode>(tok, value);
    }

    if (CurTok.type == FLOAT_LIT) {
        auto value = FloatVal;
        auto tok = CurTok;
        getNextToken();
        return make_unique<FloatASTnode>(tok, value);
    }


    if (CurTok.type == BOOL_LIT) {
        auto value = BoolVal;
        auto tok = CurTok;
        getNextToken();
        return make_unique<BoolASTnode>(tok, value);
    }

    if (CurTok.type == IDENT) {
        auto ident = parser_ident();
        auto arguments = parser_rval_fun();
        if (arguments.empty())
            return move(ident);
        return make_unique<Ident_funASTnode>(move(ident), arguments);
    }
}

// RVAL_MUL -> minus RVAL_MUL | not RVAL_MUL | RVAL_NOT
unique_ptr<ExprASTnode> parser_rval_mul() {
    if (CurTok.type == NOT or CurTok.type == MINUS) {
        auto op = CurTok;
        auto expr = parser_rval_mul();
        return make_unique<UnaryASTnode>(op, move(expr));
    }
//  TODO check the follow set first
    return parser_rval_not();
}

// RVAL_ADD_ -> mul RVAL_MUL RVAL_ADD_ | div RVAL_MUL RVAL_ADD_ | mod RVAL_MUL RVAL_ADD_ | epsilon
unique_ptr<ExprASTnode> parser_rval_add_() {
//    TODO check follow set

    if (CurTok.type == ASTERIX or CurTok.type == DIV or CurTok.type == MOD) {
        auto op = CurTok;
        getNextToken();
        auto rval = parser_rval_add();
        auto rval_ = parser_rval_comp_();
        if (typeid(rval_) == typeid(EmptyASTnode))
            return make_unique<HalfASTnode>(op, move(rval));
        auto expr = make_unique<BinaryASTnode>(move(rval), move(rval_));
        return make_unique<HalfASTnode>(op, move(expr));
    }

    return make_unique<EmptyASTnode>();
}

// RVAL_ADD -> RVAL_MUL RVAL_ADD_
unique_ptr<ExprASTnode> parser_rval_add() {
    auto rval = parser_rval_mul();
    auto rval_ = parser_rval_add_();
    if (typeid(rval) == typeid(EmptyASTnode))
        return move(rval);
    return make_unique<BinaryASTnode>(move(rval), move(rval_));
}

// RVAL_COMP_ -> add RVAL_ADD RVAL_COMP_ | ge RVAL_ADD RVAL_COMP_ | epsilon
unique_ptr<ExprASTnode> parser_rval_comp_() {
//    TODO check follow set

    if (CurTok.type == PLUS or CurTok.type == MINUS) {
        auto op = CurTok;
        getNextToken();
        auto rval = parser_rval_add();
        auto rval_ = parser_rval_comp_();
        if (typeid(rval_) == typeid(EmptyASTnode))
            return make_unique<HalfASTnode>(op, move(rval));
        auto expr = make_unique<BinaryASTnode>(move(rval), move(rval_));
        return make_unique<HalfASTnode>(op, move(expr));
    }

    return make_unique<EmptyASTnode>();
}

// RVAL_COMP -> RVAL_ADD RVAL_COMP_
unique_ptr<ExprASTnode> parser_rval_comp() {
    auto rval = parser_rval_add();
    auto rval_ = parser_rval_comp_();
    if (typeid(rval) == typeid(EmptyASTnode))
        return move(rval);
    return make_unique<BinaryASTnode>(move(rval), move(rval_));
}

// RVAL_EQ_ -> g RVAL_COMP RVAL_EQ_ | ge RVAL_COMP RVAL_EQ_ | l RVAL_COMP RVAL_EQ_ | le RVAL_COMP RVAL_EQ_ | epsilon
unique_ptr<ExprASTnode> parser_rval_eq_() {
//    TODO check follow set

    if (CurTok.type == LT or CurTok.type == GT or CurTok.type == GE or CurTok.type == LE) {
        auto op = CurTok;
        getNextToken();
        auto rval = parser_rval_comp();
        auto rval_ = parser_rval_eq_();
        if (typeid(rval_) == typeid(EmptyASTnode))
            return make_unique<HalfASTnode>(op, move(rval));
        auto expr = make_unique<BinaryASTnode>(move(rval), move(rval_));
        return make_unique<HalfASTnode>(op, move(expr));
    }

    return make_unique<EmptyASTnode>();
}

// RVAL_EQ -> RVAL_COMP RVAL_EQ_
unique_ptr<ExprASTnode> parser_rval_eq() {
    auto rval = parser_rval_comp();
    auto rval_ = parser_rval_eq_();
    if (typeid(rval) == typeid(EmptyASTnode))
        return move(rval);
    return make_unique<BinaryASTnode>(move(rval), move(rval_));
}

// RVAL_AND_ -> eq RVAL_EQ RVAL_AND_ | ne RVAL_EQ RVAL_AND_ | epsilon
unique_ptr<ExprASTnode> parser_rval_and_() {
//    TODO check follow set

    if (CurTok.type == EQ or CurTok.type == NE) {
        auto op = CurTok;
        getNextToken();
        auto rval = parser_rval_eq();
        auto rval_ = parser_rval_and_();
        if (typeid(rval_) == typeid(EmptyASTnode))
            return make_unique<HalfASTnode>(op, move(rval));
        auto expr = make_unique<BinaryASTnode>(move(rval), move(rval_));
        return make_unique<HalfASTnode>(op, move(expr));
    }

    return make_unique<EmptyASTnode>();
}

// RVAL_AND -> RVAL_EQ RVAL_AND_
unique_ptr<ExprASTnode> parser_rval_and() {
    auto rval = parser_rval_eq();
    auto rval_ = parser_rval_and_();
    if (typeid(rval) == typeid(EmptyASTnode))
        return move(rval);
    return make_unique<BinaryASTnode>(move(rval), move(rval_));
}

// RVAL_OR_ -> and RVAL_AND RVAL_OR_ | epsilon
unique_ptr<ExprASTnode> parser_rval_or_() {
//    TODO check follow set

    if (CurTok.type == AND) {
        auto op = CurTok;
        getNextToken();
        auto rval = parser_rval_or();
        auto rval_ = parser_rval_or_();
        if (typeid(rval_) == typeid(EmptyASTnode))
            return make_unique<HalfASTnode>(op, move(rval));
        auto expr = make_unique<BinaryASTnode>(move(rval), move(rval_));
        return make_unique<HalfASTnode>(op, move(expr));
    }

    return make_unique<EmptyASTnode>();
}

// RVAL_OR -> RVAL_AND RVAL_OR_
unique_ptr<ExprASTnode> parser_rval_or() {
    auto rval = parser_rval_and();
    auto rval_ = parser_rval_or_();
    if (typeid(rval) == typeid(EmptyASTnode))
        return move(rval);
    return make_unique<BinaryASTnode>(move(rval), move(rval_));
}

// RVAL_ -> or RVAL_OR RVAL_ | epsilon
unique_ptr<ExprASTnode> parser_rval_() {
//    TODO check follow set

    if (CurTok.type == OR) {
        auto op = CurTok;
        getNextToken();
        auto rval = parser_rval_or();
        auto rval_ = parser_rval_();
        if (typeid(rval_) == typeid(EmptyASTnode))
            return make_unique<HalfASTnode>(op, move(rval));
        auto expr = make_unique<BinaryASTnode>(move(rval), move(rval_));
        return make_unique<HalfASTnode>(op, move(expr));
    }

    return make_unique<EmptyASTnode>();
}

// RVAL -> RVAL_OR RVAL_
unique_ptr<ExprASTnode> parser_rval() {
    auto rval_or = parser_rval_or();
    auto rval_ = parser_rval_();
    if (typeid(rval_) == typeid(EmptyASTnode))
        return move(rval_or);
    return make_unique<BinaryASTnode>(move(rval_or), move(rval_));
}

// EXPR -> ident equal EXPR | RVAL
unique_ptr<ExprASTnode> parser_expr() {
    auto temp = CurTok;
    getNextToken();
    auto  temp2 = CurTok;
    getNextToken();
    if (temp2.type == EQ) {
        putBackToken(temp);
//      Carefull doing somehting dangerous here... might break maybe
        auto ident = move(parser_ident());
        auto expr = move(parser_expr());
        return make_unique<BinaryASTnode>(move(ident), temp2, move(expr));
    }
    putBackToken(temp2);
    putBackToken(temp);
    return parser_rval();
}

// EXPR_STMT -> ; | EXPR ;
unique_ptr<Expr_stmtASTnode> parser_expr_stmt() {
    unique_ptr<ExprASTnode> expr;
    if (CurTok.type == SC)
        return make_unique<Expr_stmtASTnode>();
    expr = move(parser_expr());
    return make_unique<Expr_stmtASTnode>(move(expr));
}

// ELSE_STMT -> else BLOCK | epsilon
unique_ptr<BlockASTnode> parser_else_stmt() {
    if (CurTok.type == ELSE)
        getNextToken();
    else
        return make_unique<BlockASTnode>();

    return move(parser_block());
}

// IF_STMT -> if ( EXPR ) BLOCK ELSE_STMT
unique_ptr<If_stmtASTnode> parser_if_stmt() {
    unique_ptr<ExprASTnode> expr;
    unique_ptr<BlockASTnode> if_block;
    unique_ptr<BlockASTnode> else_block;

    if (CurTok.type == IF)
        getNextToken();

    if (CurTok.type == LPAR)
        getNextToken();

    expr = move(parser_expr());

    if (CurTok.type == RPAR)
        getNextToken();

    if_block = move(parser_block());

    else_block = move(parser_else_stmt());

    return make_unique<If_stmtASTnode>(move(expr), move(if_block), move(else_block));
}

// WHILE_STMT -> while ( EXPR ) STMT
unique_ptr<While_stmtASTnode> parser_while_stmt() {
    unique_ptr<ExprASTnode> expr;
    unique_ptr<StmtASTnode> stmt;

    if (CurTok.type == WHILE)
        getNextToken();

    if (CurTok.type == LPAR)
        getNextToken();

    expr = move(parser_expr());

    if (CurTok.type == RPAR)
        getNextToken();

    stmt = move(parser_stmt());
    return make_unique<While_stmtASTnode>(move(expr), move(stmt));
}

// RETURN_STMT_ -> ; | EXPR ;
unique_ptr<Return_stmtASTnode> parser_return_stmt_() {
    unique_ptr<ExprASTnode> expr;
    if (CurTok.type == SC)
        return make_unique<Return_stmtASTnode>();
    expr = move(parser_expr());
    return make_unique<Return_stmtASTnode>(move(expr));
}

// RETURN_STMT -> return RETURN_STMT_
unique_ptr<Return_stmtASTnode> parser_return_stmt() {
    if (CurTok.type == RETURN)
        getNextToken();

    return parser_return_stmt_();
}

// STMT -> EXPR_STMT | BLOCK | IF_STMT | WHILE_STMT | RETURN_STMT
unique_ptr<StmtASTnode> parser_stmt() {
    if (CurTok.type == IF)
        return parser_if_stmt();
    if (CurTok.type == WHILE)
        return parser_while_stmt();
    if (CurTok.type == RETURN)
        return parser_return_stmt();
    if (CurTok.type == LBRA)
        return parser_block();
    if (CurTok.type == LPAR or CurTok.type == IDENT or CurTok.type == NOT or CurTok.type == MINUS or CurTok.type == SC)
        return parser_expr_stmt();
}

// TODO not recursive
// STMT_LIST -> STMT STMT_LIST | epsilon
vector<unique_ptr<StmtASTnode>> parser_stmt_list() {
    vector<unique_ptr<StmtASTnode>> stmt_list = vector<unique_ptr<StmtASTnode>>();

    while (CurTok.type != RBRA)
        stmt_list.push_back(move(parser_stmt()));

    return move(stmt_list);
}

// LOCAL_DECL -> VAR_TYPE IDENT ;
unique_ptr<Local_declASTnode> parser_local_decl() {
    unique_ptr<TypeASTnode> type;
    unique_ptr<IdentASTnode> ident;

    type = move(parser_fun_type());
    ident = move(parser_ident());

    if (CurTok.type == SC)
        getNextToken();

    return make_unique<Local_declASTnode>(move(type), move(ident));
}

// TODO not reccursive
// LOCAL_DECLS -> LOCAL_DECL LOCAL_DECLS
vector<unique_ptr<Local_declASTnode>> parser_local_decls() {
    vector<unique_ptr<Local_declASTnode>> local_decls = vector<unique_ptr<Local_declASTnode>>();

    while (CurTok.type == INT_TOK or CurTok.type == FLOAT_TOK or CurTok.type == BOOL_TOK)
        local_decls.push_back(move(parser_local_decl()));

    return local_decls;
}

// BLOCK -> LB LOCAL_DECLS STMT_LIST RB
unique_ptr<BlockASTnode> parser_block() {
    vector<unique_ptr<Local_declASTnode>> local_decls;
    vector<unique_ptr<StmtASTnode>> stmt_list;

    if (CurTok.type == LBRA)
        getNextToken();

    local_decls = move(parser_local_decls());
    stmt_list = move(parser_stmt_list());

    if (CurTok.type == RBRA)
        getNextToken();

    return make_unique<BlockASTnode>(move(local_decls), move(stmt_list));
}

// FUN_DECL -> FUN_TYPE ident ( PARAM ) BLOCK
unique_ptr<Fun_DeclASTnode> parser_fun_decl() {
    unique_ptr<TypeASTnode> type;
    unique_ptr<IdentASTnode> ident;
    unique_ptr<ParamsASTnode> params;
    unique_ptr<BlockASTnode> block;

    type = move(parser_fun_type());
    ident = move(parser_ident());

    if (CurTok.type == LPAR)
        getNextToken();

    params = move(parser_params());

    if (CurTok.type == RPAR)
        getNextToken();

    block = move(parser_block());

    return make_unique<Fun_DeclASTnode>(move(type), move(ident), move(params), move(block));
}

// VAR_DECL -> VAR_TYPE ident ;
unique_ptr<Var_DeclASTnode> parser_var_decl() {
    unique_ptr<TypeASTnode> type;
    unique_ptr<IdentASTnode> ident;

    type = move(parser_fun_type());
    ident = move(parser_ident());

    if (CurTok.type == SC)
        getNextToken();

    return make_unique<Var_DeclASTnode>(move(type), move(ident));
}

// DECL -> VAR_DECL | FUN_DECL
unique_ptr<DeclASTnode> parser_decl() {
    auto temp = CurTok;
    getNextToken();
    auto  temp2 = CurTok;
    getNextToken();
    if (CurTok.type == SC) {
        putBackToken(temp2);
        putBackToken(temp);
        return parser_var_decl();
    }
    putBackToken(temp2);
    putBackToken(temp);
    return parser_fun_decl();
}

// DECL_LIST_ -> DECL DECL_LIST_ | epsilon
vector<unique_ptr<DeclASTnode>> parser_decl_list_() {
    vector<unique_ptr<DeclASTnode>> decl_list = vector<unique_ptr<DeclASTnode>>();

    while (CurTok.type == VOID_TOK or CurTok.type == INT_TOK or CurTok.type == FLOAT_TOK or CurTok.type == BOOL_TOK)
        decl_list.push_back(move(parser_decl()));

    return decl_list;
}

// DECL_LIST -> DECL DECL_LIST_
unique_ptr<Decl_listASTnode> parser_decl_list(){
    vector<unique_ptr<DeclASTnode>> decl_list;

    auto temp = move(parser_decl());

    decl_list = move(parser_decl_list_());

    decl_list.push_back(move(temp));

    return make_unique<Decl_listASTnode>(move(decl_list));
}

// PROGRAM -> EXTERN_LIST DECL_LIST | DECL_LIST
void parser() {
    unique_ptr<Extern_listASTnode> extern_list;
    unique_ptr<Decl_listASTnode> decl_list;

    getNextToken();
    if (CurTok.type == EXTERN)
        extern_list = parser_extern_list();
    decl_list = parser_decl_list();
    cout << extern_list->to_string("", false);
    cout << decl_list->to_string("", true);
}
