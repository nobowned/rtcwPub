/*/
===========================================================================
L0 / rtcwPub :: g_admin_cmds.c

Admin commands.

Created: 24. Mar / 2014
===========================================================================
*/
#include "g_admin.h"

/*
===========
Deals with customm commands
===========
*/
void cmd_custom(gentity_t *ent, char *cmd) {
	char arg1[ADMIN_ARG_SIZE];
	char arg2[ADMIN_ARG_SIZE];
	char *tag, *log;

	trap_Argv(1, arg1, sizeof(arg1));
	trap_Argv(2, arg2, sizeof(arg2));

	tag = sortTag(ent);

	if (!strcmp(arg1, "")) {
		CP(va("print \"Command ^1%s ^7must have a value^1!\n\"", cmd));
	}
	else {
		// Rconpasswords or sensitve commands can be changed without public print..
		if (!strcmp(arg2, "@"))
			CP(va("print \"Info: ^2%s ^7was silently changed to ^2%s^7!\n\"", cmd, arg1));
		else
			AP(va("chat \"console: %s ^7changed ^3%s ^7to ^3%s %s\n\"", tag, cmd, arg1, arg2));
		// Change the stuff
		trap_SendConsoleCommand(EXEC_APPEND, va("%s %s %s\n", cmd, arg1, arg2));
		// Log it
		log = va("Player %s (IP: %s) changed %s to %s %s.",
			ent->client->pers.netname, clientIP(ent, qtrue), cmd, arg1, arg2);
		admLog(log);
	}
}

/*
===========
Ignore user
===========
*/
void cmd_ignore(gentity_t *ent) {
	char client_name[ADMIN_ARG_SIZE];
	char *tag, *log;
	gclient_t *client;
	int clientNum;
	tag = sortTag(ent);

	trap_Argv(1, client_name, sizeof(client_name));
	clientNum = ClientNumberFromName(ent, client_name, qtrue);
	if (clientNum == -1) {
		return;
	}

	client = level.clients + clientNum;

	if (client->sess.ignored > IGNORE_OFF){
		CP(va("print \"Player %s ^7is already ignored^1!\n\"", client->pers.netname));
		return;
	}
	else
		client->sess.ignored = IGNORE_PERMANENT;

	AP(va("chat \"console: %s ignored %s^7!\n\"", tag, client->pers.netname));

	// Log it
	log = va("Player %s (IP: %s) ignored user %s.",
		ent->client->pers.netname, clientIP(ent, qtrue), client->pers.netname);
	admLog(log);
}

/*
===========
UnIgnore user
===========
*/
void cmd_unignore(gentity_t *ent) {
	char client_name[ADMIN_ARG_SIZE];
	char *tag, *log;
	gclient_t *client;
	int clientNum;

	tag = sortTag(ent);

	trap_Argv(1, client_name, sizeof(client_name));
	clientNum = ClientNumberFromName(ent, client_name, qtrue);
	if (clientNum == -1) {
		return;
	}

	client = level.clients + clientNum;

	if (client->sess.ignored == IGNORE_OFF){
		CP(va("print \"Player %s ^7is already unignored^1!\n\"", client->pers.netname));
		return;
	}

	client->sess.ignored = IGNORE_OFF;
	client->sess.tempIgnoreCount = 0;

	AP(va("chat \"console: %s unignored %s!\n\"", tag, client->pers.netname));

	// Log it
	log = va("Player %s (IP: %s) unignored user %s.",
		ent->client->pers.netname, clientIP(ent, qtrue), client->pers.netname);
	admLog(log);
}

/*
===========
Ignore user based upon client number
===========
*/
void cmd_clientIgnore(gentity_t *ent) {
	char client_num[ADMIN_ARG_SIZE];
	gentity_t	*targetent;
	char *tag, *log;

	tag = sortTag(ent);

	trap_Argv(1, client_num, sizeof(client_num));
	if (GetClientEntity(ent, client_num, &targetent) == NULL)
	{
		return;
	}

	if (targetent->client->sess.ignored > IGNORE_OFF) {
		CP(va("print \"Player %s ^7is already ignored^1!\"", targetent->client->pers.netname));
		return;
	}

	targetent->client->sess.ignored = IGNORE_PERMANENT;
	AP(va("chat \"console: %s ignored %s^7!\"", tag, targetent->client->pers.netname));
	// Log it
	log = va("Player %s (IP: %s) has clientIgnored user %s.",
		ent->client->pers.netname, clientIP(ent, qtrue), targetent->client->pers.netname);
	admLog(log);
}

/*
===========
UnIgnore user based upon client number
===========
*/
void cmd_clientUnignore(gentity_t *ent) {
	char client_num[ADMIN_ARG_SIZE];
	gentity_t	*targetent;
	char *tag, *log;

	tag = sortTag(ent);

	trap_Argv(1, client_num, sizeof(client_num));
	if (GetClientEntity(ent, client_num, &targetent) == NULL)
	{
		return;
	}

	if (targetent->client->sess.ignored == IGNORE_OFF) {
		CP(va("print \"Player %s ^7is already unignored^1!\"", targetent->client->pers.netname));
		return;
	}

	targetent->client->sess.tempIgnoreCount = 0;

	targetent->client->sess.ignored = IGNORE_OFF;
	AP(va("chat \"console: %s unignored %s^7!\"", sortTag(ent), targetent->client->pers.netname));
	// Log it	
	log = va("Player %s (IP: %s) has clientUnignored user %s.",
		ent->client->pers.netname, clientIP(ent, qtrue), targetent->client->pers.netname);
	admLog(log);
}

/*
===========
Kick player + optional <msg>
===========
*/
void cmd_kick(gentity_t *ent) {
	char client_name[ADMIN_ARG_SIZE];
	char *tag, *log;
	gclient_t *client;
	int clientNum;

	tag = sortTag(ent);

	trap_Argv(1, client_name, sizeof(client_name));
	clientNum = ClientNumberFromName(ent, client_name, qtrue);
	if (clientNum == -1) {
		return;
	}

	client = level.clients + clientNum;

	char *reason = ConcatArgs(2);
	qboolean has_reason = strlen(reason);

	trap_DropClient(clientNum, va("kicked by %s\n^7%s", tag, (has_reason ? va("Reason: %s", reason) : "")));
	AP(va("chat \"console: %s kicked %s^7! %s\n\"", tag, client->pers.netname,
		(has_reason ? va("^3Reason: ^7%s", reason) : "")));

	// Log it
	log = va("Player %s (IP: %s) kicked user %s. %s",
		ent->client->pers.netname, clientIP(ent, qtrue), client->pers.netname,
		(has_reason ? va("Reason: %s", reason) : ""));
	admLog(log);
}

/*
===========
Kick player based upon clientnumber + optional <msg>
===========
*/
void cmd_clientkick(gentity_t *ent) {
	char client_num[ADMIN_ARG_SIZE];
	gentity_t	*targetent;
	char *tag, *log;

	tag = sortTag(ent);

	trap_Argv(1, client_num, sizeof(client_num));
	if (GetClientEntity(ent, client_num, &targetent) == NULL)
	{
		return;
	}

	char *reason = ConcatArgs(2);
	qboolean has_reason = strlen(reason);

	//kick the client
	trap_DropClient(targetent - g_entities, va("^3kicked by ^7%s \n^7%s", tag, (has_reason ? va("Reason: %s", reason) : "")));
	AP(va("chat \"console: %s kicked %s^7! %s\n\"", tag, targetent->client->pers.netname,
		(has_reason ? va("Reason: ^7%s", reason) : "")));

	// Log it
	log = va("Player %s (IP: %s) clientKicked user %s. %s",
		ent->client->pers.netname, clientIP(ent, qtrue), targetent->client->pers.netname,
		(has_reason ? va("Reason: %s", reason) : ""));
	admLog(log);
}

/*
===========
Slap player
===========
*/
void cmd_slap(gentity_t *ent) {
	char client_name[ADMIN_ARG_SIZE];
	char *tag;
	gentity_t *target;
	gclient_t *client;
	int clientNum;
	int damagetodo = 20;

	trap_Argv(1, client_name, sizeof(client_name));
	clientNum = ClientNumberFromName(ent, client_name, qtrue);
	if (clientNum == -1)
	{
		return;
	}

	tag = sortTag(ent);
	target = g_entities + clientNum;
	client = level.clients + clientNum;

	if ((!client) || (client->pers.connected != CON_CONNECTED))
	{
		CP("print \"Invalid client number^1!\n\"");
		return;
	}

	if (client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		CP("print \"You cannot slap a spectator^1!\n\"");
		return;
	}

	if (client->ps.stats[STAT_HEALTH] <= 0)
	{
		CP(va("print \"%s ^7is already dead!\n\"", client->pers.netname));
		return;
	}

	if (client->ps.stats[STAT_HEALTH] - damagetodo <= 0)
	{
		AP(va("chat \"console: %s ^7was slapped to death by %s^7!\n\"", client->pers.netname, tag));
	}
	else
	{
		AP(va("chat \"console: %s ^7was slapped by %s^7!\n\"", client->pers.netname, tag));
	}

	G_Damage(target, NULL, NULL, NULL, NULL, damagetodo, DAMAGE_NO_PROTECTION, MOD_ADMKILL);
	G_AddEvent(target, EV_GENERAL_SOUND, G_SoundIndex("sound/multiplayer/vo_revive.wav"));

	if (g_extendedLog.integer >= 2)
	{
		admLog(va("Player %s (IP: %s) slapped player %s.", ent->client->pers.netname, clientIP(ent, qtrue), client->pers.netname));
	}
}

/*
===========
Kill player
===========
*/
void cmd_kill(gentity_t *ent) {
	char client_name[ADMIN_ARG_SIZE];
	int clientNum;
	gentity_t *target;
	gclient_t *client;
	int damagetodo;
	char *tag, *log, *log2;

	tag = sortTag(ent);
	// Sort log
	log = va("Player %s (IP: %s) has killed ",
		ent->client->pers.netname, clientIP(ent, qtrue));

	trap_Argv(1, client_name, sizeof(client_name));
	clientNum = ClientNumberFromName(ent, client_name, qtrue);
	if (clientNum == -1) {
		return;
	}

	target = g_entities + clientNum;
	client = level.clients + clientNum;

	if (!client || client->pers.connected != CON_CONNECTED) {
		CP("print \"Invalid client number^1!\n\"");
		return;
	}

	if (client->ps.stats[STAT_HEALTH] <= 0) {
		CP(va("print \"%s ^7is already dead!\n\"", client->pers.netname));
		return;
	}

	if (client->sess.sessionTeam == TEAM_SPECTATOR) {
		CP("print \"You cannot kill a spectator^1!\n\"");
		return;
	}

	damagetodo = 250; // Kill the player on spot
	G_Damage(target, NULL, NULL, NULL, NULL, damagetodo, DAMAGE_NO_PROTECTION, MOD_ADMKILL);
	AP(va("chat \"console: %s ^7was killed by %s^7!\n\"", client->pers.netname, tag));
	player_die(target, target, target, 100000, MOD_ADMKILL);

	// Log it
	log2 = va("%s user %s.", log, ent->client->pers.netname);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		admLog(log2);
}

void cmd_specs2(gentity_t *ent, gentity_t *targ)
{
	char *tag, *log;

	tag = sortTag(ent);

	if (targ->client->sess.sessionTeam == TEAM_SPECTATOR) {
		CP(va("print \"Player %s ^7is already a spectator^1!\n\"", targ->client->pers.netname));
		return;
	}
	else
		SetTeam(targ, "spectator", qtrue);
	AP(va("chat \"console: %s forced %s ^7to ^3spectators^7!\n\"", tag, targ->client->pers.netname));

	// Log it
	log = va("Player %s (IP: %s) forced user %s to spectators.",
		ent->client->pers.netname, clientIP(ent, qtrue), targ->client->pers.netname);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		admLog(log);
}

/*
===========
Force user to spectators
===========
*/
void cmd_specs(gentity_t *ent) {
	char client_name[ADMIN_ARG_SIZE];
	int client_num;

	trap_Argv(1, client_name, sizeof(client_name));
	client_num = ClientNumberFromName(ent, client_name, qtrue);
	if (client_num == -1)
		return;

	cmd_specs2(ent, g_entities + client_num);
}

void cmd_axis2(gentity_t *ent, gentity_t *targ)
{
	char *tag, *log;

	tag = sortTag(ent);

	if (targ->client->sess.sessionTeam == TEAM_RED) {
		CP(va("print \"Player %s ^7is already an ^1Axis ^7recruit!\n\"", targ->client->pers.netname));
		return;
	}
	else
		SetTeam(targ, "red", qtrue);

	if (g_ffa.integer)
	{
		AP(va("chat \"console: %s forced %s ^7to the join the ^1Battle!\n\"", tag, targ->client->pers.netname));
	}
	else
	{
		AP(va("chat \"console: %s forced %s ^7to the ^1Axis!\n\"", tag, targ->client->pers.netname));
	}

	// Log it
	log = va("Player %s (IP: %s) forced user %s into Axis team.",
		ent->client->pers.netname, clientIP(ent, qtrue), targ->client->pers.netname);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		admLog(log);
}

/*
===========
Force user to Axis
===========
*/
void cmd_axis(gentity_t *ent) {
	char client_name[ADMIN_ARG_SIZE];
	int client_num;

	trap_Argv(1, client_name, sizeof(client_name));
	client_num = ClientNumberFromName(ent, client_name, qtrue);
	if (client_num == -1)
		return;

	cmd_axis2(ent, g_entities + client_num);
}

void cmd_allied2(gentity_t *ent, gentity_t *targ)
{
	char *tag, *log;

	tag = sortTag(ent);

	if (targ->client->sess.sessionTeam == TEAM_BLUE) {
		CP(va("print \"Player %s ^7is already an ^4Allies ^7recruit!\n\"", targ->client->pers.netname));
		return;
	}
	else
		SetTeam(targ, "blue", qtrue);

	if (g_ffa.integer)
	{
		AP(va("chat \"console: %s forced %s ^7to the join the ^1Battle!\n\"", tag, targ->client->pers.netname));
	}
	else
	{
		AP(va("chat \"console: %s forced %s ^7to the ^4Allies!\n\"", tag, targ->client->pers.netname));
	}

	// Log it
	log = va("Player %s (IP: %s) forced user %s into Allies team.",
		ent->client->pers.netname, clientIP(ent, qtrue), targ->client->pers.netname);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		admLog(log);
}

/*
===========
Force user to Allied
===========
*/
void cmd_allied(gentity_t *ent) {
	char client_name[ADMIN_ARG_SIZE];
	int client_num;

	trap_Argv(1, client_name, sizeof(client_name));
	client_num = ClientNumberFromName(ent, client_name, qtrue);
	if (client_num == -1)
		return;

	cmd_allied2(ent, g_entities + client_num);
}

/*
===========
Forces player to specified team
===========
*/
void cmd_forceteam(gentity_t *ent) {
	char client_name[ADMIN_ARG_SIZE];
	char team[ADMIN_ARG_SIZE];
	int client_num;
	gentity_t *targ;

	trap_Argv(1, client_name, sizeof(client_name));
	client_num = ClientNumberFromName(ent, client_name, qtrue);
	if (client_num == -1) {
		return;
	}
	targ = g_entities + client_num;

	trap_Argv(2, team, sizeof(team));

	if (!strcmp(team, "s") ||
		!strcmp(team, "spec") ||
		!strcmp(team, "spectator")) {
		cmd_specs2(ent, targ);
	}
	else if (
		!strcmp(team, "axis") ||
		!strcmp(team, "red") ||
		!strcmp(team, "r")) {
		cmd_axis2(ent, targ);
	}
	else if (
		!strcmp(team, "allies") ||
		!strcmp(team, "blue") ||
		!strcmp(team, "b")) {
		cmd_allied2(ent, targ);
	}
}

/*
===========
Forces player to specified team by client number
===========
*/
void cmd_forceteam_client(gentity_t *ent)
{
	char client_num[ADMIN_ARG_SIZE];
	char team[ADMIN_ARG_SIZE];
	gentity_t *targ;

	trap_Argv(1, client_num, sizeof(client_num));
	if (GetClientEntity(ent, client_num, &targ) == NULL)
	{
		return;
	}

	trap_Argv(2, team, sizeof(team));
	if (!strcmp(team, "s") ||
		!strcmp(team, "spec") ||
		!strcmp(team, "spectator")) {
		cmd_specs2(ent, targ);
	}
	else if (
		!strcmp(team, "axis") ||
		!strcmp(team, "red") ||
		!strcmp(team, "r")) {
		cmd_axis2(ent, targ);
	}
	else if (
		!strcmp(team, "allies") ||
		!strcmp(team, "blue") ||
		!strcmp(team, "b")) {
		cmd_allied2(ent, targ);
	}
}

/*
===========
Execute command
===========
*/
void cmd_exec(gentity_t *ent) {
	char *tag, *log;
	char no_extension[MAX_QPATH];
	char filename[ADMIN_ARG_SIZE];
	char silence_option[ADMIN_ARG_SIZE];

	trap_Argv(1, filename, sizeof(filename));
	if (!FileExists(filename, "", ".cfg", qtrue)) {
		COM_StripExtension2(filename, no_extension, sizeof(no_extension));

		CP(va("print \"%s.cfg is not on server.\n\"", no_extension));
		return;
	}

	tag = sortTag(ent);

	trap_Argv(2, silence_option, sizeof(silence_option));
	if (!strcmp(silence_option, "@"))
		CP(va("print \"^3Info: ^7%s has been executed^7!\n\"", filename));
	else
		AP(va("print \"console: %s executed ^3%s^7!\n\"", tag, filename));

	trap_SendConsoleCommand(EXEC_INSERT, va("exec \"%s\"\n", filename));

	// Log it
	log = va("Player %s (IP: %s) executed %s config.",
		ent->client->pers.netname, clientIP(ent, qtrue), filename);
	admLog(log);
}

/*
===========
Nextmap
===========
*/
void cmd_nextmap(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);
	AP(va("chat \"console: %s set the ^3nextmap ^7in rotation^7!\n\"", tag));
	trap_SendConsoleCommand(EXEC_APPEND, "vstr nextmap\n");

	// Log it
	log = va("Player %s (IP: %s) set nextmap.",
		ent->client->pers.netname, clientIP(ent, qtrue));
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		admLog(log);
}

/*
===========
Load map
===========
*/
void cmd_map(gentity_t *ent) {
	char map_name[ADMIN_ARG_SIZE];
	char *tag, *log;

	trap_Argv(1, map_name, sizeof(map_name));
	if (!Q_stricmp(map_name, ""))
	{
		CP("print \"^1Error: ^7Please give a mapname!\n\"");
		return;
	}

	if (!FileExists(map_name, "maps", ".bsp", qfalse))
	{
		CP(va("print \"^3%s ^7is not on the server.\n\"", map_name));
		return;
	}

	tag = sortTag(ent);
	AP(va("chat \"console: %s loaded map %s!\n\"", tag, map_name));
	trap_SendConsoleCommand(EXEC_APPEND, va("map %s\n", map_name));

	// Log it
	log = va("Player %s (IP: %s) loaded %s map.",
		ent->client->pers.netname, clientIP(ent, qtrue), map_name);
	admLog(log);
}

/*
===========
Vstr

Loads next map in rotation (if any)
===========
*/
void cmd_vstr(gentity_t *ent) {
	char label[ADMIN_ARG_SIZE];
	char *tag, *log;

	trap_Argv(1, label, sizeof(label));
	if (!Q_stricmp(label, ""))
	{
		CP("print \"^1Error: ^7Please set a label (second command) so it can jump to that part of rotation!\n\"");
		return;
	}

	tag = sortTag(ent);
	AP(va("chat \"console: %s set vstr to %s.\n\"", tag, label));
	trap_SendConsoleCommand(EXEC_APPEND, va("vstr %s\n", label));

	// Log it
	log = va("Player %s (IP: %s) set vstr to %s",
		ent->client->pers.netname, clientIP(ent, qtrue), label);
	admLog(log);
}

/*
===========
Center prints message to all
===========
*/
void cmd_cpa(gentity_t *ent) {
	char *message, *log;

	message = ConcatArgs(1);
	AP(va("cp \"^1ADMIN WARNING^7! \n%s\n\"", message));

	// Log it
	log = va("Player %s (IP: %s) issued CPA warning: %s",
		ent->client->pers.netname, clientIP(ent, qtrue), message);

	// Only log this if it is set to 2+
	if (g_extendedLog.integer >= 2) 
		admLog(log);
}

/*
===========
Shows message to selected user in center print
===========
*/
void cmd_cp(gentity_t *ent) {
	char client_num[ADMIN_ARG_SIZE];
	gentity_t	*targetent;
	char *message, *log;

	trap_Argv(1, client_num, sizeof(client_num));
	if (GetClientEntity(ent, client_num, &targetent) == NULL)
	{
		return;
	}

	message = ConcatArgs(2);

	// CP to user	
	CPx(targetent - g_entities, va("cp \"^1ADMIN WARNING^7! \n%s\n\"2", message));

	// Log it
	log = va("Player %s (IP: %s) issued to user %s a CP warning: %s",
		ent->client->pers.netname, clientIP(ent, qtrue), targetent->client->pers.netname, message);

	// Only log this if it is set to 2+
	if (g_extendedLog.integer >= 2) 
		admLog(log);
}

/*
===========
Shows message to all in console and center print
===========
*/
void cmd_warn(gentity_t *ent) {
	char *message, *log;

	message = ConcatArgs(1);
	AP(va("cp \"^1ADMIN WARNING^7: \n%s\n\"2", message));
	AP(va("chat \"^1ADMIN WARNING^7: \n%s\n\"", message));

	// Log it
	log = va("Player %s (IP: %s) issued global warning: %s",
		ent->client->pers.netname, clientIP(ent, qtrue), message);

	// Only log this if it is set to 2+
	if (g_extendedLog.integer >= 2) 
		admLog(log);
}

/*
===========
Shows message to all in console
===========
*/
void cmd_chat(gentity_t *ent) {
	char *message, *log;

	message = ConcatArgs(1);
	AP(va("chat \"^1ADMIN WARNING^7: \n%s\n\"", message));

	// Log it
	log = va("Player %s (IP: %s) issued CHAT warning: %s",
		ent->client->pers.netname, clientIP(ent, qtrue), message);

	// Only log this if it is set to 2+
	if (g_extendedLog.integer >= 2) 
		admLog(log);
}
/*
===========
Cancels any vote in progress
===========
*/
void cmd_cancelvote(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);

	if (level.voteTime) {
		level.voteNo = level.numNonDisconnectedClients;
		CheckVote();
		AP(va("chat \"%s cancelled the vote.\n\"2", tag));

		// Log it
		log = va("Player %s (IP: %s) cancelled a vote.",
			ent->client->pers.netname, clientIP(ent, qtrue));

		// Only log this if it is set to 2+
		if (g_extendedLog.integer >= 2) 
			admLog(log);
	}
}

/*
===========
Passes any vote in progress
===========
*/
void cmd_passvote(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);

	if (level.voteTime) {
		level.voteYes = level.numNonDisconnectedClients;
		CheckVote();
		AP(va("cp \"%s passed the vote.\n\"2", tag));

		// Log it
		log = va("Player %s (IP: %s) passed a vote.",
			ent->client->pers.netname, clientIP(ent, qtrue));

		// Only log this if it is set to 2+
		if (g_extendedLog.integer >= 2) 
			admLog(log);
	}
}

/*
===========
Map restart
===========
*/
void cmd_map_restart(gentity_t *ent) {
	char time_arg[ADMIN_ARG_SIZE];
	char *tag, *log;
	int time;

	tag = sortTag(ent);

	trap_Argv(1, time_arg, sizeof(time_arg));
	if (strlen(time_arg)) {
		time = atoi(time_arg);
		if (time > 120) time = 120;
		else if (time < 0) time = 0;

		AP(va("chat \"console: %s %sset warmup to %i %s.\n\"",
			tag, level.warmupTime ? "" : "restarted the match and ", time, time <= 1 ? "second" : "seconds"));
		trap_SendConsoleCommand(EXEC_APPEND, va("map_restart %i\n", time));
	}
	else {
		AP(va("chat \"console: %s %s the match.\n\"",
			tag, level.warmupTime ? "started" : "restarted"));
		trap_SendConsoleCommand(EXEC_APPEND, "map_restart\n");
	}

	// Log it
	log = va("Player %s (IP: %s) restarted the map.",
		ent->client->pers.netname, clientIP(ent, qtrue));

	// Only log this if it is set to 2+
	if (g_extendedLog.integer >= 2)
		admLog(log);
}

/*
===========
Reset match
===========
*/
void cmd_resetmatch(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);
	AP(va("chat \"console: %s reset the match.\n\"", tag));
	trap_SendConsoleCommand(EXEC_APPEND, "reset_match\n");

	// Log it
	log = va("Player %s (IP: %s) reset match.",
		ent->client->pers.netname, clientIP(ent, qtrue));

	// Only log this if it is set to 2+
	if (g_extendedLog.integer >= 2) 
		admLog(log);
}

/*
===========
Swap teams
===========
*/
void cmd_swap(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);
	AP(va("chat \"console: %s swapped the teams.\n\"", tag));
	trap_SendConsoleCommand(EXEC_APPEND, "swap_teams\n");

	// Log it
	log = va("Player %s (IP: %s) swapped teams.",
		ent->client->pers.netname, clientIP(ent, qtrue));

	// Only log this if it is set to 2+
	if (g_extendedLog.integer >= 2)
		admLog(log);
}

/*
===========
Shuffle
===========
*/
void cmd_shuffle(gentity_t *ent) {
	char *tag, *log;
	int count = 0, tmpCount, i;
	int players[MAX_CLIENTS];

	tag = sortTag(ent);
	memset(players, -1, sizeof(players));

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if ((!g_entities[i].inuse) || (level.clients[i].pers.connected != CON_CONNECTED))
			continue;

		// Ignore spectators
		if ((level.clients[i].sess.sessionTeam != TEAM_RED) && (level.clients[i].sess.sessionTeam != TEAM_BLUE))
			continue;

		players[count] = i;
		count++;
	}

	tmpCount = count;

	for (i = 0; i < count; i++)
	{
		int j;

		do {
			j = (rand() % count);
		} while (players[j] == -1);

		if (i & 1)
			level.clients[players[j]].sess.sessionTeam = TEAM_BLUE;
		else
			level.clients[players[j]].sess.sessionTeam = TEAM_RED;

		ClientUserinfoChanged(players[j]);
		ClientBegin(players[j]);

		players[j] = players[tmpCount - 1];
		players[tmpCount - 1] = -1;
		tmpCount--;
	}

	AP(va("chat \"console: %s shuffled the teams.\n\"", tag));
	trap_SendConsoleCommand(EXEC_APPEND, va("reset_match %i\n", GS_WARMUP));

	// Log it
	log = va("Player %s (IP: %s) shuffled the teams.",
		ent->client->pers.netname, clientIP(ent, qtrue));

	// Only log this if it is set to 2+
	if (g_extendedLog.integer >= 2)
		admLog(log);
}

/*
==================
Move lagged out or downloading clients to spectators
==================
*/
void cmd_specs999(gentity_t *ent) {
	int i;
	qboolean moved = qfalse;
	char *tag, *log;

	tag = sortTag(ent);
	for (i = 0; i < level.maxclients; i++) {
		ent = &g_entities[i];
		if (!ent->client) continue;
		if (ent->client->pers.connected != CON_CONNECTED) continue;
		if (ent->client->ps.ping >= 999) {
			SetTeam(ent, "s", qtrue);
			moved = qtrue;
		}
	}

	if (moved)
		AP(va("chat \"console: %s moved all lagged out players to spectators!\n\"", tag));
	else
		CP("print \"No one to move to spectators^1!\n\"");

	// Log it
	log = va("Player %s (IP: %s) forced all 999 to spectators.",
		ent->client->pers.netname, clientIP(ent, qtrue));

	// Only log this if it is set to 2+
	if (g_extendedLog.integer >= 2) 
		admLog(log);
}

/*
==================
Reveal location of a player.
==================
*/
void cmd_revealCamper(gentity_t *ent) {
	char client_num[ADMIN_ARG_SIZE];
	char location[64];
	char *tag, *log;
	gentity_t *targetent;

	tag = sortTag(ent);

	trap_Argv(1, client_num, sizeof(client_num));
	if (GetClientEntity(ent, client_num, &targetent) == NULL)
	{
		return;
	}

	Team_GetLocationMsg(targetent, location, sizeof(location), qtrue);
	AP(va("chat \"console: %s revealed that %s ^7is hiding at %s^7.\n\"", tag, targetent->client->pers.netname, location));

	// Log it
	log = va("Player %s (IP: %s) revealed %s location.", ent->client->pers.netname, clientIP(ent, qtrue), targetent->client->pers.netname);

	// Only log this if it is set to 2+
	if (g_extendedLog.integer >= 2)
		admLog(log);
}

/*
===========
Rename player
===========
*/
void cmd_rename(gentity_t *ent) {
	char client_num[ADMIN_ARG_SIZE];
	gentity_t	*targetent;
	char *tag, *newname;
	char userinfo[MAX_INFO_STRING];

	trap_Argv(1, client_num, sizeof(client_num));
	if (GetClientEntity(ent, client_num, &targetent) == NULL)
	{
		return;
	}

	tag = sortTag(ent);
	newname = ConcatArgs(2);

	// Print first..
	AP(va("chat \"console: %s renamed %s ^7to %s^7!\n\"", tag, targetent->client->pers.netname, newname));

	// Not vital..
	if (g_extendedLog.integer > 1)
		admLog(va("Player %s (IP: %s) renamed user %s to %s", ent->client->pers.netname, clientIP(ent, qtrue), targetent->client->pers.netname, newname));

	// Rename..
	trap_GetUserinfo(targetent->s.clientNum, userinfo, sizeof(userinfo));
	Info_SetValueForKey(userinfo, "name", newname);
	trap_SetUserinfo(targetent->s.clientNum, userinfo);
	ClientUserinfoChanged(targetent->s.clientNum);
}

/*
===========
Rename Off
===========
*/
void cmd_renameoff(gentity_t *ent) {
	char client_num[ADMIN_ARG_SIZE];
	gentity_t	*targetent;
	char *tag, *log;

	tag = sortTag(ent);

	trap_Argv(1, client_num, sizeof(client_num));
	if (GetClientEntity(ent, client_num, &targetent) == NULL)
	{
		return;
	}

	if (targetent->client->pers.nameLocked) {
		CP(va("print \"^1Error: ^7%s ^7is already name locked!\n\"", targetent->client->pers.netname));
		return;
	}

	AP(va("chat \"console: %s revoked %s^7's ability to rename.\n\"", tag, targetent->client->pers.netname));
	targetent->client->pers.nameLocked = qtrue;

	// Log it
	log = va("Player %s (IP: %s) revoked %s^7's ability to rename",
		ent->client->pers.netname, clientIP(ent, qtrue), targetent->client->pers.netname);
	admLog(log);
}

/*
===========
Rename On
===========
*/
void cmd_renameon(gentity_t *ent) {
	char client_num[ADMIN_ARG_SIZE];
	gentity_t	*targetentity;
	char *log;

	trap_Argv(1, client_num, sizeof(client_num));
	if (GetClientEntity(ent, client_num, &targetentity) == NULL)
	{
		return;
	}

	if (!targetentity->client->pers.nameLocked) {
		CP(va("print \"^1Error: ^7%s ^7can already rename!\n\"", targetentity->client->pers.netname));
		return;
	}

	AP(va("chat \"console: %s restored %s^7's ability to rename.\n\"", sortTag(ent), targetentity->client->pers.netname));
	targetentity->client->pers.nameLocked = qfalse;

	// Log it
	log = va("Player %s (IP: %s) restored %s^7's ability to rename",
		ent->client->pers.netname, clientIP(ent, qtrue), targetentity->client->pers.netname);
	admLog(log);
}

/*
===========
Lock/Unlock team

NOTE: Somewhat messy due verbosity
TODO: Clean this eventually..
===========
*/
qboolean canTeamBeLocked(int team)
{
	if (team == TEAM_RED && axisPlayers.integer < 1)
		return qfalse;
	else if (team == TEAM_BLUE && alliedPlayers.integer < 1)
		return qfalse;
	else
		return qtrue;
}

void cmd_lockteam(gentity_t *ent) {
	char team_arg[ADMIN_ARG_SIZE];
	char *tag = sortTag(ent);
	int team = TEAM_NUM_TEAMS;
	char *teamTag = "both^7";
	char *log;

	trap_Argv(1, team_arg, sizeof(team_arg));
	if (!Q_stricmp(team_arg, "")) {
		CP("print \"^1Error: ^7Please select which team you wish to lock!\n\"");
		return;
	}

	// Axis
	if (!Q_stricmp(team_arg, "red") || !Q_stricmp(team_arg, "axis") || !Q_stricmp(team_arg, "r")) {
		team = TEAM_RED;
		teamTag = "^1Axis^7";
	}
	// Allies
	else if (!Q_stricmp(team_arg, "blue") || !Q_stricmp(team_arg, "allies") || !Q_stricmp(team_arg, "allied") || !Q_stricmp(team_arg, "b"))	{
		team = TEAM_BLUE;
		teamTag = "^4Allies^7";
	}
	// Both
	else if (!(strcmp(team_arg, "both") == 0))
	{
		CP("print \"^1Error: ^7Please select which team you wish to lock!\n\"");
		return;
	}

	if (team != TEAM_NUM_TEAMS)
	{
		if (teamInfo[team].team_lock == qtrue)
		{
			CP(va("print \"^1Error: ^7%s team is already locked!\n\"", teamTag));
			return;
		}
		else
		{
			if (!canTeamBeLocked(team))
			{
				CP(va("print \"^1Error: ^7%s team is empty!\n\"", teamTag));
				return;
			}

			AP(va("chat \"console: %s locked the %s team!\n\"", tag, teamTag));
			teamInfo[team].team_lock = qtrue;
		}
	}
	else
	{
		if (teamInfo[TEAM_RED].team_lock != qtrue || teamInfo[TEAM_BLUE].team_lock != qtrue)
		{
			teamInfo[TEAM_RED].team_lock = qtrue;
			teamInfo[TEAM_BLUE].team_lock = qtrue;
			AP(va("chat \"console: %s locked %s teams!\n\"", tag, teamTag));
		}
		else
			CP("print \"^1Error: ^7Both teams are already locked!\n\"");
		return;
	}

	// Log it
	log = va("Player %s (IP: %s) issued lock for %s",
		ent->client->pers.netname, clientIP(ent, qtrue),
		(team == TEAM_RED ? "Axis team" : (team == TEAM_BLUE ? "Allies team" : "Both teams")));
	admLog(log);
}

void cmd_unlockteam(gentity_t *ent) {
	char team_arg[ADMIN_ARG_SIZE];
	char *tag = sortTag(ent);
	int team = TEAM_NUM_TEAMS;
	char *teamTag = "both^7";
	char *log;

	trap_Argv(1, team_arg, sizeof(team_arg));
	if (!Q_stricmp(team_arg, "")) {
		CP("print \"^1Error: ^7Please select which team you wish to unlock!\n\"");
		return;
	}

	// Axis
	if (!Q_stricmp(team_arg, "red") || !Q_stricmp(team_arg, "axis") || !Q_stricmp(team_arg, "r")) {
		team = TEAM_RED;
		teamTag = "^1Axis^7";
	}
	// Allies
	else if (!Q_stricmp(team_arg, "blue") || !Q_stricmp(team_arg, "allies") || !Q_stricmp(team_arg, "allied") || !Q_stricmp(team_arg, "b"))	{
		team = TEAM_BLUE;
		teamTag = "^4Allies^7";
	}
	// Both
	else if (!(strcmp(team_arg, "both") == 0))
	{
		CP("print \"^1Error: ^7Please select which team you wish to unlock!\n\"");
		return;
	}

	if (team != TEAM_NUM_TEAMS)
	{
		if (teamInfo[team].team_lock == qfalse)
		{
			CP(va("print \"^1Error: ^7%s team is already unlocked!\n\"", teamTag));
			return;
		}
		else
		{
			if (!canTeamBeLocked(team))
			{
				CP(va("print \"^1Error: ^7%s team is empty!\n\"", teamTag));
				return;
			}

			AP(va("chat \"console: %s unlocked the %s team!\n\"", tag, teamTag));
			teamInfo[team].team_lock = qfalse;
		}
	}
	else
	{
		if (teamInfo[TEAM_RED].team_lock != qfalse || teamInfo[TEAM_BLUE].team_lock != qfalse)
		{
			teamInfo[TEAM_RED].team_lock = qfalse;
			teamInfo[TEAM_BLUE].team_lock = qfalse;
			AP(va("chat \"console: %s unlocked %s teams!\n\"", tag, teamTag));
		}
		else
			CP("print \"^1Error: ^7Both teams are already unlocked!\n\"");
		return;
	}

	// Log it
	log = va("Player %s (IP: %s) has issued unlock for %s",
		ent->client->pers.netname, clientIP(ent, qtrue),
		(team == TEAM_RED ? "Axis team" : (team == TEAM_BLUE ? "Allies team" : "Both teams")));
	admLog(log);
}

/*
===========
Ban ip
===========
*/
void cmd_ban(gentity_t *ent) {
	char client_name[ADMIN_ARG_SIZE];
	int clientNum;
	char *tag, *log, *targ_ip;
	gclient_t *client;
	gentity_t *targ;

	tag = sortTag(ent);
	trap_Argv(1, client_name, sizeof(client_name));
	clientNum = ClientNumberFromName(ent, client_name, qtrue);
	if (clientNum == -1)
		return;

	targ = g_entities + clientNum;
	client = targ->client;
	targ_ip = clientIP(targ, qtrue);

	// Ban player			
	trap_SendConsoleCommand(EXEC_APPEND, va("addip \"%s/32|%s|%s\"\n", targ_ip, client->pers.netname, getDate()));

	// Kick player now
	char *reason = ConcatArgs(2);
	trap_DropClient(clientNum, va("banned by %s \n^7%s", tag, reason));
	AP(va("chat \"console: %s banned %s^7! ^3%s\n\"", tag, client->pers.netname, reason));
	CP(va("chat \"You banned %s^7 [%s]\n\"", client->pers.netname, targ_ip));

	// Log it
	log = va("Player %s (IP: %s) banned user %s (IP: %s)",
		ent->client->pers.netname, clientIP(ent, qtrue), client->pers.netname, clientIP(targ, qtrue));
	admLog(log);
}

/*
===========
Ban ip via cnum
===========
*/
void cmd_banClient(gentity_t *ent)
{
	char client_num[ADMIN_ARG_SIZE];
	char *tag, *log, *targ_ip;
	gclient_t *client;
	gentity_t *targ;

	trap_Argv(1, client_num, sizeof(client_num));
	if (GetClientEntity(ent, client_num, &targ) == NULL) {
		return;
	}

	tag = sortTag(ent);
	client = targ->client;
	targ_ip = clientIP(targ, qtrue);

	// Ban player
	trap_SendConsoleCommand(EXEC_APPEND, va("addip \"%s/32|%s|%s\"\n", targ_ip, client->pers.netname, getDate()));

	// Kick player now
	char *reason = ConcatArgs(2);
	trap_DropClient(targ - g_entities, va("banned by %s \n^7%s", tag, reason));
	AP(va("chat \"console: %s banned %s^7! ^3%s\n\"", tag, client->pers.netname, reason));
	CP(va("chat \"You banned %s^7 [ip: %s] [client_num: %s]\n\"", client->pers.netname, targ_ip, client_num));

	// Log it
	log = va("Player %s (IP: %s) client banned user %s (IP: %s)",
		ent->client->pers.netname, clientIP(ent, qtrue), client->pers.netname, clientIP(targ, qtrue));
	admLog(log);
}

/*
===========
RangeBan ip
===========
*/
void cmd_rangeBan(gentity_t *ent) {
	char client_name[ADMIN_ARG_SIZE];
	char range[ADMIN_ARG_SIZE];
	int clientNum;
	char *tag, *log;
	gclient_t *client;

	tag = sortTag(ent);
	trap_Argv(1, client_name, sizeof(client_name));
	clientNum = ClientNumberFromName(ent, client_name, qtrue);
	if (clientNum == -1)
		return;

	client = level.clients + clientNum;

	// Figure out what we're about
	trap_Argv(2, range, sizeof(range));
	if (Q_stricmp(range, "32") && Q_stricmp(range, "24") && Q_stricmp(range, "16") && Q_stricmp(range, "8")) {
		if (Q_stricmp(range, ""))
			CP(va("print \"^1Error: ^7%s is not a valid Subnet Range!\n^3Valid input is: ^7!rangeban <player> <8/16/24/32>\n", range));
		else
			CP(va("print \"^1Error: ^7Missing Subnet!\n^3Valid input is: ^7!rangeban <player> <8/16/24/32>\n", range));
		return;
	}

	// Ban player			
	trap_SendConsoleCommand(EXEC_APPEND, va("addip \"%s/%s|%s|%s\"\n",
		clientIP(&g_entities[clientNum], qtrue),
		range, client->pers.netname, getDate()));

	// Kick player now
	trap_DropClient(clientNum, va("banned by ^3%s", tag));
	AP(va("chat \"console: %s banned %s^7!\n\"", tag, client->pers.netname));

	// Log it
	log = va("Player %s (IP: %s) has (IP)rangeBanned user %s",
		ent->client->pers.netname, clientIP(ent, qtrue), client->pers.netname);
	admLog(log);
}

/*
===========
tempBan ip by name
===========
*/
void cmd_tempBan(gentity_t *ent) {
	char client_name[ADMIN_ARG_SIZE];
	char ban_duration_arg[ADMIN_ARG_SIZE];
	int clientNum;
	char *tag, *log;
	int time;
	gclient_t *client;

	tag = sortTag(ent);
	trap_Argv(1, client_name, sizeof(client_name));
	clientNum = ClientNumberFromName(ent, client_name, qtrue);
	if (clientNum == -1)
		return;

	trap_Argv(2, ban_duration_arg, sizeof(ban_duration_arg));
	if (!strlen(ban_duration_arg)) {
		CP("print \"Ban duration must be provided.\n\"");
		return;
	}

	client = level.clients + clientNum;

	time = atoi(ban_duration_arg);
	if (time <= 0) {
		CP("print \"Ban duration must be provided.\n\"");
		return;
	}
	time = time > 10080 ? 10080 : time;

	// TempBan player			
	trap_SendConsoleCommand(EXEC_APPEND, va("tempban %i %d\n", clientNum, time));

	// Kick player now
	trap_DropClient(clientNum, va("Temporarily banned by %s\n^7Tempban will expire in ^3%d ^7minutes", tag, time));
	AP(va("chat \"console: %s tempbanned %s ^7for ^3%d ^7minutes^7!\n\"", tag, client->pers.netname, time));

	// Log it
	log = va("Player %s (IP: %s) tempbanned user %s by name for %d minutes.",
		ent->client->pers.netname, clientIP(ent, qtrue), client->pers.netname, time);
	admLog(log);
}

/*
===========
tempBan ip
===========
*/
void cmd_tempBan_ip(gentity_t *ent) {
	char ip_address[ADMIN_ARG_SIZE];
	char ban_duration_arg[ADMIN_ARG_SIZE];
	char *tag, *log;
	int time, i, packed_ip;
	gentity_t *player;
	qboolean banned_client_is_connected;

	banned_client_is_connected = qfalse;

	trap_Argv(1, ip_address, sizeof(ip_address));
	if (!isValidIP(ip_address)) {
		CP("print \"Invalid ip provided.\n\"");
		return;
	}

	trap_Argv(2, ban_duration_arg, sizeof(ban_duration_arg));
	if (!strlen(ban_duration_arg)) {
		CP("print \"Ban duration in minutes must be provided.\n\"");
		return;
	}

	packed_ip = Q_GetPackedIpAddress(ip_address);
	tag = sortTag(ent);

	time = atoi(ban_duration_arg);
	time = time > 10080 ? 10080 : time;

	TempbanIp(ip_address, time);

	for (i = 0; i < level.numNonDisconnectedClients; ++i) {
		player = g_entities + level.sortedClients[i];

		if (player->client->sess.ip == packed_ip) {
			// Kick player now if they are connected
			trap_DropClient(player - g_entities, va("Temporarily banned by %s\n^7Tempban will expire in ^3%d ^7minutes", tag, time));
			AP(va("chat \"console: %s tempbanned %s ^7for ^3%d ^7minutes^7!\n\"", tag, player->client->pers.netname, time));

			// Log it
			log = va("Player %s (IP: %s) tempbanned user %s by IP for %d minutes.",
				ent->client->pers.netname, clientIP(ent, qtrue), player->client->pers.netname, time);
			admLog(log);

			banned_client_is_connected = qtrue;
		}
	}

	if (banned_client_is_connected) {
		return;
	}

	AP(va("chat \"console: %s tempbanned ^3%s ^7for ^3%d ^7minutes^7!\n\"", tag, Q_GetUnpackedIpAddress(packed_ip, qfalse), time));

	// Log it
	log = va("Player %s (IP: %s) tempbanned IP %s for %d minutes.",
		ent->client->pers.netname, clientIP(ent, qtrue), ip_address, time);
	admLog(log);
}

/*
===========
Add IP
===========
*/
void cmd_addIp(gentity_t *ent) {
	char ip_address_arg[ADMIN_ARG_SIZE];
	char *tag, *log, *ip;
	int starcount, subrange = 32;

	trap_Argv(1, ip_address_arg, sizeof(ip_address_arg));
	ip = ip_address_arg;

	tag = sortTag(ent);
	starcount = (strlen(ip) - strlen(Q_StrReplace(ip, "*", "")));

	if (starcount) {  // Banning a range
		if (starcount > 3)
			return;
		if (*strrchr(ip, '*') != *(ip + strlen(ip) - 1))
			return;
		subrange = 8 * (4 - starcount);  // This is multiplying 8 by the amount of numbered octets
		ip = Q_StrReplace(ip, "*", "0"); // Required so that the Banned call can pick up the subnet
	}

	ip = va("%s/%i", ip, subrange);

	if (!isValidIP(ip)) {
		CP("print \"Invalid ip provided.\n\"");
		return;
	}

	trap_SendConsoleCommand(EXEC_APPEND, va("addip %s\n", ip));
	AP(va("chat \"console: %s added ^3%s ^7to the banned file.\n\"", tag, ip_address_arg));

	log = va("Player %s (IP: %s) added %s to the banned file.",
		ent->client->pers.netname, clientIP(ent, qtrue), ip);
	admLog(log);
}

/*
===========
Remove IP
===========
*/
void cmd_removeIp(gentity_t *ent)
{
	char ip_address_arg[ADMIN_ARG_SIZE];
	char *tag, *log, *ip;
	int starcount, subrange = 32;
	FILE* banfile, *banfile2;
	char line[1024];
	qboolean successfulwrite = qfalse;
	qboolean ipfound = qfalse;

	trap_Argv(1, ip_address_arg, sizeof(ip_address_arg));
	ip = ip_address_arg;
	tag = sortTag(ent);
	starcount = (strlen(ip) - strlen(Q_StrReplace(ip, "*", "")));

	if (starcount) {  // removing a range. only supporting star method atm
		if (starcount > 3)
			return;
		if (*strrchr(ip, '*') != *(ip + strlen(ip) - 1))
			return;
		subrange = 8 * (4 - starcount);  // This is multiplying 8 by the amount of numbered octets
		ip = Q_StrReplace(ip, "*", "0"); // Required so that the Banned call can pick up the subnet
	}

	ip = va("%s/%i", ip, subrange);

	if (!isValidIP(ip))
		return;

	banfile = fopen("banned.txt", "r+");
	if (banfile) {
		banfile2 = fopen("banned2.txt", "w+");
		if (banfile2) {
			while (fgets(line, 1024, banfile) != NULL) {
				if (strstr(line, ip))
					ipfound = qtrue;
				else
					fputs(line, banfile2);
			}

			fclose(banfile2);

			if (!ipfound) {
				fclose(banfile);
				remove("banned2.txt");
				CP(va("print \"%s couldn't be found in the banned file^1!\n\"", ip_address_arg));
				return;
			}
			else {
				remove("banned.txt");
				successfulwrite = qtrue;
			}
		}
		fclose(banfile);
	}
	else {
		// Output
		return;
	}

	if (successfulwrite) {
		successfulwrite = qfalse;
		banfile = fopen("banned.txt", "w+");
		if (banfile) {
			banfile2 = fopen("banned2.txt", "r+");
			if (banfile2) {
				while (fgets(line, 1024, banfile2) != NULL)
				{
					fputs(line, banfile);
				}

				fclose(banfile2);
				remove("banned2.txt");
				successfulwrite = qtrue;
			}
			fclose(banfile);
			if (!successfulwrite) {
				// Output
				return;
			}
		}
		else {
			// Output
			return;
		}
	}
	else {
		// Output
		return;
	}

	AP(va("chat \"console: %s removed ^3%s ^7from the banned file.\n\"", tag, ip_address_arg));
	log = va("Player %s (IP: %s) removed %s from the banned file.",
		ent->client->pers.netname, clientIP(ent, qtrue), ip);
	admLog(log);
}

/*
===========
Sorts clans
===========
*/
void cmd_sortClans(gentity_t *ent)
{
	char clan1[ADMIN_ARG_SIZE];
	char clan2[ADMIN_ARG_SIZE];
	int	clanOneClientNumbers[MAX_CLIENTS];
	int	clanTwoClientNumbers[MAX_CLIENTS];
	int	clanOneMatchCount, clanTwoMatchCount, i;

	if (g_gamestate.integer == GS_RESET)
		return;

	trap_Argv(1, clan1, sizeof(clan1));
	trap_Argv(2, clan2, sizeof(clan2));
	if (!Q_stricmp(clan1, "") || !Q_stricmp(clan2, "")) {
		CP("print \"Usage: !sortclans clan1 clan2\n\"");
		return;
	}

	if (!Q_stricmp(clan1, clan2)) {
		CP("print \"Clan names cannot match^1!\n\"");
		return;
	}

	clanOneMatchCount = ClientNumbersFromName(clan1, clanOneClientNumbers);
	clanTwoMatchCount = ClientNumbersFromName(clan2, clanTwoClientNumbers);

	if (!clanOneMatchCount && !clanTwoMatchCount) {
		CP(va("print \"Neither ^3%s ^7nor ^3%s ^7matched any players^1!\n\"", clan1, clan2));
		return;
	}

	if (!clanOneMatchCount) {
		CP(va("print \"Clan ^3%s ^7returned zero matches\n\"", clan1));
		return;
	}
	if (!clanTwoMatchCount) {
		CP(va("print \"Clan ^3%s ^7returned zero matches\n\"", clan2));
		return;
	}

	for (i = 0; i < MAX_CLIENTS; i++) {
		if (!g_entities[i].inuse)
			continue;
		if (level.clients[i].pers.connected != CON_CONNECTED)
			continue;

		level.clients[i].sess.sessionTeam = TEAM_SPECTATOR;
	}

	for (i = 0; i < clanOneMatchCount; i++) {
		level.clients[clanOneClientNumbers[i]].sess.sessionTeam = TEAM_RED;
	}

	for (i = 0; i < clanTwoMatchCount; i++) {
		level.clients[clanTwoClientNumbers[i]].sess.sessionTeam = TEAM_BLUE;
	}

	AP(va("chat \"console: %s sorted clans ^1%s ^7and ^4%s^7.\n\"", sortTag(ent), clan1, clan2));
	trap_Cvar_Set("g_warmup", va("%i", latchedWarmup.integer));
	trap_SendConsoleCommand(EXEC_APPEND, va("map_restart 0 %i\n", GS_WARMUP));

	admLog(va("Player %s (IP: %s) sorted clans %s and %s.",
		ent->client->pers.netname, clientIP(ent, qtrue), clan1, clan2));
}

void cmd_clearDaily(gentity_t *ent)
{
	fileHandle_t df;

	if (!g_dailystats.integer) {
		CP("print \"Daily rankings are currently disabled.\n\"");
		return;
	}

	trap_FS_FOpenFile(DAILY_PATH, &df, FS_READ);
	if (!df) {
		CP("print \"There aren't any rankings to be cleared.\n\"");
		return;
	}

	trap_FS_FCloseFile(df);
	trap_FS_Rename(DAILY_PATH, "");

	AP(va("chat \"console: %s cleared the daily rankings.\n\"", sortTag(ent)));

	admLog(va("Player %s (IP: %s) cleared the daily rankings.",
		ent->client->pers.netname, clientIP(ent, qtrue)));
}

void cmd_forceClan(gentity_t *ent)
{
	char clan[ADMIN_ARG_SIZE];
	char team[ADMIN_ARG_SIZE];
	int	clanClientNumbers[MAX_CLIENTS];
	int clanMatchCount;
	int newTeam;
	int i;
	gclient_t *client;

	trap_Argv(1, clan, sizeof(clan));
	trap_Argv(2, team, sizeof(team));

	clanMatchCount = ClientNumbersFromName(clan, clanClientNumbers);

	if (!clan || !team) {
		CP(va("print \"Usage: !forceclan <clan name> <r/axis/b/allies>\n\""));
		return;
	}

	if (!clanMatchCount) {
		CP(va("print \"There aren't any clients with clan name ^3%s^7.\n\"", clan));
		return;
	}

	if (!Q_stricmp(team, "r") || !Q_stricmp(team, "axis")) {
		newTeam = TEAM_RED;
	}
	else if (!Q_stricmp(team, "b") || !Q_stricmp(team, "allies")) {
		newTeam = TEAM_BLUE;
	}
	else {
		CP(va("print \"^3%s ^7isn't a valid team name. Usage: !forceclan <clan name> <r/axis/b/allies>\n\"", team));
		return;
	}

	for (i = 0; i < level.numPlayingClients; i++) {
		client = level.clients + level.sortedClients[i];

		if (client->pers.connected != CON_CONNECTED)
			continue;

		client->sess.sessionTeam = (newTeam == TEAM_RED) ? TEAM_BLUE : TEAM_RED;
	}

	for (i = 0; i < clanMatchCount; i++) {
		client = level.clients + clanClientNumbers[i];
		client->sess.sessionTeam = newTeam;
	}

	AP(va("chat \"console: %s forced clan %s ^7to the %s ^7team.\n\"", sortTag(ent), clan, TeamNameColored(newTeam)));
	trap_Cvar_Set("g_warmup", va("%i", latchedWarmup.integer));
	trap_SendConsoleCommand(EXEC_APPEND, va("map_restart 0 %i\n", GS_WARMUP));

	admLog(va("Player %s (IP: %s) forced clan %s to %s.",
		ent->client->pers.netname, clientIP(ent, qtrue), clan, team));
}

void cmd_startmatch(gentity_t *ent)
{
	trap_SendConsoleCommand(EXEC_APPEND, "start_match\n");

	admLog(va("Player %s (IP: %s) started the match.",
		ent->client->pers.netname, clientIP(ent, qtrue)));
}

void cmd_demoing(gentity_t *ent)
{
	char *toggleMsg;
	int clientNumber = ent - g_entities;

	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
		CP("print \"Only ^3spectators ^7can secretly demo\n\"");
		return;
	}

	ent->client->sess.secretlyDemoing = !ent->client->sess.secretlyDemoing;

	if (ent->client->sess.secretlyDemoing)
	{
		ent->client->sess.ignored = IGNORE_TEMPORARY;
		ent->client->sess.incognito = qtrue;
		toggleMsg = "on";

		AP(va("print \"%s ^7disconnected\n\"", ent->client->pers.netname));
		CP("print \"You can now ^5secretly demo\n\"");
	}
	else
	{
		ent->client->sess.ignored = IGNORE_OFF;
		toggleMsg = "off";

		char userinfo[MAX_INFO_STRING];

		trap_GetUserinfo(clientNumber, userinfo, sizeof(userinfo));
		Info_SetValueForKey(userinfo, "cl_hidden", "0");
		trap_SetUserinfo(clientNumber, userinfo);

		AP(va("print \"%s ^7connected\n\"", ent->client->pers.netname));
		CP("print \"You can no longer ^5secretly demo\n\"");
	}

	ClientUserinfoChanged(clientNumber);

	admLog(va("Player %s (IP: %s) toggled demoing %s.",
		ent->client->pers.netname, clientIP(ent, qtrue), toggleMsg));
}

void cmd_pause(gentity_t *ent)
{
	gentity_t *target_ent;
	int delay_msec, i;
	
	if (level.paused)
	{
		return;
	}

	delay_msec = trap_Cvar_VariableIntegerValue("g_warmup") * 1000;

	// NOTE(nobo): setting level.paused to true will automatically set every player's pm_type to PM_FREEZE in g_active.c
	level.pause_time = level.time;
	level.warmup_start_time = level.warmupTime - delay_msec;
	level.paused = qtrue;

	// NOTE(nobo): pm_type of PM_FREEZE is enough to keep clients in-place. missiles, however, need some help.
	for (i = MAX_CLIENTS; i < MAX_GENTITIES; ++i)
	{
		target_ent = g_entities + i;

		if (target_ent->inuse)
		{
			// NOTE(nobo): force moving entities in the world to stop in-place.
			if (target_ent->s.eType > TR_INTERPOLATE &&
				target_ent->s.pos.trTime > 0)
			{
				// NOTE(nobo): Store trBase so it can be restored upon unpause of game.
				// trBase is what's used to determine a missile's position in the world, not r.currentOrigin or s.origin
				VectorCopy(target_ent->s.pos.trBase, target_ent->trBase_pre_pause);
				VectorCopy(target_ent->r.currentOrigin, target_ent->s.pos.trBase);
				target_ent->trType_pre_pause = target_ent->s.pos.trType;
				target_ent->s.pos.trType = TR_STATIONARY;
			}
		}
	}

	AP(va("chat \"%s paused the match.\n\"", sortTag(ent)));

	admLog(va("Player %s (IP: %s) paused the match.",
		ent->client->pers.netname, clientIP(ent, qtrue)));
}

void cmd_unpause(gentity_t *ent)
{
	gentity_t *target_ent;
	int i;

	if (!level.paused)
	{
		return;
	}

	for (i = 0; i < MAX_CLIENTS; ++i)
	{
		target_ent = g_entities + i;

		// NOTE(nobo): Unfreeze clients
		if (target_ent->inuse &&
			target_ent->client &&
			target_ent->client->pers.connected == CON_CONNECTED)
		{
			if (target_ent->client->ps.pm_type == PM_FREEZE)
			{
				target_ent->client->ps.pm_type = PM_NORMAL;
			}
		}
	}

	// NOTE(nobo): the timelimit will hit sooner than what the timer shows if we don't increment the startTime server-side.
	level.startTime += level.time_since_pause;
	for (i = MAX_CLIENTS; i < MAX_GENTITIES; ++i)
	{
		target_ent = g_entities + i;

		if (target_ent->inuse)
		{
			// NOTE(nobo): same goes for other time-sensitive functionality; nextthink on entities and trTime on trajectory interoplation.
			if (target_ent->think &&
				target_ent->nextthink > 0)
			{
				target_ent->nextthink += level.time_since_pause;
			}

			if (target_ent->s.eType > TR_INTERPOLATE &&
				target_ent->s.pos.trTime > 0)
			{
				VectorCopy(target_ent->trBase_pre_pause, target_ent->s.pos.trBase);
				target_ent->s.pos.trTime += level.time_since_pause;
				target_ent->s.pos.trType = target_ent->trType_pre_pause;
				target_ent->trType_pre_pause = 0;
			}
		}
	}

	level.time_since_pause = 0;
	level.unpaused_time = 0;
	level.paused = qfalse;

	AP(va("chat \"%s un-paused the match.\n\"", sortTag(ent)));

	admLog(va("Player %s (IP: %s) un-paused the match.",
		ent->client->pers.netname, clientIP(ent, qtrue)));
}