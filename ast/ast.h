//
// Created by Atom Filippi on 22/10/2019.
//

#ifndef CODE_AST_H
#define CODE_AST_H

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

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// Half completed binary
class HalfASTnode : public ExprASTnode {
public:
    TOKEN Op;
    std::unique_ptr<ExprASTnode> Expr;
public:
    HalfASTnode(TOKEN op, std::unique_ptr<ExprASTnode> expr):
    Op(op), Expr(std::move(expr)) {}
};

// empty, used to signal it was an epsilon to the parser
class EmptyASTnode : public ExprASTnode {
public:
    EmptyASTnode() {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// binary operator
class BinaryASTnode : public ExprASTnode {
    TOKEN Op;
    std::unique_ptr<ExprASTnode> Left;
    std::unique_ptr<ExprASTnode> Right;
public:
    BinaryASTnode(std::unique_ptr<ExprASTnode> left, TOKEN op, std::unique_ptr<ExprASTnode> right)
    : Op(op), Left(std::move(left)), Right(std::move(right)) {}

    BinaryASTnode(std::unique_ptr<ExprASTnode> expr, std::unique_ptr<HalfASTnode> half) :
    BinaryASTnode(std::move(expr), half->Op, move(half->Expr)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// unary operator
class UnaryASTnode : public ExprASTnode {
    TOKEN Op;
    std::unique_ptr<ExprASTnode> Expr;
public:
    UnaryASTnode(TOKEN op, std::unique_ptr<ExprASTnode> expr)
            : Op(op), Expr(std::move(expr)) {}
};

/// IntASTnode - Class for integer literals like 1, 2, 10.
class IntASTnode : public ExprASTnode {
    int Val;
    TOKEN Tok;
    std::string Name;

public:
    IntASTnode(TOKEN tok, int val) : Val(val), Tok(tok) {}

    virtual llvm::Value *codegen() override;
    // virtual std::string to_string() const override {
    // return a sting representation of this AST node
    //};
};

/// FloatASTnode - Class for float literals like 1.0, 0.22, 12.05.
class FloatASTnode : public ExprASTnode {
    double Val;
    TOKEN Tok;
    std::string Name;

public:
    FloatASTnode(TOKEN tok, double val) : Val(val), Tok(tok) {}

    virtual llvm::Value *codegen() override;
    // virtual std::string to_string() const override {
    // return a sting representation of this AST node
    //};
};

/// IntASTnode - Class for integer literals like 1, 2, 10.
class BoolASTnode : public ExprASTnode {
    bool Val;
    TOKEN Tok;
    std::string Name;

public:
    BoolASTnode(TOKEN tok, bool val) : Val(val), Tok(tok) {}

    virtual llvm::Value *codegen() override;
    // virtual std::string to_string() const override {
    // return a sting representation of this AST node
    //};
};

// ident
class IdentASTnode : public ExprASTnode {
public:
    TOKEN Token;

public:
    IdentASTnode(TOKEN token) : Token(token) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// function calls
class Ident_funASTnode : public IdentASTnode {
    std::vector<std::unique_ptr<ExprASTnode>> Arguments;
    bool Isfun = false;

public:
    Ident_funASTnode(std::unique_ptr<IdentASTnode> ident, std::vector<std::unique_ptr<ExprASTnode>> arguments) :
    IdentASTnode(ident->Token), Arguments(std::move(arguments)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// type
class TypeASTnode : public ASTnode {
public:
    TypeASTnode() {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// fun_type
class Fun_TypeASTnode : public TypeASTnode {
    TOKEN Token;
public:
    Fun_TypeASTnode(TOKEN token) : Token(token) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// var_type
class Var_TypeASTnode : public TypeASTnode {
    TOKEN Token;
public:
    Var_TypeASTnode(TOKEN token) : Token(token) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// param
class ParamASTnode : public ASTnode {
    std::unique_ptr<TypeASTnode> Type;
    std::unique_ptr<IdentASTnode> Ident;

public:
    ParamASTnode(std::unique_ptr<TypeASTnode> type, std::unique_ptr<IdentASTnode> ident)
    : Type(move(type)), Ident(move(ident)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// params
class ParamsASTnode : public ASTnode {
    std::vector<std::unique_ptr<ParamASTnode>> Params;
    bool Empty; // or void
    bool Isvoid;

public:
    ParamsASTnode(bool empy_or_void) : Empty(empy_or_void), Isvoid(!empy_or_void) {}

    ParamsASTnode(std::vector<std::unique_ptr<ParamASTnode>> params) :
    Params(std::move(params)), Empty(false), Isvoid(false) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// externs
class ExternASTnode : public ASTnode {
    std::unique_ptr<TypeASTnode> Type;
    std::unique_ptr<IdentASTnode> Ident;
    std::unique_ptr<ParamsASTnode> Params;

public:
    ExternASTnode(std::unique_ptr<TypeASTnode> type, std::unique_ptr<IdentASTnode> ident, std::unique_ptr<ParamsASTnode> params) :
    Type(std::move(type)), Ident(std::move(ident)), Params(std::move(params)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// list of externs
class Extern_listASTnode : public ASTnode {
    std::vector<std::unique_ptr<ExternASTnode>> Extern_list;

public:
    Extern_listASTnode(std::vector<std::unique_ptr<ExternASTnode>> extern_list) : Extern_list(std::move(extern_list)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

//-------------------------------------------------------------------------------------------

// stmt
class StmtASTnode : public ASTnode {
public:
    StmtASTnode() {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// expr_stmt
class Expr_stmtASTnode : StmtASTnode {
    std::unique_ptr<ExprASTnode> Expr;
    bool Empty = false;
public:
    Expr_stmtASTnode() : Empty(true) {}

    Expr_stmtASTnode(std::unique_ptr<ExprASTnode> expr) : Expr(std::move(expr)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// if_stmt
class If_stmtASTnode : StmtASTnode {
    std::unique_ptr<ExprASTnode> Expr;
    std::unique_ptr<BlockASTnode> Ifblock;
    std::unique_ptr<BlockASTnode> Elseblock;
    bool Haselse = false;
public:
    If_stmtASTnode(std::unique_ptr<ExprASTnode> expr, std::unique_ptr<BlockASTnode> ifblock) :
    Expr(std::move(expr)), Ifblock(std::move(ifblock)) {}

    If_stmtASTnode(std::unique_ptr<ExprASTnode> expr, std::unique_ptr<BlockASTnode> ifblock, std::unique_ptr<BlockASTnode> elseblock) :
    Expr(std::move(expr)), Ifblock(std::move(ifblock)), Elseblock(std::move(elseblock)), Haselse(true) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// while_stmt
class While_stmtASTnode : StmtASTnode {
    std::unique_ptr<ExprASTnode> Expr;
    std::unique_ptr<StmtASTnode> Stmt;
public:
    While_stmtASTnode(std::unique_ptr<ExprASTnode> expr, std::unique_ptr<StmtASTnode> stmt) :
    Expr(std::move(expr)), Stmt(std::move(stmt)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// return_stmt
class Return_stmtASTnode : StmtASTnode {
    std::unique_ptr<ExprASTnode> Expr;
    bool Empty = false;
public:
    Return_stmtASTnode() : Empty(true) {}

    Return_stmtASTnode(std::unique_ptr<ExprASTnode> expr) : Expr(std::move(expr)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// local_decl
class Local_declASTnode : public ASTnode {
    std::unique_ptr<TypeASTnode> Type;
    std::unique_ptr<IdentASTnode> Ident;
public:
    Local_declASTnode(std::unique_ptr<TypeASTnode> type, std::unique_ptr<IdentASTnode> ident) :
    Type(std::move(type)), Ident(std::move(ident)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// block
class BlockASTnode : public StmtASTnode {
    std::vector<std::unique_ptr<Local_declASTnode>> Local_decl_list;
    std::vector<std::unique_ptr<StmtASTnode>> Stmt_list;
    bool Empty = false; // used in else_stmt

public:
    BlockASTnode(std::vector<std::unique_ptr<Local_declASTnode>> local_decl_list, std::vector<std::unique_ptr<StmtASTnode>> stmt_list) :
            Local_decl_list(std::move(local_decl_list)), Stmt_list(std::move(stmt_list)) {}

    BlockASTnode() : Empty(true) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;

    std::string decl_to_string(std::string prefix, bool last) const;

    std::string stmt_to_string(std::string prefix, bool last) const;
};

// decl
class DeclASTnode : public ASTnode {
public:
    DeclASTnode() {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// fun_decl
class Fun_DeclASTnode : public DeclASTnode {
    std::unique_ptr<TypeASTnode> Type;
    std::unique_ptr<IdentASTnode> Ident;
    std::unique_ptr<ParamsASTnode> Params;
    std::unique_ptr<BlockASTnode> Block;

public:
    Fun_DeclASTnode(std::unique_ptr<TypeASTnode> type, std::unique_ptr<IdentASTnode> ident, std::unique_ptr<ParamsASTnode> params, std::unique_ptr<BlockASTnode> block) :
    Type(std::move(type)), Ident(std::move(ident)), Params(std::move(params)), Block(std::move(block)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

// var_decl
class Var_DeclASTnode : public DeclASTnode {
    std::unique_ptr<TypeASTnode> Type;
    std::unique_ptr<IdentASTnode> Ident;
public:
    Var_DeclASTnode(std::unique_ptr<TypeASTnode> type, std::unique_ptr<IdentASTnode> ident) :
    Type(std::move(type)), Ident(std::move(ident)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

//list of decls
class Decl_listASTnode : public ASTnode {
    std::vector<std::unique_ptr<DeclASTnode>> Decl_list;

public:
    Decl_listASTnode(std::vector<std::unique_ptr<DeclASTnode>> decl_list) : Decl_list(std::move(decl_list)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string(std::string prefix, bool last) const override;
};

#endif //CODE_AST_H
