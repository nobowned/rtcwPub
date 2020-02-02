/*
 * name:		g_combat.c
 *
 * desc:		
 *
*/

#include "g_local.h"


/*
============
AddScore

Adds score to both the client and his team
============
*/
void AddScore( gentity_t *ent, int score ) {
	if ( !ent->client ) {
		return;
	}
	// no scoring during pre-match warmup
	if ( level.warmupTime ) {
		return;
	}

	// Ridah, no scoring during single player
	// DHM - Nerve :: fix typo
	if (g_gametype.integer == GT_SINGLE_PLAYER) {
		return;
	}
	// done.


	ent->client->ps.persistant[PERS_SCORE] += score;
	if (g_gametype.integer >= GT_TEAM) // JPW NERVE changed to >=
		level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;
	CalculateRanks();
}

/*
=================
TossClientItems

Toss the weapon and powerups for the killed player
=================
*/
void TossClientItems( gentity_t *self ) {
	gitem_t		*item;
	int			weapon;
	gentity_t	*drop = 0;

	qboolean dropPackInstead = g_dropAmmoPack.integer;

	// drop the weapon if not a gauntlet or machinegun
	weapon = self->s.weapon;

	// make a special check to see if they are changing to a new
	// weapon that isn't the mg or gauntlet.  Without this, a client
	// can pick up a weapon, be killed, and not drop the weapon because
	// their weapon change hasn't completed yet and they are still holding the MG.

	// (SA) always drop what you were switching to
	if(1) {
		if ( self->client->ps.weaponstate == WEAPON_DROPPING ) {
			weapon = self->client->pers.cmd.weapon;
		}
		if ( !( COM_BitCheck( self->client->ps.weapons, weapon ) ) ) {
			weapon = WP_NONE;
		}
	}

	// JPW NERVE don't drop these weapon types
	if ((weapon == WP_FLAMETHROWER) || (weapon == WP_GARAND) || (weapon == WP_MAUSER) || (weapon == WP_VENOM))
		weapon = WP_NONE;
	// jpw

	if (dropPackInstead) {
		weapon = PrimarySlotInUse(self->client->ps.weapons);
	}

	if ( weapon > WP_NONE && weapon < WP_MONSTER_ATTACK1&& self->client->ps.ammo[ BG_FindAmmoForWeapon(weapon)] ) {
		// find the item type for this weapon
		if (dropPackInstead)
			item = BG_FindItemForWeapon(WP_AMMO);
		else
			item = BG_FindItemForWeapon( weapon );
		// spawn the item
		
		// Rafael
		if (!(self->client->ps.persistant[PERS_HWEAPON_USE])) {
			drop = Drop_Item( self, item, 0, qfalse );
			// JPW NERVE -- fix ammo counts
			drop->count = self->client->ps.ammoclip[BG_FindClipForWeapon(weapon)];				
			if (g_dropClips.integer && !g_ffa.integer)
			{
				drop->count += self->client->ps.ammo[BG_FindAmmoForWeapon(weapon)];
			}
			// jpw
		}
	}

	// L0 - dropping stuff when going to limbo

	// drop medpacks
	if (g_dropHealth.integer) {
		if (self->client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC ||
			g_ffa.integer)
		{
			int i;
			item = BG_FindItem("Med Health");

			for (i = 0; i < g_dropHealth.integer; i++)
			{
				drop = Drop_Item(self, item, 0, qfalse);
				drop->think = MagicSink;
				drop->timestamp = level.time + 31200;
				drop->parent = NULL;
			}
		}
	}

	// drop ammopacks
	if (g_dropAmmo.integer)	{
		if (self->client->ps.stats[STAT_PLAYER_CLASS] == PC_LT)
		{
			int i;
			item = BG_FindItem("Ammo Pack");

			for (i = 0; i < g_dropAmmo.integer; i++)
			{
				drop = Drop_Item(self, item, 0, qfalse);
				drop->think = MagicSink;
				drop->timestamp = level.time + 31200;
				drop->parent = NULL;
			}
		}
	}

	// drop nades
	if (g_dropNades.integer) {
		if (self->client->ps.stats[STAT_PLAYER_CLASS] == PC_ENGINEER){
			int i;
			if (self->client->sess.sessionTeam == TEAM_RED)
				item = BG_FindItem("Grenades");
			else
				item = BG_FindItem("Pineapples");

			for (i = 0; i < g_dropNades.integer; i++){
				drop = Drop_Item(self, item, 0, qfalse);
				drop->think = MagicSink;
				drop->timestamp = level.time + 31200;
				drop->parent = NULL;
			}
		}
	}  // L0 - end
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller( gentity_t *self, gentity_t *inflictor, gentity_t *attacker ) {
	vec3_t		dir;
	vec3_t		angles;

	if ( attacker && attacker != self ) {
		VectorSubtract (attacker->s.pos.trBase, self->s.pos.trBase, dir);
	} else if ( inflictor && inflictor != self ) {
		VectorSubtract (inflictor->s.pos.trBase, self->s.pos.trBase, dir);
	} else {
		self->client->ps.stats[STAT_DEAD_YAW] = self->s.angles[YAW];
		return;
	}

	self->client->ps.stats[STAT_DEAD_YAW] = vectoyaw ( dir );

	angles[YAW] = vectoyaw ( dir );
	angles[PITCH] = 0; 
	angles[ROLL] = 0;
}


/*
==============
GibHead
==============
*/
void GibHead(gentity_t *self, int killer)
{
	G_AddEvent( self, EV_GIB_HEAD, killer );
}

/*
==================
GibEntity
==================
*/
void GibEntity( gentity_t *self, int killer, qboolean explode ) {
	gentity_t *other = &g_entities[killer];
	vec3_t dir;

	if (explode)
	{
		VectorClear(dir);
		if (other->inuse) {
			if (other->client) {
				VectorSubtract(self->r.currentOrigin, other->r.currentOrigin, dir);
				VectorNormalize(dir);
			}
			else if (!VectorCompare(other->s.pos.trDelta, vec3_origin)) {
				VectorNormalize2(other->s.pos.trDelta, dir);
			}
		}

		G_AddEvent(self, EV_GIB_PLAYER, DirToByte(dir));
	}

	self->takedamage = qfalse;
	self->s.eType = ET_INVISIBLE;
	self->r.contents = 0;
}

void PrintGib(gentity_t *attacker, gentity_t *targ)
{
	if (g_gibReports.integer)
	{
		if (!attacker || !targ || !targ->client)
		{
			return;
		}

		if (!attacker->client)
			AP(va("print \"%s ^7was gibbed\n\"", targ->client->pers.netname));
		else if (!OnSameTeam(attacker, targ) || (g_ffa.integer && attacker != targ))
			AP(va("print \"%s ^7was gibbed by %s\n\"", targ->client->pers.netname, attacker->client->pers.netname));
		else if (OnSameTeam(attacker, targ) && attacker != targ)
			AP(va("print \"%s ^3WAS GIBBED BY TEAMMATE ^7%s \n\"", targ->client->pers.netname, attacker->client->pers.netname));
		else if (attacker == targ)
			AP(va("print \"%s ^7gibbed %s\n\"", targ->client->pers.netname, targ->client->sess.gender ? "herself" : "himself"));

		if (!OnSameTeam(attacker, targ) && attacker->client)
		{
			attacker->client->pers.gibs++;
			stats_StoreRoundStat(attacker->client->pers.netname, attacker->client->pers.gibs, ROUND_GIBS);
		}
	}
}

/*
==================
body_die
==================
*/
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	if ( self->health > GIB_HEALTH ) {
		return;
	}

	PrintGib(attacker, self);
	GibEntity( self, 0, qtrue );
}


// these are just for logging, the client prints its own messages
char	*modNames[] = {
	"MOD_UNKNOWN",
	"MOD_SHOTGUN",
	"MOD_GAUNTLET",
	"MOD_MACHINEGUN",
	"MOD_GRENADE",
	"MOD_GRENADE_SPLASH",
	"MOD_ROCKET",
	"MOD_ROCKET_SPLASH",
	"MOD_RAILGUN",
	"MOD_LIGHTNING",
	"MOD_BFG",
	"MOD_BFG_SPLASH",
	"MOD_KNIFE",
	"MOD_KNIFE2",
	"MOD_KNIFE_STEALTH",
	"MOD_LUGER",
	"MOD_COLT",
	"MOD_MP40",
	"MOD_THOMPSON",
	"MOD_STEN",
	"MOD_MAUSER",
	"MOD_SNIPERRIFLE",
	"MOD_GARAND",
	"MOD_SNOOPERSCOPE",
	"MOD_SILENCER",//----(SA)	
	"MOD_AKIMBO",	//----(SA)	
	"MOD_BAR",	//----(SA)	
	"MOD_FG42",
	"MOD_FG42SCOPE",
	"MOD_PANZERFAUST",
	"MOD_ROCKET_LAUNCHER",
	"MOD_GRENADE_LAUNCHER",
	"MOD_VENOM",
	"MOD_VENOM_FULL",
	"MOD_FLAMETHROWER",
	"MOD_TESLA",
	"MOD_SPEARGUN",
	"MOD_SPEARGUN_CO2",
	"MOD_GRENADE_PINEAPPLE",
	"MOD_CROSS",
	"MOD_MORTAR",
	"MOD_MORTAR_SPLASH",
	"MOD_KICKED",
	"MOD_GRABBER",
	"MOD_WATER",
	"MOD_SLIME",
	"MOD_LAVA",
	"MOD_CRUSH",
	"MOD_TELEFRAG",
	"MOD_FALLING",
	"MOD_SUICIDE",
	"MOD_TARGET_LASER",
	"MOD_TRIGGER_HURT",
	"MOD_GRAPPLE",
	"MOD_EXPLOSIVE",
	"MOD_POISONGAS",
	"MOD_ZOMBIESPIT",
	"MOD_ZOMBIESPIT_SPLASH",
	"MOD_ZOMBIESPIRIT",
	"MOD_ZOMBIESPIRIT_SPLASH",
	"MOD_LOPER_LEAP",
	"MOD_LOPER_GROUND",
	"MOD_LOPER_HIT",
// JPW NERVE
	"MOD_LT_ARTILLERY",
	"MOD_LT_AIRSTRIKE",
	"MOD_ENGINEER",	// not sure if we'll use
	"MOD_MEDIC",		// these like this or not
// jpw

// L0 - New MODs
	"MOD_ADMKILL",
	"MOD_SELFKILL",		// Suicide (not gib!)
	"MOD_THROWKNIFE",	// Killed by knife throw
	"MOD_CHICKEN",		// Funny print when player self kills to avoid being killed
	"MOD_POISONDMED",	// Killed by poison
// End

	"MOD_BAT"
};

/*
==================
player_die
==================
*/
void limbo( gentity_t *ent, qboolean makeCorpse ); // JPW NERVE

void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	int			contents = 0;
	int			killer;
	int			i;
	char		*killerName, *obit;
	qboolean	nogib = qtrue;
	gitem_t		*item=NULL; // JPW NERVE for flag drop
	vec3_t		launchvel,launchspot; // JPW NERVE
	gentity_t	*flag; // JPW NERVE

	if ( self->client->ps.pm_type == PM_DEAD ) {
		return;
	}

	if ( level.intermissiontime ) {
		return;
	}

	// L0 - Shortcuts	
	if (attacker && attacker->client) {
		self->client->pers.lastkiller_client = attacker->s.clientNum;
		attacker->client->pers.lastkilled_client = self->s.clientNum;
	}

	self->client->ps.pm_type = PM_DEAD;

	G_AddEvent( self, EV_STOPSTREAMINGSOUND, 0);

	if ( attacker ) {
		killer = attacker->s.number;
		if ( attacker->client ) {
			killerName = attacker->client->pers.netname;
		} else {
			killerName = "<non-client>";
		}
	} else {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( killer < 0 || killer >= MAX_CLIENTS ) {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( meansOfDeath < 0 || meansOfDeath >= sizeof( modNames ) / sizeof( modNames[0] ) ) {
		obit = "<bad obituary>";
	} else {
		obit = modNames[ meansOfDeath ];
	}

	// L0
	// If person gets stabbed use custom sound from soundpack
	// it's broadcasted to victim and heard only if standing near victim...
	if (meansOfDeath == MOD_KNIFE_STEALTH && !OnSameTeam(self, attacker) && g_fastStabSound.integer) {
		int r = rand() % 2;
		char *snd;

		if (r == 0)
			snd = "stab.wav";
		else
			snd = "stab_alt.wav";

		APRS(self, va("rtcwpub/sound/game/events/%s", ((g_fastStabSound.integer == 1) ? "stab.wav" :
			((g_fastStabSound.integer == 2) ? "stab_alt.wav" : snd))));

		attacker->client->pers.fastStabs++;
		stats_StoreRoundStat(attacker->client->pers.netname, attacker->client->pers.fastStabs, ROUND_FASTSTABS);
	}

	// L0 - Don't bother in warmup etc..
	if (g_gamestate.integer == GS_PLAYING)
	{
		G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n",
			killer, self->s.number, meansOfDeath, killerName,
			self->client->pers.netname, obit);
	}

	// L0 - Death Sprees
	stats_DeathSpree(self);

	// L0 - Stats
	if (attacker && attacker->client && g_gamestate.integer == GS_PLAYING) {
		// Life kills & death spress
		if (!OnSameTeam(attacker, self) || (g_ffa.integer && attacker != self)) {
			attacker->client->pers.kills++;
			attacker->client->pers.lifeKills++;

			stats_StoreRoundStat(attacker->client->pers.netname, attacker->client->pers.kills, ROUND_KILLS);
			stats_StoreRoundStat(attacker->client->pers.netname, attacker->client->pers.lifeKills, ROUND_KILLPEAK);

			if (g_mapStats.integer == 1)
				stats_StoreMapStat(attacker, attacker->client->pers.kills, MAP_KILLER);
			else if (g_mapStats.integer == 2)
				stats_StoreMapStat(attacker, attacker->client->pers.kills, MAP_KILLING_SPREE);

			// Count teamkill
		}
		else {
			// Admin bot - teamKills
			if (attacker != self)
			{
				SB_maxTeamKill(attacker);
				stats_StoreRoundStat(attacker->client->pers.netname, attacker->client->pers.teamKills, ROUND_TEAMKILLS);
			}
		}
	}

	// L0 - Stats
	if (attacker && attacker->client)
	{
		if (!OnSameTeam(attacker, self) && (attacker->client->pers.spreeDeaths > attacker->client->pers.lifeDeathsPeak))
			attacker->client->pers.lifeDeathsPeak = attacker->client->pers.spreeDeaths;

		if (!OnSameTeam(attacker, self))
			attacker->client->pers.spreeDeaths = 0;
	}

	// create the obituary event for bots to receive and process.
	if (trap_Cvar_VariableIntegerValue("bot_enable")) {
		gentity_t *obituary_event_ent = G_TempEntity(self->r.currentOrigin, EV_OBITUARY);
		obituary_event_ent->s.eventParm = meansOfDeath;
		obituary_event_ent->s.otherEntityNum = self->s.number;
		obituary_event_ent->s.otherEntityNum2 = killer;
		obituary_event_ent->r.svFlags |= SVF_BROADCAST_BOTS;
	}

	print_mod(attacker, self, meansOfDeath);

	self->enemy = attacker;

	self->client->ps.persistant[PERS_KILLED]++;

// L0 
	self->client->pers.deaths++;
	self->client->pers.spreeDeaths++;
	stats_StoreRoundStat(self->client->pers.netname, self->client->pers.deaths, ROUND_DEATHS);
	stats_StoreRoundStat(self->client->pers.netname, self->client->pers.spreeDeaths, ROUND_DEATHPEAK);

	if (g_mapStats.integer == 3)
		stats_StoreMapStat(self, self->client->pers.deaths, MAP_VICTIM);
	else if (g_mapStats.integer == 4)
		stats_StoreMapStat(self, self->client->pers.deaths, MAP_DEATH_SPREE);
// End

// JPW NERVE -- if player is holding ticking grenade, drop it
	if (g_gametype.integer != GT_SINGLE_PLAYER) {
		if ((self->client->ps.grenadeTimeLeft) && (self->s.weapon != WP_DYNAMITE)) {
			launchvel[0] = crandom();
			launchvel[1] = crandom();
			launchvel[2] = random();
			VectorScale(launchvel, 160, launchvel);
			VectorCopy(self->r.currentOrigin, launchspot);
			launchspot[2] += 40;
			fire_grenade(self, launchspot, launchvel, self->s.weapon)->damage = 0;
			self->client->ps.ammoclip[BG_FindClipForWeapon(self->s.weapon)] -= ammoTable[self->s.weapon].uses;
		}
	}
// jpw

	if (attacker && attacker->client) {
		if ( attacker == self || (OnSameTeam (self, attacker ) && !g_ffa.integer) ) {

			// DHM - Nerve :: Complaint lodging
			if ( attacker != self && level.warmupTime <= 0 ) {
				if ( attacker->client->pers.localClient ) {
					trap_SendServerCommand( self-g_entities, "complaint -4" );
				}
				else {
					trap_SendServerCommand( self-g_entities, va( "complaint %i", attacker->s.number ) );
					self->client->pers.complaintClient = attacker->s.clientNum;
					self->client->pers.complaintEndTime = level.time + 20500;
				}
			}
			// dhm

			// JPW NERVE
			if (g_gametype.integer >= GT_WOLF) // high penalty to offset medic heal
				AddScore(attacker, WOLF_FRIENDLY_PENALTY);
			else
				AddScore(attacker, -1);

			// L0 - Life stats
			// FIXME: Port this in structure..
			if (g_showLifeStats.integer && !(self->r.svFlags & SVF_BOT)) {
				float acc = 0.00f;

				acc = (self->client->pers.lifeAcc_shots == 0) ?
					0.00 : ((float)self->client->pers.lifeAcc_hits / (float)self->client->pers.lifeAcc_shots) * 100.00f;

				// Class based..				
				if (self->client->ps.stats[PC_MEDIC])
					CPx(self - g_entities, va("chat \"^7Last life: ^3Kills: ^7%d ^3Hs: ^7%d ^3Rev: ^7%d ^3Acc: ^7%2.2f\n\"",
					self->client->pers.lifeKills, self->client->pers.lifeHeadshots, self->client->pers.lifeRevives, acc));
				else if (self->client->ps.stats[PC_LT])
					CPx(self - g_entities, va("chat \"^7Last life: ^3Kills: ^7%d ^3Hs: ^7%d ^3Acc: ^7%2.2f\n\"",
					self->client->pers.lifeKills, self->client->pers.lifeHeadshots, acc));
				else if (self->client->ps.stats[PC_ENGINEER])
					CPx(self - g_entities, va("chat \"^7Last life: ^3Kills: ^7%d ^3Hs: ^7%d ^3Acc: ^7%2.2f\n\"",
					self->client->pers.lifeKills, self->client->pers.lifeHeadshots, acc));
				else
					CPx(self - g_entities, va("chat \"^7Last life: ^3Kills: ^7%d ^3Hs: ^7%d ^3Acc: ^7%2.2f\n\"",
					self->client->pers.lifeKills, self->client->pers.lifeHeadshots, acc));
			} // End

		} else {
			// JPW NERVE -- mostly added as conveneience so we can tweak from the #defines all in one place
			if (g_gametype.integer >= GT_WOLF)
			{
				AddScore(attacker, WOLF_FRAG_BONUS);

				// L0 - Stats
				stats_DoubleKill(attacker, meansOfDeath);		// Double kills
				stats_KillingSprees(attacker);				// Overall killing sprees
			}
			else
			{
				// jpw
				AddScore(attacker, 1);

				// L0 - Stats
				stats_DoubleKill(attacker, meansOfDeath);		// Double kills
				stats_KillingSprees(attacker);				// Overall killing sprees
			}

			attacker->client->lastKillTime = level.time;

			// L0 - Last blood
			if (g_showLastBlood.integer)
			{
				Q_strncpyz(level.lastKiller, attacker->client->pers.netname, sizeof(level.lastKiller));
				Q_strncpyz(level.lastVictim, self->client->pers.netname, sizeof(level.lastVictim));
			}

			// L0 - Life stats
			if (g_showLifeStats.integer && !(self->r.svFlags & SVF_BOT)) {
				float acc = 0.00f;

				acc = (self->client->pers.lifeAcc_shots == 0) ?
					0.00 : ((float)self->client->pers.lifeAcc_hits / (float)self->client->pers.lifeAcc_shots) * 100.00f;

				// Class based..
				if (self->client->ps.stats[PC_MEDIC])
					CPx(self - g_entities, va("chat \"^7Last Life: ^3Kills: ^7%d ^3Hs: ^7%d ^3Rev: ^7%d ^3Acc: ^7%2.2f ^3Killer: ^7%s ^3[^7%ihp^3]^7\n\"",
					self->client->pers.lifeKills, self->client->pers.lifeHeadshots, self->client->pers.lifeRevives, acc, attacker->client->pers.netname, attacker->health));
				else if (self->client->ps.stats[PC_LT])
					CPx(self - g_entities, va("chat \"^7Last Life: ^3Kills: ^7%d ^3Hs: ^7%d ^3Acc: ^7%2.2f ^3Killer: ^7%s ^3[^7%ihp^3]^7\n\"",
					self->client->pers.lifeKills, self->client->pers.lifeHeadshots, acc, attacker->client->pers.netname, attacker->health));
				else if (self->client->ps.stats[PC_ENGINEER])
					CPx(self - g_entities, va("chat \"^7Last Life: ^3Kills: ^7%d ^3Hs: ^7%d ^3Acc: ^7%2.2f ^3Killer: ^7%s ^3[^7%ihp^3]^7\n\"",
					self->client->pers.lifeKills, self->client->pers.lifeHeadshots, acc, attacker->client->pers.netname, attacker->health));
				else
					CPx(self - g_entities, va("chat \"^7Last Life: ^3Kills: ^7%d ^3Hs: ^7%d ^3Acc: ^7%2.2f ^3Killer: ^7%s ^3[^7%ihp^3]^7\n\"",
					self->client->pers.lifeKills, self->client->pers.lifeHeadshots, acc, attacker->client->pers.netname, attacker->health));
			} // End

			if (g_ffa.integer)
			{
				int killsRemain = g_fraglimit.integer - attacker->client->pers.kills;
				if (killsRemain < 5 && killsRemain > 0)
				{
					AP(va("chat \"console: %s^7 has ^2%i^7 %s left.\n\"", attacker->client->pers.netname, killsRemain, killsRemain > 1 ? "frags" : "frag"));
				}
			}
		}
	} else {
		AddScore( self, -1 );
	}

	// Add team bonuses
	Team_FragBonuses(self, inflictor, attacker);

	// L0 - AB low score
	// TODO: Add mod_switchteam check..
	SB_minLowScore(self);

	// if client is in a nodrop area, don't drop anything
// JPW NERVE new drop behavior
	if (g_gametype.integer == GT_SINGLE_PLAYER) {	// only drop here in single player; in multiplayer, drop @ limbo
		contents = trap_PointContents( self->r.currentOrigin, -1 );
		if ( !( contents & CONTENTS_NODROP ) ) {
			TossClientItems( self );
		}
	}

	// drop flag regardless
	if (g_gametype.integer != GT_SINGLE_PLAYER) {
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
			launchvel[0] = crandom()*20;
			launchvel[1] = crandom()*20;
			launchvel[2] = 10+random()*10;

			flag = LaunchItem(item,self->r.currentOrigin,launchvel,self->s.number);
			flag->s.modelindex2 = self->s.otherEntityNum2;// JPW NERVE FIXME set player->otherentitynum2 with old modelindex2 from flag and restore here
			flag->message = self->message;	// DHM - Nerve :: also restore item name
			// Clear out player's temp copies
			self->s.otherEntityNum2 = 0;
			self->message = NULL;
		}

		// send a fancy "MEDIC!" scream.  Sissies, ain' they?
		if (self->client != NULL) {
			if (self->health > GIB_HEALTH && meansOfDeath != MOD_SUICIDE ) {

				if (self->client->sess.sessionTeam == TEAM_RED) {
					if (random() > 0.5)
						G_AddEvent( self, EV_GENERAL_SOUND, G_SoundIndex( "sound/multiplayer/axis/g-medic2.wav" ));
					else
						G_AddEvent( self, EV_GENERAL_SOUND, G_SoundIndex( "sound/multiplayer/axis/g-medic3.wav" ));
				}
				else {
					if (random() > 0.5)
						G_AddEvent( self, EV_GENERAL_SOUND, G_SoundIndex( "sound/multiplayer/allies/a-medic3.wav" ));
					else
						G_AddEvent( self, EV_GENERAL_SOUND, G_SoundIndex( "sound/multiplayer/allies/a-medic2.wav" ));
				}
			}
		}
	}
// jpw

	Cmd_Score_f( self );		// show scores
	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		gclient_t	*client;

		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			continue;
		}
		if ( client->sess.spectatorClient == self->s.number ) {
			Cmd_Score_f( g_entities + i );
		}
	}

	self->takedamage = qtrue;	// can still be gibbed
	self->r.contents = CONTENTS_CORPSE;

	self->s.powerups = 0;
// JPW NERVE -- only corpse in SP; in MP, need CONTENTS_BODY so medic can operate
	if (g_gametype.integer == GT_SINGLE_PLAYER) {
		self->s.weapon = WP_NONE;
	}
	else {
		self->client->limboDropWeapon = self->s.weapon; // store this so it can be dropped in limbo
	}
// jpw
	vec3_t new_angles;
	if (g_useSpawnAnglesAfterRevive.integer) {
		VectorCopy(self->client->pers.spawnAngles, new_angles);
	} else {
		VectorCopy(self->client->ps.viewangles, new_angles);
	}
	// Nobo, only persist YAW value. Otherwise corpse can look strange (as well as revive anim).
	new_angles[PITCH] = 0;
	new_angles[ROLL] = 0;
	SetClientViewAngle(self, new_angles);

	LookAtKiller (self, inflictor, attacker);

	self->s.loopSound = 0;

	trap_UnlinkEntity( self );
	self->r.maxs[2] = 0;
	self->client->ps.maxs[2] = 0;
	trap_LinkEntity( self );

	// don't allow respawn until the death anim is done
	// g_forcerespawn may force spawning at some later time
	self->client->respawnTime = level.time + 800;

	// remove powerups
	memset( self->client->ps.powerups, 0, sizeof(self->client->ps.powerups) );

	// gib
	if (self->health < FORCE_LIMBO_HEALTH)
	{
		GibEntity(self, killer, self->health <= GIB_HEALTH);
		PrintGib(attacker, self);
		self->die = 0;
		nogib = qfalse;
	}

	if(nogib){
		// normal death
		// for the no-blood option, we need to prevent the health
		// from going to gib level
		if ( self->health <= GIB_HEALTH ) {
			self->health = GIB_HEALTH+1;
		}

// JPW NERVE for medic
		self->client->medicHealAmt = 0;
// jpw

		// DHM - Play death animation, and set pm_time to delay 'fallen' animation
		self->client->ps.pm_time = BG_AnimScriptEvent( &self->client->ps, ANIM_ET_DEATH, qfalse, qtrue );

		G_AddEvent( self, EV_DEATH1 + 1, killer );

		// the body can still be gibbed
		self->die = body_die;

#if 0
		// would set the bit now but it causes some important information to not get processed by the client.
		// so instead we send it later on..
		self->client->needs_delayed_teleport_bit_start_time = level.time;
		self->client->teleport_bit_delayed_frame_count = 2;
#endif
	}

	trap_LinkEntity (self);

	// L0 - show First Blood
	stats_FirstBlood(self, attacker);

// L0 - reset chicken test so it doesn't spam when not need it
	self->client->lasthurt_time = 0;
	self->client->lasthurt_client = ENTITYNUM_NONE;
	self->client->lasthurt_mod = 0;
// End
}


/*
================
CheckArmor
================
*/
int CheckArmor (gentity_t *ent, int damage, int dflags)
{
	gclient_t	*client;
	int			save;
	int			count;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	// armor
	count = client->ps.stats[STAT_ARMOR];
	save = ceil( damage * ARMOR_PROTECTION );
	if (save >= count)
		save = count;

	if (!save)
		return 0;

	client->ps.stats[STAT_ARMOR] -= save;

	return save;
}

/*
==============
G_ArmorDamage
	brokeparts is how many should be broken off now
	curbroke is how many are broken
	the difference is how many to pop off this time
==============
*/
void G_ArmorDamage(gentity_t *targ) {
	int brokeparts, curbroke;
	int numParts;
	int dmgbits = 16;	// 32/2;
	int i;

	if(!targ->client)
		return;

	if(targ->s.aiChar == AICHAR_PROTOSOLDIER ) {
		numParts = 9;
	} else if(targ->s.aiChar == AICHAR_SUPERSOLDIER ) {
		numParts = 14;
	} else if(targ->s.aiChar == AICHAR_HEINRICH ) {
		numParts = 20;
	} else {
		return;
	}

	if(numParts > dmgbits)
		numParts = dmgbits;	// lock this down so it doesn't overwrite any bits that it shouldn't.  TODO: fix this


	// determined here (on server) by location of hit and existing armor, you're updating here so 
	// the client knows which pieces are still in place, and by difference with previous state, which
	// pieces to play an effect where the part is blown off.
	// Need to do it here so we have info on where the hit registered (head, torso, legs or if we go with more detail; arm, leg, chest, codpiece, etc)

	// ... Ick, just discovered that the refined hit detection ("hit nearest to which tag") is clientside...

	// For now, I'll randomly pick a part that hasn't been cleared.  This might end up looking okay, and we won't need the refined hits.
	//	however, we still have control on the server-side of which parts come off, regardless of what shceme is used.

	brokeparts = (int)( (1 - ((float)(targ->health) / (float)(targ->client->ps.stats[STAT_MAX_HEALTH])) ) * numParts);

	if(brokeparts && ( (targ->s.dmgFlags & ((1<<numParts)-1)) != (1<<numParts)-1) ) {	// there are still parts left to clear

		// how many are removed already?
		curbroke = 0;
		for(i=0;i<numParts;i++) {
			if(targ->s.dmgFlags & (1<<i))
				curbroke++;
		}

		// need to remove more
		if(brokeparts-curbroke >= 1 && curbroke < numParts) {
			for(i=0;i<(brokeparts-curbroke);i++) {
				int remove = rand()%(numParts);

				if(!( (targ->s.dmgFlags & ((1<<numParts)-1)) != (1<<numParts)-1))	// no parts are available any more
					break;

				// FIXME: lose the 'while' loop.  Still should be safe though, since the check above verifies that it will eventually find a valid part
				while(targ->s.dmgFlags & (1<<remove)){
					remove = rand()%(numParts);
				}

				targ->s.dmgFlags |= (1<<remove);	// turn off 'undamaged' part
				if((int)(random()+0.5))							// choose one of two possible replacements
					targ->s.dmgFlags |= (1<<(numParts + remove));
			}
		}
	}
}

/*
==============
L0

Hitsounds
Note that it requires pack for it.
==============
*/
void Hitsounds(gentity_t *targ, gentity_t *attacker, int mod, qboolean head) {
	// Nobo :: people keep complaining about these.. =/
	if (mod == MOD_POISONDMED || mod == MOD_ARTILLERY ||
		mod == MOD_GRENADE_SPLASH || mod == MOD_DYNAMITE_SPLASH ||
		mod == MOD_DYNAMITE || mod == MOD_ROCKET ||
		mod == MOD_ROCKET_SPLASH || mod == MOD_KNIFE ||
		mod == MOD_THROWKNIFE || mod == MOD_GRENADE ||
		mod == MOD_LAVA || mod == MOD_AIRSTRIKE ||
		mod == MOD_ADMKILL)
		return;
	if (!targ || !attacker || !targ->client || !attacker->client)
		return;

	if (g_hitsounds.integer && !(attacker->client->sess.colorFlags & CF_HITSOUNDS_OFF)) {
		gentity_t	*te;
		qboolean 	onSameTeam = OnSameTeam(targ, attacker);

		// if player is hurting him self don't give any sounds
		if (targ->client == attacker->client) {
			return;  // this happens at flaming your self... just return silence...			
		}

		// if team mate
		if (onSameTeam && g_gamestate.integer != GS_WARMUP_COUNTDOWN && !g_ffa.integer) {
			te = G_TempEntity(attacker->s.pos.trBase, EV_GLOBAL_CLIENT_SOUND);
			te->s.eventParm = G_SoundIndex("sound/player/hitteam.wav");
			te->s.teamNum = attacker->s.clientNum;
		}

		// If enemy
		else if (attacker->s.number != ENTITYNUM_NONE &&
			attacker->s.number != ENTITYNUM_WORLD &&
			attacker != targ)
		{
			te = G_TempEntity(attacker->s.pos.trBase, EV_GLOBAL_CLIENT_SOUND);
			if (head)
				te->s.eventParm = G_SoundIndex("sound/player/hithead.wav");
			else
				te->s.eventParm = G_SoundIndex("sound/player/hit.wav");
			te->s.teamNum = attacker->s.clientNum;
		}
	}
}

/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/

void G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int damage, int dflags, int mod ) {
	gclient_t	*client;
	int			take;
	int			save;
	int			asave;
	int			knockback;
	qboolean	isHeadShot = qfalse;

	if (!targ->takedamage) {
		return;
	}

	// the intermission has allready been qualified for, so don't
	// allow any extra scoring
	if ( level.intermissionQueued || g_gamestate.integer != GS_PLAYING ) {	
		// L0 - warmup damage
		if (!g_warmupDamage.integer)
			return;
	}

	if ( !inflictor ) {
		inflictor = &g_entities[ENTITYNUM_WORLD];
	}
	if ( !attacker ) {
		attacker = &g_entities[ENTITYNUM_WORLD];
	}

// JPW NERVE
	if ((targ->waterlevel >= 3) && (mod == MOD_FLAMETHROWER))
		return;
// jpw

	// shootable doors / buttons don't actually have any health
	if ( targ->s.eType == ET_MOVER && !(targ->aiName) && !(targ->isProp) && !targ->scriptName) {
		if ( targ->use && targ->moverState == MOVER_POS1 ) {
			targ->use( targ, inflictor, attacker );
		}
		return;
	}

	if ( targ->s.eType == ET_MOVER && targ->aiName && !(targ->isProp) && !targ->scriptName)
	{
		switch (mod) {
		case MOD_GRENADE:
		case MOD_GRENADE_SPLASH:
		case MOD_ROCKET:
		case MOD_ROCKET_SPLASH:
			break;
		default:
			return;	// no damage from other weapons
		}
	}
	else if ( targ->s.eType == ET_EXPLOSIVE ) 
	{
		// 32 Explosive
		// 64 Dynamite only
		if ((targ->spawnflags & 32) || (targ->spawnflags & 64))
		{
			switch (mod) {
			case MOD_GRENADE:
			case MOD_GRENADE_SPLASH:
			case MOD_ROCKET:
			case MOD_ROCKET_SPLASH:
			case MOD_AIRSTRIKE:
			case MOD_ARTILLERY:
			case MOD_GRENADE_PINEAPPLE:
			case MOD_MORTAR:
			case MOD_MORTAR_SPLASH:
			case MOD_EXPLOSIVE:
				if (targ->spawnflags & 64)
					return;
				
				break;

			case MOD_DYNAMITE:
			case MOD_DYNAMITE_SPLASH:
				break;
			
			default: 
				return;
			}
		}
	}

	client = targ->client;

	if ( client ) {
		if ( client->noclip ) {
			return;
		}
	}

	if ( !dir ) {
		dflags |= DAMAGE_NO_KNOCKBACK;
	} else {
		VectorNormalize(dir);
	}

	knockback = damage;
	// L0 - Now by default knockback is set to 100 (was 1000) so if it's not touched
	// multiply nade and AS to 1000 so it acts and feels like default
	if (dflags & DAMAGE_RADIUS) {
		if (g_knockback.integer == 100)
			knockback *= 10;
	} // End
	if (targ->flags & FL_NO_KNOCKBACK) {
		knockback = 0;
	}
	if (dflags & DAMAGE_NO_KNOCKBACK) {
		knockback = 0;
	}

	// figure momentum add, even if the damage won't be taken
	if ( knockback && targ->client && (g_friendlyFire.integer || !OnSameTeam(targ, attacker)) ) {
		vec3_t	kvel;
		float	mass;

		mass = 200;

		if (mod == MOD_LIGHTNING && !((level.time+targ->s.number*50)%400)) {
			knockback = 60;
			dir[2] = 0.3;
		}

		VectorScale (dir, g_knockback.value * (float)knockback / mass, kvel);
		VectorAdd (targ->client->ps.velocity, kvel, targ->client->ps.velocity);

		if (targ == attacker && !(	mod != MOD_ROCKET &&
									mod != MOD_ROCKET_SPLASH &&
									mod != MOD_GRENADE &&
									mod != MOD_GRENADE_SPLASH &&
									mod != MOD_DYNAMITE))
		{
			targ->client->ps.velocity[2] *= 0.25;
		}

		// set the timer so that the other client can't cancel
		// out the movement immediately
		if ( !targ->client->ps.pm_time ) {
			int		t;

			t = knockback * 2;
			if ( t < 50 ) {
				t = 50;
			}
			if ( t > 200 ) {
				t = 200;
			}
			targ->client->ps.pm_time = t;
			targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}

	// check for completely getting out of the damage
	if ( !(dflags & DAMAGE_NO_PROTECTION) ) {

		// if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target
		// if the attacker was on the same team
		if ( targ != attacker && OnSameTeam (targ, attacker)  ) {
			if ( !g_friendlyFire.integer ) {
				return;
			}
		}

		// check for godmode
		if ( targ->flags & FL_GODMODE ) {
			return;
		}

		// RF, warzombie defense position is basically godmode for the time being
		if ( targ->flags & FL_DEFENSE_GUARD ) {
			return;
		}

		// check for invulnerability // (SA) moved from below so DAMAGE_NO_PROTECTION will still work
		if ( client && client->ps.powerups[PW_INVULNERABLE] ) {	//----(SA)	added
			return;
		}

	}

	// battlesuit protects from all radius damage (but takes knockback)
	// and protects 50% against all damage
	if ( client && client->ps.powerups[PW_BATTLESUIT] ) {
		G_AddEvent( targ, EV_POWERUP_BATTLESUIT, 0 );
		if ( dflags & DAMAGE_RADIUS ) {
			return;
		}
		damage *= 0.5;
	}

	// add to the attacker's hit counter
	if ( attacker->client && targ != attacker && targ->health > 0 ) {
		if ( OnSameTeam( targ, attacker ) ) {
			attacker->client->ps.persistant[PERS_HITS] -= damage;
			// L0 - Admin bot, Team bleed
			SB_maxTeamBleed(attacker);
		} else {
			attacker->client->ps.persistant[PERS_HITS] += damage;
		}
	}

	if ( damage < 1 ) {
		damage = 1;
	}
	take = damage;
	save = 0;

	// save some from armor
	asave = CheckArmor (targ, take, dflags);
	take -= asave;

	if (targ->headshot && targ->client) {
		if (take*2 < 50) // head shots, all weapons, do minimum 50 points damage
			take = 50;
		else
			take *= 2; // sniper rifles can do full-kill (and knock into limbo)

		if (g_debugBullets.integer > 0)
			AP(va("print \"%s ^7headshot for %i dmg\n\"", targ->client->pers.netname, take));

		if(!(targ->client->ps.eFlags & EF_HEADSHOT))	// only toss hat on first headshot
			G_AddEvent( targ, EV_LOSE_HAT, DirToByte(dir) );

		targ->client->ps.eFlags |= EF_HEADSHOT;

		isHeadShot = qtrue;

		// L0 - check for First Headshot
		stats_FirstHeadshot(attacker, targ);

		// L0 - Stats
		if ((attacker && attacker->client) && (!OnSameTeam(attacker, targ) || g_ffa.integer))
		{
			attacker->client->pers.headshots++;
			attacker->client->pers.lifeHeadshots++;

			stats_StoreRoundStat(attacker->client->pers.netname, attacker->client->pers.headshots, ROUND_HEADSHOTS);

			if (g_mapStats.integer == 6)
				stats_StoreMapStat(attacker, attacker->client->pers.deaths, MAP_HEADSHOTS);
		}
	}

	if ( g_debugDamage.integer ) {
		G_Printf( "client:%i health:%i damage:%i armor:%i\n", targ->s.number,
			targ->health, take, asave );
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( client ) {
		if ( attacker ) {
			client->ps.persistant[PERS_ATTACKER] = attacker->s.number;
		} else {
			client->ps.persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;
		}
		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;

		if ( dir ) {
			VectorCopy ( dir, client->damage_from );
			client->damage_fromWorld = qfalse;
		} else {
			VectorCopy ( targ->r.currentOrigin, client->damage_from );
			client->damage_fromWorld = qtrue;
		}
	}

	// See if it's the player hurting the emeny flag carrier
	Team_CheckHurtCarrier(targ, attacker);

	if (targ->client) {
		// set the last client who damaged the target
		targ->client->lasthurt_client = attacker->s.number;
		targ->client->lasthurt_mod = mod;
		targ->client->lasthurt_time = level.time;	// L0 - chicken test
	}

	// L0 - Stats..
	if (attacker && targ && attacker->client && targ->client && targ != attacker && targ->client->ps.stats[STAT_HEALTH] > 0){
		int healthTaken = (take > targ->client->ps.stats[STAT_HEALTH]) ? targ->client->ps.stats[STAT_HEALTH] : take;

		if (OnSameTeam(attacker, targ) && !g_ffa.integer) {
			attacker->client->pers.dmgTeam += healthTaken;
			stats_StoreRoundStat(attacker->client->pers.netname, attacker->client->pers.dmgTeam, ROUND_TEAMBLEED);
		}
		else {
			attacker->client->pers.dmgGiven += healthTaken;
			targ->client->pers.dmgReceived += healthTaken;
		}
	}

	Hitsounds(targ, attacker, mod, isHeadShot);

	// do the damage
	if (take) {
		targ->health = targ->health - take;

		// Nobo - automg42; make sure base and gun have same health at all times
		if (g_automg42.integer)
		{
			if (targ->chain)
			{
				G_Automg42UpdateHealth(targ->chain, targ->health);
			}
			else if (targ->mg42BaseEnt > 0)
			{
				G_Automg42UpdateHealth(targ, targ->health);
			}
		}
		// Nobo

		// Ridah, can't gib with bullet weapons (except VENOM)
		if ( mod != MOD_VENOM && attacker == inflictor && targ->client && targ->health <= GIB_HEALTH) {
			if (targ->aiCharacter != AICHAR_ZOMBIE)	// zombie needs to be able to gib so we can kill him (although he doesn't actually GIB, he just dies)
				targ->health= GIB_HEALTH+1;
		}

// JPW NERVE overcome previous chunk of code for making grenades work again
		if ((g_gametype.integer != GT_SINGLE_PLAYER) && (take > 190) && targ->client) // 190 is greater than 2x mauser headshot, so headshots don't gib
				targ->health = GIB_HEALTH - 1;
// jpw
		//G_Printf("health at: %d\n", targ->health);
		if ( targ->health <= 0 ) {
			if ( client ) {
				targ->flags |= FL_NO_KNOCKBACK;
// JPW NERVE -- repeated shooting sends to limbo
				if (g_gametype.integer >= GT_WOLF)  {
					if (targ->health < FORCE_LIMBO_HEALTH && targ->health >= (GIB_HEALTH - 1) && !(targ->client->ps.pm_flags & PMF_LIMBO))
					{
						if (targ->health > GIB_HEALTH) {
							if (targ->die == body_die) {
								PrintGib(attacker, targ);
							}
							targ->client->limboDropWeapon = targ->s.weapon;
							limbo(targ, qtrue);
						}
					}
				}
			}

			if (targ->health < -999)
				targ->health = -999;

			targ->enemy = attacker;
			if (targ->die)	// Ridah, mg42 doesn't have die func (FIXME)
				targ->die (targ, inflictor, attacker, take, mod);

			// if we freed ourselves in death function
			if (!targ->inuse)
				return;

			// RF, entity scripting
			if (targ->s.number >= MAX_CLIENTS && targ->health <= 0)	// might have revived itself in death function
				G_Script_ScriptEvent( targ, "death", "" );

		} else if ( targ->pain ) {
			if (dir) {	// Ridah, had to add this to fix NULL dir crash
				VectorCopy (dir, targ->rotate);
				VectorCopy (point, targ->pos3); // this will pass loc of hit
			} else {
				VectorClear( targ->rotate );
				VectorClear( targ->pos3 );
			}

			targ->pain (targ, attacker, take, point);

			// RF, entity scripting
			if (targ->s.number >= MAX_CLIENTS)
				G_Script_ScriptEvent( targ, "pain", va("%d %d", targ->health, targ->health+take) );
		}

		//G_ArmorDamage(targ);	//----(SA)	moved out to separate routine

		// Ridah, this needs to be done last, incase the health is altered in one of the event calls
		if ( targ->client ) {
			targ->client->ps.stats[STAT_HEALTH] = targ->health;
		}
	}

}


/*
============
CanDamage

Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage (gentity_t *targ, vec3_t origin) {
	vec3_t	dest;
	trace_t	tr;
	vec3_t	midpoint;

	// use the midpoint of the bounds instead of the origin, because
	// bmodels may have their origin is 0,0,0
	VectorAdd (targ->r.absmin, targ->r.absmax, midpoint);
	VectorScale (midpoint, 0.5, midpoint);
	VectorCopy (midpoint, dest);

	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	// on some surfaces (boxes, mp_base axis spawn floor) grenades startsolid and
	// damage/kill/break things they shouldn't.
	if (tr.startsolid)
	{
		origin[2] += 1;
		trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	}

	if (tr.fraction == 1.0)
		return qtrue;

	if(&g_entities[tr.entityNum] == targ)
		return qtrue;

	// this should probably check in the plane of projection, 
	// rather than in world coordinate, and also include Z
	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;


	return qfalse;
}


/*
============
G_RadiusDamage
============
*/
qboolean G_RadiusDamage ( vec3_t origin, gentity_t *attacker, float damage, float radius,
					 gentity_t *ignore, int mod) {
	float		points, dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;
	qboolean	hitClient = qfalse;
// JPW NERVE
	float		boxradius;
	vec3_t		dest; 
	trace_t		tr;
	vec3_t		midpoint;
// jpw


	if ( radius < 1 ) {
		radius = 1;
	}

	boxradius = 1.41421356 * radius; // radius * sqrt(2) for bounding box enlargement -- 
	// bounding box was checking against radius / sqrt(2) if collision is along box plane
	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - boxradius; // JPW NERVE
		maxs[i] = origin[i] + boxradius; // JPW NERVE
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

/* JPW NERVE -- we can put this back if we need to, but it kinna sucks for human-sized bboxes
		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->r.absmin[i] ) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if ( origin[i] > ent->r.absmax[i] ) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}
*/
// JPW NERVE
		if (!ent->r.bmodel)
			VectorSubtract(ent->r.currentOrigin,origin,v); // JPW NERVE simpler centroid check that doesn't have box alignment weirdness
		else {
			for ( i = 0 ; i < 3 ; i++ ) {
				if ( origin[i] < ent->r.absmin[i] ) {
					v[i] = ent->r.absmin[i] - origin[i];
				} else if ( origin[i] > ent->r.absmax[i] ) {
					v[i] = origin[i] - ent->r.absmax[i];
				} else {
					v[i] = 0;
				}
			}
		}
// jpw
		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

		points = damage * ( 1.0 - dist / radius );

// JPW NERVE -- different radiusdmg behavior for MP -- big explosions should do less damage (over less distance) through failed traces
		if( CanDamage (ent, origin) ) {
			if( LogAccuracyHit( ent, attacker ) ) {
				hitClient = qtrue;
			}
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			G_Damage (ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod);
		}
// JPW NERVE --  MP weapons should do 1/8 damage through walls over 1/8th distance
		else {
			if (g_gametype.integer != GT_SINGLE_PLAYER) { 
				VectorAdd (ent->r.absmin, ent->r.absmax, midpoint);
				VectorScale (midpoint, 0.5, midpoint);
				VectorCopy (midpoint, dest);
			
				trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
				if (tr.fraction < 1.0) {
					VectorSubtract(dest,origin,dest);
					dist = VectorLength(dest);
					if (dist < radius*0.2f) { // closer than 1/4 dist
						if( LogAccuracyHit( ent, attacker ) ) {
							hitClient = qtrue;
						}
						VectorSubtract (ent->r.currentOrigin, origin, dir);
						dir[2] += 24;
						G_Damage (ent, NULL, attacker, dir, origin, (int)(points*0.1f), DAMAGE_RADIUS, mod);
					}
				}
			}
		}
// jpw
	}
	return hitClient;
}
