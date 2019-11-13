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
IdentASTnode* parser_ident() {
    if (CurTok.type == IDENT) {
        auto temp = CurTok;
        getNextToken();
        return new IdentASTnode(temp);
    }
}

// VAR_TYPE -> int | float | bool
Var_TypeASTnode* parser_var_type() {
    if (CurTok.type == FLOAT_TOK or CurTok.type == BOOL_TOK or CurTok.type == INT_TOK) {
        auto temp = CurTok;
        getNextToken();
        return new Var_TypeASTnode(temp);
    }
}

// TYPE_SPEC -> void | VAR_TYPE
TypeASTnode* parser_fun_type() {
    if (CurTok.type == VOID_TOK) {
        auto temp = CurTok;
        getNextToken();
        return new Fun_TypeASTnode(temp);
    }
    return parser_var_type();
}

// PARAM -> VAR_TYPE ident
ParamASTnode* parser_param() {
    TypeASTnode* type;
    IdentASTnode* ident;

    type = parser_var_type();

    ident = parser_ident();

    return new ParamASTnode(type, ident);
}

// PARAM_LIST_ -> comma PARAM PARAM_LIST_ | espilon
vector<ParamASTnode*> parser_param_list_() {
    vector<ParamASTnode*> param_list;

    if (CurTok.type == RPAR)
        return vector<ParamASTnode*>();

    if (CurTok.type == COMMA)
        getNextToken();

    auto temp = parser_param();

    param_list = parser_param_list_();
    param_list.push_back(temp);

    return param_list;
}

// PARAM_LIST -> PARAM PARAM_LIST_
ParamsASTnode* parser_param_list() {
    vector<ParamASTnode*> param_list;

    auto temp = parser_param();
    param_list = parser_param_list_();
    param_list.push_back(temp);

    return new ParamsASTnode(param_list);
}

// PARAMS -> PARAM_LIST | void | epsilon
ParamsASTnode* parser_params() {
    if (CurTok.type == VOID_TOK) {
        getNextToken();
        return new ParamsASTnode(false);
    }
    if (CurTok.type == RPAR)
        return new ParamsASTnode(true);
    return parser_param_list();
}

// EXTERN -> extern TYPE_SPEC ident ( PARAMS ) ;
ExternASTnode* parser_extern() {
    TypeASTnode* type;
    IdentASTnode* ident;
    ParamsASTnode* params;

//    token should already be extern
    getNextToken();

    type = parser_fun_type();
    ident = parser_ident();

    if (CurTok.type == LPAR)
        getNextToken();

    params = parser_params();

    if (CurTok.type == RPAR)
        getNextToken();

    if (CurTok.type == SC)
        getNextToken();

    return new ExternASTnode(type, ident, params);
}

// TODO not reccursive
// EXTERN_LIST' -> EXTERN EXTERN_LIST' | epsilon
vector<ExternASTnode*> parser_extern_list_() {
    vector<ExternASTnode*> extern_list = vector<ExternASTnode*>();

    while (CurTok.type == EXTERN)
        extern_list.push_back(parser_extern());

    return extern_list;
}

// EXTERN_LIST -> EXTERN EXTERN_LIST'
Extern_listASTnode* parser_extern_list() {
    vector<ExternASTnode*> extern_list;

    auto temp = parser_extern();

    extern_list = parser_extern_list_();

    extern_list.push_back(temp);

    return new Extern_listASTnode(extern_list);
}

// ARGS_LIST_ -> comma EXPR ARG_LIST2 | epsilon
vector<ExprASTnode*> parser_args_list_() {
    if (CurTok.type == COMMA) {
        getNextToken();
        auto expr = parser_expr();
        auto list = parser_args_list_();
        list.push_back(expr);
        return list;
    }

    return vector<ExprASTnode*>();
}

// ARGS_LIST -> EXPR ARG_LIST_
vector<ExprASTnode*> parser_args_list() {
    auto expr = parser_expr();
    auto list = parser_args_list_();
    list.push_back(expr);
    return list;
}

// ARGS -> ARG_LIST | epsilon
vector<ExprASTnode*> parser_args() {
    if (CurTok.type == RPAR)
        return vector<ExprASTnode*>();
    return parser_args_list();
}

// RVAL_FUN -> ( ARGS ) | epsilon
vector<ExprASTnode*> parser_rval_fun() {
    if (CurTok.type == LPAR) {
        getNextToken();
        auto args = parser_args();
        if (CurTok.type == RPAR)
            getNextToken();
        return args;
    }

    return vector<ExprASTnode*>();
}

// RVAL_NOT -> ( EXPR ) | ident RVAL_FUN | INT_LIT | FLOAT_LIT | BOOL_LIT
ExprASTnode* parser_rval_not() {
    if (CurTok.type == LPAR) {
        getNextToken();
        auto expr = parser_expr();
        if (CurTok.type == RPAR)
            getNextToken();
        return expr;
    }

    if (CurTok.type == INT_LIT) {
        auto value = IntVal;
        auto tok = CurTok;
        getNextToken();
        return new IntASTnode(tok, value);
    }

    if (CurTok.type == FLOAT_LIT) {
        auto value = FloatVal;
        auto tok = CurTok;
        getNextToken();
        return new FloatASTnode(tok, value);
    }


    if (CurTok.type == BOOL_LIT) {
        auto value = BoolVal;
        auto tok = CurTok;
        getNextToken();
        return new BoolASTnode(tok, value);
    }

    if (CurTok.type == IDENT) {
        CurTok.print_as_string();
        auto ident = parser_ident();
        auto arguments = parser_rval_fun();
        if (arguments.empty())
            return ident;
        return new Ident_funASTnode(ident, arguments);
    }
}

// RVAL_MUL -> minus RVAL_MUL | not RVAL_MUL | RVAL_NOT
ExprASTnode* parser_rval_mul() {
    if (CurTok.type == NOT or CurTok.type == MINUS) {
        auto op = CurTok;
        auto expr = parser_rval_mul();
        return new UnaryASTnode(op, expr);
    }
//  TODO check the follow set first
    return parser_rval_not();
}

// RVAL_ADD_ -> mul RVAL_MUL RVAL_ADD_ | div RVAL_MUL RVAL_ADD_ | mod RVAL_MUL RVAL_ADD_ | epsilon
ExprASTnode* parser_rval_add_() {
//    TODO check follow set
    if (CurTok.type == ASTERIX or CurTok.type == DIV or CurTok.type == MOD) {
        auto op = CurTok;
        getNextToken();
        auto rval = parser_rval_mul();
        auto rval_ = parser_rval_add_();
        if (rval_->isEmpty)
            return new HalfASTnode(op, rval);
        auto expr = new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
        return new HalfASTnode(op, expr);
    }

    return new EmptyASTnode();
}

// RVAL_ADD -> RVAL_MUL RVAL_ADD_
ExprASTnode* parser_rval_add() {
    auto rval = parser_rval_mul();
    auto rval_ = parser_rval_add_();
    if (rval_->isEmpty)
        return rval;
    return new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
}

// RVAL_COMP_ -> add RVAL_ADD RVAL_COMP_ | ge RVAL_ADD RVAL_COMP_ | epsilon
ExprASTnode* parser_rval_comp_() {
//    TODO check follow set

    if (CurTok.type == PLUS or CurTok.type == MINUS) {
        auto op = CurTok;
        getNextToken();
        auto rval = parser_rval_add();
        auto rval_ = parser_rval_comp_();
        if (typeid(*rval_) == typeid(EmptyASTnode*))
            return new HalfASTnode(op, rval);
        auto expr = new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
        return new HalfASTnode(op, expr);
    }

    return new EmptyASTnode();
}

// RVAL_COMP -> RVAL_ADD RVAL_COMP_
ExprASTnode* parser_rval_comp() {
    auto rval = parser_rval_add();
    auto rval_ = parser_rval_comp_();
    if (rval_->isEmpty)
        return rval;
    return new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
}

// RVAL_EQ_ -> g RVAL_COMP RVAL_EQ_ | ge RVAL_COMP RVAL_EQ_ | l RVAL_COMP RVAL_EQ_ | le RVAL_COMP RVAL_EQ_ | epsilon
ExprASTnode* parser_rval_eq_() {
//    TODO check follow set

    if (CurTok.type == LT or CurTok.type == GT or CurTok.type == GE or CurTok.type == LE) {
        auto op = CurTok;
        getNextToken();
        auto rval = parser_rval_comp();
        auto rval_ = parser_rval_eq_();
        if (typeid(*rval_) == typeid(EmptyASTnode*))
            return new HalfASTnode(op, rval);
        auto expr = new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
        return new HalfASTnode(op, expr);
    }

    return new EmptyASTnode();
}

// RVAL_EQ -> RVAL_COMP RVAL_EQ_
ExprASTnode* parser_rval_eq() {
    auto rval = parser_rval_comp();
    auto rval_ = parser_rval_eq_();
//    cout << typeid(*rval_) <<"\n";
//    cout << typeid(EmptyASTnode) <<"\n";
    if (rval_->isEmpty)
        return rval;
    return new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
}

// RVAL_AND_ -> eq RVAL_EQ RVAL_AND_ | ne RVAL_EQ RVAL_AND_ | epsilon
ExprASTnode* parser_rval_and_() {
//    TODO check follow set

    if (CurTok.type == EQ or CurTok.type == NE) {
        auto op = CurTok;
        getNextToken();
        auto rval = parser_rval_eq();
        auto rval_ = parser_rval_and_();
        if (rval_->isEmpty)
            return new HalfASTnode(op, rval);
        auto expr = new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
        return new HalfASTnode(op, expr);
    }

    return new EmptyASTnode();
}

// RVAL_AND -> RVAL_EQ RVAL_AND_
ExprASTnode* parser_rval_and() {
    auto rval = parser_rval_eq();
    auto rval_ = parser_rval_and_();
    if (typeid(*rval) == typeid(EmptyASTnode))
        return rval;
    return new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
}

// RVAL_OR_ -> and RVAL_AND RVAL_OR_ | epsilon
ExprASTnode* parser_rval_or_() {
//    TODO check follow set

    if (CurTok.type == AND) {
        auto op = CurTok;
        getNextToken();
        auto rval = parser_rval_or();
        auto rval_ = parser_rval_or_();
        if (rval_->isEmpty)
            return new HalfASTnode(op, rval);
        auto expr = new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
        return new HalfASTnode(op, expr);
    }

    return new EmptyASTnode();
}

// RVAL_OR -> RVAL_AND RVAL_OR_
ExprASTnode* parser_rval_or() {
    auto rval = parser_rval_and();
    auto rval_ = parser_rval_or_();
    if (typeid(*rval) == typeid(EmptyASTnode))
        return rval;
    return new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
}

// RVAL_ -> or RVAL_OR RVAL_ | epsilon
ExprASTnode* parser_rval_() {
//    TODO check follow set

    if (CurTok.type == OR) {
        auto op = CurTok;
        getNextToken();
        auto rval = parser_rval_or();
        auto rval_ = parser_rval_();
        if (rval_->isEmpty)
            return new HalfASTnode(op, rval);
        auto expr = new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
        return new HalfASTnode(op, expr);
    }

    return new EmptyASTnode();
}

// RVAL -> RVAL_OR RVAL_
ExprASTnode* parser_rval() {
    auto rval_or = parser_rval_or();
    auto rval_ = parser_rval_();
    if (typeid(*rval_) == typeid(EmptyASTnode*))
        return rval_or;
    return new BinaryASTnode(rval_or, static_cast<HalfASTnode*>(rval_));
}

// EXPR -> ident equal EXPR | RVAL
ExprASTnode* parser_expr() {
    CurTok.print_as_string();
    auto temp = CurTok;
    getNextToken();
    CurTok.print_as_string();
    if (CurTok.type == EQ) {
        putBackToken(temp);
//      Carefull doing somehting dangerous here... might break maybe
        auto ident = parser_ident();
        cout << ident->to_string("hello1", true);
        TOKEN op;
        if (CurTok.type == EQ) {
            op = CurTok;
            getNextToken();
        }
        auto expr = parser_expr();
        cout << "printing expr\n";
        cout << expr->to_string("hello2", true);
        return new BinaryASTnode(ident, op, expr);
    }
    putBackToken(temp);
    CurTok.print_as_string();
    cout << "before expr\n";
    auto temp2 = parser_rval();
    cout << "after expr\n";
    cout << temp2->to_string("hello3", true);
    cout << "after print\n";
    return temp2;
}

// EXPR_STMT -> ; | EXPR ;
Expr_stmtASTnode* parser_expr_stmt() {
    ExprASTnode* expr;
    if (CurTok.type == SC) {
        getNextToken();
        return new Expr_stmtASTnode();
    }
    expr = parser_expr();
    if (CurTok.type == SC)
        getNextToken();
    return new Expr_stmtASTnode(expr);
}

// ELSE_STMT -> else BLOCK | epsilon
BlockASTnode* parser_else_stmt() {
    if (CurTok.type == ELSE)
        getNextToken();
    else
        return new BlockASTnode();

    return parser_block();
}

// IF_STMT -> if ( EXPR ) BLOCK ELSE_STMT
If_stmtASTnode* parser_if_stmt() {
    if (CurTok.type == IF)
        getNextToken();

    if (CurTok.type == LPAR)
        getNextToken();

    auto expr = parser_expr();

    if (CurTok.type == RPAR)
        getNextToken();

    auto if_block = parser_block();

    auto else_block = parser_else_stmt();

    return new If_stmtASTnode(expr, if_block, else_block);
}

// WHILE_STMT -> while ( EXPR ) STMT
While_stmtASTnode* parser_while_stmt() {
    if (CurTok.type == WHILE)
        getNextToken();

    if (CurTok.type == LPAR)
        getNextToken();

    auto expr = parser_expr();

    if (CurTok.type == RPAR)
        getNextToken();

    auto stmt = parser_stmt();
    return new While_stmtASTnode(expr, stmt);
}

// RETURN_STMT_ -> ; | EXPR ;
Return_stmtASTnode* parser_return_stmt_() {
    if (CurTok.type == SC)
        return new Return_stmtASTnode();
    auto expr = parser_expr();
    return new Return_stmtASTnode(expr);
}

// RETURN_STMT -> return RETURN_STMT_
Return_stmtASTnode* parser_return_stmt() {
    if (CurTok.type == RETURN)
        getNextToken();

    return parser_return_stmt_();
}

// STMT -> EXPR_STMT | BLOCK | IF_STMT | WHILE_STMT | RETURN_STMT
StmtASTnode* parser_stmt() {
//    cout << "parsing stmt\n";
    if (CurTok.type == IF){
        cout << "FOUND THE IF\n";
        return parser_if_stmt();
    }
    if (CurTok.type == WHILE)
        return parser_while_stmt();
    if (CurTok.type == RETURN)
        return parser_return_stmt();
    if (CurTok.type == LBRA)
        return parser_block();
    if (CurTok.type == LPAR or CurTok.type == IDENT or CurTok.type == NOT or CurTok.type == MINUS or CurTok.type == SC)
    {
        auto temp = parser_expr_stmt();
        cout << temp->to_string("hello5", true);
        return temp;
    }
}

// TODO not recursive
// STMT_LIST -> STMT STMT_LIST | epsilon
vector<StmtASTnode*> parser_stmt_list() {
    vector<StmtASTnode*> stmt_list = vector<StmtASTnode*>();
    while (CurTok.type != RBRA) {
        auto temp = parser_stmt();
        cout << temp->to_string("hello6", true);
        stmt_list.push_back(temp);
    }
    return stmt_list;
}

// LOCAL_DECL -> VAR_TYPE IDENT ;
Local_declASTnode* parser_local_decl() {
    auto type = parser_fun_type();
    auto ident = parser_ident();

    if (CurTok.type == SC)
        getNextToken();

    return new Local_declASTnode(type, ident);
}

// TODO not reccursive
// LOCAL_DECLS -> LOCAL_DECL LOCAL_DECLS
vector<Local_declASTnode*> parser_local_decls() {
    auto local_decls = vector<Local_declASTnode*>();

    while (CurTok.type == INT_TOK or CurTok.type == FLOAT_TOK or CurTok.type == BOOL_TOK)
        local_decls.push_back(parser_local_decl());

    return local_decls;
}

// BLOCK -> LB LOCAL_DECLS STMT_LIST RB
BlockASTnode* parser_block() {
    vector<Local_declASTnode*> local_decls;
    vector<StmtASTnode*> stmt_list;

    if (CurTok.type == LBRA)
        getNextToken();

    local_decls = parser_local_decls();
    stmt_list = parser_stmt_list();

    if (CurTok.type == RBRA)
        getNextToken();

    return new BlockASTnode(local_decls, stmt_list);
}

// FUN_DECL -> FUN_TYPE ident ( PARAM ) BLOCK
Fun_DeclASTnode* parser_fun_decl() {
    auto type = parser_fun_type();
    auto ident = parser_ident();

    if (CurTok.type == LPAR)
        getNextToken();

    auto params = move(parser_params());

    if (CurTok.type == RPAR)
        getNextToken();

    auto block = move(parser_block());

    return new Fun_DeclASTnode(type, ident, params, block);
}

// VAR_DECL -> VAR_TYPE ident ;
Var_DeclASTnode* parser_var_decl() {
    auto type = parser_fun_type();
    auto ident = parser_ident();

    if (CurTok.type == SC)
        getNextToken();

    return new Var_DeclASTnode(type, ident);
}

// DECL -> VAR_DECL | FUN_DECL
DeclASTnode* parser_decl() {
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
vector<DeclASTnode*> parser_decl_list_() {
    auto decl_list = vector<DeclASTnode*>();

    while (CurTok.type == VOID_TOK or CurTok.type == INT_TOK or CurTok.type == FLOAT_TOK or CurTok.type == BOOL_TOK)
        decl_list.push_back(parser_decl());

    return decl_list;
}

// DECL_LIST -> DECL DECL_LIST_
Decl_listASTnode* parser_decl_list(){
    auto temp = move(parser_decl());

    auto decl_list = move(parser_decl_list_());

    decl_list.push_back(temp);

    return new Decl_listASTnode(decl_list);
}

// PROGRAM -> EXTERN_LIST DECL_LIST | DECL_LIST
void parser() {
    Extern_listASTnode* extern_list;
    Decl_listASTnode* decl_list;
    getNextToken();
//    auto temp = new EmptyASTnode();
//    if (typeid(*temp) == typeid(EmptyASTnode))
//        cout << "true";
    if (CurTok.type == EXTERN)
        extern_list = parser_extern_list();
    decl_list = parser_decl_list();
    cout << extern_list->to_string("", false);
    cout << decl_list->to_string("", true);
}
