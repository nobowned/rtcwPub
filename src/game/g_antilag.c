/*/
===========================================================================
L0 / rtcwPub :: g_antilag.c

	Antilag functionality.
	Forks: Unfreeze -> s4ndmod -> wolfX -> main (1.4) and all in between.

Created: 24. Mar / 2014
===========================================================================
*/
#include "g_local.h"

// OSP 
#define IS_ACTIVE( x ) ( \
	x->r.linked == qtrue &&	\
	x->client->ps.stats[STAT_HEALTH] > 0 && \
	x->client->sess.sessionTeam != TEAM_SPECTATOR && \
	(x->client->ps.pm_flags & PMF_LIMBO) == 0	\
	)

/*
============
G_ResetTrail

Clear out the given client's origin trails (should be called from ClientBegin and when
the teleport bit is toggled)
============
*/
void G_ResetTrail(gentity_t *ent) {
	int	i, time, time_increment, sv_fps;

	sv_fps = trap_Cvar_VariableIntegerValue("sv_fps");
	time_increment = sv_fps == 0 ? 50 : 1000 / sv_fps;

	// fill up the origin trails with data (assume the current position
	// for the last 1/2 second or so)
	ent->client->trailHead = NUM_CLIENT_TRAILS - 1;
	for (i = ent->client->trailHead, time = level.time; i >= 0; i--, time -= time_increment) {
		VectorCopy(ent->r.mins, ent->client->trail[i].mins);
		VectorCopy(ent->r.maxs, ent->client->trail[i].maxs);
		VectorCopy(ent->r.currentOrigin, ent->client->trail[i].currentOrigin);
		ent->client->trail[i].leveltime = time;
		ent->client->trail[i].time = time;
		ent->client->trail[i].animInfo = ent->client->animationInfo;
	}
}


/*
============
G_StoreTrail

Keep track of where the client's been (usually called every ClientThink)
============
*/
void G_StoreTrail(gentity_t *ent) {
	int head, newtime;

	if (!IS_ACTIVE(ent)) {
		return;
	}

	head = ent->client->trailHead;

	// this code results in only one client trail being stored per server frame (every 50ms).
	// and eventually that trail will have it's time snapped to a leveltime. this results
	// in ~500 ms worth of trails into the past.
	if (ent->client->trail[head].leveltime < level.time) {
		// we're on a new frame; snap the last head up to the end of frame time
		ent->client->trail[head].time = level.previousTime;

		// increment the head
		ent->client->trailHead++;
		if (ent->client->trailHead >= NUM_CLIENT_TRAILS) {
			ent->client->trailHead = 0;
		}
		head = ent->client->trailHead;
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

		// these checks are just clamping the time in-case it gets out of control.
		if (newtime > level.time) {
			newtime = level.time;
		}
		else if (newtime <= level.previousTime) {
			newtime = level.previousTime + 1;
		}
	}

	// store all the collision-detection info and the time
	VectorCopy(ent->r.mins, ent->client->trail[head].mins);
	VectorCopy(ent->r.maxs, ent->client->trail[head].maxs);
	VectorCopy(ent->r.currentOrigin, ent->client->trail[head].currentOrigin);
	ent->client->trail[head].leveltime = level.time;
	ent->client->trail[head].time = newtime;
	ent->client->trail[head].animInfo = ent->client->animationInfo;
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
	qboolean found_sandwich;

	// find two trail records in the origin trail whose times sandwich "time".
	// assumes no two adjacent trail records have the same timestamp.
	//
	// this code will check every trail record, even if the head starts at index 0.. it'll wrap around to 9 and decrease from there.
	found_sandwich = qfalse;
	j = k = ent->client->trailHead;
	do {
		if (ent->client->trail[j].time <= time) {
			found_sandwich = j != k;
			break;
		}

		k = j;
		j--;
		if (j < 0) {
			j = NUM_CLIENT_TRAILS - 1;
		}
	} while (j != ent->client->trailHead);

	// we've found two trail times that "sandwich" the passed in "time"
	if (found_sandwich) {
		// save the current origin and bounding box; used to untimeshift the client once collision detection is complete
		VectorCopy(ent->r.mins, ent->client->saved.mins);
		VectorCopy(ent->r.maxs, ent->client->saved.maxs);
		VectorCopy(ent->r.currentOrigin, ent->client->saved.currentOrigin);
		ent->client->saved.leveltime = level.time;
		ent->client->saved.animInfo = ent->client->animationInfo;

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
		// use the trail's animation info that's nearest "time" (for head hitbox)
		// the current server animation code used for head hitboxes doesn't support interpolating
		// between two different animation frames (i.e. crouch -> standing animation), so can't interpolate here either.
		ent->client->animationInfo = ent->client->trail[nearest_trail_index].animInfo;

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

	if (time > level.time) {
		return;
	}

	// for every client
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
	// if it was saved
	if (ent->client->saved.leveltime == level.time) {
		// move it back
		VectorCopy(ent->client->saved.mins, ent->r.mins);
		VectorCopy(ent->client->saved.maxs, ent->r.maxs);
		VectorCopy(ent->client->saved.currentOrigin, ent->r.currentOrigin);
		ent->client->saved.leveltime = 0;
		ent->client->animationInfo = ent->client->saved.animInfo;

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

	ent = &g_entities[0];
	for (i = 0; i < MAX_CLIENTS; i++, ent++) {
		if (ent->client && ent->inuse && ent->client->sess.sessionTeam < TEAM_SPECTATOR && ent != skip) {
			if (!(ent->client->ps.pm_flags & PMF_LIMBO)){
				G_UnTimeShiftClient(ent);
			}
		}
	}
}
