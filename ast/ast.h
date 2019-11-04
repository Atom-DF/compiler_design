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

    virtual std::string to_string() const = 0;
};

/// IntASTnode - Class for integer literals like 1, 2, 10.
class IntASTnode : public ASTnode {
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
class FloatASTnode : public ASTnode {
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
class BoolASTnode : public ASTnode {
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

/// IdentASTnode - Class for the type of a function.
class IdentASTnode : public ASTnode {
    TOKEN Tok;
    std::string Name;

public:
    IdentASTnode(TOKEN tok, std::string name) : Name(name), Tok(tok) {}

    virtual llvm::Value *codegen() override {}

    virtual std::string to_string() const override;
};

/// TypeASTnode - Class for the type of a function or variable.
class TypeASTnode : public ASTnode {
    TOKEN Tok;
    std::string Type;

public:
    TypeASTnode(TOKEN tok, std::string type) : Type(type), Tok(tok) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string() const override;
};

/// ParamASTnode - Class for a parameter.
class ParamASTnode : public ASTnode {
    std::unique_ptr<TypeASTnode> Type;
    std::unique_ptr<IdentASTnode> Ident;
public:
    ParamASTnode(std::unique_ptr<TypeASTnode> type, std::unique_ptr<IdentASTnode> ident) : Type(std::move(type)), Ident(std::move(ident)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string() const override;
};

/// ParamsASTnode - Class which contains a std::vector of ParamASTnode.
class ParamsASTnode : public ASTnode {
    bool Empty;
    bool Void;
    std::vector<std::unique_ptr<ParamASTnode>> Params;
public:
    ParamsASTnode( bool empty, bool _void, std::vector<std::unique_ptr<ParamASTnode>> params) : Empty(std::move(empty)), Void(std::move(_void)), Params(std::move(params)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string() const override;
};

/// Fun_PrototypeASTnode - Class which contains the prototype of a function.
class Fun_PrototypeASTnode : public ASTnode {
    std::unique_ptr<TypeASTnode> Type;
    std::unique_ptr<IdentASTnode> Ident;
    std::unique_ptr<ParamsASTnode> Params;

public:
    Fun_PrototypeASTnode(std::unique_ptr<TypeASTnode> type, std::unique_ptr<IdentASTnode> ident, std::unique_ptr<ParamsASTnode> params) : Type(std::move(type)), Ident(std::move(ident)), Params(std::move(params)) {}

    virtual llvm::Value *codegen() override {}

    virtual std::string to_string() const override;
};

/// Fun_PrototypesASTnode - Class which contains a std::vector of Fun_PrototypeASTnode.
class Fun_PrototypesASTnode : public ASTnode {
    std::vector<std::unique_ptr<Fun_PrototypeASTnode>> Externs;

public:
    Fun_PrototypesASTnode(std::vector<std::unique_ptr<Fun_PrototypeASTnode>> externs) : Externs(std::move(externs)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string() const override;
};

/// Local_DeclASTnode - Class which contains a std::vector of Fun_PrototypeASTnode.
class DeclASTnode : public ASTnode {
    std::unique_ptr<TypeASTnode> Type;
    std::unique_ptr<IdentASTnode> Ident;

public:
    DeclASTnode(std::unique_ptr<TypeASTnode> type, std::unique_ptr<IdentASTnode> ident) : Type(std::move(type)), Ident(std::move(ident)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string() const override;
};

/// Local_DeclsASTnode - Class which contains a std::vector of Fun_PrototypeASTnode.
class Local_DeclsASTnode : public ASTnode {
    std::vector<std::unique_ptr<DeclASTnode>> Local_decls;
    bool Empty;

public:
    Local_DeclsASTnode(std::vector<std::unique_ptr<DeclASTnode>> local_decls) : Local_decls(std::move(local_decls)), Empty(false) {}

    Local_DeclsASTnode() : Empty(true) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string() const override;
};

/// BlockASTnode - Class which contains a std::vector of Fun_PrototypeASTnode.
class BlockASTnode : public ASTnode {
    std::unique_ptr<Local_DeclsASTnode> Local_decl;

public:
    BlockASTnode(std::unique_ptr<Local_DeclsASTnode> local_decl) : Local_decl(std::move(local_decl)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string() const override;
};

class DeclarationASTnode : public ASTnode {
public:
    DeclarationASTnode() {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string() const override;
};

/// Fun_DeclASTnode - Class which contains a std::vector of Fun_PrototypeASTnode.
class Fun_DeclASTnode : public DeclarationASTnode {
    std::unique_ptr<Fun_PrototypeASTnode> Prototype;
    std::unique_ptr<BlockASTnode> Block;

public:
    Fun_DeclASTnode(std::unique_ptr<Fun_PrototypeASTnode> prototype, std::unique_ptr<BlockASTnode> block) : Prototype(std::move(prototype)), Block(std::move(block)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string() const override;
};

class DeclarationsASTnode : public ASTnode {
    std::vector<std::unique_ptr<DeclarationASTnode>> Declarations;

public:
    DeclarationsASTnode(std::vector<std::unique_ptr<DeclarationASTnode>> declarations) : Declarations(std::move(declarations)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string() const override;
};

class Var_DeclASTnode : public DeclarationASTnode {
    std::unique_ptr<TypeASTnode> Type;
    std::unique_ptr<IdentASTnode> Ident;
public:
    Var_DeclASTnode(std::unique_ptr<TypeASTnode> type, std::unique_ptr<IdentASTnode> ident) : Type(std::move(type)), Ident(std::move(ident)) {}

    virtual llvm::Value *codegen() override {};

    virtual std::string to_string() const override;
};

#endif //CODE_AST_H
