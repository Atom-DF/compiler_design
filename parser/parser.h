//
// Created by Atom Filippi on 22/10/2019.
//

#ifndef CODE_PARSER_H
#define CODE_PARSER_H

#include "../ast/ast.h"

static std::unique_ptr<TypeASTnode> parser_Var_Type();

static std::unique_ptr<TypeASTnode> parser_Fun_Type();

static std::unique_ptr<ParamASTnode> parser_Param();

static std::unique_ptr<ParamsASTnode> parser_Param_List();

static std::unique_ptr<ParamsASTnode> parser_Params();

static std::unique_ptr<Fun_PrototypeASTnode> parser_Fun_Prototype();

static std::unique_ptr<Fun_PrototypeASTnode> parser_Extern_Def();

static std::unique_ptr<Fun_PrototypesASTnode> parser_Extern_Def_List();

//static std::unique_ptr<> parser_Decl_Def();

static std::unique_ptr<DeclASTnode> parser_Local_Decl();

static std::unique_ptr<Local_DeclsASTnode> parser_Local_Decl_List();

static std::unique_ptr<BlockASTnode> parser_Block();

void parser();

#endif //CODE_PARSER_H
