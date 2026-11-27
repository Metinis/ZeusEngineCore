@echo off
setlocal

rem Compile every .vert / .frag / .comp in this folder to SPIR-V (<name>.spv).
rem Runs from the script's own folder, so it works whatever the caller's directory.
cd /d "%~dp0"

rem Prefer the Vulkan SDK's copy; fall back to whatever is on PATH.
set "GLSLANG=glslangValidator"
if defined VULKAN_SDK if exist "%VULKAN_SDK%\Bin\glslangValidator.exe" set "GLSLANG=%VULKAN_SDK%\Bin\glslangValidator.exe"

set "FAILED=0"
for %%f in (*.vert *.frag *.comp) do (
    echo Compiling %%f
    "%GLSLANG%" -V "%%f" -o "%%f.spv"
    if errorlevel 1 (
        echo   FAILED: %%f
        set "FAILED=1"
    )
)

rem Non-zero exit makes the CMake build fail on a shader error instead of continuing.
exit /b %FAILED%