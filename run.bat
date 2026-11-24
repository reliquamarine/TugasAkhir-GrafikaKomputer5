@echo off
REM ============================================================
REM  Build & Run cepat - Abyss Explorer (tanpa Visual Studio)
REM  Klik dua kali file ini kalau F5 di Visual Studio bermasalah.
REM ============================================================
setlocal
pushd "%~dp0"

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto :novs

for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%i"
if not defined VSPATH goto :novs

call "%VSPATH%\VC\Auxiliary\Build\vcvarsall.bat" x86 >nul
if errorlevel 1 goto :novs

if not exist "Debug" mkdir "Debug"

echo [1/2] Mengompilasi main.cpp ...
cl /nologo /EHsc /std:c++20 /MD /I "freeglut" "main.cpp" /link /LIBPATH:"freeglut" freeglut.lib opengl32.lib glu32.lib /OUT:"Debug\AbyssExplorer.exe"
if errorlevel 1 goto :failed

echo [2/2] Menyalin freeglut.dll ...
copy /Y "freeglut\freeglut.dll" "Debug\" >nul

echo.
echo Berhasil! Menjalankan program...
start "" "%~dp0Debug\AbyssExplorer.exe"
popd
exit /b 0

:novs
echo Visual Studio C++ Build Tools tidak ditemukan.
echo Buka project lewat AbyssExplorer.sln di Visual Studio dan tekan F5.
popd
pause
exit /b 1

:failed
echo Kompilasi gagal. Periksa pesan error di atas.
popd
pause
exit /b 1
