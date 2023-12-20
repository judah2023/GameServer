pushd %~dp0

protoc.exe --proto_path=./ --cpp_out=./ ./Protocol.proto
IF ERRORLEVEL 1 PAUSE

XCOPY /Y Protocol.pb.cc "../../GameServer"
XCOPY /Y Protocol.pb.h "../../GameServer"

XCOPY /Y Protocol.pb.cc "../../GameClient"
XCOPY /Y Protocol.pb.h "../../GameClient"