@echo off
setlocal enabledelayedexpansion

REM ====== Blender 경로 설정 (둘 중 하나만 쓰세요) ======
REM 1) PATH에 blender가 잡혀있으면 아래 줄 그대로
set "BLENDER=blender.exe"

REM 2) PATH에 없으면 아래 줄 주석 풀고 본인 설치 경로로 수정
REM set "BLENDER=C:\Program Files\Blender Foundation\Blender 4.2\blender.exe"

REM ====== 스크립트 경로 (이 bat 옆에 있다고 가정) ======
set "SCRIPT=%~dp0fbx_externalize_textures.py"

REM ====== 입력 체크 ======
if "%~1"=="" (
  echo Usage: %~nx0 "path\to\model.fbx"
  echo Tip: FBX 파일을 이 배치파일에 드래그&드롭해도 됩니다.
  exit /b 1
)

set "INFBX=%~1"
if not exist "%INFBX%" (
  echo ERROR: Input FBX not found: "%INFBX%"
  exit /b 1
)

if not exist "%SCRIPT%" (
  echo ERROR: Script not found: "%SCRIPT%"
  exit /b 1
)

REM ====== 출력 파일명: 원본경로\원본이름_unpack.fbx ======
set "OUTFBX=%~dpn1_unpack.fbx"

REM ====== 텍스처 폴더: 원본경로\원본이름_textures ======
set "TEXDIR=%textures"

echo.
echo [Input ] "%INFBX%"
echo [TexDir] "%TEXDIR%"
echo [Output] "%OUTFBX%"
echo.

"%BLENDER%" -b -P "%SCRIPT%" -- "%INFBX%" "%TEXDIR%" "%OUTFBX%"
set "ERR=%ERRORLEVEL%"

echo.
if not "%ERR%"=="0" (
  echo FAILED. ExitCode=%ERR%
  exit /b %ERR%
) else (
  echo DONE.
  exit /b 0
)