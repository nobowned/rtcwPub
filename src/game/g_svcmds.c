// this file holds commands that can be executed by the server console, but not remote clients
#include "g_local.h"

/*
===========================================================================
L0

- Removed bunch of useless stuff for old way of IP banning as it's severly outdated.
===========================================================================
*/

/*
=================
L0 - Svcmd_AddIP_f

Adds IP to banned file
=================
*/
void Svcmd_AddIP_f(void) {
	char	arg[MAX_STRING_TOKENS];

	trap_Argv(1, arg, sizeof(arg));
	BanClient(arg);
}

/*
================
L0 - Svcmd_tempban_f

Tempban's client
================
*/
void Svcmd_tempban_f(void){
	int			clientNum;
	int			bannedtime;
	gentity_t	*ent;
	char		arg1[MAX_STRING_TOKENS];
	char		arg2[MAX_STRING_TOKENS];

	trap_Argv(1, arg1, sizeof(arg1));
	clientNum = atoi(arg1);
	ent = &g_entities[clientNum];

	trap_Argv(2, arg2, sizeof(arg2));
	bannedtime = atoi(arg2);

	TempbanClient(ent, bannedtime);
}

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities+1;
	for (e = 1; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		G_Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			G_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			G_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			G_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			G_Printf("ET_MISSILE          ");
			break;
		case ET_MOVER:
			G_Printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			G_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			G_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			G_Printf("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			G_Printf("ET_PUSH_TRIGGER     ");
			break;
// JPW NERVE
		case ET_CONCUSSIVE_TRIGGER:
			G_Printf("ET_CONCUSSIVE_TRIGGR");
			break;
// jpw
		case ET_TELEPORT_TRIGGER:
			G_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			G_Printf("ET_INVISIBLE        ");
			break;
		case ET_GRAPPLE:
			G_Printf("ET_GRAPPLE          ");
			break;
		case ET_EXPLOSIVE:
			G_Printf("ET_EXPLOSIVE        ");
			break;
		case ET_EF_TESLA:
			G_Printf("ET_EF_TESLA         ");
			break;
		case ET_EF_SPOTLIGHT:
			G_Printf("ET_EF_SPOTLIGHT     ");
			break;
		case ET_EFFECT3:
			G_Printf("ET_EFFECT3          ");
			break;
		case ET_ALARMBOX:
			G_Printf("ET_ALARMBOX          ");
			break;
		default:
			G_Printf("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			G_Printf("%s", check->classname);
		}
		G_Printf("\n");
	}
}

gclient_t	*ClientForString( const char *s ) {
	gclient_t	*cl;
	int			i;
	int			idnum;

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return NULL;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			G_Printf( "Client %i is not connected\n", idnum );
			return NULL;
		}
		return cl;
	}

	// check for a name match
	for ( i=0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( cl->pers.netname, s ) ) {
			return cl;
		}
	}

	G_Printf( "User %s is not on the server\n", s );

	return NULL;
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void	Svcmd_ForceTeam_f( void ) {
	gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	// set the team
	trap_Argv( 2, str, sizeof( str ) );
	SetTeam( &g_entities[cl - level.clients], str, qfalse );
}

/*
============
Svcmd_StartMatch_f

NERVE - SMF - starts match if in tournament mode
============
*/
void Svcmd_StartMatch_f() {
	if ( !g_noTeamSwitching.integer ) {
		trap_SendServerCommand( -1, va("print \"g_noTeamSwitching not activated.\n\""));
		return;
	}

	if ( level.numPlayingClients <= 1 ) {
		trap_SendServerCommand( -1, va("print \"Not enough playing clients to start match.\n\""));
		return;
	}

	if ( g_gamestate.integer == GS_PLAYING ) {
		trap_SendServerCommand( -1, va("print \"Match is already in progress.\n\""));
		return;
	}

	if ( g_gamestate.integer == GS_WAITING_FOR_PLAYERS ) {
		trap_SendConsoleCommand( EXEC_APPEND, va( "map_restart 0 %i\n", GS_WARMUP ) );
	}

	if (g_gamestate.integer == GS_WARMUP_COUNTDOWN) {
		trap_SendConsoleCommand(EXEC_APPEND, va("map_restart 0 %i\n", GS_PLAYING));
	}
}

/*
============
Svcmd_ResetMatch_f

NERVE - SMF - this has three behaviors
- if not in tournament mode, do a map_restart
- if in tournament mode, go back to waitingForPlayers mode
- if in stopwatch mode, reset back to first round
============
*/
void Svcmd_ResetMatch_f() {
	if ( g_gametype.integer == GT_WOLF_STOPWATCH ) {
		trap_Cvar_Set( "g_currentRound", "0" );
		trap_Cvar_Set( "g_nextTimeLimit", "0" );
	}

	if ( !g_noTeamSwitching.integer || ( g_minGameClients.integer > 1 && level.numPlayingClients >= g_minGameClients.integer ) ) {
		trap_Cvar_Set("g_warmup", va("%i", latchedWarmup.integer));
		trap_SendConsoleCommand(EXEC_APPEND, va("map_restart 0 %i\n", GS_WARMUP));
		return;
	}
	else {
		trap_SendConsoleCommand( EXEC_APPEND, va( "map_restart 0 %i\n", GS_WAITING_FOR_PLAYERS ) );
		return;
	}
}

/*
============
Svcmd_SwapTeams_f

NERVE - SMF - swaps all clients to opposite team
============
*/
void Svcmd_SwapTeams_f() {
//  if ( g_gamestate.integer != GS_PLAYING ) {
	if ( (g_gamestate.integer == GS_INITIALIZE) || // JPW NERVE -- so teams can swap between checkpoint rounds
		 (g_gamestate.integer == GS_WAITING_FOR_PLAYERS) ||
		 (g_gamestate.integer == GS_RESET) ) {
		trap_SendServerCommand( -1, va("print \"Match must be in progress to swap teams.\n\""));
		return;
	}

	if ( g_gametype.integer == GT_WOLF_STOPWATCH ) {
		trap_Cvar_Set( "g_currentRound", "0" );
		trap_Cvar_Set( "g_nextTimeLimit", "0" );
	}

	trap_Cvar_Set( "g_swapteams", "1" );
	trap_SendConsoleCommand( EXEC_APPEND, va( "map_restart 0 %i\n", GS_WARMUP ) );

	// L0 - Swap teams
	G_swapTeamLocks();
}

/*
===========
L0 - Shuffle
===========
*/
void Svcmd_Shuffle_f(void)
{
	int count = 0, tmpCount, i;
	int players[MAX_CLIENTS];
	char	cmd[MAX_TOKEN_CHARS];

	trap_Argv(1, cmd, sizeof(cmd));

	memset(players, -1, sizeof(players));

	if (g_gamestate.integer == GS_RESET)
		return;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		//skip client numbers that aren't used
		if ((!g_entities[i].inuse) || (level.clients[i].pers.connected != CON_CONNECTED))
			continue;

		//ignore spectators
		if ((level.clients[i].sess.sessionTeam != TEAM_RED) && (level.clients[i].sess.sessionTeam != TEAM_BLUE))
			continue;

		players[count] = i;
		count++;
	}

	tmpCount = count;	// copy the number of active clients

	//loop through all the active players
	for (i = 0; i < count; i++)
	{
		int j;

		do {
			j = (rand() % count);
		} while (players[j] == -1);

		//put every other random choice on allies
		if (i & 1)
			level.clients[players[j]].sess.sessionTeam = TEAM_BLUE;
		else
			level.clients[players[j]].sess.sessionTeam = TEAM_RED;

		ClientUserinfoChanged(players[j]);
		ClientBegin(players[j]);


		players[j] = players[tmpCount - 1];
		players[tmpCount - 1] = -1;
		tmpCount--;
	}

	// Auto shuffle deals with that in it's own way..
	if (Q_stricmp(cmd, "@") != 0 && Q_stricmp(cmd, "@print") != 0)
		Svcmd_ResetMatch_f();

	// Auto shuffle already prints it's own stuff..
	if (Q_stricmp(cmd, "@print") != 0)
		AP("chat \"console: Teams were shuffled!\n\"");
}

/*
============
L0 - Print Poll answer..
============
*/
void Svcmd_PollPrint_f(void) {
	AP("chat \"console:^7 Poll result is ^2Yes^7!\n\"");
}

/*
=================
L0 - (Un)Ignores player

Called by vote or by admin directly (console)
=================
*/
void Svcmd_handleIgnore_f(qboolean unignore) {
	int clientNum;
	char buf[5];

	if (trap_Argc() != 2) {
		G_Printf("Usage: ignore <clientnum>\n");
		return;
	}

	trap_Argv(1, buf, sizeof(buf));
	clientNum = atoi(buf);

	if ((clientNum < 0) || (clientNum >= MAX_CLIENTS)) {
		G_Printf("Invalid client number.\n");
		return;
	}

	if ((!g_entities[clientNum].client) || (level.clients[clientNum].pers.connected != CON_CONNECTED)) {
		G_Printf("Client not on server.\n");
		return;
	}

	if (unignore)
	{
		g_entities[clientNum].client->sess.ignored = IGNORE_OFF;
		CPx(clientNum, "cp \"You are now ^3Unignored^7!\n\"2");
		AP(va("chat \"console: ^7%s ^7has been Unignored.\n\"", g_entities[clientNum].client->pers.netname));
	}
	else
	{
		g_entities[clientNum].client->sess.ignored = IGNORE_PERMANENT;
		CPx(clientNum, "cp \"You are now ^1ignored^7!\n\"2");
		AP(va("chat \"console: ^7%s ^7has been ignored.\n\"", g_entities[clientNum].client->pers.netname));
	}
}

char	*ConcatArgs( int start );

/*
=================
ConsoleCommand

=================
*/
qboolean	ConsoleCommand( void ) {
	char	cmd[MAX_TOKEN_CHARS];

	trap_Argv( 0, cmd, sizeof( cmd ) );

// L0 - New stuff
	// Tempban (IP)
	if ( Q_stricmp(cmd, "tempban") == 0 ) {
		Svcmd_tempban_f();
		return qtrue;
	}

	// AddIP command
	if  (Q_stricmp(cmd, "addip") == 0 ) {
		Svcmd_AddIP_f();
		return qtrue;
	}

	// Shuffle
	if (Q_stricmp(cmd, "shuffle") == 0) {
		Svcmd_Shuffle_f();
		return qtrue;
	}

	// Poll
	if (Q_stricmp(cmd, "poll") == 0) {
		Svcmd_PollPrint_f();
		return qtrue;
	}

	// Ignore
	if (Q_stricmp(cmd, "ignore") == 0) {
		Svcmd_handleIgnore_f(qfalse);
		return qtrue;
	}

	// Unignore
	if (Q_stricmp(cmd, "unignore") == 0) {
		Svcmd_handleIgnore_f(qtrue);
		return qtrue;
	}
// End

	if ( Q_stricmp (cmd, "entitylist") == 0 ) {
		Svcmd_EntityList_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "forceteam") == 0 ) {
		Svcmd_ForceTeam_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "game_memory") == 0) {
		Svcmd_GameMem_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addbot") == 0) {
		Svcmd_AddBot_f();
		return qtrue;
	}

	// NERVE - SMF
	if (Q_stricmp (cmd, "start_match") == 0) {
		Svcmd_StartMatch_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "reset_match") == 0) {
		Svcmd_ResetMatch_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "swap_teams") == 0) {
		Svcmd_SwapTeams_f();
		return qtrue;
	}
	// -NERVE - SMF

	if (g_dedicated.integer) {
		if (Q_stricmp (cmd, "say") == 0) {
			trap_SendServerCommand( -1, va("print \"server:[lof] %s\"", ConcatArgs(1) ) );
			return qtrue;
		}
		// everything else will also be printed as a say command
		trap_SendServerCommand( -1, va("print \"server:[lof] %s\"", ConcatArgs(0) ) );
		return qtrue;
	}

	return qfalse;
}

