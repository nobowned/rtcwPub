/*/
===========================================================================
L0 / rtcwPub :: g_stats.c

	All the eye candy is placed here.

Created: 25. Mar / 2014
===========================================================================
*/
#include "g_local.h"

static qboolean firstheadshot;
static qboolean firstblood;

round_stats_t roundStats[ROUND_LIMIT] = {
	{ "Most Kills", "sound/player/excellent.wav" },
	{ "Most Deaths", "sound/player/you_suck.wav" },
	{ "Most Headshots", "sound/player/headhunter.wav" },
	{ "Most Team Kills", "sound/player/teamkiller.wav" },
	{ "Most Team Bleeding", "sound/player/teambleeder.wav" },
	{ "Most Poisons", "sound/props/throw/chairthudgrunt.wav" },
	{ "Most Revives", "sound/multiplayer/allies/a-medic1.wav" },
	{ "Most Ammo Given", "sound/multiplayer/allies/a-ammo.wav" },
	{ "Most Med Given", "sound/multiplayer/allies/a-medic1.wav" },
	{ "Most Gibs", "sound/player/gibsplt1.wav" },
	{ "Most Suicides", "sound/player/suicides.wav" },
	{ "Knife Juggler", "sound/player/knife_juggler.wav" },
	{ "Stealth Kills", "sound/player/assasin.wav" },
	{ "Coward", "sound/player/chicken.wav" },
	{ "Highest Kill Spree", "sound/player/killingmachine.wav" },
	{ "Highest Death Spree", "sound/player/slaughter.wav" },
	{ "Highest Accuracy", "sound/player/accuracy.wav" },
	{ "Highest Kill Ratio", "sound/player/outstanding.wav" },
	{ "Most Efficient", "sound/player/impressive.wav" }
};

static daily_stats_t dailyplayers[UCHAR_MAX];
static byte dailycount;
static byte ranked;
static time_t reset_ctime;

/*
===========
Double+ kills
===========
*/
void stats_DoubleKill(gentity_t *ent, int meansOfDeath) {
	if (!g_doubleKills.integer) {
		return;
	}
	if (g_gamestate.integer == GS_WARMUP_COUNTDOWN) {
		return;
	}
	if (!ent || !ent->client) {
		return;
	}

	// White list approach makes more sense.
	if (meansOfDeath == MOD_LUGER				// handgun
		|| meansOfDeath == MOD_COLT				// handgun
		|| meansOfDeath == MOD_KNIFE_STEALTH	// knife -- can be done :)
		|| meansOfDeath == MOD_THROWKNIFE		// Kill & a knive throw etc
		|| meansOfDeath == MOD_THOMPSON			// Thompson
		|| meansOfDeath == MOD_MP40				// MP40
		|| meansOfDeath == MOD_STEN				// STEN 
		|| meansOfDeath == MOD_POISONDMED		// Poison & kill etc
		) {

		if ((level.time - ent->client->lastKillTime) > 1000) {
			ent->client->doublekill = 0;
			ent->client->lastKillTime = level.time;
			return;
		}
		else {
			ent->client->doublekill++;
			ent->client->lastKillTime = level.time;
		}

		switch (ent->client->doublekill) {
			// 2 kills
			case 1:
				APS("sound/player/doublekill.wav");
				AP(va("chat \"^3Double Kill! ^7%s\n\"", ent->client->pers.netname));
			break;
			// 3 kills
			case 2:
				APS("sound/player/tripplekill.wav");
				AP(va("chat \"^3Triple Kill! ^7%s\n\"", ent->client->pers.netname));
			break;
			// 4 kills
			case 3:
				APS("sound/player/combowhore.wav");
				AP(va("chat \"^3Pure Ownage! ^7%s\n\"", ent->client->pers.netname));
			break;
		}
	}
}

/*
===========
First headshots

Prints who done first headshots when round starts.
===========
*/
void stats_FirstHeadshot(gentity_t *attacker, gentity_t *targ) {
	qboolean 	onSameTeam = OnSameTeam(targ, attacker);

	if (g_showFirstHeadshot.integer && g_gamestate.integer == GS_PLAYING) {

		if (!firstheadshot &&
			targ &&
			targ->client &&
			attacker &&
			attacker->client &&
			attacker->s.number != ENTITYNUM_NONE &&
			attacker->s.number != ENTITYNUM_WORLD &&
			attacker != targ &&
			g_gamestate.integer == GS_PLAYING &&
			!onSameTeam)
		{
			AP(va("chat \"%s ^7blew out %s^7's brains with the ^3FIRST HEAD SHOT^7!\"", attacker->client->pers.netname, targ->client->pers.netname));
			APS("sound/player/headshot.wav");
			firstheadshot = qtrue;
		}
	}
}

/*
===========
First blood

Prints who draw first blood when round starts.
NOTE: Atm it's only a print..once I'm not lazy I'll set it in a way it can decide winner once timelimit hits on
specific maps (like depot, destuction) - so first blood decides who won.
===========
*/
void stats_FirstBlood(gentity_t *self, gentity_t *attacker) {
	qboolean 	onSameTeam = OnSameTeam(self, attacker);

	if (g_showFirstBlood.integer && g_gamestate.integer == GS_PLAYING) {

		if (!firstblood &&
			self &&
			self->client &&
			attacker &&
			attacker->client &&
			attacker->s.number != ENTITYNUM_NONE &&
			attacker->s.number != ENTITYNUM_WORLD &&
			attacker != self &&
			g_gamestate.integer == GS_PLAYING &&
			!onSameTeam)
		{
			AP(va("chat \"%s ^7drew ^1FIRST BLOOD ^7from %s^7!\"", attacker->client->pers.netname, self->client->pers.netname));
			APS("sound/player/firstblood.wav");
			firstblood = qtrue;
		}
	}
}

/*
===========
Last Blood

Prints in console at the end of the round
===========
*/
void stats_LastBloodMessage(qboolean fOut)
{
	if (g_showLastBlood.integer)
	{
		if (Q_stricmp(level.lastKiller, ""))
		{
			if (!fOut)
			{
				if (Q_stricmp(level.lastVictim, ""))
					AP(va("print \"%s ^7drew ^1Last Blood^7 from %s^7!\n\"", level.lastKiller, level.lastVictim));
				else
					AP(va("print \"%s ^7drew the ^1Last Blood^7!\n\"", level.lastKiller));
			}
			else {
				if (Q_stricmp(level.lastVictim, ""))
					AP(va("print \"\n^1Last Blood^7: %s ^7rocked %s^7's world!\n\n\"", level.lastKiller, level.lastVictim));
				else
					AP(va("print \"\n^1Last Blood^7: %s ^7donated the Last Drop of Blood...\n\n\"", level.lastKiller));
			}
		}
	}
}

/*
===========
Killing sprees
===========
*/
void stats_KillingSprees(gentity_t *ent)
{
	int player_kills;
	int snd_idx, i;
	gentity_t *target_ent;
	const killing_sprees_t *ks;

	if (!g_killingSprees.integer || g_gamestate.integer != GS_PLAYING)
	{
		return;
	}

	player_kills = ent->client->pers.stats.kills;

	if (player_kills == 0)
	{
		return;
	}

	if ((player_kills <= 100 && (player_kills % 5) == 0) ||
		player_kills > 100 && (player_kills % 10) == 0)
	{
		snd_idx = player_kills / 5 - 1;
		
		if (snd_idx >= MAX_KILLING_SPREES)
		{
			snd_idx = MAX_KILLING_SPREES - 1;
		}

		ks = &killingSprees[snd_idx];

		AP(va("chat \"^5%s ^5(^7%dk, %dhs^5)^7: %s\n\"", ks->msg, player_kills, ent->client->pers.stats.headshots, ent->client->pers.netname));

		for (i = 0; i < level.maxclients; i++)
		{
			target_ent = g_entities + i;

			if (target_ent->client->pers.connected != CON_CONNECTED)
			{
				continue;
			}
			if (target_ent->client->sess.colorFlags & CF_MULTIKILL_OFF)
			{
				continue;
			}

			CPSound(target_ent, va("sound/player/%s", ks->snd));
		}
	}
}

/*
===========
Death spree
===========
*/
void stats_DeathSpree(gentity_t *ent) {
	int deaths = ent->client->pers.spreeDeaths;
	int n = rand() % 2;
	char *snd = "", *spree = "";

	if (!g_deathSprees.integer || deaths <= 0 || g_gamestate.integer != GS_PLAYING)
		return;

	if (deaths == 9) {
		if (n == 0) { spree = va("(^710 Dth^0)"); snd = "dSpree1.wav"; }
		else { spree = va("(^710 Dth^0)"); snd = "dSpree1_alt.wav"; }
	}
	else if (deaths == 14) {
		if (n == 0) { spree = va("(^715 Dth^0)"); snd = "dSpree2.wav"; }
		else { spree = va("(^715 Dth^0)"); snd = "dSpree2_alt.wav"; }
	}
	else if (deaths == 19) {
		if (n == 0) { spree = va("(^720 Dth^0)"); snd = "dSpree3.wav"; }
		else { spree = va("(^720 Dth^0)"); snd = "dSpree3_alt.wav"; }
	}
	else if (deaths == 24) {
		if (n == 0) { spree = va("(^725 Dth^0)"); snd = "dSpree4.wav"; }
		else { spree = va("(^725 Dth^0)"); snd = "dSpree4_alt.wav"; }
	}

	// Has to be offset by 1 as count comes late..
	if (deaths == 9 || deaths == 14 || deaths == 19 || deaths == 24) {
		AP(va("chat \"^0DEATHSPREE! %s: ^7%s\n\"", spree, ent->client->pers.netname));
		APS(va("sound/player/%s", snd));
	}
}

/*
===========
EOM (End of Match) stats
===========
*/
void stats_MatchInfo(void) {
	int i, j, r, cnt, teamcnt;
	float tot_acc = 0.00f;
	int tot_kills, tot_deaths, tot_hs, tot_gib, tot_tk, tot_dg, tot_dr, tot_td, tot_hits, tot_shots, tot_revives;
	gclient_t *cl;
	gentity_t *ent;
	char *statcolor;
	char cs[MAX_INFO_STRING];
	qboolean is_winning_team;

	AP(va("print \"\nMod: %s \n^7Server: %s  \n^7Time: ^3%s\n\n\"",
		GAMEVERSION, GetHostname(), getDateTime()));

	trap_GetConfigstring(CS_MULTI_MAPWINNER, cs, sizeof(cs));
	team_t winning_team = Info_ValueForKey(cs, "winner") == "0" ? TEAM_RED : TEAM_BLUE;

	cnt = 0;
	for (i = TEAM_RED; i <= TEAM_BLUE; i++) {
		if (!TeamCount(-1, i)) {
			continue;
		}

		is_winning_team = winning_team == i;

		tot_kills = 0;
		tot_deaths = 0;
		tot_hs = 0;
		tot_gib = 0;
		tot_tk = 0;
		tot_dg = 0;
		tot_dr = 0;
		tot_td = 0;
		tot_hits = 0;
		tot_shots = 0;
		tot_acc = 0;
		tot_revives = 0;

		AP(va("print \"%s%s ^7Team%s\n"
			"^3-----------------------------------------------------------------------------\n"
			"^3Player Name          ^3: ^3K   D   G   TK  R   ^3: ^3Acc    HS  ^3: ^3DG     DR     TD   \n"
			"^3-----------------------------------------------------------------------------\n\"", 
			is_winning_team ? va("^3* ", 20, 18) : "", (i == TEAM_RED) ? "^1Axis" : "^4Allies", is_winning_team ? va(" ^3*", 20, 18) : ""));

		for (teamcnt = 0, j = 0; j < level.numPlayingClients; j++) {
			cl = level.clients + level.sortedClients[j];

			if (cl->pers.connected != CON_CONNECTED || cl->sess.sessionTeam != i) {
				continue;
			}

			tot_kills += cl->pers.stats.kills;
			tot_deaths += cl->pers.stats.deaths;
			tot_gib += cl->pers.stats.gibs;
			tot_tk += cl->pers.stats.teamKills;
			tot_hs += cl->pers.stats.headshots;
			tot_dg += cl->pers.stats.dmgGiven;
			tot_dr += cl->pers.stats.dmgReceived;
			tot_td += cl->pers.stats.dmgTeam;
			tot_hits += cl->pers.stats.acc_hits;
			tot_shots += cl->pers.stats.acc_shots;
			tot_revives += cl->pers.stats.revives;

			float playeracc = ((cl->pers.stats.acc_shots == 0) ? 0.00f : ((float)cl->pers.stats.acc_hits / (float)cl->pers.stats.acc_shots) * 100.00f);

			cnt++;
			teamcnt++;
			for (r = 0; r < level.maxclients; ++r) {
				ent = &g_entities[r];
				if (ent->inuse && !(ent->r.svFlags & SVF_BOT)) {
					statcolor = (ent->client == cl) ? "^2" : "^7";
					CP(va("print \"%s ^3: %s%-3d %-3d %-3d %-3d %-3d ^3: %s%-6.2f %-3d ^3: %s%-6d %-6d %-5d\n\"",
						TablePrintableColorName(cl->pers.netname, 20),
						statcolor,
						cl->pers.stats.kills,
						cl->pers.stats.deaths,
						cl->pers.stats.gibs,
						cl->pers.stats.teamKills,
						cl->pers.stats.revives,
						statcolor,
						playeracc,
						cl->pers.stats.headshots,
						statcolor,
						cl->pers.stats.dmgGiven,
						cl->pers.stats.dmgReceived,
						cl->pers.stats.dmgTeam));
				}
			}
		}

		tot_acc = ((tot_shots == 0) ? 0.00f : ((float)tot_hits / (float)tot_shots) * 100.00f);
		AP(va("print \""
			"^3-----------------------------------------------------------------------------\n"
			"^3Team Totals          ^3: ^7%-3d %-3d %-3d %-3d %-3d ^3: ^7%-6.2f %-3d ^3: ^7%-6d %-6d %-5d\n"
			"^3-----------------------------------------------------------------------------\n\n\"",
			tot_kills,
			tot_deaths,
			tot_gib,
			tot_tk,
			tot_revives,
			tot_acc,
			tot_hs,
			tot_dg,
			tot_dr,
			tot_td));
	}
	AP(va("print \"%s\n\" 0", ((!cnt) ? "^3\nNo scores to report." : "")));
}

/*
===========
EOM (End of Match) stats for the FFA gametype
===========
*/
void stats_ffaMatchInfo(void) {
	int j, r, cnt;
	float tot_acc, tot_kr;
	int tot_kills, tot_deaths, tot_gp, tot_hs, tot_gib, tot_tk, tot_dg, tot_dr, tot_td, tot_hits, tot_shots, tot_revives;
	gclient_t *cl;
	gentity_t *ent;
	char *statcolor;

	AP(va("print \"\nMod: %s \n^7Server: %s  \n^7Time: ^3%s\n\n\"",
		GAMEVERSION, GetHostname(), getDateTime()));

	cnt = 0;
	tot_kills = 0;
	tot_deaths = 0;
	tot_hs = 0;
	tot_gib = 0;
	tot_tk = 0;
	tot_dg = 0;
	tot_dr = 0;
	tot_td = 0;
	tot_gp = 0;
	tot_hits = 0;
	tot_shots = 0;
	tot_revives = 0;
	tot_acc = 0.00f;
	tot_kr = 0.00f;

	AP(va("print \""
		"^3-----------------------------------------------------------------------------\n"
		"^3Player Name          ^3: ^3K   D   G   KR     ^3: ^3Acc    HS  ^3: ^3DG     DR    \n"
		"^3-----------------------------------------------------------------------------\n\""));

	for (j = 0; j < level.numPlayingClients; j++) {
		cl = level.clients + level.sortedClients[j];

		tot_kills += cl->pers.stats.kills;
		tot_deaths += cl->pers.stats.deaths;
		tot_gib += cl->pers.stats.gibs;
		tot_hs += cl->pers.stats.headshots;
		tot_dg += cl->pers.stats.dmgGiven;
		tot_dr += cl->pers.stats.dmgReceived;
		tot_gp += cl->ps.persistant[PERS_SCORE];
		tot_hits += cl->pers.stats.acc_hits;
		tot_shots += cl->pers.stats.acc_shots;

		float acc = ((cl->pers.stats.acc_shots == 0) ? 0.00f : ((float)cl->pers.stats.acc_hits / (float)cl->pers.stats.acc_shots) * 100.00f);
		float kr = cl->pers.stats.deaths == 0 ? (float)cl->pers.stats.kills / 1.00f : (float)cl->pers.stats.kills / (float)cl->pers.stats.deaths;

		cnt++;
		for (r = 0; r < level.maxclients; ++r) {
			ent = &g_entities[r];
			if (ent->inuse && !(ent->r.svFlags & SVF_BOT)) {
				statcolor = (ent->client == cl) ? "^2" : "^7";
				CP(va("print \"%s ^3: %s%-3d %-3d %-3d %-6.2f ^3: %s%-6.2f %-3d ^3: %s%-6d %-6d\n\"",
					TablePrintableColorName(cl->pers.netname, 20),
					statcolor,
					cl->pers.stats.kills,
					cl->pers.stats.deaths,
					cl->pers.stats.gibs,
					kr,
					statcolor,
					acc,
					cl->pers.stats.headshots,
					statcolor,
					cl->pers.stats.dmgGiven,
					cl->pers.stats.dmgReceived,
					statcolor,
					cl->ps.persistant[PERS_SCORE]));
			}
		}
	}

	tot_acc = ((tot_shots == 0) ? 0.00f : ((float)tot_hits / (float)tot_shots) * 100.00f);
	tot_kr = tot_deaths == 0 ? (float)tot_kills / 1.00f : ((float)tot_kills / (float)tot_deaths);

	AP(va("print \""
		"^3-----------------------------------------------------------------------------\n"
		"^3Totals               ^3: ^7%-3d %-3d %-3d %-6.2f ^3: ^7%-6.2f %-3d ^3: ^7%-6d %-6d\n"
		"^3-----------------------------------------------------------------------------\n\n\"",
		tot_kills,
		tot_deaths,
		tot_gib,
		tot_kr,
		tot_acc,
		tot_hs,
		tot_dg,
		tot_dr,
		tot_gp
		));
	AP(va("print \"%s\n\" 0", ((!cnt) ? "^3\nNo scores to report." : "")));
}

/*
===========
Map Stats

This will be bunch of bouncing code around but should be simple enough (hopefully).
===========
*/
// Builds Message String
char *mapStatsMsg(char *map, unsigned int score, char *player, char *date) {
	if (g_mapStats.integer == 1)
		return va("^1HALL OF FAME(^7%s^1) ^7>> Top Killer(^1%d^7): %s ^7/ Achieved: ^1%s^7\n\"",
		map, score, player, ((date == NULL) ? "previous round" : date));
	else if (g_mapStats.integer == 2)
		return va("^1HALL OF FAME(^7%s^1) ^7>> Top Killing Spree(^1%d^7): %s ^7/ Achieved: ^1%s^7\n\"",
		map, score, player, ((date == NULL) ? "previous round" : date));
	else if (g_mapStats.integer == 3)
		return va("^1HALL OF SHAME(^7%s^1) ^7>> Most Deaths(^1%d^7): %s ^7/ Achieved: ^1%s^7\n\"",
		map, score, player, ((date == NULL) ? "previous round" : date));
	else if (g_mapStats.integer == 4)
		return va("^1HALL OF SHAME(^7%s^1) ^7>> Highest Death Spree(^1%d^7): %s ^7/ Achieved: ^1%s^7\n\"",
		map, score, player, ((date == NULL) ? "previous round" : date));
	else if (g_mapStats.integer == 5)
		return va("^1HALL OF FAME(^7%s^1) ^7>> Most Revives(^1%d^7): %s ^7/ Achieved: ^1%s^7\n\"",
		map, score, player, ((date == NULL) ? "previous round" : date));
	else if (g_mapStats.integer == 6)
		return va("^1HALL OF FAME(^7%s^1) ^7>> Most Headshots(^1%^7): %s ^7/ Achieved: ^1%s^7\n\"",
		map, score, player, ((date == NULL) ? "previous round" : date));
	else
		return NULL;
}

// Write stats (unconditional)
void add_MapStats(char *file) {
	fileHandle_t statsFile;
	char *entry;

	entry = va("%d\\%s\\%s\\null", level.topScore, CleanFileText(level.topOwner), getDateTime());

	// Re-create new file (overwrites old one)
	trap_FS_FOpenFile(file, &statsFile, FS_WRITE);
	trap_FS_Write(entry, strlen(entry), statsFile);
	trap_FS_FCloseFile(statsFile);
}

// Set cvar
void set_mapStats(char *map, unsigned int score, char *player, char *date) {
	if (!level.mapStatsPrinted)
		AP(va("chat \"%s\n\"", mapStatsMsg(map, score, player, date)));
}

// Check stats
void handle_MapStats(char *map, char *suffix) {
	fileHandle_t	statsFile;
	char	filename[255], data[512];
	int		file;
	char	*token[512] = { NULL };

	Com_sprintf(filename, 255, "%s%s_%s", STSPTH, map, suffix);
	file = trap_FS_FOpenFile(filename, &statsFile, FS_READ);
	
	// Check|Write
	if (file > 0)
	{
		// read the existing data
		trap_FS_Read(data, file, statsFile);
		trap_FS_FCloseFile(statsFile);
		// score\\name\\date
		Q_Tokenize(data, token, "\\");

		// New record!
		if (token[0] && atoi(token[0]) < level.topScore)
		{
			add_MapStats(filename);
			set_mapStats(map, level.topScore, level.topOwner, NULL);

			if (g_mapStatsNotify.integer)
				AP(va("print \"^1New Map Record! ^7Be it good or bad, %s ^7has the stuff Legends are made of!\n\"",
				level.topOwner));
		}
		else
		{
			set_mapStats(map, atoi(token[0]), ParseFileText(token[1]), token[2]);
		}
	}
	else
	{	
		// Close the pointer..
		trap_FS_FCloseFile(statsFile);

		// Create it if there's an entry for it..
		if (level.topScore && level.topOwner)
		{
			add_MapStats(filename);
			set_mapStats(map, level.topScore, level.topOwner, NULL);

			if (g_mapStatsNotify.integer)
				AP(va("print \"^1New Map Record! ^7Be it good or bad, %s ^7has the stuff Legends are made of!\n\"",
				level.topOwner));
		}
	}
}

// Writes stats during game
void stats_StoreMapStat(gentity_t *ent, int score, int type) {
	// FCFS method!
	if (g_mapStats.integer &&
		score > level.topScore &&
		g_gamestate.integer == GS_PLAYING &&
		type == g_mapStats.integer)
	{
		Q_strncpyz(level.topOwner, "", sizeof(level.topOwner));
		strcat(level.topOwner, ent->client->pers.netname);
		level.topScore = score;
	}
}

// Front end for mapStats handling..
void stats_MapStats(void) {
	char mapName[64];
	char *suffix = "killer";
	
	if (!g_mapStats.integer)
		return;

	trap_Cvar_VariableStringBuffer("mapname", mapName, sizeof(mapName));

	// NOTE: Add more if desired/needed and simply patch/add the storing call in appropriate places..
	switch (g_mapStats.integer) {
		// Top Killer (most overall kills)
		case 1:
			suffix = "killer";
		break;
		// Top Killer Spree (most kills in a row)
		case 2:
			suffix = "killingSpree";
		break;
		// Top Victim (most overall deaths)
		case 3:
			suffix = "victim";
		break;
		// Top Death Spree (most deaths in a row)
		case 4:
			suffix = "deathSpree";
		break;
		// Top Medic (most revives)
		case 5:
			suffix = "revives";
		break;
		// Top Headshoter
		case 6:
			suffix = "headshots";
		break;
	}

	handle_MapStats(mapName, suffix);
	level.mapStatsPrinted = qtrue;
}

/*
===========
Round Stats

Stats that are printed each round.
===========
*/

// Process stats during match
void stats_StoreRoundStat(char *player, int score, int stat) {
	round_stats_t *rs;
	// Q_ChrReplace modifies the first paramter.. Safely copy the original name.
	char player_name_safe_copy[MAX_NETNAME];

	if (!g_roundStats.integer || level.warmupTime)
		return;

	rs = roundStats + stat;

	if (rs->disabled)
		return;

	if (score && rs->score <= score) {
		Q_strncpyz(player_name_safe_copy, player, sizeof(player_name_safe_copy));
		// 12 is one of the printable characters that comes out as a space/empty. So it works here to fix
		// the problem where spaces are sometimes newlined by the client's CenterPrint and color is removed.
		Q_ChrReplace(player_name_safe_copy, ' ', 12);
		// Adds comma if not a single entry
		if (rs->score == score) {
			char *newp = va("^7, %s", player_name_safe_copy);
			if (strlen(newp) + strlen(rs->players) + 2 < sizeof(rs->players))
				strcat(rs->players, newp);
		}
		else {
			Q_strncpyz(rs->players, player_name_safe_copy, sizeof(rs->players));
			rs->used = qtrue;
			rs->score = score;
		}
	}
}

// sort what's left before writing to file (efficency, killer ratio & accuracy)
void stats_ProcessExtraRoundStats(void) {
	gclient_t *cl;
	int i;
	float topKr = 0, topEff = 0, topAcc = 0;
	float kr, eff, acc;
	round_stats_t *rs;
	// Q_ChrReplace modifies the first paramter.. Safely copy the original name.
	char player_name_safe_copy[MAX_NETNAME];

	for (i = 0; i < g_maxclients.integer; i++)
	{
		cl = level.clients + i;

		if (cl->pers.connected != CON_CONNECTED)
			continue;

		int shots = cl->pers.stats.acc_shots;
		int hits = cl->pers.stats.acc_hits;
		int kills = cl->pers.stats.kills;
		int deaths = cl->pers.stats.deaths;
		Q_strncpyz(player_name_safe_copy, cl->pers.netname, sizeof(player_name_safe_copy));
		// 12 is one of the printable characters that comes out as a space/empty. So it works here to fix
		// the problem where spaces are sometimes newlined by the client's CenterPrint and color is removed.
		Q_ChrReplace(player_name_safe_copy, ' ', 12);

		rs = roundStats + ROUND_EFF;
		if (!rs->disabled)
		{
			eff = (deaths + kills == 0) ? 0 : 100 * kills / (deaths + kills);

			if (eff && eff >= topEff) {
				if (eff == topEff) {
					strcat(rs->players, va("^7, %s", player_name_safe_copy));
				}
				else {
					rs->used = qtrue;
					Q_strncpyz(rs->players, player_name_safe_copy, sizeof(rs->players));
					Q_strncpyz(rs->out, va("%2.2f", eff), sizeof(rs->out));
					topEff = eff;
				}
			}
		}

		rs = roundStats + ROUND_KR;
		if (!rs->disabled)
		{
			kr = (float)kills / (!deaths ? 1 : (float)deaths);

			if (kr && kr >= topKr) {
				if (kr == topKr) {
					strcat(rs->players, va("^7, %s", player_name_safe_copy));
				}
				else {
					rs->used = qtrue;
					Q_strncpyz(rs->players, player_name_safe_copy, sizeof(rs->players));
					Q_strncpyz(rs->out, va("%2.2f", kr), sizeof(rs->out));
					topKr = kr;
				}
			}
		}

		rs = roundStats + ROUND_ACC;
		if (!rs->disabled)
		{
			acc = (!shots) ? 0.00f : ((float)hits / (float)shots) * 100.0f;

			if (acc && acc > topAcc) {
				rs->used = qtrue;
				Q_strncpyz(rs->players, player_name_safe_copy, sizeof(rs->players));
				Q_strncpyz(rs->out, va("%2.2f (%d/%d)", acc, hits, shots), sizeof(rs->out));
				topAcc = acc;
			}
		}
	}

	return;
}

// Write (to file) stats
void stats_WriteRoundStats(void) {
	FILE *rsf;
	int stat;

	stats_ProcessExtraRoundStats();

	rsf = fopen(RS_FILE, "w+");

	if (rsf) {
		for (stat = 0; stat < ROUND_LIMIT; stat++) {
			round_stats_t *rs = roundStats + stat;

			if (rs->used) {
				if (stat < ROUND_ACC)
					Q_strncpyz(rs->out, va("%i", rs->score), sizeof(rs->out));

				fputs(va("stat\\%i\\out\\%s\\players\\%s\n", stat, rs->out, CleanFileText(rs->players)), rsf);
			}
		}

		fclose(rsf);
	}
	else {
		G_LogPrintf("roundStats.txt file could not be opened. Aborting storage of round stats for this round\n");
	}
}

// Read stats
void stats_ReadRoundStats(void) {
	FILE	*rsf;
	int		stat;
	char	data[sizeof(roundStats[0].out) + sizeof(roundStats[0].players) + 5];
	
	rsf = fopen(RS_FILE, "a+");

	while (fgets(data, sizeof(data), rsf) != NULL) {
		stat = atoi(Info_ValueForKey(data, "stat"));
		round_stats_t *rs = roundStats + stat;

		Q_strncpyz(rs->out, Info_ValueForKey(data, "out"), sizeof(rs->out));
		Q_strncpyz(rs->players, ParseFileText(Info_ValueForKey(data, "players")), sizeof(rs->players));
		rs->used = qtrue;
	}

	fclose(rsf);
	remove(RS_FILE);
}

// Compute warmup time
void stats_InitRoundStats()
{
	int stat, warmupTime;
	int statCount = 0;

	stats_ReadRoundStats();

	for (stat = 0; stat < ROUND_LIMIT; stat++) {
		if (roundStats[stat].used)
			statCount++;
		// Set disabled flag on round stat
		if (g_gamestate.integer == GS_PLAYING)
			roundStats[stat].disabled = Q_FindToken(g_excludedRoundStats.string, va("%d", stat));
	}

	if (statCount) {
		warmupTime = (COUNTDOWN_START + INITIAL_WAIT + (statCount * STATS_INTERVAL)) / 1000;

		if (warmupTime > g_warmup.integer) {
			trap_Cvar_Set("g_warmup", va("%i", warmupTime));
		}

		// Wait a bit before starting, otherwise the first round stat will be skipped
		level.rsNextPrintTime = level.time + INITIAL_WAIT;
	}
}

// Front end
void stats_RoundStats(void)
{
	static int stat;

	for (; stat < ROUND_LIMIT; ++stat)
	{
		round_stats_t *rs = roundStats + stat;

		if (rs->used)
		{
			AP(va("cp \"^3%s: ^7%s\n^7%s \n\"2", rs->reward, rs->out, rs->players));
			APS(va("%s", rs->sound));

			break;
		}
	}

	level.rsNextPrintTime = level.time + STATS_INTERVAL;
	stat++;
}

float GetPlayerEfficiency(const daily_stats_t *player)
{
	if (player->stats.kills < g_dailyMinimumKills.integer) {
		return MIN_KILLS_DECREASE;
	}
	else {
		int safe_deaths = player->stats.deaths == 0 ? 1 : player->stats.deaths;
		return
			((float)((player->stats.kills + player->stats.revives) / (float)safe_deaths)) +
			((float)(player->stats.kills / 1000)) +
			((float)(player->stats.revives / 500)) +
			((float)(player->stats.acc_hits) / (float)(player->stats.acc_shots == 0 ? 1 : player->stats.acc_shots));
	}
}

int DailyCompare(const void *a, const void *b)
{
	float player1Eff = GetPlayerEfficiency(a);
	float player2Eff = GetPlayerEfficiency(b);

	if (player1Eff > player2Eff)
		return -1;

	return player1Eff < player2Eff;
}

void stats_InitDailyRankings()
{
	int i;
	fileHandle_t dailyfp;
	daily_stats_header_t header;
	daily_stats_t *dp;

	dailycount = 0;
	reset_ctime = 0;

	trap_FS_FOpenFile(DAILY_PATH, &dailyfp, FS_READ);
	if (!dailyfp) {
		return;
	}

	trap_FS_Read(&header, sizeof(daily_stats_header_t), dailyfp);
	if (header.id != DSID) {
		G_LogPrintf("This dailystats file has an incorrect identifier and cannot be used. Delete it and try again.");
		trap_FS_FCloseFile(dailyfp);
		return;
	}
	if (header.version != DSVER) {
		G_LogPrintf("Dailystats file version mismatch. Found %d, expected %d", header.version, DSVER);
		trap_FS_FCloseFile(dailyfp);
		return;
	}

	reset_ctime = header.reset_ctime;
	// No reset hour or the cvar changed? Then compute/re-compute reset time.
	if (reset_ctime <= 0 || localtime(&reset_ctime)->tm_hour != g_dailyStatsResetHour.integer) {
		time_t current_ctime = time(NULL);
		struct tm *current_tm = localtime(&current_ctime);

		// Clear min and sec, otherwise the reset_ctime could (and most likely will) surpass the hour.
		current_tm->tm_min = 0;
		current_tm->tm_sec = 0;

		// The current hour has already surpassed our reset hour, so set our reset time to the next day at g_dailyStatsResetHour.
		if (current_tm->tm_hour >= g_dailyStatsResetHour.integer) {
			current_tm->tm_hour = g_dailyStatsResetHour.integer;
			current_tm->tm_mday++;
			reset_ctime = mktime(current_tm);
			// Not sure what causes mktime to fail. It'll auto-wrap days/months/years that are too large.
			if (reset_ctime == -1) {
				G_LogPrintf("Daily stats reset time could not be computed.. Aborting.");
				return;
			}
		}
		else {
			current_tm->tm_hour = g_dailyStatsResetHour.integer;
			reset_ctime = mktime(current_tm);
			// I guess this can happen if the user provides an invalid hour?
			if (reset_ctime == -1) {
				G_LogPrintf("Daily stats reset time could not be computed.. Aborting.");
				return;
			}
		}
	}

	dailycount = header.playercount;
	trap_FS_Read(dailyplayers, dailycount * sizeof(daily_stats_t), dailyfp);

	trap_FS_FCloseFile(dailyfp);

	qsort(dailyplayers, dailycount, sizeof(daily_stats_t), DailyCompare);

	ranked = dailycount;
	for (i = 0; i < dailycount; ++i) {
		dp = dailyplayers + i;
		if (dp->stats.kills < g_dailyMinimumKills.integer) {
			ranked = i;
			break;
		}
	}
}

void stats_UpdateDailyRankings()
{
	gclient_t *client, *client2;
	daily_stats_t *dailyplayer = NULL;
	daily_stats_t newplayers[MAX_CLIENTS];
	daily_stats_header_t header;
	fileHandle_t dailyfp;
	int i, j;
	byte newplayercount = 0;
	qboolean alreadyStored;
	
	trap_FS_FOpenFile(DAILY_PATH, &dailyfp, FS_WRITE);
	if (!dailyfp) {
		G_LogPrintf("Daily stats file could not be created.. Aborting.");
		return;
	}

	memset(newplayers, 0, sizeof(daily_stats_t) * MAX_CLIENTS);

	// Find duplicate ips
	for (i = 0; i < level.numPlayingClients; ++i) {
		client = level.clients + level.sortedClients[i];

		if (client->duplicateip) {
			continue;
		}

		for (j = 0; j < level.numPlayingClients; ++j) {
			client2 = level.clients + level.sortedClients[j];

			if (client == client2) {
				continue;
			}

			if (client->sess.ip == client2->sess.ip) {
				client->duplicateip = client2->duplicateip = qtrue;
				break;
			}
		}
	}

	// Update daily players
	for (i = 0; i < level.numPlayingClients; ++i) {
		client = level.clients + level.sortedClients[i];
		alreadyStored = qfalse;

		// Duplicates are completely discarded.. sorry guys.
		if (client->duplicateip) {
			//continue;
		}

		// Don't store bots.
		if (g_entities[level.sortedClients[i]].r.svFlags & SVF_BOT) {
			continue;
		}

		for (j = 0; j < dailycount; ++j) {
			dailyplayer = dailyplayers + j;

			if (dailyplayer->ip == client->sess.ip) {
				alreadyStored = qtrue;
				break;
			}
		}

		if (!alreadyStored) {
			if (dailycount + newplayercount == UCHAR_MAX)
				continue;
			dailyplayer = &newplayers[newplayercount++];
		}

		stats_UpdateDailyStatsForClient(client, dailyplayer);
	}

	// Write header
	header.id = DSID;
	header.version = DSVER;
	header.playercount = dailycount + newplayercount;
	header.reset_ctime = reset_ctime;
	trap_FS_Write(&header, sizeof(daily_stats_header_t), dailyfp);
	// Write new and updated daily players to file
	trap_FS_Write(newplayers, newplayercount * sizeof(daily_stats_t), dailyfp);
	trap_FS_Write(dailyplayers, dailycount * sizeof(daily_stats_t), dailyfp);

	trap_FS_FCloseFile(dailyfp);
}

daily_stats_t *stats_GetDailyStatsForClient(gclient_t *cl)
{
	daily_stats_t *player_stats = NULL;
	int i;

	for (i = 0; i < ranked; ++i) {
		daily_stats_t *ps = dailyplayers + i;
		if (ps->ip == cl->sess.ip) {
			player_stats = ps;
			break;
		}
	}

	return player_stats;
}

void stats_UpdateDailyStatsForClient(gclient_t *cl, daily_stats_t *player)
{
	int i, player_stats_fields_length;

	if (player == NULL) {
		player = stats_GetDailyStatsForClient(cl);
		if (player == NULL) {
			return;
		}
	}

	// if struct player_stats_t ever contains a stat that isn't of type unsigned short
	// then this hacky code will break horribly. beware!
	player_stats_fields_length = sizeof(player_stats_t) / sizeof(unsigned short);
	for (i = 0; i < player_stats_fields_length; ++i) {
		((unsigned short *)(&player->stats))[i] += ((unsigned short *)(&cl->pers.stats))[i];
	}

	player->ip = cl->sess.ip;
	Q_strncpyz(player->name, cl->pers.netname, sizeof(player->name));
}

void stats_DisplayDailyStats(gentity_t *ent)
{
	const int moreCount = 20;
	int i, count;
	char *statcolor;
	qboolean more = qfalse, playerfound = qfalse;

	if (!dailycount) {
		CP("print \"\nThere aren't any daily rankings yet.\nFinish a round and then try again.\n\n\"");
		return;
	}

	if (!ranked) {
		CP(va("print \"\nLooks like there aren't any ranked players!\n(minimum of %d kills required)\n\n\"", g_dailyMinimumKills.integer));
		return;
	}

	if (!ent->moreCalled) {
		ent->moreCalls = 0;
	}
	CP("print \"\n^3Rank | ^7Player Name          ^3| ^3K    D    G    TK   R    ^3| ^3Acc    HS   ^3| ^3DG    \n"
		"^3-----------------------------------------------------------------------------\n\"");

	for (count = 1, i = (ent->moreCalls * moreCount); i < ranked; i++, count++) {
		daily_stats_t *player = dailyplayers + i;
		statcolor = "^7";

		if (!playerfound && player->ip == ent->client->sess.ip)
		{
			statcolor = "^2";
			playerfound = qtrue;
		}

		float playeracc = ((player->stats.acc_shots == 0) ? 0.00f : ((float)player->stats.acc_hits / (float)player->stats.acc_shots) * 100.00f);

		CP(va("print \"^3%4d | ^7%s ^3| %s%-4d %-4d %-4d %-4d %-4d ^3| %s%-6.2f %-4d ^3| %s%-6d\n\"",
			(i + 1),
			TablePrintableColorName(player->name, 20),
			statcolor,
			player->stats.kills,
			player->stats.deaths,
			player->stats.gibs,
			player->stats.teamKills,
			player->stats.revives,
			statcolor,
			playeracc,
			player->stats.headshots,
			statcolor,
			player->stats.dmgGiven,
			player->stats.dmgReceived,
			player->stats.dmgTeam));

		if (count == moreCount && (i + 1) != ranked) {
			more = qtrue;
			break;
		}
	}

	CP("print \"^3-----------------------------------------------------------------------------\n\"");

	if (more) {
		int remaining = ranked - i - 1;
		CP(va("print \"Use ^3/more ^7to list the next %d players\n\n\"", remaining >= moreCount ? moreCount : remaining));
		ent->more = stats_DisplayDailyStats;
		return;
	}
	else {
		ent->more = 0;
		ent->moreCalls = 0;
	}

	if (reset_ctime > 0) {
		time_t current_ctime = time(NULL);
		int seconds_elapsed = reset_ctime - current_ctime;
		char *time_message = GetTimeMessage(seconds_elapsed);
		CP(va("print \"Stats reset in ^3%s\n\n\"", time_message));
	}
}

void stats_CheckDailyReset()
{
	if (level.intermissiontime) {
		level.dsCheckTime = level.time;
		return;
	}
	// Check every 5 seconds
	if (level.time - level.dsCheckTime <= 5000) {
		return;
	}
	// Invalid reset time, or hasn't been set. Return.
	if (reset_ctime <= 0) {
		return;
	}

	time_t current_ctime = time(NULL);
	// It's midnight (or about to be).. reset!
	if (current_ctime > reset_ctime) {
		reset_ctime = 0;
		dailycount = 0;
		// No fs remove available, so use this lil hack to do so.
		trap_FS_Rename(DAILY_PATH, "");

		AP("chat \"console: Daily stats have been reset!\n\"");
	}

	level.dsCheckTime = level.time;
}

void stats_GlobalStatsReconnect()
{
	trap_GlobalStatsReconnect();
}

void stats_SubmitGlobalStats()
{
	char *gt_string;
	int i;
	gentity_t *ent;
	clientPersistant_t *pers;

	// server_info\\\\\\player1_info\\\\player2_info\\\\etc.."
	// "rtcwPub\\27960\\dm\\\\\\Fox\\181.56.200.1\\325\\344\\450\\2000\\950\\10000\\\\Nobo\\12.2.100.33\\444\\222\\111\\351\\222\\10001"

	if (g_ffa.integer)
	{
		gt_string = "ffa";
	}
	else if (g_deathmatch.integer)
	{
		gt_string = "dm";
	}
	else if (g_gametype.integer == GT_WOLF_STOPWATCH)
	{
		gt_string = "sw";
	}
	else
	{
		gt_string = "obj";
	}

	char *stats = va("%s\\%i\\%s\\\\\\", GetHostname(), trap_Cvar_VariableIntegerValue("net_port"), gt_string);
	
	for (i = 0; i < level.numPlayingClients; i++) {
		ent = level.gentities + level.sortedClients[i];
		pers = &ent->client->pers;

		stats = va("%s%s\\%s\\%i\\%i\\%i\\%i\\%i\\%i\\\\",
			stats, pers->netname, clientIP(ent, qtrue), pers->stats.kills,
			pers->stats.deaths, pers->stats.headshots, pers->stats.acc_shots, pers->stats.acc_hits, level.time - pers->beganPlayingTime);
	}

	// Remove the last two back slashes (otherwise an empty player is picked up by stats server and fails)
	stats[strlen(stats) - 2] = 0;

	trap_SubmitGlobalStatsData(stats);
}
