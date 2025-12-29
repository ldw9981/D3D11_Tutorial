@echo off
setlocal

REM ====== Blender 경로 설정 ======
REM PATH에 blender.exe가 잡혀 있으면 아래 그대로 사용
set "BLENDER=blender.exe"

REM PATH에 없으면 아래 줄 주석 해제하고 본인 경로로 수정
REM set "BLENDER=C:\Program Files\Blender Foundation\Blender 4.2\blender.exe"

REM ====== 스크립트 경로 (bat 파일과 같은 폴더) ======
set "SCRIPT=%~dp0print_mat_paths.py"

REM ====== 입력(드래그&드롭) 체크 ======
if "%~1"=="" (
  echo Usage: Drag and drop an FBX onto this .bat
  pause
  exit /b 1
)

set "INFBX=%~1"
if not exist "%INFBX%" (
  echo ERROR: Input file not found: "%INFBX%"
  pause
  exit /b 1
)

if not exist "%SCRIPT%" (
  echo ERROR: Script not found: "%SCRIPT%"
  pause
  exit /b 1
)

REM ====== 출력 파일: mat_paths.txt (같은 폴더) ======
set "OUTTXT=mat_paths.txt"

echo Writing to: "%OUTTXT%"
echo.

REM 헤더(원하면 제거 가능)
echo ===== %DATE% %TIME% ===== >> "%OUTTXT%"
echo Input: "%INFBX%" >> "%OUTTXT%"
echo. >> "%OUTTXT%"

REM 실행 결과를 "추가(>>)"로 기록
"%BLENDER%" -b -P "%SCRIPT%" -- "%INFBX%" >> "%OUTTXT%" 2>&1

echo. >> "%OUTTXT%"
echo DONE. Appended output to: "%OUTTXT%"
endlocal
