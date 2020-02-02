//
// g_automg42.c
//

#include "g_local.h"

vec3_t	default_base_mins = { -24, -24, 0 };
vec3_t	default_base_maxs = { 24, 24, 48 };

vec3_t	default_gun_mins = { -24, -24, -8 };
vec3_t	default_gun_maxs = { 24, 24, 0 };

qboolean CanAutomg42HitClient(gentity_t *gun, gentity_t *target,
	float z_offest, int best_distance, int *out_distance,
	vec3_t out_dir, vec3_t mg42_angles, vec3_t gun_muzzle,
	vec3_t gun_forward, vec3_t gun_right, vec3_t gun_up)
{
	trace_t tr;
	vec3_t target_position, start_position;

#define INVALID_TRACE tr.allsolid || tr.fraction == 1.0f

	VectorCopy(target->r.currentOrigin, target_position);
	target_position[2] += z_offest;
	VectorSubtract(target_position, gun->s.pos.trBase, out_dir);
	 *out_distance = VectorNormalize(out_dir);

	if (*out_distance >= best_distance)
	{
		return qfalse;
	}

	vectoangles(out_dir, mg42_angles);
	// get our forward direction vector
	AngleVectors(mg42_angles, gun_forward, gun_right, gun_up);
	VectorCopy(gun->s.pos.trBase, gun_muzzle);
	// move muzzle forward
	VectorMA(gun_muzzle, 12, gun_forward, gun_muzzle);
	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector(gun_muzzle);

	VectorCopy(gun_muzzle, start_position);
	trap_Trace(&tr, start_position, 0, 0, target_position, gun->s.number, MASK_SHOT);

	if (INVALID_TRACE || tr.entityNum != target->s.number)
	{
		return qfalse;
	}

	return qtrue;
}

void G_Automg42SetConstantLight(gentity_t *gun, int intensity)
{
	gentity_t *base = &g_entities[gun->mg42BaseEnt];
	int brightness = gun->s.frame == 2 ? 75 : 0;

	if (gun->aiTeam == TEAM_BLUE)
	{
		G_SetConstantLight(gun, brightness, brightness, 255, intensity);
		G_SetConstantLight(base, brightness, brightness, 255, intensity);
	}
	else if (gun->aiTeam == TEAM_RED)
	{
		G_SetConstantLight(gun, 255, brightness, brightness, intensity);
		G_SetConstantLight(base, 255, brightness, brightness, intensity);
	}
	else
	{
		G_SetConstantLight(gun, 255, 255, 255, intensity);
		G_SetConstantLight(base, 255, 255, 255, intensity);
	}
}

void G_Automg42UpdateHealth(gentity_t *gun, int health)
{
	gentity_t *base = &g_entities[gun->mg42BaseEnt];

	gun->health = base->health = health;

	int fullHealth = gun->s.frame == 2 ? gun->count2 * MG42_FIXED_HEALTH_MULT : gun->count2;
	float frac = (float)((float)health / (float)fullHealth);
	int intensity = frac * gun->radius;
	if (intensity > gun->radius)
	{
		intensity = gun->radius;
	}
	else if (intensity < 0)
	{
		intensity = 0;
	}

	G_Automg42SetConstantLight(gun, intensity);
}

qboolean G_Automg42GetGunAndBase(gentity_t *ent, gentity_t **gun, gentity_t **base)
{
	if (ent->chain)
	{
		*base = ent;
		*gun = ent->chain;

		return qtrue;
	}
	else if (ent->mg42BaseEnt > 0)
	{
		*base = g_entities + ent->mg42BaseEnt;
		*gun = ent;

		return qtrue;
	}

	return qfalse;
}

void G_Automg42Disassociate(gentity_t *ent)
{
	gentity_t *base = NULL;

	while ((base = G_Find(base, FOFS(classname), "mg42_base")))
	{
		if (base->parent == ent)
		{
			base->parent = base->chain->parent = NULL;
		}
	}
}

qboolean G_IsEntityAutomg42(gentity_t *ent)
{
	return g_automg42.integer && ent && (!Q_stricmp(ent->classname, "misc_mg42") || !Q_stricmp(ent->classname, "mg42_base"));
}

void G_Automg42Repair(gentity_t *ent, gentity_t *traceEnt)
{
	gentity_t *gun, *base;

	if (!G_Automg42GetGunAndBase(traceEnt, &gun, &base))
	{
		return;
	}

	if (gun->s.frame != 2)
	{
		return;
	}

	if (base->think == automg42_base_soaring)
	{
		return;
	}

	if (gun->s.eFlags & EF_SMOKING)
	{
		// "Ammo" for this weapon is time based
		if (ent->client->ps.classWeaponTime + g_engineerChargeTime.integer < level.time) {
			ent->client->ps.classWeaponTime = level.time - g_engineerChargeTime.integer;
		}
		ent->client->ps.classWeaponTime += 150;

		if (ent->client->ps.classWeaponTime > level.time) {
			ent->client->ps.classWeaponTime = level.time;
			return;		// Out of "ammo"
		}
	}

	if (gun->health >= (gun->count2 * MG42_FIXED_HEALTH_MULT)) {
		gun->s.frame = 0;

		gun->think = automg42_think;
		gun->nextthink = level.time + 50;
		gun->health = base->health = gun->count2;
		gun->takedamage = base->takedamage = qtrue;

		G_Automg42SetConstantLight(gun, gun->radius);

		if (gun->s.eFlags & EF_SMOKING)
		{
			CP("cp \"Automg42 repaired!\n\"");
		}
		else
		{
			CP("cp \"Automg42 complete!\n\"");
		}

		gun->s.eFlags &= ~EF_SMOKING;

		G_AddEvent(ent, EV_MG42_FIXED, 0);
	}
	else
	{
		if (gun->aiTeam && gun->aiTeam != ent->client->sess.sessionTeam)
		{
			// Going from blue to red
			if (gun->aiTeam == TEAM_BLUE)
			{
				level.allies_automg42_count--;
				level.axis_automg42_count++;
			}
			// Going from red to blue
			else
			{
				level.axis_automg42_count--;
				level.allies_automg42_count++;
			}
		}
		gun->aiTeam = base->aiTeam = ent->client->sess.sessionTeam;
		gun->parent = base->parent = ent;

		G_Automg42UpdateHealth(gun, gun->health + 3);

		if (gun->s.eFlags & EF_SMOKING)
		{
			CP("cp \"Repairing automg42\n\"");
		}
		else
		{
			CP("cp \"Setting up automg42\n\"");
		}
	}
}

#define AUTOMG42_RATE_OF_FIRE 150

void automg42_think(gentity_t *gun)
{
	int i, distance, best_distance;
	gentity_t *target, *best_target;
	vec3_t best_dir, best_angles, best_muzzle, best_forward, best_right, best_up;
	vec3_t dir, mg42_angles, gun_muzzle, gun_forward, gun_right, gun_up;

	gun->nextthink = level.time + AUTOMG42_RATE_OF_FIRE;
	best_target = NULL;
	best_distance = 999999999;

	if (gun->s.frame == 2)
	{
		return;
	}

	for (i = 0; i < level.numPlayingClients; ++i)
	{
		target = g_entities + level.sortedClients[i];

		if (target->client->sess.sessionTeam == gun->aiTeam)
		{
			continue;
		}

		if (target->health <= 0 || target->client->ps.pm_flags & PMF_LIMBO)
		{
			continue;
		}

		if (VectorDistance(gun->r.currentOrigin, target->r.currentOrigin) > 4096)
		{
			continue;
		}

		if (!CanAutomg42HitClient(gun, target, 0, best_distance, &distance, dir, mg42_angles, gun_muzzle, gun_forward, gun_right, gun_up))
		{
			if (!CanAutomg42HitClient(gun, target, target->client->ps.viewheight, best_distance, &distance, dir, mg42_angles, gun_muzzle, gun_forward, gun_right, gun_up))
			{
				if (!CanAutomg42HitClient(gun, target, -23, best_distance, &distance, dir, mg42_angles, gun_muzzle, gun_forward, gun_right, gun_up))
				{
					continue;
				}
			}
		}

		best_target = target;
		best_distance = distance;
		VectorCopy(dir, best_dir);
		VectorCopy(mg42_angles, best_angles);
		VectorCopy(gun_muzzle, best_muzzle);
		VectorCopy(gun_forward, best_forward);
		VectorCopy(gun_right, best_right);
		VectorCopy(gun_up, best_up);
	}

	if (best_target)
	{
		G_SetAngle(gun, best_angles);
		// fire the weapon
		Fire_Lead(gun, gun, MG42_SPREAD, gun->damage, best_muzzle, best_forward, best_right, best_up, qtrue);
	}

	// TODO(lance): Re-engage gravity physics on gun if whatever was beneath it gone.
	// This would probably be easier/doable if the gun entity wasn't inside of the base.
	// It makes more sense for the base entity to be below the gun.
	/*
	trace_t trace;
	vec3_t ground_test;
	gentity_t *base = &g_entities[gun->mg42BaseEnt];

	VectorSet(ground_test, base->r.currentOrigin[0], base->r.currentOrigin[1], base->r.currentOrigin[2] - 1.0f);
	trap_Trace(&trace, base->r.currentOrigin, base->r.mins, base->r.maxs, ground_test, base->s.number, MASK_SOLID);

	if (!trace.allsolid && !trace.startsolid && trace.fraction == 1.0f && trace.entityNum == ENTITYNUM_NONE)
	{
		base->s.groundEntityNum = -1;
	}
	*/
}

extern gentity_t *propExplosion(gentity_t *ent);

void automg42_free(gentity_t *base)
{
	gentity_t *explosion = propExplosion(base);
	explosion->damage = 0;
	explosion->splashDamage = 0;

	APRS(base, "sound/weapons/mg42/mg42_death.wav");

	G_FreeEntity(base->chain);
	G_FreeEntity(base);
}

void automg42_remove(gentity_t *base, int time)
{
	gentity_t *gun = base->chain;

	if (gun->aiTeam == TEAM_RED)
	{
		level.axis_automg42_count--;
	}
	else
	{
		level.allies_automg42_count--;
	}

	gun->think = NULL;
	gun->nextthink = 0;

	base->think = automg42_free;
	base->nextthink = time + FRAMETIME;
}

void automg42_destroy(gentity_t *gun, gentity_t *base, gentity_t *attacker, int time_to_destroy)
{
	// one of us has already come through here. doing it twice breaks shit and double prints "destroyed"
	if (gun->think == automg42_free ||
		base->think == automg42_free)
	{
		return;
	}

	if (gun->parent &&
		gun->parent->inuse &&
		gun->parent->client &&
		gun->parent->client->pers.connected == CON_CONNECTED &&
		gun->parent->client->sess.sessionTeam == gun->aiTeam)
	{
		if (!attacker->client)
		{
			AP(va("print \"%s^7's automg42 was destroyed\n\"", gun->parent->client->pers.netname));
		}
		else if (attacker == gun->parent)
		{
			AP(va("print \"%s^7 destroyed %s own automg42\n\"", gun->parent->client->pers.netname, attacker->client->sess.gender == 0 ? "his" : "her"));
		}
		else
		{
			AP(va("print \"%s^7's automg42 was destroyed by %s^7\n\"", gun->parent->client->pers.netname, attacker->client->pers.netname));
		}
	}
	else
	{
		if (attacker->client)
		{
			AP(va("print \"Automg42 was destroyed by %s^7\n\"", attacker->client->pers.netname));
		}
		else
		{
			AP("print \"Automg42 was destroyed\n\"");
		}
	}

	automg42_remove(base, time_to_destroy);
}

void automg42_die(gentity_t *ent, gentity_t *inflictor, gentity_t *attacker, int damage, int mod)
{
	gentity_t *base, *gun;
	gentity_t *parent_base, *parent_gun;
	int count = 0, parent_entity_num;

	if (!G_Automg42GetGunAndBase(ent, &gun, &base))
	{
		return;
	}

	automg42_destroy(gun, base, attacker, level.time);

	if (g_automg42TowerCollapse.integer)
	{
		parent_entity_num = gun->poisonEnt;
		while (parent_entity_num != -1 && G_Automg42GetGunAndBase(&g_entities[parent_entity_num], &parent_gun, &parent_base))
		{
			if (parent_base->think != automg42_free)
			{
				automg42_destroy(parent_gun, parent_base, attacker, level.time + (++count * 100));
			}

			parent_entity_num = parent_gun->poisonEnt;
		}
	}
}

void automg42_disable(gentity_t *ent, gentity_t *inflictor, gentity_t *attacker, int damage, int mod)
{
	gentity_t *base, *gun;

	if (!G_Automg42GetGunAndBase(ent, &gun, &base))
	{
		return;
	}

	if (gun->s.frame == 2)
	{
		return;
	}

	gun->s.frame = 2;
	gun->s.eFlags = EF_SMOKING;

	if (gun->parent &&
		gun->parent->inuse &&
		gun->parent->client &&
		gun->parent->client->pers.connected == CON_CONNECTED &&
		gun->parent->client->sess.sessionTeam == gun->aiTeam)
	{
		if (!attacker->client)
		{
			AP(va("print \"%s^7's automg42 was disabled\n\"", gun->parent->client->pers.netname));
		}
		else if (attacker == gun->parent)
		{
			AP(va("print \"%s^7 disabled %s own automg42\n\"", gun->parent->client->pers.netname, attacker->client->sess.gender == 0 ? "his" : "her"));
		}
		else
		{
			AP(va("print \"%s^7's automg42 was disabled by %s^7\n\"", gun->parent->client->pers.netname, attacker->client->pers.netname));
		}
	}
	else
	{
		if (attacker->client)
		{
			AP(va("print \"Automg42 was disabled by %s^7\n\"", attacker->client->pers.netname));
		}
		else
		{
			AP("print \"Automg42 was disabled\n\"");
		}
	}

	gun->think = NULL;
	gun->nextthink = 0;

	G_Automg42UpdateHealth(gun, gun->health);
}

void automg42_pain(gentity_t *ent, gentity_t *attacker, int damage, vec3_t point)
{
	gentity_t *gun, *base;

	if (!G_Automg42GetGunAndBase(ent, &gun, &base))
	{
		return;
	}

	if (gun->health <= gun->count2 * 0.25f)
	{
		if (g_automg42Disable.integer)
		{
			automg42_disable(gun, attacker, attacker, damage, MOD_MACHINEGUN);
		}
		else
		{
			gun->s.eFlags = EF_SMOKINGBLACK;
		}
	}
}

void automg42_base_damage_think(gentity_t *base)
{
	gentity_t *gun = base->chain;

	if (level.time > gun->timestamp)
	{
		base->think = 0;
		base->takedamage = gun->takedamage = qtrue;
		return;
	}

#if 0
	float frac = (5000.0f - ((float)gun->timestamp - (float)level.time)) / 5000.0f;
	G_Automg42SetConstantLight(gun, frac * gun->radius);
#endif

	base->nextthink = level.time + 50;
}

void automg42_base_soaring(gentity_t *base)
{
	gentity_t *gun;
	vec3_t gun_position;
	trace_t tr;

	gun = base->chain;
	base->nextthink = level.time + FRAMETIME;

	// we're still soaring
	if (base->s.pos.trType == TR_GRAVITY)
	{
		// if we're still "soaring" after 5 seconds, then something is wrong - remove the entities.
		if (level.time - base->timestamp >= 5000)
		{
			automg42_remove(base, level.time);
			return;
		}

		return;
	}

	// if we're still entering this function after 30 seconds, then something is wrong - remove the entities.
	if (level.time - base->timestamp >= 30000)
	{
		automg42_remove(base, level.time);
		return;
	}

	// finished soaring, trace for player contact. if yes, then wait until the player gets the fuck out of us before solidifying our contents and showing the gun.
	if (base->s.pos.trType == TR_STATIONARY)
	{
		trap_Trace(&tr, base->r.currentOrigin, base->r.mins, base->r.maxs, base->r.currentOrigin, base->s.number, CONTENTS_PLAYERCLIP | CONTENTS_BODY);

		if (tr.entityNum < MAX_CLIENTS)
		{
			return;
		}

		VectorCopy(base->r.currentOrigin, gun_position);
		gun_position[2] += base->r.maxs[2];

		trap_Trace(&tr, gun_position, gun->r.mins, gun->r.maxs, gun_position, gun->s.number, CONTENTS_PLAYERCLIP | CONTENTS_BODY);

		if (tr.entityNum < MAX_CLIENTS)
		{
			return;
		}

		base->r.contents = CONTENTS_SOLID;
		base->clipmask = CONTENTS_SOLID;

		gun->r.contents = CONTENTS_SOLID;
		gun->clipmask = CONTENTS_SOLID;

		G_SetOrigin(gun, gun_position);

		trap_LinkEntity(gun);

		// using poisonEnt for fortnite tower chained destruction
		if (base->s.groundEntityNum != -1 && base->s.groundEntityNum != ENTITYNUM_WORLD)
		{
			gentity_t *gun_below, *base_below;
			if (G_Automg42GetGunAndBase(&g_entities[base->s.groundEntityNum], &gun_below, &base_below))
			{
				gun_below->poisonEnt = base_below->poisonEnt = base->s.number;
			}
		}
	}

	// we're finished soaring, time for the gun.

	base->think = automg42_base_damage_think;
	base->nextthink = level.time + FRAMETIME;

	gun->think = automg42_think;
	gun->nextthink = level.time + FRAMETIME;

	// when gun & base can be damaged.
	gun->timestamp = level.time + 5000;
}

void automg42_spawn(gentity_t *ent)
{
	gentity_t *base, *gun;
	vec3_t scale_vec;
	float scale;

	// look for general scaling (this is for .spawns file support. the client command won't use this)
	if (G_SpawnFloat("modelscale", "1", &scale_vec[0])) {
		ent->s.angles2[2] = ent->s.angles2[1] = ent->s.angles2[0] = scale_vec[0];
	}

	// look for axis specific scaling (this is for .spawns file support. the client command won't use this)
	if (G_SpawnVector("modelscale_vec", "1 1 1", &scale_vec[0])) {
		VectorCopy(scale_vec, ent->s.angles2);
	}

	scale = ent->s.angles2[0];
	VectorSet(ent->s.angles2, scale, scale, scale);

	base = G_Spawn();

	VectorCopy(ent->s.angles, base->s.angles);
	VectorCopy(ent->s.angles, base->s.apos.trBase);

	VectorCopy(default_base_mins, base->r.mins);
	VectorCopy(default_base_maxs, base->r.maxs);
	VectorScale(base->r.mins, scale, base->r.mins);
	VectorScale(base->r.maxs, scale, base->r.maxs);

	LaunchEntity(base, ent->s.origin, ent->s.pos.trDelta, ent->parent ? ent->parent->s.number : ENTITYNUM_NONE);

	base->s.apos.trType = 1;
	VectorCopy(ent->s.angles2, base->s.angles2);
	base->s.eType = ET_GAMEMODEL;
	base->s.groundEntityNum = -1;
	base->poisonEnt = -1;

	base->timestamp = level.time;
	base->think = automg42_base_soaring;
	base->nextthink = level.time + FRAMETIME;
	base->classname = "mg42_base";
	base->takedamage = qfalse;
	base->die = automg42_die;

	base->s.modelindex = G_ModelIndex("models/mapobjects/weapons/mg42b.md3");

	base->health = 0;
	trap_LinkEntity(base);

	gun = G_Spawn();

	VectorCopy(default_gun_mins, gun->r.mins);
	VectorCopy(default_gun_maxs, gun->r.maxs);
	VectorScale(gun->r.mins, scale, gun->r.mins);
	VectorScale(gun->r.maxs, scale, gun->r.maxs);

	gun->s.apos.trType = 1;
	VectorCopy(ent->s.angles2, gun->s.angles2);
	gun->s.eType = ET_GAMEMODEL;
	gun->poisonEnt = -1;

	gun->classname = "misc_mg42";
	gun->health = 0;

	gun->last_move_time = 0;
	gun->s.modelindex = G_ModelIndex("models/multiplayer/mg42/mg42.md3");

	VectorCopy(ent->s.angles, gun->s.angles);
	VectorCopy(gun->s.angles, gun->s.apos.trBase);

	gun->think = 0;
	gun->nextthink = 0;
	gun->touch = 0;
	gun->use = 0;
	gun->die = automg42_die;
	gun->s.number = gun - g_entities;
	gun->takedamage = qfalse;
	gun->damage = ent->damage;
	gun->accuracy = ent->accuracy;
	gun->s.frame = 2;
	gun->mg42BaseEnt = base->s.number;

	base->chain = gun;
	gun->parent = base->parent = ent->parent;
	gun->aiTeam = base->aiTeam = ent->aiTeam;
	gun->pain = base->pain = automg42_pain;
	gun->count2 = base->count2 = ent->count2;
	gun->radius = base->radius = ent->radius;

	if (ent->health >= (gun->count2 * MG42_FIXED_HEALTH_MULT))
	{
		gun->s.frame = 0;
	}

	G_Automg42UpdateHealth(gun, ent->health);

	if (gun->aiTeam == TEAM_RED)
	{
		level.axis_automg42_count++;
	}
	else
	{
		level.allies_automg42_count++;
	}

	G_FreeEntity(ent);
}