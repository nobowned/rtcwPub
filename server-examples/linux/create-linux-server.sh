mkdir linux-server
mkdir linux-server/main
mkdir linux-server/main/dmscripts

cp ../server-files/main/*.cfg linux-server/main
cp ../server-files/main/dmscripts/* linux-server/main/dmscripts

echo './wolfded.x86 +exec deathmatch.cfg' > linux-server/start.sh
