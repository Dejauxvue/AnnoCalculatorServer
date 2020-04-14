@echo off
SETLOCAL EnableDelayedExpansion

:: relative path from repository root to build directories
set "release_path=cpp\visual studio\x64\Release"
set "debug_path=cpp\visual studio\x64\Debug"

cd "%~dp0"
echo %cd%

echo.SETUP folders
set "rel_assets=%cd%\%release_path%"
echo %rel_assets%
if not exist %rel_assets% mkdir %rel_assets%
if not exist %rel_assets%\debug_images mkdir %rel_assets%\debug_images
set "dbg_assets=%cd%\%debug_path%"
if not exist %dbg_assets% mkdir %dbg_assets%
if not exist %dbg_assets%\debug_images mkdir %dbg_assets%\debug_images

echo.LINK assets recursively
call:link_asset_subfolders
call:iterate_externals
goto:eof

:link_asset_subfolders
if exist "assets" (
	for /d %%i in ("!cd!\assets\*") do ( 
		if not exist "!rel_assets!\%%~nxi" mklink /J "!rel_assets!\%%~nxi" "%%i"
		if not exist "!dbg_assets!\%%~nxi" mklink /J "!dbg_assets!\%%~nxi" "%%i"	
	)
)
goto:eof

:iterate_externals
if exist "externals" (
	for /d %%i in ("!cd!\externals\*") do ( 
		cd "%%i"
		call:link_asset_subfolders
		call:iterate_externals
		cd "..\.."
	)
)
goto:eof