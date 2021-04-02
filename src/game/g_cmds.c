#include "g_admin.h"

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) {
	char		entry[1024];
	char		string[1400];
	int			stringlength;
	int			i, j;
	gclient_t	*cl;
	int			numSorted;
	int			scoreFlags;
	int			secretlyDemoing;
	int			score;

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;
	scoreFlags = 0;
	secretlyDemoing = 0;

	// don't send more than 32 scores (FIXME?)
	numSorted = level.numNonDisconnectedClients;
	if ( numSorted > 32 ) {
		numSorted = 32;
	}

	for (i=0 ; i < numSorted ; i++) {
		int		ping;
		int		playerClass;
		int		respawnsLeft;

		cl = &level.clients[level.sortedClients[i]];

		if (cl->sess.secretlyDemoing && ent->client != cl && ent->client->sess.admin == ADM_NONE) {
			secretlyDemoing++;
			continue;
		}

		// NERVE - SMF - if on same team, send across player class
		if ( cl->ps.persistant[PERS_TEAM] == ent->client->ps.persistant[PERS_TEAM] )
			playerClass = cl->ps.stats[STAT_PLAYER_CLASS];
		else
			playerClass = 0;

		// NERVE - SMF - number of respawns left
		respawnsLeft = cl->ps.persistant[PERS_RESPAWNS_LEFT];
		if ( respawnsLeft == 0 && ((cl->ps.pm_flags & PMF_LIMBO) || (level.intermissiontime && g_entities[level.sortedClients[i]].health <= 0)) )
			respawnsLeft = -2;

		if ( cl->pers.connected == CON_CONNECTING ) {
			ping = -1;
		} else {
			if (g_alternatePing.integer) {
				ping = cl->pers.alternatePing < 999 ? cl->pers.alternatePing : 999;
			} else {
				ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
			}
		}

		score = g_ffa.integer ? cl->pers.kills : cl->ps.persistant[PERS_SCORE];

		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i", level.sortedClients[i],
			score, ping, (level.time - cl->pers.enterTime)/60000,
			scoreFlags, g_entities[level.sortedClients[i]].s.powerups, playerClass, respawnsLeft );
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	trap_SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i - secretlyDemoing, 
		level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE], string ) );
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
	DeathmatchScoreboardMessage( ent );
}



/*
==================
CheatsOk
==================
*/
qboolean	CheatsOk( gentity_t *ent ) {
	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
		return qfalse;
	}
	if ( ent->health <= 0 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"You must be alive to use this command.\n\""));
		return qfalse;
	}
	return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
ConcatArgs2

Used for concatinating arguments if ConcatArgs() "breaks".
==================
*/
char *ConcatArgs2(int start) {
	char *a = ConcatArgs(0);
	char *s = a;
	int slen;

	while (start-- && (s = strchr(s, ' ')) != NULL)
		s++;
	if (!s) s = "";
	slen = strlen(s);
	memcpy(a, s, slen);

	a[slen] = 0;

	return a;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString(const char *in, char *out, qboolean fToLower)
{
	while (*in) {
		if (*in == 27 || *in == '^') {
			in++;		// skip color code
			if (*in) in++;
			continue;
		}

		if (*in < 32) {
			in++;
			continue;
		}

		*out++ = (fToLower) ? tolower(*in++) : *in++;
	}

	*out = 0;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *ent, char *s ) {
	gclient_t	*cl;
	int			idnum;
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];

	// numeric values are just slot numbers
	if (s[0] >= '0' && s[0] <= '9') {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			CP(va("print \"Bad client slot: %i\n\"", idnum));
			return -1;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected != CON_CONNECTED ) {
			CP(va("print \"Client %i is not active\n\"", idnum));
			return -1;
		}
		return idnum;
	}

	// check for a name match
	SanitizeString(s, s2, qtrue);
	for (idnum = 0, cl = level.clients; idnum < level.maxclients; idnum++, cl++) {
		if (cl->pers.connected != CON_CONNECTED) {
			continue;
		}
		SanitizeString(cl->pers.netname, n2, qtrue);
		if (!strcmp(n2, s2)) {
			return idnum;
		}
	}

	CP(va("print \"^3%s ^7was unable to find a client\n\"", s));
	return -1;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (gentity_t *ent)
{
	char		*name, *amt;
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	gentity_t		*it_ent;
	trace_t		trace;
	int			amount;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	//----(SA)	check for an amount (like "give health 30")
	amt = ConcatArgs(2);
	amount = atoi(amt);
	//----(SA)	end

	name = ConcatArgs( 1 );

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;


	if (give_all || Q_stricmpn( name, "health", 6) == 0)
	{
		//----(SA)	modified
		if(amount)
			ent->health += amount;
		else
			ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for(i=0;i<WP_NUM_WEAPONS;i++) {
			if ( BG_WeaponInWolfMP(i) )
				COM_BitSet( ent->client->ps.weapons, i );
		}

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "holdable") == 0)
	{
		ent->client->ps.stats[STAT_HOLDABLE_ITEM] = (1 << (HI_BOOK3 - 1)) - 1 - (1<<HI_NONE);
		for ( i = 1 ; i <= HI_BOOK3 ; i++ ) {
			ent->client->ps.holdable[i] = 10;
		}

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmpn(name, "ammo", 4) == 0)
	{
		if(amount) {
			if(ent->client->ps.weapon)
				Add_Ammo(ent, ent->client->ps.weapon, amount, qtrue);
		} else {
			for ( i = 1 ; i < WP_MONSTER_ATTACK1 ; i++ )
				Add_Ammo(ent, i, 9999, qtrue);
		}

		if (!give_all)
			return;
	}

	//	"give allammo <n>" allows you to give a specific amount of ammo to /all/ weapons while
	//	allowing "give ammo <n>" to only give to the selected weap.
	if (Q_stricmpn(name, "allammo", 7) == 0 && amount)
	{
		for ( i = 1 ; i < WP_MONSTER_ATTACK1 ; i++ )
			Add_Ammo(ent, i, amount, qtrue);

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmpn(name, "armor", 5) == 0)
	{
		if ( g_gametype.integer == GT_SINGLE_PLAYER ) { // JPW NERVE -- no armor in multiplayer
		//----(SA)	modified
		if(amount)
			ent->client->ps.stats[STAT_ARMOR] += amount;
		else
			ent->client->ps.stats[STAT_ARMOR] = 200;
		} // jpw
		if (!give_all)
			return;
	}

	//---- (SA) Wolf keys
	if (give_all || Q_stricmp(name, "keys") == 0)
	{
		ent->client->ps.stats[STAT_KEYS] = (1 << KEY_NUM_KEYS) - 2;
		if (!give_all)
			return;
	}
	//---- (SA) end

	// spawn a specific item right on the player
	if ( !give_all ) {
		it = BG_FindItem (name);
		if (!it) {
			return;
		}

		it_ent = G_Spawn();
		VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem (it_ent, it);
		FinishSpawningItem(it_ent );
		memset( &trace, 0, sizeof( trace ) );
		it_ent->active = qtrue;
		Touch_Item (it_ent, ent, &trace);
		it_ent->active = qfalse;
		if (it_ent->inuse) {
			G_FreeEntity( it_ent );
		}
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}

/*
==================
Cmd_Nofatigue_f

Sets client to nofatigue

argv(0) nofatigue
==================
*/

void Cmd_Nofatigue_f (gentity_t *ent)
{
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_NOFATIGUE;
	if (!(ent->flags & FL_NOFATIGUE) )
		msg = "nofatigue OFF\n";
	else
		msg = "nofatigue ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}

/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	if ( ent->client->noclip ) {
		msg = "noclip OFF\n";
	} else {
		msg = "noclip ON\n";
	}
	ent->client->noclip = !ent->client->noclip;

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( gentity_t *ent ) {
	if ( !CheatsOk( ent ) ) {
		return;
	}

	// doesn't work in single player
	if ( g_gametype.integer != 0 ) {
		trap_SendServerCommand( ent-g_entities, 
			"print \"Must be in g_gametype 0 for levelshot\n\"" );
		return;
	}

	BeginIntermission();
	trap_SendServerCommand( ent-g_entities, "clientLevelShot" );
}


/*
=================
Cmd_Gib_f
=================
*/
void Cmd_Gib_f( gentity_t *ent ) {
	gentity_t *attacker;
	int contents;

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}
	if ( g_gametype.integer >= GT_WOLF && ent->client->ps.pm_flags & PMF_LIMBO ) {
		return;
	}
	if (g_ffa.integer) {
		return;
	}
	if (level.paused) {
		return;
	}
	if (!ent->takedamage) {
		return;
	}

	// L0 - Chicken
	attacker = G_FearCheck(ent);
	if (attacker) {
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		ent->client->ps.persistant[PERS_HWEAPON_USE] = 0;

		APRS(ent, "sound/player/comeback.wav");
		player_die(ent, attacker, attacker, (ent->health + 100000), MOD_CHICKEN);

		// Stats
		ent->client->pers.suicides++;
		stats_StoreRoundStat(ent->client->pers.netname, ent->client->pers.suicides, ROUND_SUICIDES);

		return;
	}

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
	ent->client->ps.persistant[PERS_HWEAPON_USE] = 0; // TTimo - if using /kill while at MG42

	contents = trap_PointContents(ent->r.currentOrigin, -1);
	if (!(contents & CONTENTS_NODROP)) {
		TossClientItems(ent);
	}

	if (ent->client->ps.pm_type != PM_DEAD) {
		ent->client->pers.suicides++;
		stats_StoreRoundStat(ent->client->pers.netname, ent->client->pers.suicides, ROUND_SUICIDES);
	}

	G_Damage(ent, ent, ent, NULL, NULL, 10000, DAMAGE_NO_PROTECTION, MOD_SELFKILL);
}

/*
=================
L0 - Cmd_Kill_f
=================
*/
void Cmd_Kill_f(gentity_t *ent) {
	gentity_t *attacker;

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
		return;
	}
	if (g_gametype.integer >= GT_WOLF && ent->client->ps.pm_flags & PMF_LIMBO) {
		return;
	}
	if (ent->client->ps.pm_type == PM_DEAD) {
		return;
	}
	if (g_ffa.integer) {
		return;
	}
	if (level.paused) {
		return;
	}

	// L0 - Chicken
	attacker = G_FearCheck(ent);
	if (attacker) {
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		ent->client->ps.persistant[PERS_HWEAPON_USE] = 0;

		APRS(ent, "sound/player/comeback.wav");
		player_die(ent, attacker, attacker, ent->health, MOD_CHICKEN);

		// Stats
		ent->client->pers.suicides++;
		stats_StoreRoundStat(ent->client->pers.netname, ent->client->pers.suicides, ROUND_SUICIDES);
		return;
	}

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
	ent->client->ps.persistant[PERS_HWEAPON_USE] = 0; // TTimo - if using /kill while at MG42
	player_die(ent, ent, ent, 100000, MOD_SELFKILL);   // L0 - Kill but not gib...

	// L0 - Stats
	ent->client->pers.suicides++;
	stats_StoreRoundStat(ent->client->pers.netname, ent->client->pers.suicides, ROUND_SUICIDES);
}

/*
=================
SetTeam
=================
*/
void SetTeam(gentity_t *ent, char *s, qboolean forced) {
	int					team, oldTeam;
	gclient_t			*client;
	int					clientNum;
	spectatorState_t	specState;
	int					specClient;
	int					i;
	char				userinfo[MAX_INFO_STRING];

	if (!forced && level.paused)
	{
		CP("cp \"You cannot join a team while the match is paused.\n\"");
		return;
	}

	//
	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;

	specState = SPECTATOR_NOT;
	if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_SCOREBOARD;
	} else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	} else if ( g_gametype.integer >= GT_TEAM ) {
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;

			if (g_ffa.integer) {
				team = TEAM_BLUE;
			}
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
		} else {
			// pick the team with the least number of players
			team = PickTeam( clientNum );
		}

		// NERVE - SMF
		if (!forced && g_noTeamSwitching.integer && team != ent->client->sess.sessionTeam && g_gamestate.integer == GS_PLAYING) {
			trap_SendServerCommand( clientNum, "cp \"You cannot switch during a match, please wait until the round ends.\n\"" );
			return;	// ignore the request
		}

		// L0 - Team Lock		
		handleTeamLocks(team);
		if (!forced && teamInfo[team].team_lock == qtrue)
		{
			CP(va("cp \"%s team is locked!\n\"", (team == TEAM_RED ? "^1Axis^7" : "^4Allied^7")));
			return;
		}

		// NERVE - SMF - merge from team arena
		if (!forced && g_teamForceBalance.integer) {
			int		counts[TEAM_NUM_TEAMS];

			counts[TEAM_BLUE] = TeamCount( ent-g_entities, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( ent-g_entities, TEAM_RED );

			// We allow a spread of one
			if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] >= 1 ) {
				trap_SendServerCommand( clientNum, 
					"cp \"The ^1Axis ^7team has too many players.\n\"" );
				return; // ignore the request
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] >= 1 ) {
				trap_SendServerCommand( clientNum, 
					"cp \"The ^4Allies ^7team have too many players.\n\"" );
				return; // ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}
		// -NERVE - SMF
		if (client->sess.secretlyDemoing) {
			CP("cp \"You can't join a team because you are secretly demoing.\n\"");
			return;
		}
	} else {
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	// override decision if limiting the players
	if ( g_gametype.integer == GT_TOURNAMENT
		&& level.numPlayingClients >= 2 ) {
		team = TEAM_SPECTATOR;
	} else if ( g_maxGameClients.integer > 0 && 
		level.numPlayingClients >= g_maxGameClients.integer ) {
		team = TEAM_SPECTATOR;
	}

	//
	// decide if we will allow the change
	//
	oldTeam = client->sess.sessionTeam;
	if ( team == oldTeam && team != TEAM_SPECTATOR ) {
		return;
	}

	// NERVE - SMF - prevent players from switching to regain deployments
	if (!forced && g_maxlives.integer > 0 && ent->client->ps.persistant[PERS_RESPAWNS_LEFT] == 0 &&
		oldTeam != TEAM_SPECTATOR && team != TEAM_SPECTATOR ) {
		trap_SendServerCommand( clientNum, 
			"cp \"You can't switch teams because you are out of lives.\n\" 3" );
		return;	// ignore the request
	}
		
	// Force players to wait g_teamSwitchTime seconds before they can join a new team.
	if (!forced &&
		g_gametype.integer >= GT_WOLF &&
		team != oldTeam && !client->pers.initialSpawn &&
		level.time - client->pers.connectTime > 10000 &&
		level.time - client->pers.enterTime < g_teamSwitchTime.integer * 1000)
	{
		int time_remaining = (int)(g_teamSwitchTime.integer - ((level.time - client->pers.enterTime) / 1000));

		CP(va( "cp \"You must wait ^3%i ^7%s before joining a new team.\n\"3", time_remaining, time_remaining == 1 ? "second" : "seconds"));
		return;
	}
  
	//
	// execute the team change
	//

	// DHM - Nerve
	if ( client->pers.initialSpawn && team != TEAM_SPECTATOR )
		client->pers.initialSpawn = qfalse;

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
	if ( oldTeam != TEAM_SPECTATOR ) {
		if ( !(ent->client->ps.pm_flags & PMF_LIMBO) ) {
			// Kill him (makes sure he loses flags, etc)
			ent->flags &= ~FL_GODMODE;
			ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
			player_die (ent, ent, ent, 100000, MOD_SUICIDE);
		}
		// Nobo - Fix for "skull -> full lives" bug
		for (i = 0; i < level.maxclients; i++) {
			if ((level.clients[i].ps.pm_flags & PMF_LIMBO)
				&& level.clients[i].sess.spectatorClient == ent->s.number) {
				Cmd_FollowCycle_f(&g_entities[i], 1);
			}
		}
	}
	// they go to the end of the line for tournements
	if ( team == TEAM_SPECTATOR ) {
		client->sess.spectatorTime = level.time;
	}

	CheckWeaponRestrictions(client);

	if (g_automg42.integer)
	{
		G_Automg42Disassociate(ent);
	}

	client->sess.sessionTeam = team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	if (!forced) {
		if (team == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR) {
			AP(va("chat \"console: %s ^7joined the ^3spectators^7.\n\"", client->pers.netname));
		}
		else if (team == TEAM_FREE || g_ffa.integer) {
			AP(va("chat \"console: %s ^7joined the ^1battle^7.\n\"", client->pers.netname));
		}
		else if (team == TEAM_RED) {	
			AP(va("chat \"console: %s ^7joined the ^1Axis ^7team.\n\"", client->pers.netname));
		}
		else if (team == TEAM_BLUE) {
			AP(va("chat \"console: %s ^7joined the ^4Allies ^7team.\n\"", client->pers.netname));
		}
	}

	// L0 - Notify them it's DM
	if (g_deathmatch.integer && !(oldTeam == TEAM_SPECTATOR && oldTeam == team)) {
		if (g_ffa.integer) {
			CP("chat \"console: This server is running in FFA mode!\n\"");
		}
		else {
			CP("chat \"console: This server is running in deathmatch mode!\n\"");
		}
	}

	stats_UpdateDailyStatsForClient(ent->client, NULL);

	if (ent->r.svFlags & SVF_BOT) {
		trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));
		Info_SetValueForKey(userinfo, "team", TeamName(client->sess.sessionTeam));
		trap_SetUserinfo(clientNum, userinfo);
	}

	// get and distribute relevent paramters
	ClientUserinfoChanged( clientNum );

	ClientBegin( clientNum );

	// L0 - sync teams
	if (!forced && g_teamAutoBalance.integer)
		checkEvenTeams();
}

// DHM - Nerve
/*
=================
SetWolfData
=================
*/
void SetWolfData( gentity_t *ent, char *ptype, char *weap, char *grenade, char *skinnum ) {	// DHM - Nerve
	gclient_t	*client;

	client = ent->client;

	client->sess.latchPlayerType = atoi( ptype );
	client->sess.latchPlayerWeapon = atoi( weap );
	client->sess.latchPlayerItem = atoi( grenade );
	client->sess.latchPlayerSkin = atoi( skinnum );
}
// dhm - end

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing( gentity_t *ent ) {
	if ( g_gametype.integer < GT_WOLF )	{		// NERVE - SMF - don't forcibly set this for multiplayer
		ent->client->sess.sessionTeam = TEAM_SPECTATOR;	
		ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;
	}

	// ATVI Wolfenstein Misc #474
	// divert behaviour if TEAM_SPECTATOR, moved the code from SpectatorThink to put back into free fly correctly
	// (I am not sure this can be called in non-TEAM_SPECTATOR situation, better be safe)
	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		// drop to free floating, somewhere above the current position (that's the client you were following)
		vec3_t pos, angle;
		int enterTime;
		gclient_t   *client = ent->client;
		VectorCopy(client->ps.origin, pos); pos[2] += 16;
		VectorCopy(client->ps.viewangles, angle);
		// ATVI Wolfenstein Misc #414, backup enterTime
		enterTime = client->pers.enterTime;
		SetTeam(ent, "spectator", qfalse);
		client->pers.enterTime = enterTime;
		VectorCopy(pos, client->ps.origin);
		SetClientViewAngle(ent, angle);
	}
	else
	{
		// legacy code, FIXME: useless?
		ent->client->sess.spectatorState = SPECTATOR_FREE;
		ent->r.svFlags &= ~SVF_BOT;
		ent->client->ps.clientNum = ent - g_entities;
	}
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) {
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];
	char		ptype[4], weap[4], pistol[4], grenade[4], skinnum[4];

	if ( trap_Argc() < 2 ) {
		oldTeam = ent->client->sess.sessionTeam;
		switch ( oldTeam ) {
		case TEAM_BLUE:
			trap_SendServerCommand( ent-g_entities, "print \"Blue team\n\"" );
			break;
		case TEAM_RED:
			trap_SendServerCommand( ent-g_entities, "print \"Red team\n\"" );
			break;
		case TEAM_FREE:
			trap_SendServerCommand( ent-g_entities, "print \"Free team\n\"" );
			break;
		case TEAM_SPECTATOR:
			trap_SendServerCommand( ent-g_entities, "print \"Spectator team\n\"" );
			break;
		}
		return;
	}

	// if they are playing a tournement game, count as a loss
	if ( g_gametype.integer == GT_TOURNAMENT && ent->client->sess.sessionTeam == TEAM_FREE ) {
		ent->client->sess.losses++;
	}

	// DHM - Nerve
	if ( g_gametype.integer >= GT_WOLF ) {
		trap_Argv( 2, ptype, sizeof( ptype ) );
		trap_Argv( 3, weap, sizeof( weap ) );
		trap_Argv( 4, pistol, sizeof( pistol ) );
		trap_Argv( 5, grenade, sizeof( grenade ) );
		trap_Argv( 6, skinnum, sizeof( skinnum ) );

		SetWolfData( ent, ptype, weap, grenade, skinnum );
	}
	// dhm - end

	trap_Argv( 1, s, sizeof( s ) );

	SetTeam( ent, s, qfalse );
}

/*
=================
FollowClient
=================
*/
void FollowClient(gentity_t *ent, qboolean byName)
{
	int		targetClientNumber;
	char	arg[MAX_TOKEN_CHARS];
	gclient_t *target;

	if (trap_Argc() != 2) {
		if (ent->client->sess.spectatorState == SPECTATOR_FOLLOW) {
			StopFollowing(ent);
		}
		return;
	}

	trap_Argv(1, arg, sizeof(arg));
	if (byName) {
		targetClientNumber = ClientNumberFromName(ent, arg, qtrue);
	}
	else {
		targetClientNumber = ClientNumberFromString(ent, arg);
	}

	if (targetClientNumber < 0) {
		return;
	}

	target = level.clients + targetClientNumber;

	// can't follow self
	if (target == ent->client) {
		CP("cp \"You can't follow yourself.\n\"");
		return;
	}

	// can't follow another spectator
	if (target->sess.sessionTeam == TEAM_SPECTATOR) {
		CP(va("cp \"%s ^7is a spectator and can't be followed.\n\"", target->pers.netname));
		return;
	}

	if (g_gametype.integer >= GT_WOLF) {
		if (target->ps.pm_flags & PMF_LIMBO) {
			CP(va("cp \"%s ^7is dead and can't be followed.\n\"", target->pers.netname));
			return;
		}
	}

	// if they are playing a tournement game, count as a loss
	if (g_gametype.integer == GT_TOURNAMENT && ent->client->sess.sessionTeam == TEAM_FREE) {
		ent->client->sess.losses++;
	}

	// first set them to spectator
	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
		SetTeam(ent, "spectator", qfalse);
	}

	// only start spectating if they are spec or were sucessfully sent to specs
	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		ent->client->sess.spectatorClient = targetClientNumber;
	}
}

/*
=================
Cmd_FollowClient_f
=================
*/
void Cmd_FollowClient_f( gentity_t *ent ) {
	FollowClient(ent, qfalse);
}

/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f(gentity_t *ent) {
	FollowClient(ent, qtrue);
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir ) {
	int		clientnum;
	int		original;

	// if they are playing a tournement game, count as a loss
	if ( g_gametype.integer == GT_TOURNAMENT && ent->client->sess.sessionTeam == TEAM_FREE ) {
		ent->client->sess.losses++;
	}
	// first set them to spectator
	if (( ent->client->sess.spectatorState == SPECTATOR_NOT ) && (!( ent->client->ps.pm_flags & PMF_LIMBO)) ) { // JPW NERVE for limbo state
		SetTeam( ent, "spectator", qfalse );
	}

	if ( dir != 1 && dir != -1 ) {
		G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
	}

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;
	do {
		clientnum += dir;
		if ( clientnum >= level.maxclients ) {
			clientnum = 0;
		}
		if ( clientnum < 0 ) {
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
			continue;
		}

		// can't follow another spectator
		if ( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

// JPW NERVE -- couple extra checks for limbo mode
		if (ent->client->ps.pm_flags & PMF_LIMBO) {
			if (level.clients[clientnum].ps.pm_flags & PMF_LIMBO)
				continue;
			if (level.clients[clientnum].sess.sessionTeam != ent->client->sess.sessionTeam &&
				ent->client->sess.sessionTeam != TEAM_SPECTATOR)
				continue;
		}
// jpw

		if ( g_gametype.integer >= GT_WOLF ) {
			if (level.clients[clientnum].ps.pm_flags & PMF_LIMBO)
				continue;
		}

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	} while ( clientnum != original );

	// leave it where it was
}

void Cmd_FollowNext_f(gentity_t *ent)
{
	Cmd_FollowCycle_f(ent, 1);
}

void Cmd_FollowPrev_f(gentity_t *ent)
{
	Cmd_FollowCycle_f(ent, -1);
}

/*
==================
L0

Shortcuts from EtPUB
==================
*/
char *G_ShortcutSanitize(char *text)
{
	// pheno: increased 'n' for [command] command line
	static char n[MAX_STRING_CHARS] = { "" };

	if (!text || !*text)
		return n;

	Q_strncpyz(n, text, sizeof(n));

	Q_strncpyz(n, Q_StrReplace(n, "[a]", "(a)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[d]", "(d)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[h]", "(h)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[k]", "(k)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[l]", "(l)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[n]", "(n)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[r]", "(r)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[p]", "(p)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[s]", "(s)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[w]", "(w)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[t]", "(t)"), sizeof(n));
	return n;
}

char *G_Shortcuts(gentity_t *ent, char *text)
{
	// pheno: increased 'out' for [command] command line
	static char out[MAX_STRING_CHARS];
	char a[MAX_NETNAME] = { "*unknown*" };
	char d[MAX_NETNAME] = { "*unknown*" };
	char h[MAX_NETNAME] = { "*unknown*" };
	char k[MAX_NETNAME] = { "*unknown*" };
	char l[MAX_NETNAME] = { "*unknown*" };
	char n[MAX_NETNAME] = { "*unknown*" };
	char r[MAX_NETNAME] = { "*unknown*" };
	char p[MAX_NETNAME] = { "*unknown*" };
	char s[MAX_NETNAME] = { "*unknown*" };
	char w[MAX_NETNAME] = { "*unknown*" };
	char t[MAX_NETNAME] = { "*unknown*" };
	gclient_t *client = NULL;
	gentity_t *crosshairEnt;
	gitem_t *weapon;
	int clip;
	int ammo;

	out[0] = '\0';

	if (!text || !*text)
		return out;

	if (ent) {
		if (ent->client->pers.lastammo_client != -1) {
			client = &level.clients[ent->client->pers.lastammo_client];
			if (client) {
				Q_strncpyz(a,
					G_ShortcutSanitize(client->pers.netname),
					sizeof(a));
			}
		}
	}

	if (ent) {
		if (ent->client->pers.lastkiller_client != -1) {
			client = &level.clients[ent->client->pers.lastkiller_client];
			if (client) {
				Q_strncpyz(d,
					G_ShortcutSanitize(client->pers.netname),
					sizeof(d));
			}
		}
	}

	if (ent) {
		if (ent->client->pers.lasthealth_client != -1) {
			client = &level.clients[ent->client->pers.lasthealth_client];
			if (client) {
				Q_strncpyz(h,
					G_ShortcutSanitize(client->pers.netname),
					sizeof(h));
			}
		}
	}

	if (ent) {
		if (ent->client->pers.lastkilled_client != -1) {
			client = &level.clients[ent->client->pers.lastkilled_client];
			if (client) {
				Q_strncpyz(k,
					G_ShortcutSanitize(client->pers.netname),
					sizeof(k));
			}
		}
	}

	if (ent) {
		char location[64];

		Team_GetLocationMsg(ent, location, sizeof(location), qtrue);
		Q_strncpyz(l,
			location,
			sizeof(l));
	}

	if (ent) {
		Q_strncpyz(n,
			G_ShortcutSanitize(ent->client->pers.netname),
			sizeof(n));
	}

	if (ent) {
		if (ent->client->pers.lastrevive_client != -1) {
			client = &level.clients[ent->client->pers.lastrevive_client];
			if (client) {
				Q_strncpyz(r,
					G_ShortcutSanitize(client->pers.netname),
					sizeof(r));
			}
		}
	}

	if (ent) {
		crosshairEnt = &g_entities[ent->client->ps.identifyClient];
		// Dens: only give the name of the other client, if the player should be able to see it
		if (crosshairEnt && crosshairEnt->client && crosshairEnt->inuse &&
			(ent->client->sess.sessionTeam == crosshairEnt->client->sess.sessionTeam ||
			ent->client->sess.sessionTeam == TEAM_SPECTATOR)) {
			client = crosshairEnt->client;
			if (client) {
				Q_strncpyz(p, G_ShortcutSanitize(client->pers.netname), sizeof(p));
			}
		}
	}

	if (ent) {
		int health;
		health = ent->health;
		if (health < 0) {
			health = 0;
		}
		Com_sprintf(s, sizeof(s), "%i", health);
	}

	if (ent && ent->client->ps.weapon) {
		weapon = BG_FindItemForWeapon(ent->client->ps.weapon);
		Q_strncpyz(w, weapon->pickup_name, sizeof(w));
	}

	if (ent && ent->client->ps.weapon) {
		clip = BG_FindClipForWeapon(ent->client->ps.weapon);
		ammo = BG_FindAmmoForWeapon(ent->client->ps.weapon);
		Com_sprintf(t, sizeof(t), "%i",
			(ent->client->ps.ammoclip[clip] +
			((ent->client->ps.weapon == WP_KNIFE) ? 0 : ent->client->ps.ammo[ammo])));
	}

	Q_strncpyz(out, text, sizeof(out));
	Q_strncpyz(out, Q_StrReplace(out, "[a]", a), sizeof(out));
	Q_strncpyz(out, Q_StrReplace(out, "[d]", d), sizeof(out));
	Q_strncpyz(out, Q_StrReplace(out, "[h]", h), sizeof(out));
	Q_strncpyz(out, Q_StrReplace(out, "[k]", k), sizeof(out));
	Q_strncpyz(out, Q_StrReplace(out, "[l]", l), sizeof(out));
	Q_strncpyz(out, Q_StrReplace(out, "[n]", n), sizeof(out));
	Q_strncpyz(out, Q_StrReplace(out, "[r]", r), sizeof(out));
	Q_strncpyz(out, Q_StrReplace(out, "[p]", p), sizeof(out));
	Q_strncpyz(out, Q_StrReplace(out, "[s]", s), sizeof(out));
	Q_strncpyz(out, Q_StrReplace(out, "[w]", w), sizeof(out));
	Q_strncpyz(out, Q_StrReplace(out, "[t]", t), sizeof(out));

	return out;
}

/*
==================
L0 - G_SayToTeam

Prints to selected team only.
==================
*/
void G_SayToTeam(int team, char *type, char *msg) {
	int j;
	gentity_t   *other;

	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];

		if (other->client->sess.sessionTeam == team)
			trap_SendServerCommand(other - g_entities, va("%s \"%s\n\"", type, msg));
	}
}

#define	MAX_SAY_TEXT	150
#define SAY_ALL		0
#define SAY_TEAM	1
#define SAY_TELL	2
#define SAY_LIMBO	3			// NERVE - SMF
#define SAY_ADMIN	4			// L0 - Admin chat

/*
==================
G_SayTo
==================
*/
void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message, qboolean localize ) { // removed static so it would link
	if (!other) {
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if ( mode == SAY_TEAM && !OnSameTeam(ent, other) ) {
		return;
	}

	if (COM_BitCheck(other->client->sess.muted, ent - g_entities)) {
		return;
	}

	// no chatting to players in tournements
	if ( g_gametype.integer == GT_TOURNAMENT
		&& other->client->sess.sessionTeam == TEAM_FREE
		&& ent->client->sess.sessionTeam != TEAM_FREE 
		// L0 - Ignore only if ignoreSpecs is on and user is not logged in..
		&& !ent->client->sess.admin
		&& g_ignoreSpecs.integer
		// End
	) {
		return;
	}

	// NERVE - SMF - if spectator, no chatting to players in WolfMP
	if (g_gametype.integer >= GT_WOLF
		&& ((ent->client->sess.sessionTeam == TEAM_FREE && other->client->sess.sessionTeam != TEAM_FREE) ||
		(ent->client->sess.sessionTeam == TEAM_SPECTATOR && other->client->sess.sessionTeam != TEAM_SPECTATOR))
		// L0 - Ignore only if ignoreSpecs is on and user is not logged in..
		&& !ent->client->sess.admin
		&& g_ignoreSpecs.integer
		// End
		) {
		return;
	}

	// L0 - Admin chat is visible only to admins..
	if (mode == SAY_ADMIN) {
		if (!ent->client->sess.admin || !other->client->sess.admin)
			return;
	}

	// NERVE - SMF
	if ( mode == SAY_LIMBO ) {
		trap_SendServerCommand( other-g_entities, va("%s \"%s%c%c%s\"", 
			"lchat", name, Q_COLOR_ESCAPE, color, message));
	}
	// -NERVE - SMF
	else {
		trap_SendServerCommand( other-g_entities, va("%s \"%s%c%c%s\" %i", 
			mode == SAY_TEAM ? "tchat" : "chat",
			name, Q_COLOR_ESCAPE, color, message, localize));
	}
}

void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
	int			j;
	gentity_t	*other;
	int			color;
	char		name[64];
	// don't let text be too long for malicious reasons
	char		text[MAX_SAY_TEXT];
	char		location[64];
	qboolean	localize = qfalse;
// L0  
	char *tag = "";
	char censoredText[MAX_SAY_TEXT];
	char *shortcuts;

	// Clean up the chatText
	Q_SanitizeClientTextInput(chatText, text, sizeof(text), qfalse);
	
	if (ent->client->sess.admin > ADM_NONE) {
		if (do_cmds(ent, text)) {
			return;
		}
	}

	// Admin tag
	if (!ent->client->sess.incognito && ent->client->sess.admin > ADM_NONE) {
		tag = va("^7(%s^7)", getTag(ent));
	}

	// Ignored
	if (!(mode == SAY_ADMIN && ent->client->sess.secretlyDemoing) && ent->client->sess.ignored > IGNORE_OFF) {
		CP(va("cp \"You are %s ignored^1!\n\"2",
			ent->client->sess.ignored == IGNORE_PERMANENT ? "permanently" : "temporarily"));		
		return;
	}

	// L0 - Censored text		
	if (g_censorWords.string[0]) {
		SanitizeString(text, censoredText, qtrue);
		if (G_CensorText(censoredText, &censorDictionary)) {
			Q_strncpyz(text, censoredText, sizeof(text));
			SB_chatWarn(ent);
		}
	}

	// Shortcuts
	if (g_shortcuts.integer) {
		shortcuts = G_Shortcuts(ent, text);
		Q_strncpyz(text, shortcuts, sizeof(text));
	}
// L0 - End

	if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
		mode = SAY_ALL;
	}

	switch ( mode ) {
	default:
	case SAY_ALL:
		G_LogPrintf( "say: %s%s: %s\n", ent->client->pers.netname, tag, text);
		Com_sprintf(name, sizeof(name), "%s%s%c%c: ", ent->client->pers.netname, tag, Q_COLOR_ESCAPE, COLOR_WHITE);
		color = COLOR_GREEN;
		break;
	case SAY_TEAM:
		localize = qtrue;
		G_LogPrintf( "say_team: %s: %s\n", ent->client->pers.netname, text);
		if (Team_GetLocationMsg(ent, location, sizeof(location), qtrue))
			Com_sprintf (name, sizeof(name), "(%s%c%c) (%s): ", 
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location);
		else
			Com_sprintf (name, sizeof(name), "(%s%c%c): ", 
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_CYAN;
		break;
	case SAY_TELL:
		// G_LogPrintf is called in Cmd_Tell_f
		if (target && g_gametype.integer >= GT_TEAM &&
			target->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
			Team_GetLocationMsg(ent, location, sizeof(location), qtrue))
			Com_sprintf (name, sizeof(name), "(%s%c%c) (%s): ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location );
		else
			Com_sprintf (name, sizeof(name), "(%s%c%c): ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_MAGENTA;
		break;
	// NERVE - SMF
	case SAY_LIMBO:
		G_LogPrintf( "say_limbo: %s: %s\n", ent->client->pers.netname, text);
		Com_sprintf (name, sizeof(name), "%s%c%c: ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		break;
	// -NERVE - SMF
	// L0 - Admin chat
	case SAY_ADMIN:
		if (ent->client->sess.admin == ADM_NONE) {
			CP("print \"^1Error: ^7Private chat is only for Admins, login if you wish to participate^1!\n\"");
			return;
		}
		G_LogPrintf("say_admin: %s: %s\n", ent->client->pers.netname, text);
		Com_sprintf(name, sizeof(name), "^3Admin Channel(^7%s^3): ", ent->client->pers.netname);
		color = COLOR_WHITE;
		break;
	//  end	
	}

	if ( target ) {
		G_SayTo( ent, target, mode, color, name, text, localize );
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) {
		G_Printf( "%s%s\n", name, text);
	}

	// send it to all the apropriate clients
	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		G_SayTo( ent, other, mode, color, name, text, localize );
	}
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
	char		*p;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	G_Say( ent, NULL, mode, p );
}

static void Cmd_SayAll_f(gentity_t *ent) {
	Cmd_Say_f(ent, SAY_ALL, qfalse);
}

static void Cmd_SayTeam_f(gentity_t *ent) {
	if (g_ffa.integer) {
		Cmd_SayAll_f(ent);
		return;
	}

	Cmd_Say_f(ent, SAY_TEAM, qfalse);
}

static void Cmd_SayLimbo_f(gentity_t *ent) {
	Cmd_Say_f(ent, SAY_LIMBO, qfalse);
}

static void Cmd_SayAdmin_f(gentity_t *ent) {
	Cmd_Say_f(ent, SAY_ADMIN, qfalse);
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	char		*p;
	char		arg[MAX_TOKEN_CHARS];

	// L0 - Ignored
	if (ent->client->sess.ignored > IGNORE_OFF) {
		CP(va("cp \"You are %s ignored^1!\n\"2",
			ent->client->sess.ignored == IGNORE_PERMANENT ? "permanently" : "temporarily"));
		return;
	}

	if (trap_Argc() < 3) {
		trap_SendServerCommand(ent - g_entities, "print \"Usage: tell <player id> <message>\n\"");
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		return;
	}

	p = ConcatArgs( 2 );

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );
	G_Say( ent, target, SAY_TELL, p );
	if (ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say( ent, ent, SAY_TELL, p );
	}
}

// NERVE - SMF
static void G_VoiceTo( gentity_t *ent, gentity_t *other, int mode, const char *id, qboolean voiceonly ) {
	int color;
	char *cmd;

	if (!other) {
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if ( mode == SAY_TEAM && !OnSameTeam(ent, other) ) {
		return;
	}

	if (COM_BitCheck(other->client->sess.muted, ent - g_entities))
	{
		return;
	}

	// no chatting to players in tournements
	if ( (g_gametype.integer == GT_TOURNAMENT )) {
		return;
	}

	if (mode == SAY_TEAM) {
		color = COLOR_CYAN;
		cmd = "vtchat";
	}
	else if (mode == SAY_TELL) {
		color = COLOR_MAGENTA;
		cmd = "vtell";
	}
	else {
		color = COLOR_GREEN;
		cmd = "vchat";
	}

	trap_SendServerCommand( other - g_entities, va("%s %d %d %d %s %i %i %i", cmd, voiceonly, ent - g_entities, color, id, 
		(int)ent->s.pos.trBase[0], (int)ent->s.pos.trBase[1], (int)ent->s.pos.trBase[2] ));
}

void G_Voice( gentity_t *ent, gentity_t *target, int mode, const char *id, qboolean voiceonly ) {
	int			j;
	gentity_t	*other;

	// L0 - Ignored
	if (ent->client->sess.ignored > IGNORE_OFF) {
		CP(va("cp \"You are %s ignored^1!\n\"2",
			ent->client->sess.ignored == IGNORE_PERMANENT ? "permanently" : "temporarily"));
		return;
	}

	if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
		mode = SAY_ALL;
	}

	// DHM - Nerve :: Don't allow excessive spamming of voice chats
	ent->voiceChatSquelch -= (level.time - ent->voiceChatPreviousTime);
	ent->voiceChatPreviousTime = level.time;

	if ( ent->voiceChatSquelch < 0 )
		ent->voiceChatSquelch = 0;

	if ( ent->voiceChatSquelch >= 30000 ) {
		trap_SendServerCommand( ent-g_entities, "print \"^1Spam Protection^7: VoiceChat ignored\n\"" );

		if (sb_system.integer && sb_autoIgnore.integer) {
			if (ent->client->pers.sb_ignored == (sb_autoIgnore.integer - 1)) {
				CPx(ent - g_entities, "chat \"^3Warning^7! Next time You will get ^3auto-ignored ^7until the next round!\n\"2");
			}
			else if (ent->client->pers.sb_ignored == sb_autoIgnore.integer) {
				if (sb_maxTempIgnores.integer && ent->client->sess.tempIgnoreCount == sb_maxTempIgnores.integer) {
					AP(va("chat \"console: %s ^7has reached the temp-ignore limit and is now permanently ignored.\n\"", ent->client->pers.netname));
					ent->client->sess.ignored = IGNORE_PERMANENT;
				}
				else {
					AP(va("print \"%s ^7has been temp-ignored for spam\n\"", ent->client->pers.netname));
					ent->client->sess.ignored = IGNORE_TEMPORARY;
				}

				ent->client->sess.tempIgnoreCount++;
			}

			ent->client->pers.sb_ignored++;
		}

		return;
	}

	if ( g_voiceChatsAllowed.integer )
		ent->voiceChatSquelch += (34000 / g_voiceChatsAllowed.integer);
	else
		return;
	// dhm

	/*
		// L0 

		Prevent any vsay exploits...
		Additionally client will get slapped..
	*/
	if ((mode == SAY_TEAM || mode == SAY_ALL) && 
		(!Q_stricmp(id, "DynamiteDefused") 
		|| !Q_stricmp(id, "DynamitePlanted") 
		|| !Q_stricmp(id, "Axiswin") 
		|| !Q_stricmp(id, "Alliedwin"))) 
	{	
		if (ent->client->sess.sessionTeam == TEAM_RED || ent->client->sess.sessionTeam == TEAM_BLUE)
		{
			CP(va("cp \"You got slapped for Vsay exploit attempt!\n\"2"));
			G_Damage(ent, NULL, NULL, NULL, NULL, 20, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
		}
		return;
	}

	if ( target ) {
		G_VoiceTo( ent, target, mode, id, voiceonly );
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) {
		G_Printf( "voice: %s %s\n", ent->client->pers.netname, id);
	}

	// send it to all the apropriate clients
	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		G_VoiceTo( ent, other, mode, id, voiceonly );
	}
}

/*
==================
Cmd_Voice_f
==================
*/
static void Cmd_Voice_f( gentity_t *ent, int mode, qboolean arg0, qboolean voiceonly ) {
	char		*p;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	G_Voice( ent, NULL, mode, p, voiceonly );
}

static void Cmd_VoiceAll_f(gentity_t *ent) {
	Cmd_Voice_f(ent, SAY_ALL, qfalse, qfalse);
}

static void Cmd_VoiceTeam_f(gentity_t *ent) {
	if (g_ffa.integer)
	{
		Cmd_VoiceAll_f(ent);
		return;
	}

	Cmd_Voice_f(ent, SAY_TEAM, qfalse, qfalse);
}

// TTimo gcc: defined but not used
#if 0
/*
==================
Cmd_VoiceTell_f
==================
*/
static void Cmd_VoiceTell_f( gentity_t *ent, qboolean voiceonly ) {
	int			targetNum;
	gentity_t	*target;
	char		*id;
	char		arg[MAX_TOKEN_CHARS];

	// L0 - Ignored
	if (ent->client->sess.ignored > IGNORE_OFF) {
		CP(va("cp \"You are %s ignored^1!\n\"2",
			ent->client->sess.ignored == IGNORE_PERMANENT ? "permanently" : "temporarily"));	
		return;
	}

	if ( trap_Argc () < 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		return;
	}

	id = ConcatArgs( 2 );

	G_LogPrintf( "vtell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, id );
	G_Voice( ent, target, SAY_TELL, id, voiceonly );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Voice( ent, ent, SAY_TELL, id, voiceonly );
	}
}
#endif

// TTimo gcc: defined but not used
#if 0
/*
==================
Cmd_VoiceTaunt_f
==================
*/
static void Cmd_VoiceTaunt_f( gentity_t *ent ) {
	gentity_t *who;
	int i;

	if (!ent->client) {
		return;
	}

	// L0 - Ignored
	if (ent->client->sess.ignored > IGNORE_OFF) {
		CP(va("cp \"You are %s ignored^1!\n\"2",
			ent->client->sess.ignored == IGNORE_PERMANENT ? "permanently" : "temporarily"));	
		return;
	}

	// insult someone who just killed you
	if (ent->enemy && ent->enemy->client && ent->enemy->client->lastkilled_client == ent->s.number) {
		// i am a dead corpse
		if (!(ent->enemy->r.svFlags & SVF_BOT)) {
//			G_Voice( ent, ent->enemy, SAY_TELL, VOICECHAT_DEATHINSULT, qfalse );
		}
		if (!(ent->r.svFlags & SVF_BOT)) {
//			G_Voice( ent, ent,        SAY_TELL, VOICECHAT_DEATHINSULT, qfalse );
		}
		ent->enemy = NULL;
		return;
	}
	// insult someone you just killed
	if (ent->client->lastkilled_client >= 0 && ent->client->lastkilled_client != ent->s.number) {
		who = g_entities + ent->client->lastkilled_client;
		if (who->client) {
			// who is the person I just killed
			if (who->client->lasthurt_mod == MOD_GAUNTLET) {
				if (!(who->r.svFlags & SVF_BOT)) {
//					G_Voice( ent, who, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalse );	// and I killed them with a gauntlet
				}
				if (!(ent->r.svFlags & SVF_BOT)) {
//					G_Voice( ent, ent, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalse );
				}
			} else {
				if (!(who->r.svFlags & SVF_BOT)) {
//					G_Voice( ent, who, SAY_TELL, VOICECHAT_KILLINSULT, qfalse );	// and I killed them with something else
				}
				if (!(ent->r.svFlags & SVF_BOT)) {
//					G_Voice( ent, ent, SAY_TELL, VOICECHAT_KILLINSULT, qfalse );
				}
			}
			ent->client->lastkilled_client = -1;
			return;
		}
	}

	if (g_gametype.integer >= GT_TEAM) {
		// praise a team mate who just got a reward
		for(i = 0; i < MAX_CLIENTS; i++) {
			who = g_entities + i;
			if (who->client && who != ent && who->client->sess.sessionTeam == ent->client->sess.sessionTeam) {
				if (who->client->rewardTime > level.time) {
					if (!(who->r.svFlags & SVF_BOT)) {
//						G_Voice( ent, who, SAY_TELL, VOICECHAT_PRAISE, qfalse );
					}
					if (!(ent->r.svFlags & SVF_BOT)) {
//						G_Voice( ent, ent, SAY_TELL, VOICECHAT_PRAISE, qfalse );
					}
					return;
				}
			}
		}
	}

	// just say something
//	G_Voice( ent, NULL, SAY_ALL, VOICECHAT_TAUNT, qfalse );
}
// -NERVE - SMF
#endif

static char	*gc_orders[] = {
	"hold your position",
	"hold this position",
	"come here",
	"cover me",
	"guard location",
	"search and destroy",
	"report"
};

void Cmd_GameCommand_f(gentity_t *ent) {
	int		player;
	int		order;
	char	str[MAX_TOKEN_CHARS];

	trap_Argv(1, str, sizeof(str));
	player = atoi(str);
	trap_Argv(2, str, sizeof(str));
	order = atoi(str);

	if (player < 0 || player >= MAX_CLIENTS) {
		return;
	}
	if (order < 0 || order > sizeof(gc_orders) / sizeof(char *)) {
		return;
	}
	G_Say(ent, &g_entities[player], SAY_TELL, gc_orders[order]);
	G_Say(ent, ent, SAY_TELL, gc_orders[order]);
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
}


static const char *gameNames[] = {
	"Free For All",
	"Tournament",
	"Single Player",
	"Team Deathmatch",
	"Capture the Flag",
	"Wolf Multiplayer",
	"Wolf Stopwatch",
	"Wolf Checkpoint"
};

/*
==================
Cmd_CallVote_f
==================
*/
void Cmd_CallVote_f( gentity_t *ent ) {
	int		i;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
	char	cleanName[64]; // JPW NERVE
	char	*check;
	static char votes[512] = { 0 };

	if (!votes[0]) {
		Q_strncpyz(votes, "map_restart, nextmap, start_match, swap_teams, reset_match, map <mapname>, g_gametype <n>, kick <player>, "
			"clientkick <clientnum>, ? (poll), shuffle, ignore, unignore", sizeof(votes));

		if (strlen(g_voteConfigs.string)) {
			Q_strcat(votes, sizeof(votes), va(", %s", Q_StrReplace(g_voteConfigs.string, " ", ", ")));
		}
	}

	// L0 - Ignored
	if (ent->client->sess.ignored > IGNORE_OFF) {
		CP(va("cp \"You are %s ignored^1!\n\"2",
			ent->client->sess.ignored == IGNORE_PERMANENT ? "permanently" : "temporarily"));
		return;
	}

	if ( !g_allowVote.integer ) {
		CP( "print \"Voting not enabled on this server.\n\"" );
		return;
	}

	if ( level.voteTime ) {
		CP( "print \"A vote is already in progress.\n\"" );
		return;
	}
	// L0 - Admins can bypass this
	if (ent->client->pers.voteCount >= g_maxVotes.integer && !ent->client->sess.admin) {
		CP( "print \"You have called the maximum number of votes.\n\"" );
		return;
	}
	// L0 - Admins can bypass this
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR && !ent->client->sess.admin) {
		CP( "print \"Not allowed to call a vote as spectator.\n\"" );
		return;
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );

	// L0 - Fix callvote exploit
	for (check = arg2; *check; ++check) {
		switch (*check) {
		case '\n':
		case '\r':
		case '"':
		case ';':
			CP("print \"Invalid vote string.\n\"");
			return;
			break;
		}
	}

	if ( !Q_stricmp( arg1, "map_restart" ) ) {
	} else if ( !Q_stricmp( arg1, "nextmap" ) ) {
	} else if ( !Q_stricmp( arg1, "map" ) ) {
	} else if ( !Q_stricmp( arg1, "g_gametype" ) ) {
	} else if ( !Q_stricmp( arg1, "kick" ) ) {
	} else if ( !Q_stricmp( arg1, "clientkick" ) ) {
	//} else if ( !Q_stricmp( arg1, "timelimit" ) ) {
	} else if ( !Q_stricmp( arg1, "start_match" ) ) {		// NERVE - SMF
	} else if ( !Q_stricmp( arg1, "reset_match" ) ) {		// NERVE - SMF
	} else if ( !Q_stricmp( arg1, "swap_teams" ) ) {		// NERVE - SMF
// L0 - New votes
	} else if ( !Q_stricmp( arg1, "shuffle" ) ) {  	
	} else if ( !Q_stricmp( arg1, "?" )) {
	} else if ( !Q_stricmp( arg1, "ignore" )) {
	} else if ( !Q_stricmp( arg1, "unignore" )) {
	} else if ( Q_FindToken(g_voteConfigs.string, arg1) ) {
// End

// JPW NERVE
#ifndef PRE_RELEASE_DEMO
	} else if ( !Q_stricmp( arg1, testid1 ) ) { 
	} else if ( !Q_stricmp( arg1, testid2 ) ) {
	} else if ( !Q_stricmp( arg1, testid3 ) ) {
#endif
// jpw
	} else {
		CP( "print \"Invalid vote string.\n\"" );
		CP( va("print \"Vote commands are: %s\n\"", votes) );
		return;
	}

	// L0 - Check if enough of time has passed before calling another vote (only non logged in players)
	if ((level.lastVoteTime + 1000 * g_voteDelay.integer) > level.time && !ent->client->sess.admin
		&& g_gamestate.integer == GS_PLAYING)
	{
		CP(va("cp \"Please wait ^3%d ^7seconds before calling a vote.\n\"2",
			(int)((level.lastVoteTime + 1000 * g_voteDelay.integer) - level.time) / 1000));
		return;
	}

	// L0 - disallow any votes defined in g_disallowedVotes cvar		
	if (Q_FindToken(g_disallowedVotes.string, arg1))
	{
		CP(va("print \"Voting for ^3%s ^7is disabled on this server!\n\"", arg1));
		return;
	} 

	// if there is still a vote to be executed
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}

	// special case for g_gametype, check for bad values
	if ( !Q_stricmp( arg1, "g_gametype" ) ) {
		i = atoi( arg2 );
		if( i < GT_WOLF || i >= GT_MAX_GAME_TYPE) {
			CP( "print \"Invalid gametype.\n\"" );
			return;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %d", arg1, i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, gameNames[i] );
	} else if ( !Q_stricmp( arg1, "map_restart" ) ) {
		// NERVE - SMF - do a warmup when we restart maps
		if ( strlen( arg2 ) )
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		else
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s", arg1, arg2 );

		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	} else if ( !Q_stricmp( arg1, "map" ) ) {
		// special case for map changes, we want to reset the nextmap setting
		// this allows a player to change maps, but not upset the map rotation
		char	s[MAX_STRING_CHARS];

		if (!strlen(arg2))
		{
			CP("print \"Please provide a map name.\n\"");
			return;
		}

		if (!FileExists(arg2, "maps", ".bsp", qfalse))
		{
			CP(va("print \"^3%s ^7is not on the server.\n\"", arg2));
			return;
		}

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (*s) {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s; set nextmap \"%s\"", arg1, arg2, s );
		} else {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
		}
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	} else if ( !Q_stricmp( arg1, "nextmap" ) ) {
		char	s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (!*s) {
			CP( "print \"nextmap not set.\n\"" );
			return;
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap");
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
// JPW NERVE
	} else if (!Q_stricmp(arg1,"kick")) {
		int kicknum=MAX_CLIENTS;
		for (i=0;i<MAX_CLIENTS;i++) {
			if (level.clients[i].pers.connected != CON_CONNECTED)
				continue;
// strip the color crap out
			Q_strncpyz( cleanName, level.clients[i].pers.netname, sizeof(cleanName) );
			Q_CleanStr( cleanName );
			if ( !Q_stricmp( cleanName, arg2 ) )
				kicknum = i;
		}
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "kick %s", level.clients[kicknum].pers.netname );
		if (kicknum != MAX_CLIENTS) { // found a client # to kick, so override votestring with better one
			Com_sprintf(level.voteString, sizeof(level.voteString),"clientkick \"%d\"",kicknum);
		}
		else { // if it can't do a name match, don't allow kick (to prevent votekick text spam wars)
			CP( "print \"Client not on server.\n\"" );
			return;
		}
// jpw

// L0 - New votes
	// Shuffle
	} else if (!Q_stricmp(arg1, "shuffle")) {
		Com_sprintf(level.voteString, sizeof(level.voteString), "Shuffle teams?");
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), "%s", level.voteString);
	// Poll
	} else if (!Q_stricmp(arg1, "?")) {
		char	*s;
		s = ConcatArgs(2);

		if (*s) {
			Com_sprintf(level.voteString, sizeof(level.voteString), "\"poll\"");
		}
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), "Poll: %s", s);
	// Ignore
	} else if (!Q_stricmp(arg1, "ignore")) {
		int num = MAX_CLIENTS;
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			if (level.clients[i].pers.connected != CON_CONNECTED) {
				continue;
			}

			Q_strncpyz(cleanName, level.clients[i].pers.netname, sizeof(cleanName));
			Q_CleanStr(cleanName);

			if (!Q_stricmp(cleanName, arg2)) {
				num = i;
			}
		}
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), "ignore %s", level.clients[num].pers.netname);
		if (num != MAX_CLIENTS) {
			Com_sprintf(level.voteString, sizeof(level.voteString), "ignore \"%d\"", num);
		}
		else {
			CP("print \"Client not on server.\n\"");
			return;
		}
	// Unignore
	} else if (!Q_stricmp(arg1, "unignore")) {
		int num = MAX_CLIENTS;
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			if (level.clients[i].pers.connected != CON_CONNECTED) {
				continue;
			}

			Q_strncpyz(cleanName, level.clients[i].pers.netname, sizeof(cleanName));
			Q_CleanStr(cleanName);

			if (!Q_stricmp(cleanName, arg2)) {
				num = i;
			}
		}
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), "unignore %s", level.clients[num].pers.netname);
		if (num != MAX_CLIENTS) {
			Com_sprintf(level.voteString, sizeof(level.voteString), "unignore \"%d\"", num);
		}
		else {
			CP("print \"Client not on server.\n\"");
			return;
		}
// End
	} else if (Q_FindToken(g_voteConfigs.string, arg1)) {
		if (!FileExists(arg1, "", ".cfg", qtrue)) {
			CP(va("print \"%s.cfg is not on server.\n\"", arg1));
			return;
		}

		Com_sprintf(level.voteString, sizeof(level.voteString), "exec %s", arg1);
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), "Change to %s?", arg1);
	} else {
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}

// L0
	// For verbosity print in console as well	
	if (!Q_stricmp(arg1, "?")) {
		AP(va("print \"%s ^7called a vote: ^3Poll\n\"", ent->client->pers.netname));
	}
	else if (!Q_stricmp(arg1, "g_gametype")) {
		AP(va("print \"%s ^7called a vote: ^3g_gametype\n\"", ent->client->pers.netname));
	}
	else if (Q_FindToken(g_voteConfigs.string, arg1)) {
		AP(va("print \"%s ^7called a vote: ^3%s\n\"", ent->client->pers.netname, arg1));
	}
	else {
		AP(va("print \"%s ^7called a vote: ^3%s\n\"", ent->client->pers.netname, level.voteString));
	}

	// Add a sound
	APS("sound/player/vote-call.wav");
// End

	// start the voting, the caller autoamtically votes yes
	level.voteTime = level.time;
	level.lastVoteTime = level.time;
	level.voteYes = 1;
	level.voteNo = 0;
	// L0 - Ehm, and where's the user vote count? Doh..
	ent->client->pers.voteCount++; 

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		level.clients[i].ps.eFlags &= ~EF_VOTED;
	}
	ent->client->ps.eFlags |= EF_VOTED;

	trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime ) );
	trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );	
	trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
	char		msg[64];
	int			num;

	// DHM - Nerve :: Complaints supercede voting (and share command)
	if ( ent->client->pers.complaintEndTime > level.time ) {
		gentity_t *tent = g_entities + ent->client->pers.complaintClient;
		gclient_t *cl = tent->client;

		if ( !cl )
			return;
		if ( cl->pers.connected != CON_CONNECTED )
			return;
		if ( cl->pers.localClient ) {
			trap_SendServerCommand( ent-g_entities, "complaint -3" );
			return;
		}

		// Reset this ent's complainEndTime so they can't send multiple complaints
		ent->client->pers.complaintEndTime = -1;
		ent->client->pers.complaintClient = -1;

		trap_Argv( 1, msg, sizeof( msg ) );

		if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
			// Increase their complaint counter
			cl->pers.complaints++;

			num = g_complaintlimit.integer - cl->pers.complaints;

			if ( num <= 0 && !cl->pers.localClient ) {
				trap_DropClient( cl - level.clients, "kicked after too many complaints." );
				trap_SendServerCommand( ent-g_entities, "complaint -1" );
				return;
			}

			trap_SendServerCommand(tent->s.clientNum, va("print \"^1Warning^7: Complaint filed against you. (%d until kicked)\n\"", num ) );
			trap_SendServerCommand( ent-g_entities, "complaint -1" );
		}
		else
			trap_SendServerCommand( ent-g_entities, "complaint -2" );

		return;
	}
	// dhm

	// Reset this ent's complainEndTime so they can't send multiple complaints
	ent->client->pers.complaintEndTime = -1;
	ent->client->pers.complaintClient = -1;

	if ( !level.voteTime ) {
		trap_SendServerCommand( ent-g_entities, "print \"No vote in progress.\n\"" );
		return;
	}
	if ( ent->client->ps.eFlags & EF_VOTED ) {
		trap_SendServerCommand( ent-g_entities, "print \"Vote already cast.\n\"" );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
		return;
	}

	trap_SendServerCommand( ent-g_entities, "print \"Vote cast.\n\"" );

	ent->client->ps.eFlags |= EF_VOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
		level.voteYes++;
		trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
		APS("sound/player/vote-yes.wav");
	} else {
		level.voteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
		APS("sound/player/vote-no.wav");
	}

	// a majority will be determined in G_CheckVote, which will also account
	// for players entering or leaving
}


qboolean G_canPickupMelee(gentity_t *ent) {

// JPW NERVE -- no "melee" weapons in net play
	if (g_gametype.integer >= GT_WOLF)
		return qfalse;
// jpw

	if(!(ent->client))
		return qfalse;	// hmm, shouldn't be too likely...

	if(!(ent->s.weapon))	// no weap, go ahead
		return qtrue;

	if(ent->client->ps.weaponstate == WEAPON_RELOADING)
		return qfalse;

	if( WEAPS_ONE_HANDED & (1<<(ent->s.weapon)) )
		return qtrue;

	return qfalse;
}




/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
		return;
	}
	if ( trap_Argc() != 5 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) {
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap_Argv( 4, buffer, sizeof( buffer ) );
	angles[YAW] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}

/*
=================
Cmd_StartCamera_f
=================
*/
void Cmd_StartCamera_f( gentity_t *ent ) {

	if ( !CheatsOk( ent ) ) {
		return;
	}

	g_camEnt->r.svFlags |= SVF_PORTAL;
	g_camEnt->r.svFlags &= ~SVF_NOCLIENT;
	ent->client->cameraPortal = g_camEnt;
	ent->client->ps.eFlags |= EF_VIEWING_CAMERA;
	ent->s.eFlags |= EF_VIEWING_CAMERA;
}

/*
=================
Cmd_StopCamera_f
=================
*/
void Cmd_StopCamera_f( gentity_t *ent ) {

	if ( !CheatsOk( ent ) ) {
		return;
	}

	if (ent->client->cameraPortal) {
		// send a script event
		G_Script_ScriptEvent( ent->client->cameraPortal, "stopcam", "" );
		// go back into noclient mode
		ent->client->cameraPortal->r.svFlags |= SVF_NOCLIENT;
		ent->client->cameraPortal = NULL;
		ent->s.eFlags &= ~EF_VIEWING_CAMERA;
		ent->client->ps.eFlags &= ~EF_VIEWING_CAMERA;
	}
}

/*
=================
Cmd_SetCameraOrigin_f
=================
*/
void Cmd_SetCameraOrigin_f( gentity_t *ent ) {
	char		buffer[MAX_TOKEN_CHARS];
	int i;

	if ( trap_Argc() != 4 ) {
		return;
	}

	VectorClear( ent->client->cameraOrigin );
	for ( i = 0 ; i < 3 ; i++ ) {
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		ent->client->cameraOrigin[i] = atof( buffer );
	}
}


// Rafael
/*
==================
Cmd_Activate_f
==================
*/
void Cmd_Activate_f (gentity_t *ent)
{
	trace_t		tr;
	vec3_t		end;
	gentity_t	*traceEnt;
	vec3_t		forward, right, up, offset;
	static int	oldactivatetime = 0;
	int			activatetime = level.time;
	qboolean	walking = qfalse;

	if(ent->client->pers.cmd.buttons & BUTTON_WALKING)
		walking = qtrue;

	AngleVectors (ent->client->ps.viewangles, forward, right, up);
	CalcMuzzlePointForActivate (ent, forward, right, up, offset);
	VectorMA (offset, 96, forward, end);

	trap_Trace (&tr, offset, NULL, NULL, end, ent->s.number, (CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE|CONTENTS_TRIGGER));

	if ( tr.surfaceFlags & SURF_NOIMPACT )
		return;

	if ( tr.entityNum == ENTITYNUM_WORLD )
		return;

	traceEnt = &g_entities[ tr.entityNum ];

	if (traceEnt->classname)
	{
		if (Q_stricmp(traceEnt->classname, "weapon_knife") == 0 && traceEnt->s.pos.trType == TR_GRAVITY)
		{
			return;
		}

		traceEnt->flags &= ~FL_SOFTACTIVATE;	// FL_SOFTACTIVATE will be set if the user is holding 'walk' key

		if (traceEnt->s.eType == ET_ALARMBOX)
		{
			trace_t		trace;

			if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR )
				return;
			
			memset( &trace, 0, sizeof(trace) );

			if(traceEnt->use)
				traceEnt->use(traceEnt, ent, 0);
		}
		else if (traceEnt->s.eType == ET_ITEM)
		{
			trace_t		trace;

			if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR )
				return;
			
			memset( &trace, 0, sizeof(trace) );

			if ( traceEnt->touch ) {
				if(ent->client->pers.autoActivate == PICKUP_ACTIVATE)
					ent->client->pers.autoActivate = PICKUP_FORCE;		//----(SA) force pickup
				traceEnt->active = qtrue;
				traceEnt->touch (traceEnt, ent, &trace);
			}

		}
		else if ((Q_stricmp (traceEnt->classname, "misc_mg42") == 0) && traceEnt->active == qfalse)
		{
			if (!infront (traceEnt, ent))
			{
				//----(SA)	make sure the client isn't holding a hot potato
				gclient_t	*cl;
				cl = &level.clients[ ent->s.clientNum ];

				if(!(cl->ps.grenadeTimeLeft) && !(cl->ps.pm_flags & PMF_DUCKED) 
					&& !(traceEnt->s.eFlags & EF_SMOKING) && (cl->ps.weapon != WP_SNIPERRIFLE)) // JPW NERVE no mg42 while scoped in
				{ 
					// DHM - Remember initial gun position to restore later
					if ( g_gametype.integer >= GT_WOLF ) {
						vec3_t	point;

						AngleVectors (traceEnt->s.apos.trBase, forward, NULL, NULL);
						VectorMA (traceEnt->r.currentOrigin, -36, forward, point);
						point[2] = ent->r.currentOrigin[2];

						// Save initial position
						VectorCopy( point, ent->TargetAngles );

						// Zero out velocity
						VectorCopy( vec3_origin, ent->client->ps.velocity );
						VectorCopy( vec3_origin, ent->s.pos.trDelta );
					}
					traceEnt->active = qtrue;
					VectorCopy( traceEnt->s.angles, traceEnt->TargetAngles );
					ent->active = qtrue;
					traceEnt->r.ownerNum = ent->s.number;
					traceEnt->s.otherEntityNum = ent->s.number;

					cl->ps.harc = traceEnt->harc;
					cl->ps.varc = traceEnt->varc;
					VectorCopy(traceEnt->s.angles, cl->ps.centerangles);
					cl->ps.centerangles[PITCH] = AngleNormalize180(cl->ps.centerangles[PITCH]);
					cl->ps.centerangles[YAW] = AngleNormalize180(cl->ps.centerangles[YAW]);
					cl->ps.centerangles[ROLL] = AngleNormalize180(cl->ps.centerangles[ROLL]);

					if (!(ent->r.svFlags & SVF_CASTAI))
						G_UseTargets( traceEnt, ent);	//----(SA)	added for Mike so mounting an MG42 can be a trigger event (let me know if there's any issues with this)
				}
			}
		}
		else if ( ( (Q_stricmp (traceEnt->classname, "func_door") == 0) || (Q_stricmp (traceEnt->classname, "func_door_rotating") == 0) ) )
		{
			if(walking)
				traceEnt->flags |= FL_SOFTACTIVATE;		// no noise
			G_TryDoor(traceEnt, ent, ent);		// (door,other,activator)
		}
		else if ( (Q_stricmp (traceEnt->classname, "team_WOLF_checkpoint") == 0) )
		{
			if ( traceEnt->count != ent->client->sess.sessionTeam ) {
				traceEnt->health++;
			}
		}
		else if ( (Q_stricmp (traceEnt->classname, "func_button") == 0) 
			&& ( traceEnt->s.apos.trType == TR_STATIONARY && traceEnt->s.pos.trType == TR_STATIONARY) 
			&& traceEnt->active == qfalse )
		{
			Use_BinaryMover (traceEnt, ent, ent);
			traceEnt->active = qtrue;
		}
		else if ( !Q_stricmp (traceEnt->classname, "func_invisible_user") )
		{
			if(walking)
				traceEnt->flags |= FL_SOFTACTIVATE;		// no noise
			traceEnt->use ( traceEnt, ent, ent );
		}
		else if ( !Q_stricmp (traceEnt->classname, "script_mover") )
		{
			G_Script_ScriptEvent( traceEnt, "activate", ent->aiName );
		}
		else if ( !Q_stricmp (traceEnt->classname, "props_footlocker") )
		{
			traceEnt->use ( traceEnt, ent, ent );
		}
	}
	else if (ent->active)
	{
		if (ent->client->ps.persistant[PERS_HWEAPON_USE])
		{
			// DHM - Nerve :: Restore original position if current position is bad
			if ( g_gametype.integer >= GT_WOLF ) {
				trap_Trace (&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, ent->s.number, MASK_PLAYERSOLID);
				if ( tr.startsolid ) {
					VectorCopy( ent->TargetAngles, ent->client->ps.origin );
					VectorCopy( ent->TargetAngles, ent->r.currentOrigin );
					ent->r.contents = CONTENTS_CORPSE;		// DHM - this will correct itself in ClientEndFrame
				}
			}
			ent->client->ps.eFlags &= ~EF_MG42_ACTIVE;			// DHM - Nerve :: unset flag
			ent->client->ps.persistant[PERS_HWEAPON_USE] = 0;
			ent->active = qfalse;
		}
		else
		{
			ent->active = qfalse;
		}
	}

	if (activatetime > oldactivatetime + 1000)
		oldactivatetime = activatetime;
}

// Rafael WolfKick
//===================
//	Cmd_WolfKick
//===================

#define WOLFKICKDISTANCE	96
int Cmd_WolfKick_f (gentity_t *ent)
{
	trace_t		tr;
	vec3_t		end;
	gentity_t	*traceEnt;
	vec3_t		forward, right, up, offset;
	gentity_t	*tent;
	static int	oldkicktime = 0;
	int			kicktime = level.time;
	qboolean	solidKick = qfalse;	// don't play "hit" sound on a trigger unless it's an func_invisible_user
	
	int			damage = 15;

	// DHM - Nerve :: No kick in wolf multiplayer
	if ( g_gametype.integer >= GT_WOLF )
		return 0;

	if(ent->client->ps.leanf)
		return 0;	// no kick when leaning

	if (oldkicktime > kicktime)
		return (0);
	else 
		oldkicktime = kicktime + 1000;

	// play the anim
	BG_AnimScriptEvent( &ent->client->ps, ANIM_ET_KICK, qfalse, qtrue );

	ent->client->ps.persistant[PERS_WOLFKICK] = 1;

	AngleVectors (ent->client->ps.viewangles, forward, right, up);

	CalcMuzzlePointForActivate (ent, forward, right, up, offset);

	// note to self: we need to determine the usable distance for wolf
	VectorMA (offset, WOLFKICKDISTANCE, forward, end);

	trap_Trace (&tr, offset, NULL, NULL, end, ent->s.number, (CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE|CONTENTS_TRIGGER));
	
	if ( tr.surfaceFlags & SURF_NOIMPACT || tr.fraction == 1.0)
	{
		tent = G_TempEntity( tr.endpos, EV_WOLFKICK_MISS );
		tent->s.eventParm = ent->s.number;
		return (1);
	}

	traceEnt = &g_entities[ tr.entityNum ];

	if (!ent->melee) // because we dont want you to open a door with a prop
	{
		if ( (Q_stricmp (traceEnt->classname, "func_door_rotating") == 0) 
			&& ( traceEnt->s.apos.trType == TR_STATIONARY && traceEnt->s.pos.trType == TR_STATIONARY) 
			&& traceEnt->active == qfalse )
		{
			if(traceEnt->key < 0)	// door force locked
			{
				//----(SA)	play kick "hit" sound
				tent = G_TempEntity( tr.endpos, EV_WOLFKICK_HIT_WALL );
				tent->s.otherEntityNum = ent->s.number;
				//----(SA)	end

				AICast_AudibleEvent( ent->s.clientNum, tr.endpos, HEAR_RANGE_DOOR_KICKLOCKED );	// "someone kicked a locked door near me!"

				if(traceEnt->soundPos3)
					G_AddEvent( traceEnt, EV_GENERAL_SOUND, traceEnt->soundPos3);
				else
					G_AddEvent( traceEnt, EV_GENERAL_SOUND, G_SoundIndex( "sound/movers/doors/default_door_locked.wav" ) );
				return 1;	//----(SA)	changed.  shows boot for locked doors
			}
			
			if(traceEnt->key > 0)	// door requires key
			{
				gitem_t *item = BG_FindItemForKey(traceEnt->key, 0);
				if(!(ent->client->ps.stats[STAT_KEYS] & (1<<item->giTag)))
				{
					//----(SA)	play kick "hit" sound
					tent = G_TempEntity( tr.endpos, EV_WOLFKICK_HIT_WALL );
					tent->s.otherEntityNum = ent->s.number;\
					//----(SA)	end

					AICast_AudibleEvent( ent->s.clientNum, tr.endpos, HEAR_RANGE_DOOR_KICKLOCKED );	// "someone kicked a locked door near me!"

					// player does not have key
					if(traceEnt->soundPos3)
						G_AddEvent( traceEnt, EV_GENERAL_SOUND, traceEnt->soundPos3);
					else
						G_AddEvent( traceEnt, EV_GENERAL_SOUND, G_SoundIndex( "sound/movers/doors/default_door_locked.wav" ) );
					return 1;	//----(SA)	changed.  shows boot animation for locked doors
				}
			}
			
			if (traceEnt->teammaster && traceEnt->team && traceEnt != traceEnt->teammaster)
			{
				traceEnt->teammaster->active = qtrue;
				traceEnt->teammaster->flags |= FL_KICKACTIVATE;
				Use_BinaryMover (traceEnt->teammaster, ent, ent);
				G_UseTargets( traceEnt->teammaster, ent);
			}
			else	
			{
				traceEnt->active = qtrue;
				traceEnt->flags |= FL_KICKACTIVATE;
				Use_BinaryMover (traceEnt, ent, ent);
				G_UseTargets( traceEnt, ent);
			}
		}
		else if ( (Q_stricmp (traceEnt->classname, "func_button") == 0) 
			&& ( traceEnt->s.apos.trType == TR_STATIONARY && traceEnt->s.pos.trType == TR_STATIONARY) 
			&& traceEnt->active == qfalse )
		{
			Use_BinaryMover (traceEnt, ent, ent);
			traceEnt->active = qtrue;

		}
		else if ( !Q_stricmp (traceEnt->classname, "func_invisible_user") )
		{
			traceEnt->flags |= FL_KICKACTIVATE;		// so cell doors know they were kicked
													// It doesn't hurt to pass this along since only ent use() funcs who care about it will check.
													// However, it may become handy to put a "KICKABLE" or "NOTKICKABLE" flag on the invisible_user
			traceEnt->use ( traceEnt, ent, ent );
			traceEnt->flags &= ~FL_KICKACTIVATE;	// reset

			solidKick = qtrue;	//----(SA)	
		}
		else if (!Q_stricmp (traceEnt->classname, "props_flippy_table") && traceEnt->use)
		{
			traceEnt->use (traceEnt, ent, ent);
		}
	}

	// snap the endpos to integers, but nudged towards the line
	SnapVectorTowards( tr.endpos, offset );

	// send bullet impact
	if ( traceEnt->takedamage && traceEnt->client ) {
		tent = G_TempEntity( tr.endpos, EV_WOLFKICK_HIT_FLESH );
		tent->s.eventParm = traceEnt->s.number;
		if( LogAccuracyHit( traceEnt, ent ) ) {
			ent->client->ps.persistant[PERS_ACCURACY_HITS]++;
		}
	} else {
		// Ridah, bullet impact should reflect off surface
		vec3_t	reflect;
		float	dot;

		if( traceEnt->r.contents >= 0 && (traceEnt->r.contents & CONTENTS_TRIGGER) && !solidKick ) {
			tent = G_TempEntity( tr.endpos, EV_WOLFKICK_MISS );	// (SA) don't play the "hit" sound if you kick most triggers
		} else {
			tent = G_TempEntity( tr.endpos, EV_WOLFKICK_HIT_WALL );
		}
		

		dot = DotProduct( forward, tr.plane.normal );
		VectorMA( forward, -2*dot, tr.plane.normal, reflect );
		VectorNormalize( reflect );

		tent->s.eventParm = DirToByte( reflect );
		// done.

		if (ent->melee)
		{
			ent->active = qfalse;
			ent->melee->health = 0;
		}
	}

	tent->s.otherEntityNum = ent->s.number;

	// try to swing chair
	if (traceEnt->takedamage) {

		if (ent->melee)
		{
			ent->active = qfalse;
			ent->melee->health = 0;
			ent->client->ps.eFlags &= ~EF_MELEE_ACTIVE;

		}

		G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, 0, MOD_KICKED);	//----(SA)	modified
	}

	return (1);
}
// done

/*
============================
Cmd_ClientMonsterSlickAngle 
============================
*/
/*
void Cmd_ClientMonsterSlickAngle (gentity_t *clent) {

	char s[MAX_STRING_CHARS];
	int	entnum;
	int angle;
	gentity_t *ent;
	vec3_t	dir, kvel;
	vec3_t	forward;

	if (trap_Argc() != 3) {
		G_Printf( "ClientDamage command issued with incorrect number of args\n" );
	}

	trap_Argv( 1, s, sizeof( s ) );
	entnum = atoi(s);
	ent = &g_entities[entnum];

	trap_Argv( 2, s, sizeof( s ) );
	angle = atoi(s);

	// sanity check (also protect from cheaters)
	if (g_gametype.integer != GT_SINGLE_PLAYER && entnum != clent->s.number) {
		trap_DropClient( clent->s.number, "Dropped due to illegal ClientMonsterSlick command\n" );
		return;
	}

	VectorClear (dir);
	dir[YAW] = angle;
	AngleVectors (dir, forward, NULL, NULL);
	
	VectorScale (forward, 32, kvel);
	VectorAdd (ent->client->ps.velocity, kvel, ent->client->ps.velocity);
}
*/

// NERVE - SMF
/*
============
ClientDamage
============
*/
void ClientDamage( gentity_t *clent, int entnum, int enemynum, int id ) {
	gentity_t *enemy, *ent;
	vec3_t vec;

	ent = &g_entities[entnum];

	enemy = &g_entities[enemynum];

	// NERVE - SMF - took this out, this is causing more problems than its helping
	//  Either a new way has to be found, or this check needs to change.
	// sanity check (also protect from cheaters)
//	if (g_gametype.integer != GT_SINGLE_PLAYER && entnum != clent->s.number) {
//		trap_DropClient( clent->s.number, "Dropped due to illegal ClientDamage command\n" );
//		return;
//	}
	// -NERVE - SMF

	// if the attacker can't see the target, then don't allow damage
	if (g_gametype.integer != GT_SINGLE_PLAYER) {
    // TTimo it can happen that enemy->client == NULL
    // see Changelog 09/22/2001
		if ((enemy->client) && (!CanDamage(ent, enemy->client->ps.origin ))) {
			return;	// don't allow damage
		}
	}

	switch (id) {
	case CLDMG_SPIRIT:
		if (g_gametype.integer == GT_SINGLE_PLAYER) {
			G_Damage( ent, enemy, enemy, vec3_origin, vec3_origin, 3, DAMAGE_NO_KNOCKBACK, MOD_ZOMBIESPIRIT );
		}
		break;
	case CLDMG_BOSS1LIGHTNING:
		if (g_gametype.integer != GT_SINGLE_PLAYER) {
			break;
		}
		if ( ent->takedamage ) {
			VectorSubtract( ent->r.currentOrigin, enemy->r.currentOrigin, vec );
			VectorNormalize( vec );
			G_Damage( ent, enemy, enemy, vec, ent->r.currentOrigin, 6 + rand()%3, 0, MOD_LIGHTNING);
		}
		break;
	case CLDMG_TESLA:
		// do some cheat protection
		if (g_gametype.integer != GT_SINGLE_PLAYER) {
			if ( enemy->s.weapon != WP_TESLA )
				break;
			if ( !(enemy->client->buttons & BUTTON_ATTACK) )
				break;
			//if ( AICast_GetCastState( enemy->s.number )->lastWeaponFiredWeaponNum != WP_TESLA )
			//	break;
			//if ( AICast_GetCastState( enemy->s.number )->lastWeaponFired < level.time - 400 )
			//	break;
		}

		if (	(ent->aiCharacter == AICHAR_PROTOSOLDIER) || 
				(ent->aiCharacter == AICHAR_SUPERSOLDIER) || 
				(ent->aiCharacter == AICHAR_LOPER) || 
				(ent->aiCharacter >= AICHAR_STIMSOLDIER1 && ent->aiCharacter <= AICHAR_STIMSOLDIER3) ) {
			break;
		}

		if ( ent->takedamage /*&& !AICast_NoFlameDamage(ent->s.number)*/ ) {
			VectorSubtract( ent->r.currentOrigin, enemy->r.currentOrigin, vec );
			VectorNormalize( vec );
			G_Damage( ent, enemy, enemy, vec, ent->r.currentOrigin, 3, 0, MOD_LIGHTNING);
		}
		break;
	case CLDMG_FLAMETHROWER:
		// do some cheat protection
/*  JPW NERVE pulled flamethrower client damage completely
		if (g_gametype.integer != GT_SINGLE_PLAYER) {
			if ( enemy->s.weapon != WP_FLAMETHROWER )
				break;
//			if ( !(enemy->client->buttons & BUTTON_ATTACK) ) // JPW NERVE flames should be able to damage while puffs are active
//				break;
		} else {
			// this is required for Zombie flame attack
			//if ((enemy->aiCharacter == AICHAR_ZOMBIE) && !AICast_VisibleFromPos( enemy->r.currentOrigin, enemy->s.number, ent->r.currentOrigin, ent->s.number, qfalse ))
			//	break;
		}

		if ( ent->takedamage && !AICast_NoFlameDamage(ent->s.number) ) {
			#define	FLAME_THRESHOLD	50
			int damage = 5;

			// RF, only do damage once they start burning
			//if (ent->health > 0)	// don't explode from flamethrower
			//	G_Damage( traceEnt, ent, ent, forward, tr.endpos, 1, 0, MOD_LIGHTNING);

			// now check the damageQuota to see if we should play a pain animation
			// first reduce the current damageQuota with time
			if (ent->flameQuotaTime && ent->flameQuota > 0) {
				ent->flameQuota -= (int)(((float)(level.time - ent->flameQuotaTime)/1000) * (float)damage/2.0);
				if (ent->flameQuota < 0)
					ent->flameQuota = 0;
			}

			// add the new damage
			ent->flameQuota += damage;
			ent->flameQuotaTime = level.time;

			// Ridah, make em burn
			if (ent->client && ( !(ent->r.svFlags & SVF_CASTAI) || ent->health <= 0 || ent->flameQuota > FLAME_THRESHOLD)) {				if (ent->s.onFireEnd < level.time)
					ent->s.onFireStart = level.time;
				if (ent->health <= 0 || !(ent->r.svFlags & SVF_CASTAI) || (g_gametype.integer != GT_SINGLE_PLAYER)) {
					if (ent->r.svFlags & SVF_CASTAI) {
						ent->s.onFireEnd = level.time + 6000;
					} else {
						ent->s.onFireEnd = level.time + FIRE_FLASH_TIME;
					}
				} else {
					ent->s.onFireEnd = level.time + 99999;	// make sure it goes for longer than they need to die
				}
				ent->flameBurnEnt = enemy->s.number;
				// add to playerState for client-side effect
				ent->client->ps.onFireStart = level.time;
			}
		}
*/
	break;
	}
}
// -NERVE - SMF

/*
============
Cmd_ClientDamage_f
============
*/
void Cmd_ClientDamage_f( gentity_t *clent ) {
	char s[MAX_STRING_CHARS];
	int entnum, id, enemynum;

	if (trap_Argc() != 4) {
		G_Printf( "ClientDamage command issued with incorrect number of args\n" );
	}

	trap_Argv( 1, s, sizeof( s ) );
	entnum = atoi(s);

	trap_Argv( 2, s, sizeof( s ) );
	enemynum = atoi(s);

	trap_Argv( 3, s, sizeof( s ) );
	id = atoi(s);

	ClientDamage( clent, entnum, enemynum, id );
}

/*
==============
Cmd_TouchedEntities_f
==============
*/
void Cmd_TouchedEntityCount_f( gentity_t *ent )
{
	CP(va("print \"touched entities = %i\n\"", level.num_entities));
}

/*
==============
Cmd_LiveEntityCount_f
==============
*/
void Cmd_LiveEntityCount_f(gentity_t *ent)
{
	int i;
	int persistants = 0;
	gentity_t *e;

	for (i = 0; i < level.num_entities; ++i) {
		e = g_entities + i;

		if (!e->inuse) {
			continue;
		}

		persistants++;
	}

	CP(va("print \"live entities = %i\n\"", persistants));
}

// NERVE - SMF
/*
============
Cmd_SetSpawnPoint_f
============
*/
void Cmd_SetSpawnPoint_f( gentity_t *clent ) {
	char	arg[MAX_TOKEN_CHARS];
//	int		spawnIndex;

	if ( trap_Argc() != 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	if (clent->client) // JPW NERVE
		clent->client->sess.spawnObjectiveIndex = atoi( arg ); // JPW NERVE
}
// -NERVE - SMF

/*
============
Cmd_SaveLocation_f
============
*/
void Cmd_SaveLocation_f(gentity_t *ent)
{
	gclient_t *client = ent->client;

	if (g_savedLocations.integer)
	{
		if (trap_Argc() < 2) {
			CP("print \"You must give a name^1!\nUsage: /save <any name>\n\"");
			return;
		}
		if (client->ps.pm_type == PM_DEAD) {
			CP("print \"The dead can't save locations^1!\n\"");
			return;
		}

		char *locationName = ConcatArgs2(1);
		qboolean locationFound = qfalse;
		saved_location_t *savedLocation;

		for (savedLocation = client->pers.savedLocations; strlen(savedLocation->locationName); savedLocation++) {
			if (!Q_stricmp(savedLocation->locationName, locationName)) {
				locationFound = qtrue;
				break;
			}
		}

		if (!locationFound) {
			if (client->pers.savedLocationCount == MAX_SAVED_LOCATIONS)
				client->pers.savedLocationCount = 0;
			savedLocation = client->pers.savedLocations + client->pers.savedLocationCount++;
		}

		VectorCopy(client->ps.origin, savedLocation->location);
		VectorCopy(client->ps.viewangles, savedLocation->viewangles);
		Q_strncpyz(savedLocation->locationName, locationName, sizeof(savedLocation->locationName));

		CP(va("print \"Location ^3%s ^7saved successfully\n\"", locationName));
	}
}

/*
============
Cmd_LoadLocation_f
============
*/
void Cmd_LoadLocation_f(gentity_t *ent)
{
	gclient_t *client = ent->client;

	if (g_savedLocations.integer)
	{
		if (trap_Argc() < 2) {
			CP("print \"You must give a name of a saved location^1!\nUsage: /load <name of saved location>\n\"");
			return;
		}

		char *locationName = ConcatArgs2(1);
		saved_location_t *savedLocation;
		qboolean locationFound = qfalse;

		for (savedLocation = client->pers.savedLocations; strlen(savedLocation->locationName); savedLocation++) {
			if (!Q_stricmp(savedLocation->locationName, locationName)) {
				locationFound = qtrue;
				break;
			}
		}

		if (!locationFound) {
			CP(va("print \"Location ^3%s ^7could not be found^1!\n\"", locationName));
			return;
		}

		if (client->ps.pm_type == PM_DEAD && !(client->ps.pm_flags & (PMF_FOLLOW |PMF_LIMBO | PMF_TIME_LOCKPLAYER)))
			ent->health = client->ps.stats[STAT_MAX_HEALTH];

		TeleportPlayer(ent, savedLocation->location, savedLocation->viewangles);

		CP(va("print \"Location ^3%s ^7loaded successfully\n\"", locationName));
	}
}

/*
============
Cmd_DisplayMaps_f
============
*/
void Cmd_DisplayMaps_f(gentity_t *ent)
{
	char maps[MAX_CONFIGSTRINGS];
	char noext[MAX_QPATH];
	int i, mapcount;

	mapcount = trap_FS_GetFileList("maps", ".bsp", maps, sizeof(maps));
	char *map = maps;

	for (i = 0; i < mapcount; ++i) {
		COM_StripExtension(map, noext);
		CP(va("print \"^3%s\n\"", noext));
		map += strlen(map) + 1;
	}
}

void Cmd_DisplayDailyRanking_f(gentity_t *ent)
{
	if (!g_dailystats.integer) {
		CP("print \"Daily stats are currently disabled\n\"");
		return;
	}

	stats_DisplayDailyRanking(ent);
}

void Cmd_DisplayDailyStats_f(gentity_t *ent)
{
	if (!g_dailystats.integer) {
		CP("print \"Daily stats are currently disabled\n\"");
		return;
	}

	stats_DisplayDailyStats(ent);
}

/*
===========
Login
===========
*/
void cmd_login(gentity_t *ent) {
	char arg1[MAX_TOKEN_CHARS];
	char str[MAX_TOKEN_CHARS];
	char arg3[MAX_TOKEN_CHARS];
	qboolean silent = qfalse;
	char *log;

	trap_Argv(1, str, sizeof(str));

	// Make sure user is not already logged in.
	if (ent->client->sess.admin != ADM_NONE) {
		CP("print \"You are already logged in^1!\n\"");
		return;
	}

	// Prevent bogus logins	
	if ((!Q_stricmp(str, "\0"))
		|| (!Q_stricmp(str, ""))
		|| (!Q_stricmp(str, "\""))
		|| (!Q_stricmp(str, "none")))
	{
		CP("print \"Incorrect password^1!\n\"");
		// No log here to avoid login by error..
		return;
	}

	// Always start with lower level as if owner screws it up 
	// and sets the same passes for more levels, the lowest is the safest bet.
	if (Q_stricmp(str, a1_pass.string) == 0) {
		ent->client->sess.admin = ADM_1;
	}
	else if (Q_stricmp(str, a2_pass.string) == 0) {
		ent->client->sess.admin = ADM_2;
	}
	else if (Q_stricmp(str, a3_pass.string) == 0) {
		ent->client->sess.admin = ADM_3;
	}
	else if (Q_stricmp(str, a4_pass.string) == 0) {
		ent->client->sess.admin = ADM_4;
	}
	else if (Q_stricmp(str, a5_pass.string) == 0) {
		ent->client->sess.admin = ADM_5;
	}
	else {
		// User shouldn't be anything but regular so make sure..
		ent->client->sess.admin = ADM_NONE;
		CP("print \"Incorrect password^1!\n\"");

		// Log it
		log = va("Time: %s\nPlayer %s (IP: %s) has tried to login using password: %s%s",
			getDateTime(), ent->client->pers.netname, clientIP(ent, qtrue), str, LOGLINE);

		if (g_extendedLog.integer)
			trap_LogToFile(PASSLOG, log);

		return;
	}

	trap_Argv(0, arg1, sizeof(arg1));
	if (arg1[0] == '@') {
		silent = qtrue;
	}

	if (trap_Argc() == 3) {
		trap_Argv(2, arg3, sizeof(arg3));
		if (Q_stricmp(arg3, "s") == 0) {
			silent = qtrue;
		}
	}

	// We came so far so go with it..
	if (silent) {
		CP("print \"Silent login successful^2!\n\"");
		ent->client->sess.incognito = 1; // Hide them

		// Log it
		log = va("Time: %s\nPlayer %s (IP: %s) has silently logged in as %s.%s",
			getDateTime(),
			ent->client->pers.netname,
			clientIP(ent, qtrue),
			getTag(ent),
			LOGLINE);

		if (g_extendedLog.integer)
			trap_LogToFile(ADMLOG, log);
	}
	else {
		AP(va("chat \"^7console: %s ^7logged in as %s^7!\n\"", ent->client->pers.netname, getMessage(ent)));

		// Log it
		log = va("Time: %s\nPlayer %s (IP: %s) logged in as %s.%s",
			getDateTime(), ent->client->pers.netname, clientIP(ent, qtrue), getTag(ent), LOGLINE);

		if (g_extendedLog.integer)
			trap_LogToFile(ADMLOG, log);
	}
	return;
}

/*
===========
Logout
===========
*/
void cmd_logout(gentity_t *ent) {
	// If user is not logged in do nothing
	if (ent->client->sess.admin == ADM_NONE) {
		return;
	}
	else {
		// Admin is hidden so don't print 
		if (ent->client->sess.incognito)
			CP("print \"You have successfully logged out^2!\n\"");
		else
			AP(va("chat \"console: %s ^7logged out!\n\"", ent->client->pers.netname));

		if (ent->client->sess.secretlyDemoing)
			cmd_demoing(ent);

		// Log them out now
		ent->client->sess.admin = ADM_NONE;

		// Set incognito to visible..
		ent->client->sess.incognito = 0;

		return;
	}
}

/*
===========
Toggle incognito
===========
*/
void cmd_incognito(gentity_t *ent) {
	if (ent->client->sess.admin == ADM_NONE)
		return;

	if (ent->client->sess.incognito == 0) {
		ent->client->sess.incognito = 1;
		CP("cp \"You are now incognito^2!\n\"2");
		return;
	}
	else {
		ent->client->sess.incognito = 0;
		CP("cp \"Your status is now set to visible^2!\n\"2");
		return;
	}
}

void Cmd_SetMp40_f(gentity_t *ent)
{
	if (!g_customMGs.integer)
	{
		CP("cp \"Custom MGs are ^3disabled ^7on this server!\n\"2");
		return;
	}

	if (ent->client->ps.stats[STAT_PLAYER_CLASS] == PC_SOLDIER ||
		ent->client->ps.stats[STAT_PLAYER_CLASS] == PC_LT) {
		CP("cp \"Only Medics and Engineers can choose a custom SMG!\n\"2");
		return;
	}

	if (ent->client->sess.selectedWeapon == WP_MP40)
	{
		ent->client->sess.selectedWeapon =
			(ent->client->sess.sessionTeam == TEAM_RED) ? WP_MP40 : WP_THOMPSON;
		CP("cp \"You will spawn with the default weapon!\n\"2");
	}
	else {
		CP("cp \"You will spawn with an ^3mp40^7!\n\"2");
		ent->client->sess.selectedWeapon = WP_MP40;
	}
}

void Cmd_SetThompson_f(gentity_t *ent)
{
	if (!g_customMGs.integer)
	{
		CP("cp \"Custom MGs are ^3disabled ^7on this server!\n\"2");
		return;
	}

	if (ent->client->ps.stats[STAT_PLAYER_CLASS] == PC_SOLDIER ||
		ent->client->ps.stats[STAT_PLAYER_CLASS] == PC_LT) {
		CP("cp \"Only Medics and Engineers can choose a custom SMG!\n\"2");
		return;
	}

	if (ent->client->sess.selectedWeapon == WP_THOMPSON)
	{
		ent->client->sess.selectedWeapon =
			(ent->client->sess.sessionTeam == TEAM_RED) ? WP_MP40 : WP_THOMPSON;
		CP("cp \"You will spawn with default weapon!\n\"2");
	}
	else {
		CP("cp \"You will spawn with a ^3thompson^7!\n\"2");
		ent->client->sess.selectedWeapon = WP_THOMPSON;
	}
}

void Cmd_SetSten_f(gentity_t *ent)
{
	if (!g_customMGs.integer)
	{
		CP("cp \"Custom MGs are ^3disabled ^7on this server!\n\"2");
		return;
	}

	if (ent->client->ps.stats[STAT_PLAYER_CLASS] == PC_SOLDIER ||
		ent->client->ps.stats[STAT_PLAYER_CLASS] == PC_LT) {
		CP("cp \"Only Medics and Engineers can choose a custom SMG!\n\"2");
		return;
	}

	if (ent->client->sess.selectedWeapon == WP_STEN)
	{
		ent->client->sess.selectedWeapon =
			(ent->client->sess.sessionTeam == TEAM_RED) ? WP_MP40 : WP_THOMPSON;
		CP("cp \"You will spawn with the default weapon!\n\"2");
	}
	else {
		CP("cp \"You will spawn with a ^3sten^7!\n\"2");
		ent->client->sess.selectedWeapon = WP_STEN;
	}
}

void Cmd_More_f(gentity_t *ent)
{
	if (ent->more) {
		if (!ent->moreCalls)
			ent->moreCalls = 1;

		ent->moreCalled = qtrue;
		ent->more(ent);
		ent->moreCalls++;
		ent->moreCalled = qfalse;
	}
}

void Cmd_Mute_f(gentity_t *ent)
{
	char muteNumberStr[MAX_TOKEN_CHARS];
	gentity_t *muteEnt;

	trap_Argv(1, muteNumberStr, sizeof(muteNumberStr));
	if (GetClientEntity(ent, muteNumberStr, &muteEnt) == NULL)
	{
		return;
	}

	COM_BitSet(ent->client->sess.muted, muteEnt - g_entities);

	CP(va("print \"%-2s: %s\n\"", muteNumberStr, muteEnt->client->pers.netname));
}

void Cmd_Unmute_f(gentity_t *ent)
{
	char muteNumberStr[MAX_TOKEN_CHARS];
	gentity_t *muteEnt;

	trap_Argv(1, muteNumberStr, sizeof(muteNumberStr));
	if (GetClientEntity(ent, muteNumberStr, &muteEnt) == NULL)
	{
		return;
	}

	COM_BitClear(ent->client->sess.muted, muteEnt - g_entities);

	CP(va("print \"%-2s: unmuted\n\"", muteNumberStr, muteEnt->client->pers.netname));
}

void Cmd_PrintMuted_f(gentity_t *ent)
{
	int i;
	qboolean someone_is_muted;

	someone_is_muted = qfalse;

	for (i = 0; i < level.maxclients; ++i)
	{
		gentity_t *muted = g_entities + i;

		if (!muted->inuse || muted->client->pers.connected != CON_CONNECTED)
		{
			continue;
		}

		if (COM_BitCheck(ent->client->sess.muted, i))
		{
			CP(va("print \"%-2d: %s\n\"", i, muted->client->pers.netname));
			someone_is_muted = qtrue;
		}
	}

	if (!someone_is_muted) {
		CP("print \"...\n\"");
	}
}

void Cmd_MuteAll_f(gentity_t *ent)
{
	memset(ent->client->sess.muted, 0xFFFFFFFF, sizeof(ent->client->sess.muted));
	Cmd_PrintMuted_f(ent);
}

void Cmd_UnmuteAll_f(gentity_t *ent)
{
	memset(ent->client->sess.muted, 0, sizeof(ent->client->sess.muted));
	Cmd_PrintMuted_f(ent);
}

void Cmd_DropReload_f(gentity_t *ent)
{
	CP("print \"\n/bind <key> ~+dropweapon; -dropweapon; +activate; -activate; -activate~\n\n\"");
}

extern vec3_t default_base_mins, default_base_maxs;

void TryDroppingAutomg42(gentity_t *ent, float scale)
{
	gentity_t *base;
	trace_t tr;
	vec3_t velocity, offset, client_angles, end, mins, maxs;

	if (g_automg42.integer <= 0 ||
		ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
		ent->client->ps.stats[STAT_HEALTH] <= 0 ||
		ent->client->ps.pm_flags & PMF_LIMBO)
	{
		return;
	}

	if (ent->client->ps.stats[STAT_PLAYER_CLASS] != PC_ENGINEER)
	{
		CP("cp \"Engineer? Nope.\n\"");
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_RED)
	{
		if (level.axis_automg42_count >= g_automg42.integer)
		{
			CP("cp \"Maximum amount of automg42s reached.\n\"");
			return;
		}
	}
	else if (ent->client->sess.sessionTeam == TEAM_BLUE)
	{
		if (level.allies_automg42_count >= g_automg42.integer)
		{
			CP("cp \"Maximum amount of automg42s reached.\n\"");
			return;
		}
	}
	else
	{
		return;
	}

	if (level.time - ent->client->ps.classWeaponTime < (g_engineerChargeTime.integer * scale))
	{
		CP("cp \"You need at-least half a charge.\n\"");
		return;
	}

	VectorCopy(ent->client->ps.viewangles, client_angles);

	AngleVectors(client_angles, velocity, NULL, NULL);
	VectorScale(velocity, 64, offset);
	VectorScale(velocity, 50, velocity);

	VectorAdd(ent->client->ps.origin, offset, end);

	// NOTE(nobo): These need to match the gun + tripod max/mins in order to properly launch weapon
	VectorCopy(default_base_mins, mins);
	VectorCopy(default_base_maxs, maxs);
	VectorScale(default_base_mins, scale, mins);
	VectorScale(default_base_maxs, scale, maxs);

	trap_Trace(&tr, ent->client->ps.origin, mins, maxs, end, ent->s.number, MASK_PLAYERSOLID | CONTENTS_ITEM);

	if (tr.fraction < 1.0f)
	{
		CP("cp \"There's something in the way!\n\"");
		return;
	}

	VectorCopy(tr.endpos, end);
	trap_Trace(&tr, end, mins, maxs, end, ENTITYNUM_NONE, MASK_PLAYERSOLID | CONTENTS_ITEM);

	if (tr.fraction < 1.0f)
	{
		CP("cp \"There's something in the way!\n\"");
		return;
	}

	base = G_Spawn();
	base->parent = ent;
	base->think = automg42_spawn;
	base->nextthink = level.time + FRAMETIME;
	base->damage = MG42_DAMAGE * scale;
	base->accuracy = 1;
	base->count2 = g_automg42Health.integer * scale; // max health
	base->radius = g_automg42LightIntensity.integer * scale; // constant light intensity
	base->aiTeam = ent->client->sess.sessionTeam;
	base->health = 0;
	VectorSet(base->s.angles, 0, random() * 360, 0);
	VectorSet(base->s.angles2, scale, scale, scale);
	VectorCopy(end, base->s.origin);
	VectorCopy(velocity, base->s.pos.trDelta);

	if (level.time - ent->client->ps.classWeaponTime > g_engineerChargeTime.integer)
	{
		ent->client->ps.classWeaponTime = level.time - g_engineerChargeTime.integer;
	}

	ent->client->ps.classWeaponTime += g_engineerChargeTime.integer * scale;
}

void Cmd_Automg42_f(gentity_t *ent)
{
	TryDroppingAutomg42(ent, 1.0f);
}

void Cmd_MiniAutomg42_f(gentity_t *ent)
{
	TryDroppingAutomg42(ent, 0.5f);
}

void Cmd_DropNadePack_f(gentity_t *ent)
{
	gitem_t *nade_item;
	gentity_t *nade_entity;
	vec3_t velocity, origin, offset;
	vec3_t mins, maxs;
	trace_t	tr;

	if (g_automg42.integer <= 0 ||
		ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
		ent->client->ps.stats[STAT_HEALTH] <= 0 ||
		ent->client->ps.pm_flags & PMF_LIMBO)
	{
		return;
	}

	if (ent->client->ps.stats[STAT_PLAYER_CLASS] != PC_ENGINEER)
	{
		CP("cp \"Engineer? Nope.\n\"");
		return;
	}

	if (level.time - ent->client->ps.classWeaponTime < g_engineerChargeTime.integer * 0.75f)
	{
		CP("cp \"Not enough charge.\n\"");
		return;
	}

	if (level.time - ent->client->ps.classWeaponTime > g_engineerChargeTime.integer)
	{
		ent->client->ps.classWeaponTime = level.time - g_engineerChargeTime.integer;
	}

	ent->client->ps.classWeaponTime += g_engineerChargeTime.integer * 0.25;

	if (ent->client->sess.sessionTeam == TEAM_RED)
	{
		nade_item = BG_FindItem("Grenades");
	}
	else
	{
		nade_item = BG_FindItem("Pineapples");
	}

	AngleVectors(ent->client->ps.viewangles, velocity, NULL, NULL);
	VectorScale(velocity, 64, offset);
	offset[2] += ent->client->ps.viewheight / 2;
	VectorScale(velocity, 75, velocity);
	velocity[2] += 50 + crandom() * 25;

	VectorAdd(ent->client->ps.origin, offset, origin);

	VectorSet(mins, -ITEM_RADIUS, -ITEM_RADIUS, 0);
	VectorSet(maxs, ITEM_RADIUS, ITEM_RADIUS, 2 * ITEM_RADIUS);

	trap_Trace(&tr, ent->client->ps.origin, mins, maxs, origin, ent->s.number, MASK_SOLID);
	VectorCopy(tr.endpos, origin);

	nade_entity = LaunchItem(nade_item, origin, velocity, ent->s.number);
	nade_entity->think = MagicSink;
	nade_entity->timestamp = level.time + 31200;
	nade_entity->parent = NULL;
	nade_entity->count = 4; // TODO(nobo): cvar?
}

void AppendString(char *to, int to_size, char *appendage)
{
	if (strlen(to))
	{
		Q_strcat(to, to_size, va("\n%s", appendage));
	}
	else
	{
		Q_strncpyz(to, appendage, to_size);
	}
}

void Cmd_ColorFlags_f(gentity_t *ent)
{
#define MAX_ARGUMENT_LENGTH 64

	char argument[MAX_ARGUMENT_LENGTH];
	char output[MAX_STRING_CHARS] = { 0 };
	int i;
	int argument_count = trap_Argc();

	if (argument_count == 1)
	{
		trap_Argv(0, argument, MAX_ARGUMENT_LENGTH);
		int cFlags = ent->client->sess.colorFlags;

		AppendString(output, sizeof(output), cFlags & CF_AUTORELOAD_OFF ? "AUTORELOAD ^1OFF^7" : "AUTORELOAD ^2ON^7");
		AppendString(output, sizeof(output), cFlags & CF_HITSOUNDS_OFF ? "HITSOUNDS ^1OFF^7" : "HITSOUNDS ^2ON^7");
		AppendString(output, sizeof(output), cFlags & CF_MULTIKILL_OFF ? "MULTIKILL ^1OFF^7" : "MULTIKILL ^2ON^7");
		AppendString(output, sizeof(output), cFlags & CF_ANTILAG_OFF ? "ANTILAG ^1OFF^7" : "ANTILAG ^2ON^7");
		AppendString(output, sizeof(output), va("DEFAULT WEAPON %s^7", cFlags & CF_USE_STEN ? "^5STEN" : cFlags & CF_USE_THOMPSON ? "^4THOMPSON" : "^0MP40"));

		if (strlen(output) == 0)
		{
			AppendString(output, sizeof(output), "No colorflags are enabled.");
		}

		AppendString(output, sizeof(output), va("\n^3Flags you can give the /%s command:^7", argument));
		AppendString(output, sizeof(output), "1 = Turn autoreload off");
		AppendString(output, sizeof(output), "2 = Turn hitsounds off");
		AppendString(output, sizeof(output), "3 = Turn multi-kill off");
		AppendString(output, sizeof(output), "4 = Set default weapon to mp40");
		AppendString(output, sizeof(output), "5 = Set default weapon to thompson");
		AppendString(output, sizeof(output), "6 = Set default weapon to sten");
		AppendString(output, sizeof(output), "7 = Turn antilag off");
		AppendString(output, sizeof(output), "\n^3Usage:^7");
		AppendString(output, sizeof(output), va("/%s <flag 1> <flag 2> <etc..>", argument));
		AppendString(output, sizeof(output), "\nDoing this will give you your desired /color number.\n");

		CP(va("print \"\n%s\n\"", output));

		return;
	}

	int result = 0;
	for (i = 1; i < argument_count; ++i)
	{
		trap_Argv(i, argument, MAX_ARGUMENT_LENGTH);
		int flag = atoi(argument);
		switch (flag)
		{
		case 1:
			result |= CF_AUTORELOAD_OFF;
			break;
		case 2:
			result |= CF_HITSOUNDS_OFF;
			break;
		case 3:
			result |= CF_MULTIKILL_OFF;
			break;
		case 4:
			result |= CF_USE_MP40;
			break;
		case 5:
			result |= CF_USE_THOMPSON;
			break;
		case 6:
			result |= CF_USE_STEN;
			break;
		case 7:
			result |= CF_ANTILAG_OFF;
			break;
		default: 
			break;
		}
	}

	CP(va("print \"/color %d\n\"", result));
}

void Cmd_ToggleDrawHitBoxes_f(gentity_t *ent) {
	if (!g_drawHitboxes.integer) {
		CP("cp \"g_drawHitboxes is disabled.\n\"");
		return;
	}

	ent->client->pers.drawHitBoxes = !ent->client->pers.drawHitBoxes;
}

typedef struct fps_match {
	int client_num;
	int average_msec;
	int fps;
} fps_match_t;

int CompareFpsMatches(const void *fps_match1, const void *fps_match2) {
	return ((fps_match_t *)fps_match1)->average_msec - ((fps_match_t *)fps_match2)->average_msec;
}

void Cmd_DisplayFps_f(gentity_t *ent) {
	gentity_t *entity;
	int i, client_num, fps_matches_count;
	fps_match_t fps_matches[MAX_CLIENTS];
	fps_match_t *fps_match;

	fps_matches_count = 0;

	for (i = 0; i < level.numConnectedClients; ++i) {
		client_num = level.sortedClients[i];
		entity = g_entities + client_num;
		fps_match = fps_matches + fps_matches_count++;
		fps_match->client_num = client_num;
		fps_match->average_msec = ClientGetMsec(&entity->client->ps);
		fps_match->fps = ClientGetFps(&entity->client->ps);
	}

	CP("print \"\n^3-----------------------------\n\"");
	CP("print \"^7Name            : ^3Fps ^7: Msec \n\"");
	CP("print \"^3-----------------------------\n\"");

	if (fps_matches_count > 0) {
		qsort(fps_matches, fps_matches_count, sizeof(fps_match_t), CompareFpsMatches);
		for (i = 0; i < fps_matches_count; ++i) {
			fps_match = fps_matches + i;
			entity = g_entities + fps_match->client_num;
			CP(va("print \"%s^7 : ^3%-3d ^7: %-3d\n\"", TablePrintableColorName(entity->client->pers.netname, 15), fps_match->fps, fps_match->average_msec));
		}
	}

	CP("print \"^3-----------------------------\n\"");
}

void Cmd_DisplayServerUptime_f(gentity_t *ent) {
	int seconds_elapsed = level.time / 1000;
	char *time_message = GetTimeMessage(seconds_elapsed);
	CP(va("chat \"%s^7, the server has been online for ^3%s\"", ent->client->pers.netname, time_message));
	CPS(ent, "sound/multiplayer/dynamite_01.wav");
}

void Cmd_GiveLife_f(gentity_t *ent) {
	char argument[ADMIN_ARG_SIZE];
	int i;

	if (trap_Argc() < 2) {
		// TODO: Automatically choose someone on their team to give a life to.
		// Possibly sort/choose teammate based on:
		//	1. Are they dead?
		//	2. Daily stats rank
		//	3. Round score
		CP("cp \"Please provide a player name.\n\"");
		return;
	}

	if (!g_giveLife.integer) {
		CP("cp \"Giving lives is disabled.\n\"");
		return;
	}

	if (!g_maxlives.integer) {
		CP("cp \"g_maxlives is disabled.\n\"");
		return;
	}

	if (ent->client->sess.sessionTeam != TEAM_RED &&
		ent->client->sess.sessionTeam != TEAM_BLUE) {
		CP("cp \"You need to be playing to give lives.\n\"");
		return;
	}

	if (ent->client->ps.persistant[PERS_RESPAWNS_LEFT] < 0) {
		CP("cp \"You're out of lives to give.\n\"");
		return;
	}

	if (ent->client->ps.persistant[PERS_RESPAWNS_LEFT] == 0 &&
		ent->client->ps.pm_flags & PMF_LIMBO) {
		CP("cp \"You're out of lives to give.\n\"");
		return;
	}

	qboolean is_number = qtrue;
	trap_Argv(1, argument, sizeof(argument));
	for (i = 0; i < strlen(argument); ++i) {
		if (!isdigit(argument[i])) {
			is_number = qfalse;
			break;
		}
	}

	int target_client_num;
	if (is_number) {
		target_client_num = atoi(argument);
		if (target_client_num < 0 || target_client_num >= MAX_CLIENTS) {
			CP(va("cp \"^3%d ^7is an invalid client number.\n\"", target_client_num));
			return;
		}
		qboolean does_client_num_match_connected_client = qfalse;
		for (i = 0; i < level.numConnectedClients; ++i) {
			if (target_client_num == level.sortedClients[i]) {
				does_client_num_match_connected_client = qtrue;
				break;
			}
		}
		if (!does_client_num_match_connected_client) {
			CP(va("cp \"Couldn't find anyone with client number ^3%d^7.\n\"", target_client_num));
			return;
		}
	}
	else {
		target_client_num = ClientNumberFromName(ent, argument, qfalse);
		if (target_client_num == -1) {
			CP(va("cp \"Couldn't find anyone with %s ^7in their name.\n\"", argument));
			return;
		}
	}

	gentity_t *target_entity = g_entities + target_client_num;

	if (ent == target_entity) {
		CP("cp \"You can't give lives to yourself. Nice try.\n\"");
		return;
	}

	if (ent->client->sess.sessionTeam != target_entity->client->sess.sessionTeam) {
		CP("cp \"You can only give lives to teammates.\n\"");
		return;
	}

	qboolean has_max_lives = qfalse;
	switch (target_entity->client->sess.sessionTeam) {
	case TEAM_BLUE:
		if (g_alliedmaxlives.integer) {
			if (target_entity->client->ps.persistant[PERS_RESPAWNS_LEFT] >= g_alliedmaxlives.integer - 1) {
				has_max_lives = qtrue;
			}
		}
		else if (g_maxlives.integer) {
			if (target_entity->client->ps.persistant[PERS_RESPAWNS_LEFT] >= g_maxlives.integer - 1) {
				has_max_lives = qtrue;
			}
		}
		break;
	case TEAM_RED:
		if (g_axismaxlives.integer) {
			if (target_entity->client->ps.persistant[PERS_RESPAWNS_LEFT] >= g_axismaxlives.integer - 1) {
				has_max_lives = qtrue;
			}
		}
		else if (g_maxlives.integer) {
			if (target_entity->client->ps.persistant[PERS_RESPAWNS_LEFT] >= g_maxlives.integer - 1) {
				has_max_lives = qtrue;
			}
		}
		break;
	}

	if (has_max_lives) {
		CP(va("cp \"%s ^7has the maximum amount of lives already.\n\"", target_entity->client->pers.netname));
		return;
	}

	if (Team_CountLiveTeammates(ent)) {
		if (g_giveLifeRequiredDamage.integer && ent->client->pers.give_life_damage < g_giveLifeRequiredDamage.integer) {
			int damage_required = g_giveLifeRequiredDamage.integer - ent->client->pers.give_life_damage;
			CP(va("cp \"You must do ^3%d ^7more damage to enemies.\n\"", damage_required));
			return;
		}
		if (g_giveLifeRequiredRevives.integer && ent->client->pers.give_life_revives < g_giveLifeRequiredRevives.integer) {
			int revives_required = g_giveLifeRequiredRevives.integer - ent->client->pers.give_life_revives;
			CP(va("cp \"You must revive ^3%d ^7teammate%s.\n\"", revives_required, revives_required > 1 ? "s" : ""));
			return;
		}
	}
	
	if (ent->client->ps.persistant[PERS_RESPAWNS_LEFT] == 0) {
		Cmd_Gib_f(ent);
		AP(va("chat \"%s^7 gave %s last life to %s^7!\n\"", ent->client->pers.netname, ent->client->sess.gender == 0 ? "his" : "her", target_entity->client->pers.netname));
	}
	else {
		AP(va("chat \"%s^7 gave a life to %s^7!\n\"", ent->client->pers.netname, target_entity->client->pers.netname));
		ent->client->ps.persistant[PERS_RESPAWNS_LEFT]--;
	}

	if (target_entity->client->ps.persistant[PERS_RESPAWNS_LEFT] == 0 &&
		target_entity->client->ps.pm_flags & PMF_LIMBO) {
		respawn(target_entity);
	}
	else {
		target_entity->client->ps.persistant[PERS_RESPAWNS_LEFT]++;
	}

	ent->client->pers.give_life_revives -= g_giveLifeRequiredRevives.integer;
	ent->client->pers.give_life_damage -= g_giveLifeRequiredDamage.integer;
}

#ifdef _DEBUG
void Cmd_Test_f(gentity_t *ent)
{
	vec3_t mins;
	vec3_t maxs;
	vec3_t range = { 200, 200, 200 };
	int hitCount, i;
	int entNumList[1024];
	gentity_t *entHit;

	VectorSubtract(ent->client->ps.origin, range, mins);
	VectorAdd(ent->client->ps.origin, range, maxs);

	hitCount = trap_EntitiesInBox(mins, maxs, entNumList, MASK_ALL);

	for (i = 0; i < hitCount; i++) {
		entHit = &g_entities[entNumList[i]];

		if (!entHit->inuse)
			continue;
		if (entHit == ent)
			continue;

		G_Printf("Classname: %s\n---------\n", entHit->classname && entHit->classname[0] ? entHit->classname : "None");
	}
}

void Cmd_Test2_f(gentity_t *ent)
{
	vec3_t end, forward;
	trace_t trace;
	gentity_t *hitEnt;
	char *contents = "";

	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);
	VectorMA(ent->client->ps.origin, 100, forward, end);
	
	trap_Trace(&trace, ent->client->ps.origin, NULL, NULL, end, ent->s.number, MASK_ALL);
	hitEnt = g_entities + trace.entityNum;

	if (hitEnt->r.contents & CONTENTS_BODY)
	{
		contents = va("%sCONTENTS_BODY", strlen(contents) ? va("%s + ", contents) : "");
	}
	if (hitEnt->r.contents & CONTENTS_CORPSE)
	{
		contents = va("%sCONTENTS_CORPSE", strlen(contents) ? va("%s + ", contents) : "");
	}
	if (hitEnt->r.contents & CONTENTS_SOLID)
	{
		contents = va("%sCONTENTS_SOLID", strlen(contents) ? va("%s + ", contents) : "");
	}
	if (hitEnt->r.contents & CONTENTS_PLAYERCLIP)
	{
		contents = va("%sCONTENTS_PLAYERCLIP", strlen(contents) ? va("%s + ", contents) : "");
	}
	if (hitEnt->r.contents & CONTENTS_WATER)
	{
		contents = va("%sCONTENTS_WATER", strlen(contents) ? va("%s + ", contents) : "");
	}

	if (!strlen(contents))
	{
		contents = va("%d", hitEnt->r.contents);
	}

	G_Printf("%s%s\n", hitEnt->classname && strlen(hitEnt->classname) ? va("Classname: %s ", hitEnt->classname) : "",
		strlen(contents) ? va("Contents: %s", contents) : "");
}

void Cmd_Test3_f(gentity_t *ent)
{
	char files[MAX_CONFIGSTRINGS];
	char noext[MAX_QPATH];
	int i, filecount;

	filecount = trap_FS_GetFileList("", ".cfg", files, sizeof(files));
	char *file = files;

	for (i = 0; i < filecount; ++i) {
		COM_StripExtension(file, noext);
		CP(va("print \"^3%s\n\"", noext));
		file += strlen(file) + 1;
	}
}
#endif

typedef struct {
	char *names;
	char *help;
	char *usage;
	void(*execute) (gentity_t *ent);
	qboolean intermission;
} player_command_t;

static const player_command_t playerCommands[] = {
	{ "givelife gl", "Gives a life to the target player", "/gl <player name>", Cmd_GiveLife_f, qfalse },
	{ "fps", "Gives a rough estiamte of each player's FPS", "/fps", Cmd_DisplayFps_f, qfalse },
	{ "colorflags cf", "Displays which color flags are enabled", "/colorflags", Cmd_ColorFlags_f, qfalse },
	{ "nadepack np", "Drops 4 nade packs.", "/np", Cmd_DropNadePack_f, qfalse },
	{ "mini_automg42 mini", "Drops a mini automg42 on the ground. Must be an engineer, and g_automg42 > 0.", "/mini", Cmd_MiniAutomg42_f, qfalse },
	{ "automg42", "Drops an automg42 on the ground. Must be an engineer, and g_automg42 > 0.", "/automg42", Cmd_Automg42_f, qfalse },
	{ "dropreload dr", "Shows the drop reload script in console", "/dropreload", Cmd_DropReload_f, qtrue },
	{ "mute", "Mutes client number provided until you leave the server.", "/mute <client number>", Cmd_Mute_f, qfalse },
	{ "unmute", "Unmutes client number provided", "/unmute <client number>", Cmd_Unmute_f, qfalse },
	{ "muted", "Displays a list of clients you have muted", "/muted", Cmd_PrintMuted_f, qfalse },
	{ "mute_all", "Mutes every client number (0-63)", "", Cmd_MuteAll_f, qfalse },
	{ "unmute_all", "Unmutes every muted client number", "", Cmd_UnmuteAll_f, qfalse },
	{ "commands", "Displays a list of server commands", "", Cmd_ListCommands_f, qtrue },
	{ "help!", "Displays help documentation for a command", "/help! <command>", Cmd_HelpDocs_f, qtrue },
	{ "dailyrank da", "Displays your daily rank", "/dailyrank", Cmd_DisplayDailyRanking_f, qfalse },
	{ "dailystats ds", "Displays today's stats", "/dailystats", Cmd_DisplayDailyStats_f, qfalse },
	{ "login @login", "Logs you in as an Administrator of the server. @login is silent.", "/login or /@login", cmd_login, qtrue },
	{ "logout", "Logs you out and removes Administrator status", "/logout", cmd_logout, qtrue },
	{ "getstatus gs", "Displays information about each player in the server, such as name and team", "", Cmd_GetStatus_f, qtrue },
	{ "stats s", "Displays your stats for the current round", "", Cmd_Stats_f, qtrue },
	{ "say_team", "Sends a message to all players on your team", "/say_team <message>", Cmd_SayTeam_f, qtrue },
	{ "savelocation save", "Saves your current location for future 'loading'", "/save <any name>", Cmd_SaveLocation_f, qfalse },
	{ "loadlocation load", "Loads a previously saved location", "/load <name of location>", Cmd_LoadLocation_f, qfalse },
	{ "maps", "Displays a list of maps supported by the server", "", Cmd_DisplayMaps_f, qfalse },
	{ "say", "Sends a message to everyone on the server", "/say <message>", Cmd_SayAll_f, qtrue },
	{ "say_limbo", "Sends a message to everyone on the server", "/say_limbo <message>", Cmd_SayLimbo_f, qtrue },
	{ "say_admin", "Sends a message to all Admins on the server", "/say_admin <message>", Cmd_SayAdmin_f, qtrue },
	{ "vsay", "Sends a voice message to everyone on the server", "/vsay <voice chat identifier>", Cmd_VoiceAll_f, qtrue },
	{ "vsay_team", "Sends a voice message to your teammates", "/vsay_team <voice chat identifier>", Cmd_VoiceTeam_f, qtrue },
	{ "tell", "Sends a private message to a specific client number", "/tell <client number> <message>", Cmd_Tell_f, qtrue },
	{ "score", "Shows scoreboard", "", Cmd_Score_f, qtrue },
	{ "team", "Sets your team and any other applicable properties", "/team <s/spectator/b/blue/r/red> <player type> <weapon> <pistol> <grenade> <skin>", Cmd_Team_f, qtrue },
	{ "incognito", "Hides your admin status (must be logged in)", "", cmd_incognito, qtrue },
	{ "msg pm m w whisper", "Sends a private message to given player(s)", "/msg <name> <message>", Cmd_SendPrivateMessage_f, qtrue },
	{ "give", "Gives items, such as grenades, to yourself (cheat)", "/give <all/health/weapons/holdable/ammo/allammo/armor/keys>", Cmd_Give_f, qfalse },
	{ "god", "Makes you invulnerable (cheat)", "", Cmd_God_f, qfalse },
	{ "nofatigue", "Removes fatigue (cheat)", "", Cmd_Nofatigue_f, qfalse },
	{ "notarget", "Makes you invisible (cheat)", "", Cmd_Notarget_f, qfalse },
	{ "noclip", "Allows you to move through anything (cheat)", "", Cmd_Noclip_f, qfalse },
	{ "gib", "Kills and automatically gibs you", "", Cmd_Gib_f, qfalse },
	{ "kill", "Kills you but does not gib", "", Cmd_Kill_f, qfalse },
	{ "mp40", "Sets the next weapon you'll spawn with to mp40", "", Cmd_SetMp40_f, qfalse },
	{ "thompson", "Sets the next weapon you'll spawn with to thompson", "", Cmd_SetThompson_f, qfalse },
	{ "sten", "Sets the next weapon you'll spawn with to sten", "", Cmd_SetSten_f, qfalse },
	{ "time", "Displays the current server time", "", Cmd_Time_f, qfalse },
	{ "uptime", "Shows server uptime", "", Cmd_DisplayServerUptime_f, qfalse },
	{ "levelshot", "Takes a picture (cheat)", "", Cmd_LevelShot_f, qfalse },
	{ "follow f", "Follows the given player", "/follow <player name>", Cmd_Follow_f, qfalse },
	{ "followclient", "Follows the given player", "/follow <client number>", Cmd_FollowClient_f, qfalse },
	{ "follownext", "If you're already spectating someone, this will change it to the next player", "", Cmd_FollowNext_f, qfalse },
	{ "followprev", "If you're already spectating someone, this will change it to the previous player", "", Cmd_FollowPrev_f, qfalse },
	{ "where", "Displays your current position in-game (x y z)", "", Cmd_Where_f, qfalse },
	{ "callvote", "Calls a vote", "/callvote <command name> <command argument>", Cmd_CallVote_f, qfalse },
	{ "vote", "Used to submit TK complaint or a vote", "/vote <y/Y/1/(any other value is no)>", Cmd_Vote_f, qfalse },
	{ "setviewpos svp", "Changes your view angles", "/setviewpos <pitch> <yaw> <roll>", Cmd_SetViewpos_f, qfalse },
	{ "touched_ents te", "Displays amount of entity slots that have been used.", "", Cmd_TouchedEntityCount_f, qfalse },
	{ "live_ents le", "Displays amount of live entities.", "", Cmd_LiveEntityCount_f, qfalse },
	{ "setspawnpt", "Sets the location of your next spawn", "/setspawnpt <0-15>", Cmd_SetSpawnPoint_f, qfalse },
	{ "more", "Displays the next several items (command dependent)", "/more", Cmd_More_f, qfalse },
	{ "smoke sg", "Throws smoke grenade (must be LT)", "/smoke or /sg", Cmd_Smoke, qfalse },
	{ "draw_hitboxes" , "Toggles hitbox drawing when g_drawHitBoxes is enabled", "/draw_hitboxes", Cmd_ToggleDrawHitBoxes_f, qfalse },
#ifdef _DEBUG
	{ "test", "Secrets!", "/test", Cmd_Test_f, qfalse },
	{ "test2", "Moar Secrets!", "/test2", Cmd_Test2_f, qfalse },
	{ "test3", "Moar Secrets!", "/test2", Cmd_Test3_f, qfalse },
#endif

	// This must be last
	{ NULL, NULL, NULL, NULL }
};

/*
=================
Display Help Docs for CMD
=================
*/
void Cmd_HelpDocs_f(gentity_t *ent) {
	const player_command_t *playerCommand;
	char cmd[MAX_TOKEN_CHARS];

	if (trap_Argc() != 2) {
		CP("print \"Usage: /help! <command>\n\"");
		return;
	}

	trap_Argv(1, cmd, sizeof(cmd));

	for (playerCommand = playerCommands; playerCommand->execute != NULL; playerCommand++) {
		if (Q_FindToken(playerCommand->names, cmd) && strlen(playerCommand->help)) {
			CP(va("print \"^3Help: ^7%s%s\n\"", playerCommand->help, strlen(playerCommand->usage) ? va("\n^3Usage: ^7%s", playerCommand->usage) : ""));
			return;
		}
	}

	CP(va("print \"Couldn't find help docs for ^3%s\n\"", cmd));
}

/*
=================
List Player Commands
=================
*/
void Cmd_ListCommands_f(gentity_t *ent) {
	const player_command_t *playerCommand;
	int count;
	const int moreCount = 20;
	qboolean more = qfalse;
	char tempname[128];

	if (!ent->moreCalled)
		ent->moreCalls = 0;

	CP("print \"\n\"");

	for (playerCommand = playerCommands + (ent->moreCalls * moreCount), count = 1; playerCommand->execute != NULL; playerCommand++, count++) {
		Q_strncpyz(tempname, playerCommand->names, sizeof(tempname));
		CP(va("print \"^3%s\n\"", Q_ChrReplace(tempname, ' ', '/')));

		if (count == moreCount && (playerCommands + 1)->execute != NULL) {
			more = qtrue;
			break;
		}
	}

	CP("print \"\n\"");
	
	if (more) {
		ent->more = Cmd_ListCommands_f;
		int remaining = (ArrayLength(playerCommands) - 1) - ((playerCommand - playerCommands) + 1);
		CP(va("print \"Use ^3/more ^7to list the next %d commands\n\"", remaining >= moreCount ? moreCount : remaining));
		CP("print \"Use ^3/help! <command> ^7for help with using a specific command\n\"");
	}
	else {
		CP("print \"There are no more commands to list\n\"");
		ent->more = 0;
		ent->moreCalls = 0;
	}
}

/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum ) {
	gentity_t *ent;
	const player_command_t *playerCommand;
	char	cmd[MAX_TOKEN_CHARS];
	char	arg[MAX_STRING_CHARS];
	int		i, argCount, totalLen = 0;

	ent = g_entities + clientNum;
	if ( !ent->client )
		return;

	argCount = trap_Argc();
	for (i = 0; i < argCount; ++i) {
		trap_Argv(i, arg, sizeof(arg));
		totalLen += strlen(arg);
	}
	if (totalLen > 259) {
		return;
	}

	trap_Argv( 0, cmd, sizeof( cmd ) );

	for (playerCommand = playerCommands; playerCommand->execute != NULL; playerCommand++) {
		if (Q_FindToken(playerCommand->names, cmd)) {
			if (!playerCommand->intermission && level.intermissiontime) {
				CP(va("print \"Server command '^3%s^7' can't be used during intermission.\n\"", cmd));
				return;
			}

			playerCommand->execute(ent);

			return;
		}
	}

	CP(va("print \"Server command ^3%s ^7could not be found^1!\n^7Use ^3/commands ^7for a list of available commands.\n\"", cmd));
}
