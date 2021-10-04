@ECHO OFF
CLS

IF [%1] == [] (
    SET SwitchFlags=/Od /W4 /Zi /FC /D INTERNAL=1
) ELSE IF [%1] == [release] (
    ECHO -------- RELEASE --------
    SET SwitchFlags=/O2 /W4 /D INTERNAL=0
) ELSE (
    ECHO ERROR: unknown build type '%1'
    GOTO :ERROR
)

SET RemoveWarnings=/wd4100 /wd4458
SET SourceFile=%CD%\code\main.cpp
SET CompilerFlags=/nologo /fp:fast /fp:except- /EHa- /GR- /GS- /Gs9999999 /Fe:"VCapture.exe"
SET LinkerFlags=/INCREMENTAL:NO /SUBSYSTEM:WINDOWS,5.02
REM /NODEFAULTLIB
SET Libraries=kernel32.lib winmm.lib ws2_32.lib Ole32.lib user32.lib gdi32.lib
PUSHD build
cl.exe %SwitchFlags% %RemoveWarnings% %CompilerFlags% %SourceFile% /link %LinkerFlags% %Libraries%
POPD

:ERROR