@echo off
set nodeInstallDir=C:\Program Files\nodejs
set ToolsDir=%cd%

if exist %nodeInstallDir% (
	cd %appdata%\CppShowcase
	npm i 
	exit
) 

else (
	powershell Expand-Archive -Path .\node.zip -DestinationPath .\node
	set CURR_DIR=%CD%\node
	setx PATH "%PATH%;%CURR_DIR%"
	refreshenv
	cd %appdata%\CppShowcase
	npm i
	exit
)
