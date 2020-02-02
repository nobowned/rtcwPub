/*/
===========================================================================
L0 / rtcwPub :: g_players.h

	New Player's commands..

Created: 25. Mar / 2014
===========================================================================
*/
#include "g_admin.h"

/*
===========
Getstatus

Prints IP's and some match info..
===========
*/
void Cmd_getStatus(gentity_t *ent) {
	gclient_t *cl;
	int	j;
	char server_ip[16];
	int server_port;
	char *ip_out = NULL;
	// uptime
	int secs = level.time / 1000;
	int mins = (secs / 60) % 60;
	int hours = (secs / 3600) % 24;
	int days = (secs / (3600 * 24));

	trap_Cvar_VariableStringBuffer("sv_ipExternal", server_ip, sizeof(server_ip));
	if (server_ip[0])
	{
		server_port = trap_Cvar_VariableIntegerValue("net_port");
		ip_out = va("^3IP: ^7%s:%i\n", server_ip, server_port);
	}

	CP(va("print \"\n^3Mod: ^7%s \n^3Server: ^7%s\n%s\"", GAMEVERSION, GetHostname(), ip_out ? ip_out : ""));
	CP("print \"^3--------------------------------------------------------------------------\n\"");
	CP("print \"^7CN : Team : Name            : ^3IP              ^7: Ping ^7: Fps  : Status\n\"");
	CP("print \"^3--------------------------------------------------------------------------\n\"");

	for (j = 0; j <= (MAX_CLIENTS - 1); j++) {

		if (g_entities[j].client && !(ent->r.svFlags & SVF_BOT)) {
			char *team, *slot, *ip, *status, *adminTag, *ignoreStatus;
			int ping, fps;
			cl = g_entities[j].client;

			// player is connecting
			if (cl->pers.connected == CON_CONNECTING) {
				CP(va("print \"%2i : >><< :                 : ^3>>Connecting<<  ^7:      : \n\"", j));
				continue;
			}

			// player is connected
			if (cl->pers.connected == CON_CONNECTED) {
				status = "";
				ignoreStatus = "";
				slot = va("%2d", j);
				adminTag = "";

				team = (cl->sess.sessionTeam == TEAM_SPECTATOR) ? "^3Spec^7" :
					(cl->sess.sessionTeam == TEAM_RED ? "^1Axis^7" : "^4Ally^7");

				ip = clientIP(&g_entities[j], (qboolean)(ent->client->sess.admin > ADM_NONE));

				ping = g_alternatePing.integer ? cl->pers.alternatePing : cl->ps.ping;
				if (ping > 999) ping = 999;

				fps = ClientGetFps(&cl->ps);

				adminTag = getTag(&g_entities[j]);

				if (cl->sess.admin > ADM_NONE && cl->sess.incognito)
					adminTag = va("%s ^7*", adminTag);

				switch (cl->sess.ignored) {
				case IGNORE_OFF:
					ignoreStatus = "";
					break;
				case IGNORE_PERMANENT:
				case IGNORE_TEMPORARY:
					ignoreStatus = "^3Ignored";
					break;
				}

				if (COM_BitCheck(ent->client->sess.muted, cl - level.clients)) {
					ignoreStatus = "^3Muted";
				}

				if (ent->client->sess.admin == ADM_NONE) {
					status = strlen(ignoreStatus) ? ignoreStatus : (!cl->sess.incognito) ? adminTag : "";
				} 
				else if (ent->client->sess.admin != ADM_NONE) {
					status = strlen(ignoreStatus) ? ignoreStatus : adminTag;
				}

				if (cl->sess.secretlyDemoing) {
					if (ent->client->sess.admin == ADM_NONE) {
						continue;
					}
					if (ent->client != cl) {
						status = "^3Demoing";
					}
				}

				// Print it now
				CP(va("print \"%-2s : %s : %s ^7: ^3%-15s ^7: %-4d ^7: %-4d : %-11s \n\"",
					slot,
					team,
					TablePrintableColorName(cl->pers.netname, 15),
					ip,
					ping,
					fps,
					status
					));
			}
		}
	}
	CP("print \"^3--------------------------------------------------------------------------\n\"");
	CP(va("print \"Time  : ^3%s \n^7Uptime: ^3%d ^7day%s ^3%d ^7hours ^3%d ^7minutes\n\"", getDateTime(), days, (days != 1 ? "s" : ""), hours, mins));
	CP("print \"\n\"");

	return;
}

/*
================
Throw knives

Originally BOTW (tho took it from s4ndmod as it's faster to implement..).
================
*/
// Knife "touch" function                                                                                                                                                                                                                                                                                               ////////////////////////////////////////////
void Touch_Knife(gentity_t *ent, gentity_t *other, trace_t *trace) {
	qboolean hurt = qfalse;
	ent->active = qfalse;

	if (!other->client) {
		return;
	}
	if (other->health < 1) {
		return;
	}

	if (VectorLength(ent->s.pos.trDelta) != 0) {
		if ((g_friendlyFire.integer) || (!OnSameTeam(other, ent->parent))) {
			int i;
			int sound;
			int damage = g_throwingKnifeDamage.integer;

			if (damage <= 0) {
				damage = 1;
			}

			// pick a random sound to play
			i = rand() % 3;
			if (i == 0) {
				sound = G_SoundIndex("/sound/weapons/knife/knife_hit1.wav");
			}
			else if (i == 1) {
				sound = G_SoundIndex("/sound/weapons/knife/knife_hit2.wav");
			}
			else {
				sound = G_SoundIndex("/sound/weapons/knife/knife_hit3.wav");
			}

			G_Sound(other, sound);
			G_Damage(other, ent->parent, ent->parent, NULL, trace->endpos, damage, 0, MOD_THROWKNIFE);
			hurt = qtrue;
		}
	}

	if (hurt == qfalse) {
		int makenoise = EV_ITEM_PICKUP;

		if (g_throwKnives.integer > 0) {
			other->client->pers.throwingKnives++;
		}
		if (g_throwKnives.integer != 0) {
			if (other->client->pers.throwingKnives > (g_throwKnives.integer + 5)) {
				other->client->pers.throwingKnives = (g_throwKnives.integer + 5);
			}
		}
		if (ent->noise_index) {
			makenoise = EV_ITEM_PICKUP_QUIET;
			G_AddEvent(other, EV_GENERAL_SOUND, ent->noise_index);
		}
		if (other->client->pers.predictItemPickup) {
			G_AddPredictableEvent(other, makenoise, ent->s.modelindex);
		}
		else {
			G_AddEvent(other, makenoise, ent->s.modelindex);
		}
	}

	ent->freeAfterEvent = qtrue;
	ent->flags |= FL_NODRAW;
	ent->r.svFlags |= SVF_NOCLIENT;
	ent->s.eFlags |= EF_NODRAW;
	ent->r.contents = 0;
	ent->nextthink = 0;
	ent->think = 0;
}
// Actual command
void Cmd_throwKnives(gentity_t *ent) {
	vec3_t velocity, angles, offset, org, mins, maxs;
	trace_t tr;
	gentity_t *ent2;
	gitem_t *item = BG_FindItemForWeapon(WP_KNIFE);

	if (g_throwKnives.integer == 0) {
		return;
	}

	if (level.time < (ent->thrownKnifeTime + 800)) {
		return;
	}

	// If out or -1/unlimited
	if ((ent->client->pers.throwingKnives == 0) &&
		(g_throwKnives.integer != -1)) {
		return;
	}

	AngleVectors(ent->client->ps.viewangles, velocity, NULL, NULL);
	VectorScale(velocity, 64, offset);
	offset[2] += ent->client->ps.viewheight / 2;
	VectorScale(velocity, 800, velocity);
	velocity[2] += 50 + random() * 35;
	VectorAdd(ent->client->ps.origin, offset, org);
	VectorSet(mins, -ITEM_RADIUS, -ITEM_RADIUS, 0);
	VectorSet(maxs, ITEM_RADIUS, ITEM_RADIUS, 2 * ITEM_RADIUS);
	trap_Trace(&tr, ent->client->ps.origin, mins, maxs, org, ent->s.number, MASK_SOLID);
	VectorCopy(tr.endpos, org);

	G_Sound(ent, G_SoundIndex("sound/weapons/knife/knife_slash1.wav"));
	ent2 = LaunchItem(item, org, velocity, ent->client->ps.clientNum);
	VectorCopy(ent->client->ps.viewangles, angles);
	angles[1] += 90;
	G_SetAngle(ent2, angles);
	ent2->touch = Touch_Knife;
	ent2->parent = ent;

	if (g_throwKnives.integer > 0) {
		ent->client->pers.throwingKnives--;
	}

	//only show the message if throwing knives are enabled
	if (g_throwKnives.integer > 0) {
		CP(va("chat \"^3Knives left:^7 %d\" %i", ent->client->pers.throwingKnives, qfalse));
	}

	ent->thrownKnifeTime = level.time;
}

/*
================
Private messages
================
*/
void Cmd_SendPrivateMessage_f(gentity_t *ent) {
	char cmd[MAX_TOKEN_CHARS];
	char name[MAX_STRING_CHARS];
	char nameList[MAX_STRING_CHARS];
	char msg[MAX_STRING_CHARS];
	int matchList[MAX_CLIENTS];
	int count, i, cantMsgHidden = 0, hiddenCantMsg = 0, cantMsgMuted = 0;

	if (!g_allowPMs.integer) {
		CP("print \"Private messages are disabled^1!\n\"");
		return;
	}

	if (trap_Argc() < 3) {
		trap_Argv(0, cmd, sizeof(cmd));
		CP(va("print \"^3Usage:^7  %s <match> <message>\n\"", cmd));
		return;
	}

	if (!ent->client->sess.secretlyDemoing && ent->client->sess.ignored > IGNORE_OFF) {
		CP(va("cp \"You are %s ignored^1!\n\"2",
			ent->client->sess.ignored == IGNORE_PERMANENT ? "permanently" : "temporarily"));
		return;
	}

	trap_Argv(1, name, sizeof(name));
	if (strlen(name) < 2) {
		CP("print \"You must match at least ^32 ^7characters of the name^1!\n\"");
		return;
	}

	count = ClientNumbersFromName(name, matchList);
	if (count == 0) {
		CP("print \"No matching clients found^1!\n\"");
		return;
	}

	Q_SanitizeClientTextInput(ConcatArgs(2), msg, sizeof(msg), qfalse);

	Q_strncpyz(nameList, "", sizeof(nameList));
	for (i = 0; i < count; i++)	{
		gentity_t *sendto = g_entities + matchList[i];

		if (sendto->client->sess.secretlyDemoing && ent->client->sess.admin == ADM_NONE) {
			cantMsgHidden++;
			continue;
		}

		if (ent->client->sess.secretlyDemoing && sendto->client->sess.admin == ADM_NONE) {
			hiddenCantMsg++;
			continue;
		}

		if (COM_BitCheck(sendto->client->sess.muted, ent - g_entities)) {
			cantMsgMuted++;
			continue;
		}

		strcat(nameList, sendto->client->pers.netname);
		if (i != (count - 1))
			strcat(nameList, "^7, ");

		// Notify receiver(s)
		CPx(matchList[i], "cp \"^3New Private Message^7!\n\"2");

		// Print full message in console..
		CPx(matchList[i], va("print \"^7Message from ^7%s^7: ^3%s\n\"", ent->client->pers.netname, msg));
	}

	if (hiddenCantMsg == count) {
		CP("print \"No matching admins found^1!\n\"");
		return;
	}

	if (cantMsgHidden == count) {
		CP("print \"No matching clients found^1!\n\"");
		return;
	}

	if (cantMsgMuted == count) {
		CP("print \"You're muted^1! ^7D:\n\"");
		return;
	}

	// Let player know who received his message
	CP(va("print \"^7Message was sent to ^7%s^7: ^3%s\n\"", nameList, msg));
}

/*
================
Shows time
================
*/
void Cmd_Time(gentity_t *ent) {
	CP(va("chat \"%s^7, the server time is ^3%s\n\"", ent->client->pers.netname, getDateTime()));
	CPS(ent, "sound/multiplayer/dynamite_01.wav");
}

/*
===================
Drag players

Came from BOTW/S4NDMoD
===================
*/
void Cmd_Drag(gentity_t *ent) {
	gentity_t *target;
	vec3_t start, dir, end;
	trace_t tr;
	target = NULL;

	if (!g_dragBodies.integer)
		return;

	if (level.time < (ent->lastDragTime + 20))
		return;

	if (ent->client->ps.stats[STAT_HEALTH] <= 0)
		return;

	AngleVectors(ent->client->ps.viewangles, dir, NULL, NULL);

	VectorCopy(ent->s.pos.trBase, start);
	start[2] += ent->client->ps.viewheight;
	VectorMA(start, 100, dir, end);

	trap_Trace(&tr, start, NULL, NULL, end, ent->s.number, CONTENTS_CORPSE);

	if (tr.entityNum >= MAX_CLIENTS)
		return;

	target = &g_entities[tr.entityNum];

	if (!target->inuse || !target->client || target->client->ps.stats[STAT_HEALTH] > 0)
		return;

	VectorCopy(target->r.currentOrigin, start);
	VectorCopy(ent->r.currentOrigin, end);
	VectorSubtract(end, start, dir);
	VectorNormalize(dir);
	VectorScale(dir, 100, target->client->ps.velocity);
	VectorCopy(dir, target->movedir);

	ent->lastDragTime = level.time;
}

/*
=================
L0 - Shove players away

Ported from BOTW source.
=================
*/
void Cmd_Push(gentity_t* ent)
{
	gentity_t *target;
	trace_t tr;
	vec3_t start, end, forward;
	float shoveAmount;

	if (!g_shove.integer)
		return;

	if (ent->client->ps.stats[STAT_HEALTH] <= 0)
		return;

	if (level.time < (ent->lastPushTime + 600))
		return;

	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);

	VectorCopy(ent->s.pos.trBase, start);
	start[2] += ent->client->ps.viewheight;
	VectorMA(start, 128, forward, end);

	trap_Trace(&tr, start, NULL, NULL, end, ent->s.number, CONTENTS_BODY);

	if (tr.entityNum >= MAX_CLIENTS)
		return;

	target = &(g_entities[tr.entityNum]);

	if ((!target->inuse) || (!target->client))
		return;

	if (target->client->ps.stats[STAT_HEALTH] <= 0)
		return;

	// L0
	// Push is meant to get rid of blockers...not to push enemy players..
	// Most that use drop reload script, push players when they (drop)reload
	// which is F annoying in a crowded server..
	// If enemy is blocking your way, KILL HIM that's the F point.
	if (ent->client->sess.sessionTeam != target->client->sess.sessionTeam)
		return;

	// Nobo
	// No pushing if target is in motion
	if (VectorLengthSquared(target->client->ps.velocity) > 0)
		return;

	shoveAmount = 512 * .8;
	VectorMA(target->client->ps.velocity, shoveAmount, forward, target->client->ps.velocity);

	APRS(target, "sound/multiplayer/vo_revive.wav");

	ent->lastPushTime = level.time;
}

/*
=================
Drop objective

Port from NQ
=================
*/
void Cmd_dropObj(gentity_t *self)
{
	gitem_t *item = NULL;

	// drop flag regardless
	if (self->client->ps.powerups[PW_REDFLAG]) {
		item = BG_FindItem("Red Flag");
		if (!item)
			item = BG_FindItem("Objective");

		self->client->ps.powerups[PW_REDFLAG] = 0;
	}
	if (self->client->ps.powerups[PW_BLUEFLAG]) {
		item = BG_FindItem("Blue Flag");
		if (!item)
			item = BG_FindItem("Objective");

		self->client->ps.powerups[PW_BLUEFLAG] = 0;
	}

	if (item) {
		vec3_t launchvel = { 0, 0, 0 };
		vec3_t forward;
		vec3_t origin;
		vec3_t angles;
		gentity_t *flag;

		VectorCopy(self->client->ps.origin, origin);
		// tjw: if the player hasn't died, then assume he's
		//      throwing objective per g_dropObj
		if (self->health > 0) {
			VectorCopy(self->client->ps.viewangles, angles);
			if (angles[PITCH] > 0)
				angles[PITCH] = 0;
			AngleVectors(angles, forward, NULL, NULL);
			VectorMA(self->client->ps.velocity,
				96, forward, launchvel);
			VectorMA(origin, 36, forward, origin);
			origin[2] += self->client->ps.viewheight;
		}

		flag = LaunchItem(item, origin, launchvel, self->s.number);

		flag->s.modelindex2 = self->s.otherEntityNum2;// JPW NERVE FIXME set player->otherentitynum2 with old modelindex2 from flag and restore here
		flag->message = self->message;	// DHM - Nerve :: also restore item name
		// Clear out player's temp copies
		self->s.otherEntityNum2 = 0;
		self->message = NULL;
		self->droppedObj = qtrue;
	}
	else
	{
		Cmd_throwKnives(self);
	}
}

/*
=================
Stats command

TODO: Unlazy my self and add targeted stats (victim, killer, <client number>)
=================
*/
void Cmd_Stats(gentity_t *ent) {
	gclient_t *client = ent->client;
	int eff;
	int deaths = client->pers.deaths;
	float killRatio = client->pers.kills;
	int shots = client->pers.acc_shots;
	float acc = 0.0f;

	if (deaths > 0)
		killRatio = (float)client->pers.kills / (float)deaths;

	if (shots > 0)
		acc = ((float)client->pers.acc_hits / (float)shots) * 100.0f;

	eff = (client->pers.deaths + client->pers.kills == 0) ? 0 : 100 * client->pers.kills / (client->pers.deaths + client->pers.kills);
	if (eff < 0) {
		eff = 0;
	}

	CP(va("print \"\n"
		"^7Mod: %s \n"
		"^7Server: %s \n"
		"^7Time: ^3%s \n\n"
		"^7Stats for %s ^7this round: \n"
		"^7------------------------------------------------------\n"
		"^3Kills       ^7%3d | ^3Team Kills    ^7%3d | ^3Poisons     ^7%3d\n"
		"^3Deaths      ^7%3d | ^3Suicides      ^7%3d | ^3Gibs        ^7%3d\n"
		"^3Revives     ^7%3d | ^3Health Given  ^7%3d | ^3Ammo Given  ^7%3d\n"
		"^3Dmg Given  ^7%4d | ^3Dmg Received ^7%4d | ^3Team Dmg   ^7%4d\n"
		"^3Headshots   ^7%3d | ^3Accuracy      ^7%6.2f (%d/%d)\n"
		"^3Efficiency  ^7%3d | ^3Kill Ratio    ^7%6.2f\n"
		"^7------------------------------------------------------\n"
		"\n\"",
		GAMEVERSION,
		sv_hostname.string,
		getDateTime(),
		client->pers.netname,
		client->pers.kills, client->pers.teamKills, client->pers.poison,
		deaths, client->pers.suicides, client->pers.gibs,
		client->pers.revives, client->pers.medPacks, client->pers.ammoPacks,
		client->pers.dmgGiven, client->pers.dmgReceived, client->pers.dmgTeam,
		client->pers.headshots, acc, client->pers.acc_hits, shots,
		eff, killRatio
		));
}

/*
================
Smoke

Based on BOTW
================
*/
void Cmd_Smoke(gentity_t *ent)
{
	gclient_t *client = ent->client;
	char message[64] = "Smoke grenade ";

	if (!g_smokeGrenades.integer)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Smoke grenades are disabled on this server.\n\""));
		return;
	}

	if ((g_smokeGrenadesLmt.integer) && (ent->thrownSmoke >= g_smokeGrenadesLmt.integer))
	{
		trap_SendServerCommand(ent - g_entities, va("cp \"You have used all the Smoke supplies!\n\" 1"));
		return;
	}

	client->ps.selectedSmoke = !client->ps.selectedSmoke;
	strcat(message, va("%s", (client->ps.selectedSmoke ? "^2enabled" : "^1disabled")));

	trap_SendServerCommand(ent - g_entities, va("cp \"%s^7!\n\" 1", message));
}

// think function
extern void G_ExplodeMissile(gentity_t *ent);
void weapon_smokeGrenade(gentity_t *ent)
{
	ent->s.eFlags |= EF_SMOKINGBLACK;
	ent->s.loopSound = G_SoundIndex("sound/world/steam.wav");
	ent->nextthink = level.time + ((g_smokeGrenades.integer - 1) * 1000);
	ent->think = G_ExplodeMissile;
}
