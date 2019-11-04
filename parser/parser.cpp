//
// Created by Atom Filippi on 22/10/2019.
//

//===----------------------------------------------------------------------===//
// Recursive Descent Parser - Function call for each production
//===----------------------------------------------------------------------===//

#include <vector>
#include <queue>
#include <memory>

#include "../mccomp.h"
#include "../ast/ast.h"
#include "../exceptions/exceptions.h"

using namespace std;
using namespace llvm;


// General parser functions

static unique_ptr<IdentASTnode> parser_Ident() {
    if (CurTok.type == IDENT) {
        auto temp = make_unique<IdentASTnode>(CurTok, IdentifierStr);
        getNextToken();
        return temp;
    }
}

static unique_ptr<TypeASTnode> parser_Var_Type() {
    if (CurTok.type == INT_TOK or CurTok.type == BOOL_TOK or CurTok.type == FLOAT_TOK) {
        auto temp = CurTok;
        getNextToken();
        return make_unique<TypeASTnode>(temp, temp.lexeme);
    }
}

static unique_ptr<TypeASTnode> parser_Fun_Type() {
    if (CurTok.type == VOID_TOK) {
        auto temp = CurTok;
        getNextToken();
        return make_unique<TypeASTnode>(temp, temp.lexeme);
    }

    return parser_Var_Type();
}

static unique_ptr<ParamASTnode> parser_Param() {
    unique_ptr<TypeASTnode> type;
    unique_ptr<IdentASTnode> ident;

    type = parser_Var_Type();

    ident = parser_Ident();

    if (CurTok.type == COMMA)
        getNextToken();

    return make_unique<ParamASTnode>(move(type), move(ident));
}

static unique_ptr<ParamsASTnode> parser_Param_List() {
    vector<unique_ptr<ParamASTnode>> params = vector<unique_ptr<ParamASTnode>>();

    while (CurTok.type != RPAR)
        params.push_back(move(parser_Param()));

    return make_unique<ParamsASTnode>(false, false, move(params));
}

static unique_ptr<ParamsASTnode> parser_Params() {
    if (CurTok.type == RPAR) {
//      We want to consume the right parentheses in parser_Fun_Prototype. So no consuming here.
        return make_unique<ParamsASTnode>(true, false, vector<unique_ptr<ParamASTnode>>());
    }

    if (CurTok.type == VOID_TOK) {
        getNextToken();
        return make_unique<ParamsASTnode>(false, true, vector<unique_ptr<ParamASTnode>>());
    }

    return parser_Param_List();
}

static unique_ptr<Fun_PrototypeASTnode> parser_Fun_Prototype() {
    unique_ptr<TypeASTnode> type;
    unique_ptr<IdentASTnode> ident;
    unique_ptr<ParamsASTnode> params;

    type = parser_Fun_Type();

    if (CurTok.type == IDENT) {
        ident = make_unique<IdentASTnode>(CurTok, CurTok.lexeme);
        getNextToken();
    }

    if (CurTok.type == LPAR) {
        getNextToken();
    }

    params = parser_Params();

    if (CurTok.type == RPAR) {
        getNextToken();
    }

    return make_unique<Fun_PrototypeASTnode>(move(type), move(ident), move(params));
}


// Extern related functions

static unique_ptr<Fun_PrototypeASTnode> parser_Extern_Def() {
    unique_ptr<Fun_PrototypeASTnode> extern_def;

    if (CurTok.type == EXTERN) {
        getNextToken();
    }

    extern_def = parser_Fun_Prototype();

    if (CurTok.type == SC) {
        getNextToken();
    }

    return extern_def;
}

static unique_ptr<Fun_PrototypesASTnode> parser_Extern_Def_List() {
    vector<unique_ptr<Fun_PrototypeASTnode>> externs = vector<unique_ptr<Fun_PrototypeASTnode>>();

    externs.push_back(move(parser_Extern_Def()));

    while (CurTok.type == EXTERN)
        externs.push_back(move(parser_Extern_Def()));

    return make_unique<Fun_PrototypesASTnode>(move(externs));
}


// Declaration related functions

//Block definitions

static unique_ptr<DeclASTnode> parser_Local_Decl() {
    unique_ptr<TypeASTnode> type;
    unique_ptr<IdentASTnode> ident;

    type = parser_Var_Type();

    ident = parser_Ident();

    if (CurTok.type == SC)
        getNextToken();

    return make_unique<DeclASTnode>(move(type), move(ident));
}

static unique_ptr<Local_DeclsASTnode> parser_Local_Decl_List() {
    vector<unique_ptr<DeclASTnode>> decls = vector<unique_ptr<DeclASTnode>>();

    while (CurTok.type == INT_TOK or CurTok.type == BOOL_TOK or CurTok.type == FLOAT_TOK)
        decls.push_back(move(parser_Local_Decl()));


    return make_unique<Local_DeclsASTnode>(move(decls));
}

static unique_ptr<BlockASTnode> parser_Block() {
    unique_ptr<Local_DeclsASTnode> local_decls = parser_Local_Decl_List();

    return make_unique<BlockASTnode>(move(local_decls));
    // parse stmts
}

// Decl_List definitions

static unique_ptr<DeclarationASTnode> parser_Declaration() {
    unique_ptr<TypeASTnode> type;
    unique_ptr<IdentASTnode> ident;
    unique_ptr<ParamsASTnode> params;
    unique_ptr<Fun_PrototypeASTnode> prototype;
    unique_ptr<BlockASTnode> block;

    type = parser_Var_Type();

    ident = parser_Ident();

    if (CurTok.type == SC) {
        getNextToken();
        return make_unique<Var_DeclASTnode>(move(type), move(ident));
    }

    if (CurTok.type == LPAR)
        getNextToken();

    params = parser_Params();

    if (CurTok.type == RPAR)
        getNextToken();

    prototype = make_unique<Fun_PrototypeASTnode>(move(type), move(ident), move(params));

    if (CurTok.type == RBRA)
        getNextToken();

    block = parser_Block();

    if (CurTok.type == LBRA)
        getNextToken();

    return make_unique<Fun_DeclASTnode>(move(prototype), move(block));

};

static unique_ptr<DeclarationsASTnode> parser_Decl_List() {
    vector<unique_ptr<DeclarationASTnode>> decls = vector<unique_ptr<DeclarationASTnode>>();

    while (CurTok.type == INT_TOK or CurTok.type == BOOL_TOK or CurTok.type == FLOAT_TOK or CurTok.type == VOID_TOK)
        decls.push_back(move(parser_Declaration()));


    return make_unique<DeclarationsASTnode>(move(decls));
};



// Entrypoint for the parser.
void parser() {
    unique_ptr<Fun_PrototypesASTnode> extern_list;
    unique_ptr<DeclarationsASTnode> decl_list;

//  Get the first token.
    getNextToken();
    if (CurTok.type == EXTERN)
        extern_list = parser_Extern_Def_List();

    decl_list = parser_Decl_List();

    cout << extern_list->to_string();
    cout << decl_list->to_string();
}