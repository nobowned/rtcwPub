#ifndef __SHARED_H
#define __SHARED_H

// g_local.h -- local definitions for game module

#include "q_shared.h"
#include "bg_public.h"
#include "g_public.h"
#include "g_stats.h"


//==================================================================

// the "gameversion" client command will print this plus compile date
//----(SA) Wolfenstein
#define	GAMEVERSION	"rtcw^1Pub"
// done.

#define BODY_QUEUE_SIZE		8

#define INFINITE			1000000

#define	FRAMETIME			100					// msec
#define	EVENT_VALID_MSEC	300
#define	CARNAGE_REWARD_TIME	3000
#define REWARD_SPRITE_TIME	2000

#define	INTERMISSION_DELAY_TIME	1000

#define MG42_MULTIPLAYER_HEALTH 350				// JPW NERVE
#define MG42_FIXED_HEALTH_MULT 0.753f
#define STRINGIFY(x) #x
#define STRINGIFYVAL(x) STRINGIFY(x)

// gentity->flags
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_DROPPED_ITEM			0x00001000
#define FL_NO_BOTS				0x00002000	// spawn point not for bot use
#define FL_NO_HUMANS			0x00004000	// spawn point just for bots
#define	FL_AI_GRENADE_KICK		0x00008000	// an AI has already decided to kick this grenade
// Rafael
#define FL_NOFATIGUE			0x00010000	// cheat flag no fatigue

#define FL_TOGGLE				0x00020000	//----(SA)	ent is toggling (doors use this for ex.)
#define FL_KICKACTIVATE			0x00040000	//----(SA)	ent has been activated by a kick (doors use this too for ex.)
#define	FL_SOFTACTIVATE			0x00000040	//----(SA)	ent has been activated while 'walking' (doors use this too for ex.)
#define	FL_DEFENSE_GUARD		0x00080000	// warzombie defense pose

#define	FL_PARACHUTE			0x00100000
#define	FL_WARZOMBIECHARGE		0x00200000
#define	FL_NO_MONSTERSLICK		0x00400000
#define	FL_NO_HEADCHECK			0x00800000

#define	FL_NODRAW				0x01000000

// movers are things like doors, plats, buttons, etc
typedef enum {
	MOVER_POS1,
	MOVER_POS2,
	MOVER_POS3,
	MOVER_1TO2,
	MOVER_2TO1,
	// JOSEPH 1-26-00
	MOVER_2TO3,
	MOVER_3TO2,
	// END JOSEPH

	// Rafael
	MOVER_POS1ROTATE,
	MOVER_POS2ROTATE,
	MOVER_1TO2ROTATE,
	MOVER_2TO1ROTATE
} moverState_t;


// door AI sound ranges
#define HEAR_RANGE_DOOR_LOCKED		128	// really close since this is a cruel check
#define HEAR_RANGE_DOOR_KICKLOCKED	512
#define HEAR_RANGE_DOOR_OPEN		256
#define HEAR_RANGE_DOOR_KICKOPEN	768

// DHM - Nerve :: Worldspawn spawnflags to indicate if a gametype is not supported
#define NO_GT_WOLF		1
#define NO_STOPWATCH	2
#define NO_CHECKPOINT	4

//============================================================================

typedef struct gentity_s gentity_t;
typedef struct gclient_s gclient_t;

//====================================================================
//
// Scripting, these structure are not saved into savegames (parsed each start)
typedef struct
{
	char	*actionString;
	qboolean (*actionFunc)(gentity_t *ent, char *params);
} g_script_stack_action_t;
//
typedef struct
{
	//
	// set during script parsing
	g_script_stack_action_t		*action;			// points to an action to perform
	char						*params;
} g_script_stack_item_t;
//
#define	G_MAX_SCRIPT_STACK_ITEMS	64
//
typedef struct
{
	g_script_stack_item_t	items[G_MAX_SCRIPT_STACK_ITEMS];
	int						numItems;
} g_script_stack_t;
//
typedef struct
{
	int					eventNum;			// index in scriptEvents[]
	char				*params;			// trigger targetname, etc
	g_script_stack_t	stack;
} g_script_event_t;
//
typedef struct
{
	char		*eventStr;
	qboolean	(*eventMatch)( g_script_event_t *event, char *eventParm );
} g_script_event_define_t;
//
// Script Flags
#define	SCFL_GOING_TO_MARKER	0x1
#define	SCFL_ANIMATING			0x2
//
// Scripting Status (NOTE: this MUST NOT contain any pointer vars)
typedef struct
{
	int		scriptStackHead, scriptStackChangeTime;
	int		scriptEventIndex;	// current event containing stack of actions to perform
	// scripting system variables
	int		scriptId;				// incremented each time the script changes
	int		scriptFlags;
	char	*animatingParams;
} g_script_status_t;
//
#define	G_MAX_SCRIPT_ACCUM_BUFFERS	8
//
void G_Script_ScriptEvent( gentity_t *ent, char *eventStr, char *params );
//====================================================================


#define	CFOFS(x) ((int)&(((gclient_t *)0)->x))

// g_alternatePing:
#define NUM_PING_SAMPLES 64

struct gentity_s {
	entityState_t	s;				// communicated by server to clients
	entityShared_t	r;				// shared by both the server system and game

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!
	//================================

	struct gclient_s	*client;			// NULL if not a client

	qboolean	inuse;

	char		*classname;			// set in QuakeEd
	int			spawnflags;			// set in QuakeEd

	qboolean	neverFree;			// if true, FreeEntity will only unlink
									// bodyque uses this

	int			flags;				// FL_* variables

	char		*model;
	char		*model2;
	int			freetime;			// level.time when the object was freed
	
	int			eventTime;			// events will be cleared EVENT_VALID_MSEC after set
	qboolean	freeAfterEvent;
	qboolean	unlinkAfterEvent;

	qboolean	physicsObject;		// if true, it can be pushed by movers and fall off edges
									// all game items are physicsObjects, 
	float		physicsBounce;		// 1.0 = continuous bounce, 0.0 = no bounce
	int			clipmask;			// brushes with this content value will be collided against
									// when moving.  items and corpses do not collide against
									// players, for instance

	// movers
	moverState_t moverState;
	int			soundPos1;
	int			sound1to2;
	int			sound2to1;
	int			soundPos2;
	int			soundLoop;
	// JOSEPH 1-26-00
	int			sound2to3;
	int			sound3to2;
	int			soundPos3;
	// END JOSEPH

	int			soundKicked;
	int			soundKickedEnd;

	int			soundSoftopen;
	int			soundSoftendo;
	int			soundSoftclose;
	int			soundSoftendc;

	gentity_t	*parent;
	gentity_t	*nextTrain;
	gentity_t	*prevTrain;
	// JOSEPH 1-26-00
	vec3_t		pos1, pos2, pos3;
	// END JOSEPH

	char		*message;

	int			timestamp;		// body queue sinking, etc

	float		angle;			// set in editor, -1 = up, -2 = down
	char		*target;
	char		*targetname;
	char		*team;
	char		*targetShaderName;
	char		*targetShaderNewName;
	gentity_t	*target_ent;

	float		speed;
	float		closespeed;		// for movers that close at a different speed than they open
	vec3_t		movedir;

	int			gDuration;
	int			gDurationBack;
	vec3_t		gDelta;
	vec3_t		gDeltaBack;

	int			nextthink;
	void		(*think)(gentity_t *self);
	void		(*reached)(gentity_t *self);	// movers call this when hitting endpoint
	void		(*blocked)(gentity_t *self, gentity_t *other);
	void		(*touch)(gentity_t *self, gentity_t *other, trace_t *trace);
	void		(*use)(gentity_t *self, gentity_t *other, gentity_t *activator);
	void		(*pain)(gentity_t *self, gentity_t *attacker, int damage, vec3_t point);
	void		(*die)(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);

	// Nobo - More
	void		(*more)(gentity_t *ent);
	int			moreCalls;
	qboolean	moreCalled;

	int			pain_debounce_time;
	int			fly_sound_debounce_time;	// wind tunnel
	int			last_move_time;

	int			health;

	qboolean	takedamage;

	int			damage;
	int			splashDamage;	// quad will increase this without increasing radius
	int			splashRadius;
	int			methodOfDeath;
	int			splashMethodOfDeath;

	int			count;

	gentity_t	*chain;
	gentity_t	*enemy;
	gentity_t	*activator;
	gentity_t	*teamchain;		// next entity in team
	gentity_t	*teammaster;	// master of the team

	int			watertype;
	int			waterlevel;

	int			noise_index;

	// timing variables
	float		wait;
	float		random;

	// Rafael - sniper variable
	// sniper uses delay, random, radius
	int			radius;
	float		delay;

	// JOSEPH 10-11-99
	int			TargetFlag;
	float		duration;
	vec3_t		rotate;
	vec3_t		TargetAngles;
	// END JOSEPH

	gitem_t		*item;			// for bonus items

	// Ridah, AI fields
	char		*aiAttributes;
	char		*aiName;
	int			aiTeam;
	void		(*AIScript_AlertEntity)( gentity_t *ent );
	qboolean	aiInactive;
	int			aiCharacter;	// the index of the type of character we are (from aicast_soldier.c)
	// done.

	char		*aiSkin;
	char		*aihSkin;

	vec3_t		dl_color;
	char		*dl_stylestring;
	char		*dl_shader;
	int			dl_atten;


	int			key;			// used by:  target_speaker->nopvs, 

	qboolean	active;
	qboolean	botDelayBegin;

	// Rafael - mg42
	float		harc;
	float		varc;

	int			props_frame_state;

	// Ridah
	int			missionLevel;		// mission we are currently trying to complete
									// gets reset each new level
	// done.

	// Rafael
	qboolean	is_dead;
	// done

	int			start_size;
	int			end_size;

	// Rafael props

	qboolean	isProp;

	int			mg42BaseEnt;

	gentity_t	*melee;

	char		*spawnitem;

	qboolean	nopickup;

	int			flameQuota, flameQuotaTime, flameBurnEnt;

	int			count2;

	int			grenadeExplodeTime;	// we've caught a grenade, which was due to explode at this time
	int			grenadeFired;		// the grenade entity we last fired

	char		*track;

	// entity scripting system
	char				*scriptName;

	int					numScriptEvents;
	g_script_event_t	*scriptEvents;	// contains a list of actions to perform for each event type
	g_script_status_t	scriptStatus;	// current status of scripting
	// the accumulation buffer
	int scriptAccumBuffer[G_MAX_SCRIPT_ACCUM_BUFFERS];

	qboolean	AASblocking;
	float		accuracy;

	char		*tagName;		// name of the tag we are attached to
	gentity_t	*tagParent;

	float		headshotDamageScale;

	int			lastHintCheckTime;			// DHM - Nerve
	// -------------------------------------------------------------------------------------------
	// if working on a post release patch, new variables should ONLY be inserted after this point
	// DHM - Nerve :: the above warning does not really apply to MP, but I'll follow it for good measure

	int			voiceChatSquelch;			// DHM - Nerve
	int			voiceChatPreviousTime;		// DHM - Nerve
	int			lastBurnedFrameNumber;		// JPW - Nerve   : to fix FT instant-kill exploit

// L0 
	qboolean	poisoned;					// Poison
	int			poisonEnt;					// Poison
	int			lastPoisonTime;				// Poison
	int			thrownKnifeTime;			// Knife throwing
	int			lastDragTime;				// Drag bodies	
	int			lastPushTime;				// Shove
	qboolean	droppedObj;					// Objective dropping	
	int			thrownSmoke;				// Smoke
	int			selectedSmoke;				// Smoke	

	// pause
	int			trType_pre_pause;
	vec3_t		trBase_pre_pause;

	// headshot
	qboolean	headshot;
	qboolean	is_head;
	gentity_t	*head;
};

// Ridah
#include "ai_cast_global.h"
// done.

typedef enum {
	CON_DISCONNECTED,
	CON_CONNECTING,
	CON_CONNECTED
} clientConnected_t;

typedef enum {
	SPECTATOR_NOT,
	SPECTATOR_FREE,
	SPECTATOR_FOLLOW,
	SPECTATOR_SCOREBOARD
} spectatorState_t;

typedef enum {
	TEAM_BEGIN,		// Beginning a team game, spawn at base
	TEAM_ACTIVE		// Now actively playing
} playerTeamStateState_t;

typedef struct {
	playerTeamStateState_t	state;

	int			location;

	int			captures;
	int			basedefense;
	int			carrierdefense;
	int			flagrecovery;
	int			fragcarrier;
	int			assists;

	float		lasthurtcarrier;
	float		lastreturnedflag;
	float		flagsince;
	float		lastfraggedcarrier;
} playerTeamState_t;

// the auto following clients don't follow a specific client
// number, but instead follow the first two active players
#define	FOLLOW_ACTIVE1	-1
#define	FOLLOW_ACTIVE2	-2

// L0 - Admins
typedef enum {
	ADM_NONE, // Normal players	
	ADM_1,    // Level 1 Admin
	ADM_2,    // Level 2 Admin
	ADM_3,    // Level 3 Admin
	ADM_4,	  // Level 4 Admin 
	ADM_5	  // Level 5 Admin
} admLvls_t;

typedef enum {
	IGNORE_OFF,
	IGNORE_PERMANENT,
	IGNORE_TEMPORARY
} ignoreState_t;

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in G_InitSessionData() / G_ReadSessionData() / G_WriteSessionData()
typedef struct {
	team_t		sessionTeam;
	int			spectatorTime;		// for determining next-in-line to play
	spectatorState_t	spectatorState;
	int			spectatorClient;	// for chasecam and follow mode
	int			wins, losses;		// tournament stats
	int			playerType;			// DHM - Nerve :: for GT_WOLF
	int			playerWeapon;		// DHM - Nerve :: for GT_WOLF
	int			playerItem;			// DHM - Nerve :: for GT_WOLF
	int			playerSkin;			// DHM - Nerve :: for GT_WOLF
	int			spawnObjectiveIndex; // JPW NERVE index of objective to spawn nearest to (returned from UI)
	int			latchPlayerType;	// DHM - Nerve :: for GT_WOLF not archived
	int			latchPlayerWeapon;	// DHM - Nerve :: for GT_WOLF not archived
	int			latchPlayerItem;	// DHM - Nerve :: for GT_WOLF not archived
	int			latchPlayerSkin;	// DHM - Nerve :: for GT_WOLF not archived

	// L0 - New stuff
	admLvls_t		admin;			// Admins
	int				ip;				// IP
	int	incognito;		// Incognito
	ignoreState_t	ignored;		// If client is ignored or not
	int			selectedWeapon;		// If enabled allows mp40, sten, thompson..
	int			colorFlags;
	int			gender;
	int			tempIgnoreCount;
	int			secretlyDemoing;
	int			muted[2];
	unsigned int uci; //geoIP //Elver added
} clientSession_t;

#define PICKUP_ACTIVATE	0	// pickup items only when using "+activate"
#define PICKUP_TOUCH	1	// pickup items when touched
#define PICKUP_FORCE	2	// pickup the next item when touched (and reset to PICKUP_ACTIVATE when done)

#define MAX_SAVED_LOCATIONS 5

typedef struct {
	vec3_t		location;
	vec3_t		viewangles;
	char		locationName[MAX_NAME_LENGTH];
} saved_location_t;

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct {
	clientConnected_t	connected;	
	usercmd_t	cmd;				// we would lose angles if not persistant
	usercmd_t	oldcmd;				// previous command processed by pmove()
	qboolean	localClient;		// true if trap_GetClientIp returns "localhost" 
	qboolean	initialSpawn;		// the first spawn should be at a cool location
	qboolean	predictItemPickup;	// based on cg_predictItems userinfo
	qboolean	pmoveFixed;			//
	char		netname[MAX_NETNAME];

	int			autoActivate;		// based on cg_autoactivate userinfo		(uses the PICKUP_ values above)
	int			emptySwitch;		// based on cg_emptyswitch userinfo (means "switch my weapon for me when ammo reaches '0' rather than -1)

	int			maxHealth;			// for handicapping
	int			enterTime;			// level.time the client entered the game
	int			connectTime;		// DHM - Nerve :: level.time the client first connected to the server
	playerTeamState_t teamState;	// status in teamplay games
	int			voteCount;			// to prevent people from constantly calling votes

	int			complaints;				// DHM - Nerve :: number of complaints lodged against this client
	int			complaintClient;		// DHM - Nerve :: able to lodge complaint against this client
	int			complaintEndTime;		// DHM - Nerve :: until this time has expired

	int			lastReinforceTime;		// DHM - Nerve :: last reinforcement

	qboolean	teamInfo;			// send team overlay updates?


	unsigned int uci;   // mcwf's GeoIP //Elver
// L0 
	// Server Bot
	int		sb_pingHits;
	int		sb_maxPingTime;
	int		sb_chatWarned;
	int		sb_ignored;
	int		sb_TKforgiven;
	int		sb_TKkillTime;
	qboolean sb_TKwarned;
	qboolean sb_TBwarned;

	// Shortcuts
	int		lastkilled_client;
	int		lastrevive_client;
	int		lastkiller_client;
	int		lastammo_client;
	int		lasthealth_client;

	// Takes ability to rename from client..it's cleared next round, map load..
	qboolean	nameLocked;	

	// Stats	
	int			kills;
	int			deaths;
	int			teamKills;	// Note that SB uses it as well!
	int			headshots;
	int			revives;
	int			medPacks;
	int			ammoPacks;
	int			acc_shots;
	int			acc_hits;
	int			dmgGiven;
	int			dmgReceived;
	int			dmgTeam;
	int			gibs;
	int			suicides;
	int			poison;
	int			chicken;
	int			knifeKills;
	int			fastStabs;
	int			beganPlayingTime;

	// Death Spree
	int			spreeDeaths;

	// Life Stats
	int			lifeKills;
	int			lifeRevives;
	int			lifeAcc_shots;
	int			lifeAcc_hits;
	int			lifeHeadshots;

	// Map Stats
	int			lifeKillsPeak;
	int			lifeDeathsPeak;

	// Throwing knives
	int			throwingKnives;

	// Weapon restrictions
	int			restrictedWeapon;

	// (no)Auto Reload hack
	qboolean	noReload;
	
	// Saved client locations for TJ
	saved_location_t savedLocations[MAX_SAVED_LOCATIONS];
	int			savedLocationCount;

	// g_alternatePing:
	int			alternatePing;
	int			pingsamples[NUM_PING_SAMPLES];
	int			samplehead;

	// ready system
	qboolean	ready;

	// used by the g_ammoPackGivesAmmo system.
	int			playerWeapon;

	// use spawn angles when revived (g_useSpawnAnglesAfterRevive "1")
	vec3_t spawnAngles;

	qboolean drawHitBoxes;

	int			timeInactive;  // kick players when timeInactive > inactivityCvar
	qboolean	lastInactivityWarnTime; // Don't spam warn.
} clientPersistant_t;

// L0 - antilag 
#define NUM_CLIENT_TRAILS 10
typedef struct {
	vec3_t    mins, maxs;
	vec3_t    currentOrigin;
	int       time, leveltime;
	clientAnimationInfo_t animInfo;
} clientTrail_t;

#define LT_SPECIAL_PICKUP_MOD	3		// JPW NERVE # of times (minus one for modulo) LT must drop ammo before scoring a point
#define MEDIC_SPECIAL_PICKUP_MOD	4	// JPW NERVE same thing for medic

// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
struct gclient_s {
	// ps MUST be the first element, because the server expects it
	playerState_t	ps;				// communicated by server to clients

	// the rest of the structure is private to game
	clientPersistant_t	pers;
	clientSession_t		sess;

	qboolean	readyToExit;		// wishes to leave the intermission

	qboolean	noclip;

	int			lastCmdTime;		// level.time of last usercmd_t, for EF_CONNECTION
									// we can't just use pers.lastCommand.time, because
									// of the g_sycronousclients case
	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	int			wbuttons;
	int			oldwbuttons;
	int			latched_wbuttons;
	vec3_t		oldOrigin;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	vec3_t		damage_from;		// origin for vector calculation
	qboolean	damage_fromWorld;	// if true, don't use the damage_from vector

	int			accurateCount;		// for "impressive" reward sound

	int			accuracy_shots;		// total number of shots
	int			accuracy_hits;		// total number of hits

	int			lastkilled_client;	// last client that this client killed
	int			lasthurt_client;	// last client that damaged this client
	int			lasthurt_mod;		// type of damage the client did
	int			lasthurt_time;		// L0 - Chicken check

	// timers
	int			respawnTime;		// can respawn when time > this, force after g_forcerespwan
	int			rewardTime;			// clear the EF_AWARD_IMPRESSIVE, etc when time > this

	int			airOutTime;

	int			lastKillTime;		// for multiple kill rewards

	qboolean	fireHeld;			// used for hook
	gentity_t	*hook;				// grapple hook if out

	int			switchTeamTime;		// time the player switched teams

	// timeResidual is used to handle events that happen every second
	// like health / armor countdowns and regeneration
	int			timeResidual;

	float		currentAimSpreadScale;

	int			medicHealAmt;

	// RF, may be shared by multiple clients/characters
	animModelInfo_t	*modelInfo;

	// -------------------------------------------------------------------------------------------
	// if working on a post release patch, new variables should ONLY be inserted after this point

	gentity_t	*persistantPowerup;
	int			portalID;
	int			ammoTimes[WP_NUM_WEAPONS];
	int			invulnerabilityTime;

	gentity_t	*cameraPortal;				// grapple hook if out
	vec3_t		cameraOrigin;

	int			dropWeaponTime; // JPW NERVE last time a weapon was dropped
	int			limboDropWeapon; // JPW NERVE weapon to drop in limbo
	int			deployQueueNumber; // JPW NERVE player order in reinforcement FIFO queue
	int			sniperRifleFiredTime; // JPW NERVE last time a sniper rifle was fired (for muzzle flip effects)
	float		sniperRifleMuzzleYaw; // JPW NERVE for time-dependent muzzle flip in multiplayer
	int			lastBurnTime; // JPW NERVE last time index for flamethrower burn
	int			PCSpecialPickedUpCount; // JPW NERVE used to count # of times somebody's picked up this LTs ammo (or medic health) (for scoring)
	int			saved_persistant[MAX_PERSISTANT];	// DHM - Nerve :: Save ps->persistant here during Limbo

// L0 
	// antilag
	int              trailHead;
	clientTrail_t    trail[NUM_CLIENT_TRAILS];
	clientTrail_t    saved; 

	// Double kill
	int			doublekill;	

	// LT Info
	int			infoTime;

	// Daily stats
	qboolean	duplicateip;

	// Unstuck
	qboolean	revivedInsideOfTheWorld;
	vec3_t		unstuckPos;
	qboolean	in_revived_state;

	qboolean	revive_animation_playing;
	int			movement_lock_begin_time;

	int			needs_delayed_teleport_bit_start_time;   // when the teleport bit request was made.
	int			teleport_bit_delayed_frame_count;		 // how many frames to wait before toggling the teleport bit.

	clientAnimationInfo_t animationInfo;
	float legsYawAngle, torsoYawAngle, torsoPitchAngle;
	qboolean torsoYawing, legsYawing, torsoPitching;
};

//
// this structure is cleared as each map is entered
//
#define	MAX_SPAWN_VARS			64
#define	MAX_SPAWN_VARS_CHARS	2048

typedef struct {
	struct gclient_s	*clients;		// [maxclients]

	struct gentity_s	*gentities;
	int			gentitySize;
	int			num_entities;		// current number, <= MAX_GENTITIES

	int			warmupTime;			// restart match at this time

	fileHandle_t	logFile;

	// store latched cvars here that we want to get at often
	int			maxclients;

	int			framenum;
	int			time;					// in msec
	int			previousTime;			// so movers can back up when blocked
	int         frameStartTime;         // L0 - antilag - actual time frame started

	int			startTime;				// level.time the map was started

	int			teamScores[TEAM_NUM_TEAMS];
	int			lastTeamLocationTime;		// last time of client team location update

	qboolean	newSession;				// don't use any old session data, because
										// we changed gametype

	qboolean	restarted;				// waiting for a map_restart to fire

	int			numNonDisconnectedClients; // not disconnected; connected or connecting
	int			numConnectedClients;		// connected, not disconected or connecting
	int			numPlayingClients;		// connected, non-spectators
	int			sortedClients[MAX_CLIENTS];		// sorted by score
	int			follow1, follow2;		// clientNums for auto-follow spectators

	int			snd_fry;				// sound index for standing in lava

	int			warmupModificationCount;	// for detecting if g_warmup is changed

	// voting state
	char		voteString[MAX_STRING_CHARS];
	char		voteDisplayString[MAX_STRING_CHARS];
	int			voteTime;				// level.time vote was called
	int			voteExecuteTime;		// time the vote is executed
	int			voteYes;
	int			voteNo;
	int			numVotingClients;		// set by CalculateRanks

	// team player count
	int			numTeamPlayers[2];		// set by CalculateRanks

	// spawn variables
	qboolean	spawning;				// the G_Spawn*() functions are valid
	int			numSpawnVars;
	char		*spawnVars[MAX_SPAWN_VARS][2];	// key / value pairs
	int			numSpawnVarChars;
	char		spawnVarChars[MAX_SPAWN_VARS_CHARS];

	// intermission state
	int			intermissionQueued;		// intermission was qualified, but
										// wait INTERMISSION_DELAY_TIME before
										// actually going there so the last
										// frag can be watched.  Disable future
										// kills during this delay
	int			intermissiontime;		// time the intermission was started
	char		*changemap;
	qboolean	readyToExit;			// at least one client wants to exit
	int			exitTime;
	vec3_t		intermission_origin;	// also used for spectator spawns
	vec3_t		intermission_angle;

	qboolean	locationLinked;			// target_locations get linked
	gentity_t	*locationHead;			// head of the location list
	int			bodyQueIndex;			// dead bodies
	gentity_t	*bodyQue[BODY_QUEUE_SIZE];

	int			portalSequence;
	// Ridah
	char		*scriptAI;
	int			reloadPauseTime;		// don't think AI/client's until this time has elapsed
	int			reloadDelayTime;		// don't start loading the savegame until this has expired

	int			lastGrenadeKick;

	int			loperZapSound;
	int			stimSoldierFlySound;
	int			bulletRicochetSound;
	// done.

	int			snipersound;

	//----(SA)	added
	int			knifeSound[4];
	//----(SA)	end

// JPW NERVE
	int			capturetimes[4]; // red, blue, none, spectator for WOLF_MP_CPH
	int			redReinforceTime, blueReinforceTime; // last time reinforcements arrived in ms
	int			redNumWaiting, blueNumWaiting; // number of reinforcements in queue
	vec3_t		spawntargets[MAX_MULTI_SPAWNTARGETS]; // coordinates of spawn targets
	int			numspawntargets; // # spawntargets in this map
// jpw

	// RF, entity scripting
	char		*scriptEntity;

	// player/AI model scripting (server repository)
	animScriptData_t	animScriptData;

	// NERVE - SMF - debugging/profiling info
	int			totalHeadshots;
	int			missedHeadshots;
	qboolean	lastRestartTime;
	// -NERVE - SMF

	int			numFinalDead[2];		// DHM - Nerve :: unable to respawn and in limbo (per team)
	int			numOidTriggers;			// DHM - Nerve

	qboolean	latchGametype;			// DHM - Nerve

// L0
	// Time between prints (Players left etc)
	int			leftCheck;			

	// Map Stats
	unsigned int	topScore;
	char			topOwner[MAX_NETNAME + 1];
	qboolean		mapStatsPrinted;

	// FFA top scorers
	gclient_t 	*ffaTopScorer;

	// Last Blood 
	char		lastKiller[MAX_NETNAME + 1];
	char		lastVictim[MAX_NETNAME + 1];

	// Countdown	
	int			cdFlags;

	// Round stats
	int			rsNextPrintTime;

	// Daily stats
	int			dsCheckTime;

	// Delay between votes..
	int			lastVoteTime;		

	// Auto balance teams timer
	int			balanceTimer;	

	// Weapons restrictions
	int			axisSniper, alliedSniper;
	int			axisPF, alliedPF;
	int			axisVenom, alliedVenom;
	int			axisFlamer, alliedFlamer;

	// Air Strike Restrictions
	int			axisBomber, alliedBomber;
	int			axisArty, alliedArty;

	// g_msgs
	int			msgTime;

	// Flag retaking
	int			flagTaken;		

	// automg42
	int			axis_automg42_count, allies_automg42_count;

	// pausing of match
	qboolean	paused;
	int			pause_time;
	int			unpaused_time;
	int			time_since_pause;
	int			warmup_start_time;

	// ready system
	int			ready_count;
} level_locals_t;

extern 	qboolean	reloading;				// loading up a savegame
// JPW NERVE
extern char testid1[];
extern char testid2[];
extern char testid3[];
// jpw

//
// g_spawn.c
//
qboolean	G_SpawnString( const char *key, const char *defaultString, char **out );
qboolean	G_SpawnFloat( const char *key, const char *defaultString, float *out );
qboolean	G_SpawnInt( const char *key, const char *defaultString, int *out );
qboolean	G_SpawnVector( const char *key, const char *defaultString, float *out );
void		G_SpawnEntitiesFromString( void );
char		*G_NewString( const char *string );
qboolean	G_CallSpawn( gentity_t *ent );
qboolean	G_ParseField(const char *key, const char *value, gentity_t *ent);

//
// g_cmds.c
//
void Cmd_Score_f (gentity_t *ent);
void StopFollowing( gentity_t *ent );
//void BroadcastTeamChange( gclient_t *client, int oldTeam );
void SetTeam(gentity_t *ent, char *s, qboolean forced);
void SetWolfData( gentity_t *ent, char *ptype, char *weap, char *grenade, char *skinnum );	// DHM - Nerve
void Cmd_FollowCycle_f( gentity_t *ent, int dir );
void SanitizeString(const char *in, char *out, qboolean fToLower);
int ClientNumberFromString(gentity_t *to, char *s);
char *ConcatArgs(int start);
char *ConcatArgs2(int start);

//
// g_items.c
//
void G_CheckTeamItems( void );
void G_RunItem( gentity_t *ent );
void RespawnItem( gentity_t *ent );

void UseHoldableItem( gentity_t *ent, int item );
void PrecacheItem (gitem_t *it);
gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle, qboolean novelocity );
gentity_t *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity, int ownerNum );
void SetRespawn (gentity_t *ent, float delay);
void G_SpawnItem (gentity_t *ent, gitem_t *item);
void FinishSpawningItem( gentity_t *ent );
void Think_Weapon (gentity_t *ent);
int ArmorIndex (gentity_t *ent);
void Fill_Clip (playerState_t *ps, int weapon);
void	Add_Ammo (gentity_t *ent, int weapon, int count, qboolean fillClip);
void Touch_Item (gentity_t *ent, gentity_t *other, trace_t *trace);
int PrimarySlotInUse(const int weapons[]);
qboolean WeaponIsPrimary(int weapon);
void DelayedWeaponPickup(gentity_t *self, gentity_t *other, trace_t *tr);

// Touch_Item_Auto is bound by the rules of autoactivation (if cg_autoactivate is 0, only touch on "activate")
void Touch_Item_Auto (gentity_t *ent, gentity_t *other, trace_t *trace);

void ClearRegisteredItems( void );
void RegisterItem( gitem_t *item );
void SaveRegisteredItems( void );
void Prop_Break_Sound (gentity_t *ent);
void Spawn_Shard (gentity_t *ent, gentity_t *inflictor, int quantity, int type);

//
// g_utils.c
//
// Ridah
int G_FindConfigstringIndex( const char *name, int start, int max, qboolean create );
// done.
int G_ModelIndex( char *name );
int		G_SoundIndex( const char *name );	
void	G_TeamCommand( team_t team, char *cmd );
void	G_KillBox (gentity_t *ent);
gentity_t *G_Find (gentity_t *from, int fieldofs, const char *match);
gentity_t *G_PickTarget (char *targetname);
void	G_UseTargets (gentity_t *ent, gentity_t *activator);
void	G_SetMovedir ( vec3_t angles, vec3_t movedir);

void	G_InitGentity( gentity_t *e );
gentity_t	*G_Spawn (void);
gentity_t *G_TempEntity( vec3_t origin, int event );
void	G_Sound( gentity_t *ent, int soundIndex );
void	G_AnimScriptSound( int soundIndex, vec3_t org, int client );
void	G_FreeEntity( gentity_t *e );
//qboolean	G_EntitiesFree( void );

void	G_TouchTriggers (gentity_t *ent);
void	G_TouchSolids (gentity_t *ent);

float	*tv (float x, float y, float z);
char	*vtos( const vec3_t v );

void G_AddPredictableEvent( gentity_t *ent, int event, int eventParm );
void G_AddEvent( gentity_t *ent, int event, int eventParm );
void G_SetOrigin( gentity_t *ent, vec3_t origin );
void AddRemap(const char *oldShader, const char *newShader, float timeOffset);
const char *BuildShaderStateConfig();
void G_SetAngle( gentity_t *ent, vec3_t angle );

qboolean infront (gentity_t *self, gentity_t *other);

void G_ProcessTagConnect(gentity_t *ent);

char *PlayerTypeToString(int ptype);

//
// g_combat.c
//
qboolean CanDamage (gentity_t *targ, vec3_t origin);
void G_Damage (gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, vec3_t dir, vec3_t point, int damage, int dflags, int mod);
qboolean G_RadiusDamage (vec3_t origin, gentity_t *attacker, float damage, float radius, gentity_t *ignore, int mod);
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath );
void TossClientItems( gentity_t *self );

// damage flags
#define DAMAGE_RADIUS			0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR			0x00000002	// armour does not protect from this damage
#define DAMAGE_NO_KNOCKBACK		0x00000008	// do not affect velocity, just view angles
#define DAMAGE_NO_PROTECTION	0x00000020  // armor, shields, invulnerability, and godmode have no effect
#define DAMAGE_NO_TEAM_PROTECTION	0x00000010  // armor, shields, invulnerability, and godmode have no effect

//
// g_missile.c
//
void G_RunMissile( gentity_t *ent );
int G_PredictMissile( gentity_t *ent, int duration, vec3_t endPos, qboolean allowBounce );

// Rafael zombiespit
void G_RunDebris( gentity_t *ent );

//DHM - Nerve :: server side flamethrower collision
void G_RunFlamechunk( gentity_t *ent );

//----(SA) removed unused q3a weapon firing
gentity_t *fire_flamechunk (gentity_t *self, vec3_t start, vec3_t dir);

gentity_t *fire_grenade (gentity_t *self, vec3_t start, vec3_t aimdir, int grenadeWPID);
gentity_t *fire_rocket (gentity_t *self, vec3_t start, vec3_t dir);
gentity_t *fire_speargun (gentity_t *self, vec3_t start, vec3_t dir);

//----(SA)	added from MP
gentity_t *fire_nail( gentity_t *self, vec3_t start, vec3_t forward, vec3_t right, vec3_t up );
gentity_t *fire_prox( gentity_t *self, vec3_t start, vec3_t aimdir );
//----(SA)	end

// Rafael sniper
void fire_lead (gentity_t *self,  vec3_t start, vec3_t dir, int damage);
qboolean visible (gentity_t *self, gentity_t *other);

gentity_t *fire_mortar (gentity_t *self, vec3_t start, vec3_t dir);
gentity_t *fire_flamebarrel (gentity_t *self, vec3_t start, vec3_t dir);
// done

//
// g_mover.c
//
gentity_t *G_TestEntityPosition(gentity_t *ent);
void G_RunMover( gentity_t *ent );
void Use_BinaryMover( gentity_t *ent, gentity_t *other, gentity_t *activator );
void G_Activate( gentity_t *ent, gentity_t *activator );

void G_TryDoor(gentity_t *ent, gentity_t *other, gentity_t *activator);	//----(SA)	added

void InitMoverRotate ( gentity_t *ent );

void InitMover( gentity_t *ent );
void SetMoverState( gentity_t *ent, moverState_t moverState, int time );

void BecomeExplosion(gentity_t *self);
void func_explosive_explode(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);

//
// g_tramcar.c
//
void Reached_Tramcar (gentity_t *ent);


//
// g_misc.c
//
#define MG42_SPREAD			200
#define MG42_DAMAGE			18
void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles );
void Fire_Lead(gentity_t *ent, gentity_t *activator, float spread, int damage, 
			   vec3_t muzzle, vec3_t forward, vec3_t right, vec3_t up, qboolean is_automg42);
void mg42_fire(gentity_t *other);

// g_automg42.c
void automg42_spawn(gentity_t *ent);
void automg42_think(gentity_t *self);
void automg42_disable(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
void automg42_die(gentity_t *ent, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
void automg42_base_soaring(gentity_t *base);
void G_Automg42SetConstantLight(gentity_t *gun, int intensity);
void G_Automg42UpdateHealth(gentity_t *gun, int health);
void G_Automg42Repair(gentity_t *ent, gentity_t *traceEnt);
void G_Automg42Disassociate(gentity_t *ent);
qboolean G_IsEntityAutomg42(gentity_t *ent);

//
// g_weapon.c
//
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker );
void CalcMuzzlePoint ( gentity_t *ent, int weapon, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint );
void SnapVectorTowards( vec3_t v, vec3_t to );
trace_t *CheckMeleeAttack( gentity_t *ent, float dist, qboolean isTest );
gentity_t *weapon_grenadelauncher_fire (gentity_t *ent, int grenadeWPID);
void MagicSink(gentity_t *self);
void AddHeadEntity(gentity_t *ent);
void FreeHeadEntity(gentity_t *ent);
void UpdateHeadEntity(gentity_t *ent);
void RemoveHeadEntity(gentity_t *ent);

void CalcMuzzlePoints(gentity_t *ent, int weapon);

// Rafael - for activate
void CalcMuzzlePointForActivate ( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint );
// done.

//
// g_client.c
//
team_t TeamCount( int ignoreClientNum, int team );			// NERVE - SMF - merge from team arena
team_t PickTeam( int ignoreClientNum );
void SetClientViewAngle( gentity_t *ent, vec3_t angle );
gentity_t *SelectRandomSpawnPoint( vec3_t avoidPoint, vec3_t origin, vec3_t angles );
gentity_t *SelectSpawnPoint(vec3_t origin, vec3_t angles);
gentity_t *SelectCyclicSpawnPoint(vec3_t origin, vec3_t angles);
void respawn (gentity_t *ent);
void BeginIntermission (void);
void InitClientPersistant (gclient_t *client);
void InitClientResp (gclient_t *client);
void InitBodyQue (void);
void ClientSpawn( gentity_t *ent, qboolean revived );
void player_die (gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
void AddScore( gentity_t *ent, int score );
void CalculateRanks( void );
qboolean SpotWouldConflict(gentity_t *spot);
char *clientIP(gentity_t *ent, qboolean full);
int ClientGetFps(playerState_t *ps);
int ClientGetMsec(playerState_t *ps);

//
// g_svcmds.c
//
qboolean ConsoleCommand( void );

//
// g_weapon.c
//
void FireWeapon( gentity_t *ent );
void G_BurnMeGood(gentity_t *self, gentity_t *body);

//
// p_hud.c
//
void MoveClientToIntermission (gentity_t *client);
void G_SetStats (gentity_t *ent);
void DeathmatchScoreboardMessage (gentity_t *client);

//
// g_cmds.c
//
void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message, qboolean localize ); // JPW NERVE removed static declaration so it would link

//
// g_pweapon.c
//


//
// g_main.c
//
void FindIntermissionPoint( void );
void G_RunThink (gentity_t *ent);
void QDECL G_LogPrintf( const char *fmt, ... );
void SendScoreboardMessageToAllClients( void );
void QDECL G_Printf( const char *fmt, ... );
void QDECL G_DPrintf( const char *fmt, ... );
void QDECL G_Error( const char *fmt, ... );
void CheckVote( void );

//
// g_client.c
//
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot );
void ClientUserinfoChanged( int clientNum );
void ClientDisconnect( int clientNum );
void ClientBegin( int clientNum );
void ClientCommand( int clientNum );

//
// g_active.c
//
void ClientThink( int clientNum );
void ClientEndFrame( gentity_t *ent );
void G_RunClient( gentity_t *ent );
void G_PlayerAngles(gentity_t *ent, int msec);
void G_PlayerAnimation(gentity_t *ent);

//
// g_team.c
//
qboolean OnSameTeam( gentity_t *ent1, gentity_t *ent2 );
// L0 - New team stuff bellow
typedef struct {
	qboolean team_lock;
	char team_name[24];
	int team_score;
} team_info;
extern team_info teamInfo[TEAM_NUM_TEAMS];
void G_swapTeamLocks( void );
void handleTeamLocks( int team );

//
// g_mem.c
//
void *G_Alloc( int size );
void G_InitMemory( void );
void Svcmd_GameMem_f( void );

//
// g_session.c
//
void G_ReadSessionData( gclient_t *client );
void G_InitSessionData( gclient_t *client, char *userinfo );

void G_InitWorldSession( void );
void G_WriteSessionData( void );

//
// g_bot.c
//
void G_InitBots( qboolean restart );
char *G_GetBotInfoByNumber( int num );
char *G_GetBotInfoByName( const char *name );
void G_CheckBotSpawn( void );
void G_QueueBotBegin( int clientNum );
qboolean G_BotConnect( int clientNum, qboolean restart );
void Svcmd_AddBot_f( void );

// ai_main.c
#define MAX_FILEPATH			144

//bot settings
typedef struct bot_settings_s
{
	char characterfile[MAX_FILEPATH];
	float skill;
	char team[MAX_FILEPATH];
} bot_settings_t;

int BotAISetup( int restart );
int BotAIShutdown( int restart );
int BotAILoadMap( int restart );
int BotAISetupClient(int client, struct bot_settings_s *settings);
int BotAIShutdownClient( int client);
int BotAIStartFrame( int time );
void BotTestAAS(vec3_t origin);


// g_cmd.c
void Cmd_Activate_f (gentity_t *ent);
int Cmd_WolfKick_f (gentity_t *ent);
// Ridah

// g_script.c
void G_Script_ScriptParse( gentity_t *ent );
qboolean G_Script_ScriptRun( gentity_t *ent );
void G_Script_ScriptEvent( gentity_t *ent, char *eventStr, char *params );
void G_Script_ScriptLoad( void );

float AngleDifference(float ang1, float ang2);

// g_props.c
void Props_Chair_Skyboxtouch (gentity_t *ent);

#include "g_team.h" // teamplay specific stuff


extern	level_locals_t	level;
extern	gentity_t		g_entities[];	//DAJ was explicit set to MAX_ENTITIES
extern	gentity_t		*g_camEnt;

#define	FOFS(x) ((size_t)&(((gentity_t *)0)->x))

extern	vmCvar_t	g_gametype;

// Rafael gameskill
extern	vmCvar_t	g_gameskill;
// done

extern	vmCvar_t	g_dedicated;
extern	vmCvar_t	g_cheats;
extern	vmCvar_t	g_maxclients;			// allow this many total, including spectators
extern	vmCvar_t	g_maxGameClients;		// allow this many active
extern	vmCvar_t	g_minGameClients;		// NERVE - SMF - we need at least this many before match actually starts
extern	vmCvar_t	g_restarted;

extern	vmCvar_t	g_dmflags;
extern	vmCvar_t	g_fraglimit;
extern	vmCvar_t	g_timelimit;
extern	vmCvar_t	g_capturelimit;
extern	vmCvar_t	g_friendlyFire;
extern	vmCvar_t	g_password;
extern	vmCvar_t	g_gravity;
extern	vmCvar_t	g_speed;
extern	vmCvar_t	g_knockback;
extern	vmCvar_t	g_quadfactor;
extern	vmCvar_t	g_forcerespawn;
extern	vmCvar_t	g_inactivity;
extern	vmCvar_t	g_debugMove;
extern	vmCvar_t	g_debugAlloc;
extern	vmCvar_t	g_debugDamage;
extern	vmCvar_t	g_debugBullets;	//----(SA)	added
extern	vmCvar_t	g_drawHitboxes;
extern	vmCvar_t	g_preciseHeadHitBox;
extern	vmCvar_t	g_headshotsOnly;
extern	vmCvar_t	g_weaponRespawn;
extern	vmCvar_t	g_syncronousClients;
extern	vmCvar_t	g_motd;
extern	vmCvar_t	g_warmup;
extern	vmCvar_t	g_allowVote;

// DHM - Nerve :: The number of complaints allowed before kick/ban
extern	vmCvar_t	g_complaintlimit;
extern	vmCvar_t	g_maxlives;				// DHM - Nerve :: number of respawns allowed (0==infinite)
extern	vmCvar_t	g_voiceChatsAllowed;	// DHM - Nerve :: number before spam control

extern	vmCvar_t	g_weaponTeamRespawn;
extern	vmCvar_t	g_teamForceBalance;
extern	vmCvar_t	g_rankings;
extern	vmCvar_t	g_enableBreath;
extern	vmCvar_t	g_smoothClients;
extern	vmCvar_t	pmove_fixed;
extern	vmCvar_t	pmove_msec;

//Rafael
extern  vmCvar_t	g_autoactivate;

extern	vmCvar_t	g_testPain;

extern	vmCvar_t	g_missionStats;
extern	vmCvar_t	ai_scriptName;		// name of AI script file to run (instead of default for that map)
extern	vmCvar_t	g_scriptName;		// name of script file to run (instead of default for that map)

extern	vmCvar_t	g_scriptDebug;

extern	vmCvar_t	g_userAim;

extern	vmCvar_t	g_forceModel;

extern	vmCvar_t	g_mg42arc;

extern	vmCvar_t	g_footstepAudibleRange;
// JPW NERVE multiplayer
extern vmCvar_t		g_redlimbotime;
extern vmCvar_t		g_bluelimbotime;
extern vmCvar_t		g_medicChargeTime;
extern vmCvar_t		g_engineerChargeTime;
extern vmCvar_t		g_LTChargeTime;
extern vmCvar_t		g_soldierChargeTime;
extern vmCvar_t		sv_screenshake;
// jpw

// NERVE - SMF
extern vmCvar_t		g_warmupLatch;
extern vmCvar_t		g_nextTimeLimit;
extern vmCvar_t		g_showHeadshotRatio;
extern vmCvar_t		g_userTimeLimit;
extern vmCvar_t		g_userAlliedRespawnTime;
extern vmCvar_t		g_userAxisRespawnTime;
extern vmCvar_t		g_currentRound;
extern vmCvar_t		g_noTeamSwitching;
extern vmCvar_t		g_altStopwatchMode;
extern vmCvar_t		g_gamestate;
extern vmCvar_t		g_swapteams;
// -NERVE - SMF

// L0 - New cvars

// Admins
extern vmCvar_t		a1_pass;
extern vmCvar_t		a2_pass;
extern vmCvar_t		a3_pass;
extern vmCvar_t		a4_pass;
extern vmCvar_t		a5_pass;
extern vmCvar_t		a1_tag;
extern vmCvar_t		a2_tag;
extern vmCvar_t		a3_tag;
extern vmCvar_t		a4_tag;
extern vmCvar_t		a5_tag;
extern vmCvar_t		a1_message;
extern vmCvar_t		a2_message;
extern vmCvar_t		a3_message;
extern vmCvar_t		a4_message;
extern vmCvar_t		a5_message;
extern vmCvar_t		a1_cmds;
extern vmCvar_t		a2_cmds;
extern vmCvar_t		a3_cmds;
extern vmCvar_t		a4_cmds;
extern vmCvar_t		a5_cmds;
extern vmCvar_t		a5_allowAll;

// Server Bot
extern vmCvar_t		sb_system;
extern vmCvar_t		sb_maxTKs;
extern vmCvar_t		sb_maxTKsTempbanMins;
extern vmCvar_t		sb_maxTeamBleed;
extern vmCvar_t		sb_maxTeamBleedTempbanMins;
extern vmCvar_t		sb_minLowScore;
extern vmCvar_t		sb_minLowScoreTempbanMins;
extern vmCvar_t		sb_maxPingFlux;
extern vmCvar_t		sb_maxPingHits;
extern vmCvar_t		sb_autoIgnore;
extern vmCvar_t		sb_maxTempIgnores;
extern vmCvar_t		sb_censorPenalty;
extern vmCvar_t		sb_censorPentalityTempbanMin;

// System
extern vmCvar_t		g_extendedLog;
extern vmCvar_t		g_maxVotes;
extern vmCvar_t		g_disallowedVotes;
extern vmCvar_t		g_voteConfigs;
extern vmCvar_t		g_voteDelay;
extern vmCvar_t		g_antilag;
extern vmCvar_t		sv_hostname;
extern vmCvar_t		g_bypassPasswords;
extern vmCvar_t		bannedMSG;
extern vmCvar_t		g_ignoreSpecs;
extern vmCvar_t		g_inactivityToSpecs;
extern vmCvar_t		g_spectatorInactivity;
extern vmCvar_t		g_autoShuffle;
extern vmCvar_t		g_censorWords;
extern vmCvar_t		g_disallowedNames;
extern vmCvar_t		g_noHardcodedCensor;
extern vmCvar_t		g_shortcuts;
extern vmCvar_t		g_allowPMs;
extern vmCvar_t		g_serverMessage;
extern vmCvar_t		g_unstuckRevive;
extern vmCvar_t		g_customSpawns;
extern vmCvar_t		g_teamSwitchTime;
extern vmCvar_t		g_readySystem;
extern vmCvar_t		g_readyPlayers;
extern vmCvar_t		g_archiveLogDay;
extern vmCvar_t		g_useSpawnAnglesAfterRevive;

// Game
extern vmCvar_t		g_dropReload;
extern vmCvar_t		g_dropClips;
extern vmCvar_t		g_unlockWeapons;
extern vmCvar_t		g_tapReports;
extern vmCvar_t		g_gibReports;
extern vmCvar_t		g_reviveReports;
extern vmCvar_t		g_weaponLockTime;
extern vmCvar_t		g_fastStabSound;
extern vmCvar_t		g_showLifeStats;
extern vmCvar_t		g_chicken;
extern vmCvar_t		g_poison;
extern vmCvar_t		g_hitsounds;
extern vmCvar_t		g_screenShake;
extern vmCvar_t		g_fixedphysics;
extern vmCvar_t		g_printMatchInfo;
extern vmCvar_t		g_teamAutoBalance;
extern vmCvar_t		g_warmupDamage;
extern vmCvar_t		g_crouchStaminaBoost;
extern vmCvar_t		g_bunnyJump;
extern vmCvar_t		g_dragBodies;
extern vmCvar_t		g_shove;
extern vmCvar_t		g_dropObj;
extern vmCvar_t		g_easyASBlock;
extern vmCvar_t		g_LTinfoMsg;
extern vmCvar_t		g_disableInv;
extern vmCvar_t		g_axisSpawnProtectionTime;
extern vmCvar_t		g_alliedSpawnProtectionTime;
extern vmCvar_t		g_fastres;
extern vmCvar_t		g_fastResMsec;
extern vmCvar_t		g_axisASdelay;
extern vmCvar_t		g_axisASdelayFFE;
extern vmCvar_t		g_alliedASdelay;
extern vmCvar_t		g_alliedASdelayFFE;
extern vmCvar_t		g_smokeGrenades;
extern vmCvar_t		g_smokeGrenadesLmt;
extern vmCvar_t		g_flagRetake;
extern vmCvar_t		g_balanceFlagRetake;
extern vmCvar_t		g_balanceFlagCanClaim;
extern vmCvar_t		g_alliedmaxlives;
extern vmCvar_t		g_axismaxlives;
extern vmCvar_t		g_savedLocations;
extern vmCvar_t		g_playersLeftTime;
extern vmCvar_t		g_alternatePing;
extern vmCvar_t		g_forceClass;
extern vmCvar_t		g_automg42;
extern vmCvar_t		g_automg42LightIntensity;
extern vmCvar_t		g_automg42Health;
extern vmCvar_t		g_automg42Disable;
extern vmCvar_t		g_automg42TowerCollapse;

// Modes
extern vmCvar_t		g_deathmatch;
extern vmCvar_t		g_ffa;

// Weapon Stuff
extern vmCvar_t		g_dropHealth;
extern vmCvar_t		g_dropNades;
extern vmCvar_t		g_dropAmmo;
extern vmCvar_t		g_dropAmmoPack;
extern vmCvar_t		g_throwKnives;
extern vmCvar_t		g_throwingKnifeDamage;
extern vmCvar_t		g_ltNades;
extern vmCvar_t		g_medicNades;
extern vmCvar_t		g_soldNades;
extern vmCvar_t		g_engNades;
extern vmCvar_t		g_syringes;
extern vmCvar_t		g_medicClips;
extern vmCvar_t		g_engineerClips;
extern vmCvar_t		g_soldierClips;
extern vmCvar_t		g_ltClips;
extern vmCvar_t		g_pistolClips;
extern vmCvar_t		g_venomClips;
extern vmCvar_t		g_mauserClips;
extern vmCvar_t		g_maxTeamPF;
extern vmCvar_t		g_maxTeamSniper;
extern vmCvar_t		g_maxTeamVenom;
extern vmCvar_t		g_maxTeamFlamer;
extern vmCvar_t		g_balancePF;
extern vmCvar_t		g_balanceSniper;
extern vmCvar_t		g_balanceVenom;
extern vmCvar_t		g_balanceFlamer;
extern vmCvar_t		g_customMGs;
extern vmCvar_t		g_allowUnderwater;
extern vmCvar_t		g_clampPacketAngles;
extern vmCvar_t		g_ammoRegen;
extern vmCvar_t		g_lockPrimary;
extern vmCvar_t		g_delayedPickup;
extern vmCvar_t		g_stickyGrenades;
extern vmCvar_t		g_stenMaxHeat;
extern vmCvar_t		g_stenCoolRate;
extern vmCvar_t		g_medicAmmoPack;
extern vmCvar_t		g_ammoPackGivesWeapon;

// Stats
extern vmCvar_t		g_doubleKills;
extern vmCvar_t		g_killingSprees;
extern vmCvar_t		g_deathSprees;
extern vmCvar_t		g_showFirstHeadshot;
extern vmCvar_t		g_showFirstBlood;
extern vmCvar_t		g_showLastBlood;
extern vmCvar_t		g_mapStats;
extern vmCvar_t		g_mapStatsNotify;
extern vmCvar_t		g_roundStats;
extern vmCvar_t		g_excludedRoundStats;
extern vmCvar_t		g_dailystats;
extern vmCvar_t		g_dailyMinimumKills;
extern vmCvar_t		g_dailyStatsResetHour;

// Static
extern vmCvar_t		shuffleTracking;
extern vmCvar_t		needsBalance;
extern vmCvar_t		needsDailyMsg;
extern vmCvar_t		latchedWarmup;
extern vmCvar_t		axisPlayers;
extern vmCvar_t		alliedPlayers;

// L0 - New Cvars end

void	trap_Printf( const char *fmt );
void	trap_Error( const char *fmt );
int		trap_Milliseconds( void );
int		trap_Argc( void );
void	trap_Argv( int n, char *buffer, int bufferLength );
void	trap_Args( char *buffer, int bufferLength );
void	trap_TokenizeString(char *string);
int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void	trap_FS_Read( void *buffer, int len, fileHandle_t f );
int		trap_FS_Write( const void *buffer, int len, fileHandle_t f );
int		trap_FS_Rename( const char *from, const char *to );
void	trap_FS_FCloseFile( fileHandle_t f );
int		trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
void	trap_SendConsoleCommand( int exec_when, const char *text );
void	trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
void	trap_Cvar_Update( vmCvar_t *cvar );
void	trap_Cvar_Set( const char *var_name, const char *value );
int		trap_Cvar_VariableIntegerValue( const char *var_name );
float	trap_Cvar_VariableValue( const char *var_name );
void	trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void	trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *gameClients, int sizeofGameClient );
void	trap_DropClient( int clientNum, const char *reason );
void	trap_SendServerCommand( int clientNum, const char *text );
void	trap_SetConfigstring( int num, const char *string );
void	trap_GetConfigstring( int num, char *buffer, int bufferSize );
void	trap_GetUserinfo( int num, char *buffer, int bufferSize );
void	trap_SetUserinfo( int num, const char *buffer );
void	trap_GetServerinfo( char *buffer, int bufferSize );
void	trap_SetBrushModel( gentity_t *ent, const char *name );
void	trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
void	trap_TraceCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
int		trap_PointContents( const vec3_t point, int passEntityNum );
qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 );
qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 );
void	trap_AdjustAreaPortalState( gentity_t *ent, qboolean open );
qboolean trap_AreasConnected( int area1, int area2 );
void	trap_LinkEntity( gentity_t *ent );
void	trap_UnlinkEntity( gentity_t *ent );
int		trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount );
qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
qboolean trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
int		trap_BotAllocateClient( void );
void	trap_BotFreeClient( int clientNum );
void	trap_GetUsercmd( int clientNum, usercmd_t *cmd );
qboolean	trap_GetEntityToken( char *buffer, int bufferSize );
qboolean trap_GetTag( gentity_t *ent, clientAnimationInfo_t *animInfo, char *tagName, orientation_t *or );
void trap_AppendEntityString(char *fileName);
void trap_FreeEntityString(void);
void trap_GetClientIp(int clientNum, char *ip_out, int ip_out_length);
int trap_GetClientProtocol(int clientNum);
void trap_SetClientName(int clientNum, char *name);
void trap_LogToFile(char* q_path, char* message);
void trap_CloseCommonLogFile();
qboolean trap_CommonLogFileExists();

void	trap_SubmitGlobalStatsData(char *statsData);
void 	trap_GlobalStatsServerConnect(void);
void	trap_GlobalStatsServerDisconnect(void);
void	trap_GlobalStatsReconnect(void);

int		trap_DebugPolygonCreate(int color, int numPoints, vec3_t *points);
void	trap_DebugPolygonDelete(int id);

int		trap_BotLibSetup( void );
int		trap_BotLibShutdown( void );
int		trap_BotLibVarSet(char *var_name, char *value);
int		trap_BotLibVarGet(char *var_name, char *value, int size);
int		trap_BotLibDefine(char *string);
int		trap_BotLibStartFrame(float time);
int		trap_BotLibLoadMap(const char *mapname);
int		trap_BotLibUpdateEntity(int ent, void /* struct bot_updateentity_s */ *bue);
int		trap_BotLibTest(int parm0, char *parm1, vec3_t parm2, vec3_t parm3);

int		trap_BotGetSnapshotEntity( int clientNum, int sequence );
int		trap_BotGetServerCommand(int clientNum, char *message, int size);
//int		trap_BotGetConsoleMessage(int clientNum, char *message, int size);
void	trap_BotUserCommand(int client, usercmd_t *ucmd);

void		trap_AAS_EntityInfo(int entnum, void /* struct aas_entityinfo_s */ *info);

int			trap_AAS_Initialized(void);
void		trap_AAS_PresenceTypeBoundingBox(int presencetype, vec3_t mins, vec3_t maxs);
float		trap_AAS_Time(void);

int			trap_AAS_PointAreaNum(vec3_t point);
int			trap_AAS_TraceAreas(vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas);

int			trap_AAS_PointContents(vec3_t point);
int			trap_AAS_NextBSPEntity(int ent);
int			trap_AAS_ValueForBSPEpairKey(int ent, char *key, char *value, int size);
int			trap_AAS_VectorForBSPEpairKey(int ent, char *key, vec3_t v);
int			trap_AAS_FloatForBSPEpairKey(int ent, char *key, float *value);
int			trap_AAS_IntForBSPEpairKey(int ent, char *key, int *value);

int			trap_AAS_AreaReachability(int areanum);

int			trap_AAS_AreaTravelTimeToGoalArea(int areanum, vec3_t origin, int goalareanum, int travelflags);

int			trap_AAS_Swimming(vec3_t origin);
int			trap_AAS_PredictClientMovement(void /* aas_clientmove_s */ *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize);

// Ridah, route-tables
void		trap_AAS_RT_ShowRoute( vec3_t srcpos, int	srcnum, int destnum );
qboolean	trap_AAS_RT_GetHidePos( vec3_t srcpos, int srcnum, int srcarea, vec3_t destpos, int destnum, int destarea, vec3_t returnPos );
int			trap_AAS_FindAttackSpotWithinRange(int srcnum, int rangenum, int enemynum, float rangedist, int travelflags, float *outpos);
void		trap_AAS_SetAASBlockingEntity( vec3_t absmin, vec3_t absmax, qboolean blocking );
// done.

void	trap_EA_Say(int client, char *str);
void	trap_EA_SayTeam(int client, char *str);
void	trap_EA_Voice(int client, char *str);
void	trap_EA_VoiceTeam(int client, char *str);
void	trap_EA_UseItem(int client, char *it);
void	trap_EA_DropItem(int client, char *it);
void	trap_EA_UseInv(int client, char *inv);
void	trap_EA_DropInv(int client, char *inv);
void	trap_EA_Gesture(int client);
void	trap_EA_Activate(int client);
void	trap_EA_Command(int client, char *command);

void	trap_EA_SelectWeapon(int client, int weapon);
void	trap_EA_Talk(int client);
void	trap_EA_Attack(int client);
void	trap_EA_Reload(int client);
void	trap_EA_Use(int client);
void	trap_EA_Respawn(int client);
void	trap_EA_Jump(int client);
void	trap_EA_DelayedJump(int client);
void	trap_EA_Crouch(int client);
void	trap_EA_MoveUp(int client);
void	trap_EA_MoveDown(int client);
void	trap_EA_MoveForward(int client);
void	trap_EA_MoveBack(int client);
void	trap_EA_MoveLeft(int client);
void	trap_EA_MoveRight(int client);
void	trap_EA_Move(int client, vec3_t dir, float speed);
void	trap_EA_View(int client, vec3_t viewangles);

void	trap_EA_EndRegular(int client, float thinktime);
void	trap_EA_GetInput(int client, float thinktime, void /* struct bot_input_s */ *input);
void	trap_EA_ResetInput(int client, void *init);


int		trap_BotLoadCharacter(char *charfile, int skill);
void	trap_BotFreeCharacter(int character);
float	trap_Characteristic_Float(int character, int index);
float	trap_Characteristic_BFloat(int character, int index, float min, float max);
int		trap_Characteristic_Integer(int character, int index);
int		trap_Characteristic_BInteger(int character, int index, int min, int max);
void	trap_Characteristic_String(int character, int index, char *buf, int size);

int		trap_BotAllocChatState(void);
void	trap_BotFreeChatState(int handle);
void	trap_BotQueueConsoleMessage(int chatstate, int type, char *message);
void	trap_BotRemoveConsoleMessage(int chatstate, int handle);
int		trap_BotNextConsoleMessage(int chatstate, void /* struct bot_consolemessage_s */ *cm);
int		trap_BotNumConsoleMessages(int chatstate);
void	trap_BotInitialChat(int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
int		trap_BotNumInitialChats(int chatstate, char *type);
int		trap_BotReplyChat(int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
int		trap_BotChatLength(int chatstate);
void	trap_BotEnterChat(int chatstate, int client, int clientto);
void	trap_BotGetChatMessage(int chatstate, char *buf, int size);
int		trap_StringContains(char *str1, char *str2, int casesensitive);
int		trap_BotFindMatch(char *str, void /* struct bot_match_s */ *match, unsigned long int context);
void	trap_BotMatchVariable(void /* struct bot_match_s */ *match, int variable, char *buf, int size);
void	trap_UnifyWhiteSpaces(char *string);
void	trap_BotReplaceSynonyms(char *string, unsigned long int context);
int		trap_BotLoadChatFile(int chatstate, char *chatfile, char *chatname);
void	trap_BotSetChatGender(int chatstate, int gender);
void	trap_BotSetChatName(int chatstate, char *name, int client);
void	trap_BotResetGoalState(int goalstate);
void	trap_BotRemoveFromAvoidGoals(int goalstate, int number);
void	trap_BotResetAvoidGoals(int goalstate);
void	trap_BotPushGoal(int goalstate, void /* struct bot_goal_s */ *goal);
void	trap_BotPopGoal(int goalstate);
void	trap_BotEmptyGoalStack(int goalstate);
void	trap_BotDumpAvoidGoals(int goalstate);
void	trap_BotDumpGoalStack(int goalstate);
void	trap_BotGoalName(int number, char *name, int size);
int		trap_BotGetTopGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int		trap_BotGetSecondGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int		trap_BotChooseLTGItem(int goalstate, vec3_t origin, int *inventory, int travelflags);
int		trap_BotChooseNBGItem(int goalstate, vec3_t origin, int *inventory, int travelflags, void /* struct bot_goal_s */ *ltg, float maxtime);
int		trap_BotChooseRouteGoal(int goalstate, int team, vec3_t origin, void *current_goal, int seconds_doing_route);
int		trap_GetRouteAtIndex(int index, void *route);
int		trap_BotTouchingGoal(vec3_t origin, void /* struct bot_goal_s */ *goal);
int		trap_BotItemGoalInVisButNotVisible(int viewer, vec3_t eye, vec3_t viewangles, void /* struct bot_goal_s */ *goal);
int		trap_BotGetNextCampSpotGoal(int num, void /* struct bot_goal_s */ *goal);
int		trap_BotGetMapLocationGoal(char *name, void /* struct bot_goal_s */ *goal);
int		trap_BotGetLevelItemGoal(int index, char *classname, void /* struct bot_goal_s */ *goal);
float	trap_BotAvoidGoalTime(int goalstate, int number);
void	trap_BotInitLevelItems(void);
void	trap_BotUpdateEntityItems(void);
int		trap_BotLoadItemWeights(int goalstate, char *filename);
void	trap_BotFreeItemWeights(int goalstate);
void	trap_BotInterbreedGoalFuzzyLogic(int parent1, int parent2, int child);
void	trap_BotSaveGoalFuzzyLogic(int goalstate, char *filename);
void	trap_BotMutateGoalFuzzyLogic(int goalstate, float range);
int		trap_BotAllocGoalState(int state);
void	trap_BotFreeGoalState(int handle);

void	trap_BotResetMoveState(int movestate);
void	trap_BotMoveToGoal(void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal, int travelflags);
int		trap_BotMoveInDirection(int movestate, vec3_t dir, float speed, int type);
void	trap_BotResetAvoidReach(int movestate);
void	trap_BotResetLastAvoidReach(int movestate);
int		trap_BotReachabilityArea(vec3_t origin, int testground);
int		trap_BotMovementViewTarget(int movestate, void /* struct bot_goal_s */ *goal, int travelflags, float lookahead, vec3_t target);
int		trap_BotPredictVisiblePosition(vec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags, vec3_t target);
int		trap_BotAllocMoveState(void);
void	trap_BotFreeMoveState(int handle);
void	trap_BotInitMoveState(int handle, void /* struct bot_initmove_s */ *initmove);
// Ridah
void	trap_BotInitAvoidReach(int handle);
// done.

int		trap_BotChooseBestFightWeapon(int weaponstate, int *inventory);
void	trap_BotGetWeaponInfo(int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo);
int		trap_BotLoadWeaponWeights(int weaponstate, char *filename);
int		trap_BotAllocWeaponState(void);
void	trap_BotFreeWeaponState(int weaponstate);
void	trap_BotResetWeaponState(int weaponstate);

int		trap_GeneticParentsAndChildSelection(int numranks, float *ranks, int *parent1, int *parent2, int *child);

void	trap_SnapVector( float *v );

int		trap_RealTime(qtime_t * qtime);

typedef enum
{
	shard_glass = 0,
	shard_wood,
	shard_metal,
	shard_ceramic,
	shard_rubble
} shards_t;

/***************************
	L0 - New stuff bellow
***************************/

//
// g_shared.c
//
void DecolorString(char *in, char *out);
char *getDateTime( void );
char *getDate( void );
const char *getMonthString(int monthIndex);
int getYearFromCYear(int cYear);
int getDaysInMonth(int monthIndex);
qboolean isValidIP(const char *ip);
void Q_Tokenize(char *str, char **splitstr, char *delim);
void APSound(char *sound);
void CPSound(gentity_t *ent, char *sound);
void APRSound(gentity_t *ent, char *sound);
void ParseStr(const char *strInput, char *strCmd, char *strArgs);
void ParseStr2(char data[], char **out, int *start);
char *CleanFileText(char *text);
char *ParseFileText(char *text);
char *TablePrintableColorName(const char *name, int maxlength);
void Q_RemoveLocalization(char *message);
qboolean FindFreePosition(gentity_t *ent, vec3_t free_position, int step_count);
qboolean FileExists(char *filename, char *directory, char *expected_extension, qboolean can_have_extension);
gentity_t *LaunchEntity(gentity_t *ent, vec3_t origin, vec3_t velocity, int ownerNum);
char *GetHostname(void);
gentity_t *FindRevivableTeammate(gentity_t *ent, int skip_num, vec3_t dir, vec3_t initial_start, vec3_t new_start, vec3_t end);
void G_SetConstantLight(gentity_t *ent, int red, int green, int blue, int intensity);
int G_GetPackedIpAddress(char *ip_address);
char *G_GetUnpackedIpAddress(int packed_ip_address, qboolean full);

//
// g_files.c
//
void BanClient(char arg[MAX_TOKEN_CHARS]);
void TempbanClient(gentity_t *ent, int minsbanned);
void TempbanIp(char *ip, int minsbanned);
char *IsTempBanned(char * Clientip);
qboolean IsBanned(char *ip, char *password);
void RemoveExpiredTempbans(void);

//
// g_antilag.c
//
void G_ResetTrail(gentity_t *ent);
void G_StoreTrail(gentity_t *ent);
void G_TimeShiftAllClients(int time, gentity_t *skip);
void G_UnTimeShiftAllClients(gentity_t *skip);

//
// g_hacks.c
//
void print_mod(gentity_t *attacker, gentity_t *self, int meansOfDeath);

//
// g_match.c
//
void CountDown(void);
gentity_t *G_FearCheck(gentity_t *ent);
void matchInfo(unsigned int type, char *msg);
void checkEvenTeams(void);
void balanceTeams(void);
int isWeaponLimited(gclient_t *client, int weap);
qboolean isWeaponBalanced(int weapon);
void setDefaultWeapon(gclient_t *client, qboolean isSold);
int FlagBalance(void);
void CheckWeaponRestrictions(gclient_t *client);

// 
// g_players.c
//
void Cmd_getStatus(gentity_t *ent);
void Cmd_throwKnives(gentity_t *ent);
void Cmd_SendPrivateMessage_f(gentity_t *ent);
void Cmd_Time(gentity_t *ent);
void Cmd_Drag(gentity_t *ent);
void Cmd_Push(gentity_t* ent);
void Cmd_dropObj(gentity_t *self);
void Cmd_Stats(gentity_t *ent);
void weapon_smokeGrenade(gentity_t *ent);
void cmd_noReload(gentity_t *ent);
void Cmd_Smoke(gentity_t *ent);

//
// g_stats.c
//
void stats_DoubleKill(gentity_t *ent, int meansOfDeath);
void stats_FirstHeadshot(gentity_t *attacker, gentity_t *targ);
void stats_FirstBlood(gentity_t *self, gentity_t *attacker);
void stats_LastBloodMessage(qboolean fOut);
void stats_KillingSprees(gentity_t *ent);
void stats_DeathSpree(gentity_t *ent);
void stats_MatchInfo(void);
void stats_ffaMatchInfo(void);
void stats_StoreMapStat(gentity_t *ent, int score, int type);
void stats_MapStats(void);
void stats_StoreRoundStat(char *player, int score, int stat);
void stats_WriteRoundStats(void);
void stats_RoundStats(void);
void stats_UpdateDailyRankings(void);
void stats_InitDailyRankings(void);
void stats_DisplayDailyRanking(gentity_t *ent);
void stats_DisplayDailyStats(gentity_t *ent);
void stats_CheckDailyReset(void);
daily_stats_t *stats_GetDailyStatsForClient(gclient_t *cl);
void stats_UpdateDailyStatsForClient(gclient_t *cl, daily_stats_t *stats);
void stats_InitRoundStats(void);
void stats_SubmitGlobalStats(void);
void stats_GlobalStatsReconnect(void);

//
// g_admin_bot.c
//
void SB_maxTeamKill(gentity_t *ent);
void SB_maxTeamBleed(gentity_t *ent);
void SB_minLowScore(gentity_t *ent);
void SB_maxPingFlux(gclient_t *client);
void SB_chatWarn(gentity_t *ent);

//
// g_geoip.c //Elver
//

typedef struct GeoIPTag {
	fileHandle_t GeoIPDatabase;
	unsigned char* cache;
	unsigned int memsize;
} GeoIP;

unsigned long GeoIP_addr_to_num(const char* addr);
unsigned int GeoIP_seek_record(GeoIP* gi, unsigned long ipnum);
void GeoIP_open(void);
void GeoIP_close(void);

extern GeoIP* gidb;



//
// g_censored.c
//
typedef struct {
	int num_nulled_words;
} wordDictionary;

extern wordDictionary censorDictionary;
extern wordDictionary censorNamesDictionary;

qboolean G_CensorName(char *testname, char *userinfo, int clientNum);
qboolean G_CensorText(char *text, wordDictionary *dictionary);

// 
// Macros
//
#define AP(x)		trap_SendServerCommand(-1, x)				// Print to all
#define CP(x)		trap_SendServerCommand(ent-g_entities, x)	// Print to an ent
#define CPx(x, y)	trap_SendServerCommand(x, y)				// Print to id = x
#define TP(x,y,z)	G_SayToTeam(x, y, z)						// Prints to selected team
#define APS(x)		APSound(x)									// Global sound 
#define APRS(x, y)	APRSound(x, y)								// Global sound with limited (radius) range
#define CPS(x, y)	CPSound(x, y)								// Client sound only

#define MT_EI	0												// Match Times - End info
#define MT_ME	1												// Match Times - Match Event(s)

// Clean file output
#define INVALID_CHAR 26
#define INVALID_CHAR_REPLACEMENT -125

// Countdown
#define CD_PREPARE 1
#define CD_FIGHT 2
#define COUNTDOWN_START 7100

// Stats
#define INITIAL_WAIT 3000
#define STATS_INTERVAL 3600

// Client Weapon Numbers
#define SW_MP40 3
#define SW_THOMPSON 4
#define SW_STEN 5
#define SW_MAUSER 6
#define SW_PANZER 8
#define SW_VENOM 9
#define SW_FLAMER 10

#endif // __SHARED_H
