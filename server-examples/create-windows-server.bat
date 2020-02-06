mkdir windows-server
mkdir windows-server\main
mkdir windows-server\main\dmscripts

copy server-files\main\*.cfg windows-server\main
copy server-files\main\dmscripts windows-server\main\dmscripts

echo WolfDED.exe +exec deathmatch.cfg > windows-server\start.bat
