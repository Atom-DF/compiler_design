//
// Created by Atom Filippi on 22/10/2019.
//

#ifndef CODE_PARSER_H
#define CODE_PARSER_H

#include "../ast/ast.h"

std::unique_ptr<IdentASTnode> parser_ident();

std::unique_ptr<Var_TypeASTnode> parser_var_type();

std::unique_ptr<TypeASTnode> parser_fun_type();

std::unique_ptr<ParamASTnode> parser_param();

std::vector<std::unique_ptr<ParamASTnode>> parser_param_list_();

std::unique_ptr<ParamsASTnode> parser_param_list();

std::unique_ptr<ParamsASTnode> parser_params();

std::unique_ptr<ExternASTnode> parser_extern();

std::vector<std::unique_ptr<ExternASTnode>> parser_extern_list_();

std::unique_ptr<Extern_listASTnode> parser_extern_list();

std::vector<std::unique_ptr<ExprASTnode>> parser_args_list_();

std::vector<std::unique_ptr<ExprASTnode>> parser_args_list();

std::vector<std::unique_ptr<ExprASTnode>> parser_args();

std::vector<std::unique_ptr<ExprASTnode>> parser_rval_fun();

std::unique_ptr<ExprASTnode> parser_rval_not();

std::unique_ptr<ExprASTnode> parser_rval_mul();

std::unique_ptr<ExprASTnode> parser_rval_add_();

std::unique_ptr<ExprASTnode> parser_rval_add();

std::unique_ptr<ExprASTnode> parser_rval_comp_();

std::unique_ptr<ExprASTnode> parser_rval_comp();

std::unique_ptr<ExprASTnode> parser_rval_eq_();

std::unique_ptr<ExprASTnode> parser_rval_eq();

std::unique_ptr<ExprASTnode> parser_rval_and_();

std::unique_ptr<ExprASTnode> parser_rval_and();

std::unique_ptr<ExprASTnode> parser_rval_or_();

std::unique_ptr<ExprASTnode> parser_rval_or();

std::unique_ptr<ExprASTnode> parser_rval_();

std::unique_ptr<ExprASTnode> parser_rval();

std::unique_ptr<ExprASTnode> parser_expr();

std::unique_ptr<Expr_stmtASTnode> parser_expr_stmt();

std::unique_ptr<BlockASTnode> parser_else_stmt();

std::unique_ptr<If_stmtASTnode> parser_if_stmt();

std::unique_ptr<While_stmtASTnode> parser_while_stmt();

std::unique_ptr<Return_stmtASTnode> parser_return_stmt_();

std::unique_ptr<Return_stmtASTnode> parser_return_stmt();

std::unique_ptr<StmtASTnode> parser_stmt();

std::vector<std::unique_ptr<StmtASTnode>> parser_stmt_list_();

std::unique_ptr<Local_declASTnode> parser_local_decl();

std::vector<std::unique_ptr<Local_declASTnode>> parser_local_decls();

std::unique_ptr<BlockASTnode> parser_block();

std::unique_ptr<Fun_DeclASTnode> parser_fun_decl();

std::unique_ptr<Var_DeclASTnode> parser_var_decl();

std::unique_ptr<DeclASTnode> parser_decl();

std::vector<std::unique_ptr<DeclASTnode>> parser_decl_list_();

std::unique_ptr<Decl_listASTnode> parser_decl_list();

void parser();

#endif //CODE_PARSER_H
