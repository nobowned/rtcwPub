#include "g_admin.h"

// g_client.c -- client functions that don't happen every frame

// Ridah, new bounding box
//static vec3_t	playerMins = {-15, -15, -24};
//static vec3_t	playerMaxs = {15, 15, 32};
vec3_t	playerMins = {-18, -18, -24};
vec3_t	playerMaxs = {18, 18, 48};
// done.

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
If the start position is targeting an entity, the players camera will start out facing that ent (like an info_notnull)
*/
void SP_info_player_deathmatch( gentity_t *ent ) {
	int		i;
	vec3_t	dir;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}

	ent->enemy = G_PickTarget( ent->target );
	if(ent->enemy)
	{
		VectorSubtract( ent->enemy->s.origin, ent->s.origin, dir );
		vectoangles( dir, ent->s.angles );
	}

}

//----(SA) added
/*QUAKED info_player_checkpoint (1 0 0) (-16 -16 -24) (16 16 32) a b c d
these are start points /after/ the level start
the letter (a b c d) designates the checkpoint that needs to be complete in order to use this start position
*/
void SP_info_player_checkpoint(gentity_t *ent) {
	ent->classname = "info_player_checkpoint";
	SP_info_player_deathmatch( ent );
}

//----(SA) end


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
equivelant to info_player_deathmatch
*/
void SP_info_player_start(gentity_t *ent) {
	ent->classname = "info_player_deathmatch";
	SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32) AXIS ALLIED
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_player_intermission( gentity_t *ent ) {

}

/*
================
SpotWouldConflict
================
*/
qboolean SpotWouldConflict(gentity_t *spot) {
	int			i, count;
	int			entList[MAX_GENTITIES];
	gentity_t	*hitEnt;
	vec3_t		mins, maxs;

	VectorAdd(spot->s.origin, playerMins, mins);
	VectorAdd(spot->s.origin, playerMaxs, maxs);
	count = trap_EntitiesInBox(mins, maxs, entList, MAX_GENTITIES);

	for (i = 0; i < count; i++) {
		hitEnt = g_entities + entList[i];

		if (hitEnt->client && hitEnt->client->ps.stats[STAT_HEALTH] > 0) {
			return qtrue;
		}
	}

	return qfalse;
}

/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectNearestDeathmatchSpawnPoint( vec3_t from ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist, nearestDist;
	gentity_t	*nearestSpot;

	nearestDist = 999999;
	nearestSpot = NULL;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {

		VectorSubtract( spot->s.origin, from, delta );
		dist = VectorLength( delta );
		if ( dist < nearestDist ) {
			nearestDist = dist;
			nearestSpot = spot;
		}
	}

	return nearestSpot;
}


/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectRandomDeathmatchSpawnPoint( void ) {
	gentity_t	*spot;
	int			count;
	int			selection;
	gentity_t	*spots[MAX_SPAWN_POINTS];

	count = 0;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		if ( SpotWouldConflict( spot ) ) {
			continue;
		}
		spots[ count ] = spot;
		count++;
	}

	if ( !count ) {	// no spots that won't telefrag
		return G_Find( NULL, FOFS(classname), "info_player_deathmatch");
	}

	selection = rand() % count;
	return spots[ selection ];
}

/*
================
SelectSpawnPoint

finds a spawn point
================
*/
gentity_t *SelectSpawnPoint(vec3_t origin, vec3_t angles) {
	gentity_t *spot = NULL;

	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		if (SpotWouldConflict(spot)) {
			continue;
		}
		break;
	}

	VectorCopy(spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy(spot->s.angles, angles);

	return spot;
}

/*
================
SelectCyclicSpawnPoint

Finds the next spawn point (defined by cyclicCounter).
This was created for the FFA gametype.
================
*/
gentity_t *SelectCyclicSpawnPoint(vec3_t origin, vec3_t angles) {
	static int cycleCounter;
	static int firstSpawnIndex;
	static int spawnCount = 0;
	gentity_t *spot = NULL;

	if (spawnCount == 0)
	{
		while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL)
		{
			if (g_ffa.integer && spot->spawnflags != 4)
			{
				continue;
			}

			if (spawnCount == 0)
			{
				cycleCounter = firstSpawnIndex = spot - g_entities;
			}

			spawnCount++;
		}
	}

	int spotsChecked = 0;
	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		if (g_ffa.integer && spot->spawnflags != 4)
		{
			continue;
		}

		if (spotsChecked++ > spawnCount)
		{
			spotsChecked = 0;
			cycleCounter = firstSpawnIndex;
		}

		if (SpotWouldConflict(spot))
		{
			continue;
		}
		if (spot->s.number < cycleCounter)
		{
			continue;
		}

		break;
	}

	if (spot && spawnCount > 0)
	{
		cycleCounter = ((spot - g_entities) + 1) % (firstSpawnIndex + spawnCount);
	}
	else
	{
		spot = SelectRandomSpawnPoint(vec3_origin, origin, angles);
		if (!spot)
		{
			G_Error(va("SelectCyclicSpawnPoint: Failed to find an 'info_player_deathmatch' spawn point. g_ffa is %i", g_ffa.integer));
		}
	}

	VectorCopy(spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy(spot->s.angles, angles);

	return spot;
}

/*
===========
SelectRandomSpawnPoint

Chooses a random player start, deathmatch start, etc
============
*/
gentity_t *SelectRandomSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles ) {
	gentity_t	*spot;
	gentity_t	*nearestSpot;

	nearestSpot = SelectNearestDeathmatchSpawnPoint( avoidPoint );

	spot = SelectRandomDeathmatchSpawnPoint ( );
	if ( spot == nearestSpot ) {
		// roll again if it would be real close to point of death
		spot = SelectRandomDeathmatchSpawnPoint ( );
		if ( spot == nearestSpot ) {
			// last try
			spot = SelectRandomDeathmatchSpawnPoint ( );
		}		
	}

	// find a single player start spot
	if (!spot) {
		G_Error( "Couldn't find a spawn point" );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
}

/*
===========
SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
gentity_t *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles ) {
	gentity_t	*spot;

	spot = NULL;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		if ( spot->spawnflags & 1 ) {
			break;
		}
	}

	if ( !spot || SpotWouldConflict( spot ) ) {
		return SelectRandomSpawnPoint( vec3_origin, origin, angles );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
}

/*
===========
SelectSpectatorSpawnPoint

============
*/
gentity_t *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles ) {
	FindIntermissionPoint();

	VectorCopy( level.intermission_origin, origin );
	VectorCopy( level.intermission_angle, angles );

	return NULL;
}

/*
=======================================================================

BODYQUE

=======================================================================
*/

/*
===============
InitBodyQue
===============
*/
void InitBodyQue (void) {
	int		i;
	gentity_t	*ent;

	level.bodyQueIndex = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++) {
		ent = G_Spawn();
		ent->classname = "bodyque";
		ent->neverFree = qtrue;
		level.bodyQue[i] = ent;
	}
}

/*
=============
BodySink

After sitting around for five seconds, fall into the ground and dissapear
=============
*/
void BodySink( gentity_t *ent ) {
	if ( level.time - ent->timestamp > 6500 ) {
		// the body ques are never actually freed, they are just unlinked
		trap_UnlinkEntity( ent );
		ent->physicsObject = qfalse;
		return;	
	}
	ent->nextthink = level.time + 100;
	ent->s.pos.trBase[2] -= 1;
}

/*
=============
CopyToBodyQue

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
void CopyToBodyQue( gentity_t *ent ) {
	gentity_t		*body;
	int			contents, i;

	trap_UnlinkEntity (ent);

	// if client is in a nodrop area, don't leave the body
	contents = trap_PointContents( ent->s.origin, -1 );
	if ( contents & CONTENTS_NODROP ) {
		return;
	}

	// grab a body que and cycle to the next one
	body = level.bodyQue[ level.bodyQueIndex ];
	level.bodyQueIndex = (level.bodyQueIndex + 1) % BODY_QUEUE_SIZE;

	trap_UnlinkEntity (body);

	body->s = ent->s;
	body->s.eFlags = EF_DEAD;		// clear EF_TALK, etc

	if(ent->client->ps.eFlags & EF_HEADSHOT )
		body->s.eFlags |= EF_HEADSHOT;			// make sure the dead body draws no head (if killed that way)

	body->s.powerups = 0;	// clear powerups
	body->s.loopSound = 0;	// clear lava burning
	body->s.number = body - g_entities;
	body->timestamp = level.time;
	body->physicsObject = qtrue;
	body->physicsBounce = 0;		// don't bounce
	if ( body->s.groundEntityNum == ENTITYNUM_NONE ) {
		body->s.pos.trType = TR_GRAVITY;
		body->s.pos.trTime = level.time;
		VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );
	} else {
		body->s.pos.trType = TR_STATIONARY;
	}
	body->s.event = 0;

	// DHM - Clear out event system
	for( i=0; i<MAX_EVENTS; i++ )
		body->s.events[i] = 0;
	body->s.eventSequence = 0;

	// DHM - Nerve
	if ( g_gametype.integer != GT_SINGLE_PLAYER )
	{
	// change the animation to the last-frame only, so the sequence
	// doesn't repeat anew for the body
	switch ( body->s.legsAnim & ~ANIM_TOGGLEBIT ) {
	case BOTH_DEATH1:
	case BOTH_DEAD1:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD1;
		break;
	case BOTH_DEATH2:
	case BOTH_DEAD2:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD2;
		break;
	case BOTH_DEATH3:
	case BOTH_DEAD3:
	default:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD3;
		break;
	}
	}
	// dhm

	body->r.svFlags = ent->r.svFlags;
	VectorCopy (ent->r.mins, body->r.mins);
	VectorCopy (ent->r.maxs, body->r.maxs);
	VectorCopy (ent->r.absmin, body->r.absmin);
	VectorCopy (ent->r.absmax, body->r.absmax);

	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	// DHM - Nerve :: allow bullets to pass through bbox
	body->r.contents = 0;
	body->r.ownerNum = ent->r.ownerNum;

	body->nextthink = level.time + 5000;
	body->think = BodySink;

	body->die = body_die;

	// don't take more damage if already gibbed
	if ( ent->health <= GIB_HEALTH ) {
		body->takedamage = qfalse;
	} else {
		body->takedamage = qtrue;
	}


	VectorCopy ( body->s.pos.trBase, body->r.currentOrigin );
	trap_LinkEntity (body);
}

//======================================================================

/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gentity_t *ent, vec3_t angle ) {
	int			i;

	// set the delta angle
	for (i=0 ; i<3 ; i++) {
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT(angle[i]);
		ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
	}
	VectorCopy( angle, ent->s.angles );
	VectorCopy (ent->s.angles, ent->client->ps.viewangles);
}

/* JPW NERVE
================
limbo
================
*/
void limbo( gentity_t *ent, qboolean makeCorpse ) {
	int i,contents;
	//int startclient = ent->client->sess.spectatorClient;
	int startclient = ent->s.clientNum;

	if (g_gametype.integer == GT_SINGLE_PLAYER) {
		G_Printf("FIXME: limbo called from single player game.  Shouldn't see this\n");
		return;
	}
	if (!(ent->client->ps.pm_flags & PMF_LIMBO)) {

		// DHM - Nerve :: First save off persistant info we'll need for respawn
		for (i = 0; i < MAX_PERSISTANT; i++)
			ent->client->saved_persistant[i] = ent->client->ps.persistant[i];
		// dhm

		ent->client->ps.pm_flags |= PMF_LIMBO;
		if (!(ent->r.svFlags & SVF_BOT))
			ent->client->ps.pm_flags |= PMF_FOLLOW;

		if (makeCorpse)
			CopyToBodyQue(ent); // make a nice looking corpse
		else
			trap_UnlinkEntity(ent);

		// DHM - Nerve :: reset these values
		ent->client->ps.viewlocked = 0;
		ent->client->ps.viewlocked_entNum = 0;

		ent->r.maxs[2] = 0;
		ent->r.currentOrigin[2] += 8;
		contents = trap_PointContents(ent->r.currentOrigin, -1); // drop stuff
		ent->s.weapon = ent->client->limboDropWeapon; // stored in player_die()
		if (makeCorpse && !(contents & CONTENTS_NODROP)) {
			TossClientItems(ent);
		}

		ent->client->sess.spectatorClient = startclient;
		if (!(ent->r.svFlags & SVF_BOT)) {
			Cmd_FollowCycle_f(ent, 1); // get fresh spectatorClient
		}

		if (ent->client->sess.spectatorClient == startclient) {
			// No one to follow, so just stay put
			ent->client->sess.spectatorState = SPECTATOR_FREE;
		}
		else
			ent->client->sess.spectatorState = SPECTATOR_FOLLOW;

//		ClientUserinfoChanged( ent->client - level.clients );		// NERVE - SMF - don't do this
		if (ent->client->sess.sessionTeam == TEAM_RED) {
			ent->client->deployQueueNumber = level.redNumWaiting;
			level.redNumWaiting++;
		}
		else if (ent->client->sess.sessionTeam == TEAM_BLUE) {
			ent->client->deployQueueNumber = level.blueNumWaiting;
			level.blueNumWaiting++;
		}
		
		if (ent->r.svFlags & SVF_BOT)
			return;
		
		for (i = 0; i < level.maxclients; i++) {
			if (level.clients[i].ps.pm_flags & PMF_LIMBO
				&& level.clients[i].sess.spectatorClient == ent->s.number) {
				Cmd_FollowCycle_f(&g_entities[i], 1);
			}
		}
	}
}

/* JPW NERVE
================
reinforce 
================
// -- called when time expires for a team deployment cycle and there is at least one guy ready to go
*/
void reinforce(gentity_t *ent) {
	int p, team;// numDeployable=0, finished=0; // TTimo unused
	char *classname;
	gclient_t *rclient;

	if (g_gametype.integer == GT_SINGLE_PLAYER) {
		G_Printf("FIXME: reinforce called from single player game.  Shouldn't see this\n");
		return;
	}
	if (!(ent->client->ps.pm_flags & PMF_LIMBO)) {
		G_Printf("player already deployed, skipping\n");
		return;
	}
	// get team to deploy from passed entity

	team = ent->client->sess.sessionTeam;

	// find number active team spawnpoints
	if (team == TEAM_RED)
		classname = "team_CTF_redspawn";
	else if (team == TEAM_BLUE)
		classname = "team_CTF_bluespawn";
	else
		assert(0);

	// DHM - Nerve :: restore persistant data now that we're out of Limbo
	rclient = ent->client;
	for (p=0; p<MAX_PERSISTANT; p++)
		rclient->ps.persistant[p] = rclient->saved_persistant[p];
	// dhm

	respawn(ent);
}
// jpw


/*
================
respawn
================
*/
void respawn( gentity_t *ent ) {
	//gentity_t	*tent;

	// Ridah, if single player, reload the last saved game for this player
	if (g_gametype.integer == GT_SINGLE_PLAYER) {

		if (reloading || saveGamePending) {
			return;
		}

		if (!(ent->r.svFlags & SVF_CASTAI)) {
			// Fast method, just do a map_restart, and then load in the savegame
			// once everything is settled.
			trap_SetConfigstring( CS_SCREENFADE, va("1 %i 500", level.time + 250) );
			reloading = qtrue;
			level.reloadDelayTime = level.time + 1500;

			return;
		}
	}

	// done.

	ent->client->ps.pm_flags &= ~PMF_LIMBO; // JPW NERVE turns off limbo

	// DHM - Nerve :: Decrease the number of respawns left
	if (g_maxlives.integer > 0 && ent->client->ps.persistant[PERS_RESPAWNS_LEFT] > 0 &&
		!level.warmupTime && g_gamestate.integer != GS_WAITING_FOR_PLAYERS) {
		ent->client->ps.persistant[PERS_RESPAWNS_LEFT]--;
	}

	G_DPrintf( "Respawning %s, %i lives left\n", ent->client->pers.netname, ent->client->ps.persistant[PERS_RESPAWNS_LEFT]);

	// DHM - Nerve :: Already handled in 'limbo()'
	if ( g_gametype.integer < GT_WOLF )
		CopyToBodyQue (ent);

	ClientSpawn(ent, qfalse);

	// DHM - Nerve :: Add back if we decide to have a spawn effect
	// add a teleportation effect
	//tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
	//tent->s.clientNum = ent->s.clientNum;

	// L0 - antilag
	G_ResetTrail(ent);
	ent->client->saved.leveltime = 0;
}

// NERVE - SMF - merge from team arena
/*
================
TeamCount

Returns number of players on a team
================
*/
team_t TeamCount( int ignoreClientNum, int team ) {
	int		i;
	int		count = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == team ) {
			count++;
		}
	}

	return count;
}
// -NERVE - SMF

/*
================
PickTeam

================
*/
team_t PickTeam( int ignoreClientNum ) {
	int		i;
	int		counts[TEAM_NUM_TEAMS];

	memset( counts, 0, sizeof( counts ) );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == TEAM_BLUE ) {
			counts[TEAM_BLUE]++;
		}
		else if ( level.clients[i].sess.sessionTeam == TEAM_RED ) {
			counts[TEAM_RED]++;
		}
	}

	if ( counts[TEAM_BLUE] > counts[TEAM_RED] ) {
		return TEAM_RED;
	}
	if ( counts[TEAM_RED] > counts[TEAM_BLUE] ) {
		return TEAM_BLUE;
	}
	// equal team count, so join the team with the lowest score
	if ( level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED] ) {
		return TEAM_RED;
	}
	return TEAM_BLUE;
}

/*
===========
ForceClientSkin

Forces a client's skin (for teamplay)
===========
*/
void ForceClientSkin( gclient_t *client, char *model, const char *skin ) {
	char *p;

	if ((p = strchr(model, '/')) != NULL) {
		*p = 0;
	}

	Q_strcat(model, MAX_QPATH, "/");
	Q_strcat(model, MAX_QPATH, skin);
}

// NERVE - SMF
/*
===========
SetWolfUserVars
===========
*/
void SetWolfUserVars( gentity_t *ent, char *wolfinfo ) {
	gclient_t *client;
	int mask, team;

	client = ent->client;
	if ( !client )
		return;

	// check if we have a valid snapshot
	mask = MP_TEAM_MASK;
	team = ( client->pers.cmd.mpSetup & mask ) >> MP_TEAM_OFFSET;

	if ( !team )
		return;

	// set player class
	mask = MP_CLASS_MASK;
	client->sess.latchPlayerType = g_forceClass.integer >= 0 ?
		g_forceClass.integer : (client->pers.cmd.mpSetup & mask) >> MP_CLASS_OFFSET;

	// set weapon
	mask = MP_WEAPON_MASK;
	client->sess.latchPlayerWeapon = ( client->pers.cmd.mpSetup & mask ) >> MP_WEAPON_OFFSET;
}

// -NERVE - SMF


// DHM - Nerve
/*
===========
SetWolfSkin

Forces a client's skin (for Wolfenstein teamplay)
===========
*/

#define MULTIPLAYER_ALLIEDMODEL	"multi"
#define MULTIPLAYER_AXISMODEL	"multi_axis"

void SetWolfSkin( gclient_t *client, char *model ) {

	switch( client->sess.sessionTeam ) {
	case TEAM_RED:
		Q_strcat( model, MAX_QPATH, "red" );
		break;
	case TEAM_BLUE:
		Q_strcat( model, MAX_QPATH, "blue" );
		break;
	default:
		Q_strcat( model, MAX_QPATH, "red" );
		break;
	}

	switch ( client->sess.playerType ) {
	case PC_SOLDIER:
		Q_strcat( model, MAX_QPATH, "soldier" );
		break;
	case PC_MEDIC:
		Q_strcat( model, MAX_QPATH, "medic" );
		break;
	case PC_ENGINEER:
		Q_strcat( model, MAX_QPATH, "engineer" );
		break;
	case PC_LT:
		Q_strcat( model, MAX_QPATH, "lieutenant" );
		break;
	default:
		Q_strcat( model, MAX_QPATH, "soldier" );
		break;
	}

	// DHM - A skinnum will be in the session data soon...
	switch ( client->sess.playerSkin ) {
	case 1:
		Q_strcat( model, MAX_QPATH, "1" );
		break;
	case 2:
		Q_strcat( model, MAX_QPATH, "2" );
		break;
	case 3:
		Q_strcat( model, MAX_QPATH, "3" );
		break;
	default:
		Q_strcat( model, MAX_QPATH, "1" );
		break;
	}
}

/*
===========
CheckMaxHealth

Checks and potentially sets STAT_MAX_HEALTH for both teams
===========
*/
void CheckMaxHealth()
{
	int numMedics = 0;
	int starthealth = 100;
	int i, team;
	gclient_t *cl;

	// check both teams
	for (team = TEAM_RED; team <= TEAM_SPECTATOR; ++team)
	{
		// count up # of medics on team
		for (i = 0; i < level.maxclients; i++) {
			cl = level.clients + i;

			if (cl->pers.connected != CON_CONNECTED)
			{
				continue;
			}
			if (cl->sess.sessionTeam != team)
			{
				continue;
			}
			if (cl->ps.stats[STAT_PLAYER_CLASS] != PC_MEDIC)
			{
				continue;
			}
			numMedics++;
		}

		// compute health mod	
		starthealth = 100 + 10 * numMedics;
		if (starthealth > 125 || g_ffa.integer) 
		{
			starthealth = 125;
		}

		// give everybody health mod in stat_max_health
		for (i = 0; i < level.maxclients; i++) 
		{
			cl = level.clients + i;

			if (cl->pers.connected == CON_DISCONNECTED) 
			{
				continue;
			}
			if (cl->sess.sessionTeam == team)
			{
				cl->pers.maxHealth = cl->ps.stats[STAT_MAX_HEALTH] = starthealth;
			}
		}
	}
}

int SwToWp(int sw)
{
	switch (sw)
	{
	case SW_FLAMER:
		return WP_FLAMETHROWER;
	case SW_MAUSER:
		return WP_MAUSER;
	case SW_MP40:
		return WP_MP40;
	case SW_PANZER:
		return WP_PANZERFAUST;
	case SW_STEN:
		return WP_STEN;
	case SW_THOMPSON:
		return WP_THOMPSON;
	case SW_VENOM:
		return WP_VENOM;
	default:
		return WP_NONE;
	}
}

/*
===========
SetWolfSpawnWeapons

Sets whatever weapons/items player should spawn with
===========
*/
void SetWolfSpawnWeapons( gclient_t *client ) {

	int		pc = client->sess.playerType;
	int		soldClips = g_soldierClips.integer;
	int		ltClips = g_ltClips.integer;
	int		gunClips = g_pistolClips.integer;
	int		venomClips = g_venomClips.integer;
	int		mauserClips = g_mauserClips.integer;
	// nades
	int		engNades = g_engNades.integer;
	int		soldNades = g_soldNades.integer;
	int		medNades = g_medicNades.integer;
	int		ltNades = g_ltNades.integer;
	// end

	if (client->sess.sessionTeam == TEAM_SPECTATOR) {
		return;
	}

	int clientNum = client - level.clients;

	// Reset special weapon time
	client->ps.classWeaponTime = -999999;

	// Communicate it to cgame
	client->ps.stats[STAT_PLAYER_CLASS] = pc;

	// Abuse teamNum to store player class as well (can't see stats for all clients in cgame)
	client->ps.teamNum = pc;

	// JPW NERVE -- zero out all ammo counts
	memset(client->ps.ammo, MAX_WEAPONS, sizeof(int));

	// All players start with a knife (not OR-ing so that it clears previous weapons)
	client->ps.weapons[0] = 0;
	client->ps.weapons[1] = 0;
	COM_BitSet(client->ps.weapons, WP_KNIFE);

	client->ps.ammo[BG_FindAmmoForWeapon(WP_KNIFE)] = 1;
	client->ps.weapon = WP_KNIFE;
	client->ps.weaponstate = WEAPON_READY;

	if (g_throwKnives.integer > 0) {
		client->pers.throwingKnives = g_throwKnives.integer;
	}

	// Engineer gets dynamite
	if (pc == PC_ENGINEER) {
		COM_BitSet(client->ps.weapons, WP_DYNAMITE);
		client->ps.ammo[BG_FindAmmoForWeapon(WP_DYNAMITE)] = 0;
		client->ps.ammoclip[BG_FindClipForWeapon(WP_DYNAMITE)] = 1;

		// NERVE - SMF
		COM_BitSet(client->ps.weapons, WP_PLIERS);
		client->ps.ammoclip[BG_FindClipForWeapon(WP_PLIERS)] = 1;
		client->ps.ammo[WP_PLIERS] = 1;
	}

	// Lieutenant gets binoculars, ammo pack, artillery, and a grenade
	if (pc == PC_LT) {
		client->ps.stats[STAT_KEYS] |= (1 << INV_BINOCS);
		COM_BitSet(client->ps.weapons, WP_AMMO);
		client->ps.ammo[BG_FindAmmoForWeapon(WP_AMMO)] = 0;
		client->ps.ammoclip[BG_FindClipForWeapon(WP_AMMO)] = 1;
		COM_BitSet(client->ps.weapons, WP_ARTY);
		client->ps.ammo[BG_FindAmmoForWeapon(WP_ARTY)] = 0;
		client->ps.ammoclip[BG_FindClipForWeapon(WP_ARTY)] = 1;

		// NERVE - SMF
		COM_BitSet(client->ps.weapons, WP_SMOKE_GRENADE);
		client->ps.ammoclip[BG_FindClipForWeapon(WP_SMOKE_GRENADE)] = 1;
		client->ps.ammo[WP_SMOKE_GRENADE] = 1;

		switch (client->sess.sessionTeam) {
		case TEAM_BLUE:
			COM_BitSet(client->ps.weapons, WP_GRENADE_PINEAPPLE);
			client->ps.ammoclip[BG_FindClipForWeapon(WP_GRENADE_PINEAPPLE)] = ltNades;
			break;
		case TEAM_RED:
			COM_BitSet(client->ps.weapons, WP_GRENADE_LAUNCHER);
			client->ps.ammoclip[BG_FindClipForWeapon(WP_GRENADE_LAUNCHER)] = ltNades;
			break;
		default:
			COM_BitSet(client->ps.weapons, WP_GRENADE_PINEAPPLE);
			client->ps.ammoclip[BG_FindClipForWeapon(WP_GRENADE_PINEAPPLE)] = ltNades;
			break;
		}
	}

	// Everyone gets a pistol
	switch (client->sess.sessionTeam) { // JPW NERVE was playerPistol

	case TEAM_RED: // JPW NERVE
		COM_BitSet(client->ps.weapons, WP_LUGER);
		client->ps.ammoclip[BG_FindClipForWeapon(WP_LUGER)] += 8;
		client->ps.ammo[BG_FindAmmoForWeapon(WP_LUGER)] += gunClips * 8;
		client->ps.weapon = WP_LUGER;
		break;
	default: // '0' // TEAM_BLUE
		COM_BitSet(client->ps.weapons, WP_COLT);
		client->ps.ammoclip[BG_FindClipForWeapon(WP_COLT)] += 8;
		client->ps.ammo[BG_FindAmmoForWeapon(WP_COLT)] += gunClips * 8;
		client->ps.weapon = WP_COLT;
		break;
	}

	switch (client->sess.sessionTeam) { // was playerItem		
		int nades;

	case TEAM_BLUE:
		COM_BitSet(client->ps.weapons, WP_GRENADE_PINEAPPLE);
		client->ps.ammo[BG_FindAmmoForWeapon(WP_GRENADE_PINEAPPLE)] = 0;
		if (pc == PC_LT) nades = ltNades;
		else if (pc == PC_ENGINEER) nades = engNades;
		else if (pc == PC_MEDIC) nades = medNades;
		else if (pc == PC_SOLDIER) nades = soldNades;
		else nades = 1;
		client->ps.ammoclip[BG_FindClipForWeapon(WP_GRENADE_PINEAPPLE)] = nades;
		break;
	case TEAM_RED:
		COM_BitSet(client->ps.weapons, WP_GRENADE_LAUNCHER);
		client->ps.ammo[BG_FindAmmoForWeapon(WP_GRENADE_LAUNCHER)] = 0;
		if (pc == PC_LT) nades = ltNades;
		else if (pc == PC_ENGINEER) nades = engNades;
		else if (pc == PC_MEDIC) nades = medNades;
		else if (pc == PC_SOLDIER) nades = soldNades;
		else nades = 1;
		client->ps.ammoclip[BG_FindClipForWeapon(WP_GRENADE_LAUNCHER)] = nades;
		break;
	default:
		COM_BitSet(client->ps.weapons, WP_GRENADE_PINEAPPLE);
		client->ps.ammo[BG_FindAmmoForWeapon(WP_GRENADE_PINEAPPLE)] = 0;
		if (pc == PC_LT) nades = ltNades;
		else if (pc == PC_ENGINEER) nades = engNades;
		else if (pc == PC_MEDIC) nades = medNades;
		else if (pc == PC_SOLDIER) nades = soldNades;
		else nades = 1;
		client->ps.ammoclip[BG_FindClipForWeapon(WP_GRENADE_PINEAPPLE)] = nades;
		break;
	}

	// JPW NERVE
	if (pc == PC_MEDIC) {
		COM_BitSet(client->ps.weapons, WP_MEDIC_SYRINGE);
		client->ps.ammoclip[BG_FindClipForWeapon(WP_MEDIC_SYRINGE)] = g_syringes.integer;

		// NERVE - SMF
		COM_BitSet(client->ps.weapons, WP_MEDKIT);
		client->ps.ammoclip[BG_FindClipForWeapon(WP_MEDKIT)] = 1;
		client->ps.ammo[WP_MEDKIT] = 1;

		if (g_medicAmmoPack.integer &&
			g_forceClass.integer == PC_MEDIC)
		{
			COM_BitSet(client->ps.weapons, WP_AMMO);
			client->ps.ammo[BG_FindAmmoForWeapon(WP_AMMO)] = 0;
			client->ps.ammoclip[BG_FindClipForWeapon(WP_AMMO)] = 1;
		}
	}
	// jpw

	if (!g_ffa.integer)
	{
		// Soldiers and Lieutenants get a 2-handed weapon
		if (pc == PC_SOLDIER || pc == PC_LT) {

			// JPW NERVE -- if LT is selected but illegal weapon, set to team-specific SMG
			if ((pc == PC_LT) && (client->sess.playerWeapon > 5)) {
				if (client->sess.sessionTeam == TEAM_RED) {
					client->sess.playerWeapon = SW_MP40;
				}
				else {
					client->sess.playerWeapon = SW_THOMPSON;
				}
			}
			// jpw
			switch (client->sess.playerWeapon) {

			case SW_MP40:
				COM_BitSet(client->ps.weapons, WP_MP40);
				client->ps.ammoclip[BG_FindClipForWeapon(WP_MP40)] += 32;
				if (pc == PC_SOLDIER) {
					client->ps.ammo[BG_FindAmmoForWeapon(WP_MP40)] += (32 * soldClips);
				}
				else {
					client->ps.ammo[BG_FindAmmoForWeapon(WP_MP40)] += (32 * ltClips);
				}
				client->ps.weapon = WP_MP40;
				break;

			case SW_THOMPSON:
				COM_BitSet(client->ps.weapons, WP_THOMPSON);
				client->ps.ammoclip[BG_FindClipForWeapon(WP_THOMPSON)] += 30;
				if (pc == PC_SOLDIER) {
					client->ps.ammo[BG_FindAmmoForWeapon(WP_THOMPSON)] += (30 * soldClips);
				}
				else {
					client->ps.ammo[BG_FindAmmoForWeapon(WP_THOMPSON)] += (30 * ltClips);
				}
				client->ps.weapon = WP_THOMPSON;
				break;

			case SW_STEN:
				COM_BitSet(client->ps.weapons, WP_STEN);
				client->ps.ammoclip[BG_FindClipForWeapon(WP_STEN)] += 32;
				if (pc == PC_SOLDIER) {
					client->ps.ammo[BG_FindAmmoForWeapon(WP_STEN)] += (32 * soldClips);
				}
				else {
					client->ps.ammo[BG_FindAmmoForWeapon(WP_STEN)] += (32 * ltClips);
				}
				client->ps.weapon = WP_STEN;
				break;

			case SW_MAUSER:
				if (pc != PC_SOLDIER) {
					return;
				}
				if (isWeaponLimited(client, client->sess.playerWeapon)) {
					CPx(clientNum, va("cp \"Sniper limit(^3%d^7) has been reached.\n Please select a different weapon.\n\"2", g_maxTeamSniper.integer));
					setDefaultWeapon(client, qtrue);
					return;
				}
				else if (!isWeaponBalanced(SW_MAUSER)) {
					CPx(clientNum, va("cp \"At least %d players per team required to spawn with Sniper rifle.\n\"2", g_balanceSniper.integer));
					setDefaultWeapon(client, qtrue);
					return;
				}
				else {
					(client->sess.sessionTeam == TEAM_RED) ? level.axisSniper++ : level.alliedSniper++;
					client->pers.restrictedWeapon = WP_MAUSER;
				}
				COM_BitSet(client->ps.weapons, WP_SNIPERRIFLE);
				client->ps.ammoclip[BG_FindClipForWeapon(WP_SNIPERRIFLE)] = 10;
				client->ps.ammo[BG_FindAmmoForWeapon(WP_SNIPERRIFLE)] = 10;
				client->ps.weapon = WP_SNIPERRIFLE;

				COM_BitSet(client->ps.weapons, WP_MAUSER);
				client->ps.ammoclip[BG_FindClipForWeapon(WP_MAUSER)] = 10;
				client->ps.ammo[BG_FindAmmoForWeapon(WP_MAUSER)] = (mauserClips * 10);
				client->ps.weapon = WP_MAUSER;
				break;

			case SW_PANZER:
				if (pc != PC_SOLDIER) {
					return;
				}
				if (isWeaponLimited(client, client->sess.playerWeapon)) {
					CPx(clientNum, va("cp \"PF limit(^3%d^7) has been reached.\n Please select a different weapon.\n\"2", g_maxTeamPF.integer));
					setDefaultWeapon(client, qtrue);
					return;
				}
				else if (!isWeaponBalanced(SW_PANZER)) {
					CPx(clientNum, va("cp \"At least %d players per team required to spawn with PanzerFaust.\n\"2", g_balancePF.integer));
					setDefaultWeapon(client, qtrue);
					return;
				}
				else {
					(client->sess.sessionTeam == TEAM_RED) ? level.axisPF++ : level.alliedPF++;
					client->pers.restrictedWeapon = WP_PANZERFAUST;
				}
				COM_BitSet(client->ps.weapons, WP_PANZERFAUST);
				client->ps.ammo[BG_FindAmmoForWeapon(WP_PANZERFAUST)] = 4;
				client->ps.weapon = WP_PANZERFAUST;
				break;

			case SW_VENOM:
				if (pc != PC_SOLDIER) {
					return;
				}
				if (isWeaponLimited(client, client->sess.playerWeapon)) {
					CPx(clientNum, va("cp \"Venom limit(^3%d^7) has been reached.\n Please select a different weapon.\n\"2", g_maxTeamVenom.integer));
					setDefaultWeapon(client, qtrue);
					return;
				}
				else if (!isWeaponBalanced(SW_VENOM)) {
					CPx(clientNum, va("cp \"At least %d players per team required to spawn with Venom.\n\"2", g_balanceVenom.integer));
					setDefaultWeapon(client, qtrue);
					return;
				}
				else {
					(client->sess.sessionTeam == TEAM_RED) ? level.axisVenom++ : level.alliedVenom++;
					client->pers.restrictedWeapon = WP_VENOM;
				}
				COM_BitSet(client->ps.weapons, WP_VENOM);
				client->ps.ammoclip[BG_FindAmmoForWeapon(WP_VENOM)] = 500;
				client->ps.ammo[BG_FindAmmoForWeapon(WP_VENOM)] += (venomClips * 500);
				client->ps.weapon = WP_VENOM;
				break;

			case SW_FLAMER:
				if (pc != PC_SOLDIER) {
					return;
				}
				if (isWeaponLimited(client, client->sess.playerWeapon)) {
					CPx(clientNum, va("cp \"Flamer limit(^3%d^7) has been reached.\n Please select a different weapon.\n\"2", g_maxTeamFlamer.integer));
					setDefaultWeapon(client, qtrue);
					return;
				}
				else if (!isWeaponBalanced(SW_FLAMER)) {
					CPx(clientNum, va("cp \"At least %d players per team required to spawn with Flamerthrower.\n\"2", g_balanceFlamer.integer));
					setDefaultWeapon(client, qtrue);
					return;
				}
				else {
					(client->sess.sessionTeam == TEAM_RED) ? level.axisFlamer++ : level.alliedFlamer++;
					client->pers.restrictedWeapon = WP_FLAMETHROWER;
				}
				COM_BitSet(client->ps.weapons, WP_FLAMETHROWER);
				client->ps.ammoclip[BG_FindAmmoForWeapon(WP_FLAMETHROWER)] = 200;
				client->ps.weapon = WP_FLAMETHROWER;
				break;

			default:    // give MP40 if given invalid weapon number
				if (client->sess.sessionTeam == TEAM_RED) { // JPW NERVE
					COM_BitSet(client->ps.weapons, WP_MP40);
					client->ps.ammoclip[BG_FindClipForWeapon(WP_MP40)] += 32;
					if (pc == PC_SOLDIER) {
						client->ps.ammo[BG_FindAmmoForWeapon(WP_MP40)] += (32 * soldClips);
					}
					else {
						client->ps.ammo[BG_FindAmmoForWeapon(WP_MP40)] += (30 * ltClips);
					}
					client->ps.weapon = WP_MP40;
				}
				else { // TEAM_ALLIED
					COM_BitSet(client->ps.weapons, WP_THOMPSON);
					client->ps.ammoclip[BG_FindClipForWeapon(WP_THOMPSON)] += 30;
					if (pc == PC_SOLDIER) {
						client->ps.ammo[BG_FindAmmoForWeapon(WP_THOMPSON)] += (30 * soldClips);
					}
					else {
						client->ps.ammo[BG_FindAmmoForWeapon(WP_THOMPSON)] += (30 * ltClips);
					}
					client->ps.weapon = WP_THOMPSON;
				}
				break;
			}
		}
		else { // medic or engineer gets assigned MP40 or Thompson with one magazine ammo
			setDefaultWeapon(client, qfalse);
		}
	}

	client->pers.playerWeapon = SwToWp(client->sess.playerWeapon);

	CheckMaxHealth();
}
// dhm - end

/*
==================
G_CheckForExistingModelInfo

  If this player model has already been parsed, then use the existing information.
  Otherwise, set the modelInfo pointer to the first free slot.

  returns qtrue if existing model found, qfalse otherwise
==================
*/
qboolean G_CheckForExistingModelInfo( gclient_t *cl, char *modelName, animModelInfo_t **modelInfo )
{
	int i;
	animModelInfo_t *trav, *firstFree=NULL;
	gclient_t *cl_trav;
	char modelsUsed[MAX_ANIMSCRIPT_MODELS];

	for ( i=0, trav=level.animScriptData.modelInfo; i<MAX_ANIMSCRIPT_MODELS; i++, trav++ ) {
		if ( trav->modelname[0] ) {
			if ( !Q_stricmp( trav->modelname, modelName ) ) {
				// found a match, use this modelinfo
				*modelInfo = trav;
				level.animScriptData.clientModels[cl->ps.clientNum] = i+1;
				return qtrue;
			}
		} else if (!firstFree) {
			firstFree = trav;
			level.animScriptData.clientModels[cl->ps.clientNum] = i+1;
		}
	}

	// set the modelInfo to the first free slot
	if (!firstFree) {
		// attempt to free a model that is no longer being used
		memset( modelsUsed, 0, sizeof(modelsUsed) );
		for ( i=0, cl_trav=level.clients; i<MAX_CLIENTS; i++, cl_trav++ ) {
			if (cl_trav != cl && g_entities[cl_trav->ps.clientNum].inuse && cl_trav->modelInfo) {
				modelsUsed[ (int)(cl_trav->modelInfo - level.animScriptData.modelInfo) ] = 1;
			}
		}
		// now use the first slot that isn't being utilized
		for ( i=0, trav=level.animScriptData.modelInfo; i<MAX_ANIMSCRIPT_MODELS; i++, trav++ ) {
			if (!modelsUsed[i]) {
				firstFree = trav;
				level.animScriptData.clientModels[cl->ps.clientNum] = i+1;
				break;
			}
		}
	}

	if (!firstFree) {
		G_Error( "unable to find a free modelinfo slot, cannot continue\n" );
	} else {
		*modelInfo = firstFree;
		// clear the structure out ready for use
		memset( *modelInfo, 0, sizeof(*modelInfo) );
	}
	// qfalse signifies that we need to parse the information from the script files
	return qfalse;
}

/*
=============
G_ParseAnimationFiles
=============
*/
static	char		text[100000];			// <- was causing callstacks >64k

qboolean G_ParseAnimationFiles( char *modelname, gclient_t *cl )
{
	char		filename[MAX_QPATH];
	fileHandle_t	f;
	int			len;

	// set the name of the model in the modelinfo structure
	Q_strncpyz( cl->modelInfo->modelname, modelname, sizeof(cl->modelInfo->modelname) );

	// load the cfg file
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/wolfanim.cfg", modelname );
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		G_Printf( "G_ParseAnimationFiles(): file '%s' not found\n", filename );		//----(SA)	added
		return qfalse;
	}
	if ( len >= sizeof( text ) - 1 ) {
		G_Printf( "File %s too long\n", filename );
		return qfalse;
	}
	trap_FS_Read( text, len, f );
	text[len] = 0;
	trap_FS_FCloseFile( f );

	// parse the text
	BG_AnimParseAnimConfig( cl->modelInfo, filename, text );

	// load the script file
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/wolfanim.script", modelname );
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		if (cl->modelInfo->version > 1) {
			return qfalse;
		}
		// try loading the default script for old legacy models
		Com_sprintf( filename, sizeof( filename ), "models/players/default.script", modelname );
		len = trap_FS_FOpenFile( filename, &f, FS_READ );
		if ( len <= 0 ) {
			return qfalse;
		}
	}
	if ( len >= sizeof( text ) - 1 ) {
		G_Printf( "File %s too long\n", filename );
		return qfalse;
	}
	trap_FS_Read( text, len, f );
	text[len] = 0;
	trap_FS_FCloseFile( f );

	// parse the text
	BG_AnimParseAnimScript( cl->modelInfo, &level.animScriptData, cl->ps.clientNum, filename, text );

	// ask the client to send us the movespeeds if available
	if (g_gametype.integer == GT_SINGLE_PLAYER && g_entities[0].client && g_entities[0].client->pers.connected == CON_CONNECTED) {
		trap_SendServerCommand( 0, va("mvspd %s", modelname) );
	}

	return qtrue;
}

/*
===========
L0 - Save players IP
============
*/
void SaveIP_f(gclient_t *client, char *sip) 
{
	if (strcmp(sip, "localhost") == 0 || sip == NULL) {
		client->sess.ip = 0;
		client->pers.localClient = qtrue;
		return;
	}

	client->sess.ip = G_GetPackedIpAddress(sip);
}

/*
===========
L0 - Print client's IP
============
*/
char *clientIP(gentity_t *ent, qboolean full)
{
	if (ent->r.svFlags & SVF_BOT)
		return "bot";

	return G_GetUnpackedIpAddress(ent->client->sess.ip, full);
}

/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/
void ClientUserinfoChanged( int clientNum ) {
	gentity_t *ent;
	char	*s;
	char	model[MAX_QPATH], modelname[MAX_QPATH];

//----(SA) added this for head separation
	char	head[MAX_QPATH];

	char	oldname[MAX_STRING_CHARS];
	gclient_t	*client;
	char	*c1;
	char	userinfo[MAX_INFO_STRING];
	char	*result;
	char	ip[MAX_STRING_TOKENS];

	ent = g_entities + clientNum;
	client = ent->client;

	ent->s.clientNum = clientNum;
	client->ps.clientNum = clientNum;

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
	trap_GetClientIp(clientNum, ip, sizeof(ip));

	result = Info_Validate(userinfo);
	if (result) {
		trap_LogToFile(CRITICALLOG, va("Time: %s (%d)\nPlayer IP: %s\n%s Userinfo:\n%s%s", getDateTime(), level.time, ip, result, userinfo, LOGLINE));
		trap_DropClient(clientNum, result);
		return;
	}

	// L0 - save IP for getstatus..
	if (ip[0] != 0) {
		SaveIP_f(client, ip);
	}

	// check the item prediction
	s = Info_ValueForKey( userinfo, "cg_predictItems" );
	if ( !atoi( s ) ) {
		client->pers.predictItemPickup = qfalse;
	} else {
		client->pers.predictItemPickup = qtrue;
	}

	// check the auto activation
	s = Info_ValueForKey( userinfo, "cg_autoactivate" );
	if ( !atoi( s ) ) {
		client->pers.autoActivate = PICKUP_ACTIVATE;
	} else {
		client->pers.autoActivate = PICKUP_TOUCH;
	}

	// check the auto empty weapon switching
	s = Info_ValueForKey( userinfo, "cg_emptyswitch" );
	if ( !atoi( s ) ) {
		client->pers.emptySwitch = 0;
	} else {
		client->pers.emptySwitch = 1;
	}

	s = Info_ValueForKey(userinfo, "sex");
	if (!Q_stricmp(s, "female")) {
		ent->client->sess.gender = 1;
	} else {
		ent->client->sess.gender = 0;
	}

	if (client->sess.secretlyDemoing) {
		Info_SetValueForKey(userinfo, "cl_hidden", "1");
		trap_SetUserinfo(clientNum, userinfo);
	}

	// set name
	Q_strncpyz ( oldname, client->pers.netname, sizeof( oldname ) );
	s = Info_ValueForKey(userinfo, "name");
	int colorlessNameLength = Q_SanitizeClientTextInput(s, client->pers.netname, sizeof(client->pers.netname), qtrue);
	if (client->pers.netname[0] == '\0' || colorlessNameLength == 0) {
		Q_strncpyz(client->pers.netname, "UnnamedPlayer", sizeof(client->pers.netname));
	}
	trap_SetClientName(clientNum, client->pers.netname);

	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			Q_strncpyz( client->pers.netname, "scoreboard", sizeof(client->pers.netname) );
		}
	}

	if (client->pers.connected == CON_CONNECTED && !level.intermissiontime && !client->sess.secretlyDemoing) {
		if (strcmp(oldname, client->pers.netname)) {
			// L0 - disallowed names
			if (g_disallowedNames.string[0]) {
				// returns true if they're kicked
				if (G_CensorName(client->pers.netname, userinfo, clientNum))
					return;
			}
			// L0 - Name locking prevents name changes			
			if (client->pers.nameLocked) {
				Q_strncpyz(client->pers.netname, oldname, sizeof(client->pers.netname));
				Info_SetValueForKey(userinfo, "name", oldname);
				trap_SetUserinfo(clientNum, userinfo);
				CPx(ent->s.clientNum, "cp \"^1Denied! ^7Admin has revoked your ability to rename for this round^1!\n\"2");
				return;
			}

			AP( va("print \"%s" S_COLOR_WHITE " renamed to %s\n\"", oldname, client->pers.netname) );
		}
	}

	CheckMaxHealth();

	// set model
	if ( g_forceModel.integer ) {
		Q_strncpyz( model, DEFAULT_MODEL, sizeof( model ) );
		Q_strcat( model, sizeof(model), "/default" );
	} else {
		Q_strncpyz( model, Info_ValueForKey (userinfo, "model"), sizeof( model ) );
	}

	/*

	Nobo - Same thing as above. Duplicate userinfo packets being sent and
	causing a userinfochanged when nothing has actually changed.. Which
	in-turn triggers these animation resets when they should NOT happen.
	(they shouldn't happen period.. these two lines of code don't exist in
	the original Q3 source code..)

	This is what caused the infamous "back break/walking revive" bug.

	// RF, reset anims so client's dont freak out
	client->ps.legsAnim = 0;
	client->ps.torsoAnim = 0;

	*/

	// DHM - Nerve :: Forcibly set both model and skin for multiplayer.
	if ( g_gametype.integer >= GT_WOLF ) {

		// To communicate it to cgame
		if (ent->r.svFlags & SVF_BOT) {
			s = Info_ValueForKey(userinfo, "class");
			client->ps.stats[STAT_PLAYER_CLASS] = client->sess.playerType = client->sess.latchPlayerType =
				(g_forceClass.integer == -1) ? atoi(s) : g_forceClass.integer;
		}
		else {
			client->ps.stats[STAT_PLAYER_CLASS] = client->sess.playerType;
		}

		if ( client->sess.sessionTeam == TEAM_BLUE )
			Q_strncpyz( model, MULTIPLAYER_ALLIEDMODEL, MAX_QPATH );
		else
			Q_strncpyz( model, MULTIPLAYER_AXISMODEL, MAX_QPATH );

		Q_strcat(model, MAX_QPATH, "/");

		SetWolfSkin( client, model );

		Q_strncpyz( head, "", MAX_QPATH );
		SetWolfSkin( client, head );
	}

	// strip the skin name
	Q_strncpyz( modelname, model, sizeof(modelname) );
	if (strstr(modelname, "/")) {
		modelname[ strstr(modelname, "/") - modelname ] = 0;
	} else if (strstr(modelname, "\\")) {
		modelname[ strstr(modelname, "\\") - modelname ] = 0;
	}

	if ( !G_CheckForExistingModelInfo( client, modelname, &client->modelInfo ) ) {
		if ( !G_ParseAnimationFiles( modelname, client ) ) {
			G_Error( "Failed to load animation scripts for model %s\n", modelname );
		}
	}

	// team`
	// DHM - Nerve :: Already took care of models and skins above
	if ( g_gametype.integer < GT_WOLF ) {

		//----(SA) added this for head separation
		// set head
		if ( g_forceModel.integer ) {
			Q_strncpyz( head, DEFAULT_HEAD, sizeof( head ) );
		} else {
			Q_strncpyz( head, Info_ValueForKey (userinfo, "head"), sizeof( head ) );
		}

		//----(SA) end

		switch (client->sess.sessionTeam) {
		case TEAM_RED:
			ForceClientSkin(client, model, "red");
			break;
		case TEAM_BLUE:
			ForceClientSkin(client, model, "blue");
			break;
		default: // TEAM_FREE, TEAM_SPECTATOR, TEAM_NUM_TEAMS not handled in switch
			break;
		}

		if ( g_gametype.integer >= GT_TEAM && client->sess.sessionTeam == TEAM_SPECTATOR ) {
			// don't ever use a default skin in teamplay, it would just waste memory
			ForceClientSkin(client, model, "red");
		}

	}
	//dhm - end


	// colors
	c1 = Info_ValueForKey( userinfo, "color" );
	int color = atoi(c1);

	if (client->sess.colorFlags != color) {
		client->sess.selectedWeapon = (color & CF_USE_STEN) ?
			WP_STEN : (color & CF_USE_THOMPSON) ?
			WP_THOMPSON : (color & CF_USE_MP40) ?
			WP_MP40 : client->sess.selectedWeapon;
	}

	client->sess.colorFlags = color;

	s = Info_ValueForKey(userinfo, "cg_autoReload");
	if (client->sess.colorFlags & CF_AUTORELOAD_OFF || (strlen(s) && !atoi(s))){		// If a client has the right color flag set...
		client->ps.noReload = qtrue;				// Do NOT autoreload their weapon
		client->pers.noReload = qtrue;
	} else {
		client->ps.noReload = qfalse;					// Else - reload it for them
		client->pers.noReload = qfalse;
	}

	// send over a subset of the userinfo keys so other clients can
	// print scoreboards, display models, and play custom sounds

//----(SA) modified these for head separation

	if (ent->r.svFlags & SVF_BOT) {
		s = va("n\\%s\\t\\%i\\model\\%s\\head\\%s\\c1\\%d\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s\\uci\\%i",
			//s = va("n\\%s\\t\\%i\\model\\%s\\head\\%s\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s\\uci\\%u",
			client->pers.netname, client->sess.sessionTeam, model, head, 0,
			client->pers.maxHealth, client->sess.wins, client->sess.losses,
			Info_ValueForKey(userinfo, "skill"), client->sess.uci /* L0 - GeoIP */);
	}
	else {
		s = va("n\\%s\\t\\%i\\model\\%s\\head\\%s\\c1\\%d\\hc\\%i\\w\\%i\\l\\%i\\uci\\%i",
			client->pers.netname, client->sess.sessionTeam, model, head, 0,
			client->pers.maxHealth, client->sess.wins, client->sess.losses, client->sess.uci /* L0 - GeoIP */); //Elver GeoIP
	}

//----(SA) end

	trap_SetConfigstring( CS_PLAYERS+clientNum, s );

	// L0 - We need to send client private info (ip) only to log..not a configstring
	// as \configstrings reveals user data which is something we don't want..
	if (!(ent->r.svFlags & SVF_BOT)) {
		char *team, *status;

		team = (client->sess.sessionTeam == TEAM_RED) ? "Axis" :
			((client->sess.sessionTeam == TEAM_BLUE) ? "Allied" : "Spectator");

		status = (client->sess.admin ? va("admin_level_%d", client->sess.admin) :
			(client->sess.ignored > IGNORE_OFF ? "ignored" : "none"));

		// Print to log only essentials and skip garbage
		s = va("slot\\%i\\name\\%s\\team\\%s\\IP\\%s\\status\\%s",
			clientNum, client->pers.netname, team,
			clientIP(&g_entities[clientNum], qtrue), status
		);
	}

	G_LogPrintf( "ClientUserinfoChanged (%s)(%d): %s\n%s\n", ip, level.time, s, userinfo);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	char		*value;
	gclient_t	*client;
	char		userinfo[MAX_INFO_STRING];
	gentity_t	*ent;
	int			i;
	char		*result;
	char	ip[21];
	char	ip_without_port[16];

	ent = &g_entities[ clientNum ];

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
	trap_GetClientIp(clientNum, ip, sizeof(ip));

	result = Info_Validate(userinfo);
	if (result) {
		trap_LogToFile(CRITICALLOG, va("Time: %s (%d)\nPlayer IP: %s\n%s Userinfo:\n%s%s", getDateTime(), level.time, ip, result, userinfo, LOGLINE));
		return va("%s\n", result);
	}

	// L0 - ASCII name bug crap..
	value = Info_ValueForKey(userinfo, "name");
	for (i = 0; i < strlen(value); i++) {
		if (value[i] < 0) {
			// extended ASCII chars have values between -128 and 0 (signed char)
			return "Change your name, extended ASCII chars are ^1NOT ^7allowed!";
		}
	}

	// check to see if they are on the banned IP list
	value = Info_ValueForKey(userinfo, "password");

	// L0 - Banned & Tempbanned
	if (firstTime && !isBot)
	{
		Q_strncpyz(ip_without_port, ip, sizeof(ip_without_port));
		char *port = strchr(ip_without_port, ':');
		if (port)
		{
			*port = 0;
		}

		if (IsBanned(ip_without_port, value))
		{
			return bannedMSG.string;
		}

		result = IsTempBanned(ip_without_port);
		if (result)
		{
			return result;
		}
	}
		
	// we don't check password for bots and local client
	// NOTE: local client <-> "ip" "localhost"
	//   this means this client is not running in our current process
	if ( !( ent->r.svFlags & SVF_BOT ) && (strcmp(ip, "localhost") != 0)) {
		// check for a password
		value = Info_ValueForKey (userinfo, "password");
		if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) &&
			strcmp( g_password.string, value) != 0) {
			return "Invalid password";
		}
	}

	// L0 - disallowed names
	if (g_disallowedNames.string[0]) {
		char censoredName[MAX_NETNAME];
		char *line;

		value = Info_ValueForKey(userinfo, "name");
		SanitizeString((char *)value, censoredName, qtrue);
		if (G_CensorText(censoredName, &censorNamesDictionary)) {
			Info_SetValueForKey(userinfo, "name", censoredName);
			trap_SetUserinfo(clientNum, userinfo);
			line = va("\n^7Name ^3%s ^7is not allowed^1!", value);
			return va("%s\n", line);
		}
	}

	// they can connect
	ent->client = level.clients + clientNum;
	client = ent->client;

	memset( client, 0, sizeof(*client) );

	client->pers.connected = CON_CONNECTING;
	client->pers.connectTime = level.time;			// DHM - Nerve

	if (firstTime) {
		client->pers.initialSpawn = qtrue;				// DHM - Nerve
	}

	client->pers.complaints = 0;					// DHM - Nerve

	// read or initialize the session data
	if ( firstTime || (g_gametype.integer < GT_WOLF && level.newSession) ) {
		G_InitSessionData( client, userinfo );
	}
	G_ReadSessionData( client );

	if( isBot ) {
		ent->r.svFlags |= SVF_BOT;
		ent->inuse = qtrue;
		
		if (!firstTime) {
			Info_SetValueForKey(userinfo, "team", TeamName(client->sess.sessionTeam));
			trap_SetUserinfo(clientNum, userinfo);
		}

		if( !G_BotConnect( clientNum, !firstTime ) ) {
			return "Bot connect failed..";
		}
	}

	// Elver GeoIP add
	// L0 - mcwf's GeoIP
	if (gidb != NULL) {
		// L0 - bug fix :)
		// Since we lose ip after initial connection, flags get resetted
		// as soon as client info changes (renames etc..) so we get "unknown" flag.
		// This fixes the issue, although it can happen that if client connects
		// few seconds when map get's resetted or restarted, he ends up with ""unknown"
		// location, but that gets fixed as soon as game ends/map restarts etc...

		//value = Info_ValueForKey(userinfo, "ip");

		if (!strcmp(value, "localhost")) {
			client->sess.uci = 0;
		}
		if (firstTime) {
			Info_SetValueForKey(userinfo, "ip", ip_without_port);
			value = Info_ValueForKey(userinfo, "ip");
			value = ip_without_port;
		}
		else {
			value = G_GetUnpackedIpAddress(client->sess.ip, qtrue);

		}

		if (!strcmp(value, "localhost")) {
			client->sess.uci = 0;
		}
		else
		{
			unsigned long ip = GeoIP_addr_to_num(value);
			if (((ip & 0xFF000000) == 0x0A000000) ||
				((ip & 0xFFF00000) == 0xAC100000) ||
				((ip & 0xFFFF0000) == 0xC0A80000)) {
				client->sess.uci = 0;
			}
			else
			{
				unsigned int ret = GeoIP_seek_record(gidb, ip);
				if (ret > 0) {
					client->sess.uci = ret;
				}
				else {
					client->sess.uci = 246;
					G_LogPrintf("GeoIP: This IP:%s cannot be located\n", value);
				}
			}
		}
	}
	else { // gidb == NULL
		client->sess.uci = 255; //Don't draw anything if DB error
	}
	// L0 - end

	// get and distribute relevent paramters
	G_LogPrintf( "ClientConnect: %i\n", clientNum );
	ClientUserinfoChanged( clientNum );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime && !isBot ) {
		// L0 - Print always..
		AP(va("print \"%s" S_COLOR_WHITE " connected\n\"", client->pers.netname));

		// L0 - connect message
		if (strlen(g_serverMessage.string))
			CP(va("cp \"%s\n\"3", g_serverMessage.string));
	}

	// count current clients and rank for scoreboard
	CalculateRanks();

	return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin( int clientNum ) {
	gentity_t	*ent;
	gclient_t	*client;
	int			flags, ping;
	int			spawn_count;		// DHM - Nerve
	// for fps restoration after ps memset
	int			msec_samples[MAX_MSEC_SAMPLES];
	int			msec_samples_index;
	int			msec_averaged_samples[MAX_MSEC_SAMPLES];
	int			msec_averaged_samples_index;

	ent = g_entities + clientNum;

	if( ent->botDelayBegin ) {
		G_QueueBotBegin( clientNum );
		ent->botDelayBegin = qfalse;
		return;
	}

	client = level.clients + clientNum;

	if ( ent->r.linked ) {
		trap_UnlinkEntity( ent );
	}
	G_InitGentity( ent );
	ent->touch = 0;
	ent->pain = 0;
	ent->client = client;

	client->pers.connected = CON_CONNECTED;
	client->pers.enterTime = level.time;
	client->pers.teamState.state = TEAM_BEGIN;

	if (client->pers.beganPlayingTime <= 0)
	{
		client->pers.beganPlayingTime = level.time;
	}

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the new position
	// DHM - Nerve :: Also save PERS_SPAWN_COUNT, so that CG_Respawn happens
	spawn_count = client->ps.persistant[PERS_SPAWN_COUNT];
	flags = client->ps.eFlags;
	ping = client->ps.ping;
	// save fps/msec samples
	memcpy(msec_samples, client->ps.msec_samples, sizeof(msec_samples));
	msec_samples_index = client->ps.msec_samples_index;
	memcpy(msec_averaged_samples, client->ps.msec_averaged_samples, sizeof(msec_averaged_samples));
	msec_averaged_samples_index = client->ps.msec_averaged_samples_index;
	// clear ps
	memset( &client->ps, 0, sizeof( client->ps ) );
	// restore
	client->ps.eFlags = flags;
	client->ps.ping = ping;
	client->ps.persistant[PERS_SPAWN_COUNT] = spawn_count;
	// restore msec/fps samples
	memcpy(client->ps.msec_samples, msec_samples, sizeof(client->ps.msec_samples));
	client->ps.msec_samples_index = msec_samples_index;
	memcpy(client->ps.msec_averaged_samples, msec_averaged_samples, sizeof(client->ps.msec_averaged_samples));
	client->ps.msec_averaged_samples_index = msec_averaged_samples_index;

	// MrE: use capsule for collision
	//client->ps.eFlags |= EF_CAPSULE;
	//ent->r.svFlags |= SVF_CAPSULE;

	client->pers.complaintClient = -1;
	client->pers.complaintEndTime = -1;

// Mod
	// No Reload hack
	client->ps.noReload = client->pers.noReload; 
	// Shortcuts
	client->pers.lastkilled_client = -1;
	client->pers.lastammo_client = -1;
	client->pers.lasthealth_client = -1;
	client->pers.lastrevive_client = -1;
	client->pers.lastkiller_client = -1;
	// Stats
	client->pers.dmgGiven = 0;
	client->pers.dmgReceived = 0;
	client->pers.spreeDeaths = 0;
	// If temporarily ignored clear here (every map load or team change)
	if (client->sess.ignored == IGNORE_TEMPORARY) {
		if (!client->sess.secretlyDemoing) {
			client->sess.ignored = IGNORE_OFF;
		}
	}
// End

	// locate ent at a spawn point
	ClientSpawn( ent, qfalse );

// L0 - antilag
	G_ResetTrail(ent);
	ent->client->saved.leveltime = 0;
// End

	// Xian -- Changed below for team independant maxlives

	if (g_maxlives.integer > 0)
		ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = (g_maxlives.integer - 1);
	else
		ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = -1;

	if (g_axismaxlives.integer > 0 || g_alliedmaxlives.integer > 0) {
		if (client->sess.sessionTeam == TEAM_RED)
			ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = (g_axismaxlives.integer - 1);
		else if (client->sess.sessionTeam == TEAM_BLUE)
			ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = (g_alliedmaxlives.integer - 1);
		else
			ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = -1;
	}

	// DHM - Nerve :: Start players in limbo mode if they change teams during the match
	if ( g_gametype.integer >= GT_WOLF && client->sess.sessionTeam != TEAM_SPECTATOR 
		&& (level.time - client->pers.connectTime) > 60000 ) {
		ent->client->ps.pm_type = PM_DEAD;
		ent->r.contents = CONTENTS_CORPSE;
		ent->health = 0;
		ent->client->ps.stats[STAT_HEALTH] = 0;

		if ( g_maxlives.integer > 0 )
			ent->client->ps.persistant[PERS_RESPAWNS_LEFT]++;

		limbo( ent, qfalse );
	}

	// Ridah, trigger a spawn event
	// DHM - Nerve :: Only in single player
	if ( g_gametype.integer == GT_SINGLE_PLAYER && !(ent->r.svFlags & SVF_CASTAI) ) {
		AICast_ScriptEvent( AICast_GetCastState(clientNum), "spawn", "" );
	}

	G_LogPrintf( "ClientBegin: %i\n", clientNum );

	// count current clients and rank for scoreboard
	CalculateRanks();

}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
void ClientSpawn(gentity_t *ent, qboolean revived) {
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	clientPersistant_t	saved;
	clientSession_t		savedSess;
	int		persistant[MAX_PERSISTANT];
	gentity_t	*spawnPoint;
	int		flags;
	int		savedPing;
	int		savedTeam;
	// for fps restoration after ps memset
	int		msec_samples[MAX_MSEC_SAMPLES];
	int		msec_samples_index;
	int		msec_averaged_samples[MAX_MSEC_SAMPLES];
	int		msec_averaged_samples_index;
	qboolean savedVoted = qfalse;			// NERVE - SMF

	index = ent - g_entities;
	client = ent->client;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client

	if ( revived )
	{
		spawnPoint = ent;
		VectorCopy( ent->client->ps.origin, spawn_origin );
		if (g_useSpawnAnglesAfterRevive.integer) {
			VectorCopy(ent->client->pers.spawnAngles, spawn_angles);
		} else {
			VectorCopy(ent->client->ps.viewangles, spawn_angles);
		}
	}
	else
	{
		ent->aiName = "player";	// needed for script AI
		//ent->aiTeam = 1;		// member of allies
		//ent->client->ps.teamNum = ent->aiTeam;
		//AICast_ScriptParse( AICast_GetCastState(ent->s.number) );
	// done.

		if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
			spawnPoint = SelectSpectatorSpawnPoint ( 
				spawn_origin, spawn_angles);
		} else if (g_gametype.integer >= GT_TEAM) {
			if (g_ffa.integer)
			{
				spawnPoint = SelectCyclicSpawnPoint(spawn_origin, spawn_angles);
			}
			else
			{
				spawnPoint = SelectCTFSpawnPoint(
					client->sess.sessionTeam,
					client->pers.teamState.state,
					spawn_origin, spawn_angles, client->sess.spawnObjectiveIndex);
			}
		} else {
			do {
				// the first spawn should be at a good looking spot
				if ( !client->pers.initialSpawn && client->pers.localClient ) {
					client->pers.initialSpawn = qtrue;
					spawnPoint = SelectInitialSpawnPoint( spawn_origin, spawn_angles );
				} else {
					// don't spawn near existing origin if possible
					spawnPoint = SelectRandomSpawnPoint(
						client->ps.origin, 
						spawn_origin, spawn_angles);
				}

				if ( ( spawnPoint->flags & FL_NO_BOTS ) && ( ent->r.svFlags & SVF_BOT ) ) {
					continue;	// try again
				}
				// just to be symetric, we have a nohumans option...
				if ( ( spawnPoint->flags & FL_NO_HUMANS ) && !( ent->r.svFlags & SVF_BOT ) ) {
					continue;	// try again
				}

				break;

			} while ( 1 );
		}
	}

	client->pers.teamState.state = TEAM_ACTIVE;

	// toggle the teleport bit so the client knows to not lerp
	flags = ent->client->ps.eFlags & EF_TELEPORT_BIT;
	flags ^= EF_TELEPORT_BIT;
	// L0 - Fixes vote abuse by suicide and vote override..
	flags |= (client->ps.eFlags & EF_VOTED);

	// clear everything but the persistant data

	saved = client->pers;
	savedSess = client->sess;
	savedPing = client->ps.ping;
	savedTeam = client->ps.teamNum;
	// save fps/msec samples
	memcpy(msec_samples, client->ps.msec_samples, sizeof(msec_samples));
	msec_samples_index = client->ps.msec_samples_index;
	memcpy(msec_averaged_samples, client->ps.msec_averaged_samples, sizeof(msec_averaged_samples));
	msec_averaged_samples_index = client->ps.msec_averaged_samples_index;

	// NERVE - SMF
	if ( client->ps.eFlags & EF_VOTED )
		savedVoted = qtrue;

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		persistant[i] = client->ps.persistant[i];
	}

	memset (client, 0, sizeof(*client));

	client->pers = saved;
	client->sess = savedSess;
	client->ps.ping = savedPing;
	client->ps.teamNum = savedTeam;
	// restore msec/fps samples
	memcpy(client->ps.msec_samples, msec_samples, sizeof(client->ps.msec_samples));
	client->ps.msec_samples_index = msec_samples_index;
	memcpy(client->ps.msec_averaged_samples, msec_averaged_samples, sizeof(client->ps.msec_averaged_samples));
	client->ps.msec_averaged_samples_index = msec_averaged_samples_index;

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		client->ps.persistant[i] = persistant[i];
	}

	// increment the spawncount so the client will detect the respawn
	client->ps.persistant[PERS_SPAWN_COUNT]++;
	client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

	client->airOutTime = level.time + 12000;

	// clear entity values
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;
	client->ps.eFlags = flags;
	// MrE: use capsules for AI and player
	//client->ps.eFlags |= EF_CAPSULE;

	// NERVE - SMF
	if (savedVoted) {
		client->ps.eFlags |= EF_VOTED;
	}

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->client = &level.clients[index];
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	if (!(ent->r.svFlags & SVF_CASTAI))
		ent->classname = "player";
	ent->r.contents = CONTENTS_BODY;

	// RF, AI should be clipped by monsterclip brushes
	if (ent->r.svFlags & SVF_CASTAI)
		ent->clipmask = MASK_PLAYERSOLID|CONTENTS_MONSTERCLIP;
	else
		ent->clipmask = MASK_PLAYERSOLID;

	ent->client->animationInfo.bodyModelHandle = ent->client->sess.sessionTeam == TEAM_RED ?
		AXIS_MODEL_HANDLE : ALLIED_MODEL_HANDLE;

	// DHM - Nerve :: Init to -1 on first spawn;
	if ( !revived )
		ent->props_frame_state = -1;

	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = 0;

// L0 - New stuff	
	// Poison
	ent->poisoned = qfalse;	
	ent->lastPoisonTime = 0;	

	// Mapstats / Store Life kills Peak for map stats if enabled
	if (ent->client->pers.lifeKills > ent->client->pers.lifeKillsPeak)
		ent->client->pers.lifeKillsPeak = ent->client->pers.lifeKills;

	// Life Stats
	ent->client->pers.lifeKills = 0;
	ent->client->pers.lifeRevives = 0;
	ent->client->pers.lifeAcc_hits = 0;
	ent->client->pers.lifeAcc_shots = 0;
	ent->client->pers.lifeHeadshots = 0;
	// Dropped objective
	ent->droppedObj = qfalse;
	// Smoke
	ent->thrownSmoke = 0; 
	// No reload
	ent->client->ps.noReload = ent->client->pers.noReload;
// End
	
	VectorCopy (playerMins, ent->r.mins);
	VectorCopy (playerMaxs, ent->r.maxs);

	// Ridah, setup the bounding boxes and viewheights for prediction
	VectorCopy (ent->r.mins, client->ps.mins);
	VectorCopy (ent->r.maxs, client->ps.maxs);
	
	client->ps.crouchViewHeight = CROUCH_VIEWHEIGHT;
	client->ps.standViewHeight = DEFAULT_VIEWHEIGHT;
	client->ps.deadViewHeight = DEAD_VIEWHEIGHT;
	
	client->ps.crouchMaxZ = client->ps.maxs[2] - (client->ps.standViewHeight - client->ps.crouchViewHeight);

	client->ps.runSpeedScale = 0.8;
	client->ps.sprintSpeedScale = 1.1;
	client->ps.crouchSpeedScale = 0.25;

	// Rafael
	client->ps.sprintTime = 20000;
	client->ps.sprintExertTime = 0;

	client->ps.friction = 1.0;
	// done.

	client->ps.clientNum = index;
	ent->s.clientNum = index;

	trap_GetUsercmd( client - level.clients, &ent->client->pers.cmd );	// NERVE - SMF - moved this up here

	SetWolfUserVars( ent, NULL );			// NERVE - SMF

	// DHM - Nerve :: Add appropriate weapons
	if ( g_gametype.integer >= GT_WOLF ) {

		CheckWeaponRestrictions(client);

		if ( !revived ) {
			qboolean update = qfalse;

			if ( client->sess.playerType != client->sess.latchPlayerType )
				update = qtrue;

			client->sess.playerType = client->sess.latchPlayerType;
			client->sess.playerWeapon = client->sess.latchPlayerWeapon;
			client->sess.playerItem = client->sess.latchPlayerItem;
			client->sess.playerSkin = client->sess.latchPlayerSkin;

			if ( update )
				ClientUserinfoChanged( index );
		}

		// TTimo keep it isolated from spectator to be safe still
		if (client->sess.sessionTeam != TEAM_SPECTATOR)
		{
			// Xian - Moved the invul. stuff out of SetWolfSpawnWeapons and put it here for clarity
			if (g_fastres.integer == 1 && revived) {
				client->ps.powerups[PW_INVULNERABLE] = level.time + g_fastResMsec.integer;
			}
			else {
				if (revived)
				{
					// NOTE(nobo): This is what the INVUL and pm_time should be set to (actual animation duration).
					// However, that is 3100-3200 ms. It would solve some of the bs around revive, but I doubt people want longer revive time.
					// .. Maybe use g_disableInv cvar?? (shooting gun disables revive invul).
					//client->ps.powerups[PW_INVULNERABLE] = level.time + BG_GetAnimationDuration(&client->ps, ANIM_ET_REVIVE);
					client->ps.powerups[PW_INVULNERABLE] = level.time + 3000;
				}
				else
				{
					if (client->sess.sessionTeam == TEAM_RED)
						client->ps.powerups[PW_INVULNERABLE] = level.time + g_axisSpawnProtectionTime.integer;
					else if (client->sess.sessionTeam == TEAM_BLUE)
						client->ps.powerups[PW_INVULNERABLE] = level.time + g_alliedSpawnProtectionTime.integer;
					else
						client->ps.powerups[PW_INVULNERABLE] = level.time + 3000;
				}
			}
		}

		SetWolfSpawnWeapons( client ); // JPW NERVE -- increases stats[STAT_MAX_HEALTH] based on # of medics in game
	}
	// dhm - end

	// JPW NERVE ***NOTE*** the following line is order-dependent and must *FOLLOW* SetWolfSpawnWeapons() in multiplayer
	// SetWolfSpawnWeapons() now adds medic team bonus and stores in ps.stats[STAT_MAX_HEALTH].
	ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH];

	G_SetOrigin( ent, spawn_origin );
	VectorCopy( spawn_origin, client->ps.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;

	SetClientViewAngle( ent, spawn_angles );
	VectorCopy(spawn_angles, ent->client->pers.spawnAngles);

	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		//G_KillBox( ent );
		trap_LinkEntity (ent);
	}

	client->respawnTime = level.time;
	client->latched_buttons = 0;
	client->latched_wbuttons = 0;	//----(SA)	added

	if ( level.intermissiontime ) {
		MoveClientToIntermission( ent );
	} else {
		// fire the targets of the spawn point
		if ( !revived )
			G_UseTargets( spawnPoint, ent );
	}

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink( ent-g_entities );

	// positively link the client, even if the command times are weird
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
		VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
		trap_LinkEntity( ent );
	}

	// run the presend to set anything else
	ClientEndFrame( ent );

	// clear entity state values
	BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );

	// add the head entity if it already hasn't been
	AddHeadEntity(ent);
}


/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect( int clientNum ) {
	gentity_t	*ent;
	gentity_t	*flag=NULL;
	gitem_t		*item=NULL;
	vec3_t		launchvel;
	int			i;

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;
	}

	// stop any following clients
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].sess.sessionTeam == TEAM_SPECTATOR
			&& level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
			&& level.clients[i].sess.spectatorClient == clientNum ) {
			StopFollowing( &g_entities[i] );
		}
		if ( g_gametype.integer >= GT_WOLF
			&& level.clients[i].ps.pm_flags & PMF_LIMBO
			&& level.clients[i].sess.spectatorClient == clientNum ) {
			Cmd_FollowCycle_f( &g_entities[i], 1 );
		}
	}

	// NERVE - SMF - remove complaint client
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.complaintClient == clientNum ) {
			level.clients[i].pers.complaintClient = -1;
			level.clients[i].pers.complaintEndTime = 0;

			trap_SendServerCommand( i, "complaint -2" );
			break;
		}
	}

	// send effect if they were completely connected
	if ( ent->client->pers.connected == CON_CONNECTED 
		&& ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {

		// They don't get to take powerups with them!
		// Especially important for stuff like CTF flags
		TossClientItems ( ent );

		// New code for tossing flags
		if ( g_gametype.integer >= GT_WOLF ) {
			if (ent->client->ps.powerups[PW_REDFLAG]) {
				item = BG_FindItem("Red Flag");
				if (!item)
					item = BG_FindItem("Objective");

				ent->client->ps.powerups[PW_REDFLAG] = 0;
			}
			if (ent->client->ps.powerups[PW_BLUEFLAG]) {
				item = BG_FindItem("Blue Flag");
				if (!item)
					item = BG_FindItem("Objective");

				ent->client->ps.powerups[PW_BLUEFLAG] = 0;
			}

			if (item) {
				// L0 - Fix for docs exploit 				
				launchvel[0] = 0;
				launchvel[1] = 0;
				launchvel[2] = 0;
				// End

				flag = LaunchItem(item,ent->r.currentOrigin,launchvel,ent->s.number);
				flag->s.modelindex2 = ent->s.otherEntityNum2;// JPW NERVE FIXME set player->otherentitynum2 with old modelindex2 from flag and restore here
				flag->message = ent->message;	// DHM - Nerve :: also restore item name
				// Clear out player's temp copies
				ent->s.otherEntityNum2 = 0;
				ent->message = NULL;
			}
		}
	}

	// L0 - Weapon balancing
	if (ent->client->pers.restrictedWeapon == WP_MAUSER) {
		(ent->client->sess.sessionTeam == TEAM_RED)
			? level.axisSniper-- : level.alliedSniper--;
	}
	if (ent->client->pers.restrictedWeapon == WP_PANZERFAUST) {
		(ent->client->sess.sessionTeam == TEAM_RED)
			? level.axisPF-- : level.alliedPF--;
	}
	if (ent->client->pers.restrictedWeapon == WP_VENOM) {
		(ent->client->sess.sessionTeam == TEAM_RED)
			? level.axisVenom-- : level.alliedVenom--;
	}
	if (ent->client->pers.restrictedWeapon == WP_FLAMETHROWER) {
		(ent->client->sess.sessionTeam == TEAM_RED)
			? level.axisFlamer-- : level.alliedFlamer--;
	} // End

	if (g_automg42.integer)
	{
		G_Automg42Disassociate(ent);
	}

	stats_UpdateDailyStatsForClient(ent->client, NULL);

	G_LogPrintf( "ClientDisconnect: %i\n", clientNum );

	// if we are playing in tourney mode and losing, give a win to the other player
	if ( g_gametype.integer == GT_TOURNAMENT && !level.intermissiontime
		&& !level.warmupTime && level.sortedClients[1] == clientNum ) {
		level.clients[ level.sortedClients[0] ].sess.wins++;
		ClientUserinfoChanged( level.sortedClients[0] );
	}

	trap_UnlinkEntity (ent);
	ent->s.modelindex = 0;
	ent->inuse = qfalse;
	ent->classname = "disconnected";
	ent->client->pers.connected = CON_DISCONNECTED;
	ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
	ent->client->sess.sessionTeam = TEAM_FREE;
	ent->active = 0;

	FreeHeadEntity(ent);

	trap_SetConfigstring( CS_PLAYERS + clientNum, "");

	CalculateRanks();

	// L0 - sync teams
	if (g_teamAutoBalance.integer)
		checkEvenTeams();

	if ( ent->r.svFlags & SVF_BOT ) {
		BotAIShutdownClient( clientNum );
	}
}


/*
==================
G_RetrieveMoveSpeedsFromClient
==================
*/
void G_RetrieveMoveSpeedsFromClient( int entnum, char *text ) {
	char *text_p, *token;
	animation_t *anim;
	animModelInfo_t *modelInfo;

	text_p = text;

	// get the model name
	token = COM_Parse( &text_p );
	if (!token || !token[0]) {
		G_Error("G_RetrieveMoveSpeedsFromClient: internal error");
	}

	modelInfo = BG_ModelInfoForModelname( token );

	if (!modelInfo) {
		// ignore it
		return;
	}

	while (1) {
		token = COM_Parse( &text_p );
		if (!token || !token[0]) {
			break;
		}

		// this is a name
		anim = BG_AnimationForString( token, modelInfo );
		if (anim->moveSpeed == 0) {
			G_Error("G_RetrieveMoveSpeedsFromClient: trying to set movespeed for non-moving animation");
		}

		// get the movespeed
		token = COM_Parse( &text_p );
		if (!token || !token[0]) {
			G_Error("G_RetrieveMoveSpeedsFromClient: missing movespeed");
		}
		anim->moveSpeed = atoi( token );
	}
}

/*
===========
ClientGetFps
============
*/
int ClientGetFps(playerState_t *ps) {
	int msec = ClientGetMsec(ps);
	if (msec <= 0) {
		return 0;
	}
	return 1000 / msec;
}

/*
===========
ClientGetMsec
============
*/
int ClientGetMsec(playerState_t *ps) {
	int s;
	float average_msec = 0.0f;
	int sum = 0;
	int sum_count = 0;

	for (s = 0; s < ps->msec_samples_index; ++s) {
		if (ps->msec_samples[s] == 0) {
			continue;
		}
		sum += ps->msec_samples[s];
		sum_count++;
	}
	if (sum_count > 0) {
		sum /= sum_count;
		sum_count = 1;
	}

	for (s = 0; s < MAX_MSEC_SAMPLES; ++s) {
		if (ps->msec_averaged_samples[s] == 0) {
			continue;
		}
		sum += ps->msec_averaged_samples[s];
		sum_count++;
	}
	if (sum_count > 0) {
		average_msec = (float)sum / (float)sum_count;
		average_msec = roundf(average_msec);
		if (average_msec <= 0.0f) {
			average_msec = 0.0f;
		}
	}

	return (int)average_msec;
}