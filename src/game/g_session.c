#include "g_local.h"


/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client ) {
	const char	*s;
	const char	*var;

	s = va("%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",		// DHM - Nerve
		client->sess.sessionTeam,
		client->sess.spectatorTime,
		client->sess.spectatorState,
		client->sess.spectatorClient,
		client->sess.wins,
		client->sess.losses,
		client->sess.playerType,			// DHM - Nerve
		client->sess.playerWeapon,			// DHM - Nerve
		client->sess.playerItem,			// DHM - Nerve
		client->sess.playerSkin,			// DHM - Nerve
		client->sess.spawnObjectiveIndex,	// DHM - Nerve
		client->sess.latchPlayerType,		// DHM - Nerve
		client->sess.latchPlayerWeapon,		// DHM - Nerve
		client->sess.latchPlayerItem,		// DHM - Nerve
		client->sess.latchPlayerSkin,		// DHM - Nerve
		// L0 - New stuff
		client->sess.admin,
		client->sess.ip,
		client->sess.incognito,
		client->sess.ignored,
		client->sess.selectedWeapon,
		client->sess.colorFlags,
		client->sess.gender,
		client->sess.tempIgnoreCount,
		client->sess.secretlyDemoing,
		client->sess.muted[0],
		client->sess.muted[1]
		);

	var = va( "session%i", client - level.clients );

	trap_Cvar_Set( var, s );
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client ) {
	char	s[MAX_STRING_CHARS];
	const char	*var;
	qboolean test;

	var = va( "session%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );

	sscanf(s, "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",		// DHM - Nerve
		(int *)&client->sess.sessionTeam,
		&client->sess.spectatorTime,
		(int *)&client->sess.spectatorState,
		&client->sess.spectatorClient,
		&client->sess.wins,
		&client->sess.losses,
		&client->sess.playerType,			// DHM - Nerve
		&client->sess.playerWeapon,			// DHM - Nerve
		&client->sess.playerItem,			// DHM - Nerve
		&client->sess.playerSkin,			// DHM - Nerve
		&client->sess.spawnObjectiveIndex,	// DHM - Nerve
		&client->sess.latchPlayerType,		// DHM - Nerve
		&client->sess.latchPlayerWeapon,	// DHM - Nerve
		&client->sess.latchPlayerItem,		// DHM - Nerve
		&client->sess.latchPlayerSkin,		// DHM - Nerve
		// L0 - New stuff
		(int *)&client->sess.admin,
		&client->sess.ip,
		&client->sess.incognito,
		(int *)&client->sess.ignored,
		&client->sess.selectedWeapon,
		&client->sess.colorFlags,
		&client->sess.gender,
		&client->sess.tempIgnoreCount,
		&client->sess.secretlyDemoing,
		&client->sess.muted[0],
		&client->sess.muted[1]
		);

	// NERVE - SMF
	if ( g_altStopwatchMode.integer )	
		test = qtrue;
	else
		test = g_currentRound.integer == 1;

	if ( g_gametype.integer == GT_WOLF_STOPWATCH && level.warmupTime > 0 && test ) {
		if ( client->sess.sessionTeam == TEAM_RED ) {
			client->sess.sessionTeam = TEAM_BLUE;
		}
		else if ( client->sess.sessionTeam == TEAM_BLUE ) {
			client->sess.sessionTeam = TEAM_RED;
		}
	}

	if ( g_swapteams.integer ) {
		trap_Cvar_Set( "g_swapteams", "0" );

		if ( client->sess.sessionTeam == TEAM_RED ) {
			client->sess.sessionTeam = TEAM_BLUE;
		}
		else if ( client->sess.sessionTeam == TEAM_BLUE ) {
			client->sess.sessionTeam = TEAM_RED;
		}
	}

	if (g_ffa.integer)
	{
		// NOTE(nobo): All players need to be forced to allies in ffa.
		if (client->sess.sessionTeam == TEAM_RED)
		{
			client->sess.sessionTeam = TEAM_BLUE;
		}
	}
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData( gclient_t *client, char *userinfo ) {
	clientSession_t	*sess;
	const char		*value;

	sess = &client->sess;

	// initial team determination
	if ( g_gametype.integer >= GT_TEAM ) {
		// always spawn as spectator in team games
		sess->sessionTeam = TEAM_SPECTATOR;	
	} else {
		value = Info_ValueForKey( userinfo, "team" );
		if ( value[0] == 's' ) {
			// a willing spectator, not a waiting-in-line
			sess->sessionTeam = TEAM_SPECTATOR;
		} else {
			switch ( g_gametype.integer ) {
			default:
			case GT_FFA:
			case GT_SINGLE_PLAYER:
				if ( g_maxGameClients.integer > 0 && 
					level.numPlayingClients >= g_maxGameClients.integer ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			case GT_TOURNAMENT:
				// if the game is full, go into a waiting mode
				if ( level.numPlayingClients >= 2 ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			}
		}
	}

	sess->spectatorState = SPECTATOR_FREE;
	sess->spectatorTime = level.time;

	// DHM - Nerve
	sess->latchPlayerType = sess->playerType = 0;
	sess->latchPlayerWeapon = sess->playerWeapon = 0;
	sess->latchPlayerItem = sess->playerItem = 0;
	sess->latchPlayerSkin = sess->playerSkin = 0;

	sess->spawnObjectiveIndex = 0;
	// dhm - end

	// L0 - New stuff
	sess->admin = ADM_NONE;
	sess->ip = 0;
	sess->incognito = 0;
	sess->ignored = IGNORE_OFF;
	sess->colorFlags = 0;
	sess->gender = 0;

	G_WriteClientSessionData( client );
}


/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) {
	char	s[MAX_STRING_CHARS];
	int		gt;
	char	*tmp = s;
	qboolean swap = ((g_altStopwatchMode.integer != 0 || g_currentRound.integer == 1) ? qtrue : qfalse);

	trap_Cvar_VariableStringBuffer( "session", s, sizeof(s) );
	gt = atoi( s );
	
	// if the gametype changed since the last session, don't use any
	// client sessions
	if ( g_gametype.integer != gt ) {
		level.newSession = qtrue;
		G_Printf( "Gametype changed, clearing session data.\n" );
	}

// OSP
#define GETVAL( x ) if ( ( tmp = strchr( tmp, ' ' ) ) == NULL ) {return;} x = atoi(++tmp);
	
	GETVAL(gt);
	teamInfo[TEAM_RED].team_lock = (gt & TEAM_RED) ? qtrue : qfalse;
	teamInfo[TEAM_BLUE].team_lock = (gt & TEAM_BLUE) ? qtrue : qfalse;

	// L0 - We handle rest of swaps in g_svcmds.c ..
	if (g_gametype.integer == GT_WOLF_STOPWATCH && g_gamestate.integer != GS_PLAYING && swap) {
		G_swapTeamLocks();
	}
// End

	ammoTable[WP_STEN].maxHeat = g_stenMaxHeat.integer;
	ammoTable[WP_STEN].coolRate = g_stenCoolRate.integer;
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( void ) {
	int		i;
	
	trap_Cvar_Set( "session", va("%i %i", 
		g_gametype.integer, 
		// OSP
		(teamInfo[TEAM_RED].team_lock * TEAM_RED | teamInfo[TEAM_BLUE].team_lock * TEAM_BLUE)
	) );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			G_WriteClientSessionData( &level.clients[i] );
		}
	}
}
