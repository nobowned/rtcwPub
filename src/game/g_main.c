#include "g_local.h"

// L0 - (etPUB port) censored
wordDictionary censorDictionary;
wordDictionary censorNamesDictionary;

level_locals_t	level;

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
	int			modificationCount;  // for tracking changes
	qboolean	trackChange;	    // track this variable, and announce if changed
  qboolean teamShader;        // track and if changed, update shader state
} cvarTable_t;

gentity_t		g_entities[MAX_GENTITIES];
gclient_t		g_clients[MAX_CLIENTS];

gentity_t		*g_camEnt = NULL;	//----(SA)	script camera

// Rafael gameskill
extern int bg_pmove_gameskill_integer;
// done

vmCvar_t	g_gametype;

// Rafael gameskill
vmCvar_t	g_gameskill;
// done

vmCvar_t	g_dmflags;
vmCvar_t	g_fraglimit;
vmCvar_t	g_timelimit;
vmCvar_t	g_capturelimit;
vmCvar_t	g_friendlyFire;
vmCvar_t	g_password;
vmCvar_t	g_maxclients;
vmCvar_t	g_maxGameClients;
vmCvar_t	g_minGameClients;		// NERVE - SMF
vmCvar_t	g_dedicated;
vmCvar_t	g_speed;
vmCvar_t	g_gravity;
vmCvar_t	g_cheats;
vmCvar_t	g_knockback;
vmCvar_t	g_quadfactor;
vmCvar_t	g_forcerespawn;
vmCvar_t	g_inactivity;
vmCvar_t	g_debugMove;
vmCvar_t	g_debugDamage;
vmCvar_t	g_debugAlloc;
vmCvar_t	g_debugBullets;	//----(SA)	added
vmCvar_t	g_drawHitboxes;
vmCvar_t	g_preciseHeadHitBox;
vmCvar_t	g_headshotsOnly;
vmCvar_t	g_weaponRespawn;
vmCvar_t	g_motd;
vmCvar_t	g_syncronousClients;
vmCvar_t	g_warmup;

// NERVE - SMF
vmCvar_t	g_warmupLatch;
vmCvar_t	g_nextTimeLimit;
vmCvar_t	g_showHeadshotRatio;
vmCvar_t	g_userTimeLimit;
vmCvar_t	g_userAlliedRespawnTime;
vmCvar_t	g_userAxisRespawnTime;
vmCvar_t	g_currentRound;
vmCvar_t	g_noTeamSwitching;
vmCvar_t	g_altStopwatchMode;
vmCvar_t	g_gamestate;
vmCvar_t	g_swapteams;
// -NERVE - SMF

vmCvar_t	g_restarted;
vmCvar_t	g_log;
vmCvar_t	g_logSync;
vmCvar_t	g_podiumDist;
vmCvar_t	g_podiumDrop;
vmCvar_t	g_allowVote;
vmCvar_t	g_complaintlimit;		// DHM - Nerve
vmCvar_t	g_maxlives;				// DHM - Nerve
vmCvar_t	g_voiceChatsAllowed;	// DHM - Nerve

vmCvar_t	g_weaponTeamRespawn;
vmCvar_t	g_teamForceBalance;
vmCvar_t	g_listEntity;
vmCvar_t	g_rankings;
vmCvar_t	g_enableBreath;
vmCvar_t	g_smoothClients;
vmCvar_t	pmove_fixed;
vmCvar_t	pmove_msec;

// Rafael
vmCvar_t	g_autoactivate;
vmCvar_t	g_testPain;

vmCvar_t	g_missionStats;
vmCvar_t	ai_scriptName;		// name of AI script file to run (instead of default for that map)
vmCvar_t	g_scriptName;		// name of script file to run (instead of default for that map)

vmCvar_t	g_developer;

vmCvar_t	g_userAim;

vmCvar_t	g_forceModel;

vmCvar_t	g_mg42arc;

vmCvar_t	g_footstepAudibleRange;
// JPW NERVE multiplayer reinforcement times
vmCvar_t	g_redlimbotime;
vmCvar_t	g_bluelimbotime;
// charge times for character class special weapons
vmCvar_t	g_medicChargeTime;
vmCvar_t	g_engineerChargeTime;
vmCvar_t	g_LTChargeTime;
vmCvar_t	g_soldierChargeTime;
// screen shakey magnitude multiplier
vmCvar_t	sv_screenshake;
// jpw

// L0 - New stuff

// Admins
vmCvar_t	a1_pass;		// Level 1 admin
vmCvar_t	a2_pass;		// Level 2 admin
vmCvar_t	a3_pass;		// Level 3 admin
vmCvar_t	a4_pass;		// Level 4 admin
vmCvar_t	a5_pass;		// Level 4 admin
vmCvar_t	a1_tag;			// Level 1 admin tag
vmCvar_t	a2_tag;			// Level 2 admin tag
vmCvar_t	a3_tag;			// Level 3 admin tag
vmCvar_t	a4_tag;			// Level 3 admin tag
vmCvar_t	a5_tag;			// Level 5 admin tag
vmCvar_t	a1_message;		// Level 1 login message
vmCvar_t	a2_message;		// Level 2 login message
vmCvar_t	a3_message;		// Level 3 login message
vmCvar_t	a4_message;		// Level 4 login message
vmCvar_t	a5_message;		// Level 5 login message
vmCvar_t	a1_cmds;		// Level 1 admin commands
vmCvar_t	a2_cmds;		// Level 2 admin commands
vmCvar_t	a3_cmds;		// Level 3 admin commands
vmCvar_t	a4_cmds;		// Level 4 admin commands
vmCvar_t	a5_cmds;		// Level 5 admin commands
vmCvar_t	a5_allowAll;	// Allows level 5 to execute all admin commands + any other that's set in a5_cmds ->
							// In this case, use a5_cmds for server specific cvars like g_allowVote that would otherwise require rcon etc..

// Server Bot
vmCvar_t	sb_system;						// Controls all SB functionality
vmCvar_t	sb_tempban;						// Tempbans
vmCvar_t	sb_maxTKs;						// Max TK's before client gets kicked
vmCvar_t	sb_maxTKsTempbanMins;			// Minutes it bans for TK (0 = disabled)
vmCvar_t	sb_maxTeamBleed;				// Max bleeding pts before client gets kicked (1 hit = 1 point)
vmCvar_t	sb_maxTeamBleedTempbanMins;		// Minutes it bans for team bleeding (0 = disabled)
vmCvar_t	sb_minLowScore;					// Minimum score allowed before client gets kicked for low score
vmCvar_t	sb_minLowScoreTempbanMins;		// Minutes it bans for low score (0 = disabled)
vmCvar_t	sb_maxPingFlux;					// Top limit ping can hit before client gets kicked
vmCvar_t	sb_maxPingHits;					// How many seconds or times (1 time = 1 sec) can it peak above limit
vmCvar_t	sb_autoIgnore;					// Auto ignores players (for the round) that reach spam check more than 3 times.
vmCvar_t	sb_maxTempIgnores;				// Maximum amount of time the autoIgnore feature will temp-ignore. Once limit is reached player is perm-ignored
vmCvar_t	sb_censorPenalty;				// Auto ignores (1) or kicks (2) client after 4th warning for cursing..
vmCvar_t	sb_censorPentalityTempbanMin;	// Minutes it bans for censory kick (sb_censorPenality has to be 2), 0 = disabled.

// System
vmCvar_t	g_extendedLog;			// Logs various Admin and other related actions
vmCvar_t	g_maxVotes;				// Max votes client can call per round
vmCvar_t	g_disallowedVotes;		// Disalloved votes separeted by space..
vmCvar_t	g_voteConfigs;			// Configs that can be executed by vote
vmCvar_t	g_voteDelay;			// Time that has to pass before another vote can be called
vmCvar_t	g_antilag;				// Antilag
vmCvar_t	sv_hostname;			// So it's more accessible
vmCvar_t	g_bypassPasswords;		// Tokens separated by space for bypassing a ban
vmCvar_t	bannedMSG;				// Meesage printed to banned clients
vmCvar_t	g_ignoreSpecs;			// Ignores spectators - Admins can still bypass the ignore..
vmCvar_t	g_inactivityToSpecs;	// Puts inactive players in spectators instead of dropping them.
vmCvar_t	g_spectatorInactivity;	// Drop spectators after some time..
vmCvar_t	g_mapConfigs;			// Essentials for custom map configs...
vmCvar_t	g_autoSwap;				// Auto swaps teams 
vmCvar_t	g_autoSwapRounds;		// How many rounds until it auto swaps
vmCvar_t	g_autoShuffle;			// Auto shuffles teams after rounds set here | 0 = off
vmCvar_t	g_censorWords;			// Censored words
vmCvar_t	g_disallowedNames;		// Disallowed names
vmCvar_t	g_noHardcodedCensor;	// Don't use hard coded censor..
vmCvar_t	g_shortcuts;			// Enable shortcuts
vmCvar_t	g_allowPMs;				// Allow private messages
vmCvar_t	g_serverMessage;		// Shows a center print each time when player switches teams.
vmCvar_t	g_customSpawns;			// Spawn your own shit
vmCvar_t	g_teamSwitchTime;		// Time in seconds before player can switch teams
vmCvar_t	g_readySystem;			// Enable/disable the /ready system for players
vmCvar_t	g_readyPlayers;			// The amount of total players required to /ready before the round will start.

// Game 
vmCvar_t	g_dropReload;			// Enable / Disable Drop reload
vmCvar_t	g_dropClips;			// Enable dropping extra clips with gun
vmCvar_t	g_unlockWeapons;		// Enable weapon dropping..
vmCvar_t	g_tapReports;			// Print tap outs / 0 = off, 1 = prints to everyone, 2 = prints to team only
vmCvar_t	g_gibReports;			// Prints Gib reports
vmCvar_t	g_reviveReports;		// Revive reports. 0 = 0ff, 1 = prints to everyone, 2 = prints to team only.
vmCvar_t	g_fastStabSound;		// 0 = off, 1 = (OSP's) goat sound, 2 = humiliation sound, 3 = random between 1 or 2
vmCvar_t	g_showLifeStats;		// If enabled it prints some life info to player when (s)he dies
vmCvar_t	g_chicken;				// Check if player is 'laming' out of combat with self kill..
vmCvar_t	g_poison;				// Poison... anything above 0 is value that will be used
vmCvar_t	g_hitsounds;			// Hitsounds - Requires soundpack
vmCvar_t	g_screenShake;			// Screenshaking on explosions (4 = default, 2 = half.. etc)
vmCvar_t	g_fixedphysics;			// Tries to keep things more fair..
vmCvar_t	g_printMatchInfo;		// Prints events when they happen (retake, obj planted..)
vmCvar_t	g_teamAutoBalance;		// Auto balances the teams..
vmCvar_t	g_warmupDamage;			// Allows damage in warmup
vmCvar_t	g_crouchStaminaBoost;			// If enabled it recharges stamina faster when player is crouching..
vmCvar_t	g_bunnyJump;			// Enable bunny jumps...
vmCvar_t	g_dragBodies;			// If enabled player can drag bodies using salute or activate command
vmCvar_t	g_shove;				// Enables or disables shove..(pushing of players..)
vmCvar_t	g_dropObj;				// Drop Objective
vmCvar_t	g_easyASBlock;			// If enabled it prints when AirStrike is blocked
vmCvar_t	g_LTinfoMsg;			// Shows how much ammo client player is pointing at has
vmCvar_t	g_disableInv;			// Disables (re)spawn/revive protection crap when one shoots..
vmCvar_t	g_axisSpawnProtectionTime;		// How long Axis player is invulrable when (s)he spawns.
vmCvar_t	g_alliedSpawnProtectionTime;	// How long Allied player is invulrable when (s)he spawns.
vmCvar_t	g_fastres;				// Xian's Fast (animation) restoring on revives
vmCvar_t	g_fastResMsec;			// Xian's Fast (animation) restoring on revives
vmCvar_t	g_axisASdelay;			// Limit for Axis Air strikes
vmCvar_t	g_axisASdelayFFE;		// Limit for Axis Air strikes (Firing for effect)
vmCvar_t	g_alliedASdelay;		// Limit for Allied Air strikes
vmCvar_t	g_alliedASdelayFFE;		// Limit for Allied Air strikes (Firing for effect)
vmCvar_t	g_smokeGrenades;		// Smoke Grenades for Leut
vmCvar_t	g_smokeGrenadesLmt;		// Smoke Grenades limit per life
vmCvar_t	g_flagRetake;			// How many times flag can be retaken
vmCvar_t	g_balanceFlagRetake;	// Checks if flag can be taken..it always allows team with less to claim it last and then locks till even.
vmCvar_t	g_balanceFlagCanClaim;	// If enabled team will be able to claim (not reclaim!) the flag regardless if they're the team with more..
vmCvar_t	g_alliedmaxlives;		// Xian -> 1.4 port
vmCvar_t	g_axismaxlives;			// Xian'-> 1.4 port
vmCvar_t	g_savedLocations;		// Enables saving locations, primarily used for TJ.
vmCvar_t	g_playersLeftTime;      // Time before the "Players Left:" message is displayed (seconds)
vmCvar_t	g_alternatePing;		// Shows an alternate ping calculated using usercmd packets.
vmCvar_t	g_unstuckRevive;		// Enables/disables the unstucking after getting stuck after a revive.
vmCvar_t	g_forceClass;			// Force this class.. -1 = None, 0 = Soldier, 1 = Medic, 2 = Eng, 3 = LT
vmCvar_t	g_automg42;				// Amount of automg42s allowed per team.
vmCvar_t	g_automg42LightIntensity; // Automg42 light intensity (how bright it is).
vmCvar_t	g_automg42Health;		// Automg42 health
vmCvar_t	g_automg42Disable;		// Should the automg42 be disabled before destroyed?
vmCvar_t	g_automg42TowerCollapse;		// Should automg42 towers collapse?
vmCvar_t	g_archiveLogDay;		// Archive console.log file at this day in the month (1-31).
vmCvar_t	g_useSpawnAnglesAfterRevive;	// Use spawn angles after revive (kinda random feeling, but OG behavior).
vmCvar_t	g_giveLife;				// Allows clients to give each other lives.
vmCvar_t	g_giveLifeRequiredDamage;		// Damage required before a life can be given.
vmCvar_t	g_giveLifeRequiredRevives;		// Revives required before a life can be given.

// Modes
vmCvar_t	g_deathmatch;			// Death Match
vmCvar_t	g_ffa;					// FFA gametype.. TODO: Actually make it a gametype.

// Weapon
vmCvar_t	g_dropHealth;		// The number of medpacks medic will drop when going to limbo
vmCvar_t	g_dropNades;		// The number of grenades eng will drop when going to limbo
vmCvar_t	g_dropAmmo;			// The number of ammo packs leut drops when going to limbo
vmCvar_t	g_dropAmmoPack;		// Gib drops ammo pack instead of weapon
vmCvar_t	g_throwKnives;		// 0 = disabled, anything else is the value of knives player gets, alt -1 = unlimited.
vmCvar_t	g_throwingKnifeDamage;		// Damage throwing knives do
vmCvar_t	g_ltNades;			// Number of nades a lt starts with 
vmCvar_t	g_medicNades;		// Number of nades a med starts with 
vmCvar_t	g_soldNades;		// Number of nades sold starts with
vmCvar_t	g_engNades;			// Number of nades eng starts with
vmCvar_t	g_syringes;
vmCvar_t	g_medicClips;		// Number of clips in weapon med starts with 
vmCvar_t	g_engineerClips;	// Number of clips in weapon eng starts with
vmCvar_t	g_soldierClips;		// Number of clips in weapon sold starts with
vmCvar_t	g_ltClips;		// Number of clips in weapon leut starts with
vmCvar_t	g_pistolClips;		// How many clips pistol gets
vmCvar_t	g_venomClips;		// How many clips venom starts with
vmCvar_t	g_mauserClips;		// How many clips mauser starts with
vmCvar_t	g_maxTeamPF;		// Max Pf's per team
vmCvar_t	g_maxTeamSniper;	// Max snipers per team
vmCvar_t	g_maxTeamVenom;		// Max venoms per team
vmCvar_t	g_maxTeamFlamer;	// Max flamers per team
vmCvar_t	g_balancePF;		// Some basic weapon balancing
vmCvar_t	g_balanceSniper;	// Some basic weapon balancing
vmCvar_t	g_balanceVenom;		// Some basic weapon balancing
vmCvar_t	g_balanceFlamer;	// Some basic weapon balancing
vmCvar_t	g_customMGs;		// Allow custom mg's so player can chose with what weapon to spawn with.
vmCvar_t	g_allowUnderwater;	// Weapons separated by string
vmCvar_t	g_clampPacketAngles; // If 1 medpacks launch at level plane, otherwise they launch based on client angles
vmCvar_t	g_ammoRegen;		// How much ammo will regenerate every second for weapon that is in use (any weapon atm)
vmCvar_t	g_lockPrimary;		// Primary weapon cannot be dropped to the ground. Dropping will initiate something identical to the "insta-drop reload script"
vmCvar_t	g_delayedPickup;	// Time to delay pickup for clients that didn't drop the weapon.
vmCvar_t	g_stickyGrenades;	// Grenades stick to shit once impact
vmCvar_t	g_stenMaxHeat;		// Heat at which sten will overheat
vmCvar_t	g_stenCoolRate;		// Rate at which sten will cool down
vmCvar_t	g_medicAmmoPack;	// Medics have ammo packs. NOTE: This forces g_LTChargeTime to be g_medicChargeTime * 4. Otherwise animation break client-side.
vmCvar_t	g_ammoPackGivesWeapon; // If no primary weapon (lost it or w/e) then ammo pack will give it back.

// Stats
vmCvar_t	g_doubleKills;			// Double, tripple & quad kills
vmCvar_t	g_killingSprees;		// Killing sprees for each 5/10 kill..
vmCvar_t	g_deathSprees;			// Death spress
vmCvar_t	g_showFirstHeadshot;	// Show who done it
vmCvar_t	g_showFirstBlood;		// Show who done it
vmCvar_t	g_showLastBlood;		// Prints in console at the end of the match
vmCvar_t	g_mapStats;				// Top records for each map.
vmCvar_t	g_mapStatsNotify;		// Notifies when record gets broken (during intermission)
vmCvar_t	g_roundStats;			// Prints high achievers each round
vmCvar_t	g_excludedRoundStats;	// List of excluded stats (not tracked and not printed)
vmCvar_t	g_dailystats;			// Today's stats
vmCvar_t	g_dailyMinimumKills;	// Minimum kills until you are "ranked" in daily stats
vmCvar_t	g_dailyStatsResetHour;  // Hour the stats will reset. Valid hours are 0-23. 0 = midnight.

// Level messages
vmCvar_t	g_showMsgs;			// Enable g_msgs
vmCvar_t	g_msg1;				// MESSAGE 1
vmCvar_t	g_msg2;				// MESSAGE 2
vmCvar_t	g_msg3;				// MESSAGE 3
vmCvar_t	g_msg4;				// MESSAGE 4
vmCvar_t	g_msg5;				// MESSAGE 5
vmCvar_t	g_msg6;				// MESSAGE 6
vmCvar_t	g_msg7;				// MESSAGE 7
vmCvar_t	g_msg8;				// MESSAGE 8
vmCvar_t	g_msg9;				// MESSAGE 9
vmCvar_t	g_msg10;			// MESSAGE 10
vmCvar_t	g_msg11;			// MESSAGE 11
vmCvar_t	g_msg12;			// MESSAGE 12
vmCvar_t	g_msgTime;			// Time between each message
vmCvar_t	g_msgStartTime;		// Time until messages start churnin

// Static
vmCvar_t	g_swapCounter;		// Count times so it auto swaps once it reaches it..
vmCvar_t	shuffleTracking;	// Tracks rounds for (auto) shuffle
vmCvar_t	needsBalance;		// Flag for auto balance check
vmCvar_t	needsDailyMsg;		// Set to 1 if the daily reset msg needs to be printed
vmCvar_t	latchedWarmup;		// This is needed! Don't fuck with it!
vmCvar_t	axisPlayers;		// axisPlayers.integer weapon balance fix
vmCvar_t	alliedPlayers;		// alliedPlayers.integer weapon balance fix

// Forced cvars
vmCvar_t	cl_allowdownload;		// Map downloading 
vmCvar_t	r_znear;				// Fix wall glitching
vmCvar_t	rate;					// Most people have broadband now...
vmCvar_t	snaps;					// To sort 1000 issues..
vmCvar_t	cl_anonymous;			// Useless..
vmCvar_t	r_mapoverbrightbits;	// Force 2..
vmCvar_t	r_vertexLight;			// Fixes bug on certain maps (hit shader)
vmCvar_t	r_depthbits;			// Fix bug where players can see through walls with this at 16
vmCvar_t	cl_timenudge;			// Yeah right...let them teleport in star trek not in rtcw..
vmCvar_t	com_maxfps;				// Ppl keep saying they can't handle 125.. yeah well... it's 2009, they can play tetris then...
vmCvar_t	cl_packetdup;			// The days of 56k modem are over...all should be able to use 1...
vmCvar_t	cl_maxpackets;			// Again... no more warping and crap...

// End

cvarTable_t		gameCvarTable[] = {
	// don't override the cheat state set by the system
	{ &g_cheats, "sv_cheats", "", 0, qfalse },

	// noset vars
	{ NULL, "gamename", GAMEVERSION, CVAR_SERVERINFO | CVAR_ROM, 0, qfalse },
	{ NULL, "gamedate", __DATE__, CVAR_ROM, 0, qfalse },
	{ &g_restarted, "g_restarted", "0", CVAR_ROM, 0, qfalse },
	{ NULL, "sv_mapname", "", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse },

	// latched vars
	// DHM - Nerve :: default to GT_WOLF
	{ &g_gametype, "g_gametype", "5", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse },

	// Rafael gameskill
	{ &g_gameskill, "g_gameskill", "3", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse },
	// done

	// JPW NERVE multiplayer stuffs
	{ &sv_screenshake, "sv_screenshake", "5", CVAR_ARCHIVE, 0, qfalse },
	{ &g_redlimbotime, "g_redlimbotime", "30000", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse },
	{ &g_bluelimbotime, "g_bluelimbotime", "30000", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse },
	{ &g_medicChargeTime, "g_medicChargeTime", "45000", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse },
	{ &g_engineerChargeTime, "g_engineerChargeTime", "30000", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse },
	{ &g_LTChargeTime, "g_LTChargeTime", "40000", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse },
	{ &g_soldierChargeTime, "g_soldierChargeTime", "20000", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse },
	// jpw

	{ &g_maxclients, "sv_maxclients", "20", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse },			// NERVE - SMF - made 20 from 8
	{ &g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse },
	{ &g_minGameClients, "g_minGameClients", "8", CVAR_SERVERINFO, 0, qfalse },								// NERVE - SMF

	// change anytime vars
	{ &g_dmflags, "dmflags", "0", /*CVAR_SERVERINFO |*/ CVAR_ARCHIVE, 0, qtrue },
	{ &g_fraglimit, "fraglimit", "0", /*CVAR_SERVERINFO |*/ CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_timelimit, "timelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_capturelimit, "capturelimit", "8", /*CVAR_SERVERINFO |*/ CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },

	{ &g_syncronousClients, "g_syncronousClients", "0", CVAR_SYSTEMINFO, 0, qfalse },

	{ &g_friendlyFire, "g_friendlyFire", "1", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qtrue },

	{ &g_teamForceBalance, "g_teamForceBalance", "0", CVAR_ARCHIVE },							// NERVE - SMF - merge from team arena

	{ &g_warmup, "g_warmup", "20", CVAR_ARCHIVE, 0, qfalse },

	// NERVE - SMF
	{ &g_warmupLatch, "g_warmupLatch", "1", 0, 0, qfalse },

	{ &g_nextTimeLimit, "g_nextTimeLimit", "0", CVAR_WOLFINFO, 0, qfalse },
	{ &g_currentRound, "g_currentRound", "0", CVAR_WOLFINFO, 0, qfalse },
	{ &g_altStopwatchMode, "g_altStopwatchMode", "0", CVAR_ARCHIVE, 0, qtrue },
	{ &g_gamestate, "gamestate", "-1", CVAR_WOLFINFO | CVAR_ROM, 0, qfalse },

	{ &g_noTeamSwitching, "g_noTeamSwitching", "0", CVAR_ARCHIVE, 0, qtrue },

	{ &g_showHeadshotRatio, "g_showHeadshotRatio", "0", 0, 0, qfalse },

	{ &g_userTimeLimit, "g_userTimeLimit", "0", 0, 0, qfalse },
	{ &g_userAlliedRespawnTime, "g_userAlliedRespawnTime", "0", 0, 0, qfalse },
	{ &g_userAxisRespawnTime, "g_userAxisRespawnTime", "0", 0, 0, qfalse },

	{ &g_swapteams, "g_swapteams", "0", CVAR_ROM, 0, qfalse },
	// -NERVE - SMF

	{ &g_log, "g_log", "", CVAR_ARCHIVE, 0, qfalse },
	{ &g_logSync, "g_logSync", "0", CVAR_ARCHIVE, 0, qfalse },

	{ &g_password, "g_password", "", CVAR_USERINFO, 0, qfalse },

	{ &g_dedicated, "dedicated", "0", 0, 0, qfalse },

	{ &g_speed, "g_speed", "320", 0, 0, qtrue },
	{ &g_gravity, "g_gravity", "800", 0, 0, qtrue },
	{ &g_knockback, "g_knockback", "100", 0, 0, qtrue },
	{ &g_quadfactor, "g_quadfactor", "3", 0, 0, qtrue },
	{ &g_weaponRespawn, "g_weaponrespawn", "5", 0, 0, qtrue },
	{ &g_weaponTeamRespawn, "g_weaponTeamRespawn", "30", 0, 0, qtrue },
	{ &g_forcerespawn, "g_forcerespawn", "0", 0, 0, qtrue },
	{ &g_inactivity, "g_inactivity", "0", 0, 0, qtrue },
	{ &g_debugMove, "g_debugMove", "0", 0, 0, qfalse },
	{ &g_debugDamage, "g_debugDamage", "0", 0, 0, qfalse },
	{ &g_debugAlloc, "g_debugAlloc", "0", 0, 0, qfalse },
	{ &g_debugBullets, "g_debugBullets", "0", 0, 0, qfalse },	//----(SA)	added
	{ &g_drawHitboxes, "g_drawHitboxes", "0", 0, 0, qfalse },
	{ &g_preciseHeadHitBox, "g_preciseHeadHitBox", "0", 0, 0, qfalse },
	{ &g_headshotsOnly, "g_headshotsOnly", "0", 0, 0, qfalse },
	{ &g_motd, "g_motd", "", 0, 0, qfalse },

	{ &g_podiumDist, "g_podiumDist", "80", 0, 0, qfalse },
	{ &g_podiumDrop, "g_podiumDrop", "70", 0, 0, qfalse },

	{ &g_allowVote, "g_allowVote", "1", 0, 0, qfalse },
	{ &g_listEntity, "g_listEntity", "0", 0, 0, qfalse },

	{ &g_complaintlimit, "g_complaintlimit", "3", CVAR_ARCHIVE, 0, qtrue },						// DHM - Nerve
	{ &g_maxlives, "g_maxlives", "0", CVAR_ARCHIVE | CVAR_LATCH | CVAR_SERVERINFO, 0, qtrue },		// DHM - Nerve
	{ &g_voiceChatsAllowed, "g_voiceChatsAllowed", "4", CVAR_ARCHIVE, 0, qfalse },				// DHM - Nerve

	{ &g_enableBreath, "g_enableBreath", "1", CVAR_SERVERINFO, 0, qtrue },
	{ &g_testPain, "g_testPain", "0", CVAR_CHEAT, 0, qfalse },
	{ &g_missionStats, "g_missionStats", "0", CVAR_ROM, 0, qfalse },
	{ &g_developer, "developer", "0", CVAR_TEMP, 0, qfalse },
	{ &g_rankings, "g_rankings", "0", 0, 0, qfalse },
	{ &g_userAim, "g_userAim", "1", CVAR_CHEAT, 0, qfalse },

	{ &g_smoothClients, "g_smoothClients", "1", 0, 0, qfalse },
	{ &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, 0, qfalse },
	{ &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0, qfalse },

	{ &g_mg42arc, "g_mg42arc", "0", CVAR_TEMP, 0, qfalse },

	{ &g_footstepAudibleRange, "g_footstepAudibleRange", "256", CVAR_CHEAT, 0, qfalse },

// L0 - New cvars

	// Admins	
	{ &a1_pass, "a1_pass", "none", CVAR_ARCHIVE, 0, qfalse },
	{ &a2_pass, "a2_pass", "none", CVAR_ARCHIVE, 0, qfalse },
	{ &a3_pass, "a3_pass", "none", CVAR_ARCHIVE, 0, qfalse },
	{ &a4_pass, "a4_pass", "none", CVAR_ARCHIVE, 0, qfalse },
	{ &a5_pass, "a5_pass", "none", CVAR_ARCHIVE, 0, qfalse },
	{ &a1_tag, "a1_tag", "^2Referee", CVAR_ARCHIVE, 0, qfalse },
	{ &a2_tag, "a2_tag", "^1Member", CVAR_ARCHIVE, 0, qfalse },
	{ &a3_tag, "a3_tag", "^3Admin", CVAR_ARCHIVE, 0, qfalse },
	{ &a4_tag, "a4_tag", "^5Admin", CVAR_ARCHIVE, 0, qfalse },
	{ &a5_tag, "a5_tag", "^0Admin", CVAR_ARCHIVE, 0, qfalse },
	{ &a1_message, "a1_message", "", CVAR_ARCHIVE, 0, qfalse },
	{ &a2_message, "a2_message", "", CVAR_ARCHIVE, 0, qfalse },
	{ &a3_message, "a3_message", "", CVAR_ARCHIVE, 0, qfalse },
	{ &a4_message, "a4_message", "", CVAR_ARCHIVE, 0, qfalse },
	{ &a5_message, "a5_message", "", CVAR_ARCHIVE, 0, qfalse },
	{ &a1_cmds, "a1_cmds", "", CVAR_ARCHIVE, 0, qfalse },
	{ &a2_cmds, "a2_cmds", "", CVAR_ARCHIVE, 0, qfalse },
	{ &a3_cmds, "a3_cmds", "", CVAR_ARCHIVE, 0, qfalse },
	{ &a4_cmds, "a4_cmds", "", CVAR_ARCHIVE, 0, qfalse },
	{ &a5_cmds, "a5_cmds", "", CVAR_ARCHIVE, 0, qfalse },
	{ &a5_allowAll, "a5_allowAll", "0", CVAR_ARCHIVE, 0, qfalse },

	// ServerBot
	{ &sb_system, "sb_system", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },	
	{ &sb_maxTKs, "sb_maxTKs", "-1", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &sb_maxTKsTempbanMins, "sb_maxTKsTempbanMins", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &sb_maxTeamBleed, "sb_maxTeamBleed", "-1", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &sb_maxTeamBleedTempbanMins, "sb_maxTeamBleedTempbanMins", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &sb_minLowScore, "sb_minLowScore", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &sb_minLowScoreTempbanMins, "sb_minLowScoreTempbanMins", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &sb_maxPingFlux, "sb_maxPingFlux", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &sb_maxPingHits, "sb_maxPingHits", "30", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &sb_autoIgnore, "sb_autoIgnore", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &sb_maxTempIgnores, "sb_maxTempIgnores", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &sb_censorPenalty, "sb_censorPenalty", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &sb_censorPentalityTempbanMin, "sb_censorPentalityTempbanMin", "0", CVAR_ARCHIVE, 0, qfalse },

	// System
	{ &g_extendedLog, "g_extendedLog", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &g_maxVotes, "g_maxVotes", "3", CVAR_ARCHIVE, 0, qfalse },
	{ &g_disallowedVotes, "g_disallowedVotes", "", CVAR_ARCHIVE, qfalse },
	{ &g_voteConfigs, "g_voteConfigs", "", CVAR_ARCHIVE, qfalse },
	{ &g_voteDelay, "g_voteDelay", "30", CVAR_ARCHIVE, 0, qfalse },
	{ &g_antilag, "g_antilag", "1", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qtrue },	
	{ &sv_hostname, "sv_hostname", "", CVAR_SERVERINFO, 0, qfalse },
	{ &g_bypassPasswords, "g_bypassPasswords", "", CVAR_ARCHIVE, 0, qfalse },
	{ &bannedMSG, "bannedMSG", "^7You are ^1Banned^7 from this server^1!", CVAR_ARCHIVE, 0, qfalse},
	{ &g_ignoreSpecs, "g_ignoreSpecs", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_inactivityToSpecs, "g_inactivityToSpecs", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &g_spectatorInactivity, "g_spectatorInactivity", "0", CVAR_ARCHIVE, 0, qtrue },
	{ &g_mapConfigs, "g_mapConfigs", "0", CVAR_LATCH, 0, qfalse },	
	{ &g_autoSwap, "g_autoSwap", "0", CVAR_ARCHIVE, 0, qtrue },
	{ &g_autoSwapRounds, "g_autoSwapRounds", "1", CVAR_ARCHIVE, 0, qtrue },
	{ &g_autoShuffle, "g_autoShuffle", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_censorWords, "g_censorWords", "fuuck", 0 },
	{ &g_disallowedNames, "g_disallowedNames", "nazi, admin, console", 0 },
	{ &g_noHardcodedCensor, "g_noHardcodedCensor", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_shortcuts, "g_shortcuts", "0", 0 },
	{ &g_allowPMs, "g_allowPMs", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_axisSpawnProtectionTime, "g_axisSpawnProtectionTime", "3000", CVAR_ARCHIVE, 0, qfalse },
	{ &g_alliedSpawnProtectionTime, "g_alliedSpawnProtectionTime", "3000", CVAR_ARCHIVE, 0, qfalse },
	{ &g_fastres, "g_fastres", "0", CVAR_ARCHIVE, 0, qtrue },
	{ &g_fastResMsec, "g_fastResMsec", "1600", CVAR_ARCHIVE, 0, qtrue },
	{ &g_axisASdelay, "g_axisASdelay", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_axisASdelayFFE, "g_axisASdelayFFE", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_alliedASdelay, "g_alliedASdelay", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_alliedASdelayFFE, "g_alliedASdelayFFE", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_serverMessage, "g_serverMessage", "", CVAR_ARCHIVE, 0, qfalse },
	{ &g_unstuckRevive, "g_unstuckRevive", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_customSpawns, "g_customSpawns", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_teamSwitchTime, "g_teamSwitchTime", "2", 0, 0, qfalse },
	{ &g_readySystem, "g_readySystem", "0", 0, 0, qfalse },
	{ &g_readyPlayers, "g_readySystem", "0", 0, 0, qfalse },
	{ &g_useSpawnAnglesAfterRevive, "g_useSpawnAnglesAfterRevive", "0", 0, 0, qfalse },
	{ &g_giveLife, "g_giveLife", "0", 0, 0, qfalse },
	{ &g_giveLifeRequiredDamage, "g_giveLifeRequiredDamage", "0", 0, 0, qfalse },
	{ &g_giveLifeRequiredRevives, "g_giveLifeRequiredRevives", "0", 0, 0, qfalse },

	// Modes
	{ &g_deathmatch, "g_deathmatch", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_ffa, "g_ffa", "0", CVAR_ARCHIVE | CVAR_LATCH, qtrue },
	{ &g_forceClass, "g_forceClass", "-1", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qtrue },

	// Game
	{ &g_dropReload, "g_dropReload", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_dropClips, "g_dropClips", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_unlockWeapons, "g_unlockWeapons", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_tapReports, "g_tapReports", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_gibReports, "g_gibReports", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_reviveReports, "g_reviveReports", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_fastStabSound, "g_fastStabSound", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_showLifeStats, "g_showLifeStats", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_chicken, "g_chicken", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_poison, "g_poison", "0", CVAR_ARCHIVE, 0, qtrue },
	{ &g_hitsounds, "g_hitsounds", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_screenShake, "g_screenShake", "2", CVAR_ARCHIVE, 0, qtrue },
	{ &g_fixedphysics, "g_fixedphysics", "1", CVAR_ARCHIVE | CVAR_SERVERINFO },
	{ &g_printMatchInfo, "g_printMatchInfo", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &g_teamAutoBalance, "g_teamAutoBalance", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_warmupDamage, "g_warmupDamage", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_crouchStaminaBoost, "g_crouchStaminaBoost", "3000", CVAR_ARCHIVE, 0, qfalse },
	{ &g_bunnyJump, "g_bunnyJump", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_dragBodies, "g_dragBodies", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_shove, "g_shove", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_dropObj, "g_dropObj", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_easyASBlock, "g_easyASBlock", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_LTinfoMsg, "g_LTinfoMsg", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_disableInv, "g_disableInv", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_smokeGrenades, "g_smokeGrenades", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_smokeGrenadesLmt, "g_smokeGrenadesLmt", "4", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_flagRetake, "g_flagRetake", "-1", CVAR_ARCHIVE, 0, qfalse },
	{ &g_balanceFlagRetake, "g_balanceFlagRetake", "0", CVAR_ARCHIVE, 0, qtrue },
	{ &g_balanceFlagCanClaim, "g_balanceFlagCanClaim", "0", CVAR_ARCHIVE, 0, qtrue },
	{ &g_alliedmaxlives, "g_alliedmaxlives", "0", CVAR_LATCH | CVAR_SERVERINFO, 0, qtrue },
	{ &g_axismaxlives, "g_axismaxlives", "0", CVAR_LATCH | CVAR_SERVERINFO, 0, qtrue },
	{ &g_savedLocations, "g_savedLocations", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_playersLeftTime, "g_playersLeftTime", "30", CVAR_ARCHIVE, 0, qfalse },
	{ &g_alternatePing, "g_alternatePing", "1", CVAR_LATCH, qfalse },
	{ &g_automg42, "g_automg42", "0", CVAR_LATCH, qfalse },
	{ &g_automg42LightIntensity, "g_automg42LightIntensity", "50", CVAR_LATCH, qfalse },
	{ &g_automg42Health, "g_automg42Health", STRINGIFYVAL(MG42_MULTIPLAYER_HEALTH), CVAR_LATCH, qfalse },
	{ &g_automg42Disable, "g_automg42Disable", "1", CVAR_LATCH, qfalse },
	{ &g_automg42TowerCollapse, "g_automg42TowerCollapse", "1", CVAR_LATCH, qfalse },
	{ &g_archiveLogDay, "g_archiveLogDay", "0", CVAR_LATCH, qfalse },

	// Weapon
	{ &g_dropHealth, "g_dropHealth", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_dropNades, "g_dropNades", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_dropAmmo, "g_dropAmmo", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_dropAmmoPack, "g_dropAmmoPack", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_throwKnives, "g_throwKnives", "0", CVAR_ARCHIVE, 0, qtrue },
	{ &g_throwingKnifeDamage, "g_throwingKnifeDamage", "20", CVAR_ARCHIVE, 0, qtrue },
	{ &g_ltNades, "g_ltNades", "1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_medicNades, "g_medicNades", "1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_engNades, "g_engNades", "8", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_soldNades, "g_soldNades", "4", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_syringes, "g_syringes", "10", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_medicClips, "g_medicClips", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_engineerClips, "g_engineerClips", "1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_soldierClips, "g_soldierClips", "2", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_ltClips, "g_ltClips", "1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_pistolClips, "g_pistolClips", "8", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_venomClips, "g_venomClips", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_mauserClips, "g_mauserClips", "1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_maxTeamPF, "g_maxTeamPF", "-1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_maxTeamSniper, "g_maxTeamSniper", "-1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_maxTeamVenom, "g_maxTeamVenom", "-1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_maxTeamFlamer, "g_maxTeamFlamer", "-1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_balancePF, "g_balancePF", "0", CVAR_ARCHIVE, 0, qtrue },
	{ &g_balanceSniper, "g_balanceSniper", "0", CVAR_ARCHIVE, 0, qtrue },
	{ &g_balanceVenom, "g_balanceVenom", "0", CVAR_ARCHIVE, 0, qtrue },
	{ &g_balanceFlamer, "g_balanceFlamer", "0", CVAR_ARCHIVE, 0, qtrue },
	{ &g_customMGs, "g_customMGs", "0", CVAR_ARCHIVE, 0, qtrue },
	{ &g_allowUnderwater, "g_allowUnderwater", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_clampPacketAngles, "g_clampPacketAngles", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &g_ammoRegen, "g_ammoRegen", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_lockPrimary, "g_lockPrimary", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_delayedPickup, "g_delayedPickup", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_stickyGrenades, "g_stickyGrenades", "0", 0, 0, qtrue },
	{ &g_stenMaxHeat, "g_stenMaxHeat", "700", CVAR_LATCH, 0, qtrue },
	{ &g_stenCoolRate, "g_stenCoolRate", "300", CVAR_LATCH, 0, qtrue },
	{ &g_medicAmmoPack, "g_medicAmmoPack", "0", CVAR_LATCH, 0, qtrue },
	{ &g_ammoPackGivesWeapon, "g_ammoPackGivesWeapon", "0", CVAR_LATCH, 0, qtrue },

	// Stats
	{ &g_doubleKills, "g_doubleKills", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_killingSprees, "g_killingSprees", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_deathSprees, "g_deathSprees", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_showFirstHeadshot, "g_showFirstHeadshot", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_showFirstBlood, "g_showFirstBlood", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_showLastBlood, "g_showLastBlood", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_mapStats, "g_mapStats", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },		
	{ &g_roundStats, "g_roundStats", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_excludedRoundStats, "g_excludedRoundStats", "", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_dailystats, "g_dailystats", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_dailyMinimumKills, "g_dailyMinimumKills", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_dailyStatsResetHour, "g_dailyStatsResetHour", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },

	// Level messages	
	{ &g_showMsgs, "g_showMsgs", "0", 0, 0, qfalse },
	{ &g_msg1, "g_msg1", "", 0, 0, qfalse },
	{ &g_msg2, "g_msg2", "", 0, 0, qfalse },
	{ &g_msg3, "g_msg3", "", 0, 0, qfalse },
	{ &g_msg4, "g_msg4", "", 0, 0, qfalse },
	{ &g_msg5, "g_msg5", "", 0, 0, qfalse },
	{ &g_msg6, "g_msg6", "", 0, 0, qfalse },
	{ &g_msg7, "g_msg7", "", 0, 0, qfalse },
	{ &g_msg8, "g_msg8", "", 0, 0, qfalse },
	{ &g_msg9, "g_msg9", "", 0, 0, qfalse },
	{ &g_msg10, "g_msg10", "", 0, 0, qfalse },
	{ &g_msg11, "g_msg11", "", 0, 0, qfalse },
	{ &g_msg12, "g_msg12", "", 0, 0, qfalse },
	{ &g_msgTime, "g_msgTime", "80", 0, 0, qtrue },
	{ &g_msgStartTime, "g_msgStartTime", "5", 0, 0, qtrue },

	// Static
	{ &g_swapCounter, "g_swapCounter", "1", 0, 0, qfalse },
	{ &shuffleTracking, "shuffleTracking", "0", 0, 0, qfalse },
	{ &needsBalance, "needsBalance", "0", CVAR_CHEAT, 0, qfalse },
	{ &needsDailyMsg, "needsDailyMsg", "0", 0, 0, qfalse },
	{ &latchedWarmup, "latchedWarmup", "-1", CVAR_LATCH, 0, qfalse }, // NOTE(nobo): KEEP -1 OR DIE !!
	{ &axisPlayers, "axisPlayers", "0", 0, 0, qfalse },
	{ &alliedPlayers, "alliedPlayers", "0", 0, 0, qfalse },

	// Forced stuff
	{ 0, "cl_allowdownload", "1", CVAR_SYSTEMINFO, qfalse },
	{ 0, "r_znear", "1", CVAR_SYSTEMINFO, qfalse },			
	{ 0, "rate", "25000", CVAR_SYSTEMINFO, qfalse },
	{ 0, "snaps", "20", CVAR_SYSTEMINFO, qfalse },
	{ 0, "cl_anonymous", "0", CVAR_SYSTEMINFO, qfalse },
	{ 0, "r_vertexLight", "1", CVAR_SYSTEMINFO, qfalse },
	//{ 0, "r_mapoverbrightbits", "3", CVAR_SYSTEMINFO, qfalse }, Nobo - Too many complaints
	{ 0, "r_depthbits", "24", CVAR_SYSTEMINFO, qfalse },
	{ 0, "cl_timenudge", "0", CVAR_SYSTEMINFO, qfalse },
	//{ 0, "com_maxfps", "125", CVAR_SYSTEMINFO, qfalse }, Nobo - Forcing the fps causes it to default to 90...
	{ 0, "cl_packetdup", "1", CVAR_SYSTEMINFO, qfalse },
	{ 0, "cl_maxpackets", "100", CVAR_SYSTEMINFO, qfalse },
// End

	{&g_scriptName, "g_scriptName", "", CVAR_ROM, 0, qfalse},
	{&ai_scriptName, "ai_scriptName", "", CVAR_ROM, 0, qfalse}
};

// bk001129 - made static to avoid aliasing
static int gameCvarTableSize = sizeof( gameCvarTable ) / sizeof( gameCvarTable[0] );


void G_InitGame( int levelTime, int randomSeed, int restart );
void G_RunFrame( int levelTime );
void G_ShutdownGame( int restart );
void CheckExitRules( void );

// Ridah, Cast AI
qboolean AICast_VisibleFromPos(	vec3_t srcpos, int srcnum,
								vec3_t destpos, int destnum, qboolean updateVisPos );
qboolean AICast_CheckAttackAtPos(int entnum, int enemy, vec3_t pos, qboolean ducking, qboolean allowHitWorld);
void AICast_Init (void);
// done.

void G_RetrieveMoveSpeedsFromClient( int entnum, char *text );

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
#if defined(__MACOS__)
#pragma export on
#endif
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6 ) {
#if defined(__MACOS__)
#pragma export off
#endif
	switch ( command ) {
	case GAME_INIT:
		G_InitGame( arg0, arg1, arg2 );
		return 0;
	case GAME_SHUTDOWN:
		G_ShutdownGame( arg0 );
		return 0;
	case GAME_CLIENT_CONNECT:
		return (int)ClientConnect( arg0, arg1, arg2 );
	case GAME_CLIENT_THINK:
		ClientThink( arg0 );
		return 0;
	case GAME_CLIENT_USERINFO_CHANGED:
		ClientUserinfoChanged( arg0 );
		return 0;
	case GAME_CLIENT_DISCONNECT:
		ClientDisconnect( arg0 );
		return 0;
	case GAME_CLIENT_BEGIN:
		ClientBegin( arg0 );
		return 0;
	case GAME_CLIENT_COMMAND:
		ClientCommand( arg0 );
		return 0;
	case GAME_RUN_FRAME:
		G_RunFrame( arg0 );
		return 0;
	case GAME_CONSOLE_COMMAND:
		return ConsoleCommand();
	case BOTAI_START_FRAME:
		return BotAIStartFrame( arg0 );
	// Ridah, Cast AI
	case AICAST_VISIBLEFROMPOS:
		return AICast_VisibleFromPos( (float *)arg0, arg1, (float *)arg2, arg3, arg4 );
	case AICAST_CHECKATTACKATPOS:
		return AICast_CheckAttackAtPos( arg0, arg1, (float *)arg2, arg3, arg4 );
	// done.

	case GAME_RETRIEVE_MOVESPEEDS_FROM_CLIENT:
		G_RetrieveMoveSpeedsFromClient( arg0, (char *)arg1 );
		return 0;
	}

	return -1;
}

void QDECL G_Printf( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);

	trap_Printf( text );
}

void QDECL G_DPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	if (!g_developer.integer)
		return;

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);

	trap_Printf( text );
}

void QDECL G_Error( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);

	trap_Error( text );
}


#define CH_KNIFE_DIST		48	// from g_weapon.c
#define CH_LADDER_DIST		100
#define CH_WATER_DIST		100
#define CH_BREAKABLE_DIST	64
#define CH_DOOR_DIST		96
#define CH_ACTIVATE_DIST	96
#define CH_EXIT_DIST		256

#define CH_MAX_DIST			256		// use the largest value from above
#define CH_MAX_DIST_ZOOM	8192	// max dist for zooming hints
/*
==============
G_CheckForCursorHints
	non-AI's check for cursor hint contacts

	server-side because there's info we want to show that the client
	just doesn't know about.  (health or other info of an explosive,invisible_users,items,etc.)

	traceEnt is the ent hit by the trace, checkEnt is the ent that is being
	checked against (in case the traceent was an invisible_user or something)
	
==============
*/
void G_CheckForCursorHints(gentity_t *ent) {
	vec3_t		forward, right, up, offset, end;
	trace_t		*tr;
	float		dist;
	gentity_t	*checkEnt, *traceEnt = 0;
  //gentity_t *traceEnt2 = 0; // JPW NERVE // TTimo unused
	playerState_t *ps;
	int			hintType, hintDist, hintVal;
	qboolean	zooming, indirectHit;	// indirectHit means the checkent was not the ent hit by the trace (checkEnt!=traceEnt)
	int			trace_contents;			// DHM - Nerve
	qboolean isMg42;

	// FIXME:	no need at all to do this trace/string comparison every frame.
	//			stagger it at least a little bit

//	if(!servercursorhints)
//		return;

	if(!ent->client)
		return;

	ps = &ent->client->ps;

	if(ps->aiChar != AICHAR_NONE)
		return;

	indirectHit = qfalse;

	zooming = (qboolean)(ps->eFlags & EF_ZOOMING);

	AngleVectors (ps->viewangles, forward, right, up);

	VectorCopy( ps->origin, offset );
	offset[2] += ps->viewheight;

	// lean
	if(ps->leanf) {
		VectorMA(offset, ps->leanf, right, offset);
	}

//	SnapVector( offset );

	if(zooming)
		VectorMA (offset, CH_MAX_DIST_ZOOM, forward, end);
	else
		VectorMA (offset, CH_MAX_DIST, forward, end);

	tr = &ps->serverCursorHintTrace;

	// DHM - Nerve
	if ( g_gametype.integer == GT_SINGLE_PLAYER )
		trace_contents = (CONTENTS_TRIGGER|CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY);
	else
		trace_contents = (CONTENTS_TRIGGER|CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY|CONTENTS_CORPSE);

	trap_Trace (tr, offset, NULL, NULL, end, ps->clientNum, trace_contents);
	// dhm - end

	// reset all
	hintType	= ps->serverCursorHint		= HINT_NONE;
	hintVal		= ps->serverCursorHintVal	= 0;
	
	if(zooming) {
		dist		= tr->fraction * CH_MAX_DIST_ZOOM;
		hintDist	= CH_MAX_DIST_ZOOM;
	} else {
		dist		= tr->fraction * CH_MAX_DIST;
		hintDist	= CH_MAX_DIST;
	}

	if(tr->fraction == 1) {
		return;
	}

	traceEnt = &g_entities[tr->entityNum];

	// DHM - Nerve :: Ignore trigger_objective_info
	if ( (ps->stats[ STAT_PLAYER_CLASS ] != PC_MEDIC && traceEnt->client) 
		 || !(strcmp(traceEnt->classname, "trigger_objective_info")) 
		 || !(strcmp(traceEnt->classname, "trigger_multiple")) ) {

		trap_Trace (tr, offset, NULL, NULL, end, traceEnt->s.number, trace_contents);
		if(zooming) {
			dist		= tr->fraction * CH_MAX_DIST_ZOOM;
			hintDist	= CH_MAX_DIST_ZOOM;
		} else {
			dist		= tr->fraction * CH_MAX_DIST;
			hintDist	= CH_MAX_DIST;
		}
		if(tr->fraction == 1) {
			return;
		}
		traceEnt = &g_entities[tr->entityNum];
	}

	//
	// WORLD
	//
	if(tr->entityNum == ENTITYNUM_WORLD) {
		if ((tr->contents & CONTENTS_WATER) && !(ps->powerups[PW_BREATHER])) {
			hintDist = CH_WATER_DIST;
			hintType = HINT_WATER;
		}
		
		// ladder
		else if( (tr->surfaceFlags & SURF_LADDER) && !(ps->pm_flags & PMF_LADDER) ) {
			hintDist = CH_LADDER_DIST;
			hintType = HINT_LADDER;
		}
	}

	//
	// PEOPLE
	//
	else if(tr->entityNum < MAX_CLIENTS ) {
		// DHM - Nerve
		if ( g_gametype.integer >= GT_WOLF ) {
			if (ps->stats[STAT_PLAYER_CLASS] == PC_MEDIC
				&& traceEnt->client
				&& traceEnt->client->ps.pm_type == PM_DEAD
				&& !(traceEnt->client->ps.pm_flags & PMF_LIMBO)) {
				if (traceEnt->client->sess.sessionTeam != ent->client->sess.sessionTeam) {
					traceEnt = FindRevivableTeammate(ent, traceEnt->s.clientNum, forward, offset, tr->endpos, end);
					if (traceEnt == NULL) {
						return;
					}
				}

				hintDist = 48; // JPW NERVE matches weapon_syringe in g_weapon.c 
				hintType = HINT_REVIVE;
			}
		}
		// dhm - Nerve

	}

	//
	// OTHER ENTITIES
	//
	else {
		checkEnt = traceEnt;

		// check invisible_users first since you don't want to draw a hint based
		// on that ent, but rather on what they are targeting.
		// so find the target and set checkEnt to that to show the proper hint.
		if(traceEnt->s.eType == ET_GENERAL) {

			// ignore trigger_aidoor.  can't just not trace for triggers, since I need invisible_users...
			// damn, I would like to ignore some of these triggers though.

			if(!Q_stricmp(traceEnt->classname, "trigger_aidoor")) {
				return;
			}

			if(!Q_stricmp(traceEnt->classname, "func_invisible_user")) {
				indirectHit = qtrue;

				// DHM - Nerve :: Put this back in only in multiplayer
				if(g_gametype.integer >= GT_WOLF && traceEnt->s.dmgFlags) {	// hint icon specified in entity
					hintType = traceEnt->s.dmgFlags;
					hintDist = CH_ACTIVATE_DIST;
					checkEnt = 0;
				}
				else {						// use target for hint icon
					checkEnt = G_Find (NULL, FOFS(targetname), traceEnt->target);
					if(!checkEnt) {		// no target found
						hintType = HINT_BAD_USER;
						hintDist = CH_MAX_DIST_ZOOM;	// show this one from super far for debugging
					}
				}
			}
		}


		if(checkEnt) {
			isMg42 = !Q_stricmp(traceEnt->classname, "misc_mg42") || !Q_stricmp(traceEnt->classname, "mg42_base");

			if(checkEnt->s.eType == ET_GENERAL || (isMg42 && checkEnt->s.eType == ET_GAMEMODEL)) {

				// this is effectively an 'exit' brush.  they should be created with:
				//
				// classname = 'ai_trigger'
				// ainame = 'player'
				// target = 'endmap'
				//
				if(!Q_stricmp(traceEnt->classname, "ai_trigger")) {
					if(	(!Q_stricmp(traceEnt->aiName, "player")) &&
						(!Q_stricmp(traceEnt->target, "endmap")) ) {
							hintDist = CH_EXIT_DIST;
							hintType = HINT_EXIT;

							// show distance in the cursorhint bar
							hintVal = (int)dist;	// range for this hint is 256, so it happens to translate nicely
					}
				}

				if( isMg42 && (ps->weapon != WP_SNIPERRIFLE) ) { // JPW NERVE no hint if you're scoped in sniperwise
					if (traceEnt->chain)
					{
						traceEnt = traceEnt->chain;
					}
					hintDist = CH_ACTIVATE_DIST;
					hintType = HINT_MG42;

					// JPW NERVE -- busted MG42 shouldn't show a use hint by default
					if (traceEnt->health <= 0 || g_automg42.integer) {
						hintDist = 0;
						hintType = HINT_FORCENONE;
						ps->serverCursorHint = HINT_FORCENONE;
					}

					// DHM - Nerve :: Engineers can repair turrets
					if ( g_gametype.integer >= GT_WOLF ) {
						if ( ps->stats[ STAT_PLAYER_CLASS ] == PC_ENGINEER ) {
							if ( !traceEnt->takedamage || traceEnt->s.frame == 2 ) {
								if (!(g_automg42.integer && g_entities[traceEnt->mg42BaseEnt].think == automg42_base_soaring)) {
									hintType = HINT_BUILD;
									hintDist = CH_BREAKABLE_DIST;
									int fullHealth = g_automg42.integer ? traceEnt->count2 : MG42_MULTIPLAYER_HEALTH;
									int fixedHealth = fullHealth * MG42_FIXED_HEALTH_MULT;
									float fixRate = 255.0f / (float)fixedHealth;
									hintVal = fixRate * traceEnt->health;
									if (hintVal > 255)
										hintVal = 255;
								}
							}
						}
					}
				}
			}
			else if(checkEnt->s.eType == ET_EXPLOSIVE) {
//				if(traceEnt->s.dmgFlags) {	 // override flag		// hint icon specified in entity, this overrides type
//					hintType = traceEnt->s.dmgFlags;
//				} else {
					if ((checkEnt->health > 0) || (checkEnt->spawnflags & 64)) {	// JPW NERVE they are now if it's dynamite			// 0 health explosives are not breakable
						hintDist	= CH_BREAKABLE_DIST;
						hintType	= HINT_BREAKABLE;

						// DHM - Nerve :: Show different hint if it can only be destroyed with dynamite
						if ( g_gametype.integer >= GT_WOLF && (checkEnt->spawnflags & 64) ) {
// JPW NERVE only show hint for players who can blow it up
							vec3_t mins,maxs,range = { 40, 40, 52 };
							int i,num;
              //int defendingTeam=0; // TTimo unused
              int touch[MAX_GENTITIES];
							gentity_t *hit = NULL;

							VectorSubtract( ent->client->ps.origin, range, mins );
							VectorAdd( ent->client->ps.origin, range, maxs );
							num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );
							for ( i=0 ; i<num ; i++ ) {
								hit = &g_entities[touch[i]];
								if ( !( hit->r.contents & CONTENTS_TRIGGER ) )
									continue;
								if (!strcmp(hit->classname,"trigger_objective_info")) {
									if ( !(hit->spawnflags & (AXIS_OBJECTIVE|ALLIED_OBJECTIVE)) )
										continue;
// we're in a trigger_objective_info field with at least one owner, so use this one and bail
									break;
								}
							}
							if ( (hit) && 
								 ( ((ent->client->sess.sessionTeam == TEAM_RED) && (hit->spawnflags & ALLIED_OBJECTIVE)) || 
								   ((ent->client->sess.sessionTeam == TEAM_BLUE) && (hit->spawnflags & AXIS_OBJECTIVE)) ) 
								   ) {
								hintDist = CH_BREAKABLE_DIST * 2;
								hintType = HINT_BREAKABLE_DYNAMITE;
							}
							else {
								hintDist = 0;
								hintType = ps->serverCursorHint		= HINT_FORCENONE;
								hintVal		= ps->serverCursorHintVal	= 0;
								return;
							}
// jpw
						}
						// dhm - end

						hintVal		= checkEnt->health;		// also send health to client for visualization
					}
//				}
			}
			else if(checkEnt->s.eType == ET_ALARMBOX) {
				if(checkEnt->health > 0) {
//					hintDist	= CH_BREAKABLE_DIST;
					hintType	= HINT_ACTIVATE;
				}
			}
			else if(checkEnt->s.eType == ET_ITEM) {
				gitem_t *it;
				it = &bg_itemlist[checkEnt->item - bg_itemlist];

				hintDist = CH_ACTIVATE_DIST;

				switch(it->giType) {
					case IT_HEALTH:
						hintType = HINT_HEALTH;
						break;
					case IT_TREASURE:
						hintType = HINT_TREASURE;
						break;
					case IT_CLIPBOARD:
						hintType = HINT_CLIPBOARD;
						break;
					case IT_WEAPON:
// JPW NERVE changed this to be more specific
						if (it->giTag != WP_LUGER && it->giTag != WP_COLT) {
							if (ps->stats[STAT_PLAYER_CLASS] == PC_SOLDIER) { // soldier can pick up all weapons
								hintType = HINT_WEAPON;
								break;
							}
							if (ps->stats[STAT_PLAYER_CLASS] != PC_LT) // medics & engrs can't pick up squat
								break;
							if (it->giTag == WP_THOMPSON || it->giTag == WP_MP40 || it->giTag == WP_STEN)
								hintType = HINT_WEAPON;
						}
// jpw
						break;
					case IT_AMMO:	
						hintType = HINT_AMMO;
						break;
					case IT_ARMOR:	
						hintType = HINT_ARMOR;
						break;
					case IT_POWERUP:	
						hintType = HINT_POWERUP;
						break;
					case IT_HOLDABLE:
						hintType = HINT_HOLDABLE;
						break;
					case IT_KEY:
						hintType = HINT_INVENTORY;
						break;
					case IT_TEAM:
						if ( !Q_stricmp( traceEnt->classname, "team_CTF_redflag" ) && ent->client->sess.sessionTeam == TEAM_BLUE )
							hintType = HINT_POWERUP;
						else if ( !Q_stricmp( traceEnt->classname, "team_CTF_blueflag" ) && ent->client->sess.sessionTeam == TEAM_RED )
							hintType = HINT_POWERUP;
						break;
					case IT_BAD:
					default:
						break;
				}
			}
			else if (checkEnt->s.eType == ET_MOVER) {
				if(!Q_stricmp(checkEnt->classname, "func_door_rotating")) {
					if(checkEnt->moverState == MOVER_POS1ROTATE) {	// stationary/closed
						hintDist = CH_DOOR_DIST;
						hintType = HINT_DOOR_ROTATING;

						if(checkEnt->key == -1) {	// locked
	//						hintType = HINT_DOOR_ROTATING_LOCKED;
						}
					}
				}
				else if(!Q_stricmp(checkEnt->classname, "func_door")) {
					if(checkEnt->moverState == MOVER_POS1) {	// stationary/closed
						hintDist = CH_DOOR_DIST;
						hintType = HINT_DOOR;

						if(checkEnt->key == -1) {	// locked
	//						hintType = HINT_DOOR_LOCKED;
						}
					}
				}
				else if(!Q_stricmp(checkEnt->classname, "func_button")) {
					hintDist = CH_ACTIVATE_DIST;
					hintType = HINT_BUTTON;
				}/*
				else if(checkEnt->s.dmgFlags == HINT_CHAIR) {
					hintDist = CH_ACTIVATE_DIST;
					hintType = HINT_CHAIR;
				}*/
			}

			// DHM - Nerve :: Handle wolf multiplayer hints
			if ( g_gametype.integer >= GT_WOLF ) {

				if ( checkEnt->s.eType == ET_MISSILE ) {
					if ( ps->stats[ STAT_PLAYER_CLASS ] == PC_ENGINEER ) {
						hintDist	= CH_BREAKABLE_DIST;
						hintType	= HINT_DISARM;
						hintVal		= checkEnt->health;		// also send health to client for visualization
						if ( hintVal > 255 )
							hintVal = 255;
					}
				}

			}
			// dhm - end

			// hint icon specified in entity (and proper contact was made, so hintType was set)
			// first try the checkent...
			if(checkEnt->s.dmgFlags && hintType) {
				hintType = checkEnt->s.dmgFlags;
			}
		}

		// then the traceent
		if(traceEnt->s.dmgFlags && hintType) {
			hintType = traceEnt->s.dmgFlags;
		}

	}

	if(zooming) {

		hintDist = CH_MAX_DIST_ZOOM;

		// zooming can eat a lot of potential hints
		switch(hintType) {

			// allow while zooming
			case HINT_PLAYER:
			case HINT_TREASURE:
			case HINT_LADDER:
			case HINT_EXIT:
			case HINT_PLYR_FRIEND:
			case HINT_PLYR_NEUTRAL:
			case HINT_PLYR_ENEMY:
			case HINT_PLYR_UNKNOWN:
				break;

			default:
				return;
		}
	}

	if(dist <= hintDist) {
		ps->serverCursorHint = hintType;
		ps->serverCursorHintVal = hintVal;
	}


//	Com_Printf("hint: %d\n", ps->serverCursorHint);
}


/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams( void ) {
	gentity_t	*e, *e2;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for ( i=1, e=g_entities+i ; i < level.num_entities ; i++,e++ ){
		if (!e->inuse)
			continue;
		
		if (!e->team)
			continue;

		if (e->flags & FL_TEAMSLAVE)
			continue;
		
		if (!Q_stricmp (e->classname, "func_tramcar"))
		{
			if (e->spawnflags & 8) // leader
			{
				e->teammaster = e;
			}
			else 
			{
				continue;
			}
		}
		
		c++;
		c2++;
		for (j=i+1, e2=e+1 ; j < level.num_entities ; j++,e2++)
		{
			if (!e2->inuse)
				continue;
			if (!e2->team)
				continue;
			if (e2->flags & FL_TEAMSLAVE)
				continue;
			if (!strcmp(e->team, e2->team))
			{
				c2++;
				e2->teamchain = e->teamchain;
				e->teamchain = e2;
				e2->teammaster = e;
//				e2->key = e->key;	// (SA) I can't set the key here since the master door hasn't finished spawning yet and therefore has a key of -1
				e2->flags |= FL_TEAMSLAVE;

				if (!Q_stricmp (e2->classname, "func_tramcar"))
				{
					trap_UnlinkEntity (e2);
				}
		
				// make sure that targets only point at the master
				if ( e2->targetname ) {
					e->targetname = e2->targetname;
					
					// Rafael
					// note to self: added this because of problems
					// pertaining to keys and double doors
					if (Q_stricmp (e2->classname, "func_door_rotating"))
						e2->targetname = NULL;
				}
			}
		}
	}

	if (trap_Cvar_VariableIntegerValue("g_gametype") != GT_SINGLE_PLAYER) {
		G_Printf ("%i teams with %i entities\n", c, c2);
	}
}


/*
==============
G_RemapTeamShaders
==============
*/
void G_RemapTeamShaders() {
#ifdef MISSIONPACK
	char string[1024];
	float f = level.time * 0.001;
	Com_sprintf( string, sizeof(string), "team_icon/%s_red", g_redteam.string );
	AddRemap("textures/ctf2/redteam01", string, f); 
	AddRemap("textures/ctf2/redteam02", string, f); 
	Com_sprintf( string, sizeof(string), "team_icon/%s_blue", g_blueteam.string );
	AddRemap("textures/ctf2/blueteam01", string, f); 
	AddRemap("textures/ctf2/blueteam02", string, f); 
	trap_SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
#endif
}

/*
=================
G_ForceGametypeCvars
=================
*/
void G_ForceGametypeCvars(void)
{
	if (g_ffa.integer)
	{
		trap_Cvar_Set("g_deathmatch", "1");
		trap_Cvar_Set("g_forceClass", "0"); // Force class to soldier
		trap_Cvar_Set("g_minGameClients", "2");
		trap_Cvar_Set("g_maxlives", "0");
		trap_Cvar_Set("g_alliedmaxlives", "0");
		trap_Cvar_Set("g_axismaxlives", "0");
		trap_Cvar_Set("g_teamForceBalance", "0");
		trap_Cvar_Set("g_soldNades", "0");

		if (!g_fraglimit.integer)
		{
			trap_Cvar_Set("fraglimit", "10");
		}
	}

	if (g_medicAmmoPack.integer &&
		g_forceClass.integer == PC_MEDIC)
	{
		// NOTE(nobo): This is required, otherwise ammo pack animations will try to play client-side even though there isn't enough charge.
		trap_Cvar_Set("g_LTChargeTime", va("%i", trap_Cvar_VariableIntegerValue("g_medicChargeTime") * 3));
	}
}

/*
=================
G_RegisterCvars
=================
*/
void G_RegisterCvars(void) {
	int			i;
	cvarTable_t	*cv;
	qboolean remapped = qfalse;

	for (i = 0, cv = gameCvarTable; i < gameCvarTableSize; i++, cv++) {
		trap_Cvar_Register(cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags);
		if (cv->vmCvar)
			cv->modificationCount = cv->vmCvar->modificationCount;

		if (cv->teamShader) {
			remapped = qtrue;
		}
	}

	if (remapped) {
		G_RemapTeamShaders();
	}

	// check some things
	// DHM - Gametype is currently restricted to supported types only
	if (g_gametype.integer < GT_WOLF || g_gametype.integer > GT_WOLF_CPH) { // JPW NERVE WOLF_CPH now highest type
		G_Printf("g_gametype %i is out of range, defaulting to GT_WOLF(5)\n", g_gametype.integer);
		trap_Cvar_Set("g_gametype", "5");
		trap_Cvar_Update(&g_gametype);
	}

	if (g_forceClass.integer > 3 || g_forceClass.integer < -1) {
		G_Printf("g_forceClass %i is out of range, defaulting to -1 (Off)\n", g_forceClass.integer);
		trap_Cvar_Set("g_forceClass", "-1");
	}

	if (trap_Cvar_VariableIntegerValue("g_warmup") < 0) {
		trap_Cvar_Set("g_warmup", "0");
	}
	if (trap_Cvar_VariableIntegerValue("latchedWarmup") == -1) {
		trap_Cvar_Set("latchedWarmup", va("%i", trap_Cvar_VariableIntegerValue("g_warmup")));
	}

	// Rafael gameskill
	if (g_gameskill.integer < GSKILL_EASY || g_gameskill.integer > GSKILL_VERYHARD)
	{
		G_Printf("g_gameskill %i is out of range, default to medium\n", g_gameskill.integer);
		trap_Cvar_Set("g_gameskill", "3"); // default to medium
	}

	bg_pmove_gameskill_integer = g_gameskill.integer;
	// done

	level.warmupModificationCount = g_warmup.modificationCount;

	G_ForceGametypeCvars();
}

/*
=================
G_UpdateCvars
=================
*/
void G_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	qboolean remapped = qfalse;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		if ( cv->vmCvar ) {
			trap_Cvar_Update( cv->vmCvar );

			if ( cv->modificationCount != cv->vmCvar->modificationCount ) {
				cv->modificationCount = cv->vmCvar->modificationCount;

				if ( cv->trackChange ) {
					trap_SendServerCommand( -1, va("print \"Server:[lof] %s [lon]changed to[lof] %s\n\"", 
						cv->cvarName, cv->vmCvar->string ) );
				}

				if (cv->teamShader) {
					remapped = qtrue;
				}
			}
		}
	}

	if (remapped) {
		G_RemapTeamShaders();
	}
}

/*
==============
G_SpawnScriptCamera
	create the game entity that's used for camera<->script communication and portal location for camera view
==============
*/
void G_SpawnScriptCamera(void) {
	if(g_camEnt)
		G_FreeEntity(g_camEnt);

	g_camEnt = G_Spawn();

	g_camEnt->scriptName = "scriptcamera";

	g_camEnt->s.eType = ET_CAMERA;
	g_camEnt->s.apos.trType = TR_STATIONARY;
	g_camEnt->s.apos.trTime = 0;
	g_camEnt->s.apos.trDuration = 0;
	VectorCopy (g_camEnt->s.angles, g_camEnt->s.apos.trBase);
	VectorClear (g_camEnt->s.apos.trDelta );

	g_camEnt->s.frame = 0;

	g_camEnt->r.svFlags |= SVF_NOCLIENT;		// only broadcast when in use

	if (g_camEnt->s.number >= MAX_CLIENTS && g_camEnt->scriptName) {
		G_Script_ScriptParse( g_camEnt);
		G_Script_ScriptEvent( g_camEnt, "spawn", "" );
	}

}

qboolean G_ArchiveLogFile(const char *logfile, void (*preArchive)()) {
	char log_filename[MAX_CVAR_VALUE_STRING];

	if (g_archiveLogDay.integer <= 0) {
		return qfalse;
	}

	// archive the console.log on "g_archiveLogDay" every month.
	// format: Jun2017-Jul2017.log
	time_t currentSeconds = time(NULL);
	struct tm today = *localtime(&currentSeconds);
	int days = getDaysInMonth(today.tm_mon);

	if (g_archiveLogDay.integer == today.tm_mday ||
		(today.tm_mday == days && g_archiveLogDay.integer > days)) {
		Q_strncpyz(log_filename, COM_SkipPath((char*)logfile), sizeof(log_filename));
		COM_StripExtension2(log_filename, log_filename, sizeof(log_filename));
		// 86400 seconds in a day
		time_t monthAgoSeconds = currentSeconds - ((time_t)86400 * ((time_t)today.tm_mday + 2));
		struct tm monthAgo = *localtime(&monthAgoSeconds);
		char *archiveExtension = ".log";
		char *archiveFileName = va("%s%d-%s%d_%s%s",
			getMonthString(monthAgo.tm_mon), getYearFromCYear(monthAgo.tm_year),
			getMonthString(today.tm_mon), getYearFromCYear(today.tm_year),
			log_filename, archiveExtension);
		char *archiveDirectoryName = "archive"; // TODO: cvar?

		if (!FileExists(archiveFileName, archiveDirectoryName, archiveExtension, qtrue)) {
			if (preArchive != NULL) {
				preArchive();
			}
			char *archivePath = va("%s/%s", archiveDirectoryName, archiveFileName);
			trap_FS_Rename(logfile, archivePath);
			return qtrue;
		}
	}
	return qfalse;
}

/* Examples:
weapon_magicammo
{
	{
		"origin" "123 321 213"
		"respawn" "30"
		"ammo" "32"
	}
	{
		"origin" "321 123 213"
		"respawn" "30"
		"ammo" "64"
	}
}
weapon_mp40
{
	{
		"origin" "444 111 222"
		"respawn" "120"
		"ammo" "128"
	}
}
*/
// Not used anymore :(
#if 0
void G_InitItemSpawns()
{
	int				len, numitems = 0;
	fileHandle_t	spawnsFile;
	char			buf[MAX_SPAWN_VARS_CHARS];
	char			key[MAX_SPAWN_VARS];
	char			mapname[MAX_QPATH];
	char			*token;
	char			*data;
	gitem_t			*item;
	gentity_t		*ent;

	trap_Cvar_VariableStringBuffer("mapname", mapname, sizeof(mapname));
	char *filename = va("maps/%s.is", mapname);
	len = trap_FS_FOpenFile(filename, &spawnsFile, FS_READ);

	if (!spawnsFile) {
		G_Printf(va("item spawns file not found: %s\n", filename));
		return;
	}
	if (len >= MAX_SPAWN_VARS_CHARS) {
		G_Printf(va("file too large: %s is %i, max allowed is %i\n", filename, len, MAX_BOTS_TEXT));
		trap_FS_FCloseFile(spawnsFile);
		return;
	}

	trap_FS_Read(buf, len, spawnsFile);
	buf[len] = 0;
	trap_FS_FCloseFile(spawnsFile);
	data = (char *)buf;

	COM_BeginParseSession("G_InitItemSpawns");

	while ((token = COM_Parse(&data)) && strlen(token)) {
		item = BG_FindItemForClassname(token);
		if (!item) {
			COM_ParseError("%s is an invalid item classname!", token);
			return;
		}

		token = COM_Parse(&data);
		if (Q_stricmp(token, "{")) {
			COM_ParseError("Invalid token. Found %s, expected {", token);
			return;
		}
		
		while ((token = COM_Parse(&data)) && Q_stricmp(token, "}")) {
			if (Q_stricmp(token, "{")) {
				COM_ParseError("Invalid token. Found %s, expected {", token);
				return;
			}

			ent = G_Spawn();
			ent->classname = item->classname;

			// NOTE(nobo): This will free it next frame if something goes awry.
			// That way I don't have to have a G_FreeEntity above every return statement below.
			ent->think = G_FreeEntity;
			ent->nextthink = level.time + FRAMETIME;

			while ((token = COM_Parse(&data)) && Q_stricmp(token, "}")) {
				if (!strlen(token)) {
					COM_ParseError("Missing a key!");
					return;
				}

				Q_strncpyz(key, token, sizeof(key));

				token = COM_Parse(&data);
				if (!strlen(token)) {
					COM_ParseError("Key %s is missing a value!", key);
					return;
				}

				if (!G_ParseField(key, token, ent)) {
					COM_ParseError("Invalid key/value given. Key: %s, Value: %s", key, token);
					return;
				}
			}

			if (Q_stricmp(token, "}")) {
				COM_ParseError("Invalid token. Found %s, expected }", token);
				return;
			}

			G_SpawnItem(ent, item);
			numitems++;
		}

		if (Q_stricmp(token, "}")) {
			COM_ParseError("Invalid token. Found %s, expected }", token);
			return;
		}
	}

	G_Printf("spawned %d items (%s)\n", numitems, filename);
}
#endif

/*
============
G_InitGame

============
*/
void G_InitGame(int levelTime, int randomSeed, int restart) {
	int					i;
	char				cs[MAX_INFO_STRING];
	char				mapName[MAX_QPATH];
	qboolean			needsMapName;

	int initial_time, end_time;

	initial_time = trap_Milliseconds();

	if (trap_Cvar_VariableIntegerValue("g_gametype") != GT_SINGLE_PLAYER) {
		G_Printf("------- Game Initialization -------\n");
		G_Printf("gamename: %s\n", GAMEVERSION);
		G_Printf("gamedate: %s\n", __DATE__);
	}

	srand(randomSeed);

	G_RegisterCvars();

	needsMapName =
		g_mapConfigs.integer ||
		g_ffa.integer ||
		g_customSpawns.integer;

	G_InitMemory();

	// NERVE - SMF - intialize gamestate
	if (g_gamestate.integer == GS_INITIALIZE) {
		if (g_noTeamSwitching.integer) {
			trap_Cvar_Set("gamestate", va("%i", GS_WAITING_FOR_PLAYERS));
		}
		else {
			trap_Cvar_Set("gamestate", va("%i", GS_WARMUP));
		}
	}

	// set some level globals
	memset(&level, 0, sizeof(level));
	level.time = levelTime;
	level.startTime = levelTime;

	level.snd_fry = G_SoundIndex("sound/player/fry.wav");	// FIXME standing in lava / slime
	level.bulletRicochetSound = G_SoundIndex("bulletRicochet");
	level.snipersound = G_SoundIndex("sound/weapons/mauser/mauserf1.wav");
	level.knifeSound[0] = G_SoundIndex("sound/weapons/knife/knife_hitwall1.wav");

	// RF, init the anim scripting
	level.animScriptData.soundIndex = G_SoundIndex;
	level.animScriptData.playSound = G_AnimScriptSound;

	if (g_gametype.integer != GT_SINGLE_PLAYER && g_log.string[0]) {
		if (g_logSync.integer) {
			trap_FS_FOpenFile(g_log.string, &level.logFile, FS_APPEND_SYNC);
		}
		else {
			trap_FS_FOpenFile(g_log.string, &level.logFile, FS_APPEND);
		}
		if (!level.logFile) {
			G_Printf("WARNING: Couldn't open logfile: %s\n", g_log.string);
		}
		else {
			char	serverinfo[MAX_INFO_STRING];

			trap_GetServerinfo(serverinfo, sizeof(serverinfo));

			G_LogPrintf("------------------------------------------------------------\n");
			G_LogPrintf("InitGame (%s) (%d) : %s\n", getDateTime(), levelTime, serverinfo);
		}
	}
	else {
		if (trap_Cvar_VariableIntegerValue("g_gametype") != GT_SINGLE_PLAYER) {
			G_Printf("Not logging to disk.\n");
		}
	}

	G_InitWorldSession();

	// DHM - Nerve :: Clear out spawn target config strings
	if (g_gametype.integer >= GT_WOLF) {
		trap_GetConfigstring(CS_MULTI_INFO, cs, sizeof(cs));
		Info_SetValueForKey(cs, "numspawntargets", "0");
		trap_SetConfigstring(CS_MULTI_INFO, cs);

		for (i = CS_MULTI_SPAWNTARGETS; i < CS_MULTI_SPAWNTARGETS + MAX_MULTI_SPAWNTARGETS; i++) {
			trap_SetConfigstring(i, "");
		}
	}

	// initialize all entities for this game
	memset(g_entities, 0, MAX_GENTITIES * sizeof(g_entities[0]));
	level.gentities = g_entities;

	// initialize all clients for this game
	level.maxclients = g_maxclients.integer;
	memset(g_clients, 0, MAX_CLIENTS * sizeof(g_clients[0]));
	level.clients = g_clients;

	// set client fields on player ents
	for (i = 0; i < level.maxclients; i++) {
		g_entities[i].client = level.clients + i;
	}

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	level.num_entities = MAX_CLIENTS;

	// let the server system know where the entites are
	trap_LocateGameData(level.gentities, level.num_entities, sizeof(gentity_t),
		&level.clients[0].ps, sizeof(level.clients[0]));

	// load level script
	G_Script_ScriptLoad();

	// set the respawn time if one is specified in the .cfg
	if (g_userAxisRespawnTime.integer)
	{
		trap_Cvar_Set("g_redlimbotime", va("%i", g_userAxisRespawnTime.integer * 1000));
	}
	if (g_userAlliedRespawnTime.integer)
	{
		trap_Cvar_Set("g_bluelimbotime", va("%i", g_userAlliedRespawnTime.integer * 1000));
	}

	// reserve some spots for dead player bodies
	InitBodyQue();

	ClearRegisteredItems();

	if (needsMapName)
	{
		trap_Cvar_VariableStringBuffer("mapname", mapName, sizeof(mapName));
	}

	if (!restart)
	{
		// NOTE(nobo): Append to entity string before it's parsed by G_SpawnEntitiesFromString
		if (g_customSpawns.integer)
		{
			trap_AppendEntityString(va("maps/%s.spawns", mapName));
		}
		// NOTE(nobo): ffa has it's own special spawns that we don't want conflicting with "customSpawns"
		if (g_ffa.integer)
		{
			trap_AppendEntityString(va("maps/%s_ffa.spawns", mapName));
		}
	}

	// parse the key/value pairs and spawn gentities
	G_SpawnEntitiesFromString();

	// create the camera entity that will communicate with the scripts
	G_SpawnScriptCamera();

	// general initialization
	G_FindTeams();

	// make sure we have flags for CTF, etc
	if (g_gametype.integer >= GT_TEAM) {
		G_CheckTeamItems();
	}

	SaveRegisteredItems();

	// L0 - auto cfg for each map
	if (g_mapConfigs.integer) {
		trap_SendConsoleCommand(EXEC_APPEND, va("exec mapconfigs/%s.cfg\n", mapName));
	}

	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		if (BotAISetup(restart) == 0) {
			BotAILoadMap(restart);
			G_InitBots(restart);
		}
		else {
			trap_Cvar_Set("bot_enable", "0");
			G_LogPrintf("G_InitGame: BotAISetup failed.\n");
		}
	}

	G_RemapTeamShaders();

	// Nobo - Initialize daily rankings
	if (g_dailystats.integer) {
		stats_InitDailyRankings();
	}

	// Nobo - Initialize round stats
	if (g_roundStats.integer) {
		stats_InitRoundStats();
	}

	end_time = trap_Milliseconds();

	G_Printf(va("G_InitGame load time: %d msec\n", end_time - initial_time));

	if (trap_Cvar_VariableIntegerValue("g_gametype") != GT_SINGLE_PLAYER) {
		G_Printf("-----------------------------------\n");
	}
}

const char* logfile_cvar_name = "logfile";

void PreArchiveCommonLogFile() {
	trap_CloseCommonLogFile();
	trap_Cvar_Set(logfile_cvar_name, "0");
}

/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame( int restart ) {
	char logfile[MAX_CVAR_VALUE_STRING];

	if (g_gametype.integer != GT_SINGLE_PLAYER) {
		G_Printf ("==== ShutdownGame ====\n");
	}

	if (level.logFile) {
		G_LogPrintf("ShutdownGame:\n");
		G_LogPrintf("------------------------------------------------------------\n");
		trap_FS_FCloseFile(level.logFile);
		Q_strncpyz(logfile, g_log.string, sizeof(logfile));
		G_ArchiveLogFile(logfile, NULL);
	}

	if (trap_CommonLogFileExists()) {
		trap_Cvar_VariableStringBuffer("logfileName", logfile, sizeof(logfile));
		int logfileValue = trap_Cvar_VariableIntegerValue(logfile_cvar_name);
		if (G_ArchiveLogFile(logfile, PreArchiveCommonLogFile)) {
			trap_Cvar_Set(logfile_cvar_name, va("%d", logfileValue));
		}
	}

	// Ridah, shutdown the Botlib, so weapons and things get reset upon doing a "map xxx" command
	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		int i;

		// Ridah, kill AI cast's
		for (i=0 ; i<g_maxclients.integer ; i++) {
			if ( g_entities[i].r.svFlags & SVF_CASTAI ) {
				trap_DropClient( i, "Drop Cast AI" );
			}
		}
		// done.
	}
	// done.

	// write all the client session data so we can get it back
	G_WriteSessionData();
	
	// L0 - Tempban
	RemoveExpiredTempbans();

	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		BotAIShutdown( restart );
	}
}



//===================================================================

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link

void QDECL Com_Error ( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	G_Error( "%s", text);
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	G_Printf ("%s", text);
}

#endif

/*
========================================================================

PLAYER COUNTING / SCORE SORTING

========================================================================
*/

/*
=============
AddTournamentPlayer

If there are less than two tournament players, put a
spectator in the game and restart
=============
*/
void AddTournamentPlayer( void ) {
	int			i;
	gclient_t	*client;
	gclient_t	*nextInLine;

	if ( level.numPlayingClients >= 2 ) {
		return;
	}

	// never change during intermission
	if ( level.intermissiontime ) {
		return;
	}

	nextInLine = NULL;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			continue;
		}
		// never select the dedicated follow or scoreboard clients
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD || 
			client->sess.spectatorClient < 0  ) {
			continue;
		}

		if ( !nextInLine || client->sess.spectatorTime < nextInLine->sess.spectatorTime ) {
			nextInLine = client;
		}
	}

	if ( !nextInLine ) {
		return;
	}

	level.warmupTime = -1;

	// set them to free-for-all team
	SetTeam( &g_entities[ nextInLine - level.clients ], "f", qfalse );
}

/*
=======================
RemoveTournamentLoser

Make the loser a spectator at the back of the line
=======================
*/
void RemoveTournamentLoser( void ) {
	int			clientNum;

	if ( level.numPlayingClients != 2 ) {
		return;
	}

	clientNum = level.sortedClients[1];

	if ( level.clients[ clientNum ].pers.connected != CON_CONNECTED ) {
		return;
	}

	// make them a spectator
	SetTeam( &g_entities[ clientNum ], "s", qfalse );
}


/*
=======================
AdjustTournamentScores

=======================
*/
void AdjustTournamentScores( void ) {
	int			clientNum;

	clientNum = level.sortedClients[0];
	if ( level.clients[ clientNum ].pers.connected == CON_CONNECTED ) {
		level.clients[ clientNum ].sess.wins++;
		ClientUserinfoChanged( clientNum );
	}

	clientNum = level.sortedClients[1];
	if ( level.clients[ clientNum ].pers.connected == CON_CONNECTED ) {
		level.clients[ clientNum ].sess.losses++;
		ClientUserinfoChanged( clientNum );
	}

}

/*
=============
SortRanks

=============
*/
int QDECL SortRanks( const void *a, const void *b ) {
	gclient_t	*ca, *cb;

	ca = &level.clients[*(int *)a];
	cb = &level.clients[*(int *)b];

	// sort special clients last
	if ( ca->sess.spectatorState == SPECTATOR_SCOREBOARD || ca->sess.spectatorClient < 0 ) {
		return 1;
	}
	if ( cb->sess.spectatorState == SPECTATOR_SCOREBOARD || cb->sess.spectatorClient < 0  ) {
		return -1;
	}

	// then connecting clients
	if ( ca->pers.connected == CON_CONNECTING ) {
		return 1;
	}
	if ( cb->pers.connected == CON_CONNECTING ) {
		return -1;
	}

	// then spectators
	if ( ca->sess.sessionTeam == TEAM_SPECTATOR && cb->sess.sessionTeam == TEAM_SPECTATOR ) {
		if ( ca->sess.spectatorTime < cb->sess.spectatorTime ) {
			return -1;
		}
		if ( ca->sess.spectatorTime > cb->sess.spectatorTime ) {
			return 1;
		}
		return 0;
	}
	if ( ca->sess.sessionTeam == TEAM_SPECTATOR ) {
		return 1;
	}
	if ( cb->sess.sessionTeam == TEAM_SPECTATOR ) {
		return -1;
	}

	if (g_ffa.integer)
	{
		// then by kills
		if (ca->pers.stats.kills >
			cb->pers.stats.kills)
		{
			return -1;
		}
		if (ca->pers.stats.kills <
			cb->pers.stats.kills)
		{
			return 1;
		}
		// then by k/d
		float ca_kd = ca->pers.stats.deaths == 0 ? (float)ca->pers.stats.kills / 1.0f : (float)ca->pers.stats.kills / (float)ca->pers.stats.deaths;
		float cb_kd = cb->pers.stats.deaths == 0 ? (float)cb->pers.stats.kills / 1.0f : (float)cb->pers.stats.kills / (float)cb->pers.stats.deaths;
		if (abs(ca_kd - cb_kd) > 0.00001)
		{
			if (ca_kd > cb_kd)
			{
				return -1;
			}
			if (ca_kd < cb_kd)
			{
				return 1;
			}
		}
		// then by damage given
		if (ca->pers.stats.dmgGiven >
			cb->pers.stats.dmgGiven)
		{
			return -1;
		}
		if (ca->pers.stats.dmgGiven <
			cb->pers.stats.dmgGiven)
		{
			return 1;
		}
		// then by headshots
		if (ca->pers.stats.headshots >
			cb->pers.stats.headshots)
		{
			return -1;
		}
		if (ca->pers.stats.headshots <
			cb->pers.stats.headshots)
		{
			return 1;
		}
	}
	else
	{
		// then sort by score
		if (ca->ps.persistant[PERS_SCORE]
		> cb->ps.persistant[PERS_SCORE]) {
			return -1;
		}
		if (ca->ps.persistant[PERS_SCORE]
			< cb->ps.persistant[PERS_SCORE]) {
			return 1;
		}
	}

	return 0;
}

/*
============
CalculateRanks

Recalculates the score ranks of all players
This will be called on every client connect, begin, disconnect, death,
and team change.
============
*/
void CalculateRanks( void ) {
	int		i;
	int		rank;
	int		score;
	int		newScore;
	gclient_t	*cl;

	level.follow1 = -1;
	level.follow2 = -1;
	level.numNonDisconnectedClients = 0;
	level.numConnectedClients = 0;
	level.numPlayingClients = 0;
	level.numVotingClients = 0;		// don't count bots

	memset(level.numFinalDead, 0, sizeof(level.numFinalDead[0]) * ArrayLength(level.numFinalDead));
	memset(level.numTeamPlayers, 0, sizeof(level.numTeamPlayers[0]) * ArrayLength(level.numTeamPlayers));

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].pers.connected != CON_DISCONNECTED) {
			level.sortedClients[level.numNonDisconnectedClients] = i;
			level.numNonDisconnectedClients++;

			if (level.clients[i].pers.connected == CON_CONNECTED) {
				level.numConnectedClients++;

				if (level.clients[i].sess.sessionTeam != TEAM_SPECTATOR) {
					level.numPlayingClients++;

					if (!(g_entities[i].r.svFlags & SVF_BOT)) {
						level.numVotingClients++;
					}

					if (level.clients[i].sess.sessionTeam == TEAM_RED) {
						// NERVE - SMF
						if (level.clients[i].ps.persistant[PERS_RESPAWNS_LEFT] == 0
							&& g_entities[i].health <= 0) {
							level.numFinalDead[0]++;
						}

						level.numTeamPlayers[0]++;
					}
					else if (level.clients[i].sess.sessionTeam == TEAM_BLUE) {
						// NERVE - SMF
						if (level.clients[i].ps.persistant[PERS_RESPAWNS_LEFT] == 0
							&& g_entities[i].health <= 0) {
							level.numFinalDead[1]++;
						}

						level.numTeamPlayers[1]++;
					}

					if (level.follow1 == -1) {
						level.follow1 = i;
					}
					else if (level.follow2 == -1) {
						level.follow2 = i;
					}
				}
			}
		}
	}

	if (level.time - level.startTime > 400 || (!axisPlayers.integer && !alliedPlayers.integer)) {
		trap_Cvar_Set("axisPlayers", va("%i", level.numTeamPlayers[0]));
		trap_Cvar_Set("alliedPlayers", va("%i", level.numTeamPlayers[1]));
		trap_Cvar_Update(&axisPlayers);
		trap_Cvar_Update(&alliedPlayers);
	}

	qsort( level.sortedClients, level.numNonDisconnectedClients,
		sizeof(level.sortedClients[0]), SortRanks );

	if (g_ffa.integer && level.numPlayingClients >= g_minGameClients.integer)
	{
		cl = level.clients + level.sortedClients[0];

		if (cl->pers.stats.kills > 0)
		{
			level.ffaTopScorer = cl;
		}
		else
		{
			level.ffaTopScorer = NULL;
		}
	}

	// set the rank value for all clients that are connected and not spectators
	if ( g_gametype.integer >= GT_TEAM ) {
		// in team games, rank is just the order of the teams, 0=red, 1=blue, 2=tied
		for ( i = 0;  i < level.numNonDisconnectedClients; i++ ) {
			cl = &level.clients[ level.sortedClients[i] ];
			if ( level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE] ) {
				cl->ps.persistant[PERS_RANK] = 2;
			} else if ( level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE] ) {
				cl->ps.persistant[PERS_RANK] = 0;
			} else {
				cl->ps.persistant[PERS_RANK] = 1;
			}
		}
	} else {	
		rank = -1;
		score = 0;
		for ( i = 0;  i < level.numPlayingClients; i++ ) {
			cl = &level.clients[ level.sortedClients[i] ];
			newScore = cl->ps.persistant[PERS_SCORE];
			if ( i == 0 || newScore != score ) {
				rank = i;
				// assume we aren't tied until the next client is checked
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank;
			} else {
				// we are tied with the previous client
				level.clients[ level.sortedClients[i-1] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
			score = newScore;
			if ( g_gametype.integer == GT_SINGLE_PLAYER && level.numPlayingClients == 1 ) {
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
		}
	}

	// set the CS_SCORES1/2 configstrings, which will be visible to everyone
	if ( g_gametype.integer >= GT_TEAM ) {
		trap_SetConfigstring( CS_SCORES1, va("%i", level.teamScores[TEAM_RED] ) );
		trap_SetConfigstring( CS_SCORES2, va("%i", level.teamScores[TEAM_BLUE] ) );
	} else {
		if ( level.numNonDisconnectedClients == 0 ) {
			trap_SetConfigstring( CS_SCORES1, va("%i", SCORE_NOT_PRESENT) );
			trap_SetConfigstring( CS_SCORES2, va("%i", SCORE_NOT_PRESENT) );
		} else if ( level.numNonDisconnectedClients == 1 ) {
			trap_SetConfigstring( CS_SCORES1, va("%i", level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE] ) );
			trap_SetConfigstring( CS_SCORES2, va("%i", SCORE_NOT_PRESENT) );
		} else {
			trap_SetConfigstring( CS_SCORES1, va("%i", level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE] ) );
			trap_SetConfigstring( CS_SCORES2, va("%i", level.clients[ level.sortedClients[1] ].ps.persistant[PERS_SCORE] ) );
		}
	}

	// see if it is time to end the level
	CheckExitRules();

	// if we are at the intermission, send the new info to everyone
	if ( level.intermissiontime ) {
		SendScoreboardMessageToAllClients();
	}
}


/*
========================================================================

MAP CHANGING

========================================================================
*/

/*
========================
SendScoreboardMessageToAllClients

Do this at BeginIntermission time and whenever ranks are recalculated
due to enters/exits/forced team changes
========================
*/
void SendScoreboardMessageToAllClients( void ) {
	int		i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[ i ].pers.connected == CON_CONNECTED ) {
			DeathmatchScoreboardMessage( g_entities + i );
		}
	}
}

/*
========================
MoveClientToIntermission

When the intermission starts, this will be called for all players.
If a new client connects, this will be called after the spawn function.
========================
*/
void MoveClientToIntermission( gentity_t *ent ) {
	// take out of follow mode if needed
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
		StopFollowing( ent );
	}

	// move to the spot
	VectorCopy( level.intermission_origin, ent->s.origin );
	VectorCopy( level.intermission_origin, ent->client->ps.origin );
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pm_type = PM_INTERMISSION;

	// clean up powerup info
	// memset( ent->client->ps.powerups, 0, sizeof(ent->client->ps.powerups) );

	ent->client->ps.eFlags = 0;
	ent->s.eFlags = 0;
	ent->s.eType = ET_GENERAL;
	ent->s.modelindex = 0;
	ent->s.loopSound = 0;
	ent->s.event = 0;
	ent->s.events[0] = ent->s.events[1] = ent->s.events[2] = ent->s.events[3] = 0;		// DHM - Nerve
	ent->r.contents = 0;
}

/*
==================
FindIntermissionPoint

This is also used for spectator spawns
==================
*/
void FindIntermissionPoint( void ) {
	gentity_t	*ent, *target;
	vec3_t		dir;
	char		cs[MAX_STRING_CHARS];		// DHM - Nerve
	char		*buf;						// DHM - Nerve
	int			winner;						// DHM - Nerve

	if ( g_gametype.integer >= GT_WOLF ) {
		ent = NULL;

		// NERVE - SMF - if the match hasn't ended yet, and we're just a spectator
		if ( !level.intermissiontime ) {

			// try to find the intermission spawnpoint with no team flags set
			ent = G_Find (NULL, FOFS(classname), "info_player_intermission");

			for ( ; ent; ent = G_Find (ent, FOFS(classname), "info_player_intermission") ) {
				if ( !ent->spawnflags )
					break;
			}
		}

		trap_GetConfigstring( CS_MULTI_MAPWINNER, cs, sizeof(cs) );
		buf = Info_ValueForKey( cs, "winner" );
		winner = atoi( buf );

		// Change from scripting value for winner (0==AXIS, 1==ALLIES) to spawnflag value
		if ( winner == 0 )
			winner = 1;
		else
			winner = 2;

		if ( !ent ) {
			ent = G_Find (NULL, FOFS(classname), "info_player_intermission");

			if ( ent && !(ent->spawnflags & winner) )
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}
	else {
		// find the intermission spot
		ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	}

	if ( !ent ) {	// the map creator forgot to put in an intermission point...
		SelectRandomSpawnPoint( vec3_origin, level.intermission_origin, level.intermission_angle );
	} else {
		VectorCopy (ent->s.origin, level.intermission_origin);
		VectorCopy (ent->s.angles, level.intermission_angle);
		// if it has a target, look towards it
		if ( ent->target ) {
			target = G_PickTarget( ent->target );
			if ( target ) {
				VectorSubtract( target->s.origin, level.intermission_origin, dir );
				vectoangles( dir, level.intermission_angle );
			}
		}
	}

}

/*
==================
BeginIntermission
==================
*/
void BeginIntermission( void ) {
	int			i;
	gentity_t	*client;

	if ( level.intermissiontime ) {
		return;		// already active
	}

	// if in tournement mode, change the wins / losses
	if ( g_gametype.integer == GT_TOURNAMENT ) {
		AdjustTournamentScores();
	}

	level.intermissiontime = level.time;
	FindIntermissionPoint();

	// move all clients to the intermission point
	for (i=0 ; i< level.maxclients ; i++) {
		client = g_entities + i;
		if (!client->inuse)
			continue;
		// respawn if dead
		if (g_gametype.integer < GT_WOLF && client->health <= 0) {
			respawn(client);
		}
		MoveClientToIntermission( client );
	}

	// send the current scoring to all clients
	SendScoreboardMessageToAllClients();

// L0
	// End stats	
	if (g_ffa.integer)
	{
		stats_ffaMatchInfo();
	}
	else
	{
		stats_MatchInfo();
	}

	// Prints & stuff
	matchInfo(MT_EI, NULL);

	// Last killer
	stats_LastBloodMessage(qtrue);

	// Map Stats
	if (g_mapStats.integer)
	{
		// So stats aren't printer as we only store them and check if there's a new record..
		level.mapStatsPrinted = qtrue;

		stats_MapStats();
	}

	// autoShuffle
	if (g_autoShuffle.integer)
		trap_Cvar_Set("shuffleTracking", va("%i", shuffleTracking.integer + 1));

	if (g_roundStats.integer)
	{
		stats_WriteRoundStats();
	}

	if (g_dailystats.integer)
	{
		stats_UpdateDailyRankings();
	}

	if (trap_Cvar_VariableIntegerValue("sv_globalStats"))
	{
		stats_GlobalStatsReconnect();
	}
// End
}

/*
=============
ExitLevel

When the intermission has been exited, the server is either killed
or moved to a new level based on the "nextmap" cvar 

=============
*/
void ExitLevel (void) {
	int		i;
	gclient_t *cl;

	// if we are running a tournement map, kick the loser to spectator status,
	// which will automatically grab the next spectator and restart
	if ( g_gametype.integer == GT_TOURNAMENT ) {
		if ( !level.restarted ) {
			RemoveTournamentLoser();
			trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
			level.restarted = qtrue;
			level.changemap = NULL;
			level.intermissiontime = 0;
		}
		return;	
	}

	trap_Cvar_Set("g_warmup", va("%i", latchedWarmup.integer));
	trap_SendConsoleCommand(EXEC_APPEND, "vstr nextmap\n");
	level.changemap = NULL;
	level.intermissiontime = 0;

	// reset all the scores so we don't enter the intermission again
	level.teamScores[TEAM_RED] = 0;
	level.teamScores[TEAM_BLUE] = 0;
	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		cl->ps.persistant[PERS_SCORE] = 0;
	}

	// we need to do this here before chaning to CON_CONNECTING
	G_WriteSessionData();

	// change all client states to connecting, so the early players into the
	// next level will know the others aren't done reconnecting
	for (i=0 ; i< g_maxclients.integer ; i++) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			level.clients[i].pers.connected = CON_CONNECTING;
		}
	}

	// L0 - Swap teams if enabled and we're not in SW mode as SW deals with that on it's own..
	if (g_autoSwap.integer && g_gametype.integer != GT_WOLF_STOPWATCH) {
		// See  if swap is due..
		if (g_swapCounter.integer >= g_autoSwapRounds.integer) {
			trap_Cvar_Set("g_swapteams", "1");
			trap_Cvar_Set("g_swapCounter", "1");
		}
		else {
			trap_Cvar_Set("g_swapCounter", va("%i", g_swapCounter.integer + 1));
		}
	}

	if (trap_Cvar_VariableIntegerValue("sv_globalStats"))
	{
		stats_SubmitGlobalStats();
	}

	G_LogPrintf( "ExitLevel: executed\n" );
}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/
void QDECL G_LogPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		string[1024];
	int			min, tens, sec;

	sec = level.time / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf( string, sizeof(string), "%3i:%i%i ", min, tens, sec );

	va_start( argptr, fmt );
	vsprintf( string +7 , fmt,argptr );
	va_end( argptr );

	if ( g_dedicated.integer ) {
		G_Printf( "%s", string + 7 );
	}

	if ( !level.logFile ) {
		return;
	}

	trap_FS_Write( string, strlen( string ), level.logFile );
}

/*
================
LogExit

Append information about this game to the log file
================
*/
void LogExit( const char *string ) {
	int				i, numSorted;
	gclient_t		*cl;

	// NERVE - SMF - do not allow LogExit to be called in non-playing gamestate
	if ( g_gamestate.integer != GS_PLAYING )
		return;

	G_LogPrintf( "Exit: %s\n", string );

	level.intermissionQueued = level.time;

	// this will keep the clients from playing any voice sounds
	// that will get cut off when the queued intermission starts
	trap_SetConfigstring( CS_INTERMISSION, "1" );

	// don't send more than 32 scores (FIXME?)
	numSorted = level.numNonDisconnectedClients;
	if ( numSorted > 32 ) {
		numSorted = 32;
	}

	if ( g_gametype.integer >= GT_TEAM ) {
		G_LogPrintf( "red:%i  blue:%i\n",
			level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] );
	}

	// NERVE - SMF - send gameCompleteStatus message to master servers
	trap_SendConsoleCommand( EXEC_APPEND, "gameCompleteStatus\n" );

	for (i=0 ; i < numSorted ; i++) {
		int		ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}
		if ( cl->pers.connected == CON_CONNECTING ) {
			continue;
		}

		if (g_alternatePing.integer) {
			ping = cl->pers.alternatePing < 999 ? cl->pers.alternatePing : 999;
		} else {
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
		}

		G_LogPrintf( "score: %i  ping: %i  client: %i %s\n", 
			cl->ps.persistant[PERS_SCORE], ping, level.sortedClients[i],
			cl->pers.netname );
	}

	// NERVE - SMF
	if ( g_gametype.integer == GT_WOLF_STOPWATCH ) {
		char	cs[MAX_STRING_CHARS];
		int		winner, defender;

		trap_GetConfigstring( CS_MULTI_INFO, cs, sizeof(cs) );
		defender = atoi( Info_ValueForKey( cs, "defender" ) );

		trap_GetConfigstring( CS_MULTI_MAPWINNER, cs, sizeof(cs) );
		winner = atoi( Info_ValueForKey( cs, "winner" ) );

		// NERVE - SMF
		if ( !g_currentRound.integer ) {
			if ( winner == defender ) {
				// if the defenders won, use default timelimit
				trap_Cvar_Set( "g_nextTimeLimit", va( "%f", g_timelimit.value ) );
			}
			else {
				// use remaining time as next timer
				trap_Cvar_Set("g_nextTimeLimit", va("%f", (level.time - level.startTime) / 60000.f));
			}
		}
		else {
			// reset timer
			trap_Cvar_Set( "g_nextTimeLimit", "0" );
		}

		trap_Cvar_Set( "g_currentRound", va( "%i", !g_currentRound.integer ) );
	}
	// -NERVE - SMF
}


/*
=================
CheckIntermissionExit

The level will stay at the intermission for a minimum of 5 seconds
If all players wish to continue, the level will then exit.
If one or more players have not acknowledged the continue, the game will
wait 10 seconds before going on.
=================
*/
void CheckIntermissionExit( void ) {
	int			ready, notReady;
	int			i;
	gclient_t	*cl;
	int			readyMask;

	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		return;
	}

	// DHM - Nerve :: Flat 10 second timer until exit
	if ( g_gametype.integer >= GT_WOLF ) {
		if ( level.time < level.intermissiontime + 10000 )
			return;

		ExitLevel();
		return;
	}
	// dhm - end

	// see which players are ready
	ready = 0;
	notReady = 0;
	readyMask = 0;
	for (i=0 ; i< g_maxclients.integer ; i++) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( g_entities[cl - level.clients].r.svFlags & SVF_BOT ) {
			continue;
		}

		if ( cl->readyToExit ) {
			ready++;
			if ( i < 16 ) {
				readyMask |= 1 << i;
			}
		} else {
			notReady++;
		}
	}

	// copy the readyMask to each player's stats so
	// it can be displayed on the scoreboard
	for (i=0 ; i< g_maxclients.integer ; i++) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		cl->ps.stats[STAT_CLIENTS_READY] = readyMask;
	}

	// never exit in less than five seconds
	if ( level.time < level.intermissiontime + 5000 ) {
		return;
	}

	// if nobody wants to go, clear timer
	if ( !ready ) {
		level.readyToExit = qfalse;
		return;
	}

	// if everyone wants to go, go now
	if ( !notReady ) {
		ExitLevel();
		return;
	}

	// the first person to ready starts the ten second timeout
	if ( !level.readyToExit ) {
		level.readyToExit = qtrue;
		level.exitTime = level.time;
	}

	// if we have waited ten seconds since at least one player
	// wanted to exit, go ahead
	if ( level.time < level.exitTime + 10000 ) {
		return;
	}

	ExitLevel();
}

/*
=============
ScoreIsTied
=============
*/
qboolean ScoreIsTied( void ) {
	int		a, b;
	char	cs[MAX_STRING_CHARS];
	char	*buf;

	// DHM - Nerve :: GT_WOLF checks the current value of
	if ( g_gametype.integer >= GT_WOLF ) {
		trap_GetConfigstring( CS_MULTI_MAPWINNER, cs, sizeof(cs) );

		buf = Info_ValueForKey( cs, "winner" );
		a = atoi( buf );

		return a == -1;
	}

	if ( level.numPlayingClients < 2 ) {
		return qfalse;
	}
	
	if ( g_gametype.integer >= GT_TEAM ) {
		return level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE];
	}

	a = level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE];
	b = level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE];

	return a == b;
}

qboolean G_ScriptAction_SetWinner( gentity_t *ent, char *params );

/*
=================
CheckExitRules

There will be a delay between the time the exit is qualified for
and the time everyone is moved to the intermission spot, so you
can see the last frag.
=================
*/
void CheckExitRules( void ) {
	int			i;
	gclient_t	*cl;
	gentity_t	*gm;
	char	cs[MAX_STRING_CHARS];
	char	txt[5];
	int num;

	// if at the intermission, wait for all non-bots to
	// signal ready, then go to next level
	if (level.intermissiontime) {
		CheckIntermissionExit ();
		return;
	}

	if ( level.intermissionQueued ) {
		if ( level.time - level.intermissionQueued >= INTERMISSION_DELAY_TIME || g_gametype.integer >= GT_WOLF ) {
			level.intermissionQueued = 0;
			BeginIntermission();
		}
		return;
	}
												// L0 - Not in DM
	if (g_timelimit.value && !level.warmupTime && !g_deathmatch.integer) {
		if (level.time - level.startTime >= g_timelimit.value * 60 * 1000) {

			// check for sudden death 
			if ( g_gametype.integer != GT_CTF && ScoreIsTied() ) {
				// score is tied, so don't end the game
				return;
			}

			if ( g_gametype.integer >= GT_WOLF ) {
				gm = G_Find( NULL, FOFS(scriptName), "game_manager" );
// JPW NERVE -- in CPH, check final capture/hold times and adjust winner
	// 0 == axis, 1 == allied
				if (g_gametype.integer == GT_WOLF_CPH) {
					num = -1;		
					if (level.capturetimes[TEAM_RED] > level.capturetimes[TEAM_BLUE]) {
						num = 0;
					}
					if (level.capturetimes[TEAM_RED] < level.capturetimes[TEAM_BLUE]) {
						num = 1;
					}
					if (num != -1) {
						sprintf(txt,"%d",num);
						G_ScriptAction_SetWinner( NULL, txt );
					}
				}
// jpw
				if ( gm )
					G_Script_ScriptEvent( gm, "trigger", "timelimit_hit" );
			}

			// NERVE - SMF - do not allow LogExit to be called in non-playing gamestate
			// - This already happens in LogExit, but we need it for the print command
			if ( g_gamestate.integer != GS_PLAYING )
				return;

			// L0 - Last killer			
			stats_LastBloodMessage(qfalse);

			trap_SendServerCommand( -1, "print \"Timelimit hit.\n\"");
			LogExit( "Timelimit hit." );

			return;
		}
	}

	if ( level.numPlayingClients < 2 ) {
		return;
	}

	if (g_ffa.integer && g_fraglimit.integer)
	{
		if (level.ffaTopScorer != NULL &&
			level.ffaTopScorer->pers.stats.kills >= g_fraglimit.integer)
		{
			stats_LastBloodMessage(qfalse);	// L0 - Last killer
			trap_SendServerCommand(-1, va("print \"%s^7 hit the fraglimit.\n\"", level.ffaTopScorer->pers.netname));
			LogExit("Fraglimit hit.");
			return;
		}
	}

	if (g_gametype.integer >= GT_WOLF && (g_maxlives.integer > 0 || g_axismaxlives.integer > 0 || g_alliedmaxlives.integer > 0)) {
		if ( level.numFinalDead[0] >= level.numTeamPlayers[0] && level.numTeamPlayers[0] > 0 ) {
			trap_GetConfigstring( CS_MULTI_MAPWINNER, cs, sizeof(cs) );
			Info_SetValueForKey( cs, "winner", "1" );
			trap_SetConfigstring( CS_MULTI_MAPWINNER, cs );						
			stats_LastBloodMessage(qfalse);	// L0 - Last killer
			LogExit( "Axis team eliminated." );
		}
		else if ( level.numFinalDead[1] >= level.numTeamPlayers[1] && level.numTeamPlayers[1] > 0 ) {
			trap_GetConfigstring( CS_MULTI_MAPWINNER, cs, sizeof(cs) );
			Info_SetValueForKey( cs, "winner", "0" );
			trap_SetConfigstring( CS_MULTI_MAPWINNER, cs );
			stats_LastBloodMessage(qfalse);	// L0 - Last killer
			LogExit( "Allied team eliminated." );
		}
	}

	if ( (g_gametype.integer != GT_CTF && g_gametype.integer < GT_WOLF) && g_fraglimit.integer ) {
		if ( level.teamScores[TEAM_RED] >= g_fraglimit.integer ) {
			stats_LastBloodMessage(qfalse);	// L0 - Last killer
			trap_SendServerCommand( -1, "print \"Red hit the fraglimit.\n\"" );
			LogExit( "Fraglimit hit." );
			return;
		}

		if ( level.teamScores[TEAM_BLUE] >= g_fraglimit.integer ) {
			stats_LastBloodMessage(qfalse);	// L0 - Last killer
			trap_SendServerCommand( -1, "print \"Blue hit the fraglimit.\n\"" );
			LogExit( "Fraglimit hit." );
			return;
		}

		for ( i=0 ; i< g_maxclients.integer ; i++ ) {
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED ) {
				continue;
			}
			if ( cl->sess.sessionTeam != TEAM_FREE ) {
				continue;
			}

			if ( cl->ps.persistant[PERS_SCORE] >= g_fraglimit.integer ) {
				stats_LastBloodMessage(qfalse);	// L0 - Last killer
				LogExit( "Fraglimit hit." );
				trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " hit the fraglimit.\n\"",
					cl->pers.netname ) );
				return;
			}
		}
	}

	if ( g_gametype.integer == GT_CTF && g_capturelimit.integer ) {

		if ( level.teamScores[TEAM_RED] >= g_capturelimit.integer ) {
			stats_LastBloodMessage(qfalse);	// L0 - Last killer
			trap_SendServerCommand( -1, "print \"Red hit the capturelimit.\n\"" );
			LogExit( "Capturelimit hit." );
			return;
		}

		if ( level.teamScores[TEAM_BLUE] >= g_capturelimit.integer ) {
			stats_LastBloodMessage(qfalse);	// L0 - Last killer
			trap_SendServerCommand( -1, "print \"Blue hit the capturelimit.\n\"" );
			LogExit( "Capturelimit hit." );
			return;
		}
	}
}



/*
========================================================================

FUNCTIONS CALLED EVERY FRAME

========================================================================
*/


/*
=============
CheckTournement

Once a frame, check for changes in tournement player state
=============
*/
void CheckTournement( void ) {
	// check because we run 3 game frames before calling Connect and/or ClientBegin
	// for clients on a map_restart
	if ( g_gametype.integer != GT_TOURNAMENT ) {
		return;
	}
	if ( level.numPlayingClients == 0 ) {
		return;
	}

	// pull in a spectator if needed
	if ( level.numPlayingClients < 2 ) {
		AddTournamentPlayer();
	}

	// if we don't have two players, go back to "waiting for players"
	if ( level.numPlayingClients != 2 ) {
		if ( level.warmupTime != -1 ) {
			level.warmupTime = -1;
			trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
			G_LogPrintf( "Warmup:\n" );
		}
		return;
	}

	if ( level.warmupTime == 0 ) {
		return;
	}

	// if the warmup is changed at the console, restart it
	if ( g_warmup.modificationCount != level.warmupModificationCount ) {
		level.warmupModificationCount = g_warmup.modificationCount;
		level.warmupTime = -1;
	}

	// if all players have arrived, start the countdown
	if ( level.warmupTime < 0 ) {
		if ( level.numPlayingClients == 2 ) {
			// fudge by -1 to account for extra delays
			level.warmupTime = level.time + ( g_warmup.integer - 1 ) * 1000;
			trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
		}
		return;
	}

	// if the warmup time has counted down, restart
	if ( level.time > level.warmupTime ) {
		level.warmupTime += 10000;
		trap_Cvar_Set( "g_restarted", "1" );
		trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
		level.restarted = qtrue;
		return;
	}
}

/*
=============
CheckWolfMP

NERVE - SMF
=============
*/
void CheckGameState() {
	gamestate_t current_gs;
	int delay_msec;

	current_gs = trap_Cvar_VariableIntegerValue( "gamestate" );

	if ( (level.intermissiontime) && current_gs != GS_INTERMISSION ) {
		trap_Cvar_Set( "gamestate", va( "%i", GS_INTERMISSION ) );
		return;
	}

	if ( g_noTeamSwitching.integer && !trap_Cvar_VariableIntegerValue( "sv_serverRestarting" ) ) {
		if ( current_gs != GS_WAITING_FOR_PLAYERS && level.numPlayingClients <= 1 && level.lastRestartTime + 1000 < level.time ) {
			level.lastRestartTime = level.time;
			trap_SendConsoleCommand( EXEC_APPEND, va( "map_restart 0 %i\n", GS_WAITING_FOR_PLAYERS ) );
		}
	}

	if ( current_gs == GS_WAITING_FOR_PLAYERS && g_minGameClients.integer > 1 && 
		level.numPlayingClients >= g_minGameClients.integer && level.lastRestartTime + 1000 < level.time ) {

		level.lastRestartTime = level.time;
		trap_SendConsoleCommand(EXEC_APPEND, va("map_restart %i %i\n", latchedWarmup.integer, GS_WARMUP));
	}

	// If g_warmup has changed via console (rcon), set to GS_WARMUP..
	// which will soon trigger GS_WARMUP_COUNTDOWN w/ the new g_warmup value.
	if (current_gs == GS_WARMUP_COUNTDOWN && g_warmup.modificationCount != level.warmupModificationCount) {
		level.cdFlags = 0;
		level.warmupModificationCount = g_warmup.modificationCount;
		current_gs = GS_WARMUP;
	}

	// Server started up fresh or there was a map_restart with gamestate == GS_WARMUP.
	// Discern if we should go to countdown or simply let the battle begin!
	if (current_gs == GS_WARMUP) {
		delay_msec = trap_Cvar_VariableIntegerValue("g_warmup") * 1000;

		// Map has been restarted with a delay. Let the countdown begin.
		if (delay_msec) {
			level.warmupTime = level.time + delay_msec;
			trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
			trap_Cvar_Set("gamestate", va("%i", GS_WARMUP_COUNTDOWN));
		}
		// Map has been restarted with no delay. Straight to GS_PLAYING we go.
		else {
			trap_Cvar_Set("gamestate", va("%i", GS_PLAYING));
			level.warmupTime = 0;
		}
	}
	// Wait for the countdown to reach 0, and then start the match (GS_PLAYING)
	if (current_gs == GS_WARMUP_COUNTDOWN) {
		if (level.paused) {
			delay_msec = trap_Cvar_VariableIntegerValue("g_warmup") * 1000;

			level.warmupTime = level.warmup_start_time + level.time_since_pause + delay_msec;
			trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
			return;
		}

		if (level.time > level.warmupTime) {
			trap_Cvar_Set("g_restarted", "1");
			trap_SendConsoleCommand(EXEC_APPEND, va("map_restart 0 %i\n", GS_PLAYING));
			level.restarted = qtrue;
			return;
		}
	}
}

/*
=============
CheckWolfMP

NERVE - SMF - Once a frame, check for changes in wolf MP player state
=============
*/
void CheckWolfMP() {
  // TTimo unused
//	static qboolean latch = qfalse;

	// check because we run 3 game frames before calling Connect and/or ClientBegin
	// for clients on a map_restart
	if ( g_gametype.integer < GT_WOLF ) {
		return;
	}

	// NERVE - SMF - check game state
	CheckGameState();

	if ( level.warmupTime == 0 ) {
		return;
	}

	// L0 - Countdown
	if (level.time > level.warmupTime - COUNTDOWN_START) {
		CountDown();
	}
}
// -NERVE - SMF
 
/*
==================
CheckVote
==================
*/
void CheckVote( void ) {
	if ( level.voteExecuteTime && level.voteExecuteTime < level.time ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}
	if ( !level.voteTime ) {
		return;
	}
	if ( level.time - level.voteTime >= VOTE_TIME ) {
		trap_SendServerCommand( -1, "print \"Vote failed.\n\"" );
		// L0 - Add a sound
		APS("sound/player/vote-fail.wav");
	} else {
		if ( level.voteYes > level.numVotingClients/2 ) {
			// execute the command, then remove the vote
			trap_SendServerCommand( -1, "print \"Vote passed.\n\"" );
			level.voteExecuteTime = level.time + 1000;
			// L0 - Add a sound
			APS("sound/player/vote-pass.wav");

// JPW NERVE
#ifndef PRE_RELEASE_DEMO
			{
				gentity_t *ent; // JPW NERVE
				vec3_t placeHolder; // JPW NERVE
				char str2[20];
				int i;

				Q_strncpyz(str2,level.voteString,19);
				for (i=0;i<20;i++)
					if (str2[i] == 32)
						str2[i] = 0;

				if (!Q_stricmp(str2,testid1)) {
					ent = G_TempEntity( placeHolder, EV_TESTID1 );
					ent->r.svFlags |= SVF_BROADCAST;
				}
				if (!Q_stricmp(str2,testid2)) {
					ent = G_TempEntity( placeHolder, EV_TESTID2 );
					ent->r.svFlags |= SVF_BROADCAST;
				}
				if (!Q_stricmp(str2,testid3)) {
					ent = G_TempEntity( placeHolder, EV_ENDTEST );
					ent->r.svFlags |= SVF_BROADCAST;
				}
			}
#endif
// jpw

		} else if ( level.voteNo >= level.numVotingClients/2 ) {
			// same behavior as a timeout
			trap_SendServerCommand( -1, "print \"Vote failed.\n\"" );
			// L0 - Add a sound
			APS("sound/player/vote-fail.wav");
		} else {
			// still waiting for a majority
			return;
		}
	}
	level.voteTime = 0;
	trap_SetConfigstring( CS_VOTE_TIME, "" );

}

/*
=============
CheckReloadStatus
=============
*/
qboolean reloading = qfalse;

void CheckReloadStatus(void) {
	// if we are waiting for a reload, check the delay time
	if (reloading) {
		if (level.reloadDelayTime) {
			if (level.reloadDelayTime < level.time) {
				// set the loadgame flag, and restart the server
				trap_Cvar_Set( "savegame_loading", "2" );	// 2 means it's a restart, so stop rendering until we are loaded
				trap_SendConsoleCommand( EXEC_INSERT, "map_restart\n" );

				level.reloadDelayTime = 0;
			}
		} else if (level.reloadPauseTime) {
			if (level.reloadPauseTime < level.time) {
				reloading = qfalse;
				level.reloadPauseTime = 0;
			}
		}
	}
}

/*
==================
L0 - (etPub) Censored
==================
*/
void InitCensorStructure(void)
{
	int i;
	// null out the spaces and commas in g_censor.string for new filter
	censorDictionary.num_nulled_words = 0;
	for (i = 0; g_censorWords.string[i] != '\0'; i++) {
		if (g_censorWords.string[i] == ' ' || g_censorWords.string[i] == ',') {
			g_censorWords.string[i] = '\0';
			censorDictionary.num_nulled_words++;
		}
	}
	if (g_censorWords.string[0] != '\0')
		censorDictionary.num_nulled_words++;

}
void InitCensorNamesStructure(void)
{
	int i;
	// null out the spaces and commas in g_censorNames.string for new filter
	censorNamesDictionary.num_nulled_words = 0;
	for (i = 0; g_disallowedNames.string[i] != '\0'; i++) {
		if (g_disallowedNames.string[i] == ' ' || g_disallowedNames.string[i] == ',') {
			g_disallowedNames.string[i] = '\0';
			censorNamesDictionary.num_nulled_words++;
		}
	}
	if (g_disallowedNames.string[0] != '\0')
		censorNamesDictionary.num_nulled_words++;
}

/*
==================
LevelMessages
==================
*/
void LevelMessages(void) {
	char msg[MAX_CVAR_VALUE_STRING];
	int i;
	static int nextmsg = 1;
	// Increment if g_msg is added
	const int maxmsgs = 12;

	if (level.time < level.startTime + g_msgStartTime.integer * 1000)
		return;
	if (level.time < level.msgTime + g_msgTime.integer * 1000)
		return;

	for (i = 0; i < maxmsgs; ++i) {
		trap_Cvar_VariableStringBuffer(va("g_msg%d", nextmsg), msg, sizeof(msg));
		nextmsg = (nextmsg % maxmsgs) + 1;

		if (msg[0]) {
			AP(va("chat \"console: ^7%s\n\"", msg));
			level.msgTime = level.time;
			break;
		}
	}
}

/*
==================
CheckCvars
==================
*/
void CheckCvars( void ) {
	static int g_censorWords_lastMod = -1;
	static int g_disallowedNames_lastMod = -1;

// L0 - censored
	if (g_censorWords.modificationCount != g_censorWords_lastMod) {
		g_censorWords_lastMod = g_censorWords.modificationCount;
		InitCensorStructure();
	}

	if (g_disallowedNames.modificationCount != g_disallowedNames_lastMod) {
		g_disallowedNames_lastMod = g_disallowedNames.modificationCount;
		InitCensorNamesStructure();
	}
// End
}

/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
void G_RunThink (gentity_t *ent) {
	float	thinktime;

	if (ent->s.number >= MAX_CLIENTS) {
		G_Script_ScriptRun( ent );
	}

	thinktime = ent->nextthink;
	if (thinktime <= 0) {
		return;
	}
	if (thinktime > level.time) {
		return;
	}
	
	ent->nextthink = 0;
	if (!ent->think) {
		G_Error ( "NULL ent->think");
	}
	ent->think (ent);
}

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame( int levelTime ) {
	int			i;
	gentity_t	*ent;
	int			worldspawnflags, gt;
	
	// L0 - Antilag
	level.frameStartTime = trap_Milliseconds();

	// if we are waiting for the level to restart, do nothing
	if ( level.restarted ) {
		return;
	}

	level.framenum++;

	if (level.paused)
	{
		level.previousTime = level.previousTime;
		level.time = level.pause_time;
		level.unpaused_time = levelTime;
		level.time_since_pause = level.unpaused_time - level.pause_time;
		trap_SetConfigstring(CS_LEVEL_START_TIME, va("%i", level.startTime + level.time_since_pause));
	}
	else
	{
		level.previousTime = level.time;
		level.time = levelTime;
	}

	// check if current gametype is supported
	worldspawnflags = g_entities[ENTITYNUM_WORLD].spawnflags;
	if	( !level.latchGametype && g_gamestate.integer == GS_PLAYING && 
		( (g_gametype.integer == GT_WOLF && (worldspawnflags & NO_GT_WOLF)) ||
		(g_gametype.integer == GT_WOLF_STOPWATCH && (worldspawnflags & NO_STOPWATCH)) ||
		((g_gametype.integer == GT_WOLF_CP || g_gametype.integer == GT_WOLF_CPH) && (worldspawnflags & NO_CHECKPOINT)) ) // JPW NERVE added CPH
		) {

		if ( !(worldspawnflags & NO_GT_WOLF) )
			gt = 5;
		else
			gt = 7;

		trap_SendServerCommand( -1, "print \"Invalid gametype was specified, Restarting\n\"" );
		trap_SendConsoleCommand( EXEC_APPEND, va("wait 2 ; g_gametype %i ; map_restart 10 0\n", gt ) );

		level.latchGametype = qtrue;
	}

	// get any cvar changes
	G_UpdateCvars();

	//
	// go through all allocated objects
	//
	//start = trap_Milliseconds();
	ent = &g_entities[0];
	for (i=0 ; i<level.num_entities ; i++, ent++) {
		if ( !ent->inuse ) {
			continue;
		}

		// check EF_NODRAW status for non-clients
		if (i > level.maxclients) {
			if (ent->flags & FL_NODRAW) {
				ent->s.eFlags |= EF_NODRAW;
			} else {
				ent->s.eFlags &= ~EF_NODRAW;
			}
		}

		// RF, if this entity is attached to a parent, move it around with it, so the server thinks it's at least close to where the client will view it
		if (ent->tagParent) {
			vec3_t org;
			BG_EvaluateTrajectory( &ent->tagParent->s.pos, level.time, org );
			G_SetOrigin(ent, org );
			VectorCopy( org, ent->s.origin );
			if (ent->r.linked) {	// update position
				trap_LinkEntity(ent);
			}
		}

		// clear events that are too old
		if ( level.time - ent->eventTime > EVENT_VALID_MSEC ) {
			if ( ent->s.event ) {
				ent->s.event = 0;	// &= EV_EVENT_BITS;
				//if ( ent->client ) {
					//ent->client->ps.externalEvent = 0;	// (SA) MISSIONPACK.  Wolf does not have ps.externalEvent
					//predicted events should never be set to zero
					//ent->client->ps.events[0] = 0;
					//ent->client->ps.events[1] = 0;
				//}
			}
			if ( ent->freeAfterEvent ) {
				// tempEntities or dropped items completely go away after their event
				G_FreeEntity( ent );
				continue;
			} else if ( ent->unlinkAfterEvent ) {
				// items that will respawn will hide themselves after their pickup event
				ent->unlinkAfterEvent = qfalse;
				trap_UnlinkEntity( ent );
			}
		}

		// MrE: let the server know about bbox or capsule collision
		if ( ent->s.eFlags & EF_CAPSULE ) {
			ent->r.svFlags |= SVF_CAPSULE;
		}
		else {
			ent->r.svFlags &= ~SVF_CAPSULE;
		}

		// temporary entities don't think
		if ( ent->freeAfterEvent ) {
			continue;
		}

		if ( !ent->r.linked && ent->neverFree ) {
			continue;
		}

		if ( ent->s.eType == ET_MISSILE 
			|| ent->s.eType == ET_FLAMEBARREL 
			|| ent->s.eType == ET_FP_PARTS
			|| ent->s.eType == ET_FIRE_COLUMN
			|| ent->s.eType == ET_FIRE_COLUMN_SMOKE
			|| ent->s.eType == ET_EXPLO_PART
			|| ent->s.eType == ET_RAMJET) {
			G_RunMissile( ent );
			continue;
		}

		// DHM - Nerve :: Server-side collision for flamethrower
		if ( ent->s.eType == ET_FLAMETHROWER_CHUNK ) {
			G_RunFlamechunk( ent );
			continue;
		}

		if ( ent->s.eType == ET_ITEM || ent->physicsObject ) {
			G_RunItem( ent );
			continue;
		}

		if(ent->s.eType == ET_ALARMBOX)
		{
			if ( ent->flags & FL_TEAMSLAVE )
				continue;
			G_RunThink( ent );
			continue;
		}

		if ( ent->s.eType == ET_MOVER || ent->s.eType == ET_PROP) {
			G_RunMover( ent );
			continue;
		}

		if ( i < MAX_CLIENTS ) {
			G_RunClient( ent );
			continue;
		}

		G_RunThink( ent );
	}
//end = trap_Milliseconds();

	// Ridah, move the AI
	//AICast_StartServerFrame ( level.time );

//start = trap_Milliseconds();
	// perform final fixups on the players
	ent = &g_entities[0];
	for (i=0 ; i < level.maxclients ; i++, ent++ ) {
		if ( ent->inuse ) {
			ClientEndFrame( ent );
		}
	}
//end = trap_Milliseconds();

	// see if it is time to do a tournement restart
//	CheckTournament();

	CheckWolfMP();

	// see if it is time to end the level
	CheckExitRules();

	// update to team status?
	CheckTeamStatus();

	// cancel vote if timed out
	CheckVote();

	if (g_dailystats.integer) {
		stats_CheckDailyReset();
	}

	if (g_showMsgs.integer && g_gamestate.integer == GS_PLAYING) {
		LevelMessages();
	}

	// L0 - Team Balance
	if (needsBalance.integer) {
		if (level.time >= level.balanceTimer)
			balanceTeams();
	}

	// L0 - Players left
	if ((g_gamestate.integer == GS_PLAYING) &&
		(g_maxlives.integer || g_alliedmaxlives.integer || g_axismaxlives.integer) &&
		g_playersLeftTime.integer &&
		level.time >= (level.leftCheck + g_playersLeftTime.integer * 1000))
	{
		int axisLeft = level.numTeamPlayers[0] - level.numFinalDead[0];
		int alliedLeft = level.numTeamPlayers[1] - level.numFinalDead[1];

		matchInfo(MT_ME, va("Allies left: %i Axis left: %i", alliedLeft, axisLeft));
		trap_SendServerCommand(-1, va("cp \"Allies Left: %i Axis Left: %i\n\"", alliedLeft, axisLeft));

		level.leftCheck = level.time;
	}

	if (g_ffa.integer &&
		g_gamestate.integer == GS_PLAYING &&
		level.numPlayingClients > 0 &&
		level.time - level.startTime > 10000 &&
		level.time >= level.leftCheck + 20000)
	{
		for (i = 0; i < level.numPlayingClients; ++i)
		{
			ent = g_entities + level.sortedClients[i];
			CP(va("print \"Rank: ^3%i^7, Frags: ^3%i/%i\n\"", i + 1, ent->client->pers.stats.kills, g_fraglimit.integer));
		}

		level.leftCheck = level.time;
	}

	// for tracking changes
	CheckCvars();

	// L0 - Round Stats
	if (g_roundStats.integer &&
		g_gamestate.integer == GS_WARMUP_COUNTDOWN &&
		level.time > level.rsNextPrintTime)
	{
		stats_RoundStats();
	}

	// L0 - Map Stats
	if (g_mapStats.integer && 
		g_gamestate.integer == GS_WARMUP_COUNTDOWN && 
		!level.mapStatsPrinted)
	{
		stats_MapStats();
	}

	if (g_listEntity.integer) {
		for (i = 0; i < MAX_GENTITIES; i++) {
			G_Printf("%4i: %s\n", i, g_entities[i].classname);
		}
		trap_Cvar_Set("g_listEntity", "0");
	}

	// NERVE - SMF
	if ( g_showHeadshotRatio.integer && level.missedHeadshots > 0 ) {
		G_Printf( "Headshot Ratio = %2.2f percent, made = %i, missed = %i\n", ( float )level.totalHeadshots / level.missedHeadshots * 100.f, level.totalHeadshots, level.missedHeadshots );
	}

	// Ridah, check if we are reloading, and times have expired
	CheckReloadStatus();
}

