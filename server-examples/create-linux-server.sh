mkdir linux-server
mkdir linux-server/main
mkdir linux-server/main/dmscripts

cp ../assets/server/main/*.* linux-server/main
cp ../assets/server/main/dmscripts/* linux-server/main/dmscripts

echo './wolfded.x86 +exec deathmatch.cfg' > linux-server/start.sh
