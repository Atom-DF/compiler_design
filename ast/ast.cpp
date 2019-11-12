//
// Created by Atom Filippi on 22/10/2019.
//

//===----------------------------------------------------------------------===//
// AST nodes
//===----------------------------------------------------------------------===//
#include "ast.h"

using namespace std;

string IdentASTnode::to_string(string prefix, bool last) const {
    string temp;
    temp += prefix;
    temp += (last ? "└──" : "├──" );
    temp += "Ident: ";
    temp += Token.lexeme + "\n";
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
    temp += "Decl_List\n";

//    if (Local_decl_list.size() == 1) {
//        temp += Local_decl_list.at(Local_decl_list.size() - 1)->to_string(prefix + (last ? "    " : "│   "), true);
//        return temp;
//    }
//
//    for (int i = 0; i <= Local_decl_list.size() - 2; ++i) {
//        temp += Local_decl_list.at(i)->to_string(prefix + (last ? "    " : "│   "), false);
//    }
//
//    temp += Local_decl_list.at(Local_decl_list.size() - 1)->to_string(prefix + (last ? "    " : "│   "), true);

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
