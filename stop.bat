for /f "tokens=5" %%a in ('netstat -ano ^| findstr 127.0.0.1:3333') do (taskkill /f /pid %%a)
for /f "tokens=5" %%a in ('netstat -ano ^| findstr 0.0.0.0:8080') do (taskkill /f /pid %%a)
cmd /c "taskkill /f /im node.exe"
echo "--------------------------STOPPED---------------------"
