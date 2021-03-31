mkdir windows-server
mkdir windows-server\main
mkdir windows-server\main\dmscripts

copy ..\assets\server\main\*.* windows-server\main
copy ..\assets\server\main\dmscripts windows-server\main\dmscripts

echo WolfDED.exe +exec deathmatch.cfg > windows-server\start.bat
