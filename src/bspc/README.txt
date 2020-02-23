This tool is primarily used to create aas files from a bsp. An aas file needs to be created for maps where you want to use the q3 bots.
If you find the single player .bsp files you'll notice they have an .aas file accompanying them. This is what allows the ai to pathfind in campaign.

How to generate a .aas file:
* Open bspc.sln
* Build Release
* The built bspc.exe will output to src/Release folder (not the src/bspc/Release folder)
* Grab your .bsp file. Chances are it's within a .pk3. 7zip is a quick way to open the .pk3 and copy only the .bsp you want out of it.
* Run the following command: bspc.exe -bsp2aas your_bsp.bsp -forcesidesvisible
* your_bsp.aas should be created along-side the bspc.exe
* Take your created .aas file and place it in the main/maps directory of your dedicated server

The very first time you run your dedicated server with that .aas file (and bots enabled) the server will have to generate an .rcd file.
It takes some time, but only has to happen once. It will freeze your server while it's generating.. that is expected.