@echo off
REM 使用 MSYS2 UCRT64 里的 g++
set COMPILER=C:\msys64\ucrt64\bin\g++.exe

echo [Build] Compiling main.c and draw.c ...
"%COMPILER%" main.c draw.c -o main.exe -lfreeglut -lopengl32 -lglu32

if errorlevel 1 (
    echo [Build] Failed.
) else (
    echo [Build] Succeeded. Output: main.exe
)

pause
