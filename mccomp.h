//
// Created by Atom Filippi on 22/10/2019.
//

#ifndef CODE_MCCOMP_H
#define CODE_MCCOMP_H


#include <queue>
#include <stack>
#include <iostream>
#include "llvm/IR/IRBuilder.h"

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

// The lexer returns one of these for known things.
enum TOKEN_TYPE {

    IDENT = -1,        // [a-zA-Z_][a-zA-Z_0-9]*
    ASSIGN = int('='), // '='

    // delimiters
    LBRA = int('{'),  // left brace
    RBRA = int('}'),  // right brace
    LPAR = int('('),  // left parenthesis
    RPAR = int(')'),  // right parenthesis
    SC = int(';'),    // semicolon
    COMMA = int(','), // comma

    // types
    INT_TOK = -2,   // "int"
    VOID_TOK = -3,  // "void"
    FLOAT_TOK = -4, // "float"
    BOOL_TOK = -5,  // "bool"

    // keywords
    EXTERN = -6,  // "extern"
    IF = -7,      // "if"
    ELSE = -8,    // "else"
    WHILE = -9,   // "while"
    RETURN = -10, // "return"
    // TRUE   = -12,     // "true"
    // FALSE   = -13,     // "false"

    // literals
    INT_LIT = -14,   // [0-9]+
    FLOAT_LIT = -15, // [0-9]+.[0-9]+
    BOOL_LIT = -16,  // "true" or "false" key words

    // logical operators
    AND = -17, // "&&"
    OR = -18,  // "||"

    // operators
    PLUS = int('+'),    // addition or unary plus
    MINUS = int('-'),   // substraction or unary negative
    ASTERIX = int('*'), // multiplication
    DIV = int('/'),     // division
    MOD = int('%'),     // modular
    NOT = int('!'),     // unary negation

    // comparison operators
    EQ = -19,      // equal
    NE = -20,      // not equal
    LE = -21,      // less than or equal to
    LT = int('<'), // less than
    GE = -23,      // greater than or equal to
    GT = int('>'), // greater than

    // special tokens
    EOF_TOK = 0, // signal end of file

    // invalid
    INVALID = -100 // signal invalid token
};

struct TOKEN {
    int type = -100;
    std::string lexeme;
    int lineNo;
    int columnNo;

public:
    void print_as_string();
};

extern std::string IdentifierStr; // Filled in if IDENT
extern int IntVal;                // Filled in if INT_LIT
extern bool BoolVal;              // Filled in if BOOL_LIT
extern float FloatVal;            // Filled in if FLOAT_LIT
extern std::string StringVal;     // Filled in if String Literal
extern int lineNo, columnNo;


//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.
extern TOKEN CurTok;
extern std::deque <TOKEN> tok_buffer;

extern void LogError(const char* message);

extern void LogErrorSyntax(const char* message, std::string lexeme);

extern void putBackToken(TOKEN tok);

TOKEN getNextToken();

// IR generation code.

extern llvm::LLVMContext TheContext;
extern llvm::IRBuilder<> Builder;
extern std::unique_ptr <llvm::Module> TheModule;
extern std::map<std::string, llvm::Value*> global;
extern std::map<std::string, llvm::AllocaInst*> Curr_scope;

#endif //CODE_MCCOMP_H
