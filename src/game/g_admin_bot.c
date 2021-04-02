/*/
===========================================================================
L0 / rtcwPub :: g_admin.c

	Server (Admin) Bot (SB) functionality.

Created: 26. Mar / 2014
===========================================================================
*/
#include "g_local.h"

/*
===========
Kicks for teamkills
===========
*/
void SB_maxTeamKill(gentity_t *ent) {
	int count = ent->client->pers.stats.teamKills - ent->client->pers.sb_TKforgiven;

	if (level.warmupTime || !sb_system.integer || sb_maxTKs.integer < 0)
	{
		return;
	}

	if (sb_maxTKs.integer - count == 1 && !ent->client->pers.sb_TKwarned)
	{
		CP(va("chat \"console: ^1Warning! ^7Continue and get %s on next ^3Team Kill^7!\n\"", sb_maxTKsTempbanMins.integer ? "tempbanned" : "kicked"));
		ent->client->pers.sb_TKwarned = qtrue;
	}

	// Give them some time to make it up.. (ie tk-revive)
	ent->client->pers.sb_TKkillTime = level.time + 10000;
	return;
}

/*
===========
Kicks for bleeding
===========
*/
void SB_maxTeamBleed(gentity_t *ent) {
	if (level.warmupTime || !sb_system.integer || sb_maxTeamBleed.integer < 0)
		return;

	int count = ent->client->pers.stats.dmgTeam;
	int tempban = sb_maxTeamBleedTempbanMins.integer;

	if ((sb_maxTeamBleed.integer - count) <= 50 && !ent->client->pers.sb_TBwarned) {
		CP(va("chat \"console: ^1Warning! ^7You're close to being %s for ^3Team Bleeding^7!\n\"", tempban ? "tempbanned" : "kicked"));
		ent->client->pers.sb_TBwarned = qtrue;
	}

	if ((count >= sb_maxTeamBleed.integer) && count) {
		// Tempban
		if (tempban)
		{	
			trap_SendConsoleCommand(EXEC_APPEND, va("tempban %i %i\n", ent->s.clientNum, tempban));
		}

		AP(va("chat \"console: %s ^7got %s for ^3Team Bleeding^7.\n\"", ent->client->pers.netname, tempban ? "tempbanned": "kicked"));

		trap_DropClient(ent - g_entities, va("%s \n^3For Team Bleeding.", tempban ? "Tempbanned" : "Kicked"));
	}
	return;
}

/*
===========
Kicks for low score
===========
*/
void SB_minLowScore(gentity_t *ent) {
	// Positive values are ignored.
	if (level.warmupTime || !sb_system.integer || sb_minLowScore.integer >= 0)
		return;

	if (ent->client->ps.persistant[PERS_SCORE] < sb_minLowScore.integer){
		int tempban = sb_minLowScoreTempbanMins.integer;

		// Tempban
		if (tempban)
		{
			trap_SendConsoleCommand(EXEC_APPEND, va("tempban %i %i\n", ent->s.clientNum, tempban));
		}

		AP(va("chat \"console: %s ^7got %s for ^3Low Score^7.\n\"", ent->client->pers.netname, tempban ? "tempbanned" : "kicked"));
		trap_DropClient(ent->s.clientNum, va("%s \n^3For Low Score.", tempban ? "Tempbanned" : "Kicked"));
	}
	return;
}

/*
===========
Kicks laggers
===========
*/
void SB_maxPingFlux(gclient_t *client) {
	if (level.warmupTime || !sb_system.integer || sb_maxPingFlux.integer < 50 || client->pers.connected == CON_DISCONNECTED)
		return;

	if (client->pers.sb_pingHits >= sb_maxPingHits.integer) {
		trap_DropClient(client - level.clients, "Kicked \nDue to unstable ping or max ping limit.");
		AP(va("chat \"console: %s ^7got kicked due to ^3Unstable Ping^7.\n\"", client->pers.netname));
	}
	else if (client->ps.ping >= sb_maxPingFlux.integer && level.time > client->pers.sb_maxPingTime) {
		client->pers.sb_pingHits++;
		client->pers.sb_maxPingTime = level.time + 1000;
	}

	return;
}

/*
===========
Censoring penalty
===========
*/
void SB_chatWarn(gentity_t *ent) {
	int n = rand() % 4; // Randomize messages

	if (!sb_system.integer || (sb_system.integer && !sb_censorPenalty.integer))
		return;

	// Only for non logged in users..
	// Chat will still get censored they just wont get kicked or ignored for it..
	if (ent->client->sess.admin != ADM_NONE)
		return;

	if (ent->client->pers.sb_chatWarned == 0) {
		if (n == 0)
			CP("chat \"^3Strike one! ^7You should really wash your mouth.\n\"");
		else if (n == 1)
			CP("chat \"^3Strike one! ^7You got warned for foul language..\n\"");
		else if (n == 2)
			CP("chat \"^3Strike one! ^7This is not your local pub..\n\"");
		else
			CP("chat \"^3Strike one! ^7Cursing is not allowed here.\n\"");
	}
	else if (ent->client->pers.sb_chatWarned == 1) {
		if (n == 0)
			CP("chat \"^3Strike two! ^7Don't push it..\n\"");
		else if (n == 1)
			CP("chat \"^3Strike two! ^7You where warned..\n\"");
		else if (n == 2)
			CP("chat \"^3Strike two! ^7Do you talk to your parents like this?\n\"");
		else
			CP("chat \"^3Strike two! ^7Foul language is not allowed here.\n\"");
	}
	else if (ent->client->pers.sb_chatWarned == 2) {
		if (n == 0)
			CP("chat \"^3Strike three! ^7Last warning!\n\"");
		else if (n == 1)
			CP("chat \"^3Strike three! ^7There wont be strike four..\n\"");
		else if (n == 2)
			CP("chat \"^3Strike three! ^7There's no more warnings after this one.\n\"");
		else
			CP("chat \"^3Strike three! ^7Care to see how strike four looks like?\n\"");
	}
	else {
		if (sb_censorPenalty.integer == 1) {
			ent->client->sess.ignored = IGNORE_PERMANENT;
			AP(va("chat \"console: %s ^7has been ignored due to foul language!\n\"", ent->client->pers.netname));
		}
		else {
			int tempban = sb_censorPentalityTempbanMin.integer;

			// Tempban
			if (tempban)
			{	
				trap_SendConsoleCommand(EXEC_APPEND, va("tempban %i %i\n", ent->s.clientNum, tempban));
			}

			AP(va("chat \"console: %s ^7got %s for foul language!\n\"", ent->client->pers.netname, tempban ? "tempbanned" : "kicked"));
			trap_DropClient(ent - g_entities, va("%s for ^3Foul ^3Language!", tempban ? "Tempbanned" : "Kicked"));
		}
		return;
	}

	// Count it..
	ent->client->pers.sb_chatWarned++;
	// Use sound if they have sound pack..
	CPS(ent, "sound/player/warn.wav");

	return;
}


