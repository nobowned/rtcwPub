/*/
===========================================================================
L0 / rtcwPub :: g_admin.c

	Main Admin functionality.

Created: 24. Mar / 2014
===========================================================================
*/
#include "g_admin.h"

/*
===========
Admin Log

Just a wrapper with syntax fixed..
===========
*/
void admLog(char *info) {
	
	if (!g_extendedLog.integer)
		return;

	trap_LogToFile(ADMACT, va("Time: %s\n%s%s", getDateTime(), info, LOGLINE));
}
// Admin Activity


/*
===========
Sort tag
===========
*/
char *sortTag(gentity_t *ent) {
	static char tag[MAX_CVAR_VALUE_STRING];
	gclient_t *client = ent->client;

	if (client->sess.incognito)
		SanitizeString(getTag(ent), tag, qfalse);
	else
		Q_strncpyz(tag, client->pers.netname, sizeof(tag));

	Q_strcat(tag, sizeof(tag), "^7");

	return tag;
}

char *getTag(gentity_t *ent) {
	gclient_t *client = ent->client;
	char *tag;

	if (client->sess.admin == ADM_1)
		tag = a1_tag.string;
	else if (client->sess.admin == ADM_2)
		tag = a2_tag.string;
	else if (client->sess.admin == ADM_3)
		tag = a3_tag.string;
	else if (client->sess.admin == ADM_4)
		tag = a4_tag.string;
	else if (client->sess.admin == ADM_5)
		tag = a5_tag.string;
	else
		tag = "";

	return va("%s", tag);
}

char *getCommandList(gentity_t *ent) {
	gclient_t *client = ent->client;
	char *list;

	if (client->sess.admin == ADM_1)
		list = a1_cmds.string;
	else if (client->sess.admin == ADM_2)
		list = a2_cmds.string;
	else if (client->sess.admin == ADM_3)
		list = a3_cmds.string;
	else if (client->sess.admin == ADM_4)
		list = a4_cmds.string;
	else if (client->sess.admin == ADM_5)
		list = a5_cmds.string;
	else
		list = "";

	return va("%s", list);
}

char *getMessage(gentity_t *ent) {
	gclient_t *client = ent->client;
	char *message;

	if (client->sess.admin == ADM_1)
		message = a1_message.string;
	else if (client->sess.admin == ADM_2)
		message = a2_message.string;
	else if (client->sess.admin == ADM_3)
		message = a3_message.string;
	else if (client->sess.admin == ADM_4)
		message = a4_message.string;
	else if (client->sess.admin == ADM_5)
		message = a5_message.string;
	else
		message = "";

	if (!strlen(message)) {
		return va(" ^7logged in as %s^7!", getTag(ent));
	}

	return va("%s", message);
}

/*
===========
Get client numbers from name
===========
*/
int ClientNumbersFromName(char *name, int *matches) {
	int i, textLen;
	char nm[MAX_NETNAME];
	char c;
	int index = 0;

	Q_strncpyz(nm, name, sizeof(nm));
	Q_CleanStr(nm);
	textLen = strlen(nm);
	c = *nm;

	for (i = 0; i < level.maxclients; i++)
	{
		int j, len;
		char playerName[MAX_NETNAME];

		if ((!g_entities[i].client) || (g_entities[i].client->pers.connected != CON_CONNECTED))
			continue;

		Q_strncpyz(playerName, g_entities[i].client->pers.netname, sizeof(playerName));
		Q_CleanStr(playerName);
		len = strlen(playerName);

		for (j = 0; j < len; j++)
		{
			if (tolower(c) == tolower(playerName[j]))
			{
				if (!Q_stricmpn(nm, playerName + j, textLen))
				{
					matches[index] = i;
					index++;
					break;
				}
			}
		}
	}
	return index;
}

/*
===========
Get client number from name
===========
*/
int ClientNumberFromName(gentity_t *ent, char *name, qboolean send_error_message) {
	int i, textLen;
	char nm[MAX_NETNAME];
	char c;
	int count = 0;
	int matches[MAX_CLIENTS];

	Q_strncpyz(nm, name, sizeof(nm));
	Q_CleanStr(nm);
	textLen = strlen(nm);
	c = *nm;

	for (i = 0; i < level.maxclients; i++)
	{
		int j, len;
		char playerName[MAX_NETNAME];

		if ((!g_entities[i].client) || (g_entities[i].client->pers.connected != CON_CONNECTED))
			continue;

		Q_strncpyz(playerName, g_entities[i].client->pers.netname, sizeof(playerName));
		Q_CleanStr(playerName);
		len = strlen(playerName);

		for (j = 0; j < len; j++)
		{
			if (tolower(c) == tolower(playerName[j]))
			{
				if (!Q_stricmpn(nm, playerName + j, textLen))
				{
					matches[count] = i;
					count++;
					break;
				}
			}
		}
	}

	if (count == 0) {
		if (send_error_message) {
			CP(va("print \"There aren't any clients with ^1%s ^7in their name^1!\n\"", name));
		}
		return -1;
	}
	else if (count > 1) {
		if (send_error_message) {
			CP(va("print \"Too many clients with ^1%s ^7in their name^1!\n\"", name));
		}
		return -1;
	}

	return matches[0];
}

gentity_t *GetClientEntity(gentity_t *ent, char *cNum, gentity_t **found)
{
	int clientNum, i;
	qboolean allZeroes = qtrue;
	gentity_t *match;
	*found = NULL;

	for (i = 0; i < strlen(cNum); ++i)
	{
		if (cNum[i] != '0')
		{
			allZeroes = qfalse;
			break;
		}
	}

	if (allZeroes)
	{
		clientNum = 0;
	}
	else
	{
		clientNum = atoi(cNum);
		if (clientNum <= 0 || clientNum >= level.maxclients)
		{
			CP(va("print \"Invalid client number provided: ^3%s\n\"", cNum));
			return *found;
		}
	}

	match = g_entities + clientNum;
	if (!match->inuse || match->client->pers.connected != CON_CONNECTED)
	{
		CP(va("print \"No connected client with client number: ^3%i\n\"", clientNum));
		return *found;
	}

	*found = match;
	return *found;
}

/*
===========
Admin help
===========
*/
typedef struct {
	char *command;
	void(*execute) (gentity_t *ent);
	char *help;
	char *usage;
	char *alt_commands;
	qboolean any_admin_can_use;
} admin_cmd_t;

static const admin_cmd_t admin_cmd_list[] = {
	{ "pause", cmd_pause, "Pauses the match.", "!pause" },
	{ "unpause", cmd_unpause, "Un-pauses the match", "!unpause" },
	{ "help", cmd_help, "Shows how to use a command", "!help <command>", "", qtrue },
	{ "commands", cmd_listCmds, "Shows all available commands for your level.", "!commands", "cmds", qtrue },
	{ "ignore", cmd_ignore, "Takes player's ability to chat, use vsay or callvotes.", "!ignore <unique part of name>", "i" },
	{ "unignore", cmd_unignore, "Restores player's ability to chat, use vsay or call votes.", "!unignore <unique part of name>", "ui" },
	{ "clientignore", cmd_clientIgnore, "Takes player's ability to chat, callvotes or use vsay.", "!clientignore <client slot>", },
	{ "clientunignore", cmd_clientUnignore, "Restores player's ability to chat, callvotes or use vsay.", "!clientunignore <client slot>" },
	{ "kick", cmd_kick, "Kicks player from server.", "!kick <unique part of name> <message>", "k" },
	{ "clientkick", cmd_clientkick, "Kicks player from server.", "!clientkick <client slot> <optional message>" },
	{ "slap", cmd_slap, "Slaps player and takes 20hp.", "!slap <unique part of name>" },
	{ "kill", cmd_kill, "Kills player on spot.", "!kill <unique part of name>" },
	{ "lock", cmd_lockteam, "Locks the team(s) so players can't join.", "!lock <red/axis/r/blue/allied/b/both>" },
	{ "unlock", cmd_unlockteam, "Unlocks the team(s) so players can join.", "!unlock <red/axis/r/blue/allied/b/both>" },
	{ "specs", cmd_specs, "Forces player to spectators.", "!s <unique part of name>", "s spec spectator spectators" },
	{ "axis", cmd_axis, "Forces player to Axis team.", "!r <unique part of name>", "ax r red" },
	{ "allies", cmd_allied, "Forces player to Allied team.", "!b <unique part of name>", "al b blue allied" },
	{ "exec", cmd_exec, "Executes server config file.", "You can use @ at the end to silently execute file, e.g. !exec server @" },
	{ "nextmap", cmd_nextmap, "Loads the nextmap.", NULL },
	{ "map", cmd_map, "Loads the map of your choice.", "!map mp_base" },
	{ "cpa", cmd_cpa, "Center Prints Admin message to everyone.", "!cpa <msg>" },
	{ "cp", cmd_cp, "Center Prints Admin message to selected user.", "!cp <client slot> <message>" },
	{ "chat", cmd_chat, "Shows warning message to all in global chat.", "!chat <msg>" },
	{ "warn", cmd_warn, "Shows warning message to all in global chat and center print.", "!warn <msg>" },
	{ "cancelvote", cmd_cancelvote, "Cancels any vote in progress.", NULL },
	{ "passvote", cmd_passvote, "Passes any vote in progress.", NULL },
	{ "map_restart", cmd_map_restart, "If round in progress will restart. If warmup will start the match", "!map_restart <time>", "mr" },
	{ "reset_match", cmd_resetmatch, "Resets the match.", NULL, "rm" },
	{ "swap_teams", cmd_swap, "Swaps the teams.", NULL, "st" },
	{ "shuffle", cmd_shuffle, "Shuffles the teams.", NULL },
	{ "spec999", cmd_specs999, "Moves all lagged (999) players to spectators.", NULL },
	{ "whereis", cmd_revealCamper, "Reveals players location to all.", "!whereis <client slot>" },
	{ "ban", cmd_ban, "Bans player.", "!ban <unique part of name>" },
	{ "clientban", cmd_banClient, "Bans player.", "!clientban <client number>", "banclient" },
	{ "rangeban", cmd_rangeBan, "Range Bans player.", "!rangeban <unique part of name> <8/16/24/32>" },
	{ "tempban", cmd_tempBan, "Temporarily Bans player.", "!tempban <unique part of name> <minutes>" },
	{ "tempbanip", cmd_tempBan_ip, "Temporarily Bans player.", "!tempban <ip address> <minutes>" },
	{ "addip", cmd_addIp, "Adds IP to banned file. You can also use subranges.", "!addip 100.100.100.100 or !addip 100.100.*.*" },
	{ "removeip", cmd_removeIp, "Removes IP from banned file. You can also use subranges.", "!removeip 100.100.100.100 or !removeip 100.100.*.*" },
	{ "rename", cmd_rename, "Renames players.", "!rename <client slot> <new name>" },
	{ "vstr", cmd_vstr, "Loads a level from rotation file. Note - You need to know rotation labels..", "!vstr map1" },
	{ "renameon", cmd_renameon, "Restores ability to rename from client.", "!renameon <client number>" },
	{ "renameoff", cmd_renameoff, "Revokes ability to rename from client (lasts only that round).", "!renameoff <client number>" },
	{ "forceteam", cmd_forceteam, "Forces player to specified team.", "!forceteam <unique part of name> <axis/allies/spec>", "ft" },
	{ "forceclient", cmd_forceteam_client, "Forces player to specified team using client number.", "!forceclient <client number> <axis/allies/spec>" },
	{ "sortclans", cmd_sortClans, "Sorts the two clans passed in.", "!sortclans <clan 1> <clan 2>", "sc" },
	{ "cleardaily", cmd_clearDaily, "Clears the daily rankings", "!cleardaily", "clear_daily" },
	{ "forceclan", cmd_forceClan, "Forces clan to the given team", "!forceclan <clan name> <r/axis/b/allies>", "fc" },
	{ "start_match", cmd_startmatch, "Starts the match", "!start_match", "sm" },
	{ "demoing", cmd_demoing, "Makes it so you can demo anonymously", "!demoing", "demo", qtrue },

	// This must be last
	{ NULL, NULL, NULL, NULL }
};

/*
===========
List commands
===========
*/
void cmd_listCmds(gentity_t *ent) {
	char *list = getCommandList(ent);
	const admin_cmd_t *cmd_option;

	list = va("%s\n\n^3Commands any admin level can use:^7\n", list);
	for (cmd_option = admin_cmd_list; cmd_option->command != NULL; cmd_option++) {
		if (cmd_option->any_admin_can_use) {
			strcat(list, va("%s ", cmd_option->command));
		}
	}

	if (ent->client->sess.admin == 5 && a5_allowAll.integer) {
		list = va("%s\n\n^5Additional server commands allowed for level 5 admins:^7\n", list);

		for (cmd_option = admin_cmd_list; cmd_option->command != NULL; cmd_option++) {
			strcat(list, va("%s ", cmd_option->command));
		}
	}

	CP(va("print \"^3Commands available at your admin level:^7\n%s\n\n^3Use !help <command> for help with using a command.\n\"", list));

	return;
}

/*
===========
Admin commands
===========
*/
qboolean do_cmds(gentity_t *ent, char *text) {
	const admin_cmd_t *cmd_option;
	char cmd[128];

	if (!strlen(text) || text[0] != '!') {
		return qfalse;
	}

	// Tokenize then get cmd and args from the tokenized string.
	trap_TokenizeString(text);
	trap_Argv(0, cmd, sizeof(cmd));
	memmove(cmd, cmd + 1, sizeof(cmd) - 1);

	for (cmd_option = admin_cmd_list; cmd_option->command != NULL && cmd_option->execute != NULL; cmd_option++) {
		if (!Q_stricmp(cmd, cmd_option->command) || Q_FindToken(cmd_option->alt_commands, cmd)) {
			if (Q_FindToken(getCommandList(ent), cmd_option->command) || (ent->client->sess.admin == ADM_5 && a5_allowAll.integer) || cmd_option->any_admin_can_use) {
				cmd_option->execute(ent);
			}
			else {
				CP(va("print \"Command ^1%s ^7is not allowed as your level^1!\n\"", cmd));
			}

			return qtrue;
		}
	}

	if (Q_FindToken(getCommandList(ent), cmd)) {
		cmd_custom(ent, cmd);
	}
	else { 
		CP(va("print \"Command ^1%s ^7was not found^1!\n^7Use ^3!commands ^7for a list of available commands.\n\"", cmd));
	}

	return qtrue;
}

/*
===========
Help an admin out!
===========
*/
void cmd_help(gentity_t *ent) {
	char cmd[ADMIN_ARG_SIZE];
	const admin_cmd_t *cmd_option;

	trap_Argv(1, cmd, sizeof(cmd));

	if (!strlen(cmd)) {
		CP("print \"You didn't give a command to search for. Usage: !help <command>\n\"");
		return;
	}

	for (cmd_option = admin_cmd_list; cmd_option->command != NULL && cmd_option->execute != NULL; cmd_option++) {
		if (!Q_stricmp(cmd, cmd_option->command) || Q_FindToken(cmd_option->alt_commands, cmd)) {
			CP(va("print \"^3%s %s %s %s\n\"",
				va(cmd_option->usage ? "Help ^7:" : "Help^7:"),
				cmd_option->help,
				va("%s", (cmd_option->usage ? va("\n^3Usage^7: %s", cmd_option->usage) : "")),
				va("%s", (cmd_option->alt_commands ? va("\n^3Alternates^7: %s\n", Q_StrReplace(cmd_option->alt_commands, " ", ", ")) : ""))));
			return;
		}
	}

	CP(va("print \"Couldn't find any help docs for ^1%s^7!\n\"", cmd));
}
