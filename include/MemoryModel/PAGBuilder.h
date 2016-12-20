//===- PAGBuilder.h -- Building PAG-------------------------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

/*
 * PAGBuilder.h
 *
 *  Created on: Nov 1, 2013
 *      Author: Yulei Sui
 */

#ifndef PAGBUILDER_H_
#define PAGBUILDER_H_

#include "MemoryModel/PAG.h"
#include "Util/ExtAPI.h"

#include <llvm/IR/InstVisitor.h>	// for instruction visitor


/*!
 *  PAG Builder
 */
class PAGBuilder: public llvm::InstVisitor<PAGBuilder> {
private:
    PAG* pag;
public:
    /// Constructor
    PAGBuilder() :
        pag(PAG::getPAG()) {
    }
    /// Destructor
    virtual ~PAGBuilder() {
    }

    /// Start building PAG here
    PAG* build(llvm::Module& module);

    /// Return PAG
    PAG* getPAG() const {
        return pag;
    }

    /// Initialize nodes and edges
    //@{
    void initalNode();
    void addEdge(NodeID src, NodeID dst, PAGEdge::PEDGEK kind,
                 Size_t offset = 0, llvm::Instruction* cs = NULL);
    // @}

    /// Sanity check for PAG
    void sanityCheck();

    /// Get different kinds of node
    //@{
    // GetValNode - Return the value node according to a LLVM Value.
    NodeID getValueNode(const llvm::Value *V) {
        // first handle gep edge if val if a constant expression
        processCE(V);

        // strip off the constant cast and return the value node
        return pag->getValueNode(V);
    }

    /// GetObject - Return the object node (stack/global/heap/function) according to a LLVM Value
    inline NodeID getObjectNode(const llvm::Value *V) {
        return pag->getObjectNode(V);
    }

    /// getReturnNode - Return the node representing the unique return value of a function.
    inline NodeID getReturnNode(const llvm::Function *func) {
        return pag->getReturnNode(func);
    }

    /// getVarargNode - Return the node representing the unique variadic argument of a function.
    inline NodeID getVarargNode(const llvm::Function *func) {
        return pag->getVarargNode(func);
    }
    //@}

    /// Handle globals including (global variable and functions)
    //@{
    void visitGlobal(llvm::Module& module);
    void InitialGlobal(const llvm::GlobalVariable *gvar, llvm::Constant *C,
                       Size_t offset);
    NodeID getGlobalVarField(const llvm::GlobalVariable *gvar, Size_t offset);
    //@}

    /// Process constant expression
    void processCE(const llvm::Value *val);

    /// Compute offset of a gep instruction or gep constant expression
    bool computeGepOffset(const llvm::User *V, LocationSet& ls);

    /// Handle direct call
    void handleDirectCall(llvm::CallSite cs, const llvm::Function *F);

    /// Handle indirect call
    void handleIndCall(llvm::CallSite cs);

    /// Handle external call
    //@{
    virtual void handleExtCall(llvm::CallSite cs, const llvm::Function *F);
    std::vector<LocationSet> getFlattenedFields(llvm::Value* v);
    void addComplexConsForExt(llvm::Value *D, llvm::Value *S,u32_t sz = 0);
    //@}

    /// Our visit overrides.
    //@{
    // Instructions that cannot be folded away.
    virtual void visitAllocaInst(llvm::AllocaInst &AI);
    void visitPHINode(llvm::PHINode &I);
    void visitStoreInst(llvm::StoreInst &I);
    void visitLoadInst(llvm::LoadInst &I);
    void visitGetElementPtrInst(llvm::GetElementPtrInst &I);
    void visitCallInst(llvm::CallInst &I) {
        visitCallSite(&I);
    }
    void visitInvokeInst(llvm::InvokeInst &II) {
        visitCallSite(&II);
        visitTerminatorInst(II);
    }
    void visitCallSite(llvm::CallSite cs);
    void visitReturnInst(llvm::ReturnInst &I);
    void visitCastInst(llvm::CastInst &I);
    void visitSelectInst(llvm::SelectInst &I);
    void visitIntToPtrInst(llvm::IntToPtrInst &inst);
    void visitExtractValueInst(llvm::ExtractValueInst &EVI);
    void visitInsertValueInst(llvm::InsertValueInst &IVI) {
    }
    // Terminators
    void visitTerminatorInst(llvm::TerminatorInst &TI) {
    }
    void visitBinaryOperator(llvm::BinaryOperator &I) {
    }
    void visitCmpInst(llvm::CmpInst &I) {
    }

    /// TODO: do we need to care about these corner cases?
    void visitPtrToIntInst(llvm::PtrToIntInst &inst) {
    }
    void visitVAArgInst(llvm::VAArgInst &I) {
    }
    void visitExtractElementInst(llvm::ExtractElementInst &I);

    void visitInsertElementInst(llvm::InsertElementInst &I) {
    }
    void visitShuffleVectorInst(llvm::ShuffleVectorInst &I) {
    }
    void visitLandingPadInst(llvm::LandingPadInst &I) {
    }

    /// Instruction not that often
    void visitResumeInst(llvm::TerminatorInst &I) { /*returns void*/
    }
    void visitUnwindInst(llvm::TerminatorInst &I) { /*returns void*/
    }
    void visitUnreachableInst(llvm::TerminatorInst &I) { /*returns void*/
    }
    void visitFenceInst(llvm::FenceInst &I) { /*returns void*/
    }
    void visitAtomicCmpXchgInst(llvm::AtomicCmpXchgInst &I) {
    }
    void visitAtomicRMWInst(llvm::AtomicRMWInst &I) {
    }

    /// Provide base case for our instruction visit.
    inline void visitInstruction(llvm::Instruction &I) {
        // If a new instruction is added to LLVM that we don't handle.
        // TODO: ignore here:
    }
    //}@
};

/*!
 * Build PAG from a user specified file (for debugging purpose)
 */
class PAGBuilderFromFile {

private:
    PAG* pag;
    std::string file;
public:
    /// Constructor
    PAGBuilderFromFile(std::string f) :
        pag(PAG::getPAG(true)), file(f) {
    }
    /// Destructor
    ~PAGBuilderFromFile() {
    }

    /// Return PAG
    PAG* getPAG() const {
        return pag;
    }

    /// Return file name
    std::string getFileName() const {
        return file;
    }

    /// Start building
    PAG* build();

    // Add edges
    void addEdge(NodeID nodeSrc, NodeID nodeDst, Size_t offset,
                 std::string edge);
};

#endif /* PAGBUILDER_H_ */
