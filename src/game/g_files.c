/*/
===========================================================================
L0 / rtcwPub :: g_files.c

	Deals with File handling

Created: 24. Mar / 2014
===========================================================================
*/
#include "g_admin.h"

#define BANNED_FILE_PATH "banned.txt"
#define TEMPBANNED_FILE_PATH "tempbanned.txt"

/*
===========
Add IP to banned file
===========
*/
void BanClient(char arg[MAX_TOKEN_CHARS]) {
	FILE		*bannedfile;

	bannedfile = fopen(BANNED_FILE_PATH, "a+");
	if (bannedfile)
	{
		fputs(va("%s\n", arg), bannedfile);
		fclose(bannedfile);
	}
	else 
	{
		G_LogPrintf(BANNED_FILE_PATH " could not be opened. Aborting ban of IP %s\n", arg);
	}
}

/*
===========
IsBanned (file check version)
===========
*/
qboolean IsBanned(char *ip, char *password)
{
#define IP_OCTETS 4

	int linenumber = 0;
	char line[MAX_STRING_CHARS];
	int clientIP[IP_OCTETS];
	int match[IP_OCTETS];
	qboolean banned = qfalse;
	qboolean badip;
	int subrange;
	int matchcount;
	//char data[MAX_STRING_TOKENS];
	FILE* banfile;
	int i;

	banfile = fopen(BANNED_FILE_PATH, "r");
	if (banfile)
	{
		matchcount = sscanf(ip, "%i.%i.%i.%i", &clientIP[0], &clientIP[1], &clientIP[2], &clientIP[3]);

		if (matchcount != 4)
		{
			G_LogPrintf("Banned: Bad IP provided: %s", ip);
			return qfalse;
		}

		while (fgets(line, MAX_STRING_CHARS, banfile) != NULL)
		{
			memset(match, 0, sizeof(match));
			subrange = 0;
			banned = qfalse;
			badip = qfalse;
			++linenumber;

			// Here for later on so I can tackle bypasses and banned reasons later..
			//sscanf(line, "%3u.%3u.%3u.%3u/%2u|%[^\n]", &match[0], &match[1], &match[2], &match[3], &match[4], &data);
			matchcount = sscanf(line, "%i.%i.%i.%i/%i", &match[0], &match[1], &match[2], &match[3], &subrange);

			// Some (really basic) sanity checks
			if (matchcount < 5)
			{
				badip = qtrue;
			}
			else
			{
				if (subrange != 8 && subrange != 16 && subrange != 24 && subrange != 32)
				{
					badip = qtrue;
				}
				else
				{
					for (i = 0; i < 4; ++i)
					{
						if (match[i] < 0 || match[i] > 255)
						{
							badip = qtrue;
							break;
						}
					}
				}
			}

			if (badip)
			{
				if (matchcount != -1)
				{
					G_LogPrintf("Banned: Bad IP entry in banned file: %s (line: %d)\n", line, linenumber);
				}

				continue;
			}

			// Check it now..only bothers with it, if first bit matches..
			if (clientIP[0] == match[0])
			{
				// TODO(nobo): Change from 32/24/16/8 -> 4/3/2/1. This doesn't match subnet filtering anymore, so why use the bitmask values and confuse non-programmers?
				if (subrange == 32 && clientIP[1] == match[1] && clientIP[2] == match[2] && clientIP[3] == match[3])
				{
					banned = qtrue;
				}
				else if (subrange == 24 && clientIP[2] == match[2] && clientIP[1] == match[1])
				{
					banned = qtrue;
				}
				else if (subrange == 16 && clientIP[1] == match[1])
				{
					banned = qtrue;
				}
				else if (subrange == 8)
				{
					banned = qtrue;
				}
			}

			if (banned)
			{
				// Overwrite if needed..
				if (Q_FindToken(g_bypassPasswords.string, password))
				{
					banned = qfalse;
				}

				break;
			}
		}

		fclose(banfile);
	}

	return banned;
}

/*
===========
TempbanClient
===========
*/
void TempbanClient(gentity_t *ent, int minsbanned)
{
	TempbanIp(clientIP(ent, qtrue), minsbanned);
}

/*
===========
TempbanIp

Writes ip address to tempban file.
===========
*/
void TempbanIp(char *ip, int minsbanned)
{
	FILE *file;
	time_t seconds_since_epoch;

	file = fopen(TEMPBANNED_FILE_PATH, "a+");

	if (file)
	{
		time(&seconds_since_epoch);

		fputs(va("%s %lli\n", ip, seconds_since_epoch + (long long int)(minsbanned * 60)), file);
		fclose(file);
	}
	else
	{
		G_LogPrintf(TEMPBANNED_FILE_PATH " could not be opened! Aborting tempban of ip %s\n", ip);
	}
}

/*
===========
RemoveExpiredTempbans
===========
*/
void RemoveExpiredTempbans(void)
{
	#define TEMP_TEMPBANNED_FILE_PATH "tempbannedtemp.txt"

	FILE *tempbannedfile;
	FILE *temp_tempbannedfile;
	char tempbanned_line[64];
	char tempbannedip[39];
	long long int tempban_expiration_seconds;
	time_t current_seconds;
	qboolean all_tempbans_expired = qtrue;

	time(&current_seconds);

	tempbannedfile = fopen(TEMPBANNED_FILE_PATH, "r+");

	if (tempbannedfile)
	{
		temp_tempbannedfile = fopen(TEMP_TEMPBANNED_FILE_PATH, "w+");

		if (temp_tempbannedfile)
		{
			while (fgets(tempbanned_line, 64, tempbannedfile) != NULL)
			{
				sscanf(tempbanned_line, "%s %lli", tempbannedip, &tempban_expiration_seconds);

				if ((tempban_expiration_seconds - current_seconds) > 0)
				{
					fputs(tempbanned_line, temp_tempbannedfile);
					all_tempbans_expired = qfalse;
				}
			}

			fclose(temp_tempbannedfile);
			fclose(tempbannedfile);

			temp_tempbannedfile = fopen(TEMP_TEMPBANNED_FILE_PATH, "r");
			tempbannedfile = fopen(TEMPBANNED_FILE_PATH, "w+");

			if (temp_tempbannedfile && tempbannedfile)
			{
				while (fgets(tempbanned_line, 64, temp_tempbannedfile) != NULL)
				{
					fputs(tempbanned_line, tempbannedfile);
				}

				fclose(tempbannedfile);
				fclose(temp_tempbannedfile);
			}
			else if (temp_tempbannedfile)
			{
				fclose(temp_tempbannedfile);
			}
			else if (tempbannedfile)
			{
				fclose(tempbannedfile);
			}

			remove(TEMP_TEMPBANNED_FILE_PATH);
			if (all_tempbans_expired)
			{
				remove(TEMPBANNED_FILE_PATH);
			}
		}
	}
}

/*
===========
IsTempBanned
===========
*/
char *IsTempBanned(char *ip)
{
	FILE *tempbannedfile;
	char tempbannedip[39];
	int	hours;
	int	tens;
	int	mins;
	long long int seconds;
	char tempbannedinfo[1024];
	char *sortTime;
	char *theTime;
	long long int tempban_expiration_seconds;
	time_t seconds_since_epoch;

	tempbannedfile = fopen(TEMPBANNED_FILE_PATH, "r");

	if (tempbannedfile)
	{
		time(&seconds_since_epoch);

		while (fgets(tempbannedinfo, 1024, tempbannedfile) != NULL)
		{
			sscanf(tempbannedinfo, "%s %lli", tempbannedip, &tempban_expiration_seconds);
			long long int seconds_remaining = tempban_expiration_seconds - seconds_since_epoch;

			if (!Q_stricmp(ip, tempbannedip) && seconds_remaining > 0)
			{
				fclose(tempbannedfile);
				seconds = seconds_remaining;
				mins = seconds / 60;
				hours = mins / 60;
				seconds -= mins * 60;
				tens = seconds / 10;
				seconds -= tens * 10;
				mins -= hours * 60;

				// Convert to hours, mins etc..
				sortTime = (hours > 0) ? ((mins > 9) ? va("%i:", hours) : va("%i:0", hours)) : "";
				theTime = va("%s%i:%i%i", sortTime, mins, tens, seconds);

				return va("You are temporarily banned for ^7%s", theTime);
			}
		}
		
		fclose(tempbannedfile);
	}

	return NULL;
}


