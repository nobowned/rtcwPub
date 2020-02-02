This is a "Simply Enhanced" Vanilla 1.0 RTCW Client. Here are the differences:

/cg_muzzleFlash 0 or 1:
Disables or enables the flash shader around your gun's muzzle when shooting.

/cg_bloodFlash <0 to 1>:
Changes the intensity (0 = off) of the blood flash when you get shot.

/cg_damageNudge <0 to 1>:
Changes the intensity (0 = off) of the damage nudge/damage feedback that happens when you are shot

/cg_bloodDamageBlend <0 to 1>:
Similar to blood flash. Set this to zero as well if you want all blood from damage to disappear.

/cg_crosshairColor <0-7>:
Sets the color of crosshairs. The number to color mapping used is identical to name colors (i.e. 0 is black, 1 is red, etc..).
Note: This doesn't require a special crosshair shader anymore, the default crosshairs that come in mp_pak0.pk3 will work (it's hardcoded).

/cg_crosshairPulse 0 or 1:
Removes the pulsing of your crosshair (aim spread indicator).

/cg_drawRespawnTime 0 or 1:
Draws the reinforcement timer at all times while a round is in progress (don't have to open the scoreboard).

/cg_noTextChats 0 or 1:
Removes all text chat, team and non-team. (use cg_teamChatsOnly to remove global chat only).

/dropreload <key>:
Binds drop reload script to the specified key

Server list is now scrollable using the scrollwheel on your mouse. Simply hover your mouse over the server list and begin scrolling up/down.

The scoreboard while in-game (+scores/tab) now shows spectator ping.

All map downloads are attempted using HTTP unless the web server (currently www.x-labs.co.uk) doesn't have the map, then it will download it from the wolf server.