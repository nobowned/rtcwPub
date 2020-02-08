/*/
===========================================================================
L0 / rtcwPub :: g_antilag.c

	Antilag functionality.
	Forks: Unfreeze -> s4ndmod -> wolfX -> main (1.4) and all in between.

Created: 24. Mar / 2014
===========================================================================
*/
#include "g_local.h"

/*
============
IsActiveClient

Is the entity a client that's currently playing in the world (active)?
============
*/
qboolean IsActiveClient(gentity_t* ent) {
	return	
		ent->r.linked == qtrue &&
		ent->client &&
		ent->client->ps.stats[STAT_HEALTH] > 0 &&
		ent->client->sess.sessionTeam != TEAM_SPECTATOR &&
		(ent->client->ps.pm_flags & PMF_LIMBO) == 0;
}

/*
============
G_ResetTrail

Clear out the given client's origin trails (should be called from ClientBegin and when
the teleport bit is toggled)
============
*/
void G_ResetTrail(gentity_t *ent) {
	int	i, time;

	// we want to store half a second worth of trails (500ms)
	const int trail_time_range_ms = 500;
	const int time_increment = round((double)trail_time_range_ms / (double)NUM_CLIENT_TRAILS);

	// fill up the origin trails with data (assume the current position
	// for the last 1/2 second or so)
	ent->client->trailHead = NUM_CLIENT_TRAILS - 1;
	for (i = ent->client->trailHead, time = level.time; i >= 0; i--, time -= time_increment) {
		VectorCopy(ent->r.mins, ent->client->trail[i].mins);
		VectorCopy(ent->r.maxs, ent->client->trail[i].maxs);
		VectorCopy(ent->r.currentOrigin, ent->client->trail[i].currentOrigin);
		ent->client->trail[i].time = time;
		ent->client->trail[i].animationInfo = ent->client->animationInfo;
	}
}


/*
============
G_StoreTrail

Keep track of where the client's been (usually called every ClientThink)
============
*/
void G_StoreTrail(gentity_t *ent) {
	int newtime;

	// only store trails for actively playing clients.
	// also, don't store trails if the level time hasn't been set yet (it'll happen next SV_Frame).
	if (!IsActiveClient(ent) || !level.time || !level.previousTime) {
		return;
	}

	// limit how often higher fps clients store trails. otherwise we lose too much time-sensitive data that's required for higher ping players.
	int trail_time_since_last_store = ent->client->pers.cmd.serverTime - ent->client->last_store_trail_time;
	ent->client->accum_store_trail_time += trail_time_since_last_store;
	ent->client->last_store_trail_time = ent->client->pers.cmd.serverTime;
	if (ent->client->accum_store_trail_time < 6) {
		return;
	}
	ent->client->accum_store_trail_time = 0;

	// increment the head
	ent->client->trailHead++;
	if (ent->client->trailHead >= NUM_CLIENT_TRAILS) {
		ent->client->trailHead = 0;
	}

	if (ent->r.svFlags & SVF_BOT) {
		// bots move only once per frame
		newtime = level.time;
	}
	else {
		// level.frameStartTime is set to trap_Milliseconds() within G_RunFrame.
		//
		// we want to store where the server thinks the client is after receiving and processing
		// one of their usercmd packets (move command). trap_Milliseconds() is used for a more granular timestamp,
		// since level.time is only ever incremented every 50-ish milliseconds (depends on sv_fps).
		// 
		// if level.time were used then clients with high fps, high maxpackets, and high rate would have
		// many trail records with duplicate timestamps.
		int realTimeSinceFrameStartTime = trap_Milliseconds() - level.frameStartTime;
		newtime = level.previousTime + realTimeSinceFrameStartTime;
	}

	// store all the collision-detection info and the time
	clientTrail_t* trail = &ent->client->trail[ent->client->trailHead];
	VectorCopy(ent->r.mins, trail->mins);
	VectorCopy(ent->r.maxs, trail->maxs);
	VectorCopy(ent->r.currentOrigin, trail->currentOrigin);
	trail->time = newtime;
	trail->animationInfo = ent->client->animationInfo;
}

/*
=================
Interpolates

Interpolates along two vectors (start -> end).
=================
*/
void Interpolate(float frac, vec3_t start, vec3_t end, vec3_t out) {
	float comp = 1.0f - frac;

	out[0] = start[0] * frac + end[0] * comp;
	out[1] = start[1] * frac + end[1] * comp;
	out[2] = start[2] * frac + end[2] * comp;
}


/*
=================
G_TimeShiftClient

Move a client back to where he was at the specified "time"
=================
*/
void G_TimeShiftClient(gentity_t *ent, int time) {
	int	j, k;
	qboolean found_trail_times_that_sandwich_time;

	// this prevents looping through every trail if we know none of them are <= time.
	// the trails are "sorted" by time, so if the oldest one isn't <= time, then none of them can be.
	if (ent->client->trail[(ent->client->trailHead + 1) & (NUM_CLIENT_TRAILS - 1)].time > time) {
		return;
	}

	// find two trail records in the origin trail whose times sandwich "time".
	// assumes no two adjacent trail records have the same timestamp.
	//
	// this code will check every trail record, even if the head starts at index 0.. it'll wrap around to NUM_CLIENT_TRAILS - 1 and decrease from there.
	found_trail_times_that_sandwich_time = qfalse;
	j = k = ent->client->trailHead;
	do {
		if (ent->client->trail[j].time <= time) {
			found_trail_times_that_sandwich_time = j != k;
			break;
		}

		k = j;
		j--;
		if (j < 0) {
			j = NUM_CLIENT_TRAILS - 1;
		}
	} while (j != ent->client->trailHead);

	memset(&ent->client->saved_trail, 0, sizeof(clientTrail_t));

	// we've found two trail times that "sandwich" the passed in "time"
	if (found_trail_times_that_sandwich_time) {
		// save the current origin and bounding box; used to untimeshift the client once collision detection is complete
		VectorCopy(ent->r.mins, ent->client->saved_trail.mins);
		VectorCopy(ent->r.maxs, ent->client->saved_trail.maxs);
		VectorCopy(ent->r.currentOrigin, ent->client->saved_trail.currentOrigin);
		ent->client->saved_trail.animationInfo = ent->client->animationInfo;

		// calculate a fraction that will be used to shift the client's position back to the trail record that's nearest to "time"
		float frac = (float)(time - ent->client->trail[j].time) / (float)(ent->client->trail[k].time - ent->client->trail[j].time);

		// find the "best" origin between the sandwiching trails via interpolation
		Interpolate(frac, ent->client->trail[j].currentOrigin, ent->client->trail[k].currentOrigin, ent->r.currentOrigin);
		// find the "best" mins & maxs (crouching/standing).
		// it doesn't make sense to interpolate mins and maxs. the server either thinks the client
		// is crouching or not, and updates the mins & maxs immediately. there's no inbetween.
		int nearest_trail_index = frac < 0.5 ? j : k;
		VectorCopy(ent->client->trail[nearest_trail_index].mins, ent->r.mins);
		VectorCopy(ent->client->trail[nearest_trail_index].maxs, ent->r.maxs);
		// use the trail's animation info that's nearest "time" (for head hitbox).
		// the current server animation code used for head hitboxes doesn't support interpolating
		// between two different animation frames (i.e. crouch -> standing animation), so can't interpolate here either.
		ent->client->animationInfo = ent->client->trail[nearest_trail_index].animationInfo;

		// this will recalculate absmin and absmax
		trap_LinkEntity(ent);
	}
}


/*
=====================
G_TimeShiftAllClients

Move ALL clients back to where they were at the specified "time",
except for "skip"
=====================
*/
void G_TimeShiftAllClients(int time, gentity_t *skip) {
	int			i;
	gentity_t	*ent;

	// don't shift clients if "skip" (the client that's trying to timeshift everyone) is more than 500ms behind the current server time (very laggy).
	if (level.time - time > 500) {
		return;
	}

	// shift every client thats:
	// not a spectator, not the "timeshifter" (skip), and not in limbo.
	ent = &g_entities[0];
	for (i = 0; i < MAX_CLIENTS; i++, ent++) {
		if (ent->client && ent->inuse && ent->client->sess.sessionTeam < TEAM_SPECTATOR && ent != skip) {
			if (!(ent->client->ps.pm_flags & PMF_LIMBO)){
				G_TimeShiftClient(ent, time);
			}
		}
	}
}


/*
===================
G_UnTimeShiftClient

Move a client back to where he was before the time shift
===================
*/
void G_UnTimeShiftClient(gentity_t *ent) {
	// if ent was time shifted
	if (ent->client->saved_trail.mins[0]) {
		// revert the time shift
		VectorCopy(ent->client->saved_trail.mins, ent->r.mins);
		VectorCopy(ent->client->saved_trail.maxs, ent->r.maxs);
		VectorCopy(ent->client->saved_trail.currentOrigin, ent->r.currentOrigin);
		ent->client->animationInfo = ent->client->saved_trail.animationInfo;

		// this will recalculate absmin and absmax
		trap_LinkEntity(ent);
	}
}

/*
=======================
G_UnTimeShiftAllClients

Move ALL the clients back to where they were before the time shift,
except for "skip"
=======================
*/
void G_UnTimeShiftAllClients(gentity_t *skip) {
	int			i;
	gentity_t	*ent;

	// unshift every client thats:
	// not a spectator, not the "timeshifter" (skip), and not in limbo.
	ent = &g_entities[0];
	for (i = 0; i < MAX_CLIENTS; i++, ent++) {
		if (ent->client && ent->inuse && ent->client->sess.sessionTeam < TEAM_SPECTATOR && ent != skip) {
			if (!(ent->client->ps.pm_flags & PMF_LIMBO)){
				G_UnTimeShiftClient(ent);
			}
		}
	}
}
