//
// Created by Atom Filippi on 22/10/2019.
//

//===----------------------------------------------------------------------===//
// AST nodes
//===----------------------------------------------------------------------===//
#include "ast.h"

using namespace std;

string HalfASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "HalfASTnode\n";
    return temp;
}

string ExprASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Ident: ";
    temp += "ExprASTnode\n";
    return temp;
}

string EmptyASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "EmptyASTnode\n";
    return temp;
}

string BinaryASTnode::to_string(string prefix, bool last) const {
    string temp = "";
    cout << prefix;
    cout << (last ? "└──" : "├──" );
    cout << "BinaryASTnode:" + Op.lexeme;
    cout << "\n";
    cout << Left->to_string(prefix + (last ? "    " : "│   "), false);
    cout << Right->to_string(prefix + (last ? "    " : "│   "), true);
    return temp;
//    string temp;
//    temp += prefix;
//    temp += (last ? "└──" : "├──" );
//    temp += "BinaryASTnode:" + Op.lexeme;
//    temp += "\n";
//    temp += Left->to_string(prefix + (last ? "    " : "│   "), false);
//    temp += Right->to_string(prefix + (last ? "    " : "│   "), true);
//    return temp;
}

string UnaryASTnode::to_string(string prefix, bool last) const {
    string temp = "";
    cout << prefix;
    cout << (last ? "└──" : "├──" );
    cout << "UnaryASTnode:" + Op.lexeme;
    cout << "\n";
    cout << Expr->to_string(prefix + (last ? "    " : "│   "), true);
    return temp;
//    string temp;
//    temp += prefix;
//    temp += (last ? "└──" : "├──" );
//    temp += "UnaryASTnode:" + Op.lexeme;
//    temp += "\n";
//    temp += Expr->to_string(prefix + (last ? "    " : "│   "), true);
//    return temp;
}

string IntASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Int_LIT: ";
    temp += Tok.lexeme + "\n";
    return temp;
}

string FloatASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Float_LIT: ";
    temp += Tok.lexeme + "\n";
    return temp;
}

string BoolASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Bool_lit: ";
    temp += Tok.lexeme + "\n";
    return temp;
}

string IdentASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Ident: ";
    temp += Token.lexeme + "\n";
    return temp;
}

string Ident_funASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Ident: ";
    temp += Token.lexeme + "\n";
    if (!Hasargs)
        return temp;
    if (Arguments.size() == 1) {
        temp += Arguments.at(Arguments.size() - 1)->to_string(prefix + (last ? "    " : "│   "), true);
        return temp;
    }

    for (int i = 0; i <= Arguments.size() - 2; ++i) {
        temp += Arguments.at(i)->to_string(prefix + (last ? "    " : "│   "), false);
    }

    temp += Arguments.at(Arguments.size() - 1)->to_string(prefix + (last ? "    " : "│   "), true);
    return temp;
}

string Var_TypeASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Var_type: ";
    temp += Token.lexeme + "\n";
    return temp;
}

string Fun_TypeASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Fun_type: ";
    temp += Token.lexeme + "\n";
    return temp;
}

string ParamASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Param\n";
    temp += Type->to_string(prefix + (last ? "    " : "│   "), false);
    temp += Ident->to_string(prefix + (last ? "    " : "│   "), true);
    return temp;
}

string ParamsASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Params\n";

    if (Empty) {
        temp += prefix + (last ? "    " : "│   ");
        temp += "└──Empty\n";
        return temp;
    }

    if (Isvoid) {
        temp += prefix + (last ? "    " : "│   ");
        temp += "└──Void\n";
        return temp;
    }

    if (Params.size() == 1) {
        temp += Params.at(Params.size() - 1)->to_string(prefix + (last ? "    " : "│   "), true);
        return temp;
    }
    for (int i = 0; i <= Params.size() - 2; ++i) {
        temp += Params.at(i)->to_string(prefix + (last ? "    " : "│   "), false);
    }

    temp += Params.at(Params.size() - 1)->to_string(prefix + (last ? "    " : "│   "), true);

    return temp;
}

string ExternASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Extern\n";
    temp += Type->to_string(prefix + (last ? "    " : "│   "), false);
    temp += Ident->to_string(prefix + (last ? "    " : "│   "), false);
    temp += Params->to_string(prefix + (last ? "    " : "│   "), true);
    return temp;
}

string Extern_listASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Extern_List\n";

    if (Extern_list.empty())
        return temp;

    if (Extern_list.size() == 1) {
        temp += Extern_list.at(Extern_list.size() - 1)->to_string(prefix + (last ? "    " : "│   "), true);
        return temp;
    }

    for (int i = 0; i <= Extern_list.size() - 2; ++i) {
        temp += Extern_list.at(i)->to_string(prefix + (last ? "    " : "│   "), false);
    }

    temp += Extern_list.at(Extern_list.size() - 1)->to_string(prefix + (last ? "    " : "│   "), true);

    return temp;
}

string Expr_stmtASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Expr_stmt\n";
    if (Empty)
        return temp;
    temp += Expr->to_string(prefix + (last ? "    " : "│   "), true);
    return temp;
}

string If_stmtASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "If_stmt\n";
    temp += Expr->to_string(prefix + (last ? "    " : "│   "), false);
    if (!Haselse) {
        temp += Ifblock->to_string(prefix + (last ? "    " : "│   "), true);
        return temp;
    }
    temp += Ifblock->to_string(prefix + (last ? "    " : "│   "), false);
    temp += Elseblock->to_string(prefix + (last ? "    " : "│   "), true);
    return temp;
}

string While_stmtASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "While_stmt\n";
    temp += Expr->to_string(prefix + (last ? "    " : "│   "), false);
    temp += Stmt->to_string(prefix + (last ? "    " : "│   "), true);
    return temp;
}

string Return_stmtASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Return_stmt\n";
    if (Empty)
        return temp;
    temp += Expr->to_string(prefix + (last ? "    " : "│   "), true);
    return temp;
}

string Local_declASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Local_decl\n";
    temp += Type->to_string(prefix + (last ? "    " : "│   "), false);
    temp += Ident->to_string(prefix + (last ? "    " : "│   "), true);
    return temp;
}

string BlockASTnode::decl_to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Decl_List\n";

    if (Local_decl_list.empty())
        return temp;

    if (Local_decl_list.size() == 1) {
        temp += Local_decl_list.at(Local_decl_list.size() - 1)->to_string(prefix + (last ? "    " : "│   "), true);
        return temp;
    }

    for (int i = 0; i <= Local_decl_list.size() - 2; ++i) {
        temp += Local_decl_list.at(i)->to_string(prefix + (last ? "    " : "│   "), false);
    }

    temp += Local_decl_list.at(Local_decl_list.size() - 1)->to_string(prefix + (last ? "    " : "│   "), true);

    return temp;
}

string BlockASTnode::stmt_to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Stmt_List\n";

    if (Stmt_list.empty())
        return temp;

    if (Stmt_list.size() == 1) {
        temp += Stmt_list.at(Stmt_list.size() - 1)->to_string(prefix + (last ? "    " : "│   "), true);
        return temp;
    }

    for (int i = 0; i <= Stmt_list.size() - 2; ++i) {
        temp += Stmt_list.at(i)->to_string(prefix + (last ? "    " : "│   "), false);
    }

    temp += Stmt_list.at(Stmt_list.size() - 1)->to_string(prefix + (last ? "    " : "│   "), true);

    return temp;
}

string BlockASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Block\n";

    temp += decl_to_string(prefix + (last ? "    " : "│   "), false);
    temp += stmt_to_string(prefix + (last ? "    " : "│   "), true);

    return temp;
}

string Fun_DeclASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Fun_Decl\n";
    temp += Type->to_string(prefix + (last ? "    " : "│   "), false);
    temp += Ident->to_string(prefix + (last ? "    " : "│   "), false);
    temp += Params->to_string(prefix + (last ? "    " : "│   "), false);
    temp += Block->to_string(prefix + (last ? "    " : "│   "), true);
    return temp;
}

string Var_DeclASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Var_Decl\n";
    temp += Type->to_string(prefix + (last ? "    " : "│   "), false);
    temp += Ident->to_string(prefix + (last ? "    " : "│   "), true);
    return temp;
}

string Decl_listASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Decl_list\n";

    if (Decl_list.empty())
        return temp;

    if (Decl_list.size() == 1) {
        temp += Decl_list.at(Decl_list.size() - 1)->to_string(prefix + (last ? "    " : "│   "), true);
        return temp;
    }

    for (int i = 0; i <= Decl_list.size() - 2; ++i) {
        temp += Decl_list.at(i)->to_string(prefix + (last ? "    " : "│   "), false);
    }

    temp += Decl_list.at(Decl_list.size() - 1)->to_string(prefix + (last ? "    " : "│   "), true);

    return temp;
}
