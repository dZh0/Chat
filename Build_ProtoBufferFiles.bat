@ECHO off
SET PROTO_EXEC="%~dp0%external\protobuf-3.13.0\bin\protoc.exe"
SET PROTO_DIR=%~dp0%scr\ProtoBuffer\
SET GEN_HEADER="%PROTO_DIR%Messages.generated.h"
ECHO //This header is generated by %~n0%~x0. > %GEN_HEADER%
ECHO //DO NOT EDIT! >> %GEN_HEADER%
ECHO #pragma once >> %GEN_HEADER%
SETLOCAL ENABLEDELAYEDEXPANSION
FOR %%f in (%PROTO_DIR%*.proto) DO (
	echo Parsing "%%f ..."
	%PROTO_EXEC% -I=%PROTO_DIR% --cpp_out=%PROTO_DIR% "%%f"
	SET file_name=%%~nf
	ECHO #include "../ProtoBuffer/!file_name!.pb.h" >> %GEN_HEADER%
)
ENDLOCAL