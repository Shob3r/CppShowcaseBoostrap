@echo off
set nodeInstallDir=C:\Program Files\nodejs
set ToolsDir=%cd%

if exist %nodeInstallDir% (
	echo node installed!
	cd %appdata%\CppShowcase
	npm i 
	cmd %appdata%\CppShowcase\start.bat
	pause
) 

else (
	echo node not installed!
	powershell Expand-Archive -Path .\node.zip -DestinationPath .\node
	set CURR_DIR=%CD%\node
	setx PATH "%PATH%;%CURR_DIR%"
	refreshenv
	cd %appdata%\CppShowcase
	npm i
	cmd %appdata%\CppShowcase\start.bat
	pause
)
