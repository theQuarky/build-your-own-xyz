#include "codegen/llvm/llvm_code_gen.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <iostream>

namespace codegen {

LLVMCodeGen::LLVMCodeGen(core::ErrorReporter &errorReporter,
                         const std::string &moduleName)
    : errorReporter_(errorReporter), context_(moduleName),
      typeBuilder_(context_), optimizer_(context_) {}

bool LLVMCodeGen::generateCode(const parser::AST &ast) {
  try {
    // For now, just create a simple main function that returns 0
    auto &module = context_.getModule();
    auto &builder = context_.getBuilder();

    // Create the main function
    llvm::FunctionType *mainType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(context_.getContext()), // Return type (int)
        false                                          // No parameters
    );

    llvm::Function *mainFunc = llvm::Function::Create(
        mainType, llvm::Function::ExternalLinkage, "main", module);

    // Create the entry block
    llvm::BasicBlock *entry =
        llvm::BasicBlock::Create(context_.getContext(), "entry", mainFunc);
    builder.SetInsertPoint(entry);

    // Return 0
    builder.CreateRet(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_.getContext()),
                               0,   // Return value = 0
                               true // Signed
                               ));

    // Verify the function
    if (llvm::verifyFunction(*mainFunc, &llvm::errs())) {
      error(core::SourceLocation(), "Function verification failed");
      return false;
    }

    // Apply optimizations if requested
    optimizer_.optimizeAll();

    return true;
  } catch (const std::exception &e) {
    error(core::SourceLocation(),
          std::string("Code generation failed: ") + e.what());
    return false;
  }
}

void LLVMCodeGen::optimize(OptimizationLevel level) {
  optimizer_.setOptimizationLevel(level);
  optimizer_.optimizeAll();
}

bool LLVMCodeGen::writeToFile(const std::string &filename) const {
  return context_.writeModuleToFile(filename);
}

bool LLVMCodeGen::executeCode() {
  try {
    // Initialize LLVM targets
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    // Create execution engine
    std::string errorStr;
    llvm::ExecutionEngine *executionEngine =
        llvm::EngineBuilder()
            .setErrorStr(&errorStr)
            .create();

    if (!executionEngine) {
      error(core::SourceLocation(),
            "Failed to create execution engine: " + errorStr);
      return false;
    }

    // Find the main function
    llvm::Function *mainFunc = executionEngine->FindFunctionNamed("main");
    if (!mainFunc) {
      error(core::SourceLocation(), "No main function found for execution");
      return false;
    }

    // Prepare arguments (none for main in this case)
    std::vector<llvm::GenericValue> args;

    // Execute the function
    llvm::GenericValue result = executionEngine->runFunction(mainFunc, args);

    // Print the return value
    std::cout << "Program executed, returned: " << result.IntVal.getSExtValue()
              << std::endl;

    // Clean up
    delete executionEngine;
    return true;
  } catch (const std::exception &e) {
    error(core::SourceLocation(),
          std::string("Error during execution: ") + e.what());
    return false;
  }
}

void LLVMCodeGen::error(const core::SourceLocation &location,
                        const std::string &message) {
  errorReporter_.error(location, message);
}

void LLVMCodeGen::warning(const core::SourceLocation &location,
                          const std::string &message) {
  errorReporter_.warning(location, message);
}

} // namespace codegen