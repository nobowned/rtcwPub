
#include "g_local.h"

#include "ai_cast_fight.h"	// need these for avoidance


extern void G_CheckForCursorHints(gentity_t *ent);



/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback( gentity_t *player ) {
	gclient_t	*client;
	float	count;
	vec3_t	angles;

	client = player->client;
	if ( client->ps.pm_type == PM_DEAD ) {
		return;
	}

	// total points of damage shot at the player this frame
	count = client->damage_blood + client->damage_armor;
	if ( count == 0 ) {
		return;		// didn't take any damage
	}

	if ( count > 127 ) {
		count = 127;
	}

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if ( client->damage_fromWorld ) {
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;

		client->damage_fromWorld = qfalse;
	} else {
		vectoangles( client->damage_from, angles );
		client->ps.damagePitch = angles[PITCH]/360.0 * 256;
		client->ps.damageYaw = angles[YAW]/360.0 * 256;
	}

	// play an apropriate pain sound
	if ( (level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE) && !(player->r.svFlags & SVF_CASTAI) && !(player->s.powerups & PW_INVULNERABLE) ) {	//----(SA)	
		player->pain_debounce_time = level.time + 700;
		G_AddEvent( player, EV_PAIN, player->health );
	}

	client->ps.damageEvent++;	// Ridah, always increment this since we do multiple view damage anims

	client->ps.damageCount = count;

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_knockback = 0;
}


#define MIN_BURN_INTERVAL 399 // JPW NERVE set burn timeinterval so we can do more precise damage (was 199 old model)

/*
=============
P_WorldEffects

Check for lava / slime contents and drowning
=============
*/
void P_WorldEffects( gentity_t *ent ) {
	qboolean	envirosuit;
	int			waterlevel;
	// TTimo: unused
//	static int	lastflameburntime = 0; // JPW NERVE for slowing flamethrower burn intervals and doing more damage per interval

	if ( ent->client->noclip ) {
		ent->client->airOutTime = level.time + 12000;	// don't need air
		return;
	}

	waterlevel = ent->waterlevel;

	envirosuit = ent->client->ps.powerups[PW_BATTLESUIT] > level.time;

//	G_Printf("breathe: %d   invuln: %d   nofatigue: %d\n", ent->client->ps.powerups[PW_BREATHER], level.time - ent->client->ps.powerups[PW_INVULNERABLE], ent->client->ps.powerups[PW_NOFATIGUE]);

	//
	// check for drowning
	//
	if ( waterlevel == 3 ) {
		// envirosuit give air
		if ( envirosuit ) {
			ent->client->airOutTime = level.time + 10000;
		}

		//----(SA)	both these will end up being by virtue of having the 'breather' powerup
		if(ent->client->ps.aiChar == AICHAR_FROGMAN) {	// let frogmen breathe forever
			ent->client->airOutTime = level.time + 10000;
		}

		if(ent->client->ps.aiChar == AICHAR_SEALOPER) {	// ditto
			ent->client->airOutTime = level.time + 10000;
		}

		// if out of air, start drowning
		if ( ent->client->airOutTime < level.time) {

			if(ent->client->ps.powerups[PW_BREATHER]) {	// take air from the breather now that we need it
				ent->client->ps.powerups[PW_BREATHER] -= (level.time - ent->client->airOutTime);
				ent->client->airOutTime = level.time + (level.time - ent->client->airOutTime);
			}
			else {


				// drown!
				ent->client->airOutTime += 1000;
				if ( ent->health > 0 ) {
					// take more damage the longer underwater
					ent->damage += 2;
					if (ent->damage > 15)
						ent->damage = 15;

					// play a gurp sound instead of a normal pain sound
					if (ent->health <= ent->damage) {
						G_Sound(ent, G_SoundIndex("*drown.wav"));
					} else if (rand()&1) {
						G_Sound(ent, G_SoundIndex("sound/player/gurp1.wav"));
					} else {
						G_Sound(ent, G_SoundIndex("sound/player/gurp2.wav"));
					}

					// don't play a normal pain sound
					ent->pain_debounce_time = level.time + 200;

					G_Damage (ent, NULL, NULL, NULL, NULL, 
						ent->damage, DAMAGE_NO_ARMOR, MOD_WATER);
				}
			}
		}
	} else {
		ent->client->airOutTime = level.time + 12000;
		ent->damage = 2;
	}

	//
	// check for sizzle damage (move to pmove?)
	//
	if (waterlevel && (ent->watertype&CONTENTS_LAVA) ) {
		if (ent->health > 0	&& ent->pain_debounce_time <= level.time ) {

			if ( envirosuit ) {
				G_AddEvent( ent, EV_POWERUP_BATTLESUIT, 0 );
			} else {
				if (ent->watertype & CONTENTS_LAVA) {
					G_Damage (ent, NULL, NULL, NULL, NULL, 
						30*waterlevel, 0, MOD_LAVA);
				}
			}

		}
	}

// L0 
	// Flamer Bug fix

	//
	// check for burning from flamethrower
	//
	// JPW NERVE MP way
	if (ent->s.onFireEnd && ent->client) {
		//Martin 6/28/08 REDONE
		if ((level.time - ent->client->lastBurnTime >= 1000) && (ent->health > 0)) {
			ent->client->lastBurnTime = level.time;
			if ((ent->s.onFireEnd > level.time) && (ent->health > 0)) {
				gentity_t *attacker;
				attacker = g_entities + ent->flameBurnEnt;
				G_Damage(ent, attacker->parent, attacker->parent, NULL, NULL, 5, DAMAGE_NO_KNOCKBACK, MOD_FLAMETHROWER); // JPW NERVE was 7
			}
			// G_burnMeGood isnt called if they are dead so do the work here.
		}
		else if ((level.time - ent->client->lastBurnTime >= 10) && (ent->health <= 0)) {
			ent->client->lastBurnTime = level.time;
			if ((ent->s.onFireEnd > level.time) && (ent->health <= 0)) {
				gentity_t *attacker;
				attacker = g_entities + ent->flameBurnEnt;
				G_Damage(ent, attacker->parent, attacker->parent, NULL, NULL, 5, DAMAGE_NO_KNOCKBACK, MOD_FLAMETHROWER);
			}
		}
	}
	
	// Poison
	if (ent->poisoned && ent->client)
	{
		// Check if person is under spawn protection before any damange can be done
		if (ent->client->ps.powerups[PW_INVULNERABLE]) {
			ent->poisoned = qfalse; // if he's under protecion reset poison or it kicks in once protection expires =X
			return;
		}

		//if the guy isn't dead and it's been a second since the last time we did this
		if ((level.time >= (ent->lastPoisonTime + 1000)) && (ent->health > 0))
		{
			gentity_t *attacker = g_entities + ent->poisonEnt;	//the person who poisoned him
			vec3_t angles;

			G_Sound(ent, G_SoundIndex("sound/props/throw/chairthudgrunt.wav"));

			//make the poisoned guy's view go crazy
			angles[YAW] = (crandom() * 90);
			angles[PITCH] = (crandom() * 90);
			angles[ROLL] = 0;
			SetClientViewAngle(ent, angles);

			G_Damage(ent, attacker, attacker, NULL, NULL, g_poison.integer, 0, MOD_POISONDMED);	//hurt him!
			ent->lastPoisonTime = level.time;
		}
	} 
// L0 - End
}



/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound( gentity_t *ent ) {
	if (ent->aiCharacter)
		return;

	if (ent->waterlevel && (ent->watertype&CONTENTS_LAVA) )	//----(SA)	modified since slime is no longer deadly
		ent->s.loopSound = level.snd_fry;
	else
		ent->s.loopSound = 0;
}



//==============================================================

/*
==============
ClientImpacts
==============
*/
void ClientImpacts( gentity_t *ent, pmove_t *pm ) {
	int		i, j;
	trace_t	trace;
	gentity_t	*other;

	memset( &trace, 0, sizeof( trace ) );
	for (i=0 ; i<pm->numtouch ; i++) {
		for (j=0 ; j<i ; j++) {
			if (pm->touchents[j] == pm->touchents[i] ) {
				break;
			}
		}
		if (j != i) {
			continue;	// duplicated
		}
		other = &g_entities[ pm->touchents[i] ];

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, other, &trace );
		}

		if ( !other->touch ) {
			continue;
		}

		other->touch( other, ent, &trace );
	}

}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void	G_TouchTriggers( gentity_t *ent ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	trace_t		trace;
	vec3_t		mins, maxs;
	static vec3_t	range = { 40, 40, 52 };

	if ( !ent->client ) {
		return;
	}

	// dead clients don't activate triggers!
	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	VectorSubtract( ent->client->ps.origin, range, mins );
	VectorAdd( ent->client->ps.origin, range, maxs );

	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	// can't use ent->absmin, because that has a one unit pad
	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );

	for ( i=0 ; i<num ; i++ ) {
		hit = &g_entities[touch[i]];

		if ( !hit->touch && !ent->touch ) {
			continue;
		}
		if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
			continue;
		}

		// ignore most entities if a spectator
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			if ( hit->s.eType != ET_TELEPORT_TRIGGER ) {
				continue;
			}
		}

		// use seperate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		if ( hit->s.eType == ET_ITEM ) {
			if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) ) {
				continue;
			}
		} else {
			// MrE: always use capsule for player
			//if ( !trap_EntityContactCapsule( mins, maxs, hit ) ) {
			if ( !trap_EntityContact( mins, maxs, hit ) ) {
				continue;
			}
		}

		memset( &trace, 0, sizeof(trace) );

		if ( hit->touch ) {
			hit->touch (hit, ent, &trace);
		}

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, hit, &trace );
		}
	}
}

qboolean TrySpectatingWithCrosshair_Exact(gclient_t *client) {
	trace_t tr;
	vec3_t forward, start, end;
	vec3_t mins = { -5, -5, -5 };
	vec3_t maxs = { 5, 5, 5 };
	int prevEntNum = -1;

	AngleVectors(client->ps.viewangles, forward, NULL, NULL);
	VectorMA(client->ps.origin, 48, forward, start);
	VectorMA(client->ps.origin, 8192, forward, end);
	int earlyExitCounter = 0;
	while (1) {
		trap_Trace(&tr, start, mins, maxs, end, client->ps.clientNum, MASK_PLAYERSOLID);
		if (tr.entityNum < MAX_CLIENTS) {
			client->sess.spectatorState = SPECTATOR_FOLLOW;
			client->sess.spectatorClient = tr.entityNum;
			return qtrue;
		}
		else {
			if (tr.entityNum == prevEntNum) {
				VectorMA(start, 2, forward, start);
				earlyExitCounter++;
			}
			else {
				VectorCopy(tr.endpos, start);
			}
			if (earlyExitCounter > 150) {
				break;
			}
			prevEntNum = tr.entityNum;
			if (Distance(client->ps.origin, start) >= Distance(client->ps.origin, end)) {
				break;
			}
		}
	}

	return qfalse;
}

qboolean TrySpectatingWithCrosshair_Fuzzy(gclient_t *client) {
	vec3_t facing_dir;
	int i;
	float best_dot = -1.0f;
	gentity_t *best_other = NULL;

	AngleVectors(client->ps.viewangles, facing_dir, NULL, NULL);

	for (i = 0; i < level.numPlayingClients; ++i) {
		gentity_t *other = g_entities + level.sortedClients[i];
		vec3_t dir_to_client;
		VectorSubtract(other->client->ps.origin, client->ps.origin, dir_to_client);
		VectorNormalize(dir_to_client);

		float dot = DotProduct(facing_dir, dir_to_client);
		if (dot > best_dot) {
			best_dot = dot;
			best_other = other;
		}
	}

	if (best_other != NULL) {
		client->sess.spectatorState = SPECTATOR_FOLLOW;
		client->sess.spectatorClient = best_other - g_entities;
		return qtrue;
	}

	return qfalse;
}

/*
=================
SpectatorThink
=================
*/
void SpectatorThink( gentity_t *ent, usercmd_t *ucmd ) {
	pmove_t	pm;
	gclient_t	*client;

	client = ent->client;

	if ( client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		client->ps.pm_type = PM_SPECTATOR;
		client->ps.speed = 400;	// faster than normal
		if (client->ps.sprintExertTime)
			client->ps.speed *= 3;	// (SA) allow sprint in free-cam mode


		// set up for pmove
		memset (&pm, 0, sizeof(pm));
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;	// spectators can fly through bodies
		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;

		Pmove(&pm); // JPW NERVE

		// Rafael - Activate
		// Ridah, made it a latched event (occurs on keydown only)
		if (client->latched_buttons & BUTTON_ACTIVATE)
		{
			Cmd_Activate_f (ent);
		}

		// save results of pmove
		VectorCopy( client->ps.origin, ent->s.origin );

		G_TouchTriggers( ent );
		trap_UnlinkEntity( ent );
	}
	else {
		ent->client->ps.commandTime = ucmd->serverTime;
	}

	if (ent->flags & FL_NOFATIGUE)
		ent->client->ps.sprintTime = 20000;


	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;

//----(SA)	added
	client->oldwbuttons = client->wbuttons;
	client->wbuttons = ucmd->wbuttons;

	// attack button cycles through spectators
	if ( ( client->buttons & BUTTON_ATTACK ) && ! ( client->oldbuttons & BUTTON_ATTACK ) ) {

		if (client->sess.spectatorState == SPECTATOR_FREE && client->sess.sessionTeam == TEAM_SPECTATOR) {
			if (TrySpectatingWithCrosshair_Exact(client)) {
				return;
			}
			if (TrySpectatingWithCrosshair_Fuzzy(client)) {
				return;
			}
		}

		Cmd_FollowCycle_f(ent, 1);
	} else if (
		(client->sess.sessionTeam == TEAM_SPECTATOR) && // don't let dead team players do free fly
		(client->sess.spectatorState == SPECTATOR_FOLLOW) &&
		(client->buttons & BUTTON_ACTIVATE) &&
		!(client->oldbuttons & BUTTON_ACTIVATE) &&
		!(client->buttons & BUTTON_TALK)) {
		// code moved to StopFollowing
		StopFollowing(ent);
	}
}

/*
=================
ClientInactive
=================
*/
qboolean ClientInactive( gentity_t *ent, int msec ) 
{
	int inactiveTimeout, timeRemaining;
	gclient_t *client = ent->client;

	if (client->pers.localClient)
	{
		return qfalse;
	}

	if (g_gamestate.integer == GS_WARMUP_COUNTDOWN)
	{
		return qfalse;
	}

	if ((!g_inactivity.integer && (client->sess.sessionTeam == TEAM_RED || client->sess.sessionTeam == TEAM_BLUE)) ||
		(!g_spectatorInactivity.integer && client->sess.sessionTeam == TEAM_SPECTATOR))
	{
		client->pers.timeInactive = 0;
		client->pers.lastInactivityWarnTime = 0;

		return qfalse;
	}

	if (client->pers.cmd.forwardmove || client->pers.cmd.rightmove || client->pers.cmd.upmove ||
		(client->pers.cmd.wbuttons & WBUTTON_ATTACK2) || (client->pers.cmd.buttons & BUTTON_ATTACK) ||
		(client->pers.cmd.wbuttons & WBUTTON_LEANLEFT) || (client->pers.cmd.wbuttons & WBUTTON_LEANRIGHT))
	{
		if (client->pers.timeInactive && client->pers.lastInactivityWarnTime)
		{
			CP("cp \"\n\"2");
		}

		client->pers.timeInactive = 0;
		client->pers.lastInactivityWarnTime = 0;

		return qfalse;
	}

	if (client->ps.pm_flags & PMF_FOLLOW)
	{
		return qfalse;
	}

	client->pers.timeInactive += msec;

	inactiveTimeout = ((client->sess.sessionTeam == TEAM_RED || client->sess.sessionTeam == TEAM_BLUE) ? g_inactivity.integer : g_spectatorInactivity.integer) * 1000;
	timeRemaining = inactiveTimeout - client->pers.timeInactive;

	if (timeRemaining <= 0)
	{
		if (client->sess.sessionTeam == TEAM_SPECTATOR || !g_inactivityToSpecs.integer)
		{
			trap_DropClient(client - level.clients, "^3Player Kicked: Inactivity");
		}
		else
		{
			SetTeam(g_entities + client->ps.clientNum, "s", qtrue);
			AP(va("chat \"console: %s ^7sent to ^3spectators ^7due to inactivity.\n\"", client->pers.netname));
			CP("cp \"\n\"2");
		}

		client->pers.timeInactive = 0;
		client->pers.lastInactivityWarnTime = 0;

		return qtrue;
	}

	if (timeRemaining <= 10000 && level.time - client->pers.lastInactivityWarnTime >= 500)
	{
		CP(va("cp \"^3(^1%d^3) Inactivity warning! Move!\n\"2", timeRemaining / 1000));
		client->pers.lastInactivityWarnTime = level.time;
	}

	return qfalse;
}

/*
==================
ClientTimerActions

Actions that happen once a second
==================
*/
void ClientTimerActions( gentity_t *ent, int msec ) {
	gclient_t *client;

	client = ent->client;
	client->timeResidual += msec;

	while ( client->timeResidual >= 1000 ) {
		client->timeResidual -= 1000;

		// regenerate
		// JPW NERVE, split these completely
		if (g_gametype.integer < GT_WOLF) {
			if ( client->ps.powerups[PW_REGEN] ) {
				if ( ent->health < client->ps.stats[STAT_MAX_HEALTH]) {
					ent->health += 15;
					if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] * 1.1 ) {
						ent->health = client->ps.stats[STAT_MAX_HEALTH] * 1.1;
					}
					G_AddEvent( ent, EV_POWERUP_REGEN, 0 );
				} else if ( ent->health < client->ps.stats[STAT_MAX_HEALTH] * 2) {
					ent->health += 2;
					if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] * 2 ) {
						ent->health = client->ps.stats[STAT_MAX_HEALTH] * 2;
					}
					G_AddEvent( ent, EV_POWERUP_REGEN, 0 );
				}
			} else {
				// count down health when over max
				if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] ) {
					ent->health--;
				}
			}
		}
// JPW NERVE
		else { // GT_WOLF
			if ( client->ps.powerups[PW_REGEN] ) {
				if ( ent->health < client->ps.stats[STAT_MAX_HEALTH]) {
					ent->health += 3;
					if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] * 1.1){
						ent->health = client->ps.stats[STAT_MAX_HEALTH] * 1.1;
					}
				} else if ( ent->health < client->ps.stats[STAT_MAX_HEALTH] * 1.12) {
						ent->health += 2;
					if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] * 1.12 ) {
						ent->health = client->ps.stats[STAT_MAX_HEALTH] * 1.12;
					}
				}
			} else {
				// count down health when over max
				if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] ) {
					ent->health--;
				}
			}
		}
// jpw
		// count down armor when over max
		if ( client->ps.stats[STAT_ARMOR] > client->ps.stats[STAT_MAX_HEALTH] ) {
			client->ps.stats[STAT_ARMOR]--;
		}

		if (g_ammoRegen.integer) {
			if (client->ps.weaponstate == WEAPON_READY && WeaponIsPrimary(client->ps.weapon)) {
				int totalammo =
					client->ps.ammo[BG_FindAmmoForWeapon(client->ps.weapon)] +
					client->ps.ammoclip[BG_FindClipForWeapon(client->ps.weapon)];

				if (totalammo < ammoTable[client->ps.weapon].maxammo) {
					int increase = g_ammoRegen.integer;

					if (ammoTable[client->ps.weapon].maxammo - increase < totalammo)
						increase = ammoTable[client->ps.weapon].maxammo - totalammo;

					client->ps.ammo[client->ps.weapon] += increase;
				}
			}
		}
	}
}

/*
====================
ClientIntermissionThink
====================
*/
void ClientIntermissionThink( gclient_t *client ) {
	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;

	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = client->pers.cmd.buttons;

//----(SA)	added
	client->oldwbuttons = client->wbuttons;
	client->wbuttons = client->pers.cmd.wbuttons;
	
	if(	( client->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) & ( client->oldbuttons ^ client->buttons ) ) ||
		( client->wbuttons & WBUTTON_ATTACK2 & ( client->oldwbuttons ^ client->wbuttons ) ) ) {
		client->readyToExit ^= 1;
	}
}


/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void ClientEvents( gentity_t *ent, int oldEventSequence ) {
	int			i;
	int			event;
	gclient_t	*client;
	int			damage;
	vec3_t		dir;

	client = ent->client;

	if ( oldEventSequence < client->ps.eventSequence - MAX_EVENTS ) {
		oldEventSequence = client->ps.eventSequence - MAX_EVENTS;
	}
	for ( i = oldEventSequence ; i < client->ps.eventSequence ; i++ ) {
		event = client->ps.events[ i & (MAX_EVENTS-1) ];

		switch ( event ) {
		case EV_FALL_NDIE:
		//case EV_FALL_SHORT:
		case EV_FALL_DMG_10:
		case EV_FALL_DMG_15:
		case EV_FALL_DMG_25:
		//case EV_FALL_DMG_30:
		case EV_FALL_DMG_50:
		//case EV_FALL_DMG_75:
			
			if ( ent->s.eType != ET_PLAYER ) {
				break;		// not in the player model
			}
			if ( g_dmflags.integer & DF_NO_FALLING ) {
				break;
			}
			if ( event == EV_FALL_NDIE ) 
			{
				damage = 9999;
			}
			else if (event == EV_FALL_DMG_50)
			{
				damage = 50;
				ent->client->ps.pm_time = 1000;
				ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
				VectorClear (ent->client->ps.velocity);
			}
			else if (event == EV_FALL_DMG_25)
			{
				damage = 25;
				ent->client->ps.pm_time = 250;
				ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
				VectorClear (ent->client->ps.velocity);
			}
			else if (event == EV_FALL_DMG_15)
			{
				damage = 15;
				ent->client->ps.pm_time = 1000;
				ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
				VectorClear (ent->client->ps.velocity);
			}
			else if (event == EV_FALL_DMG_10)
			{
				damage = 10;
				ent->client->ps.pm_time = 1000;
				ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
				VectorClear (ent->client->ps.velocity);
			}
			else
				damage = 5; // never used
			VectorSet (dir, 0, 0, 1);
			ent->pain_debounce_time = level.time + 200;	// no normal pain sound
			G_Damage (ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING);
			break;
// JPW NERVE
		case EV_TESTID1:
		case EV_TESTID2:
		case EV_ENDTEST:
			break;
// jpw
		case EV_FIRE_WEAPON:			
		case EV_FIRE_WEAPONB:
		case EV_FIRE_WEAPON_LASTSHOT:			
			FireWeapon( ent );
			break;

		case EV_FIRE_WEAPON_MG42:
			mg42_fire(ent);
			break;

//----(SA)	modified
		case EV_USE_ITEM1:		// ( HI_MEDKIT )	medkit
		case EV_USE_ITEM2:		// ( HI_WINE )		wine
		case EV_USE_ITEM3:		// ( HI_SKULL )		skull of invulnerable
		case EV_USE_ITEM4:		// ( HI_WATER )		protection from drowning
		case EV_USE_ITEM5:		// ( HI_ELECTRIC )	protection from electric attacks
		case EV_USE_ITEM6:		// ( HI_FIRE )		protection from fire attacks
		case EV_USE_ITEM7:		// ( HI_STAMINA )	restores fatigue bar and sets "nofatigue" for a time period
		case EV_USE_ITEM8:		// ( HI_BOOK1 )		
		case EV_USE_ITEM9:		// ( HI_BOOK2 )		
		case EV_USE_ITEM10:		// ( HI_BOOK3 )		
			UseHoldableItem( ent, event - EV_USE_ITEM0);
			break;
//----(SA)	end

		default:
			break;
		}
	}

}

/*
==============
SendPendingPredictableEvents
==============
*/
void SendPendingPredictableEvents( playerState_t *ps ) {
	/*
	gentity_t *t;
	int event, seq;
	int extEvent, number;

	// if there are still events pending
	if ( ps->entityEventSequence < ps->eventSequence ) {
		// create a temporary entity for this event which is sent to everyone
		// except the client generated the event
		seq = ps->entityEventSequence & (MAX_EVENTS-1);
		event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		// set external event to zero before calling BG_PlayerStateToEntityState
		extEvent = ps->externalEvent;
		ps->externalEvent = 0;
		// create temporary entity for event
		t = G_TempEntity( ps->origin, event );
		number = t->s.number;
		BG_PlayerStateToEntityState( ps, &t->s, qtrue );
		t->s.number = number;
		t->s.eType = ET_EVENTS + event;
		t->s.eFlags |= EF_PLAYER_EVENT;
		t->s.otherEntityNum = ps->clientNum;
		// send to everyone except the client who generated the event
		t->r.svFlags |= SVF_NOTSINGLECLIENT;
		t->r.singleClient = ps->clientNum;
		// set back external event
		ps->externalEvent = extEvent;
	}
	*/
}

// DHM - Nerve
void WolfFindMedic( gentity_t *self ) {
	int i, medic=-1;
	gclient_t	*cl;
	vec3_t	start, end, temp;
	trace_t	tr;
	float	bestdist=1024, dist;

	self->client->ps.viewlocked_entNum = 0;
	self->client->ps.viewlocked = 0;
	self->client->ps.stats[STAT_DEAD_YAW] = 999;

	VectorCopy( self->s.pos.trBase, start );
	start[2] += self->client->ps.viewheight;

	for ( i=0; i<level.numPlayingClients; i++ ) {
		cl = &level.clients[ level.sortedClients[i] ];

		if ( cl->ps.clientNum == self->client->ps.clientNum )
			continue;
		if ( cl->sess.sessionTeam != self->client->sess.sessionTeam )
			continue;
		if ( cl->ps.stats[ STAT_HEALTH ] <= 0 )
			continue;
		if ( cl->ps.stats[ STAT_PLAYER_CLASS ] != PC_MEDIC )
			continue;

		VectorCopy( g_entities[level.sortedClients[i]].s.pos.trBase, end );
		end[2] += cl->ps.viewheight;

		trap_Trace (&tr, start, NULL, NULL, end, self->s.number, CONTENTS_SOLID);
		if ( tr.fraction < 0.95 )
			continue;

		VectorSubtract( end, start, end );
		dist = VectorNormalize( end );

		if ( dist < bestdist ) {
			medic = cl->ps.clientNum;
			vectoangles( end, temp );
			self->client->ps.stats[STAT_DEAD_YAW] = temp[YAW];
			bestdist = dist;
		}
	}

	if ( medic >= 0 ) {
		self->client->ps.viewlocked_entNum = medic;
		self->client->ps.viewlocked = 7;
	}
}

/*
==============
L0 - LTinfoMsg

Shows ammo stocks of clients..
==============
*/
char *weaponStr(int weapon)
{
	switch (weapon) {
	case WP_MP40:				return "MP40";
	case WP_THOMPSON:			return "Thompson";
	case WP_STEN:				return "Sten";
	case WP_MAUSER:				return "Mauser";
	case WP_SNIPERRIFLE:		return "Sniper Rifle";
	case WP_FLAMETHROWER:		return "Flamethrower";
	case WP_PANZERFAUST:		return "Panzerfaust";
	case WP_VENOM:				return "Venom";
	case WP_GRENADE_LAUNCHER:	return "Grenade";
	case WP_GRENADE_PINEAPPLE:	return "Grenade";
	case WP_KNIFE:				return "Knife";
	case WP_KNIFE2:				return "Knife";
	case WP_LUGER:				return "Luger";
	case WP_COLT:				return "Colt";
	case WP_MEDIC_SYRINGE:		return "Syringe";
	default:
		return "";
	}
}
// Draw str
void LTinfoMSG(gentity_t *ent) {
	unsigned int current = 0;
	unsigned int stock = 0;
	unsigned int nades = 0;
	weapon_t weapon;

	gentity_t *target;
	trace_t tr;
	vec3_t start, end, forward;

	if (ent->client->ps.stats[STAT_HEALTH] <= 0)
		return;

	if (g_gamestate.integer != GS_PLAYING)
		return;

	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);

	VectorCopy(ent->s.pos.trBase, start);	//set 'start' to the player's position (plus the viewheight)
	start[2] += ent->client->ps.viewheight;
	VectorMA(start, 512, forward, end);	//put 'end' 512 units forward of 'start'

	//see if we hit anything between 'start' and 'end'
	trap_Trace(&tr, start, NULL, NULL, end, ent->s.number, (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_CORPSE | CONTENTS_TRIGGER));

	if (tr.surfaceFlags & SURF_NOIMPACT)	return;
	if (tr.entityNum == ENTITYNUM_WORLD)	return;
	if (tr.entityNum >= MAX_CLIENTS)		return;

	target = &g_entities[tr.entityNum];
	if ((!target->inuse) || (!target->client))		return;
	if (target->client->ps.stats[STAT_HEALTH] <= 0)	return;
	if (!OnSameTeam(target, ent))					return;

	ent->client->infoTime = level.time;
	weapon = target->client->ps.weapon;
	current += target->client->ps.ammoclip[BG_FindClipForWeapon(weapon)];
	stock += target->client->ps.ammo[BG_FindAmmoForWeapon(weapon)];
	nades += target->client->ps.ammoclip[BG_FindClipForWeapon(WP_GRENADE_PINEAPPLE)];
	nades += target->client->ps.ammoclip[BG_FindClipForWeapon(WP_GRENADE_LAUNCHER)];

	if (Q_stricmp(weaponStr(weapon), ""))
	{
		if (weapon == WP_GRENADE_PINEAPPLE || weapon == WP_GRENADE_LAUNCHER)
			CP(va("cp \"%s: %i\n\"1", weaponStr(weapon), current));
		else if (weapon == WP_KNIFE || weapon == WP_KNIFE2)
			CP(va("cp \"%s - Grenades: %i\n\"1", weaponStr(weapon), current, nades));
		else
			CP(va("cp \"%s: %i/%i - Grenades: %i\n\"1", weaponStr(weapon), current, stock, nades));
	}
}

/*
==================
CG_SwingAngles
==================
*/
void G_SwingAngles(float destination, float swingTolerance, float clampTolerance, float speed, float *angle, qboolean *swinging, int msec) {
	float	swing;
	float	move;
	float	scale;

#define	SWING_RIGHT	1
#define SWING_LEFT	2

	if (!*swinging) {
		// see if a swing should be started
		swing = AngleSubtract(*angle, destination);
		if (swing > swingTolerance || swing < -swingTolerance) {
			*swinging = qtrue;
		}
	}

	if (!*swinging) {
		return;
	}

	// modify the speed depending on the delta
	// so it doesn't seem so linear
	swing = AngleSubtract(destination, *angle);
	scale = fabs(swing);
	scale *= 0.05;
	if (scale < 0.5)
		scale = 0.5;

	// swing towards the destination angle
	if (swing >= 0) {
		move = msec * scale * speed;
		if (move >= swing) {
			move = swing;
			*swinging = qfalse;
		}
		else {
			*swinging = SWING_LEFT;		// left
		}
		*angle = AngleMod(*angle + move);
	}
	else if (swing < 0) {
		move = msec * scale * -speed;
		if (move <= swing) {
			move = swing;
			*swinging = qfalse;
		}
		else {
			*swinging = SWING_RIGHT;	// right
		}
		*angle = AngleMod(*angle + move);
	}

	// clamp to no more than tolerance
	swing = AngleSubtract(destination, *angle);
	if (swing > clampTolerance) {
		*angle = AngleMod(destination - (clampTolerance - 1));
	}
	else if (swing < -clampTolerance) {
		*angle = AngleMod(destination + (clampTolerance - 1));
	}
}

/*
===============
G_PlayerAngles

Handles seperate torso motion

legs pivot based on direction of movement

head always looks exactly at cent->lerpAngles

if motion < 20 degrees, show in head only
if < 45 degrees, also show in torso
===============
*/
void G_PlayerAngles(gentity_t *ent, int msec) {
	vec3_t		legsAngles, torsoAngles, headAngles;
	float		dest;
	vec3_t		velocity;
	float		speed;
	float		clampTolerance;
	int			legsSet, torsoSet;

#define SWING_SPEED 0.1f

	legsSet = ent->s.legsAnim & ~ANIM_TOGGLEBIT;
	torsoSet = ent->s.torsoAnim & ~ANIM_TOGGLEBIT;

	VectorCopy(ent->s.apos.trBase, headAngles);
	headAngles[YAW] = AngleMod(headAngles[YAW]);
	VectorClear(legsAngles);
	VectorClear(torsoAngles);

	// --------- yaw -------------

	// Is the concept of "yawing" and "pitching" needed?
	// allow yaw to drift a bit, unless these conditions don't allow them
	if (!(BG_GetConditionValue(ent->s.number, ANIM_COND_MOVETYPE, qfalse) & ((1 << ANIM_MT_IDLE) | (1 << ANIM_MT_IDLECR)))) {
		ent->client->torsoYawing = qtrue;	// always center
		ent->client->torsoPitching = qtrue;	// always center
		ent->client->legsYawing = qtrue;	// always center
	}
	else if (BG_GetConditionValue(ent->s.number, ANIM_COND_FIRING, qtrue)) {
		ent->client->torsoYawing = qtrue;	// always center
		ent->client->torsoPitching = qtrue;	// always center
	}

	// adjust legs for movement dir
	if (ent->s.eFlags & EF_DEAD) {
		// don't let dead bodies twitch
		legsAngles[YAW] = headAngles[YAW];
		torsoAngles[YAW] = headAngles[YAW];
	}
	else {
		legsAngles[YAW] = headAngles[YAW] + ent->s.angles2[YAW];

		if (ent->s.eFlags & EF_NOSWINGANGLES) {
			legsAngles[YAW] = torsoAngles[YAW] = headAngles[YAW];	// always face firing direction
			clampTolerance = 60;
		}
		else if (!(ent->s.eFlags & EF_FIRING)) {
			torsoAngles[YAW] = headAngles[YAW] + 0.35 * ent->s.angles2[YAW];
			clampTolerance = 90;
		}
		else {	// must be firing
			torsoAngles[YAW] = headAngles[YAW];	// always face firing direction
												//if (fabs(cent->currentState.angles2[YAW]) > 30)
												//	legsAngles[YAW] = headAngles[YAW];
			clampTolerance = 60;
		}

		// torso
		G_SwingAngles(torsoAngles[YAW], 25, clampTolerance, SWING_SPEED, &ent->client->torsoYawAngle, &ent->client->torsoYawing, msec);

		// if the legs are yawing (facing heading direction), allow them to rotate a bit, so we don't keep calling
		// the legs_turn animation while an AI is firing, and therefore his angles will be randomizing according to their accuracy

		clampTolerance = 150;

		if (BG_GetConditionValue(ent->s.number, ANIM_COND_MOVETYPE, qfalse) & (1 << ANIM_MT_IDLE))
		{
			ent->client->legsYawing = qfalse; // set it if they really need to swing
			G_SwingAngles(legsAngles[YAW], 20, clampTolerance, 0.5*SWING_SPEED, &ent->client->legsYawAngle, &ent->client->legsYawing, msec);
		}
		else
			//if	( BG_GetConditionValue( ci->clientNum, ANIM_COND_MOVETYPE, qfalse ) & ((1<<ANIM_MT_STRAFERIGHT)|(1<<ANIM_MT_STRAFELEFT)) )
			if (strstr(BG_GetAnimString(ent->s.number, legsSet), "strafe"))
			{
				ent->client->legsYawing = qfalse; // set it if they really need to swing
				legsAngles[YAW] = headAngles[YAW];
				G_SwingAngles(legsAngles[YAW], 0, clampTolerance, SWING_SPEED, &ent->client->legsYawAngle, &ent->client->legsYawing, msec);
			}
			else
				if (ent->client->legsYawing)
				{
					G_SwingAngles(legsAngles[YAW], 0, clampTolerance, SWING_SPEED, &ent->client->legsYawAngle, &ent->client->legsYawing, msec);
				}
				else
				{
					G_SwingAngles(legsAngles[YAW], 40, clampTolerance, SWING_SPEED, &ent->client->legsYawAngle, &ent->client->legsYawing, msec);
				}

		torsoAngles[YAW] = ent->client->torsoYawAngle;
		legsAngles[YAW] = ent->client->legsYawAngle;
	}

	// --------- pitch -------------

	// only show a fraction of the pitch angle in the torso
	if (headAngles[PITCH] > 180) {
		dest = (-360 + headAngles[PITCH]) * 0.75;
	}
	else {
		dest = headAngles[PITCH] * 0.75;
	}
	G_SwingAngles(dest, 15, 30, 0.1, &ent->client->torsoPitchAngle, &ent->client->torsoPitching, msec);
	torsoAngles[PITCH] = ent->client->torsoPitchAngle;

	// --------- roll -------------

	// lean towards the direction of travel
	VectorCopy(ent->s.pos.trDelta, velocity);
	speed = VectorNormalize(velocity);
	if (speed) {
		vec3_t	axis[3];
		float	side;

		speed *= 0.05;

		AnglesToAxis(legsAngles, axis);
		side = speed * DotProduct(velocity, axis[1]);
		legsAngles[ROLL] -= side;

		side = speed * DotProduct(velocity, axis[0]);
		legsAngles[PITCH] += side;
	}

	// We don't care about the minute changs pain twitch inflict.
	/*
	// pain twitch
	CG_AddPainTwitch(cent, torsoAngles);
	*/

	// pull the angles back out of the hierarchial chain
	AnglesSubtract(headAngles, torsoAngles, headAngles);
	AnglesSubtract(torsoAngles, legsAngles, torsoAngles);
	AnglesToAxis(legsAngles, ent->client->animationInfo.legsAxis);
	AnglesToAxis(torsoAngles, ent->client->animationInfo.torsoAxis);
	AnglesToAxis(headAngles, ent->client->animationInfo.headAxis);
}

void G_PlayerAnimation(gentity_t *ent) {
	int legsSet, torsoSet;
	animModelInfo_t	*modelInfo = NULL;

	modelInfo = BG_ModelInfoForClient(ent->s.number);
	if (!modelInfo) {
		return;
	}

	legsSet = ent->s.legsAnim & ~ANIM_TOGGLEBIT;
	torsoSet = ent->s.torsoAnim & ~ANIM_TOGGLEBIT;

	ent->client->animationInfo.legsFrame = modelInfo->animations[legsSet].firstFrame + modelInfo->animations[legsSet].numFrames - 1;
	ent->client->animationInfo.torsoFrame = modelInfo->animations[torsoSet].firstFrame + modelInfo->animations[torsoSet].numFrames - 1;
}

gentity_t *G_DropWeapon(gentity_t *ent, trace_t *tr) {
	gclient_t *client = ent->client;

	if (!client->dropWeaponTime) {
		client->dropWeaponTime = 1; // just latch it for now

		if (client->ps.weapon == WP_KNIFE) {
			if (client->ps.stats[STAT_HEALTH] > 0) {
				if (g_dropObj.integer > 0)
					Cmd_dropObj(ent);
				else
					Cmd_throwKnives(ent);
			}
			return NULL;
		}

		// L0 - Patched it for g_unlockWeapons..
		if ((client->ps.stats[STAT_PLAYER_CLASS] == PC_SOLDIER && !client->ps.grenadeTimeLeft ||
			client->ps.stats[STAT_PLAYER_CLASS] == PC_LT && !client->ps.grenadeTimeLeft ||
			g_unlockWeapons.integer && client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC && !client->ps.grenadeTimeLeft ||
			g_unlockWeapons.integer && client->ps.stats[STAT_PLAYER_CLASS] == PC_ENGINEER && !client->ps.grenadeTimeLeft) &&
			client->ps.pm_type != PM_DEAD) {

			int weapon = PrimarySlotInUse(client->ps.weapons);

			// only smgs can be dropped in ffa, otherwise do nothing.
			if (g_ffa.integer)
			{
				if (weapon != WP_MP40 &&
					weapon != WP_THOMPSON &&
					weapon != WP_STEN)
				{
					weapon = WP_NONE;
				}
			}

			if (weapon) {
				vec3_t angles, velocity, offset, org, mins, maxs;
				gitem_t *item;

				item = BG_FindItemForWeapon(weapon);
				VectorCopy(client->ps.viewangles, angles);

				// clamp pitch
				if (angles[PITCH] < -30)
					angles[PITCH] = -30;
				else if (angles[PITCH] > 30)
					angles[PITCH] = 30;

				AngleVectors(angles, velocity, NULL, NULL);
				VectorScale(velocity, 64, offset);
				offset[2] += client->ps.viewheight / 2;
				VectorScale(velocity, 75, velocity);
				velocity[2] += 50 + random() * 35;

				VectorAdd(client->ps.origin, offset, org);

				VectorSet(mins, -ITEM_RADIUS, -ITEM_RADIUS, 0);
				VectorSet(maxs, ITEM_RADIUS, ITEM_RADIUS, 2 * ITEM_RADIUS);

				trap_Trace(tr, client->ps.origin, mins, maxs, org, ent->s.number, MASK_SOLID);
				VectorCopy(tr->endpos, org);

				gentity_t *dropped_weapon = LaunchItem(item, org, velocity, client->ps.clientNum);

				if (g_delayedPickup.integer) {
					dropped_weapon->touch = DelayedWeaponPickup;
					dropped_weapon->parent = ent;
					dropped_weapon->timestamp = level.time + g_delayedPickup.value * 1000;
				}

				COM_BitClear(client->ps.weapons, weapon);

				if (weapon == WP_MAUSER)
					COM_BitClear(client->ps.weapons, WP_SNIPERRIFLE);

				// Clear out empty weapon, change to next best weapon
				G_AddEvent(ent, EV_NOAMMO, 0);

				if (client->ps.weapon == weapon) {
					client->ps.weapon = 0;
				}
				dropped_weapon->count = client->ps.ammoclip[BG_FindClipForWeapon(weapon)];
				client->ps.ammoclip[BG_FindClipForWeapon(weapon)] = 0;
				if (g_dropClips.integer)
				{
					dropped_weapon->count += client->ps.ammo[BG_FindAmmoForWeapon(weapon)];
					client->ps.ammo[BG_FindAmmoForWeapon(weapon)] = 0;
				}

				return dropped_weapon;
			}
		}
	}

	return NULL;
}

void limbo( gentity_t *ent, qboolean makeCorpse ); // JPW NERVE
void reinforce (gentity_t *ent ); // JPW NERVE

void ClientDamage( gentity_t *clent, int entnum, int enemynum, int id );		// NERVE - SMF

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
void ClientThink_real( gentity_t *ent ) {
	gclient_t *client;
	pmove_t pm;
	int oldEventSequence;
	int	msec;
	usercmd_t *ucmd;
	int	monsterslick = 0;
	int	i;
	vec3_t muzzlebounce;
	gentity_t *dropped_weapon;
	int levelTime;
	trace_t tr;

	client = ent->client;
	dropped_weapon = NULL;

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if (client->pers.connected != CON_CONNECTED) {
		return;
	}

	if (client->cameraPortal) {
		G_SetOrigin( client->cameraPortal, client->ps.origin );
		trap_LinkEntity(client->cameraPortal);
		VectorCopy( client->cameraOrigin, client->cameraPortal->s.origin2);
	}

	// L0 - LT info
	if (g_LTinfoMsg.integer &&
		(client->ps.stats[STAT_PLAYER_CLASS] == PC_LT) &&
		(level.time >= client->infoTime + 1000))
	{
		LTinfoMSG(ent);
	}

	// mark the time, so the connection sprite can be removed
	ucmd = &ent->client->pers.cmd;

	ent->client->ps.identifyClient = ucmd->identClient;		// NERVE - SMF

	if (g_alternatePing.integer) {
		int sum = 0;
		client->pers.pingsamples[client->pers.samplehead] = level.previousTime - ucmd->serverTime;
		client->pers.samplehead++;

		if (client->pers.samplehead >= NUM_PING_SAMPLES) {
			client->pers.samplehead -= NUM_PING_SAMPLES;
		}

		for (i = 0; i < NUM_PING_SAMPLES; i++) {
			sum += client->pers.pingsamples[i];
		}

		client->pers.alternatePing = sum / NUM_PING_SAMPLES;

		if (client->pers.alternatePing < 0) {
			client->pers.alternatePing = 0;
		}
	}

// JPW NERVE -- update counter for capture & hold display
	if (g_gametype.integer == GT_WOLF_CPH) {
		client->ps.stats[STAT_CAPTUREHOLD_RED] = level.capturetimes[TEAM_RED];
		client->ps.stats[STAT_CAPTUREHOLD_BLUE] = level.capturetimes[TEAM_BLUE];
	}
// jpw
	
	levelTime = level.paused ? level.unpaused_time : level.time;

	// sanity check the command time to prevent speedup cheating
	if ( ucmd->serverTime > levelTime + 200 ) {
		ucmd->serverTime = levelTime + 200;
//		G_Printf("serverTime <<<<<\n" );
	}
	if ( ucmd->serverTime < levelTime - 1000 ) {
		ucmd->serverTime = levelTime - 1000;
//		G_Printf("serverTime >>>>>\n" );
	} 

	msec = ucmd->serverTime - client->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if ( msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		return;
		/*
		// Ridah, fixes savegame timing issue
		if (msec < -100) {
			client->ps.commandTime = ucmd->serverTime - 100;
			msec = 100;
		} else {
			return;
		}
		*/
		// done.
	}
	if ( msec > 200 ) {
		msec = 200;
	}

	client->ps.msec_samples[client->ps.msec_samples_index] = msec;
	if (client->ps.msec_samples_index == MAX_MSEC_SAMPLES - 1) {
		int s, sum = 0;
		for (s = 0; s < MAX_MSEC_SAMPLES; ++s) {
			sum += client->ps.msec_samples[s];
		}
		float average_msec = (float)sum / (float)s;
		average_msec = roundf(average_msec);
		client->ps.msec_averaged_samples[client->ps.msec_averaged_samples_index] = (int)average_msec;
		client->ps.msec_averaged_samples_index = (client->ps.msec_averaged_samples_index + 1) % MAX_MSEC_SAMPLES;
	}
	client->ps.msec_samples_index = (client->ps.msec_samples_index + 1) % MAX_MSEC_SAMPLES;

	if ( pmove_msec.integer < 8 ) {
		trap_Cvar_Set("pmove_msec", "8");
	}
	else if (pmove_msec.integer > 33) {
		trap_Cvar_Set("pmove_msec", "33");
	}

	if ( pmove_fixed.integer || client->pers.pmoveFixed ) {
		ucmd->serverTime = ((ucmd->serverTime + pmove_msec.integer-1) / pmove_msec.integer) * pmove_msec.integer;
		//if (ucmd->serverTime - client->ps.commandTime <= 0)
		//	return;
	}

	//
	// check for exiting intermission
	//
	if ( level.intermissiontime ) {
		ClientIntermissionThink( client );
		client->ps.commandTime = ucmd->serverTime;
		return;
	}

	// L0 - Admin bot, ping fluxation
	SB_maxPingFlux(client);

	// Check for client inactivity
	if (ClientInactive(ent, msec)) {
		return;
	}

	if (g_forceClass.integer >= 0) {
		int oldType = (client->pers.oldcmd.mpSetup & MP_CLASS_MASK) >> MP_CLASS_OFFSET;
		int currentType = (client->pers.cmd.mpSetup & MP_CLASS_MASK) >> MP_CLASS_OFFSET;

		if (oldType != currentType && currentType != g_forceClass.integer)
			CP(va("cp \"Player class is restricted to ^3%s^7!\n\"", PlayerTypeToString(g_forceClass.integer)));
	}

	// spectators don't do much
	// DHM - Nerve :: In limbo use SpectatorThink
	if ( client->sess.sessionTeam == TEAM_SPECTATOR || client->ps.pm_flags & PMF_LIMBO ) {
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			return;
		}
		SpectatorThink( ent, ucmd );
		return;
	}

	// JPW NERVE do some time-based muzzle flip -- this never gets touched in single player (see g_weapon.c)
	// #define RIFLE_SHAKE_TIME 150 // JPW NERVE this one goes with the commented out old damped "realistic" behavior below
	#define RIFLE_SHAKE_TIME 300 // per Id request, longer recoil time
	if (client->sniperRifleFiredTime) {
		if (level.time - client->sniperRifleFiredTime > RIFLE_SHAKE_TIME)
			client->sniperRifleFiredTime = 0;
		else {
			VectorCopy(client->ps.viewangles,muzzlebounce);

			// JPW per Id request, longer recoil time
			muzzlebounce[PITCH] -= 2 * cos(2.5*(level.time - client->sniperRifleFiredTime) / RIFLE_SHAKE_TIME);
			muzzlebounce[YAW] += 0.5*client->sniperRifleMuzzleYaw*cos(1.0 - (level.time - client->sniperRifleFiredTime) * 3 / RIFLE_SHAKE_TIME);
			muzzlebounce[PITCH] -= 0.25*random()*(1.0f - (level.time - client->sniperRifleFiredTime) / RIFLE_SHAKE_TIME);
			muzzlebounce[YAW] += 0.5*crandom()*(1.0f - (level.time - client->sniperRifleFiredTime) / RIFLE_SHAKE_TIME);
			SetClientViewAngle(ent, muzzlebounce);
		}
	}

	if (client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC)
		if (level.time > client->ps.powerups[PW_REGEN]+5000)
			client->ps.powerups[PW_REGEN] = level.time;
		// also update weapon recharge time

	if (g_gametype.integer != GT_SINGLE_PLAYER && !level.paused) {
		if (ucmd->wbuttons & WBUTTON_DROP) {
			dropped_weapon = G_DropWeapon(ent, &tr);
		} else {
			client->dropWeaponTime = 0;
		}
	}

	if (reloading || client->cameraPortal) {
		ucmd->buttons = 0;
		ucmd->forwardmove = 0;
		ucmd->rightmove = 0;
		ucmd->upmove = 0;
		ucmd->wbuttons = 0;
		ucmd->wolfkick = 0;
		if (client->cameraPortal) {
			client->ps.pm_type = PM_FREEZE;
		}
	} else if ( client->noclip ) {
		client->ps.pm_type = PM_NOCLIP;
	}
	else if (client->ps.stats[STAT_HEALTH] <= 0) {
		client->ps.pm_type = PM_DEAD;
	} else if (level.paused) {
		client->ps.pm_type = PM_FREEZE;
	} else {
		client->ps.pm_type = PM_NORMAL;
	}

	// set parachute anim condition flag
	BG_UpdateConditionValue( ent->s.number, ANIM_COND_PARACHUTE, (ent->flags & FL_PARACHUTE) != 0, qfalse );

	// all playing clients are assumed to be in combat mode
	if ( !client->ps.aiChar ) {
		client->ps.aiState = AISTATE_COMBAT;
	}

	client->ps.gravity = g_gravity.value;

	// set speed
	client->ps.speed = g_speed.value;

	if ( client->ps.powerups[PW_HASTE] ) {
		client->ps.speed *= 1.3;
	}

	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	client->currentAimSpreadScale = (float)client->ps.aimSpreadScale/255.0;

	memset (&pm, 0, sizeof(pm));

	pm.ps = &client->ps;
	pm.cmd = *ucmd;
	pm.oldcmd = client->pers.oldcmd;
	if ( pm.ps->pm_type == PM_DEAD ) {
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
		// DHM-Nerve added:: EF_DEAD is checked for in Pmove functions, but wasn't being set
		//              until after Pmove
		pm.ps->eFlags |= EF_DEAD;
		// dhm-Nerve end
	}
	else {
		pm.tracemask = MASK_PLAYERSOLID;
	}
	// MrE: always use capsule for AI and player
	//pm.trace = trap_TraceCapsule;//trap_Trace;
	//DHM - Nerve :: We've gone back to using normal bbox traces
	pm.trace = trap_Trace;
	pm.pointcontents = trap_PointContents;
	pm.debugLevel = g_debugMove.integer;
	pm.noFootsteps = ( g_dmflags.integer & DF_NO_FOOTSTEPS ) > 0;

	pm.pmove_fixed = pmove_fixed.integer | client->pers.pmoveFixed;
	pm.pmove_msec = pmove_msec.integer;

	pm.noWeapClips = ( g_dmflags.integer & DF_NO_WEAPRELOAD ) > 0;
	if (ent->aiCharacter && AICast_NoReload(ent->s.number))
		pm.noWeapClips = qtrue;	// ensure AI characters don't use clips if they're not supposed to.

	// L0 - Fixedpsyhics
	if (g_fixedphysics.integer) {
		pm.fixedphysicsfps = 125;
	}

	// Ridah
//	if (ent->r.svFlags & SVF_NOFOOTSTEPS)
//		pm.noFootsteps = qtrue;

	VectorCopy( client->ps.origin, client->oldOrigin );

	// NERVE - SMF
	pm.gametype = g_gametype.integer;
	pm.ltChargeTime = g_LTChargeTime.integer;
	pm.soldierChargeTime = g_soldierChargeTime.integer;
	pm.engineerChargeTime = g_engineerChargeTime.integer;
	pm.medicChargeTime = g_medicChargeTime.integer;
	// -NERVE - SMF

	monsterslick = Pmove (&pm);

	if (ent->client->revive_animation_playing)
	{
		if (ent->client->ps.pm_time == 0 || !(ent->client->ps.pm_flags & PMF_TIME_LOCKPLAYER))
		{
			int lock_time_remaining = 2100 - (level.time - ent->client->movement_lock_begin_time);

			if (lock_time_remaining <= 0)
			{
				ent->client->revive_animation_playing = qfalse;
				ent->client->ps.legsTimer = 0;
				ent->client->ps.torsoTimer = 0;
			}
			else
			{
				ent->client->ps.pm_flags |= PMF_TIME_LOCKPLAYER;
				ent->client->ps.pm_time = lock_time_remaining;
			}
		}
	}

	if (monsterslick && !(ent->flags & FL_NO_MONSTERSLICK))
	{
		//vec3_t	dir;
		//vec3_t	kvel;
		//vec3_t	forward;
  // TTimo gcc: might be used unitialized in this function
		float	angle = 0.0f;
		qboolean bogus = qfalse;

		// NE
		if ((monsterslick & SURF_MONSLICK_N) && (monsterslick & SURF_MONSLICK_E))
		{
			angle = 45;
		}
		// NW
		else if ((monsterslick & SURF_MONSLICK_N) && (monsterslick & SURF_MONSLICK_W))
		{
			angle = 135;
		}
		// N
		else if (monsterslick & SURF_MONSLICK_N)
		{
			angle = 90;
		}

		// SE
		else if ((monsterslick & SURF_MONSLICK_S) && (monsterslick & SURF_MONSLICK_E))
		{
			angle = 315;
		}
		// SW
		else if ((monsterslick & SURF_MONSLICK_S) && (monsterslick & SURF_MONSLICK_W))
		{
			angle = 225;
		}
		// S
		else if (monsterslick & SURF_MONSLICK_S)
		{
			angle = 270;
		}

		// E
		else if (monsterslick & SURF_MONSLICK_E)
		{
			angle = 0;
		}
		// W
		else if (monsterslick & SURF_MONSLICK_W)
		{
			angle = 180;
		}
		else
		{
			bogus = qtrue;
		}
	}

	// server cursor hints
	if ( ent->lastHintCheckTime < level.time ) {
		G_CheckForCursorHints(ent);

		ent->lastHintCheckTime = level.time + FRAMETIME;
	}

	// DHM - Nerve :: Set animMovetype to 1 if ducking
	if ( ent->client->ps.pm_flags & PMF_DUCKED )
		ent->s.animMovetype = 1;
	else
		ent->s.animMovetype = 0;

	// save results of pmove
	if ( ent->client->ps.eventSequence != oldEventSequence ) {
		ent->eventTime = level.time;
		ent->r.eventTime = level.time;
	}

	// Ridah, fixes jittery zombie movement
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
	}
	else {
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
	}

	if ( !( ent->client->ps.eFlags & EF_FIRING ) ) {
		client->fireHeld = qfalse;		// for grapple
	}

//
//	// use the precise origin for linking
//	VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
//
//	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );

	VectorCopy (pm.mins, ent->r.mins);
	VectorCopy (pm.maxs, ent->r.maxs);

	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;

	G_PlayerAngles(ent, msec);
	G_PlayerAnimation(ent);

	// execute client events
	ClientEvents( ent, oldEventSequence );

	// link entity now, after any personal teleporters have been used
	trap_LinkEntity (ent);
	if ( !ent->client->noclip ) {
		G_TouchTriggers(ent);

		qboolean validLean1 = !client->ps.leanf;
		qboolean validLean2 = (client->ps.leanf && (client->pers.cmd.rightmove || client->pers.oldcmd.rightmove));

		if ( dropped_weapon && ( (!PrimarySlotInUse(client->ps.weapons) && (validLean1 || validLean2) && g_lockPrimary.integer)
			|| (ent->r.svFlags & SVF_BOT) ) ) {
			dropped_weapon->touch(dropped_weapon, ent, &tr);
			G_FreeEntity(dropped_weapon);
		}
	}

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );

	// L0 - antilag
	G_StoreTrail(ent);

	// touch other objects
	ClientImpacts( ent, &pm );

	// save results of triggers and client events
	if (ent->client->ps.eventSequence != oldEventSequence) {
		ent->eventTime = level.time;
	}

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons = client->buttons & ~client->oldbuttons;
//	client->latched_buttons |= client->buttons & ~client->oldbuttons;	// FIXME:? (SA) MP method (causes problems for us.  activate 'sticks')

	//----(SA)	added
	client->oldwbuttons = client->wbuttons;
	client->wbuttons = ucmd->wbuttons;
	client->latched_wbuttons = client->wbuttons & ~client->oldwbuttons;
//	client->latched_wbuttons |= client->wbuttons & ~client->oldwbuttons;	// FIXME:? (SA) MP method

	// Rafael - Activate
	// Ridah, made it a latched event (occurs on keydown only)
	if (client->latched_buttons & BUTTON_ACTIVATE) {
		Cmd_Activate_f(ent);
	}

	if (client->buttons & BUTTON_ACTIVATE) {
		Cmd_Drag(ent);
	}

	if (client->latched_buttons & BUTTON_GESTURE) {
		Cmd_Push(ent);
	}

	if (client->buttons & BUTTON_GESTURE) {
		Cmd_Drag(ent);
	}

	if (ent->flags & FL_NOFATIGUE)
		ent->client->ps.sprintTime = 20000;


	// check for respawning
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) {

		// DHM - Nerve
		if ( g_gametype.integer >= GT_WOLF )
			WolfFindMedic( ent );
		// dhm - end

		// wait for the attack button to be pressed
		if ( level.time > client->respawnTime ) {
			// forcerespawn is to prevent users from waiting out powerups
			if ((g_gametype.integer != GT_SINGLE_PLAYER) &&
				(g_forcerespawn.integer > 0) && 
				(( level.time - client->respawnTime ) > g_forcerespawn.integer * 1000)  && 
				(!(ent->client->ps.pm_flags & PMF_LIMBO)) ) { // JPW NERVE
				// JPW NERVE
				if (g_gametype.integer >= GT_WOLF)
					limbo(ent, qtrue);
				else
					respawn(ent);
				// jpw
				return;
			}

			// DHM - Nerve :: Single player game respawns immediately as before,
			//				  but in multiplayer, require button press before respawn
			if ( g_gametype.integer == GT_SINGLE_PLAYER )
				respawn( ent );
			// NERVE - SMF - we want to only respawn on jump button now
			else if (( ucmd->upmove > 0 ) &&
				(!(ent->client->ps.pm_flags & PMF_LIMBO)) ) { // JPW NERVE
				// JPW NERVE
				if (g_gametype.integer >= GT_WOLF){
					limbo(ent, qtrue);

					// L0 - Tap reports
					if (g_tapReports.integer > 1) {
						// Axis 
						if (ent->client->sess.sessionTeam == TEAM_RED)
							G_TeamCommand(TEAM_RED, va("print \"%s ^7tapped out\n\"", ent->client->pers.netname));
						// Allied
						else if (ent->client->sess.sessionTeam == TEAM_BLUE)
							G_TeamCommand(TEAM_BLUE, va("print \"%s ^7tapped out\n\"", ent->client->pers.netname));
					}
					else if (g_tapReports.integer == 1) {
						AP(va("print \"%s ^7tapped out\n\"", ent->client->pers.netname));
					} // End
				}
				else
					respawn(ent);
				// jpw
			}
			// dhm - Nerve :: end
			// NERVE - SMF - we want to immediately go to limbo mode if gibbed
			else if ( client->ps.stats[STAT_HEALTH] <= GIB_HEALTH && !( ent->client->ps.pm_flags & PMF_LIMBO ) ) {
				if (g_gametype.integer >= GT_WOLF)
					limbo(ent, qfalse);
				else
					respawn(ent);
			}
			// -NERVE - SMF
		}
		return;
	}

	// L0 - Max TKs - Drop player if over limit and time has expired..
	if (sb_system.integer && sb_maxTKs.integer > 0)
	{
		int count = ent->client->pers.teamKills + 1 - ent->client->pers.sb_TKforgiven;

		if (count > sb_maxTKs.integer)
		{
			if (ent->client->pers.sb_TKkillTime < level.time)
			{
				int tempban = sb_maxTKsTempbanMins.integer;
				// Tempban
				if (tempban)
				{				
					trap_SendConsoleCommand(EXEC_APPEND, va("tempban %i %i\n", ent->s.clientNum, tempban));
				}

				trap_DropClient(ent - g_entities, va("%s \n^3For Team Killing.", tempban ? "Tempbanned" : "Kicked"));
				AP(va("chat \"console: %s ^7got %s for ^3Team Killing^7.\n\"", ent->client->pers.netname, tempban ? "tempbanned" : "kicked"));
			}
		}
	}

	// perform once-a-second actions
	ClientTimerActions( ent, msec );
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink( int clientNum ) {
	gentity_t *ent;

	ent = g_entities + clientNum;
	ent->client->pers.oldcmd = ent->client->pers.cmd;
	trap_GetUsercmd( clientNum, &ent->client->pers.cmd );

	// mark the time we got info, so we can display the
	// phone jack if they don't get any for a while
	ent->client->lastCmdTime = level.paused ? level.unpaused_time : level.time;

	if (!(ent->r.svFlags & SVF_BOT) && !g_syncronousClients.integer) {
		ClientThink_real(ent);
	}
}


void G_RunClient( gentity_t *ent ) {
	if (!(ent->r.svFlags & SVF_BOT) && !g_syncronousClients.integer) {
		return;
	}
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink_real( ent );
}

#if DEBUG
#include "be_ai_goal.h"

void G_DrawBotRoutes(gentity_t *ent) {
	gentity_t *bboxEnt;
	bot_route_t route;
	vec3_t b1, b2;
	int route_index, path_index;

	vec3_t mins = { -8, -8, -8 };
	vec3_t maxs = { 8, 8, 8 };

	static int last_draw_time = 0;

	if (ent->r.svFlags & SVF_BOT) {
		return;
	}

	if (level.time - last_draw_time < 500) {
		return;
	}

	route_index = 0;
	while (trap_GetRouteAtIndex(route_index, &route) != -1) {
		for (path_index = 0; path_index < route.path_length; ++path_index) {
			VectorCopy(route.path[path_index], b1);
			VectorCopy(route.path[path_index], b2);
			VectorAdd(b1, mins, b1);
			VectorAdd(b2, maxs, b2);
			bboxEnt = G_TempEntity(b1, EV_RAILTRAIL);
			VectorCopy(b2, bboxEnt->s.origin2);
			bboxEnt->s.dmgFlags = 1;
			bboxEnt->s.otherEntityNum2 = ent->s.number;
		}
		route_index++;
	}

	last_draw_time = level.time;
}
#endif

/*
==================
SpectatorClientEndFrame

==================
*/
void SpectatorClientEndFrame( gentity_t *ent ) {
	gclient_t	*cl;
	int do_respawn=0; // JPW NERVE
	int	savedScore;		// DHM - Nerve
	int savedRespawns;	// DHM - Nerve
	int savedClass;		// NERVE - SMF
	int flags;
	int testtime;
	// for fps restoration after ps memset
	int	msec_samples[MAX_MSEC_SAMPLES];
	int	msec_samples_index;
	int	msec_averaged_samples[MAX_MSEC_SAMPLES];
	int	msec_averaged_samples_index;
	int	command_time;

	// if we are doing a chase cam or a remote view, grab the latest info
	if (( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) || (ent->client->ps.pm_flags & PMF_LIMBO)) { // JPW NERVE for limbo
		int		clientNum;

		if (ent->client->sess.sessionTeam == TEAM_RED) {
			// L0 - If warmup damage is on respawn almost instantly 
			// - due weapon nuke we can't do it instantly.
			if (g_warmupDamage.integer && g_gamestate.integer != GS_PLAYING)
				testtime = level.time % 2000;
			else
				testtime = level.time%g_redlimbotime.integer;

			if (testtime < ent->client->pers.lastReinforceTime)
				do_respawn=1;

			ent->client->pers.lastReinforceTime = testtime;
		}
		else if (ent->client->sess.sessionTeam == TEAM_BLUE) {
			// L0 - If warmup damage is on respawn almost instantly 
			// - due to weapon nuke we can't do it instantly.
			if (g_warmupDamage.integer && g_gamestate.integer != GS_PLAYING)
				testtime = level.time % 2000;
			else
				testtime = level.time%g_bluelimbotime.integer;

			if (testtime < ent->client->pers.lastReinforceTime)
				do_respawn=1;

			ent->client->pers.lastReinforceTime = testtime;
		}

		if ((g_maxlives.integer > 0 || g_alliedmaxlives.integer > 0 || g_axismaxlives.integer > 0) && ent->client->ps.persistant[PERS_RESPAWNS_LEFT] == 0) {
			do_respawn = 0;
		}

		if ( do_respawn ) {
			reinforce(ent);
			return;
		}

		if (g_ffa.integer &&
			ent->client->ps.pm_flags & PMF_LIMBO)
		{
			StopFollowing(ent);
			return;
		}

		clientNum = ent->client->sess.spectatorClient;

		// team follow1 and team follow2 go to whatever clients are playing
		if ( clientNum == -1 ) {
			clientNum = level.follow1;
		} else if ( clientNum == -2 ) {
			clientNum = level.follow2;
		}
		if ( clientNum >= 0 ) {
			cl = &level.clients[ clientNum ];
			if (cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam != TEAM_SPECTATOR) {
				// L0 - Ping & Score bug fix
				// This solves the /serverstatus and score table (who's specing/demoing you) bug..
				int ping = ent->client->ps.ping;
				int score = ent->client->ps.persistant[PERS_SCORE];
				// save fps/msec samples
				memcpy(msec_samples, ent->client->ps.msec_samples, sizeof(msec_samples));
				msec_samples_index = ent->client->ps.msec_samples_index;
				memcpy(msec_averaged_samples, ent->client->ps.msec_averaged_samples, sizeof(msec_averaged_samples));
				msec_averaged_samples_index = ent->client->ps.msec_averaged_samples_index;
				command_time = ent->client->ps.commandTime;


				// DHM - Nerve :: carry flags over
				flags = (cl->ps.eFlags & ~(EF_VOTED)) | (ent->client->ps.eFlags & (EF_VOTED));
				// JPW NERVE -- limbo latch
				if (ent->client->sess.sessionTeam != TEAM_SPECTATOR && ent->client->ps.pm_flags & PMF_LIMBO) {
					// abuse do_respawn var
					savedScore = ent->client->ps.persistant[PERS_SCORE];
					do_respawn = ent->client->ps.pm_time;
					savedRespawns = ent->client->ps.persistant[PERS_RESPAWNS_LEFT];
					savedClass = ent->client->ps.stats[STAT_PLAYER_CLASS];

					ent->client->ps = cl->ps;
					ent->client->ps.pm_flags |= PMF_FOLLOW;
					ent->client->ps.pm_flags |= PMF_LIMBO;

					ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = savedRespawns;
					ent->client->ps.pm_time = do_respawn;							// put pm_time back
					ent->client->ps.persistant[PERS_SCORE] = savedScore;			// put score back
					ent->client->ps.stats[STAT_PLAYER_CLASS] = savedClass;			// NERVE - SMF - put player class back
				}
				else {
					ent->client->ps = cl->ps;
					ent->client->ps.pm_flags |= PMF_FOLLOW;
				}
				// jpw
				// DHM - Nerve :: carry flags over
				ent->client->ps.eFlags = flags;
				// L0 - Ping & Score bug fix
				ent->client->ps.ping = ping;
				ent->client->ps.persistant[PERS_SCORE] = score;
				// restore msec/fps samples
				memcpy(ent->client->ps.msec_samples, msec_samples, sizeof(ent->client->ps.msec_samples));
				ent->client->ps.msec_samples_index = msec_samples_index;
				memcpy(ent->client->ps.msec_averaged_samples, msec_averaged_samples, sizeof(ent->client->ps.msec_averaged_samples));
				ent->client->ps.msec_averaged_samples_index = msec_averaged_samples_index;
				ent->client->ps.commandTime = command_time;
				return;
			} else if (cl->pers.connected == CON_DISCONNECTED && (cl->sess.sessionTeam == TEAM_RED || cl->sess.sessionTeam == TEAM_BLUE)) {
				// continue;
			} else {
				// Try following the next valid client
				Cmd_FollowCycle_f(ent, 1);
				// Couldn't find a new client.. Stop spectating
				if (ent->client->sess.spectatorClient == clientNum) {
					StopFollowing(ent);
				}
			}
		}
	}

#if DEBUG
	if (trap_Cvar_VariableIntegerValue("bot_enable")) {
		G_DrawBotRoutes(ent);
	}
#endif

	if ( ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
		ent->client->ps.pm_flags |= PMF_SCOREBOARD;
	} else {
		ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
	}
}

qboolean ClientsAreIntersecting(gentity_t *self, gentity_t *hit) {
	vec3_t	hitmin, hitmax;
	vec3_t	selfmin, selfmax;

	if (!hit->inuse) {
		return qfalse;
	}
	if (hit == self) {
		return qfalse;
	}
	if (!hit->client) {
		return qfalse;
	}
	if (!hit->s.solid) {
		return qfalse;
	}
	if (hit->health <= 0) {
		return qfalse;
	}

	VectorAdd(hit->r.currentOrigin, hit->r.mins, hitmin);
	VectorAdd(hit->r.currentOrigin, hit->r.maxs, hitmax);
	VectorAdd(self->r.currentOrigin, self->r.mins, selfmin);
	VectorAdd(self->r.currentOrigin, self->r.maxs, selfmax);

	if (hitmin[0] > selfmax[0]) {
		return qfalse;
	}
	if (hitmax[0] < selfmin[0]) {
		return qfalse;
	}
	if (hitmin[1] > selfmax[1]) {
		return qfalse;
	}
	if (hitmax[1] < selfmin[1]) {
		return qfalse;
	}
	if (hitmin[2] > selfmax[2]) {
		return qfalse;
	}
	if (hitmax[2] < selfmin[2]) {
		return qfalse;
	}

	return qtrue;
}

extern vec3_t playerMins, playerMaxs;
#define WR_PUSHAMOUNT 13

void GetIntersectingClients(gentity_t *self, gentity_t **intersecting_clients, int *intersecting_count_out) {
	int i, j, intersecting_count;
	gentity_t *hit;
	vec3_t mins, maxs;
	int trapped_count, hit_count;
	int entities_from_query[MAX_GENTITIES] = { -1 };

	trapped_count = 0;
	intersecting_count = 0;

	for (i = 0; i < level.numPlayingClients; ++i) {
		hit = &g_entities[level.sortedClients[i]];
		if (ClientsAreIntersecting(self, hit)) {
			intersecting_clients[intersecting_count++] = hit;
		}
	}

	if (!intersecting_count && !(self->r.contents & CONTENTS_CORPSE)) {
		return;
	}

	VectorAdd(self->r.currentOrigin, playerMins, mins);
	VectorAdd(self->r.currentOrigin, playerMaxs, maxs);

	hit_count = trap_EntitiesInBox(mins, maxs, entities_from_query, MAX_GENTITIES);

	for (i = 0; i < hit_count; ++i) {
		hit = &g_entities[entities_from_query[i]];
		if (hit->health > 0 && hit->client && hit->s.number != self->s.number) {
			for (j = 0; j < intersecting_count; ++j) {
				if (intersecting_clients[j] == hit) {
					break;
				}
			}
			if (j == intersecting_count) {
				intersecting_clients[intersecting_count + trapped_count++] = hit;
			}
		}
	}

	*intersecting_count_out = intersecting_count + trapped_count;
}

void PushPlayerStuckInOtherPlayers(gentity_t *ent, gentity_t **intersecting_clients, int intersecting_count) {
	const float push_mult = 2.2f;

	int i;
	vec3_t push_velocity, average_dir;

	VectorSet(average_dir, 0, 0, 0);

	for (i = 0; i < intersecting_count; ++i) {
		gentity_t *intersecting_client = intersecting_clients[i];
		vec3_t dir;

		VectorSubtract(ent->r.currentOrigin, intersecting_client->r.currentOrigin, dir);
		VectorAdd(average_dir, dir, average_dir);

		intersecting_client->r.contents = CONTENTS_CORPSE;
	}

	VectorScale(average_dir, 1.0f / (float)intersecting_count, average_dir);
	VectorNormalizeFast(average_dir);
	average_dir[2] = 0;

	VectorScale(average_dir, WR_PUSHAMOUNT * push_mult, push_velocity);
	VectorAdd(ent->s.pos.trDelta, push_velocity, ent->s.pos.trDelta);
	VectorAdd(ent->client->ps.velocity, push_velocity, ent->client->ps.velocity);
}

qboolean IntersectedRevivablePlayer(gentity_t **intersecting_clients, int intersecting_count) {
	int i;

	for (i = 0; i < intersecting_count; ++i) {
		gentity_t *intersecting_client = intersecting_clients[i];

		if (intersecting_client->client->in_revived_state) {
			return qtrue;
		}
	}

	return qfalse;
}

void CheckForStuckPlayersAndUnstuckThem(gentity_t *ent) {
	gentity_t *intersecting_clients[MAX_CLIENTS] = { 0 };
	int intersecting_count = 0;

	GetIntersectingClients(ent, intersecting_clients, &intersecting_count);
	if (intersecting_count > 0) {
		if (ent->client->in_revived_state || !IntersectedRevivablePlayer(intersecting_clients, intersecting_count)) {
			PushPlayerStuckInOtherPlayers(ent, intersecting_clients, intersecting_count);
		}
		ent->r.contents = CONTENTS_CORPSE;
	} else if (ent->r.contents & CONTENTS_CORPSE) {
		ent->client->in_revived_state = qfalse;
		ent->r.contents = CONTENTS_BODY;
	}
}

void WolfReviveStuckInWorld(gentity_t *ent) {
#define MAX_STUCK_CHECKS 6

	trap_UnlinkEntity(ent);
	VectorCopy(ent->client->unstuckPos, ent->client->ps.origin);
	VectorCopy(ent->client->ps.origin, ent->r.currentOrigin);
	trap_LinkEntity(ent);
	ent->client->revivedInsideOfTheWorld = qfalse;
}

void FixForWonkyReviveAnim(gentity_t *ent) {
	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR &&
		ent->client->ps.pm_type == PM_DEAD &&
		!(ent->client->ps.pm_flags & PMF_LIMBO)) {
		if (ent->client->needs_delayed_teleport_bit_start_time) {
			int sv_fps = trap_Cvar_VariableIntegerValue("sv_fps");
			int frameMsec = sv_fps == 0 ? 50 : 1000 / sv_fps;

			if (level.time - ent->client->needs_delayed_teleport_bit_start_time >= frameMsec * ent->client->teleport_bit_delayed_frame_count) {
				// toggle it!
				ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
				ent->client->needs_delayed_teleport_bit_start_time = 0;
			}
		}
	}
}

void G_DrawHitBoxes(gentity_t *ent) {
	gentity_t *bboxEnt;
	vec3_t b1, b2;

	// Draw body hitbox
	VectorCopy(ent->r.currentOrigin, b1);
	VectorCopy(ent->r.currentOrigin, b2);
	VectorAdd(b1, ent->r.mins, b1);
	VectorAdd(b2, ent->r.maxs, b2);
	bboxEnt = G_TempEntity(b1, EV_RAILTRAIL);
	VectorCopy(b2, bboxEnt->s.origin2);
	bboxEnt->s.dmgFlags = 1;
	bboxEnt->s.otherEntityNum2 = ent->s.number;

	// Draw head hitbox
	UpdateHeadEntity(ent);
	VectorCopy(ent->head->r.currentOrigin, b1);
	VectorCopy(ent->head->r.currentOrigin, b2);
	VectorAdd(b1, ent->head->r.mins, b1);
	VectorAdd(b2, ent->head->r.maxs, b2);
	bboxEnt = G_TempEntity(b1, EV_RAILTRAIL);
	VectorCopy(b2, bboxEnt->s.origin2);
	bboxEnt->s.dmgFlags = 1;
	bboxEnt->s.otherEntityNum2 = ent->s.number;
	RemoveHeadEntity(ent);
}

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEdFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gentity_t *ent ) {
	int			i;

	if (g_alternatePing.integer) {
		if (ent->client->ps.ping >= 999) {
			ent->client->pers.alternatePing = ent->client->ps.ping;
		}
	}

	if (( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) || (ent->client->ps.pm_flags & PMF_LIMBO)) { // JPW NERVE
		SpectatorClientEndFrame( ent );
		return;
	}

	if (!ent->aiCharacter) {
		// turn off any expired powerups
		for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {

			if(	i == PW_FIRE ||				// these aren't dependant on level.time
				i == PW_ELECTRIC ||
				i == PW_BREATHER ||
				i == PW_NOFATIGUE) {

				continue;
			}

			if ( ent->client->ps.powerups[ i ] < level.time ) {
				ent->client->ps.powerups[ i ] = 0;
			}
		}
	}

	// save network bandwidth
#if 0
	if ( !g_synchronousClients->integer && ent->client->ps.pm_type == PM_NORMAL ) {
		// FIXME: this must change eventually for non-sync demo recording
		VectorClear( ent->client->ps.viewangles );
	}
#endif

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if ( level.intermissiontime ) {
		return;
	}

	// burn from lava, etc
	P_WorldEffects (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	// add the EF_CONNECTION flag if we haven't gotten commands recently
	if ( level.time - ent->client->lastCmdTime > 1000 ) {
		ent->s.eFlags |= EF_CONNECTION;
	} else {
		ent->s.eFlags &= ~EF_CONNECTION;
	}

	ent->client->ps.stats[STAT_HEALTH] = ent->health;	// FIXME: get rid of ent->health...

	G_SetClientSound (ent);

	// set the latest infor

	// Ridah, fixes jittery zombie movement
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, ((ent->r.svFlags & SVF_CASTAI) == 0) );
	}
	else {
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, ((ent->r.svFlags & SVF_CASTAI) == 0) );
	}

	//SendPendingPredictableEvents( &ent->client->ps );

	// DHM - Nerve :: If it's been a couple frames since being revived, and props_frame_state
	//					wasn't reset, go ahead and reset it
	if ( ent->props_frame_state >= 0 && ( (level.time - ent->s.effect3Time) > 100 ) )
		ent->props_frame_state = -1;

	if (ent->health > 0) {
		CheckForStuckPlayersAndUnstuckThem(ent);
	}

	if (g_unstuckRevive.integer && ent->client->revivedInsideOfTheWorld) {
		WolfReviveStuckInWorld(ent);
	}

	// DHM - Nerve :: Reset 'count2' for flamethrower
	if ( !(ent->client->buttons & BUTTON_ATTACK) )
		ent->count2 = 0;
	// dhm
	
#if 0
	FixForWonkyReviveAnim(ent);
#endif

	if (ent->client->pers.drawHitBoxes && g_drawHitboxes.integer && ent->health > 0) {
		G_DrawHitBoxes(ent);
	}
}
