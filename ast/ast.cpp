//
// Created by Atom Filippi on 22/10/2019.
//

//===----------------------------------------------------------------------===//
// AST nodes
//===----------------------------------------------------------------------===//
#include "ast.h"

using namespace std;

string IdentASTnode::to_string() const {
    return Name;
}

string TypeASTnode::to_string() const {
    return Type;
}

string ParamASTnode::to_string() const {
    return Type->to_string() + " " + Ident->to_string();
}

string ParamsASTnode::to_string() const {
    if (Empty)
        return string();
    if (Void)
        return string("void");
    string total;
    for (int i = 0; i != Params.size(); i++)
        total += Params[i]->to_string() + ", ";
    if (total.size () > 2)
        total.resize (total.size () - 2);
    return total;
}

string Fun_PrototypeASTnode::to_string() const {
    return "extern " + Type->to_string() + " " + Ident->to_string() + " (" + Params->to_string() + ");";
}

string Fun_PrototypesASTnode::to_string() const {
    string total;
    for (int i = 0; i != Externs.size(); i++)
        total += Externs[i]->to_string() + "\n";
    return total;
}

string DeclASTnode::to_string() const {
    return Type->to_string() + " " + Ident->to_string() + ";";
}

string Local_DeclsASTnode::to_string() const {
    if (Empty) return string();
    string total;
    for (int i = 0; i != Local_decls.size(); i++)
        total += Local_decls[i]->to_string() + "\n";
    return total;
}

string BlockASTnode::to_string() const {
    return Local_decl->to_string();
}

string Fun_DeclASTnode::to_string() const {
    return Prototype->to_string() + "\n" + Block->to_string();
}

string DeclarationsASTnode::to_string() const {
    string total;
    for (int i = 0; i != Declarations.size(); i++)
        total += Declarations[i]->to_string() + "\n";
    return total;
}

string Var_DeclASTnode::to_string() const {
    return Type->to_string() + " " + Ident->to_string() + ";";
}
