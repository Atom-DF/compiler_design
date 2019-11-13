//
// Created by Atom Filippi on 22/10/2019.
//

#ifndef CODE_PARSER_H
#define CODE_PARSER_H

#include "../ast/ast.h"

IdentASTnode* parser_ident();

Var_TypeASTnode* parser_var_type();

TypeASTnode* parser_fun_type();

ParamASTnode* parser_param();

std::vector<ParamASTnode*> parser_param_list_();

ParamsASTnode* parser_param_list();

ParamsASTnode* parser_params();

ExternASTnode* parser_extern();

std::vector<ExternASTnode*> parser_extern_list_();

Extern_listASTnode* parser_extern_list();

std::vector<ExprASTnode*> parser_args_list_();

std::vector<ExprASTnode*> parser_args_list();

std::vector<ExprASTnode*> parser_args();

std::vector<ExprASTnode*> parser_rval_fun();

ExprASTnode* parser_rval_not();

ExprASTnode* parser_rval_mul();

ExprASTnode* parser_rval_add_();

ExprASTnode* parser_rval_add();

ExprASTnode* parser_rval_comp_();

ExprASTnode* parser_rval_comp();

ExprASTnode* parser_rval_eq_();

ExprASTnode* parser_rval_eq();

ExprASTnode* parser_rval_and_();

ExprASTnode* parser_rval_and();

ExprASTnode* parser_rval_or_();

ExprASTnode* parser_rval_or();

ExprASTnode* parser_rval_();

ExprASTnode* parser_rval();

ExprASTnode* parser_expr();

Expr_stmtASTnode* parser_expr_stmt();

BlockASTnode* parser_else_stmt();

If_stmtASTnode* parser_if_stmt();

While_stmtASTnode* parser_while_stmt();

Return_stmtASTnode* parser_return_stmt_();

Return_stmtASTnode* parser_return_stmt();

StmtASTnode* parser_stmt();

std::vector<StmtASTnode*> parser_stmt_list_();

Local_declASTnode* parser_local_decl();

std::vector<Local_declASTnode*> parser_local_decls();

BlockASTnode* parser_block();

Fun_DeclASTnode* parser_fun_decl();

Var_DeclASTnode* parser_var_decl();

DeclASTnode* parser_decl();

std::vector<DeclASTnode*> parser_decl_list_();

Decl_listASTnode* parser_decl_list();

void parser();

#endif //CODE_PARSER_H
