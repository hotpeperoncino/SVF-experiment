//===- ThreadAPI.h -- API for threads-----------------------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

/*
 * ThreadAPI.h
 *
 *  Created on: Jan 21, 2014
 *      Author: Yulei Sui
 */

#ifndef THREADAPI_H_
#define THREADAPI_H_

#include "Util/BasicTypes.h"
#include <llvm/ADT/StringMap.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/CallSite.h>

/*
 * ThreadAPI class contains interfaces for pthread programs
 */
class ThreadAPI {

public:
    enum TD_TYPE {
        TD_DUMMY = 0,		/// dummy type
        TD_FORK,         /// create a new thread
        TD_JOIN,         /// wait for a thread to join
        TD_DETACH,       /// detach a thread directly instead wait for it to join
        TD_ACQUIRE,      /// acquire a lock
        TD_TRY_ACQUIRE,  /// try to acquire a lock
        TD_RELEASE,      /// release a lock
        TD_EXIT,		   /// exit/kill a thread
        TD_CANCEL,	   /// cancel a thread by another
        TD_COND_WAIT,    /// wait a condition
        TD_COND_SIGNAL,    /// signal a condition
        TD_COND_BROADCAST,    /// broadcast a condition
        TD_MUTEX_INI,	     /// initial a mutex variable
        TD_MUTEX_DESTROY,   /// initial a mutex variable
        TD_CONDVAR_INI,	   /// initial a mutex variable
        TD_CONDVAR_DESTROY /// initial a mutex variable
    };

    typedef llvm::StringMap<TD_TYPE> TDAPIMap;

private:
    /// API map, from a string to threadAPI type
    TDAPIMap tdAPIMap;

    /// Constructor
    ThreadAPI () {
        init();
    }

    /// Initialize the map
    void init();

    /// Static reference
    static ThreadAPI* tdAPI;

    /// Get the function type if it is a threadAPI function
    inline TD_TYPE getType(const llvm::Function* F) const {
        if(F) {
            TDAPIMap::const_iterator it= tdAPIMap.find(F->getName().str());
            if(it != tdAPIMap.end())
                return it->second;
        }
        return TD_DUMMY;
    }

public:
    /// Return a static reference
    static ThreadAPI* getThreadAPI() {
        if(tdAPI == NULL) {
            tdAPI = new ThreadAPI();
        }
        return tdAPI;
    }

    /// Return the callee/callsite/func
    //@{
    const llvm::Function* getCallee(const llvm::Instruction *inst) const;
    const llvm::Function* getCallee(const llvm::CallSite cs) const;
    const llvm::CallSite getLLVMCallSite(const llvm::Instruction *inst) const;
    //@}

    /// Return true if this call create a new thread
    //@{
    inline bool isTDFork(const llvm::Instruction *inst) const {
        return getType(getCallee(inst)) == TD_FORK;
    }

    inline bool isTDFork(llvm::CallSite cs) const {
        return isTDFork(cs.getInstruction());
    }
    //@}

    /// Return arguments/attributes of pthread_create
    //@{
    /// Return the first argument of the call,
    /// Note that, it is the pthread_t pointer
    inline const llvm::Value* getForkedThread(const llvm::Instruction *inst) const {
        assert(isTDFork(inst) && "not a thread fork function!");
        llvm::CallSite cs = getLLVMCallSite(inst);
        return cs.getArgument(0);
    }
    inline const llvm::Value* getForkedThread(llvm::CallSite cs) const {
        return getForkedThread(cs.getInstruction());
    }

    /// Return the third argument of the call,
    /// Note that, it could be function type or a void* pointer
    inline const llvm::Value* getForkedFun(const llvm::Instruction *inst) const {
        assert(isTDFork(inst) && "not a thread fork function!");
        llvm::CallSite cs = getLLVMCallSite(inst);
        return cs.getArgument(2)->stripPointerCasts();
    }
    inline const llvm::Value* getForkedFun(llvm::CallSite cs) const {
        return getForkedFun(cs.getInstruction());
    }

    /// Return the forth argument of the call,
    /// Note that, it is the sole argument of start routine ( a void* pointer )
    inline const llvm::Value* getActualParmAtForkSite(const llvm::Instruction *inst) const {
        assert(isTDFork(inst) && "not a thread fork function!");
        llvm::CallSite cs = getLLVMCallSite(inst);
        return cs.getArgument(3);
    }
    inline const llvm::Value* getActualParmAtForkSite(llvm::CallSite cs) const {
        return getForkedFun(cs.getInstruction());
    }
    //@}


    /// Return true if this call wait for a worker thread
    //@{
    inline bool isTDJoin(const llvm::Instruction *inst) const {
        return getType(getCallee(inst)) == TD_JOIN;
    }

    inline bool isTDJoin(llvm::CallSite cs) const {
        return isTDJoin(cs.getInstruction());
    }
    //@}

    /// Return arguments/attributes of pthread_join
    //@{
    /// Return the first argument of the call,
    /// Note that, it is the pthread_t pointer
    inline const llvm::Value* getJoinedThread(const llvm::Instruction *inst) const {
        assert(isTDJoin(inst) && "not a thread join function!");
        llvm::CallSite cs = getLLVMCallSite(inst);
        llvm::Value* join = cs.getArgument(0);
        if(llvm::isa<llvm::LoadInst>(join))
            return llvm::cast<llvm::LoadInst>(join)->getPointerOperand();
        else if(llvm::isa<llvm::Argument>(join))
            return join;
        assert(false && "the value of the first argument at join is not a load instruction?");
        return NULL;
    }
    inline const llvm::Value* getJoinedThread(llvm::CallSite cs) const {
        return getJoinedThread(cs.getInstruction());
    }
    /// Return the send argument of the call,
    /// Note that, it is the pthread_t pointer
    inline const llvm::Value* getRetParmAtJoinedSite(const llvm::Instruction *inst) const {
        assert(isTDJoin(inst) && "not a thread join function!");
        llvm::CallSite cs = getLLVMCallSite(inst);
        return cs.getArgument(1);
    }
    inline const llvm::Value* getRetParmAtJoinedSite(llvm::CallSite cs) const {
        return getRetParmAtJoinedSite(cs.getInstruction());
    }
    //@}


    /// Return true if this call exits/terminate a thread
    //@{
    inline bool isTDExit(const llvm::Instruction *inst) const {
        return getType(getCallee(inst)) == TD_EXIT;
    }

    inline bool isTDExit(llvm::CallSite cs) const {
        return getType(getCallee(cs)) == TD_EXIT;
    }
    //@}

    /// Return true if this call acquire a lock
    //@{
    inline bool isTDAcquire(const llvm::Instruction *inst) const {
        return getType(getCallee(inst)) == TD_ACQUIRE;
    }

    inline bool isTDAcquire(llvm::CallSite cs) const {
        return getType(getCallee(cs)) == TD_ACQUIRE;
    }
    //@}

    /// Return true if this call release a lock
    //@{
    inline bool isTDRelease(const llvm::Instruction *inst) const {
        return getType(getCallee(inst)) == TD_RELEASE;
    }

    inline bool isTDRelease(llvm::CallSite cs) const {
        return getType(getCallee(cs)) == TD_RELEASE;
    }
    //@}

    /// Return lock value
    //@{
    /// First argument of pthread_mutex_lock/pthread_mutex_unlock
    inline const llvm::Value* getLockVal(const llvm::Instruction *inst) const {
        assert((isTDAcquire(inst) || isTDRelease(inst)) && "not a lock acquire or release function");
        llvm::CallSite cs = getLLVMCallSite(inst);
        return cs.getArgument(0);
    }
    inline const llvm::Value* getLockVal(llvm::CallSite cs) const {
        return getLockVal(cs.getInstruction());
    }
    //@}

    void performAPIStat(llvm::Module* m);
    void statInit(llvm::StringMap<u32_t>& tdAPIStatMap);
};

#endif /* THREADAPI_H_ */
