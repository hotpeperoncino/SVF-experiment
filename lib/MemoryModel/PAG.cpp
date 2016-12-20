//===- PAG.cpp -- Program assignment graph------------------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

/*
 * PAG.cpp
 *
 *  Created on: Nov 1, 2013
 *      Author: Yulei Sui
 */

#include "MemoryModel/PAG.h"
#include "Util/AnalysisUtil.h"
#include "Util/GraphUtil.h"

#include <llvm/Support/DOTGraphTraits.h>	// for dot graph traits

using namespace llvm;
using namespace analysisUtil;

static cl::opt<bool> HANDLEVGEP("vgep", cl::init(false),
                                cl::desc("Hanle variant gep/field edge"));

static cl::opt<bool> HANDBLACKHOLE("blk", cl::init(false),
                                   cl::desc("Hanle blackhole edge"));

PAG* PAG::pag = NULL;


/*!
 * Add Address edge
 */
bool PAG::addAddrEdge(NodeID src, NodeID dst) {
    PAGNode* srcNode = getPAGNode(src);
    PAGNode* dstNode = getPAGNode(dst);
    if(hasIntraEdge(srcNode,dstNode, PAGEdge::Addr))
        return false;
    else
        return addEdge(srcNode,dstNode, new AddrPE(srcNode, dstNode));
}

/*!
 * Add Copy edge
 */
bool PAG::addCopyEdge(NodeID src, NodeID dst) {
    PAGNode* srcNode = getPAGNode(src);
    PAGNode* dstNode = getPAGNode(dst);
    if(hasIntraEdge(srcNode,dstNode, PAGEdge::Copy))
        return false;
    else
        return addEdge(srcNode,dstNode, new CopyPE(srcNode, dstNode));
}

/*!
 * Add Load edge
 */
bool PAG::addLoadEdge(NodeID src, NodeID dst) {
    PAGNode* srcNode = getPAGNode(src);
    PAGNode* dstNode = getPAGNode(dst);
    if(hasIntraEdge(srcNode,dstNode, PAGEdge::Load))
        return false;
    else
        return addEdge(srcNode,dstNode, new LoadPE(srcNode, dstNode));
}

/*!
 * Add Store edge
 */
bool PAG::addStoreEdge(NodeID src, NodeID dst) {
    PAGNode* srcNode = getPAGNode(src);
    PAGNode* dstNode = getPAGNode(dst);
    if(hasIntraEdge(srcNode,dstNode, PAGEdge::Store))
        return false;
    else
        return addEdge(srcNode,dstNode, new StorePE(srcNode, dstNode));
}

/*!
 * Add Call edge
 */
bool PAG::addCallEdge(NodeID src, NodeID dst, const llvm::Instruction* cs) {
    PAGNode* srcNode = getPAGNode(src);
    PAGNode* dstNode = getPAGNode(dst);
    if(hasInterEdge(srcNode,dstNode, PAGEdge::Call, cs))
        return false;
    else
        return addEdge(srcNode,dstNode, new CallPE(srcNode, dstNode, cs));
}

/*!
 * Add Return edge
 */
bool PAG::addRetEdge(NodeID src, NodeID dst, const llvm::Instruction* cs) {
    PAGNode* srcNode = getPAGNode(src);
    PAGNode* dstNode = getPAGNode(dst);
    if(hasInterEdge(srcNode,dstNode, PAGEdge::Ret, cs))
        return false;
    else
        return addEdge(srcNode,dstNode, new RetPE(srcNode, dstNode, cs));
}

/*!
 * Add blackhole/constant edge
 */
bool PAG::addBlackHoleAddrEdge(NodeID node) {
    if(HANDBLACKHOLE)
        return pag->addAddrEdge(pag->getBlackHoleNode(), node);
    else
        return pag->addCopyEdge(pag->getNullPtr(), node);
}

/*!
 * Add Thread fork edge for parameter passing from a spawner to its spawnees
 */
bool PAG::addThreadForkEdge(NodeID src, NodeID dst, const llvm::Instruction* cs) {
    PAGNode* srcNode = getPAGNode(src);
    PAGNode* dstNode = getPAGNode(dst);
    if(hasInterEdge(srcNode,dstNode, PAGEdge::ThreadFork, cs))
        return false;
    else
        return addEdge(srcNode,dstNode, new TDForkPE(srcNode, dstNode, cs));
}

/*!
 * Add Thread fork edge for parameter passing from a spawnee back to its spawners
 */
bool PAG::addThreadJoinEdge(NodeID src, NodeID dst, const llvm::Instruction* cs) {
    PAGNode* srcNode = getPAGNode(src);
    PAGNode* dstNode = getPAGNode(dst);
    if(hasInterEdge(srcNode,dstNode, PAGEdge::ThreadJoin, cs))
        return false;
    else
        return addEdge(srcNode,dstNode, new TDJoinPE(srcNode, dstNode, cs));
}


/*!
 * Add Offset(Gep) edge
 * Find the base node id of src and connect base node to dst node
 * Create gep offset:  (offset + baseOff <nested struct gep size>)
 */
bool PAG::addGepEdge(NodeID src, NodeID dst, const LocationSet& ls) {

    PAGNode* node = getPAGNode(src);
    if (node->hasIncomingVariantGepEdge()) {
        /// Since the offset from base to src is variant,
        /// the new gep edge being created is also a VariantGepPE edge.
        return addVariantGepEdge(src, dst);
    }
    else {
        return addNormalGepEdge(src, dst, ls);
    }
}

/*!
 * Add normal (Gep) edge
 */
bool PAG::addNormalGepEdge(NodeID src, NodeID dst, const LocationSet& ls) {
    const LocationSet& baseLS = getLocationSetFromBaseNode(src);
    PAGNode* baseNode = getPAGNode(getBaseValNode(src));
    PAGNode* dstNode = getPAGNode(dst);
    if(hasIntraEdge(baseNode, dstNode, PAGEdge::NormalGep))
        return false;
    else
        return addEdge(baseNode, dstNode, new NormalGepPE(baseNode, dstNode, ls+baseLS));
}

/*!
 * Add variant(Gep) edge
 * Find the base node id of src and connect base node to dst node
 */
bool PAG::addVariantGepEdge(NodeID src, NodeID dst) {
    if (HANDLEVGEP == false)
        return addCopyEdge(src,dst);

    PAGNode* baseNode = getPAGNode(getBaseValNode(src));
    PAGNode* dstNode = getPAGNode(dst);
    if(hasIntraEdge(baseNode, dstNode, PAGEdge::VariantGep))
        return false;
    else
        return addEdge(baseNode, dstNode, new VariantGepPE(baseNode, dstNode));
}

/*!
 * Add global Gep edge
 */
bool PAG::addGlobalGepEdge(NodeID src, NodeID dst, const LocationSet& ls) {
    const llvm::Instruction* cinst = curInst;
    const llvm::BasicBlock* cbb = curBB;
    setCurrentLocation(NULL,NULL);
    bool added = addGepEdge(src, dst, ls);
    setCurrentLocation(cinst,cbb);
    return added;
}


/*!
 * Add global black hole address edge
 */
bool PAG::addGlobalBlackHoleAddrEdge(NodeID node) {
    const llvm::Instruction* cinst = curInst;
    const llvm::BasicBlock* cbb = curBB;
    setCurrentLocation(NULL,NULL);
    bool added = addBlackHoleAddrEdge(node);
    setCurrentLocation(cinst,cbb);
    return added;
}

/*!
 * Add black hole Address edge for formal params
 */
bool PAG::addFormalParamBlackHoleAddrEdge(NodeID node, const llvm::Function* func)
{
    const llvm::Instruction* cinst = curInst;
    const llvm::BasicBlock* cbb = curBB;
    setCurrentLocation(NULL,&(func->getEntryBlock()));
    bool added = addBlackHoleAddrEdge(node);
    setCurrentLocation(cinst,cbb);
    return added;
}


/*!
 * Add a temp field value node according to base value and offset
 * FIXME: this node is after the initial node method, it is out of scope of symInfo table
 * The gep edge created are like constexpr (same edge may appear at multiple callsites)
 * so bb/inst of this edge may be rewritten several times, we treat it as global here.
 */
NodeID PAG::getGepValNode(const llvm::Value* val, const LocationSet& ls) {
    NodeID base = getBaseValNode(getValueNode(val));
    NodeLocationSetMap::iterator iter = GepValNodeMap.find(std::make_pair(base, ls));
    if (iter == GepValNodeMap.end()) {
        NodeID gepNode= addGepValNode(stripConstantCasts(val),ls,nodeNum);
        addGlobalGepEdge(base, gepNode, ls);
        return gepNode;
    } else
        return iter->second;
}

/*!
 * Add a temp field value node, this method can only invoked by getGepValNode
 */
NodeID PAG::addGepValNode(const llvm::Value* val, const LocationSet& ls, NodeID i) {
    NodeID base = getBaseValNode(getValueNode(val));
    //assert(findPAGNode(i) == false && "this node should not be created before");
    assert(0==GepValNodeMap.count(std::make_pair(base, ls))
           && "this node should not be created before");
    GepValNodeMap[std::make_pair(base, ls)] = i;
    GepValPN *node = new GepValPN(val, i, ls);
    return addNode(node,i);
}

/*!
 * Given an object node, find its field object node
 */
NodeID PAG::getGepObjNode(NodeID id, const LocationSet& ls) {
    PAGNode* node = pag->getPAGNode(id);
    if (GepObjPN* gepNode = dyn_cast<GepObjPN>(node))
        return getGepObjNode(gepNode->getMemObj(), gepNode->getLocationSet() + ls);
    else if (FIObjPN* baseNode = dyn_cast<FIObjPN>(node))
        return getGepObjNode(baseNode->getMemObj(), ls);
    else {
        assert(false && "new gep obj node kind?");
        return id;
    }
}

/*!
 * Get a field obj PAG node according to base mem obj and offset
 * To support flexible field sensitive analysis with regard to MaxFieldOffset
 * offset = offset % obj->getMaxFieldOffsetLimit() to create limited number of mem objects
 * maximum number of field object creation is obj->getMaxFieldOffsetLimit()
 */
NodeID PAG::getGepObjNode(const MemObj* obj, const LocationSet& ls) {
    NodeID base = getObjectNode(obj);

    /// if this obj is field-insensitive, just return the field-insensitive node.
    if (obj->isFieldInsensitive())
        return getFIObjNode(obj);

    LocationSet newLS = SymbolTableInfo::Symbolnfo()->getModulusOffset(obj->getTypeInfo(),ls);

    NodeLocationSetMap::iterator iter = GepObjNodeMap.find(std::make_pair(base, newLS));
    if (iter == GepObjNodeMap.end()) {
        NodeID gepNode= addGepObjNode(obj,newLS,nodeNum);
        return gepNode;
    } else
        return iter->second;

}

/*!
 * Add a field obj node, this method can only invoked by getGepObjNode
 */
NodeID PAG::addGepObjNode(const MemObj* obj, const LocationSet& ls, NodeID i) {
    //assert(findPAGNode(i) == false && "this node should not be created before");
    NodeID base = getObjectNode(obj);
    assert(0==GepObjNodeMap.count(std::make_pair(base, ls))
           && "this node should not be created before");
    GepObjNodeMap[std::make_pair(base, ls)] = i;
    GepObjPN *node = new GepObjPN(obj->getRefVal(), i, obj, ls);
    memToFieldsMap[base].set(i);
    return addNode(node,i);
}

/*!
 * Add a field-insensitive node, this method can only invoked by getFIGepObjNode
 */
NodeID PAG::addFIObjNode(const MemObj* obj, NodeID i)
{
    //assert(findPAGNode(i) == false && "this node should not be created before");
    NodeID base = getObjectNode(obj);
    memToFieldsMap[base].set(i);
    FIObjPN *node = new FIObjPN(obj->getRefVal(), i, obj);
    return addNode(node,i);
}


/*!
 * Return true if it is an intra-procedural edge
 */
bool PAG::hasIntraEdge(PAGNode* src, PAGNode* dst, PAGEdge::PEDGEK kind) {
    PAGEdge edge(src,dst,kind);
    return PAGEdgeKindToSetMap[kind].find(&edge) != PAGEdgeKindToSetMap[kind].end();
}

/*!
 * Return true if it is an inter-procedural edge
 */
bool PAG::hasInterEdge(PAGNode* src, PAGNode* dst, PAGEdge::PEDGEK kind, const llvm::Instruction* callInst) {
    PAGEdge edge(src,dst,PAGEdge::makeEdgeFlagWithCallInst(kind,callInst));
    return PAGEdgeKindToSetMap[kind].find(&edge) != PAGEdgeKindToSetMap[kind].end();
}

/*!
 * Add a PAG edge into edge map
 */
bool PAG::addEdge(PAGNode* src, PAGNode* dst, PAGEdge* edge) {

    DBOUT(DPAGBuild,
          outs() << "add edge from " << src->getId() << " kind :"
          << src->getNodeKind() << " to " << dst->getId()
          << " kind :" << dst->getNodeKind() << "\n");
    src->addOutEdge(edge);
    dst->addInEdge(edge);
    bool added = PAGEdgeKindToSetMap[edge->getEdgeKind()].insert(edge).second;
    assert(added && "duplicated edge, not added!!!");
    // PAG edges in a function
    if(curInst) {
        assert(curBB && "instruction does not have a basic block??");
        inst2PAGEdgesMap[curInst].push_back(edge);
        edge->setInst(curInst);
    }
    // global PAG edges
    else {
        if(curBB && (&curBB->getParent()->getEntryBlock() == curBB))
            funToEntryPAGEdges[curBB->getParent()].insert(edge);
        else
            globPAGEdgesSet.insert(edge);
    }
    return true;

}

/*!
 * Get all fields object nodes of an object
 */
NodeBS& PAG::getAllFieldsObjNode(const MemObj* obj) {
    NodeID base = getObjectNode(obj);
    return memToFieldsMap[base];
}

/*!
 * Get all fields object nodes of an object
 */
NodeBS& PAG::getAllFieldsObjNode(NodeID id) {
    const PAGNode* node = pag->getPAGNode(id);
    assert(llvm::isa<ObjPN>(node) && "need an object node");
    const ObjPN* obj = llvm::cast<ObjPN>(node);
    return getAllFieldsObjNode(obj->getMemObj());
}

/*!
 * Get all fields object nodes of an object
 * If this object is collapsed into one field insensitive object
 * Then only return this field insensitive object
 */
NodeBS PAG::getFieldsAfterCollapse(NodeID id) {
    const PAGNode* node = pag->getPAGNode(id);
    assert(llvm::isa<ObjPN>(node) && "need an object node");
    const MemObj* mem = llvm::cast<ObjPN>(node)->getMemObj();
    if(mem->isFieldInsensitive()) {
        NodeBS bs;
        bs.set(getFIObjNode(mem));
        return bs;
    }
    else
        return getAllFieldsObjNode(mem);
}

/*!
 * Get a base pointer given a pointer
 * Return the source node of its connected gep edge if this pointer has
 * Otherwise return the node id itself
 */
NodeID PAG::getBaseValNode(NodeID nodeId) {
    PAGNode* node  = getPAGNode(nodeId);
    if (node->hasIncomingEdges(PAGEdge::NormalGep) ||  node->hasIncomingEdges(PAGEdge::VariantGep)) {
        PAGEdge::PAGEdgeSetTy& ngeps = node->getIncomingEdges(PAGEdge::NormalGep);
        PAGEdge::PAGEdgeSetTy& vgeps = node->getIncomingEdges(PAGEdge::VariantGep);

        assert(((ngeps.size()+vgeps.size())==1) && "one node can only be connected by at most one gep edge!");

        PAGNode::iterator it;
        if(!ngeps.empty())
            it = ngeps.begin();
        else
            it = vgeps.begin();

        assert(isa<GepPE>(*it) && "not a gep edge??");
        return (*it)->getSrcID();
    }
    else
        return nodeId;
}

/*!
 * Get a base PAGNode given a pointer
 * Return the source node of its connected normal gep edge
 * Otherwise return the node id itself
 * Size_t offset : gep offset
 */
LocationSet PAG::getLocationSetFromBaseNode(NodeID nodeId) {
    PAGNode* node  = getPAGNode(nodeId);
    PAGEdge::PAGEdgeSetTy& geps = node->getIncomingEdges(PAGEdge::NormalGep);
    /// if this node is already a base node
    if(geps.empty())
        return LocationSet(0);

    assert(geps.size()==1 && "one node can only be connected by at most one gep edge!");
    PAGNode::iterator it = geps.begin();
    const PAGEdge* edge = *it;
    assert(isa<NormalGepPE>(edge) && "not a get edge??");
    const NormalGepPE* gepEdge = cast<NormalGepPE>(edge);
    return gepEdge->getLocationSet();
}

/*!
 * Clean up memory
 */
void PAG::destroy() {
    for (PAGEdge::PAGKindToEdgeSetMapTy::iterator I =
                PAGEdgeKindToSetMap.begin(), E = PAGEdgeKindToSetMap.end(); I != E;
            ++I) {
        for (PAGEdge::PAGEdgeSetTy::iterator edgeIt = I->second.begin(),
                endEdgeIt = I->second.end(); edgeIt != endEdgeIt; ++edgeIt) {
            delete *edgeIt;
        }
    }
    delete symInfo;
    symInfo = NULL;
}

/*!
 * Print this PAG graph including its nodes and edges
 */
void PAG::print() {
    for (iterator I = begin(), E = end(); I != E; ++I) {
        PAGNode* node = I->second;
        if (!isa<DummyValPN>(node) && !isa<DummyObjPN>(node)) {
            outs() << "node " << node->getId() << " " << *(node->getValue())
                   << "\n";
            outs() << "\t InEdge: { ";
            for (PAGNode::iterator iter = node->getInEdges().begin();
                    iter != node->getInEdges().end(); ++iter) {
                outs() << (*iter)->getSrcID() << " ";
                if (NormalGepPE* edge = dyn_cast<NormalGepPE>(*iter))
                    outs() << " offset=" << edge->getOffset() << " ";
                else if (isa<VariantGepPE>(*iter))
                    outs() << " offset=variant";
            }
            outs() << "}\t";
            outs() << "\t OutEdge: { ";
            for (PAGNode::iterator iter = node->getOutEdges().begin();
                    iter != node->getOutEdges().end(); ++iter) {
                outs() << (*iter)->getDstID() << " ";
                if (NormalGepPE* edge = dyn_cast<NormalGepPE>(*iter))
                    outs() << " offset=" << edge->getOffset() << " ";
                else if (isa<VariantGepPE>(*iter))
                    outs() << " offset=variant";
            }
            outs() << "}\n";
        }
        outs() << "\n";
    }

}

/*
 * If this is a dummy node or node does not have incoming edges we assume it is not a pointer here
 */
bool PAG::isValidPointer(NodeID nodeId) const {
    PAGNode* node = pag->getPAGNode(nodeId);
    if ((node->getInEdges().empty() && node->getOutEdges().empty()))
        return false;
    return node->isPointer();
}

/*!
 * PAGEdge constructor
 */
PAGEdge::PAGEdge(PAGNode* s, PAGNode* d, GEdgeFlag k) :
    GenericPAGEdgeTy(s,d,k),inst(NULL) {
    edgeId = PAG::getPAG()->getTotalEdgeNum();
    PAG::getPAG()->incEdgeNum();
}

/*!
 * PAGNode constructor
 */
PAGNode::PAGNode(const llvm::Value* val, NodeID i, PNODEK k) :
    GenericPAGNodeTy(i,k), value(val) {

    assert( ValNode <= k && k<= DummyObjNode && "new PAG node kind?");

    switch (k) {
    case ValNode:
    case GepValNode: {
        assert(val != NULL && "value is NULL for ValPN or GepValNode");
        isTLPointer = val->getType()->isPointerTy();
        isATPointer = false;
        break;
    }

    case RetNode: {
        assert(val != NULL && "value is NULL for RetNode");
        isTLPointer = llvm::cast<llvm::Function>(val)->getReturnType()->isPointerTy();
        isATPointer = false;
        break;
    }

    case VarargNode:
    case DummyValNode: {
        isTLPointer = true;
        isATPointer = false;
        break;
    }

    case ObjNode:
    case GepObjNode:
    case FIObjNode:
    case DummyObjNode: {
        isTLPointer = false;
        isATPointer = true;
        break;
    }
    }
}

/*!
 * Dump this PAG
 */
void PAG::dump(std::string name) {
    GraphPrinter::WriteGraphToFile(llvm::outs(), name, this);
}

namespace llvm {
/*!
 * Write value flow graph into dot file for debugging
 */
template<>
struct DOTGraphTraits<PAG*> : public DefaultDOTGraphTraits {

    typedef PAGNode NodeType;
    typedef NodeType::iterator ChildIteratorType;
    DOTGraphTraits(bool isSimple = false) :
        DefaultDOTGraphTraits(isSimple) {
    }

    /// Return name of the graph
    static std::string getGraphName(PAG *graph) {
        return graph->getGraphName();
    }

    /// Return label of a VFG node with two display mode
    /// Either you can choose to display the name of the value or the whole instruction
    static std::string getNodeLabel(PAGNode *node, PAG *graph) {
        bool briefDisplay = true;
        bool nameDisplay = true;
        std::string str;
        raw_string_ostream rawstr(str);

        if (briefDisplay) {
            if (isa<ValPN>(node)) {
                if (nameDisplay)
                    rawstr << node->getId() << ":" << node->getValueName();
                else
                    rawstr << node->getId();
            } else
                rawstr << node->getId();
        } else {
            // print the whole value
            if (!isa<DummyValPN>(node) || !isa<DummyObjPN>(node))
                rawstr << *node->getValue();
            else
                rawstr << "";

        }

        return rawstr.str();

    }

    static std::string getNodeAttributes(PAGNode *node, PAG *pag) {
        if (isa<ValPN>(node)) {
            if(isa<GepValPN>(node))
                return "shape=hexagon";
            else if (isa<DummyValPN>(node))
                return "shape=diamond";
            else
                return "shape=circle";
        } else if (isa<ObjPN>(node)) {
            if(isa<GepObjPN>(node))
                return "shape=doubleoctagon";
            else if(isa<FIObjPN>(node))
                return "shape=septagon";
            else if (isa<DummyObjPN>(node))
                return "shape=Mcircle";
            else
                return "shape=doublecircle";
        } else if (isa<RetPN>(node)) {
            return "shape=Mrecord";
        } else if (isa<VarArgPN>(node)) {
            return "shape=octagon";
        } else {
            assert(0 && "no such kind node!!");
        }
        return "";
    }

    template<class EdgeIter>
    static std::string getEdgeAttributes(PAGNode *node, EdgeIter EI, PAG *pag) {
        const PAGEdge* edge = *(EI.getCurrent());
        assert(edge && "No edge found!!");
        if (isa<AddrPE>(edge)) {
            return "color=green";
        } else if (isa<CopyPE>(edge)) {
            return "color=black";
        } else if (isa<GepPE>(edge)) {
            return "color=purple";
        } else if (isa<StorePE>(edge)) {
            return "color=blue";
        } else if (isa<LoadPE>(edge)) {
            return "color=red";
        } else if (isa<TDForkPE>(edge)) {
            return "color=Turquoise";
        } else if (isa<TDJoinPE>(edge)) {
            return "color=Turquoise";
        } else if (isa<CallPE>(edge)) {
            return "color=black,style=dashed";
        } else if (isa<RetPE>(edge)) {
            return "color=black,style=dotted";
        }
        else {
            assert(0 && "No such kind edge!!");
        }
    }

    template<class EdgeIter>
    static std::string getEdgeSourceLabel(PAGNode *node, EdgeIter EI) {
        const PAGEdge* edge = *(EI.getCurrent());
        assert(edge && "No edge found!!");
        if(const CallPE* calledge = dyn_cast<CallPE>(edge)) {
            const llvm::Instruction* callInst= calledge->getCallInst();
            return analysisUtil::getSourceLoc(callInst);
        }
        else if(const RetPE* retedge = dyn_cast<RetPE>(edge)) {
            const llvm::Instruction* callInst= retedge->getCallInst();
            return analysisUtil::getSourceLoc(callInst);
        }
        return "";
    }
};
}
