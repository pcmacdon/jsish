# Sample CMakeLists.txt to be edited/used when building jsi into a larger system (untested).

#SET( CMAKE_C_FLAGS "-g -Wall" )
add_definitions(-DJSI__SQLITE)
add_definitions(-DJSI__WEBSOCKET)
add_definitions(-DJSI__SOCKET)
add_definitions(-DJSI__MYSQL)

include_directories(../sqlite3 ../libwebsockets ../miniz)

add_library(libjsi
  jsiLexer.c jsiFunc.c jsiValue.c jsiRegexp.c jsiPstate.c jsiInterp.c
  jsiUtils.c jsiProto.c jsiFilesys.c jsiChar.c jsiString.c jsiBool.c
  jsiNumber.c jsiArray.c jsiLoad.c jsiHash.c jsiOptions.c jsiStubs.c
  jsiFormat.c jsiExec.c jsiJSON.c jsiTclUtil.c jsiCmds.c jsiFileCmds.c jsiObj.c jsiSignal.c
  jsiTree.c jsiMD5.c jsiDString.c jsiMath.c jsmn.c jsiZvfs.c jsiUtf8.c jsiUserObj.c parser.c
  jsiEval.c 
  win/compat.c win/regex.c win/strptime.c 
  ../sqlite3/sqlite3.c ../miniz/miniz.c
)

#target_link_libraries(libjsi m libz dl libwebsockets sqlite3)

