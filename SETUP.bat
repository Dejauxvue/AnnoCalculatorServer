@echo off
SETLOCAL EnableDelayedExpansion

echo.SETUP Git
git.exe lfs install
git.exe submodule update --init --recursive

echo.SETUP vcpkg
if not exist "cpp\vcpkg\vcpkg.exe" "cpp\vcpkg\bootstrap-vcpkg.bat"
"cpp\vcpkg\vcpkg.exe" integrate install

"link_assets.bat"