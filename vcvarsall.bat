

@if not "%VSCMD_DEBUG%" GEQ "3" echo off

@REM -------------------------------------------------------------------------
:parse_args
set __VCVARSALL_STORE=
set __VCVARSALL_WINSDK=
set __VCVARSALL_PARSE_ERROR=
set __VCVARSALL_TARGET_ARCH=
set __VCVARSALL_HOST_ARCH=
set __VCVARSALL_VER=

@REM Parse command line arguments.  This implementation does not care about
@REM argument order.

if "%VSCMD_DEBUG%" GEQ "1" (
    @echo [DEBUG:%~nx0] init with arguments '%*'
)

set __VCVARSALL_PARSE_ERROR=0
set "__VCVARSALL_ARGS_LIST=%*"
call :parse_loop
set __VCVARSALL_ARGS_LIST=

if "%VSCMD_DEBUG%" GEQ "1" (
    @echo [DEBUG:%~nx0] Command line parse completed with values:
    @echo [DEBUG:%~nx0] __VCVARSALL_TARGET_ARCH='%__VCVARSALL_TARGET_ARCH%'
    @echo [DEBUG:%~nx0] __VCVARSALL_HOST_ARCH='%__VCVARSALL_HOST_ARCH%'
    @echo [DEBUG:%~nx0] __VCVARSALL_WINSDK='%__VCVARSALL_WINSDK%'
    @echo [DEBUG:%~nx0] __VCVARSALL_STORE='%__VCVARSALL_STORE%'
    @echo [DEBUG:%~nx0] __VCVARSALL_HELP='%__VCVARSALL_HELP%'
    @echo [DEBUG:%~nx0] __VCVARSALL_PARSE_ERROR='%__VCVARSALL_PARSE_ERROR%'
)

if "%__VCVARSALL_CLEAN_ENV%" NEQ "" goto :call_vsdevcmd
if "%__VCVARSALL_PARSE_ERROR%" NEQ "0" goto :usage_error
if "%__VCVARSALL_HELP%" NEQ "" goto :usage

@REM -------------------------------------------------------------------------
:check_platform
@REM This script is installed to ...\VC\Auxiliary\Build.
@REM vsdevcmd is installed to    ...\Common7\Tools.
if not exist "%~dp0..\..\..\Common7\Tools\vsdevcmd.bat" goto missing

@REM Assemble the arguments to pass to vsdevcmd.bat
if "%__VCVARSALL_TARGET_ARCH%" == "" goto :usage_error
if "%__VCVARSALL_HOST_ARCH%" == "" goto :usage_error

set "__VCVARSALL_VSDEVCMD_ARGS=-arch=%__VCVARSALL_TARGET_ARCH% -host_arch=%__VCVARSALL_HOST_ARCH%"
if "%__VCVARSALL_WINSDK%" NEQ "" (
    set "__VCVARSALL_VSDEVCMD_ARGS=%__VCVARSALL_VSDEVCMD_ARGS% -winsdk=%__VCVARSALL_WINSDK%"
)
if "%__VCVARSALL_STORE%" NEQ "" (
    set "__VCVARSALL_VSDEVCMD_ARGS=%__VCVARSALL_VSDEVCMD_ARGS% -app_platform=UWP"
)
if "%__VCVARSALL_VER%"=="14.0" (
    set "__VCVARSALL_VSDEVCMD_ARGS=%__VCVARSALL_VSDEVCMD_ARGS% -vcvars_ver=14.0"
)

goto :call_vsdevcmd

@REM -------------------------------------------------------------------------
@REM Call vsdevcmd.bat to setup the command prompt environment

:call_vsdevcmd

@REM This temporary environment variable is used to control setting of VC++ 
@REM command prompt-specific environment variables that should not be set
@REM by the VS Developer Command prompt (specifically vsdevcmd\ext\vcvars.bat). 
@REM The known case this effects is the Platform environment variable, which 
@REM will override platform target for .NET builds.
set VSCMD_VCVARSALL_INIT=1

@REM Special handling for the /clean_env argument
if "%__VCVARSALL_CLEAN_ENV%" NEQ "" (
    call "%~dp0..\..\..\Common7\Tools\vsdevcmd.bat" /clean_env
    goto :end
)

call "%~dp0..\..\..\Common7\Tools\vsdevcmd.bat" %__VCVARSALL_VSDEVCMD_ARGS%

if "%ERRORLEVEL%"=="0" (
    @REM Print the target command prompt architecture...
    if "%__VCVARSALL_HOST_ARCH%" NEQ "%__VCVARSALL_TARGET_ARCH%" (
        echo [%~nx0] Environment initialized for: '%__VCVARSALL_HOST_ARCH%_%__VCVARSALL_TARGET_ARCH%'
    ) else (
        echo [%~nx0] Environment initialized for: '%__VCVARSALL_TARGET_ARCH%'
    )
)
goto :end

:parse_loop
for /F "tokens=1,* delims= " %%a in ("%__VCVARSALL_ARGS_LIST%") do (
    if "%VSCMD_DEBUG%" GEQ "2" (
        @echo [DEBUG:%~nx0] inner argument {%%a}
    )
    call :parse_argument %%a
    set "__VCVARSALL_ARGS_LIST=%%b"
    goto :parse_loop
)

exit /B 0

:parse_argument

@REM called by :parse_loop and expects the arguments to either be:
@REM 1. a single argument in %1
@REM 2. an argument pair from the command line specified as '%1=%2'

set __local_ARG_FOUND=
@REM Architecture
if /I "%1"=="x86" (
    set __VCVARSALL_TARGET_ARCH=x86
    set __VCVARSALL_HOST_ARCH=x86
    set __local_ARG_FOUND=1
)
if /I "%1"=="x86_amd64" (
    set __VCVARSALL_TARGET_ARCH=x64
    set __VCVARSALL_HOST_ARCH=x86
    set __local_ARG_FOUND=1
)
if /I "%1"=="x86_x64" (
    set __VCVARSALL_TARGET_ARCH=x64
    set __VCVARSALL_HOST_ARCH=x86
    set __local_ARG_FOUND=1
)
if /I "%1"=="x86_arm" (
    set __VCVARSALL_TARGET_ARCH=arm
    set __VCVARSALL_HOST_ARCH=x86
    set __local_ARG_FOUND=1
)
if /I "%1"=="x86_arm64" (
    set __VCVARSALL_TARGET_ARCH=arm64
    set __VCVARSALL_HOST_ARCH=x86
    set __local_ARG_FOUND=1
)
if /I "%1"=="amd64" (
    set __VCVARSALL_TARGET_ARCH=x64
    set __VCVARSALL_HOST_ARCH=x64
    set __local_ARG_FOUND=1
)
if /I "%1"=="x64" (
    set __VCVARSALL_TARGET_ARCH=x64
    set __VCVARSALL_HOST_ARCH=x64
    set __local_ARG_FOUND=1
)
if /I "%1"=="amd64_x86" (
    set __VCVARSALL_TARGET_ARCH=x86
    set __VCVARSALL_HOST_ARCH=x64
    set __local_ARG_FOUND=1
)
if /I "%1"=="x64_x86" (
    set __VCVARSALL_TARGET_ARCH=x86
    set __VCVARSALL_HOST_ARCH=x64
    set __local_ARG_FOUND=1
)
if /I "%1"=="amd64_arm" (
    set __VCVARSALL_TARGET_ARCH=arm
    set __VCVARSALL_HOST_ARCH=x64
    set __local_ARG_FOUND=1
)
if /I "%1"=="x64_arm" (
    set __VCVARSALL_TARGET_ARCH=arm
    set __VCVARSALL_HOST_ARCH=x64
    set __local_ARG_FOUND=1
)
if /I "%1"=="amd64_arm64" (
    set __VCVARSALL_TARGET_ARCH=arm64
    set __VCVARSALL_HOST_ARCH=x64
    set __local_ARG_FOUND=1
)
if /I "%1"=="x64_arm64" (
    set __VCVARSALL_TARGET_ARCH=arm64
    set __VCVARSALL_HOST_ARCH=x64
    set __local_ARG_FOUND=1
)
if /I "%1"=="-vcvars_ver" (
    set "__VCVARSALL_VER=%2"
    set __local_ARG_FOUND=1
)
if /I "%1"=="/vcvars_ver" (
    set "__VCVARSALL_VER=%2"
    set __local_ARG_FOUND=1
)
if /I "%1"=="--vcvars_ver" (
    set "__vcvarsall_ver=%2"
    set __local_ARG_FOUND=1
)
if /I "%1"=="help" (
    set __VCVARSALL_HELP=1
    set __local_ARG_FOUND=1
)
if /I "%1"=="/help" (
    set __VCVARSALL_HELP=1
    set __local_ARG_FOUND=1
)
if /I "%1"=="-help" (
    set __VCVARSALL_HELP=1
    set __local_ARG_FOUND=1
)
if /I "%1"=="/?" (
    set __VCVARSALL_HELP=1
    set __local_ARG_FOUND=1
)
if /I "%1"=="-?" (
    set __VCVARSALL_HELP=1
    set __local_ARG_FOUND=1
)

@REM -- /clean_env --
@REM Mostly used for internal testing to restore the state of
@REM the command line environment to its state prior to vcvarsall.bat
@REM being executed.
if /I "%1"=="/clean_env" (
    set __VCVARSALL_CLEAN_ENV=/clean_env
    set __local_ARG_FOUND=1
)
if /I "%1"=="-clean_env" (
    set __VCVARSALL_CLEAN_ENV=/clean_env
    set __local_ARG_FOUND=1
)

@REM Windows SDK Version
if /I "%1"=="8.1" (
    set "__VCVARSALL_WINSDK=8.1"
    set __local_ARG_FOUND=1
)

set __temp1=%1
if /I "%__temp1:~0,3%"=="10." (
    set "__VCVARSALL_WINSDK=%1"
    set __local_ARG_FOUND=1
)
set __temp1=

@REM Store/UWP
if /I "%1"=="store" (
    set "__VCVARSALL_STORE=-app_platform=UWP"
    set __local_ARG_FOUND=1
)
if /I "%1"=="uwp" (
    set "__VCVARSALL_STORE=-app_platform=UWP"
    set __local_ARG_FOUND=1
)

if "%__local_ARG_FOUND%" NEQ "1" (
    set /A __VCVARSALL_PARSE_ERROR=__VCVARSALL_PARSE_ERROR+1
    if "%2"=="" (
        @echo [ERROR:%~nx0] Invalid argument found : %1
    ) else (
        @echo [ERROR:%~nx0] Invalid argument found : %1=%2
    )
)
set __local_ARG_FOUND=
exit /B 0

:usage_error
echo [ERROR:%~nx0] Error in script usage. The correct usage is:
goto :usage

:usage
echo Syntax:
echo     %~nx0 [arch] [platform_type] [winsdk_version] [-vcvars_ver=vc_version]
echo where : 
echo     [arch]: x86 ^| amd64 ^| x86_amd64 ^| x86_arm ^| x86_arm64 ^| amd64_x86 ^| amd64_arm ^| amd64_arm64
echo     [platform_type]: {empty} ^| store ^| uwp 
echo     [winsdk_version] : full Windows 10 SDK number (e.g. 10.0.10240.0) or "8.1" to use the Windows 8.1 SDK.   
echo     [vc_version] : "14.0" for VC++ 2015 Compiler Toolset ^| {empty} for default VS 2017 VC++ compiler toolset
echo:
echo The store parameter sets environment variables to support Universal Windows Platform application 
echo development and is an alias for 'uwp'.
echo:
echo For example:
echo     %~nx0 x86_amd64
echo     %~nx0 x86_amd64 10.0.10240.0
echo     %~nx0 x86_arm uwp 10.0.10240.0
echo     %~nx0 x86_arm onecore 10.0.10240.0 -vcvars_ver=14.0
echo     %~nx0 x64 8.1
echo     %~nx0 x64 store 8.1
echo:
echo Please make sure either Visual Studio or C++ Build SKU is installed.
goto :end

:missing
echo The specified configuration type is missing.  The tools for the
echo configuration might not be installed.
goto :end

:end
set __VCVARSALL_TARGET_ARCH=
set __VCVARSALL_HOST_ARCH=
set __VCVARSALL_STORE=
set __VCVARSALL_WINSDK=
set __VCVARSALL_PARSE_ERROR=
set __VCVARSALL_CLEAN_ENV=
set VSCMD_VCVARSALL_INIT=
set __VCVARSALL_VSDEVCMD_ARGS=
set __VCVARSALL_HELP=
set __VCVARSALL_VER=
