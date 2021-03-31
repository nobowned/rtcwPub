/*/
===========================================================================
L0 / rtcwPub :: g_shared.c

	Game logic shared functionality

Created: 24. Mar / 2014
===========================================================================
*/
#include "g_local.h"

/*
==================
Ported from et: NQ
DecolorString

Remove color characters
==================
*/
void DecolorString(char *in, char *out)
{
	while (*in) {
		if (*in == 27 || *in == '^') {
			in++;		// skip color code
			if (*in) in++;
			continue;
		}
		*out++ = *in++;
	}
	*out = 0;
}

// Nobo -
// Turns out non-alphanumeric chars don't register as color, (NATE'S CLIENT ONLY!!!)
// so no need to remove them.
void DecolorString2(char *in, char *out)
{
	while (*in) {
		if (*in == '^') {
			if (*(in + 1) && (isdigit(*(in + 1)) || isalpha(*(in + 1)))) {
				in += 2;
				continue;
			}
		}
		*out++ = *in++;
	}
	*out = 0;
}

/*
==================
Time

Returns current time.
==================
*/
const char *months[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
// Returns current time % date
char *getDateTime( void ) {
	qtime_t		ct;
	trap_RealTime(&ct);
	
	return va("%s %02d %d %02d:%02d:%02d",
		months[ct.tm_mon], ct.tm_mday, getYearFromCYear(ct.tm_year), ct.tm_hour, ct.tm_min, ct.tm_sec);
}

// Returns current date
char *getDate( void ) {
	qtime_t		ct;
	trap_RealTime(&ct);

	return va("%02d/%s/%d",	ct.tm_mday,	months[ct.tm_mon], getYearFromCYear(ct.tm_year));
}

// returns month string abbreviation (i.e. Jun)
const char *getMonthString(int monthIndex) {
	if (monthIndex < 0 || monthIndex >= ArrayLength(months)) {
		return "InvalidMonth";
	}

	return months[monthIndex];
}

// returns current year
int getYearFromCYear(int cYear) {
	return 1900 + cYear;
}

// returns the last day for that month.
int getDaysInMonth(int monthIndex) {
	switch (monthIndex) {
	case 1:  // Feb
		return 28;
	case 3:  // Apr
	case 5:  // Jun
	case 8:  // Sep
	case 10: // Nov
		return 30;
	default: // Jan, Mar, May, Jul, Aug, Oct, Dec
		return 31;
	}
}

/*
=========
ValidIP

Works with regular IPv4 (without port) and IPv4 with subnet
=========
*/
qboolean isValidIP(const char *ip) {
	unsigned int b1, b2, b3, b4, subnet = 8;
	int rc;

	rc = sscanf(ip, "%3u.%3u.%3u.%3u/%2u", &b1, &b2, &b3, &b4, &subnet);
	if (rc < 4 || rc > 5)
		return qfalse;
	if ((b1 | b2 | b3 | b4) > 255 || (subnet > 32 || subnet < 8) || (subnet % 8 != 0))
		return qfalse;
	if (strspn(ip, "0123456789./") < strlen(ip))
		return qfalse;
	return qtrue;
}

/*
===========
Global sound
===========
*/
void APSound(char *sound) {
	gentity_t *te;

	te = G_TempEntity(vec3_origin, EV_GLOBAL_SOUND);
	te->s.eventParm = G_SoundIndex(sound);
	te->r.svFlags |= SVF_BROADCAST;
}

/*
===========
Client sound
===========
*/
void CPSound(gentity_t *ent, char *sound) {
	gentity_t *te;

	te = G_TempEntity(ent->s.pos.trBase, EV_GLOBAL_CLIENT_SOUND);
	te->s.eventParm = G_SoundIndex(sound);
	te->s.teamNum = ent->s.clientNum;
}

/*
===========
Global sound with limited range
===========
*/
void APRSound(gentity_t *ent, char *sound) {
	gentity_t   *te;

	te = G_TempEntity(ent->r.currentOrigin, EV_GENERAL_SOUND);
	te->s.eventParm = G_SoundIndex(sound);
}

/*
==================
L0 - Splits string into tokens
==================
*/
void Q_Tokenize(char *str, char **splitstr, char *delim) {
	char *p;
	int i = 0;

	p = strtok(str, delim);
	while (p != NULL)
	{
		splitstr[i] = G_Alloc(strlen(p) + 1);

		if (splitstr[i])
			strcpy(splitstr[i], p);
		i++;

		p = strtok(NULL, delim);
	}
}

/*
==================
Parse Strings

From S4NDMoD
==================
*/
void ParseStr(const char *strInput, char *strCmd, char *strArgs)
{
	int i = 0, j = 0;
	int foundspace = 0;

	while (strInput[i] != 0) {
		if (!foundspace) {
			if (strInput[i] == ' ') {
				foundspace = 1;
				strCmd[i] = 0;
			}
			else
				strCmd[i] = strInput[i];
			i++;
		}
		else {
			strArgs[j++] = strInput[i++];
		}
	}
	if (!foundspace)
		strCmd[i] = 0;

	strArgs[j] = 0;
}

/*
===================
Nobo - ParseStr2

Improved ParseStr that's far more efficient memory-wise
start will become the end position

NOTE: data[] will be manipulated.. Create temporary data c-string if this is a concern.
===================
*/

void ParseStr2(char data[], char **out, int *start)
{
	char *d = data + *start;
	char *i = d;

	if (!*d && *(d + 1))
		d++;

	while (*d <= ' ') {
		if (!*d)
			 return;
		d++;
	}

	*out = d;
	unsigned char cmp = (unsigned char)255;

	while (*d >= '!' && (unsigned char)*d <= cmp)
		d++;

	*d = 0;

	*start = *start + d - i;
}

char *CleanFileText(char *text)
{
	return Q_ChrReplace(text, INVALID_CHAR, INVALID_CHAR_REPLACEMENT);
}

char *ParseFileText(char *text)
{
	return Q_ChrReplace(text, INVALID_CHAR_REPLACEMENT, INVALID_CHAR);
}

// NOTE: This was made to accomodate the rtcwMP client. Symbols can't be used as colors.
void RemoveColorImpurities(char *name)
{
	while (*name) {
		if (*name == '^') {
			if (*(name + 1) && !Q_IsColorString2(name + 1)) {
				memmove(name, name + 2, strlen(name + 2) + 1);
				continue;
			}
		}
		name++;
	}

	if (*(name - 1) == '^')
		*(name - 1) = 0;
}

char *TablePrintableColorName(const char *name, int maxlength)
{
	char dirty[MAX_NETNAME];
	char clean[MAX_NETNAME];
	char spaces[MAX_NETNAME] = "";
	int cleanlen;

	Q_strncpyz(dirty, name, sizeof(dirty));

	DecolorString(dirty, clean);

	cleanlen = strlen(clean);

	if (cleanlen > maxlength) {
		int remove = cleanlen - maxlength;
		char *end = dirty + strlen(dirty) - 1;

		while (*end && *(end - 1) && remove) {
			if (*(end - 1) == Q_COLOR_ESCAPE)
				end--;
			else
				remove--;

			end--;
		}

		*++end = 0;
	}
	else if (cleanlen < maxlength) {
		for (; cleanlen < maxlength; cleanlen++)
			strcat(spaces, " ");
	}

	return va("%s%s", dirty, spaces);
}

void Q_RemoveLocalization(char *message)
{
	int i;
	char *ptr;

	char *strings[] = { "[lon]", "[lof]" };

	for (i = 0; i < ArrayLength(strings); i++) {
		ptr = message;

		while ((ptr = strstr(ptr, strings[i])) != NULL)
			memmove(ptr, ptr + strlen(strings[i]), strlen(ptr) - 1);
	}
}

/*
==================
FindFreePosition
==================
*/
qboolean FindFreePosition(gentity_t *ent, vec3_t free_position, int step_count) {
	vec3_t pos, testdir, floor;
	trace_t tr;
	int i, x, y, scale, mult;

	if (step_count <= 1)
	{
		step_count = 2;
	}

	mult = ent->r.maxs[0] + abs(ent->r.mins[0]);

	for (i = 1; i < step_count; ++i)
	{
		scale = mult * i;
		for (x = -1; x < 2; ++x)
		{
			for (y = -1; y < 2; ++y)
			{
				VectorSet(testdir, x, y, 0);
				if (VectorCompare(testdir, vec3_origin))
				{
					continue;
				}
				VectorMA(ent->r.currentOrigin, scale, testdir, pos);
				trap_Trace(&tr, pos, ent->r.mins, ent->r.maxs, pos, ent->s.number, MASK_PLAYERSOLID);
				if (tr.fraction == 1.0f)
				{
					VectorCopy(pos, floor);
					floor[2] += ent->r.mins[2] - 1;
					trap_Trace(&tr, pos, ent->r.mins, ent->r.maxs, floor, ent->s.number, MASK_PLAYERSOLID);
					if (tr.fraction < 1.0f && tr.contents != 0 && !VectorCompare(tr.plane.normal, vec3_origin))
					{
						VectorCopy(pos, free_position);
						return qtrue;
					}
				}
			}
		}
	}

	return qfalse;
}

void GetFileExtension(const char *filename, char *out)
{
	qboolean at_extension;

	at_extension = qfalse;

	while (*filename)
	{
		if (*filename == '.')
		{
			at_extension = qtrue;
		}

		if (at_extension)
		{
			*out++ = *filename;
		}

		filename++;
	}

	*out = 0;
}

qboolean FileExists(char *filename, char *directory, char *expected_extension, qboolean can_have_extension)
{
	char files[MAX_CONFIGSTRINGS];
	char file_exists[MAX_QPATH];
	char *fs_filename, *fs_filepath;
	char file_extension[10];
	int i, filecount;

	GetFileExtension(filename, file_extension);
	if (file_extension[0])
	{
		if (Q_stricmp(file_extension, expected_extension))
		{
			return qfalse;
		}

		Q_strncpyz(file_exists, filename, sizeof(file_exists));

		if (!can_have_extension)
		{
			*strstr(filename, file_extension) = 0;
		}
	}
	else
	{
		Q_strncpyz(file_exists, filename, sizeof(file_exists));
		Q_strcat(file_exists, sizeof(file_exists), expected_extension);
	}

	filecount = trap_FS_GetFileList(directory, expected_extension, files, sizeof(files));
	fs_filepath = files;

	for (i = 0; i < filecount; ++i)
	{
		fs_filename = COM_SkipPath(fs_filepath);

		if (!Q_stricmp(file_exists, fs_filename))
		{
			return qtrue;
		}

		fs_filepath += strlen(fs_filepath) + 1;
	}

	return qfalse;
}

/*
================
LaunchEntity

Spawns an entity and tosses it forward
================
*/
gentity_t *LaunchEntity(gentity_t *ent, vec3_t origin, vec3_t velocity, int ownerNum)
{
	trace_t		tr;
	vec3_t		vec, temp;
	int			i;

	ent->s.eType = ET_GENERAL;
	ent->physicsObject = qtrue;
	ent->r.contents = CONTENTS_TRIGGER | CONTENTS_ITEM;
	ent->clipmask = CONTENTS_SOLID | CONTENTS_MISSILECLIP;

	trap_Trace(&tr, origin, ent->r.mins, ent->r.maxs, origin, ownerNum, MASK_SOLID);
	if (tr.startsolid)
	{
		VectorSubtract(g_entities[ownerNum].s.origin, origin, temp);
		VectorNormalize(temp);

		for (i = 16; i <= 48; i += 16)
		{
			VectorScale(temp, i, vec);
			VectorAdd(origin, vec, origin);

			trap_Trace(&tr, origin, ent->r.mins, ent->r.maxs, origin, ownerNum, MASK_SOLID);
			if (!tr.startsolid)
			{
				break;
			}
		}
	}

	G_SetOrigin(ent, origin);
	ent->s.pos.trType = TR_GRAVITY;
	ent->s.pos.trTime = level.time;
	VectorCopy(velocity, ent->s.pos.trDelta);

	ent->s.eFlags |= EF_BOUNCE_HALF;

	ent->think = G_FreeEntity;

	return ent;
}

/*
================
GetHostname

returns host name without control characters (if any)
================
*/
char *GetHostname(void)
{
	static char hostname[MAX_CVAR_VALUE_STRING] = { 0 };
	static int lastModificationCount = 0;
	int i, j;

	if (sv_hostname.modificationCount > lastModificationCount ||
		!hostname[0])
	{
		for (i = 0, j = 0; i < strlen(sv_hostname.string); ++i)
		{
			if (sv_hostname.string[i] < 32)
			{
				continue;
			}

			hostname[j++] = sv_hostname.string[i];
		}

		lastModificationCount = sv_hostname.modificationCount;
	}

	return hostname;
}

#define IS_REVIVABLE (traceEnt && traceEnt != ent && traceEnt->client && traceEnt->client->ps.pm_type == PM_DEAD)
#define MAX_BOX_SIZE 10
#define REVIVE_MAX_DIST 48

gentity_t *FindRevivableTeammate(gentity_t *ent, int skip_num, vec3_t dir, vec3_t initial_start, vec3_t new_start, vec3_t end)
{
	int hitCount, i;
	int hitEnts[MAX_BOX_SIZE];
	vec3_t range = { 5, 5, 1 };
	vec3_t mins, maxs;
	trace_t tr;
	gentity_t *traceEnt;
	int previous_skip_num = skip_num;

	// trace through dead enemies until a friendly corpse is hit
	while (1)
	{
		trap_Trace(&tr, new_start, NULL, NULL, end, skip_num, MASK_SHOT);

		if (tr.fraction == 1.0)
		{
			return NULL;
		}

		if (Distance(initial_start, tr.endpos) > REVIVE_MAX_DIST)
		{
			return NULL;
		}

		if (tr.entityNum == previous_skip_num) // don't want to bounce back and forth between two enemies
		{
			VectorMA(new_start, 2, dir, new_start);

			if (Distance(initial_start, new_start) > REVIVE_MAX_DIST)
			{
				return NULL;
			}

			previous_skip_num = skip_num;
			skip_num = tr.entityNum;
			continue;
		}

		if (tr.entityNum < MAX_CLIENTS)  // hit a client, continue trace if not teammate 
		{
			traceEnt = g_entities + tr.entityNum;
			if (IS_REVIVABLE)
			{
				if (traceEnt->client->sess.sessionTeam == ent->client->sess.sessionTeam)
				{
					return traceEnt;
				}
				else
				{
					VectorCopy(tr.endpos, new_start);
					previous_skip_num = skip_num;
					skip_num = tr.entityNum;
				}
			}
			else
			{
				break;
			}
		}
		else // once non-client is hit, then do box trace.
		{
			VectorAdd(tr.endpos, range, maxs);
			VectorSubtract(tr.endpos, range, mins);

			hitCount = trap_EntitiesInBox(mins, maxs, hitEnts, MAX_BOX_SIZE);

			for (i = 0; i < hitCount; ++i)
			{
				traceEnt = g_entities + hitEnts[i];
				if (IS_REVIVABLE && traceEnt->client->sess.sessionTeam == ent->client->sess.sessionTeam)
				{
					return traceEnt;
				}
			}

			break;
		}
	}

	return NULL;
}

void G_SetConstantLight(gentity_t *ent, int red, int green, int blue, int intensity)
{
	ent->s.constantLight = red | (green << 8) | (blue << 16) | (intensity << 24);
}

char *GetTimeMessage(int seconds) {
	int minutes = seconds / 60;
	if (minutes < 0) {
		minutes = 0;
	}
	int hours = minutes / 60;
	minutes = minutes % 60;

	char *hoursMsg = "";
	if (hours > 0) {
		hoursMsg = va("%d %s ", hours, (hours == 1) ? "hour" : "hours");
	}

	char *minutesMsg = "";
	if (minutes > 0) {
		minutesMsg = va("%d %s", minutes, (minutes == 1) ? "minute" : "minutes");
	}

	if (!hours && !minutes) {
		hoursMsg = "a moment";
	}

	return va("%s%s", hoursMsg, minutesMsg);
}