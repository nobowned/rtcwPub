/*/
===========================================================================
L0 / rtcwPub :: g_admin.h

	Admin Prototypes

Created: 24. Mar / 2014
===========================================================================
*/
#ifndef __ADMIN_H
#define __ADMIN_H

#include "g_local.h"

#define ADMIN_ARG_SIZE 128

// g_cmds.c
void Cmd_ListCommands_f(gentity_t *ent);
void Cmd_HelpDocs_f(gentity_t *ent);

//
// g_admin.c
//
void admLog(char *info);
char *sortTag(gentity_t *ent);
char *getTag(gentity_t *ent);
char *getCommandList(gentity_t *ent);
char *getMessage(gentity_t *ent);
qboolean do_cmds(gentity_t *ent, char *text);
void cmd_help(gentity_t *ent);
int ClientNumbersFromName(char *name, int *matches);
int ClientNumberFromName(gentity_t *ent, char *name, qboolean send_error_message);
gentity_t *GetClientEntity(gentity_t *ent, char *cNum, gentity_t **found);

//
// g_admin_cmds.c
//
void cmd_custom(gentity_t *ent, char *cmd);
void cmd_ignore(gentity_t *ent);
void cmd_unignore(gentity_t *ent);
void cmd_clientIgnore(gentity_t *ent);
void cmd_clientUnignore(gentity_t *ent);
void cmd_kick(gentity_t *ent);
void cmd_clientkick(gentity_t *ent);
void cmd_slap(gentity_t *ent);
void cmd_kill(gentity_t *ent);
void cmd_specs(gentity_t *ent);
void cmd_axis(gentity_t *ent);
void cmd_allied(gentity_t *ent);
void cmd_exec(gentity_t *ent);
void cmd_nextmap(gentity_t *ent);
void cmd_map(gentity_t *ent);
void cmd_vstr(gentity_t *ent);
void cmd_cpa(gentity_t *ent);
void cmd_cp(gentity_t *ent);
void cmd_warn(gentity_t *ent);
void cmd_chat(gentity_t *ent);
void cmd_cancelvote(gentity_t *ent);
void cmd_passvote(gentity_t *ent);
void cmd_map_restart(gentity_t *ent);
void cmd_resetmatch(gentity_t *ent);
void cmd_swap(gentity_t *ent);
void cmd_shuffle(gentity_t *ent);
void cmd_specs999(gentity_t *ent);
void cmd_revealCamper(gentity_t *ent);
void cmd_rename(gentity_t *ent);
void cmd_renameon(gentity_t *ent);
void cmd_renameoff(gentity_t *ent);
void cmd_lockteam(gentity_t *ent);
void cmd_unlockteam(gentity_t *ent);
void cmd_ban(gentity_t *ent);
void cmd_banClient(gentity_t *ent);
void cmd_rangeBan(gentity_t *ent);
void cmd_tempBan(gentity_t *ent);
void cmd_tempBan_ip(gentity_t *ent);
void cmd_addIp(gentity_t *ent);
void cmd_forceteam(gentity_t *ent);
void cmd_forceteam_client(gentity_t *ent);
void cmd_sortClans(gentity_t *ent);
void cmd_removeIp(gentity_t *ent);
void cmd_clearDaily(gentity_t *ent);
void cmd_listCmds(gentity_t *ent);
void cmd_forceClan(gentity_t *ent);
void cmd_startmatch(gentity_t *ent);
void cmd_demoing(gentity_t *ent);
void cmd_pause(gentity_t *ent);
void cmd_unpause(gentity_t *ent);
void cmd_listcountries(gentity_t *ent);

#endif // __ADMIN_H
