mkdir windows-server
mkdir windows-server\main
mkdir windows-server\dmscripts

copy ..\server-files\main windows-server\main
copy ..\server-files\dmscripts windows-server\dmscripts

echo WolfDED.exe +exec deathmatch.cfg > windows-server\start.bat
