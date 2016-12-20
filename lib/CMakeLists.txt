
include_directories (${SVFexperiment_SOURCE_DIR}/include ${SVFexperiment_SOURCE_DIR}/CUDD)

set (SVFexperimentSourceCodes
 ./CUDD/cuddTable.c
 ./CUDD/cuddZddIsop.c
 ./CUDD/dddmpStoreAdd.c
 ./CUDD/cuddInteract.c
 ./CUDD/cuddBddCorr.c
 ./CUDD/cuddGenetic.c
 ./CUDD/cuddZddGroup.c
 ./CUDD/cpu_stats.c
 ./CUDD/cuddReorder.c
 ./CUDD/cuddZddReord.c
 ./CUDD/prtime.c
 ./CUDD/cuddLevelQ.c
 ./CUDD/cuddBridge.c
 ./CUDD/cuddSplit.c
 ./CUDD/test-res.c
 ./CUDD/epd.c
 ./CUDD/dddmpUtil.c
 ./CUDD/ptime.c
 ./CUDD/cuddDecomp.c
 ./CUDD/cuddBddIte.c
 ./CUDD/cuddSubsetHB.c
 ./CUDD/cuddSymmetry.c
 ./CUDD/cuddGroup.c
 ./CUDD/cuddAddApply.c
 ./CUDD/dddmpBinary.c
 ./CUDD/cuddSign.c
 ./CUDD/cuddGenCof.c
 ./CUDD/cuddCompose.c
 ./CUDD/dddmpLoad.c
 ./CUDD/cuddAnneal.c
 ./CUDD/cuddZddLin.c
 ./CUDD/cuddAndAbs.c
 ./CUDD/dddmpNodeBdd.c
 ./CUDD/cuddExport.c
 ./CUDD/restart.c
 ./CUDD/test-sav.c
 ./CUDD/testmtr.c
 ./CUDD/cuddAPI.c
 ./CUDD/dddmpDbg.c
 ./CUDD/cuddApprox.c
 ./CUDD/cuddApa.c
 ./CUDD/cuddSolve.c
 ./CUDD/cuddZddMisc.c
 ./CUDD/cuddMatMult.c
 ./CUDD/cuddWindow.c
 ./CUDD/dddmpStoreBdd.c
 ./CUDD/strsav.c
 ./CUDD/dddmpStoreCnf.c
 ./CUDD/cuddZddSetop.c
 ./CUDD/dddmpConvert.c
 ./CUDD/pipefork.c
 ./CUDD/dddmpDdNodeBdd.c
 ./CUDD/state.c
 ./CUDD/cuddRead.c
 ./CUDD/cuddCheck.c
 ./CUDD/cuddZddCount.c
 ./CUDD/safe_mem.c
 ./CUDD/stub.c
 ./CUDD/st.c
 ./CUDD/cuddZddSymm.c
 ./CUDD/cuddSubsetSP.c
 ./CUDD/cuddLiteral.c
 ./CUDD/cuddAddIte.c
 ./CUDD/cuddLinear.c
 ./CUDD/testcudd.c
 ./CUDD/texpand.c
 ./CUDD/datalimit.c
 ./CUDD/cuddClip.c
 ./CUDD/cuddZddFuncs.c
 ./CUDD/cuddAddWalsh.c
 ./CUDD/dddmpLoadCnf.c
 ./CUDD/mtrGroup.c
 ./CUDD/testdddmp.c
 ./CUDD/cuddSat.c
 ./CUDD/dddmpNodeCnf.c
 ./CUDD/cuddCache.c
 ./CUDD/mtrBasic.c
 ./CUDD/cuddInit.c
 ./CUDD/cuddExact.c
 ./CUDD/cuddAddInv.c
 ./CUDD/cuddHarwell.c
 ./CUDD/cuddAddAbs.c
 ./CUDD/cuddPriority.c
 ./CUDD/cuddUtil.c
 ./CUDD/cuddZddPort.c
 ./CUDD/cuddBddAbs.c
 ./CUDD/cpu_time.c
 ./CUDD/cuddZddUtil.c
 ./CUDD/cuddAddNeg.c
 ./CUDD/cuddLCache.c
 ./CUDD/pathsearch.c
 ./CUDD/cuddRef.c
 ./CUDD/dddmpStoreMisc.c
 ./CUDD/saveimage.c
 ./CUDD/cuddEssent.c
 ./CUDD/cuddCof.c
 ./CUDD/cuddAddFind.c
 ./CUDD/dddmpNodeAdd.c
 ./SABER/SaberAnnotator.cpp
 ./SABER/LeakChecker.cpp
 ./SABER/SrcSnkDDA.cpp
 ./SABER/FileChecker.cpp
 ./SABER/DoubleFreeChecker.cpp
 ./SABER/ProgSlice.cpp
 ./SABER/SaberCheckerAPI.cpp
 ./SABER/SaberSVFGBuilder.cpp
 ./MSSA/MemRegion.cpp
 ./MSSA/MemSSA.cpp
 ./MSSA/SVFGBuilder.cpp
 ./MSSA/SVFGOPT.cpp
 ./MSSA/SVFGStat.cpp
 ./MSSA/SVFG.cpp
 ./MSSA/MemPartition.cpp
 ./MemoryModel/PAG.cpp
 ./MemoryModel/LocationSet.cpp
 ./MemoryModel/LocMemModel.cpp
 ./MemoryModel/PointerAnalysis.cpp
 ./MemoryModel/ConsG.cpp
 ./MemoryModel/MemModel.cpp
 ./MemoryModel/PAGBuilder.cpp
 ./WPA/AndersenStat.cpp
 ./WPA/FlowSensitiveStat.cpp
 ./WPA/WPAPass.cpp
 ./WPA/FlowSensitive.cpp
 ./WPA/AndersenWave.cpp
 ./WPA/AndersenWaveDiff.cpp
 ./WPA/Andersen.cpp
 ./WPA/AndersenLCD.cpp
 ./Util/PTAStat.cpp
 ./Util/ThreadCallGraph.cpp
 ./Util/PTACallGraph.cpp
 ./Util/PathCondAllocator.cpp
 ./Util/RaceAnnotator.cpp
 ./Util/BreakConstantExpr.cpp
 ./Util/ThreadAPI.cpp
 ./Util/AnalysisUtil.cpp
 ./Util/DataFlowUtil.cpp
 ./Util/ExtAPI.cpp
 ./Util/Conditions.cpp
)
 
add_library (SVFexperimentObj OBJECT ${SVFexperimentSourceCodes})
#add_library (SVFexperiment SHARED $<TARGET_OBJECTS:SVFexperimentObj>)
add_library (SVFexperimentStatic STATIC $<TARGET_OBJECTS:SVFexperimentObj>)
target_link_libraries (SVFexperimentStatic LLVMCore LLVMSupport)


add_executable(WPAexe
  WPA/Andersen.cpp
  WPA/AndersenStat.cpp
  WPA/AndersenWaveDiff.cpp
  WPA/FlowSensitiveStat.cpp
  WPA/AndersenLCD.cpp
  WPA/AndersenWave.cpp
  WPA/FlowSensitive.cpp
  WPA/WPAPass.cpp
)

target_link_libraries (WPAexe SVFexperimentStatic LLVMLTO LLVMCore LLVMSupport)


add_executable(SABERexe
  SABER/DoubleFreeChecker.cpp
  SABER/LeakChecker.cpp
  SABER/ProgSlice.cpp
  SABER/SaberCheckerAPI.cpp
  SABER/SrcSnkDDA.cpp
  SABER/FileChecker.cpp
  SABER/SaberAnnotator.cpp
  SABER/SaberSVFGBuilder.cpp
)

target_link_libraries (SABERexe SVFexperimentStatic LLVMLTO LLVMCore LLVMSupport)