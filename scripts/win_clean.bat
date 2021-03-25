@echo off

echo ##
echo ## CLEAN
echo ##

call %~dp0win_env.bat %1

rmdir %BUILD_PATH% /S /Q
rmdir %INSTALL_PATH% /S /Q
