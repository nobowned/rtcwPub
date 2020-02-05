mkdir linux-server
mkdir linux-server/main
mkdir linux-server/dmscripts

cp ../server-files/main/* linux-server/main
cp ../server-files/dmscripts/* linux-server/dmscripts

echo './wolfded.x86 +exec deathmatch.cfg' > linux-server/start.sh
