/*llvm-3.9.0.src/lib/Transforms/Hello/Hello.cpp*/
//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/PassRegistry.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/LegacyPassManager.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Type.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
using namespace llvm;

#define DEBUG_TYPE "hello"

STATISTIC(HelloCounter, "Counts number of functions greeted");

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct Hello : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    Hello() : FunctionPass(ID) {}

    Function *insert_func = NULL;

    bool runOnFunction(Function &F) override {
      errs().write_escaped(F.getName()) << '\n';

     if(insert_func == NULL && strcmp("invoke", F.getName().begin()) == 0){
	insert_func = &F;
     }

     for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
  	//errs() << *I << "\n";
	if(strcmp("call", I->getOpcodeName()) == 0){
		Function *f = cast<CallInst>(*I).getCalledFunction();
		if(f == NULL){
			//errs() << "------------------\n";
			continue;
		}else
			//errs() << "*********" << f->getName() << '\n';
		if(strcmp("multi", f->getName().begin()) == 0){
		//if(0){
			//AllocaInst* ai = new AllocaInst(Type::getInt32Ty(F.getContext()));
			//Value *Args[] = {ConstantInt::get(Type::getInt32Ty(F.getContext()), 1)};


			//Value *Args[] = {ConstantDataArray::getString(F.getContext(), f->getName())};


			 //ArrayType* ArrayTy_0 = ArrayType::get(IntegerType::get(F.getContext(), 8), 14);
			 //Type *Int8Ty = Type::getInt8Ty(F.getContext());		
			 //IntegerType *Int8Ty = IntegerType::get(F.getContext(), 8);
			 Constant *const_array_9 = ConstantDataArray::getString(F.getContext(), "hahaha", true);
				
			 GlobalVariable* gvar_array__str = new GlobalVariable(/*Module=*/*F.getParent(), 
			 /*Type=*/const_array_9->getType(),
			 /*isConstant=*/true,
			 /*Linkage=*/GlobalValue::PrivateLinkage,
			 /*Initializer=*/0, // has initializer, specified below
			 /*Name=*/".str");
			 gvar_array__str->setAlignment(1);

			 //Constant *const_array_9 = ConstantDataArray::getString(F.getContext(), "a parameter", true);
			 ConstantInt* const_int32_12 = ConstantInt::get(F.getContext(), APInt(32, StringRef("0"), 10));

			 std::vector<Constant*> const_ptr_14_indices;
			 const_ptr_14_indices.push_back(const_int32_12);
			 const_ptr_14_indices.push_back(const_int32_12);
			 Constant* const_ptr_14 = ConstantExpr::getGetElementPtr(NULL, gvar_array__str, const_ptr_14_indices);

			 gvar_array__str->setInitializer(const_array_9);
				
			 std::vector<Value*> int32_call1_params;
			 int32_call1_params.push_back(const_ptr_14);

			CallInst* ci = CallInst::Create(insert_func, int32_call1_params);
			Instruction *pi = &cast<Instruction>(*I);
			//pi->getParent()->getInstList().insert(pi, ai);

			std::vector<llvm::Type *> AsmArgTypes;
 	  	        std::vector<llvm::Value *> AsmArgs;
                       
			AsmArgTypes.push_back(llvm::Type::getInt32Ty(F.getContext()));
			AsmArgTypes.push_back(llvm::Type::getInt64Ty(F.getContext()));
			

			AsmArgs.push_back(llvm::ConstantInt::get(Type::getInt32Ty(F.getContext()), 316));
			AsmArgs.push_back(llvm::ConstantInt::get(Type::getInt64Ty(F.getContext()), 0x56465f0));
	
		    llvm::FunctionType *AsmFTy = llvm::FunctionType::get(Type::getInt32Ty(F.getContext()), AsmArgTypes, false);
		    llvm::InlineAsm *IA = llvm::InlineAsm::get(AsmFTy, "syscall", "={ax},0,{di},~{dx}~{dirflag},~{fpsr},~{flags}", true, false, llvm::InlineAsm::AD_ATT);
		    CallInst *asmi = CallInst::Create(IA, AsmArgs);
			ci->insertAfter(pi);
			asmi->insertAfter(pi);
		}
	}
      }

	//BasicBlock *pb = I->getParent();
	
      return false;
    }

  };
}


char Hello::ID = 0;
static RegisterPass<Hello> X("hello", "Hello World Pass");


static void loadPass(const PassManagerBuilder &Builder, legacy::PassManagerBase &PM) {
  PM.add(new Hello());
}
// These constructors add our pass to a list of global extensions.
static RegisterStandardPasses helloLoader_Ox(PassManagerBuilder::EP_OptimizerLast, loadPass);
static RegisterStandardPasses helloLoader_O0(PassManagerBuilder::EP_EnabledOnOptLevel0, loadPass);

namespace {
  // Hello2 - The second implementation with getAnalysisUsage implemented.
  struct Hello2 : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    Hello2() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      ++HelloCounter;
      errs() << "Hello: ";
      errs().write_escaped(F.getName()) << '\n';
      return false;
    }

    // We don't modify the program, so we preserve all analyses.
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }
  };
}

char Hello2::ID = 0;
static RegisterPass<Hello2>
Y("hello2", "Hello World Pass (with getAnalysisUsage implemented)");
