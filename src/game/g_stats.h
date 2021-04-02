/*/
===========================================================================
L0 / rtcwPub :: g_stats.h

	Stats prototypes

Created: 25. Mar / 2014
===========================================================================
*/
#ifndef __STATS_H
#define __STATS_H

// Path where stats will be stored
#define STSPTH "./stats/"

/**** Killing sprees (consistent over round) ****/
typedef struct {
	char *msg;
	char *snd;
} killing_sprees_t;

#define MAX_KILLING_SPREES 21

static const killing_sprees_t killingSprees[MAX_KILLING_SPREES] = {
	{ "MULTI KILL!", "multikill.wav" },
	{ "MEGA KILL!", "megakill.wav" },
	{ "RAMPAGE!", "rampage.wav" },
	{ "GUNSLINGER!", "gunslinger.wav" },
	{ "ULTRA KILL!", "ultrakill.wav" },
	{ "MANIAC!", "maniac.wav" },
	{ "SLAUGHTER!", "slaughter.wav" },
	{ "MASSACRE!", "massacre.wav" },
	{ "IMPRESSIVE!", "impressive.wav" },
	{ "DOMINATING!", "dominating.wav" },
	{ "BLOODBATH!", "bloodbath.wav" },
	{ "KILLING MACHINE!", "killingmachine.wav" },
	{ "MONSTER KILL!", "monsterkill.wav" },
	{ "LUDICROUS KILL!", "ludicrouskill.wav" },
	{ "UNSTOPPABLE!", "unstoppable.wav" },
	{ "UNREAL!", "unreal.wav" },
	{ "OUTSTANDING!", "outstanding.wav" },
	{ "WICKED SICK!", "wickedsick.wav" },
	{ "HOLY SHIT!", "holyshit.wav" },
	{ "BLAZE OF GLORY!!", "blazeofglory.wav" },
	{ NULL, NULL }
};

/**** Map Stats ****/
#define MAP_KILLER			1
#define MAP_KILLING_SPREE   2
#define MAP_VICTIM			3
#define MAP_DEATH_SPREE		4
#define MAP_REVIVES			5
#define MAP_HEADSHOTS		6

/**** RMS (Round Match Stats) ****/
#define ROUND_KILLS			0
#define ROUND_DEATHS		1
#define ROUND_HEADSHOTS		2
#define ROUND_TEAMKILLS		3
#define ROUND_TEAMBLEED		4
#define ROUND_POISON		5
#define ROUND_REVIVES		6
#define ROUND_AMMOGIVEN		7
#define ROUND_MEDGIVEN		8
#define ROUND_GIBS			9
#define ROUND_SUICIDES		10
#define ROUND_KNIFETHROW	11
#define ROUND_FASTSTABS		12
#define ROUND_CHICKEN		13
#define ROUND_KILLPEAK		14
#define ROUND_DEATHPEAK		15
#define ROUND_ACC			16
#define ROUND_KR			17
#define ROUND_EFF			18

// Should always be last!
#define ROUND_LIMIT			19

#define RS_MAX_PLAYERS 20
#define RS_FILE "roundStats.txt"

typedef struct {
	char *reward;
	char *sound;
	qboolean used;
	qboolean disabled;
	int score;			// Most of stats
	char players[MAX_NETNAME*RS_MAX_PLAYERS];	// Players that made the list
	char out[64];		// For output
} round_stats_t;

#define DAILY_PATH "daily_stats.dat"
#define MIN_KILLS_DECREASE -9999999
#define DSVER 3
#define DSID  (('S' << 0) | ('T' << 8) | ('A' << 16) | ('T' << 24))

typedef struct {
	int id;
	byte version;
	byte playercount;
	time_t reset_ctime;
} daily_stats_header_t;

#endif // __STATS_H
