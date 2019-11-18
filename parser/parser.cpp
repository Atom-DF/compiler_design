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

using namespace std;
using namespace llvm;

// TODO add epsilon checking everywhere lol

// parse ident
IdentASTnode* parser_ident() {
    if (CurTok.type == IDENT) {
        auto temp = CurTok;
        getNextToken();
        return new IdentASTnode(temp);
    } else
        LogError("Expecting an identifier");
    return nullptr;
}

// VAR_TYPE -> int | float | bool
Var_TypeASTnode* parser_var_type() {
    if (CurTok.type == FLOAT_TOK or CurTok.type == BOOL_TOK or CurTok.type == INT_TOK) {
        auto temp = CurTok;
        getNextToken();
        return new Var_TypeASTnode(temp);
    } else
        LogError("Expected a float, bool or int token.");
    return nullptr;
}

// TYPE_SPEC -> void | VAR_TYPE
TypeASTnode* parser_fun_type() {
    if (CurTok.type == VOID_TOK) {
        auto temp = CurTok;
        getNextToken();
        return new Fun_TypeASTnode(temp);
    } else
        if (CurTok.type != FLOAT_TOK and CurTok.type != INT_TOK and CurTok.type != BOOL_TOK)
            LogError("Expected float, int, bool or void token.");
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
    else
        LogError("Expected ) token or , param.");

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
    if (CurTok.type == FLOAT_TOK or CurTok.type == BOOL_TOK or CurTok.type == INT_TOK)
        return parser_param_list();
    else
        LogError("Expected void, ) or var_type tokens.");
    return nullptr;
}

// EXTERN -> extern TYPE_SPEC ident ( PARAMS ) ;
ExternASTnode* parser_extern() {
    TypeASTnode* type;
    IdentASTnode* ident;
    ParamsASTnode* params;

    if (CurTok.type == EXTERN)
        getNextToken();
    else
        LogError("Expected extern token.");

    type = parser_fun_type();
    ident = parser_ident();

    if (CurTok.type == LPAR)
        getNextToken();
    else
        LogError("Expected ( token.");

    params = parser_params();

    if (CurTok.type == RPAR)
        getNextToken();
    else
        LogError("Expected ) token.");

    if (CurTok.type == SC)
        getNextToken();
    else
        LogError("Expected ; token.");

    return new ExternASTnode(type, ident, params);
}

// TODO not reccursive
// EXTERN_LIST' -> EXTERN EXTERN_LIST' | epsilon
vector<ExternASTnode*> parser_extern_list_() {
    vector<ExternASTnode*> extern_list = vector<ExternASTnode*>();

    while (CurTok.type == EXTERN)
        extern_list.push_back(parser_extern());

    if (CurTok.type == VOID_TOK or CurTok.type == FLOAT_TOK or CurTok.type == INT_TOK or CurTok.type == BOOL_TOK)
        return extern_list;
    else
        LogError("Expected void, int, float or bool token.");
    return vector<ExternASTnode*>();
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

    if (CurTok.type == RPAR)
        return vector<ExprASTnode*>();
    else
        LogError("Expected , expr or ) tokens.");
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

    vector<int> set{ IDENT, MINUS, NOT, LPAR, INT_LIT, FLOAT_LIT, BOOL_LIT };
    if (find(set.begin(), set.end(), CurTok.type) != set.end())
        return parser_args_list();
    else
        LogError("Expected expr or ) token.");
    return vector<ExprASTnode*>();
}

// RVAL_FUN -> ( ARGS ) | epsilon
vector<ExprASTnode*> parser_rval_fun() {
    if (CurTok.type == LPAR) {
        getNextToken();
        auto args = parser_args();
        if (CurTok.type == RPAR)
            getNextToken();
        else
            LogError("Expected ) token.");
        return args;
    }

    vector<int> set{ ASTERIX, DIV, MOD, PLUS, MINUS, LE, LT, GE, GT, EQ, NE, AND, OR, COMMA, RPAR, SC };
    if (find(set.begin(), set.end(), CurTok.type) != set.end())
        return vector<ExprASTnode*>();
    else
        LogError("Expected arguments for a function, the rest of an expression, a ) or ; tokens.");
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
        auto ident = parser_ident();
        auto arguments = parser_rval_fun();
        if (arguments.empty())
            return ident;
        return new Ident_funASTnode(ident, arguments);
    }

    LogError("Expected ( expr ), ident or a lit token.");
    return nullptr;
}

// RVAL_MUL -> minus RVAL_MUL | not RVAL_MUL | RVAL_NOT
ExprASTnode* parser_rval_mul() {
    if (CurTok.type == NOT or CurTok.type == MINUS) {
        auto op = CurTok;
        getNextToken();
        auto expr = parser_rval_mul();
        return new UnaryASTnode(op, expr);
    }
    return parser_rval_not();
}

// RVAL_ADD_ -> mul RVAL_MUL RVAL_ADD_ | div RVAL_MUL RVAL_ADD_ | mod RVAL_MUL RVAL_ADD_ | epsilon
ExprASTnode* parser_rval_add_() {
    if (CurTok.type == ASTERIX or CurTok.type == DIV or CurTok.type == MOD) {
        auto op = CurTok;
        getNextToken();
        auto rval = parser_rval_mul();
        auto rval_ = parser_rval_add_();
        if (rval_->isEmpty())
            return new HalfASTnode(op, rval);
        auto expr = new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
        return new HalfASTnode(op, expr);
    }

    vector<int> set{ PLUS, MINUS, LE, LT, GE, GT, EQ, NE, AND, OR, COMMA, RPAR, SC };
    if (find(set.begin(), set.end(), CurTok.type) != set.end())
        return new EmptyASTnode();
    else
        LogError("Expected the rest of an expression (plus and onwards), a ) or ; tokens.");
    return nullptr;
}

// RVAL_ADD -> RVAL_MUL RVAL_ADD_
ExprASTnode* parser_rval_add() {
    auto rval = parser_rval_mul();
    auto rval_ = parser_rval_add_();
    if (rval_->isEmpty())
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
        if (rval_->isEmpty())
            return new HalfASTnode(op, rval);
        auto expr = new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
        return new HalfASTnode(op, expr);
    }

    vector<int> set{ LE, LT, GE, GT, EQ, NE, AND, OR, COMMA, RPAR, SC };
    if (find(set.begin(), set.end(), CurTok.type) != set.end())
        return new EmptyASTnode();
    else
        LogError("Expected bolean operators, a , , a ) or ; tokens.");
    return nullptr;
}

// RVAL_COMP -> RVAL_ADD RVAL_COMP_
ExprASTnode* parser_rval_comp() {
    auto rval = parser_rval_add();
    auto rval_ = parser_rval_comp_();
    if (rval_->isEmpty())
        return rval;
    return new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
}

// RVAL_EQ_ -> g RVAL_COMP RVAL_EQ_ | ge RVAL_COMP RVAL_EQ_ | l RVAL_COMP RVAL_EQ_ | le RVAL_COMP RVAL_EQ_ | epsilon
ExprASTnode* parser_rval_eq_() {
    if (CurTok.type == LT or CurTok.type == GT or CurTok.type == GE or CurTok.type == LE) {
        auto op = CurTok;
        getNextToken();
        auto rval = parser_rval_comp();
        auto rval_ = parser_rval_eq_();
        if (rval_->isEmpty())
            return new HalfASTnode(op, rval);
        auto expr = new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
        return new HalfASTnode(op, expr);
    }

    vector<int> set{ EQ, NE, AND, OR, COMMA, RPAR, SC };
    if (find(set.begin(), set.end(), CurTok.type) != set.end())
        return new EmptyASTnode();
    else
        LogError("Expected ==, !=, and, or , a ) or ; tokens.");
    return nullptr;
}

// RVAL_EQ -> RVAL_COMP RVAL_EQ_
ExprASTnode* parser_rval_eq() {
    auto rval = parser_rval_comp();
    auto rval_ = parser_rval_eq_();
    if (rval_->isEmpty())
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
        if (rval_->isEmpty())
            return new HalfASTnode(op, rval);
        auto expr = new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
        return new HalfASTnode(op, expr);
    }

    vector<int> set{ AND, OR, COMMA, RPAR, SC };
    if (find(set.begin(), set.end(), CurTok.type) != set.end())
        return new EmptyASTnode();
    else
        LogError("Expected  and, or , a ) or ; tokens.");
    return nullptr;
}

// RVAL_AND -> RVAL_EQ RVAL_AND_
ExprASTnode* parser_rval_and() {
    auto rval = parser_rval_eq();
    auto rval_ = parser_rval_and_();
    if (rval_->isEmpty())
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
        if (rval_->isEmpty())
            return new HalfASTnode(op, rval);
        auto expr = new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
        return new HalfASTnode(op, expr);
    }

    vector<int> set{ OR, COMMA, RPAR, SC };
    if (find(set.begin(), set.end(), CurTok.type) != set.end())
        return new EmptyASTnode();
    else
        LogError("Expected or, a ) or ; tokens.");
    return nullptr;
}

// RVAL_OR -> RVAL_AND RVAL_OR_
ExprASTnode* parser_rval_or() {
    auto rval = parser_rval_and();
    auto rval_ = parser_rval_or_();
    if (rval_->isEmpty())
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
        if (rval_->isEmpty())
            return new HalfASTnode(op, rval);
        auto expr = new BinaryASTnode(rval, static_cast<HalfASTnode*>(rval_));
        return new HalfASTnode(op, expr);
    }

    vector<int> set{ COMMA, RPAR, SC };
    if (find(set.begin(), set.end(), CurTok.type) != set.end())
        return new EmptyASTnode();
    else
        LogError("Expected a comma, a ) or ; tokens.");
    return nullptr;
}

// RVAL -> RVAL_OR RVAL_
ExprASTnode* parser_rval() {
    auto rval_or = parser_rval_or();
    auto rval_ = parser_rval_();
    if (rval_->isEmpty())
        return rval_or;
    return new BinaryASTnode(rval_or, static_cast<HalfASTnode*>(rval_));
}

// EXPR -> ident equal EXPR | RVAL
ExprASTnode* parser_expr() {
    auto temp = CurTok;
    getNextToken();
    if (CurTok.type == ASSIGN) {
        putBackToken(temp);
        auto ident = parser_ident();
        TOKEN op;
        if (CurTok.type == ASSIGN) {
            op = CurTok;
            getNextToken();
        }
        auto expr = parser_expr();
        return new BinaryASTnode(ident, op, expr);
    }
    putBackToken(temp);
    return parser_rval();
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
    else {
        vector<int> set{ SC, IDENT, MINUS, NOT, LPAR, INT_LIT, FLOAT_LIT, BOOL_LIT, IF, RETURN, LBRA, WHILE, RBRA };
        if (find(set.begin(), set.end(), CurTok.type) != set.end())
            return new BlockASTnode();
        else
            LogError("Expected an else stmt, a new statement or the end of the funciton.");
    }
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

    if (else_block->Empty)
        return new If_stmtASTnode(expr, if_block);

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
    if (CurTok.type == SC)
        getNextToken();
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
    LogError("Expected a stmt (if, while, return, block or expr).");
    return nullptr;
}

// TODO not recursive
// STMT_LIST -> STMT STMT_LIST | epsilon
vector<StmtASTnode*> parser_stmt_list() {
    vector<StmtASTnode*> stmt_list = vector<StmtASTnode*>();
    while (CurTok.type != RBRA)
        stmt_list.push_back(parser_stmt());

    vector<int> set{ RBRA };
    if (find(set.begin(), set.end(), CurTok.type) != set.end())
        return stmt_list;
    else
        LogError("Expected a right bracket at the end of the stmt list.");
    return vector<StmtASTnode*>();
}

// LOCAL_DECL -> VAR_TYPE IDENT ;
Local_declASTnode* parser_local_decl() {
    auto type = parser_fun_type();
    auto ident = parser_ident();

    if (CurTok.type == SC)
        getNextToken();
    else
        LogError("Expected a semi colon at the end of a local declaration");

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
    else
        LogError("Expected left bracket, at the beginning of a block.");

    local_decls = parser_local_decls();
    stmt_list = parser_stmt_list();

    if (CurTok.type == RBRA)
        getNextToken();
    else
        LogError("Expected right bracket, at the end of a block.");

    return new BlockASTnode(local_decls, stmt_list);
}

// FUN_DECL -> FUN_TYPE ident ( PARAM ) BLOCK
Fun_DeclASTnode* parser_fun_decl() {
    auto type = parser_fun_type();
    auto ident = parser_ident();

    if (CurTok.type == LPAR)
        getNextToken();
    else
        LogError("Expected a ( in function declaration.");

    auto params = move(parser_params());

    if (CurTok.type == RPAR)
        getNextToken();
    else
        LogError("Expected a ) in function declaration.");

    auto block = move(parser_block());

    return new Fun_DeclASTnode(type, ident, params, block);
}

// VAR_DECL -> VAR_TYPE ident ;
Var_DeclASTnode* parser_var_decl() {
    auto type = parser_fun_type();
    auto ident = parser_ident();

    if (CurTok.type == SC)
        getNextToken();
    else
        LogError("Expected a semi colon at the end of the variable declaration.");

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

    if (CurTok.type == EOF_TOK)
        return decl_list;
    else
        LogError("Expected end of file after declaration list.");
    return vector<DeclASTnode*>();
}

// DECL_LIST -> DECL DECL_LIST_
Decl_listASTnode* parser_decl_list(){
    auto temp = move(parser_decl());

    auto decl_list = move(parser_decl_list_());

    decl_list.push_back(temp);

    return new Decl_listASTnode(decl_list);
}

// PROGRAM -> EXTERN_LIST DECL_LIST | DECL_LIST
RootASTnode* parser() {
    Extern_listASTnode* extern_list;
    Decl_listASTnode* decl_list;
    bool externs = false;
    getNextToken();
    if (CurTok.type == EXTERN) {
        externs = true;
        extern_list = parser_extern_list();
    }
    decl_list = parser_decl_list();
    if (externs)
        return new RootASTnode(extern_list, decl_list);
    return new RootASTnode(decl_list);
}
