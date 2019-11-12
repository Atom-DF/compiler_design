#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <string.h>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "ast/ast.h"
#include "parser/parser.h"
#include "mccomp.h"
#include "exceptions/exceptions.h"

using namespace std;
using namespace llvm;
using namespace llvm::sys;

FILE *pFile;

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

// Definition of TOKEN_TYPE moved to .h file.

// Definition of TOKEN moved to .h file.
void TOKEN::print_as_string() {
    cout << "type: " + to_string(type) + "\n";
    cout << "lexeme: " << lexeme.c_str() << "\n";
    cout << "(line, column): " << "(" + to_string(lineNo) + ", " + to_string(columnNo) + ")\n\n";
}

std::string IdentifierStr; // Filled in if IDENT
int IntVal;                // Filled in if INT_LIT
bool BoolVal;              // Filled in if BOOL_LIT
float FloatVal;            // Filled in if FLOAT_LIT
std::string StringVal;     // Filled in if String Literal
int lineNo, columnNo;

static TOKEN returnTok(string lexVal, int tok_type) {
    TOKEN return_tok;
    return_tok.lexeme = lexVal;
    return_tok.type = tok_type;
    return_tok.lineNo = lineNo;
    return_tok.columnNo = columnNo - lexVal.length() - 1;
    return return_tok;
}

// Read file line by line -- or look for \n and if found add 1 to line number
// and reset column number to 0
/// gettok - Return the next token from standard input.
static TOKEN gettok() {

    static int LastChar = ' ';
    static int NextChar = ' ';

    // Skip any whitespace.
    while (isspace(LastChar)) {
        if (LastChar == '\n' || LastChar == '\r') {
            lineNo++;
            columnNo = 1;
        }
        LastChar = getc(pFile);
        columnNo++;
    }

    if (isalpha(LastChar) ||
        (LastChar == '_')) { // identifier: [a-zA-Z_][a-zA-Z_0-9]*
        IdentifierStr = LastChar;
        columnNo++;

        while (isalnum((LastChar = getc(pFile))) || (LastChar == '_')) {
            IdentifierStr += LastChar;
            columnNo++;
        }

        if (IdentifierStr == "int")
            return returnTok("int", INT_TOK);
        if (IdentifierStr == "bool")
            return returnTok("bool", BOOL_TOK);
        if (IdentifierStr == "float")
            return returnTok("float", FLOAT_TOK);
        if (IdentifierStr == "void")
            return returnTok("void", VOID_TOK);
        if (IdentifierStr == "bool")
            return returnTok("bool", BOOL_TOK);
        if (IdentifierStr == "extern")
            return returnTok("extern", EXTERN);
        if (IdentifierStr == "if")
            return returnTok("if", IF);
        if (IdentifierStr == "else")
            return returnTok("else", ELSE);
        if (IdentifierStr == "while")
            return returnTok("while", WHILE);
        if (IdentifierStr == "return")
            return returnTok("return", RETURN);
        if (IdentifierStr == "true") {
            BoolVal = true;
            return returnTok("true", BOOL_LIT);
        }
        if (IdentifierStr == "false") {
            BoolVal = false;
            return returnTok("false", BOOL_LIT);
        }

        return returnTok(IdentifierStr.c_str(), IDENT);
    }

    if (LastChar == '=') {
        NextChar = getc(pFile);
        if (NextChar == '=') { // EQ: ==
            LastChar = getc(pFile);
            columnNo += 2;
            return returnTok("==", EQ);
        } else {
            LastChar = NextChar;
            columnNo++;
            return returnTok("=", ASSIGN);
        }
    }

    if (LastChar == '{') {
        LastChar = getc(pFile);
        columnNo++;
        return returnTok("{", LBRA);
    }
    if (LastChar == '}') {
        LastChar = getc(pFile);
        columnNo++;
        return returnTok("}", RBRA);
    }
    if (LastChar == '(') {
        LastChar = getc(pFile);
        columnNo++;
        return returnTok("(", LPAR);
    }
    if (LastChar == ')') {
        LastChar = getc(pFile);
        columnNo++;
        return returnTok(")", RPAR);
    }
    if (LastChar == ';') {
        LastChar = getc(pFile);
        columnNo++;
        return returnTok(";", SC);
    }
    if (LastChar == ',') {
        LastChar = getc(pFile);
        columnNo++;
        return returnTok(",", COMMA);
    }

    if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9]+.
        string NumStr;

        if (LastChar == '.') { // Floatingpoint Number: .[0-9]+
            do {
                NumStr += LastChar;
                LastChar = getc(pFile);
                columnNo++;
            } while (isdigit(LastChar));

            FloatVal = strtof(NumStr.c_str(), nullptr);
            return returnTok(NumStr, FLOAT_LIT);
        } else {
            do { // Start of Number: [0-9]+
                NumStr += LastChar;
                LastChar = getc(pFile);
                columnNo++;
            } while (isdigit(LastChar));

            if (LastChar == '.') { // Floatingpoint Number: [0-9]+.[0-9]+)
                do {
                    NumStr += LastChar;
                    LastChar = getc(pFile);
                    columnNo++;
                } while (isdigit(LastChar));

                FloatVal = strtof(NumStr.c_str(), nullptr);
                return returnTok(NumStr, FLOAT_LIT);
            } else { // Integer : [0-9]+
                IntVal = strtod(NumStr.c_str(), nullptr);
                return returnTok(NumStr, INT_LIT);
            }
        }
    }

    if (LastChar == '&') {
        NextChar = getc(pFile);
        if (NextChar == '&') { // AND: &&
            LastChar = getc(pFile);
            columnNo += 2;
            return returnTok("&&", AND);
        } else {
            LastChar = NextChar;
            columnNo++;
            return returnTok("&", int('&'));
        }
    }

    if (LastChar == '|') {
        NextChar = getc(pFile);
        if (NextChar == '|') { // OR: ||
            LastChar = getc(pFile);
            columnNo += 2;
            return returnTok("||", OR);
        } else {
            LastChar = NextChar;
            columnNo++;
            return returnTok("|", int('|'));
        }
    }

    if (LastChar == '!') {
        NextChar = getc(pFile);
        if (NextChar == '=') { // NE: !=
            LastChar = getc(pFile);
            columnNo += 2;
            return returnTok("!=", NE);
        } else {
            LastChar = NextChar;
            columnNo++;
            return returnTok("!", NOT);;
        }
    }

    if (LastChar == '<') {
        NextChar = getc(pFile);
        if (NextChar == '=') { // LE: <=
            LastChar = getc(pFile);
            columnNo += 2;
            return returnTok("<=", LE);
        } else {
            LastChar = NextChar;
            columnNo++;
            return returnTok("<", LT);
        }
    }

    if (LastChar == '>') {
        NextChar = getc(pFile);
        if (NextChar == '=') { // GE: >=
            LastChar = getc(pFile);
            columnNo += 2;
            return returnTok(">=", GE);
        } else {
            LastChar = NextChar;
            columnNo++;
            return returnTok(">", GT);
        }
    }

    if (LastChar == '/') { // could be division or could be the start of a comment
        LastChar = getc(pFile);
        columnNo++;
        if (LastChar == '/') { // definitely a comment
            do {
                LastChar = getc(pFile);
                columnNo++;
            } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

            if (LastChar != EOF)
                return gettok();
        } else
            return returnTok("/", DIV);
    }

    // Check for end of file.  Don't eat the EOF.
    if (LastChar == EOF) {
        columnNo++;
        return returnTok("0", EOF_TOK);
    }

    // Otherwise, just return the character as its ascii value.
    int ThisChar = LastChar;
    string s(1, ThisChar);
    LastChar = getc(pFile);
    columnNo++;
    return returnTok(s, int(ThisChar));
}

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

// CurTok and tok_buffer moved to .h file.

TOKEN CurTok;
std::deque <TOKEN> tok_buffer;

TOKEN getNextToken() {
    if (tok_buffer.size() == 0)
        tok_buffer.push_back(gettok());

    TOKEN temp = tok_buffer.front();
    tok_buffer.pop_front();

    return CurTok = temp;
}

void putBackToken(TOKEN tok) {
    tok_buffer.push_front(CurTok);
    CurTok = tok;
}

// The AST nodes were moved into their own .cpp/.h files.

//===----------------------------------------------------------------------===//
// Code Generation
//===----------------------------------------------------------------------===//

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static unique_ptr <Module> TheModule;

//===----------------------------------------------------------------------===//
// AST Printer
//===----------------------------------------------------------------------===//

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                     const ASTnode &ast) {
    os << ast.to_string("", true);
    return os;
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main(int argc, char **argv) {

    if (argc == 2) {
        pFile = fopen(argv[1], "r");
        if (pFile == NULL)
            perror("Error opening file");
    } else {
        cout << "Usage: ./code InputFile\n";
        return 1;
    }

    // initialize line number and column numbers to zero
    lineNo = 1;
    columnNo = 1;

    // get the first token
//  getNextToken();
//  while (CurTok.type != EOF_TOK) {
//    fprintf(stderr, "Token: %s with type %d\n", CurTok.lexeme.c_str(),
//            CurTok.type);
//    getNextToken();
//  }
//  fprintf(stderr, "Lexer Finished\n");

    // Make the module, which holds all the code.
    TheModule = llvm::make_unique<Module>("mini-c", TheContext);

    // Run the parser now.
    parser();
    fprintf(stderr, "Parsing Finished\n");

    //********************* Start printing final IR **************************
    // Print out all of the generated code into a file called output.ll
    auto Filename = "output.ll";
    error_code EC;
    raw_fd_ostream dest(Filename, EC, sys::fs::F_None);

    if (EC) {
        errs() << "Could not open file: " << EC.message();
        return 1;
    }
    // TheModule->print(errs(), nullptr); // print IR to terminal
    TheModule->print(dest, nullptr);
    //********************* End printing final IR ****************************

    fclose(pFile); // close the file that contains the code that was parsed
    return 0;
}
