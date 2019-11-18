//
// Created by Atom Filippi on 22/10/2019.
//

#ifndef CODE_AST_H
#define CODE_AST_H

#include <typeinfo>

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/DataLayout.h"
#include "../mccomp.h"

//===----------------------------------------------------------------------===//
// AST nodes
//===----------------------------------------------------------------------===//

/// ASTnode - Base class for all AST nodes.
class ASTnode {
public:
    virtual ~ASTnode() {}

    virtual llvm::Value *codegen() = 0;

    virtual std::string to_string(std::string prefix, bool last) const = 0;
};

class BlockASTnode;

// expr (abstract)
class ExprASTnode : public ASTnode {
public:
    ExprASTnode() {}

    virtual bool isEmpty() = 0;

    virtual llvm::Value *codegen() override {return nullptr;};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// Half completed binary
class HalfASTnode : public ExprASTnode {
public:
    TOKEN Op;
    ExprASTnode* Expr;
public:
    HalfASTnode(TOKEN op, ExprASTnode* expr):
    Op(op), Expr(expr) {}

    virtual bool isEmpty() override {return false;};

    virtual llvm::Value *codegen() override {return nullptr;};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// empty, used to signal it was an epsilon to the parser
class EmptyASTnode : public ExprASTnode {
public:
    EmptyASTnode() {}

    virtual bool isEmpty() override {return true;};

    virtual llvm::Value *codegen() override {return nullptr;};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// binary operator
class BinaryASTnode : public ExprASTnode {
    TOKEN Op;
    ExprASTnode* Left;
    ExprASTnode* Right;
public:
    BinaryASTnode(ExprASTnode* left, TOKEN op, ExprASTnode* right)
    : Op(op), Left((left)), Right((right)) {}

    BinaryASTnode(ExprASTnode* expr, HalfASTnode* half) :
    BinaryASTnode(expr, half->Op, half->Expr) {}

    virtual bool isEmpty() override {return false;};

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// unary operator
class UnaryASTnode : public ExprASTnode {
    TOKEN Op;
    ExprASTnode* Expr;
public:
    UnaryASTnode(TOKEN op, ExprASTnode* expr)
            : Op(op), Expr((expr)) {}

    virtual bool isEmpty() override {return false;};

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};

/// IntASTnode - Class for integer literals like 1, 2, 10.
class IntASTnode : public ExprASTnode {
    int Val;
    TOKEN Tok;
    std::string Name;

public:
    IntASTnode(TOKEN tok, int val) : Val(val), Tok(tok) {}

    virtual bool isEmpty() override {return false;};

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};

/// FloatASTnode - Class for float literals like 1.0, 0.22, 12.05.
class FloatASTnode : public ExprASTnode {
    double Val;
    TOKEN Tok;
    std::string Name;

public:
    FloatASTnode(TOKEN tok, double val) : Val(val), Tok(tok) {}

    virtual bool isEmpty() override {return false;};

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};

/// IntASTnode - Class for integer literals like 1, 2, 10.
class BoolASTnode : public ExprASTnode {
    bool Val;
    TOKEN Tok;
    std::string Name;

public:
    BoolASTnode(TOKEN tok, bool val) : Val(val), Tok(tok) {}

    virtual bool isEmpty() override {return false;};

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// ident
class IdentASTnode : public ExprASTnode {
public:
    TOKEN Token;

public:
    IdentASTnode(TOKEN token) : Token(token) {}

    virtual bool isEmpty() override {return false;};

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// function calls
class Ident_funASTnode : public IdentASTnode {
    std::vector<ExprASTnode*> Arguments;
    bool Hasargs = false;

public:
    Ident_funASTnode(IdentASTnode* ident) :
    IdentASTnode(ident->Token) {}

    Ident_funASTnode(IdentASTnode* ident, std::vector<ExprASTnode*> arguments) :
    IdentASTnode(ident->Token), Arguments((arguments)), Hasargs(true) {}

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// type
class TypeASTnode : public ASTnode {
public:
    TOKEN Token;
    TypeASTnode(TOKEN token) : Token(token) {}

//    virtual llvm::Value *codegen() override {return nullptr;};
//
//    virtual std::string to_string(std::string prefix, bool last) const override;
};

// fun_type
class Fun_TypeASTnode : public TypeASTnode {
public:
    Fun_TypeASTnode(TOKEN token) : TypeASTnode(token) {}

    virtual llvm::Value *codegen() override {return nullptr;};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// var_type
class Var_TypeASTnode : public TypeASTnode {
public:
    Var_TypeASTnode(TOKEN token) : TypeASTnode(token) {}

    virtual llvm::Value *codegen() override {return nullptr;};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// param
class ParamASTnode : public ASTnode {
public:
    TypeASTnode* Type;
    IdentASTnode* Ident;

    ParamASTnode(TypeASTnode* type, IdentASTnode* ident)
    : Type(type), Ident(ident) {}

    virtual llvm::Value *codegen() override {return nullptr;};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// params
class ParamsASTnode : public ASTnode {
public:
    std::vector<ParamASTnode*> Params;
    bool Empty; // or void
    bool Isvoid;

    ParamsASTnode(bool empy_or_void) : Empty(empy_or_void), Isvoid(!empy_or_void) {}

    ParamsASTnode(std::vector<ParamASTnode*> params) :
    Params((params)), Empty(false), Isvoid(false) {}

    virtual llvm::Value *codegen() override {return nullptr;}

    virtual llvm::Value *codegen(std::vector<llvm::Type *> *param_types);

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// externs
class ExternASTnode : public ASTnode {
    TypeASTnode* Type_;
    IdentASTnode* Ident;
    ParamsASTnode* Params;

public:
    ExternASTnode(TypeASTnode* type, IdentASTnode* ident, ParamsASTnode* params) :
    Type_((type)), Ident((ident)), Params((params)) {}

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// list of externs
class Extern_listASTnode : public ASTnode {
    std::vector<ExternASTnode*> Extern_list;

public:
    Extern_listASTnode(std::vector<ExternASTnode*> extern_list) : Extern_list((extern_list)) {}

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};

//-------------------------------------------------------------------------------------------

// stmt
class StmtASTnode : public ASTnode {
public:
    StmtASTnode() {}

    virtual llvm::Value *codegen() override {return nullptr;};

    virtual bool return_stmt() {
        return false;
    }

//    virtual std::string to_string(std::string prefix, bool last) const override;
};

// expr_stmt
class Expr_stmtASTnode : public StmtASTnode {
    ExprASTnode* Expr;
    bool Empty = false;
public:
    Expr_stmtASTnode() : Empty(true) {}

    Expr_stmtASTnode(ExprASTnode* expr) : Expr((expr)) {}

    virtual llvm::Value *codegen() override {return nullptr;};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// if_stmt
class If_stmtASTnode : public StmtASTnode {
    ExprASTnode* Expr;
    BlockASTnode* Ifblock;
    BlockASTnode* Elseblock;
    bool Haselse = false;
public:
    If_stmtASTnode(ExprASTnode* expr, BlockASTnode* ifblock) :
    Expr((expr)), Ifblock((ifblock)) {}

    If_stmtASTnode(ExprASTnode* expr, BlockASTnode* ifblock, BlockASTnode* elseblock) :
    Expr((expr)), Ifblock((ifblock)), Elseblock((elseblock)), Haselse(true) {}

    virtual llvm::Value *codegen() override {return nullptr;};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// while_stmt
class While_stmtASTnode : public StmtASTnode {
    ExprASTnode* Expr;
    StmtASTnode* Stmt;
public:
    While_stmtASTnode(ExprASTnode* expr, StmtASTnode* stmt) :
    Expr((expr)), Stmt((stmt)) {}

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// return_stmt
class Return_stmtASTnode : public StmtASTnode {
    ExprASTnode* Expr;
    bool Empty = false;
public:
    Return_stmtASTnode() : Empty(true) {}

    Return_stmtASTnode(ExprASTnode* expr) : Expr((expr)) {}

//  Used in IR generation
    bool return_stmt() override {
        return true;
    }

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// local_decl
class Local_declASTnode : public ASTnode {
    TypeASTnode* Type_;
    IdentASTnode* Ident;
public:
    Local_declASTnode(TypeASTnode* type, IdentASTnode* ident) :
    Type_((type)), Ident((ident)) {}

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// block
class BlockASTnode : public StmtASTnode {
    std::vector<Local_declASTnode*> Local_decl_list;
    std::vector<StmtASTnode*> Stmt_list;
public:
    bool Empty = false; // used in else_stmt
    BlockASTnode(std::vector<Local_declASTnode*> local_decl_list, std::vector<StmtASTnode*> stmt_list) :
            Local_decl_list((local_decl_list)), Stmt_list((stmt_list)) {}

    BlockASTnode() : Empty(true) {}

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;

    std::string decl_to_string(std::string prefix, bool last) const;

    std::string stmt_to_string(std::string prefix, bool last) const;
};

// decl
class DeclASTnode : public ASTnode {
public:
    DeclASTnode() {}

    virtual llvm::Value *codegen() override {return nullptr;};
//
//    virtual std::string to_string(std::string prefix, bool last) const override;
};

// fun_decl
class Fun_DeclASTnode : public DeclASTnode {
public:
    TypeASTnode* Type_;
    IdentASTnode* Ident;
    ParamsASTnode* Params;
    BlockASTnode* Block;

    Fun_DeclASTnode(TypeASTnode* type, IdentASTnode* ident, ParamsASTnode* params, BlockASTnode* block) :
    Type_((type)), Ident((ident)), Params((params)), Block((block)) {}

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// var_decl
class Var_DeclASTnode : public DeclASTnode {
public:
    TypeASTnode* Type;
    IdentASTnode* Ident;

    Var_DeclASTnode(TypeASTnode* type, IdentASTnode* ident) :
    Type((type)), Ident((ident)) {}

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};

//list of decls
class Decl_listASTnode : public ASTnode {
    std::vector<DeclASTnode*> Decl_list;

public:
    Decl_listASTnode(std::vector<DeclASTnode*> decl_list) : Decl_list((decl_list)) {}

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};


class RootASTnode : public ASTnode {
    Extern_listASTnode* Extern;
    Decl_listASTnode* Decl;
    bool Externs = true;

public:
    RootASTnode(Extern_listASTnode* _extern, Decl_listASTnode* decl) :
    Extern(_extern), Decl(decl) {}

    RootASTnode(Decl_listASTnode* decl) :
    Decl(decl), Externs(false) {}

    virtual llvm::Value *codegen() override;

    virtual std::string to_string(std::string prefix, bool last) const override;
};

#endif //CODE_AST_H
