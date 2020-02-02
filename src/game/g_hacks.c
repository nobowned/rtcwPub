/*
===========================================================================
L0 / rtcwPub :: g_hacks.c

	Functions that should be client-server sided but due server only mod
	they're more or less hacked into a game..

	NOTE: If you're ever planning to create a client-server mod,
		  then patch stuff accordingly and remove this file..

Created: 24. Mar / 2014
===========================================================================
*/
#include "g_local.h"

/*
================
print_mod
================
*/
void print_mod(gentity_t *attacker, gentity_t *self, int meansOfDeath){
	gentity_t	*te;
	gentity_t *other;
	char		*message;
	char		*message2;
	char index[MAX_OSPATH];
	int			i;

	message = 0;
	message2 = 0;

	if (attacker != self){
		if (OnSameTeam(attacker, self) && !g_ffa.integer)
			trap_SendServerCommand(attacker - g_entities, va("cp \"You killed ^1TEAMMATE ^7%s\n\"", self->client->pers.netname));
		else
			trap_SendServerCommand(attacker - g_entities, va("cp \"You killed %s", self->client->pers.netname));
	}

	if (!attacker->client){
		if (meansOfDeath == MOD_FALLING){
			if (self->client->sess.gender == 0)
				message = "fell to his death";
			else
				message = "fell to her death";
		}
		else if (meansOfDeath == MOD_ADMKILL){//for  admin kill		was 1007
			return;
		}
		else if (meansOfDeath == MOD_CRUSH){
			message = "was crushed";
		}
		else if (meansOfDeath == MOD_WATER){
			message = "drowned";
		}
		else if (meansOfDeath == MOD_SLIME){
			message = "died by toxic materials";
		}
		else if (meansOfDeath == MOD_TRIGGER_HURT){
			message = "was killed";
		}
		else if (meansOfDeath == MOD_MORTAR || meansOfDeath == MOD_MORTAR_SPLASH){
			message = "was shelled";
		}
		else if (meansOfDeath == MOD_FLAMETHROWER){
			message = "was cooked";
		}
		else if (meansOfDeath == MOD_LOPER_LEAP){
			message = "dove on a landmine";
		}
		else if (meansOfDeath == MOD_STUCK) {
			message = "was killed for getting irreversibly stuck";
		}
		else if (meansOfDeath == MOD_MACHINEGUN) {
			message = "was perforated by an automg42";
		}
		else{
			message = "died";
		}
		if (message) {
			trap_SendServerCommand(-1, va("print \"^7%s ^7%s\n", self->client->pers.netname, message));
			return;
		}
	}
	if (attacker == self) {
		if (meansOfDeath == MOD_SELFKILL || meansOfDeath == MOD_SUICIDE){
			if (self->client->sess.gender == 0)
				message = "slit his own throat";
			else
				message = "slit her own throat";
		}
		else if (meansOfDeath == MOD_DYNAMITE_SPLASH || meansOfDeath == MOD_DYNAMITE){
			if (self->client->sess.gender == 0)
				message = "dynamited himself to pieces";
			else
				message = "dynamited herself to pieces";
		}
		else if (meansOfDeath == MOD_GRENADE_PINEAPPLE || meansOfDeath == MOD_GRENADE_SPLASH || meansOfDeath == MOD_GRENADE_LAUNCHER){
			if (self->client->sess.gender == 0)
				message = "dove on his own grenade";
			else
				message = "dove on her own grenade";
		}
		else if (meansOfDeath == MOD_ROCKET_SPLASH){
			if (self->client->sess.gender == 0)
				message = "vaporized himself";
			else
				message = "vaporized herself";
		}
		else if (meansOfDeath == MOD_AIRSTRIKE){
			if (self->client->sess.gender == 0)
				message = "obliterated himself";
			else
				message = "obliterated herself";
		}
		else if (meansOfDeath == MOD_ARTILLERY){
			if (self->client->sess.gender == 0)
				message = "fired for effect on himself";
			else
				message = "fired for effect on herself";
		}
		else if (meansOfDeath == MOD_EXPLOSIVE){
			if (self->client->sess.gender == 0)
				message = "died in his own explosion";
			else
				message = "died in her own explosion";
		}
		else if (meansOfDeath == MOD_MACHINEGUN){
			if (self->client->sess.gender == 0)
				message = "perforated himself";
			else
				message = "perforated herself";
		}
		else if (meansOfDeath == MOD_FLAMETHROWER){
			if (self->client->sess.gender == 0)
				message = "cooked himself";
			else
				message = "cooked herself";
		}
		else if (meansOfDeath == MOD_THROWKNIFE){
			if (self->client->sess.gender == 0)
				message = "failed to juggle his own knives";
			else
				message = "failed to juggle her own knives";
		}
		else{
			if (self->client->sess.gender == 0)
				message = "killed himself";
			else
				message = "killed herself";
		}
		if (message) {
			trap_SendServerCommand(-1, va("print \"^7%s ^7%s\n", self->client->pers.netname, message));
			return;
		}
	}

	if (attacker && attacker->client) {
		if (meansOfDeath == MOD_KNIFE2 || meansOfDeath == MOD_KNIFE || meansOfDeath == MOD_KNIFE_STEALTH){
			message = "was stabbed by";
			message2 = "'s knife";

			if (!OnSameTeam(attacker, self)){
				Q_strncpyz(index, "sound/player/humiliation.wav", sizeof(index));
				for (i = 0; i<MAX_CLIENTS; i++){
					if (level.clients[i].pers.connected != CON_CONNECTED)
						continue;
					other = &g_entities[i];
					te = G_TempEntity(other->s.pos.trBase, EV_GLOBAL_CLIENT_SOUND);
					te->s.eventParm = G_SoundIndex(index);
					te->s.teamNum = other->s.clientNum;
				}
			}
		}

		else if (meansOfDeath == MOD_LUGER){
			message = "was killed by";
			message2 = "'s Luger 9mm";
		}

		else if (meansOfDeath == MOD_COLT){
			message = "was killed by";
			message2 = "'s .45ACP 1911";
		}

		else if (meansOfDeath == MOD_MP40){
			message = "was killed by";
			message2 = "'s MP40";
		}

		else if (meansOfDeath == MOD_THOMPSON){
			message = "was killed by";
			message2 = "'s Thompson";
		}

		else if (meansOfDeath == MOD_STEN){
			message = "was killed by";
			message2 = "'s Sten";
		}

		else if (meansOfDeath == MOD_MAUSER){
			message = "was killed by";
			message2 = "'s Mauser";
		}

		else if (meansOfDeath == MOD_SNIPERRIFLE){
			message = "was killed by";
			message2 = "'s sniper rifle";
		}
		else if (meansOfDeath == MOD_DYNAMITE_SPLASH || meansOfDeath == MOD_DYNAMITE){
			message = "was blasted by";
			message2 = "'s dynamite";
		}
		else if (meansOfDeath == MOD_ROCKET_SPLASH || meansOfDeath == MOD_ROCKET_LAUNCHER || meansOfDeath == MOD_ROCKET){
			message = "was blasted by";
			message2 = "'s Panzerfaust";
		}
		else if (meansOfDeath == MOD_GRENADE_PINEAPPLE || meansOfDeath == MOD_GRENADE_SPLASH || meansOfDeath == MOD_GRENADE || meansOfDeath == MOD_GRENADE_LAUNCHER){
			message = "was exploded by";
			message2 = "'s grenade";
		}
		else if (meansOfDeath == MOD_VENOM){
			message = "was ventilated by";
			message2 = "'s Venom";
		}
		else if (meansOfDeath == MOD_FLAMETHROWER){
			message = "was cooked by";
			message2 = "'s flamethrower";
		}
		else if (meansOfDeath == MOD_MACHINEGUN){
			message = "was perforated by";
			if (g_automg42.integer)
			{
				message2 = "'s automg42";
			}
			else
			{
				message2 = "'s crew-served MG42";
			}
		}
		else if (meansOfDeath == MOD_AIRSTRIKE){
			message = "was blasted by";
			message2 = "'s support fire"; // JPW NERVE changed since it gets called for both air strikes and artillery
		}
		else if (meansOfDeath == MOD_ARTILLERY){
			message = "was shelled by";
			message2 = "'s artillery support"; // JPW NERVE changed since it gets called for both air strikes and artillery
		}
		else if (meansOfDeath == MOD_POISONDMED){		//WAS means of death == 1001
			message = "was poisoned by";
			attacker->client->pers.poison++;
			stats_StoreRoundStat(attacker->client->pers.netname, attacker->client->pers.poison, ROUND_POISON);
		}
		else if (meansOfDeath == MOD_THROWKNIFE){		// WAS meansOfDeath == 1003
			message = "was impaled by";
			message2 = "'s throwing knife";
			attacker->client->pers.knifeKills++;
			stats_StoreRoundStat(attacker->client->pers.netname, attacker->client->pers.knifeKills, ROUND_KNIFETHROW);
		}
		else if (meansOfDeath == MOD_FALLING){
			message = "was pushed by";
			message2 = "";
		}
		else if (meansOfDeath == MOD_TELEFRAG){
			message = "was telefragged by";

		}
		else if (meansOfDeath == MOD_CHICKEN)
		{
			message = "was scared to death by";
			message2 = "";

			self->client->pers.chicken++;
			stats_StoreRoundStat(self->client->pers.netname, self->client->pers.chicken, ROUND_CHICKEN);
			attacker->client->pers.kills++;
			attacker->client->pers.lifeKills++;
			stats_StoreRoundStat(attacker->client->pers.netname, attacker->client->pers.kills, ROUND_KILLS);
			stats_StoreRoundStat(attacker->client->pers.netname, attacker->client->pers.lifeKills, ROUND_KILLPEAK);
		}
		else{
			message = "was killed by";
		}

		// JPW NERVE if attacker != target but on same team
		if ((OnSameTeam(attacker, self)) && (!(attacker == self)) && !g_ffa.integer){
			if (meansOfDeath == MOD_FALLING){
				message = "^1WAS PUSHED BY TEAMMATE";
				message2 = "";
			}
			else{
				message = "^1WAS KILLED BY TEAMMATE";
				message2 = "";
			}
		}
		if (!message2) {
			trap_SendServerCommand(-1, va("print \"^7%s ^7%s ^7%s\n", self->client->pers.netname, message, attacker->client->pers.netname));
			return;
		}
		else{
			trap_SendServerCommand(-1, va("print \"^7%s ^7%s ^7%s^7%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2));
			return;
		}
	}
}
