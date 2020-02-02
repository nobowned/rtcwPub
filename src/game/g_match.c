/*/
===========================================================================
L0 / rtcwPub :: g_match.c

	Holds match specific stuff	

Created: 25. Mar / 2014
===========================================================================
*/
#include "g_local.h"

/*
=================
Countdown
=================
*/
void CountDown(void) {
	if ((level.cdFlags ^ (CD_PREPARE | CD_FIGHT)) == 0) {
		return;
	}

	int wuRemaining = level.warmupTime - level.time;

	// Countdown...
	if (wuRemaining > 1500 && !(level.cdFlags & CD_PREPARE))
	{
		APS("sound/player/prepare.wav");
		AP("cp \"Prepare to fight^1!\n\"2");

		level.cdFlags |= CD_PREPARE;
	}
	else if (wuRemaining <= 500 && !(level.cdFlags & CD_FIGHT))
	{
		APS("sound/player/fight_rb.wav");
		AP("print \"^1Fight!\n\"2");

		level.cdFlags |= CD_FIGHT;
	}

	// Auto shuffle if enabled and threshold is reached
	if (g_autoShuffle.integer)
	{
		if (shuffleTracking.integer >= g_autoShuffle.integer)
		{
			trap_SendConsoleCommand(EXEC_APPEND, "shuffle @print\n");
			trap_Cvar_Set("shuffleTracking", 0);
			AP("chat \"console: Teams were ^3Auto shuffled^7!\n\"");
		}
		// Notify that shuffle will occur the next round..
		else if ((g_autoShuffle.integer > 2) &&
			(shuffleTracking.integer == (g_autoShuffle.integer - 1)))
		{
			AP(("chat \"^3Notice: ^7Teams will be ^3Auto Shuffled ^7next round!\n\""));
		}
	}
}

/*
=================
Chicken Run

Originally from etPUB (i think)
=================
*/
gentity_t *G_FearCheck(gentity_t *ent) {
	qboolean fear = qfalse;
	gentity_t *attacker = &level.gentities[ent->client->lasthurt_client];

	if (g_chicken.integer && attacker && attacker->client &&
		(level.time - ent->client->lasthurt_time) < g_chicken.integer &&
		attacker->client->sess.sessionTeam != ent->client->sess.sessionTeam &&
		attacker->health > 0) {

		fear = qtrue;
	}

	return (fear ? attacker : NULL);
}

/*
=================
Match Info

Basically just some info prints..
=================
*/
// Gracefully taken from s4ndmod :p
char *GetLevelTime(void) {
	int Objseconds, Objmins, Objtens;

	Objseconds = (((g_timelimit.value * 60 * 1000) - ((level.time - level.startTime))) / 1000); // begin martin - this line was a bitch :-)
																								// L0 - I know, that's why I took it. :p
	Objmins = Objseconds / 60;
	Objseconds -= Objmins * 60;
	Objtens = Objseconds / 10;
	Objseconds -= Objtens * 10;

	if (Objseconds < 0) {
		Objseconds = 0;
	}
	if (Objtens < 0) {
		Objtens = 0;
	}
	if (Objmins < 0) {
		Objmins = 0;
	}

	return va("%i:%i%i", Objmins, Objtens, Objseconds);  //end martin
}
// Prints stuff
void matchInfo(unsigned int type, char *msg) {

	// End of Match info
	if (type == MT_EI)
	{
		if (g_gametype.integer == GT_WOLF_STOPWATCH) {
			if (g_currentRound.integer == 1) {
				AP(va("print \"*** ^3Clock set to: ^7%d:%02d\n\"",
					g_nextTimeLimit.integer,
					(int)(60.0 * (float)(g_nextTimeLimit.value - g_nextTimeLimit.integer))));
			}
			else {
				float val = (float)((level.time - (level.startTime + level.time - level.intermissiontime)) / 60000.0);
				if (val < g_timelimit.value)
				{
					AP(va("print \"*** ^3Objective reached at ^7%d:%02d ^3(original: ^7%d:%02d^3)\n\"",
						(int)val,
						(int)(60.0 * (val - (int)val)),
						g_timelimit.integer,
						(int)(60.0 * (float)(g_timelimit.value - g_timelimit.integer))));
				}
				else {
					AP(va("print \"*** ^3Objective NOT reached in time (^7%d:%02d^3)\n\"",
						g_timelimit.integer,
						(int)(60.0 * (float)(g_timelimit.value - g_timelimit.integer))));
				}
			}
		}
	}
	// Match events
	else if (type == MT_ME)
	{
		if (g_printMatchInfo.integer)
			AP(va("print \"[%s] ^3%s \n\"", GetLevelTime(), msg));
		else
			AP(va("cp \"%s \n\"", msg));
	}
}

/*
=================
AutoBalanceTeam

Checks if any team needs balancing
=================
*/
void checkEvenTeams(void) {
	if (!g_teamAutoBalance.integer ||
		(g_gamestate.integer != GS_PLAYING) ||
		!g_teamForceBalance.integer ||
		needsBalance.integer ||
		g_deathmatch.integer)
		return;

	if ((alliedPlayers.integer - axisPlayers.integer) > 1) {
		trap_Cvar_Set("needsBalance", "1");
		AP("chat \"console: ^1Axis ^7team will be balanced in ^115^7 seconds.\n\"");
		level.balanceTimer = level.time + 15000;
	}

	if ((axisPlayers.integer - alliedPlayers.integer) > 1) {
		trap_Cvar_Set("needsBalance", "1");
		AP("chat \"console: ^4Allied ^7team will be balanced in ^415^7 seconds.\n\"");
		level.balanceTimer = level.time + 15000;
	}
}

void balanceTeams(void) {
	int lowScoreClient = -1;
	int lowScore = 0;
	int i;

	while ( ((axisPlayers.integer - alliedPlayers.integer) > 1) ||
		((alliedPlayers.integer - axisPlayers.integer) > 1)) {

		lowScoreClient = -1;
		if ((axisPlayers.integer - alliedPlayers.integer) > 1) {

			for (i = 0; i < level.maxclients; i++) {
				if (level.clients[i].pers.connected != CON_CONNECTED)
					continue;
				if (level.clients[i].sess.sessionTeam != TEAM_RED)
					continue;

				if (lowScoreClient == -1) {
					lowScoreClient = i;
					lowScore = level.clients[i].ps.persistant[PERS_SCORE];

				}
				else if (level.clients[i].ps.persistant[PERS_SCORE] < lowScore) {
					lowScoreClient = i;
					lowScore = level.clients[i].ps.persistant[PERS_SCORE];
				}
			}

			SetTeam(&g_entities[lowScoreClient], "b", qtrue);
			AP(va("chat \"console: %s ^7was forced to ^4Allies ^7to balance the teams.\n\"",
				level.clients[lowScoreClient].pers.netname));
		}

		if ((alliedPlayers.integer - axisPlayers.integer) > 1) {

			for (i = 0; i < level.maxclients; i++) {
				if (level.clients[i].pers.connected != CON_CONNECTED)
					continue;
				if (level.clients[i].sess.sessionTeam != TEAM_BLUE)
					continue;

				if (lowScoreClient == -1) {
					lowScoreClient = i;
					lowScore = level.clients[i].ps.persistant[PERS_SCORE];

				}
				else if (level.clients[i].ps.persistant[PERS_SCORE] < lowScore) {
					lowScoreClient = i;
					lowScore = level.clients[i].ps.persistant[PERS_SCORE];
				}
			}

			SetTeam(&g_entities[lowScoreClient], "r", qtrue);
			AP(va("chat \"console: %s ^7was forced to ^1Axis ^7to balance the teams.\n\"",
				level.clients[lowScoreClient].pers.netname));
		}
	}
	trap_Cvar_Set("needsBalance", "0");
}

/*
=================
Weapon limiting

See if player can spawn with weapon...
TODO: Add first in line...
=================
*/
int sortWeaponLimit(int weap) {

	if (weap == SW_MAUSER) {
		if (g_maxTeamSniper.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamSniper.integer;
	}

	if (weap == SW_PANZER) {
		if (g_maxTeamPF.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamPF.integer;
	}

	if (weap == SW_VENOM) {
		if (g_maxTeamVenom.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamVenom.integer;
	}

	if (weap == SW_FLAMER) {
		if (g_maxTeamFlamer.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamFlamer.integer;
	}

	return g_maxclients.integer;
}

int isWeaponLimited(gclient_t *client, int weap) {
	int count = 0;

	// Limit
	if ((weap == SW_MAUSER) && (client->pers.restrictedWeapon != WP_MAUSER))
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisSniper : level.alliedSniper;
	else if ((weap == SW_PANZER) && (client->pers.restrictedWeapon != WP_PANZERFAUST))
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisPF : level.alliedPF;
	else if ((weap == SW_VENOM) && (client->pers.restrictedWeapon != WP_VENOM))
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisVenom : level.alliedVenom;
	else if ((weap == SW_FLAMER) && (client->pers.restrictedWeapon != WP_FLAMETHROWER))
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisFlamer : level.alliedFlamer;

	if (count >= sortWeaponLimit(weap))
		return 1;
	else
		return 0;

	return 0;
}

/*
=================
Weapon balance

Checks if there's enough of players per team so player
can spawn with heavy weapon..
=================
*/
qboolean isWeaponBalanced(int weapon) {
	// Sniper
	if (g_balanceSniper.integer && (weapon == SW_MAUSER)) {
		if ((axisPlayers.integer >= g_balanceSniper.integer) && (alliedPlayers.integer >= g_balanceSniper.integer))
			return qtrue;
		else
			return qfalse;
		// PF check
	}
	else if (g_balancePF.integer && (weapon == SW_PANZER)) {
		if ((axisPlayers.integer >= g_balancePF.integer) && (alliedPlayers.integer >= g_balancePF.integer))
			return qtrue;
		else
			return qfalse;
		// Venom check
	}
	else if (g_balanceVenom.integer && (weapon == SW_VENOM)) {
		if ((axisPlayers.integer >= g_balanceVenom.integer) && (alliedPlayers.integer >= g_balanceVenom.integer))
			return qtrue;
		else
			return qfalse;
		// Flamer check
	}
	else if (g_balanceFlamer.integer && (weapon == SW_FLAMER)) {
		if ((axisPlayers.integer >= g_balanceFlamer.integer) && (alliedPlayers.integer >= g_balanceFlamer.integer))
			return qtrue;
		else
			return qfalse;
	}

	return qtrue;
}


/*
================
Default weapon

Accounts for "selected weapon" as well.
================
*/
///////////
// Deals only with soldier for weapon restrictions (To avoid breaking anything..).
void setDefWeap(gclient_t *client, int clips) {
	if (client->sess.sessionTeam == TEAM_RED)
	{
		COM_BitSet(client->ps.weapons, WP_MP40);
		client->sess.playerWeapon = SW_MP40;
		client->ps.ammoclip[BG_FindClipForWeapon(WP_MP40)] += 32;
		client->ps.ammo[BG_FindAmmoForWeapon(WP_MP40)] += (32 * clips);
		client->ps.weapon = WP_MP40;
	}
	else {
		COM_BitSet(client->ps.weapons, WP_THOMPSON);
		client->sess.playerWeapon = SW_THOMPSON;
		client->ps.ammoclip[BG_FindClipForWeapon(WP_THOMPSON)] += 30;
		client->ps.ammo[BG_FindAmmoForWeapon(WP_THOMPSON)] += (30 * clips);
		client->ps.weapon = WP_THOMPSON;
	}
}

// NOTE: Selected weapons only works for eng and med..sold and lt can pick their weapons already..
//       so setting it can potentialy overlap with client spawn scripts..
void setDefaultWeapon(gclient_t *client, qboolean isSold) {
	int ammo;

	// This deals with weapon restrictions.
	if (isSold)  {
		setDefWeap(client, g_soldierClips.integer);
		return;
	}

	// Sorts ammo
	ammo = (client->sess.selectedWeapon == WP_THOMPSON) ? 30 : 32;

	// Medic
	if (client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC) {
		if (g_customMGs.integer && client->sess.selectedWeapon != 0) {
			COM_BitSet(client->ps.weapons, client->sess.selectedWeapon);
			client->ps.ammoclip[BG_FindClipForWeapon(client->sess.selectedWeapon)] += ammo;
			client->ps.ammo[BG_FindAmmoForWeapon(client->sess.selectedWeapon)] += (ammo * g_medicClips.integer);
			client->ps.weapon = client->sess.selectedWeapon;
			return;
		}
		else {
			setDefWeap(client, g_medicClips.integer);
			return;
		}
	}

	// Engineer
	if (client->ps.stats[STAT_PLAYER_CLASS] == PC_ENGINEER) {
		if (g_customMGs.integer && client->sess.selectedWeapon != 0) {
			COM_BitSet(client->ps.weapons, client->sess.selectedWeapon);
			client->ps.ammoclip[BG_FindClipForWeapon(client->sess.selectedWeapon)] += ammo;
			client->ps.ammo[BG_FindAmmoForWeapon(client->sess.selectedWeapon)] += (ammo * g_engineerClips.integer);
			client->ps.weapon = client->sess.selectedWeapon;
			return;
		}
		else {
			setDefWeap(client, g_engineerClips.integer);
			return;
		}
	}
}

/*
=================
FlagBalance

Checks if teams are uneven and favours one with less to claim last take-retake
till teams even up again, then it unlocks so it can be (re)claimed again.
=================
*/
int FlagBalance(void) {
	if (!g_balanceFlagRetake.integer)
		return 0;

	// Avoid dealing with check if there's less than 2 players..
	if (axisPlayers.integer < 2 && alliedPlayers.integer < 2)
		return 0;

	// Even or difference by 1
	if (axisPlayers.integer == alliedPlayers.integer)
		return 0;
	// Axis (less)
	else if ((alliedPlayers.integer - axisPlayers.integer) > 1)
		return 1;
	// Allied (less)
	else if ((axisPlayers.integer - alliedPlayers.integer) > 1)
		return 2;
	// We don't know what happen...
	else
		return 0;
}

void CheckWeaponRestrictions(gclient_t *client)
{
	switch (client->pers.restrictedWeapon)
	{
	case WP_PANZERFAUST:
		(client->sess.sessionTeam == TEAM_RED) ? level.axisPF-- : level.alliedPF--;
		break;
	case WP_VENOM:
		(client->sess.sessionTeam == TEAM_RED) ? level.axisVenom-- : level.alliedVenom--;
		break;
	case WP_FLAMETHROWER:
		(client->sess.sessionTeam == TEAM_RED) ? level.axisFlamer-- : level.alliedFlamer--;
		break;
	case WP_MAUSER:
		(client->sess.sessionTeam == TEAM_RED) ? level.axisSniper-- : level.alliedSniper--;
		break;
	default:
		break;
	}

	client->pers.restrictedWeapon = WP_NONE;
}
