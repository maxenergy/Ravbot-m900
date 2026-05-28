# Windows 手动启动 RavBot Gateway 脚本
# 使用说明：双击此文件或在命令行中运行

@echo off
chcp 65001 >nul
setlocal

echo.
echo ================================================
echo   RavBot Gateway - 手动启动脚本
echo ================================================
echo.

REM 查找 ravbot.exe
set RAVBOT_EXE=
if exist "%~dp0build\ravbot.exe" (
    set RAVBOT_EXE=%~dp0build\ravbot.exe
) else if exist "%~dp0build\Debug\ravbot.exe" (
    set RAVBOT_EXE=%~dp0build\Debug\ravbot.exe
) else if exist "%~dp0build\Release\ravbot.exe" (
    set RAVBOT_EXE=%~dp0build\Release\ravbot.exe
) else if exist "%USERPROFILE%\.ravbot\ravbot.exe" (
    set RAVBOT_EXE=%USERPROFILE%\.ravbot\ravbot.exe
)

if "%RAVBOT_EXE%"=="" (
    echo [错误] 未找�?ravbot.exe
    echo.
    echo 请确保已完成编译或安装：
    echo   cmake --build build --parallel
    pause
    exit /b 1
)

echo [信息] 找到 RavBot: %RAVBOT_EXE%
echo.

REM 创建日志目录
set LOG_DIR=%~dp0logs
if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"

REM 检查配置文�?set CONFIG_FILE=%USERPROFILE%\.ravbot\ravbot.json
if not exist "%CONFIG_FILE%" (
    echo [警告] 配置文件不存�? %CONFIG_FILE%
    echo 请先运行: ravbot onboard
    echo.
    pause
    exit /b 1
)

REM 启动 Gateway
echo [信息] 启动 Gateway (�?Ctrl+C 停止)...
echo [信息] 日志文件: %LOG_DIR%\gateway-manual.log
echo.
echo [%DATE% %TIME%] Gateway started manually >> "%LOG_DIR%\gateway-manual.log"

"%RAVBOT_EXE%" gateway run 2>&1 | powershell -NoProfile -Command "ForEach-Object { $_ | Tee-Object -FilePath '%LOG_DIR%\gateway-manual.log' -Append }; exit $LASTEXITCODE"

REM 检查退出代�?if %ERRORLEVEL% neq 0 (
    echo.
    echo [错误] Gateway 退出，代码: %ERRORLEVEL%
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [信息] Gateway 已停�?pause
