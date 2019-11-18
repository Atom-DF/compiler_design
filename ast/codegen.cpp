//
// Created by Atom Filippi on 16/11/2019.
//

#include "../mccomp.h"
#include "ast.h"
#include <stack>
#include "llvm/ADT/ArrayRef.h"
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

using namespace std;
using namespace llvm;
using namespace llvm::sys;

Type *Get_Type(TOKEN Tok){
    if(Tok.type == INT_TOK){
        return Type::getInt32Ty(TheContext);
    }
    else if(Tok.type == BOOL_TOK){
        return Type::getInt1Ty(TheContext);
    }
    else if(Tok.type == FLOAT_TOK){
        return Type::getFloatTy(TheContext);
    }
    else{
        return Type::getVoidTy(TheContext);
    }
}

Constant *Default_Val(TOKEN varType){
    if(varType.type == INT_TOK){
        return ConstantInt::get(TheContext, APInt(sizeof(int)*8,0));
    }
    else if(varType.type == BOOL_TOK){
        return ConstantInt::get(TheContext, APInt(1,0));
    }
    else{
        return ConstantFP::get(TheContext, APFloat(0.0f));
    }
}

AllocaInst *CreateEntryBlockAlloca(Function *function, const std::string &lexeme, Type *type) {
    IRBuilder<> TmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
    return TmpB.CreateAlloca(type, 0, lexeme.c_str());
}

Value *RootASTnode::codegen(){
    Extern->codegen();
    Decl->codegen();
    return nullptr;
}

Value *IntASTnode::codegen() {
    return ConstantInt::get(TheContext, APInt(sizeof(int)*8,Val));
}

Value *FloatASTnode::codegen(){
    return ConstantFP::get(TheContext, APFloat(Val));
}

Value *BoolASTnode::codegen(){
    if(Val){
        int value = 1;
        return ConstantInt::get(TheContext,APInt(1,value));
    }
    else{
        int value = 0;
        return ConstantInt::get(TheContext, APInt(1,value));
    }
}

Value *IdentASTnode::codegen() {
    Value *V = Curr_scope[Token.lexeme];

    if(!V)
        LogErrorSyntax("Unknow variable name.", Token.lexeme);
    return Builder.CreateLoad(V,Token.lexeme);

}

Value *Ident_funASTnode::codegen() {
    Function *function_name = TheModule->getFunction(Token.lexeme);

    if(!function_name)
        LogErrorSyntax("Function is not declared: ", Token.lexeme);

    if(function_name->arg_size() != Arguments.size())
        LogErrorSyntax("Incorrect number of arguments passed for function call:", Token.lexeme);

    vector<Value*> ArgsV;
    for (unsigned i = 0, e = Arguments.size(); i != e; ++i)
        ArgsV.push_back(Arguments[i]->codegen());

    return Builder.CreateCall(function_name, ArgsV, "calltmp");
}

Value *Extern_listASTnode::codegen() {
    for (auto ex : Extern_list)
        ex->codegen();
    return nullptr;
}

Value *ExternASTnode::codegen() {
    auto *param_types = new vector<Type *>();
    Params->codegen(param_types);
    FunctionType *function_type;
    switch (Type_->Token.type) {
        case VOID_TOK:
            function_type = FunctionType::get(Type::getVoidTy(TheContext), *param_types, false);
            break;
        case INT_TOK:
            function_type = FunctionType::get(Type::getInt32Ty(TheContext), *param_types, false);
            break;
        case FLOAT_TOK:
            function_type = FunctionType::get(Type::getFloatTy(TheContext), *param_types, false);
            break;
        default:
            function_type = FunctionType::get(Type::getInt1Ty(TheContext), *param_types, false);
    }

    Function *function = Function::Create(function_type, Function::ExternalLinkage, Ident->Token.lexeme, TheModule.get());

    int index = 0;
    for (auto &arg : function->args())
        arg.setName(Params->Params[index++]->Ident->Token.lexeme);
    return nullptr;
}

Value *ParamsASTnode::codegen(std::vector<llvm::Type *> *param_types) {
     for (auto param : Params)
         param_types->push_back(Get_Type(param->Type->Token));
     return nullptr;
}

Value *Decl_listASTnode::codegen() {
    for (auto decl : Decl_list)
        decl->codegen();
    return nullptr;
}

Value *Var_DeclASTnode::codegen() {
    GlobalVariable *global_var;
    global_var = new GlobalVariable(*TheModule, Get_Type(Type->Token), false, GlobalValue::CommonLinkage,0, Ident->Token.lexeme);

    global_var->setAlignment(4);

    global_var->setInitializer(Default_Val(Type->Token));

    global[Ident->Token.lexeme] = global_var;

    return nullptr;
}

Value *Fun_DeclASTnode::codegen() {
    auto param_types = new vector<Type * >();
    Params->codegen(param_types);
    FunctionType *function_type;
    switch (Type_->Token.type) {
        case VOID_TOK:
            function_type = FunctionType::get(Type::getVoidTy(TheContext), *param_types, false);
            break;
        case INT_TOK:
            function_type = FunctionType::get(Type::getInt32Ty(TheContext), *param_types, false);
            break;
        case FLOAT_TOK:
            function_type = FunctionType::get(Type::getFloatTy(TheContext), *param_types, false);
            break;
        default:
            function_type = FunctionType::get(Type::getInt1Ty(TheContext), *param_types, false);
    }

    Function *function = Function::Create(function_type, Function::ExternalLinkage, Ident->Token.lexeme, TheModule.get());

    int index = 0;
    for (auto &arg : function->args())
        arg.setName(Params->Params[index++]->Ident->Token.lexeme);

    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", function);
    Builder.SetInsertPoint(BB);
    Curr_scope.clear();

    for (auto &arg : function->args()){
        AllocaInst *Alloc = CreateEntryBlockAlloca(function, arg.getName(), Get_Type(Type_->Token));
        Builder.CreateStore(&arg, Alloc);
        Curr_scope[arg.getName()] = Alloc;
    }

    if (Value *block = Block->codegen()){
        Builder.CreateRet(block);
        llvm::verifyFunction(*function);
        return function;
    }

    function->eraseFromParent();
    return nullptr;

}

Value *BlockASTnode::codegen() {
    for (auto decl : Local_decl_list)
        decl->codegen();

    for (auto stmt : Stmt_list) {
        auto temp = stmt->codegen();
        if (stmt->return_stmt())
            return temp;
    }

    return ConstantFP::get(TheContext, APFloat(0.0f));
}

Value *Local_declASTnode::codegen() {
    Function *function = Builder.GetInsertBlock()->getParent();
    Value *default_value = Default_Val(Type_->Token);
    if(!default_value)
        LogErrorSyntax("IR error, could not generate default value for type: ", Type_->Token.lexeme);
    Type *type = Get_Type(Type_->Token);


    AllocaInst *alloca = CreateEntryBlockAlloca(function, Ident->Token.lexeme, type);
    Builder.CreateStore(default_value, alloca);

    Curr_scope[Ident->Token.lexeme] = alloca;

    return nullptr;
}

Value *Return_stmtASTnode::codegen() {
    Value *ReturnVal = Expr->codegen();
    if(!ReturnVal) {
        cout << "this is null omega lol" << "\n";
        return nullptr;
    }
    cout << string(ReturnVal->getName()) << "\n";
    return ReturnVal;
}

Value *While_stmtASTnode::codegen() {
    Value *cond = Expr->codegen();

    if(!cond)
        return nullptr;

    cond = Builder.CreateFCmpONE(cond, ConstantFP::get(TheContext, APFloat(0.0f)),"whilecond");

    Function *function = Builder.GetInsertBlock()->getParent();

    BasicBlock *LoopHeader = BasicBlock::Create(TheContext, "header",function);
    BasicBlock *LoopBody = BasicBlock::Create(TheContext, "loopbody");
    BasicBlock *LoopEnd = BasicBlock::Create(TheContext,"loopend");

    Builder.CreateCondBr(cond, LoopBody, LoopEnd);

    Builder.SetInsertPoint(LoopHeader);

    return nullptr;
}

Value *UnaryASTnode::codegen() {
    Value *expr = Expr->codegen();
    if (!expr){
        return nullptr;
    }
    switch(Op.type){
        case NOT:
            return Builder.CreateNot(expr,"nottmp");
        case MINUS:
            return Builder.CreateFNeg(expr, "negtmp");
        default:
            LogErrorSyntax("Invalid operator: ", Op.lexeme);
    }
    return nullptr;
}

Value *BinaryASTnode::codegen() {
    Value *left_expr = Left->codegen();
    Value *right_expr = Right->codegen();
    if (!left_expr | !right_expr){
        return nullptr;
    }
    switch (Op.type){
        case PLUS:
            return Builder.CreateFAdd(left_expr,right_expr,"addtmp");
        case MINUS:
            return Builder.CreateFSub(left_expr,right_expr,"subtmp");
        case ASTERIX:
            return Builder.CreateFMul(left_expr,right_expr,"multmp");
        case LT:
            left_expr = Builder.CreateFCmpULT(left_expr,right_expr,"cmptmp");
            return Builder.CreateUIToFP(left_expr, Type::getFloatTy(TheContext), "booltmp");
        case LE:
            left_expr = Builder.CreateFCmpULE(left_expr,right_expr,"cmptmp");
            return Builder.CreateUIToFP(left_expr, Type::getFloatTy(TheContext), "booltmp");
        case GT:
            left_expr = Builder.CreateFCmpUGT(left_expr,right_expr,"cmptmp");
            return Builder.CreateUIToFP(left_expr, Type::getFloatTy(TheContext), "booltmp");
        case GE:
            left_expr = Builder.CreateFCmpUGE(left_expr,right_expr,"cmptmp");
            return Builder.CreateUIToFP(left_expr, Type::getFloatTy(TheContext), "booltmp");
        case EQ:
            left_expr = Builder.CreateFCmpUEQ(left_expr,right_expr,"cmptmp");
            return Builder.CreateUIToFP(left_expr, Type::getFloatTy(TheContext), "booltmp");
        case NE:
            left_expr = Builder.CreateFCmpUNE(left_expr,right_expr,"cmptmp");
            return Builder.CreateUIToFP(left_expr, Type::getFloatTy(TheContext), "booltmp");
        case AND:
            return Builder.CreateAnd(left_expr,right_expr,"andtmp");
        case OR:
            return Builder.CreateOr(left_expr,right_expr,"ortmp");
        case ASSIGN: {
            Value *Val = Right->codegen();
            if (!Val)
                return nullptr;
            auto ident = static_cast<IdentASTnode *>(Left);
            Value *Variable = Curr_scope[ident->Token.lexeme];

            if (!Variable)
                LogErrorSyntax("Error variable not declared: ", ident->Token.lexeme);

            Builder.CreateStore(Val, Variable);
            return Val;
        }
        default:
            LogErrorSyntax("Error could not find operator: ", Op.lexeme);
    }
    return nullptr;
}

