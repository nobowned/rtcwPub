/*
===========================================================================
L0 - g_censored.c

Censoring 
This whole file is pretty much a dump from etPUB - with few adjustments here and there.

------
Original Source note:
// josh: NT is Neil Toronto. I got a lot of the new filter stuff from
//       his Quake3 altfire mod.
------

Created: 17. Nov / 2012
Last Updated: 17. Nov. / 2012
===========================================================================
*/
#include "g_local.h"

static char *filteredWords[] = {
	// first we have words that are fine but have swear words in them
	// these will not be substituted, but will advance the counter
	"ACCUMUL", NULL,
	"ACUMEN", NULL,
	"ASSAIL", NULL,
	"ASSASSIN", NULL,
	"ASSAULT", NULL,
	"ASSEMBL", NULL,
	"ASSENT", NULL,
	"ASSERT", NULL,
	"ASSESS", NULL,
	"ASSET", NULL,
	"ASSIGN", NULL,
	"ASSIMILAT", NULL,
	"ASSIST", NULL,
	"ASSOCIA", NULL,
	"ASSONAN", NULL,
	"ASSORT", NULL,
	"ASSUAGE", NULL,
	"ASSUM", NULL,
	"ASSUR", NULL,
	"ASSYRI", NULL,
	"BABCOCK", NULL,
	"BASS", NULL,
	"BRASS", NULL,
	"CANVASS", NULL,
	"CARCASS", NULL,
	"CASSANDRA", NULL,
	"CASSEROLE", NULL,
	"CASSIOPEIA", NULL,
	"CASSITE", NULL,
	"CASSIUS", NULL,
	"CASSOCK", NULL,
	"CHASSIS", NULL,
	"CHUMP", NULL,
	"CIRCUM", NULL,
	"COCKATOO", NULL,
	"COCKCROW", NULL,
	"COCKEYE", NULL,
	"COCKLE", NULL,
	"COCKPIT", NULL,
	"COCKROACH", NULL,
	"COCKTAIL", NULL,
	"COCKY", NULL,
	"CRASS", NULL,
	"CUMBER", NULL,
	"CUMMIN", NULL,
	"CUMULAT", NULL,
	"CUMULUS", NULL,
	"DICKER", NULL,
	"DICKEY", NULL,
	"DICKINSON", NULL,
	"DICKSON", NULL,
	"DICKY", NULL,
	"DOCUMENT", NULL,
	"ECUMENI", NULL,
	"EMBARRASS", NULL,
	"ENCUMB", NULL,
	"GASSE", NULL,
	"GASSING", NULL,
	"GASSY", NULL,
	"GRASS", NULL,
	"HANCOCK", NULL,
	"HARASS", NULL,
	"HASSLE", NULL,
	"HELLBENDER", NULL,
	"HELLENIC", NULL,
	"HELLENIZ", NULL,
	"HELLESPONT", NULL,
	"HELLFIRE", NULL,
	"HELLISH", NULL,
	"HELLMAN", NULL,
	"HELLO", NULL,
	"HELL'S", NULL,
	"HITCHCOCK", NULL,
	"HUMPBACK", NULL,
	"HUMPHREY", NULL,
	"HUMPTY", NULL,
	"INCUMBENT", NULL,
	"JANUS", NULL,
	"JURASSIC", NULL,
	"LASS", NULL,
	"MANUSCRIPT", NULL,
	"MASS", NULL,
	"MITCHELL", NULL,
	"MODICUM", NULL,
	"NASSAU", NULL,
	"PASS", NULL,
	"PEACOCK", NULL,
	"PICASSO", NULL,
	"PISSED", NULL,  // i'm pissed... etc
	"RECUMBENT", NULL,
	"SASSING", NULL,
	"SCHELLING", NULL,
	"SCUM", NULL,
	"SHELL", NULL,
	"SHUTTLECOCK", NULL,
	"STOPCOCK", NULL,
	"SUCCUMB", NULL,
	"SWANK", NULL,
	"TALLAHASSEE", NULL,
	"TASS", NULL,
	"TETANUS", NULL,
	"THUMP", NULL,
	"TRIASSIC", NULL,
	"URANUS", NULL,
	"VASSAL", NULL,
	"WEATHERCOCK", NULL,
	"WEIERSTRASS", NULL,
	"WHELLER", NULL,
	"WINCHELL", NULL,
	"WOODCOCK", NULL,

	// then we have the stuff that will be substituted
	"scheisse", "********", // German
	"testicle", "********",
	"knockers", "********",
	"jack off", "**** ***",
	"blow job", "**** ***",
	"jackoff", "*******",
	"blowjob", "*******",
	"bastard", "*******",
	"genital", "*******",
	"pierdol", "*******", // Polish
	"blow me", "**** me"
	"putana", "******",  // Spanish
	"mierda", "******",	 // Spanish
	"orgasm", "******",
	"vagina", "******",
	"testes", "******",
	"crotch", "******",
//	"dammit", "darnit",
//	"faggot", "******",
//	"faggit", "******",
	"biatch", "*iatch",	
	"putain", "******",   //French	
	"cojeme", "******",  // Spanish
	"spierd"  "******",  // Polish	
	"bzdura", "******",  // Polish
	"palant", "******",  // Polish
	"zasran", "******",  // Polish
	"ching", "*****",   // Spanish
	"merde", "*****",	// French
	"pedal", "*****",	// Polish	
	"verga", "*****",	// Spanish	
	"salop", "*****",   // French
	"arsch", "*****",	// German
	"pussy", "p***y",
	"biach", "wiach",	
	"dildo", "di*do",
	"penis", "pe*is",
	"bitch", "bi*ch",
	"boner", "*****",
	"kweef", "*****",
	"queef", "*****",
	"sperm", "sp**m",
	"baise", "*****", // French
	"kurwa", "kur*a", // Polish	
	"dupek", "*****", // Polish
	"gowno", "*****", // Polish
	"pizdo", "*****", // Polish	
	"jebal", "*****", // Polish	
	"pojeb", "*****", // Polish
	"puto", "****", // Spanish
//	"puta", "****", // Spanish
//	"pute", "****",	// French
	"culo", "****", // Spanish
	"joto", "****", // Spanish
	"aasi", "****", // French
	"cwel", "****", // Polish
	"shit", "sh*t",
	"fuck", "****",
	"fick", "****",  // German
//	"damn", "darn",
//	"hell", "heck",
	"cunt", "****",
	"dick", "****",
	"anus", "****",
	"cock", "****",
//	"hump", "****",
//	"piss", "p*ss",
//	"puss", "****",
	"wank", "****",
	"ass", "***",
	"cum", "***",
	"fag", "gay",
};
static const int nFilteredWords = sizeof(filteredWords) / sizeof(char *) / 2;

// these MUST be ordered by precedence (usually largest first)
// we have two lists because our filtering needs are different
static char *filteredNames[] = {
	// first we have words that are fine but have swear words in them
	// these will not be substituted, but will advance the counter
	"ACCUMUL", NULL,
	"ACUMEN", NULL,
	"ASSAIL", NULL,
	"ASSASSIN", NULL,
	"ASSAULT", NULL,
	"ASSEMBL", NULL,
	"ASSENT", NULL,
	"ASSERT", NULL,
	"ASSESS", NULL,
	"ASSET", NULL,
	"ASSIGN", NULL,
	"ASSIMILAT", NULL,
	"ASSIST", NULL,
	"ASSOCIA", NULL,
	"ASSONAN", NULL,
	"ASSORT", NULL,
	"ASSUAGE", NULL,
	"ASSUM", NULL,
	"ASSUR", NULL,
	"ASSYRI", NULL,
	"BABCOCK", NULL,
	"BASS", NULL,
	"BRASS", NULL,
	"CANVASS", NULL,
	"CARCASS", NULL,
	"CASSANDRA", NULL,
	"CASSEROLE", NULL,
	"CASSIOPEIA", NULL,
	"CASSITE", NULL,
	"CASSIUS", NULL,
	"CASSOCK", NULL,
	"CHASSIS", NULL,
	"CHUMP", NULL,
	"CIRCUM", NULL,
	"COCKATOO", NULL,
	"COCKCROW", NULL,
	"COCKEYE", NULL,
	"COCKLE", NULL,
	"COCKPIT", NULL,
	"COCKROACH", NULL,
	"COCKTAIL", NULL,
	"COCKY", NULL,
	"CRASS", NULL,
	"CUMBER", NULL,
	"CUMMIN", NULL,
	"CUMULAT", NULL,
	"CUMULUS", NULL,
	"DICKER", NULL,
	"DICKEY", NULL,
	"DICKINSON", NULL,
	"DICKSON", NULL,
	"DICKY", NULL,
	"DOCUMENT", NULL,
	"ECUMENI", NULL,
	"EMBARRASS", NULL,
	"ENCUMB", NULL,
	"GASSE", NULL,
	"GASSING", NULL,
	"GASSY", NULL,
	"GRASS", NULL,
	"HANCOCK", NULL,
	"HARASS", NULL,
	"HASSLE", NULL,
	"HITCHCOCK", NULL,
	"HUMPBACK", NULL,
	"HUMPHREY", NULL,
	"HUMPTY", NULL,
	"INCUMBENT", NULL,
	"JANUS", NULL,
	"JURASSIC", NULL,
	"LASS", NULL,
	"MANUSCRIPT", NULL,
	"MASS", NULL,
	"MODICUM", NULL,
	"NASSAU", NULL,
	"PASS", NULL,
	"PEACOCK", NULL,
	"PICASSO", NULL,
	"RECUMBENT", NULL,
	"SASSING", NULL,
	"SCUM", NULL,
	"SHUTTLECOCK", NULL,
	"STOPCOCK", NULL,
	"SUCCUMB", NULL,
	"SWANK", NULL,
	"TALLAHASSEE", NULL,
	"TASS", NULL,
	"TETANUS", NULL,
	"THUMP", NULL,
	"TRIASSIC", NULL,
	"URANUS", NULL,
	"VASSAL", NULL,
	"WEATHERCOCK", NULL,
	"WEIERSTRASS", NULL,
	"WOODCOCK", NULL,

	// then we have the stuff that will be substituted
	"jack off", "**** ***",
	"jackoff", "*******",
	"blowjob", "*******",
	"bastard", "*******",
	"genital", "*******",
	"blow me", "**** me",
	"console", "*******",
	"vagina", "******",
	"faggot", "******",
	"biatch", "******",
	"hitler", "*****",
	"biach", "*****",
	"dildo", "*****",
	"penis", "*****",
	"bitch", "*****",
	"whore", "*****",
	"admin", "*****",
	"adm!n", "*****",
	"fuck", "****",
};
static const int nFilteredNames = sizeof(filteredNames) / sizeof(char *) / 2;

//                   "AB C DE FG HI J KL M NO P Q RST U V W X YZ",
char *matchingNums = "48\0\03\06\01\0\01\0\00\0\0\057\0\0\0\0\02";
//                   "A B C D E F GHI J KL M N O P Q RST U V W X Y Z",
char *matchingSymb = "@\0\0\0\0\0\0#!\0\0!\0\0\0\0\0\0$+\0\0\0\0\0\0";


// returns qtrue if the character matches
// this should filter out most number and symbol substitutions
qboolean filter_charmatches( char c1, char c2 ) {
	c1 = toupper( c1 );
	c2 = toupper( c2 );

	if ( c1 == c2 )
		return qtrue;

	if ( c2 >= 'A' && c2 <= 'Z' ) {
		int idx = c2 - 'A';

		if ( c1 == matchingNums[idx] )
			return qtrue;

		if ( c1 == matchingSymb[idx] )
			return qtrue;
	} 

	return qfalse;
}

qboolean filter_isletter( char c ) {
	if ( c >= 'A'  && c <= 'Z' )
		return qtrue;

	if ( c >= 'a'  && c <= 'z' )
		return qtrue;

	return qfalse;
}


// returns the comparison of msg with sub, ignoring any color
// coding in msg
qboolean filter_cmpgood( char *msg, char *sub ) {
	int			msglen, sublen;
	char		*msgptr, *subptr, *msgend, *subend;

	// don't bother if it doesn't begin a word
	if ( *msg == '^' || *msg <= ' ' || *msg == '_' )
		return qfalse;

	msgptr = msg;
	msglen = strlen(msg);
	msgend = msg + msglen;

	subptr = sub;
	sublen = strlen(sub);
	subend = sub + sublen;

	for ( ; msgptr < msgend && subptr < subend; msgptr++, subptr++ ) {
		// skip color codes, whitespace and underscores
		while ( *msgptr == '^' ) {
			msgptr += 2;
			if ( msgptr >= msgend )
				return qfalse;
		}

		if ( toupper(*msgptr) == toupper(*subptr) )
			continue;
		else
			return qfalse;
	}

	if ( subptr >= subend )
		return qtrue;
	else
		return qfalse;
}

// returns the comparison of msg with sub, ignoring any color
// coding, extra spaces, underscores, etc., etc.
// wstart is true if msg begins a word
qboolean filter_cmp( char *msg, char *sub, qboolean wstart ) {
	int			numspaces = 0, numnumbers = 0;
	int			msglen, sublen;
	char		*msgptr, *subptr, *msgend, *subend;

	// don't bother if it doesn't begin a word
	if ( msg[0] == '^' || msg[0] <= ' ' || msg[0] == '_' )
		return qfalse;

	msgptr = msg;
	msglen = strlen(msg);
	msgend = msg + msglen;

	subptr = sub;
	sublen = strlen(sub);
	subend = sub + sublen;

	for ( ; msgptr < msgend && subptr < subend; msgptr++, subptr++ ) {
		// skip color codes, whitespace and underscores
		while ( *msgptr == '^' || *msgptr <= ' ' || *msgptr == '_' ) {
			if ( *msgptr == '^' ) {
				msgptr += 2;
			} else {
				msgptr++;
				numspaces++;
			}

			if ( msgptr >= msgend )
				return qfalse;
		}

		// skip underscores in checked word
		while ( *subptr == '_' ) {
			subptr++;
			if ( subptr >= subend )
				return qtrue;
		}

		// keep track of the number of numbers
		if ( *msgptr >= '0' && *msgptr <= '9' )
			numnumbers++;

		if ( filter_charmatches( *msgptr, *subptr ) )
			continue;
		else
			return qfalse;
	}

	if ( subptr >= subend ) {
		// return false if all numbers (455 might be filtered otherwise)
		if ( numnumbers >= sublen )
			return qfalse;

		// make sure the match starts and ends a word
		// otherwise, we can't be sure it's bad ("dish i took" "as sara" "about it")
		if ( numspaces ) {
			if ( wstart && !filter_isletter(*msgptr) ) {
				return qtrue;
			} else {
				// if we match and there are three or more spaces, the chances of having a legit
				// word are very, very remote
				if ( numspaces >= 3 )
					return qtrue;
				else
					return qfalse;
			}
		}

		return qtrue;
	}
	else
		return qfalse;
}

// inserts sub into the beginning of msg, skipping any color
// coding in msg
int filter_insert( char *msg, char *sub ) {
	int			msglen, sublen;
	char		*msgptr, *subptr, *msgend, *subend;

	msgptr = msg;
	msglen = strlen(msg);
	msgend = msg + msglen;

	subptr = sub;
	sublen = strlen(sub);
	subend = sub + sublen;

	for ( ; msgptr < msgend && subptr < subend; msgptr++, subptr++ ) {
		// skip color codes, whitespace and underscores
		while ( *msgptr == '^' || *msgptr <= ' ' || *msgptr == '_' ) {
			if ( *msgptr == '^' ) {
				msgptr += 2;
				if ( msgptr >= msgend )
					return msglen;
			} else {
				msgptr++;
				if ( msgptr >= msgend )
					return msglen;
			}
		}

		// skip underscores in subbed word
		while ( *subptr == '_' ) {
			subptr++;
			if ( subptr >= subend )
				return msgptr - msg;
		}

		// preserve the case of the original
		if ( *subptr >= 'a' && *subptr <= 'z' && *msgptr >= 'A' && *msgptr <= 'Z' ) {
			*msgptr = *subptr + ('A' - 'a');
		} else {
			*msgptr = *subptr;
		}
	}

	return msgptr - msg;
}

// inserts the specified number of *'s into the beginning of
// msg, skipping any color coding in msg
int filter_blank( char *msg, int len2 ) {
	int		i1, i2;
	int		len1;

	len1 = strlen(msg);

	for ( i1 = 0, i2 = 0; i1 < len1 && i2 < len2; i1++, i2++ ) {
		// skip color codes, whitespace and underscores
		while ( msg[i1] == '^' || msg[i1] <= ' ' || msg[i1] == '_' ) {
			if ( msg[i1] == '^' ) {
				i1 += 2;
				if ( i1 >= len1 )
					return i1 - 1;
			} else {
				i1++;
				if ( i1 >= len1 )
					return i1;
			}

		}

		msg[i1] = '*';
	}

	return i1;
}

qboolean G_FilterString( char *str, char **words, int numWords, char *customWords, int numCustomWords ) {
	int		i, j, len, fidx, sidx;
	char	*filtered;
	qboolean	res = qfalse;
	char	lastchar;

	lastchar = ' ';
	for ( j = 0; str[j] != '\0'; j++ ) {
		for ( i = 0; i < numWords; i++) {
			fidx = i * 2;
			sidx = i * 2 + 1;
			len = strlen( words[fidx] );
			if ( words[sidx] == NULL ) {
				if ( filter_cmpgood( &str[j], words[fidx] ) ) {
					j += len - 1;
				}
			} else {
				if ( filter_cmp( &str[j], words[fidx], !filter_isletter(lastchar) ) ) {
					len = filter_insert( &str[j], words[sidx] );
					res = qtrue;
					j += len - 1;
				}
			}
		}
		lastchar = str[j];
	}

	if ( customWords[0] ) {
		lastchar = ' ';
		for ( j = 0; str[j] != '\0'; j++ ) {
			filtered = customWords;
			for ( i = 0; i < numCustomWords; i++) {
				len = strlen( filtered );
				if ( filter_cmp( &str[j], filtered, !filter_isletter(lastchar) ) ) {
					len = filter_blank( &str[j], len );
					res = qtrue;
					j += len - 1;
				}

				while ( *filtered != '\0' )
					filtered++;

				filtered++;
			}
			lastchar = str[j];
		}
	}

	return res;
}


// Josh: now uses Neil Toronto's code from altfire. A little smarter
qboolean G_CensorText(char *text, wordDictionary *dictionary) {
	int neil_words = nFilteredWords;
	int neil_names = nFilteredNames;
	
	if (!g_noHardcodedCensor.integer ) {
		neil_words = 0;
		neil_names = 0;
	} else if (g_noHardcodedCensor.integer == 1 ) {
		neil_words = 0;
		neil_names = nFilteredNames;
	} else if (g_noHardcodedCensor.integer == 2 ) {
		neil_words = nFilteredWords;
		neil_names = 0;
	} else if (g_noHardcodedCensor.integer == 3 ) {
		neil_words = nFilteredWords;
		neil_names = nFilteredNames;
	}

	if (dictionary == &censorDictionary) 
		return G_FilterString(text, filteredWords, neil_words, g_censorWords.string, dictionary->num_nulled_words);
	else
		return G_FilterString(text, filteredNames, neil_names, g_disallowedNames.string, dictionary->num_nulled_words);
}

qboolean G_CensorName(char *testname, char *userinfo, int clientNum) {
	char censoredName[MAX_NETNAME];
	char name[MAX_NETNAME];

	Q_strncpyz( name, testname, sizeof(name) );
	SanitizeString(name, censoredName, qtrue);
	if (G_CensorText(censoredName,&censorNamesDictionary)) {
		//Q_strncpyz( testname, censoredName, sizeof(censoredName) );	

		trap_DropClient(clientNum,va("\n^3Disallowed name!^7\nPlease change your name."));
	return qtrue;	
	}
return qfalse;
}
