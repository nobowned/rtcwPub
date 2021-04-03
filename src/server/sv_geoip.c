//
// sv_geoip.c
//
// original code from maxmind (now legacy & deprecated): https://github.com/maxmind/geoip-api-c
// free updates of legacy database: https://mailfud.org/geoip-legacy/

#include "server.h"

#define UNKNOWN_COUNTRY_NAME "Unknown"

typedef struct {
	byte *contents;
	int contents_size;
} geoip_database_t;

const char *country_names[GEOIP_MAX_COUNTRY_NAMES] = {
    UNKNOWN_COUNTRY_NAME,
    "Asia/Pacific Region",
    "Europe",
    "Andorra",
    "United Arab Emirates",
    "Afghanistan",
    "Antigua and Barbuda",
    "Anguilla",
    "Albania",
    "Armenia",
    "Curacao",
    "Angola",
    "Antarctica",
    "Argentina",
    "American Samoa",
    "Austria",
    "Australia",
    "Aruba",
    "Azerbaijan",
    "Bosnia and Herzegovina",
    "Barbados",
    "Bangladesh",
    "Belgium",
    "Burkina Faso",
    "Bulgaria",
    "Bahrain",
    "Burundi",
    "Benin",
    "Bermuda",
    "Brunei Darussalam",
    "Bolivia",
    "Brazil",
    "Bahamas",
    "Bhutan",
    "Bouvet Island",
    "Botswana",
    "Belarus",
    "Belize",
    "Canada",
    "Cocos (Keeling) Islands",
    "Congo, The Democratic Republic of the",
    "Central African Republic",
    "Congo",
    "Switzerland",
    "Cote D'Ivoire",
    "Cook Islands",
    "Chile",
    "Cameroon",
    "China",
    "Colombia",
    "Costa Rica",
    "Cuba",
    "Cape Verde",
    "Christmas Island",
    "Cyprus",
    "Czech Republic",
    "Germany",
    "Djibouti",
    "Denmark",
    "Dominica",
    "Dominican Republic",
    "Algeria",
    "Ecuador",
    "Estonia",
    "Egypt",
    "Western Sahara",
    "Eritrea",
    "Spain",
    "Ethiopia",
    "Finland",
    "Fiji",
    "Falkland Islands (Malvinas)",
    "Micronesia, Federated States of",
    "Faroe Islands",
    "France",
    "Sint Maarten (Dutch part)",
    "Gabon",
    "United Kingdom",
    "Grenada",
    "Georgia",
    "French Guiana",
    "Ghana",
    "Gibraltar",
    "Greenland",
    "Gambia",
    "Guinea",
    "Guadeloupe",
    "Equatorial Guinea",
    "Greece",
    "South Georgia and the South Sandwich Islands",
    "Guatemala",
    "Guam",
    "Guinea-Bissau",
    "Guyana",
    "Hong Kong",
    "Heard Island and McDonald Islands",
    "Honduras",
    "Croatia",
    "Haiti",
    "Hungary",
    "Indonesia",
    "Ireland",
    "Israel",
    "India",
    "British Indian Ocean Territory",
    "Iraq",
    "Iran, Islamic Republic of",
    "Iceland",
    "Italy",
    "Jamaica",
    "Jordan",
    "Japan",
    "Kenya",
    "Kyrgyzstan",
    "Cambodia",
    "Kiribati",
    "Comoros",
    "Saint Kitts and Nevis",
    "Korea, Democratic People's Republic of",
    "Korea, Republic of",
    "Kuwait",
    "Cayman Islands",
    "Kazakhstan",
    "Lao People's Democratic Republic",
    "Lebanon",
    "Saint Lucia",
    "Liechtenstein",
    "Sri Lanka",
    "Liberia",
    "Lesotho",
    "Lithuania",
    "Luxembourg",
    "Latvia",
    "Libya",
    "Morocco",
    "Monaco",
    "Moldova, Republic of",
    "Madagascar",
    "Marshall Islands",
    "Macedonia",
    "Mali",
    "Myanmar",
    "Mongolia",
    "Macau",
    "Northern Mariana Islands",
    "Martinique",
    "Mauritania",
    "Montserrat",
    "Malta",
    "Mauritius",
    "Maldives",
    "Malawi",
    "Mexico",
    "Malaysia",
    "Mozambique",
    "Namibia",
    "New Caledonia",
    "Niger",
    "Norfolk Island",
    "Nigeria",
    "Nicaragua",
    "Netherlands",
    "Norway",
    "Nepal",
    "Nauru",
    "Niue",
    "New Zealand",
    "Oman",
    "Panama",
    "Peru",
    "French Polynesia",
    "Papua New Guinea",
    "Philippines",
    "Pakistan",
    "Poland",
    "Saint Pierre and Miquelon",
    "Pitcairn Islands",
    "Puerto Rico",
    "Palestinian Territory",
    "Portugal",
    "Palau",
    "Paraguay",
    "Qatar",
    "Reunion",
    "Romania",
    "Russian Federation",
    "Rwanda",
    "Saudi Arabia",
    "Solomon Islands",
    "Seychelles",
    "Sudan",
    "Sweden",
    "Singapore",
    "Saint Helena",
    "Slovenia",
    "Svalbard and Jan Mayen",
    "Slovakia",
    "Sierra Leone",
    "San Marino",
    "Senegal",
    "Somalia",
    "Suriname",
    "Sao Tome and Principe",
    "El Salvador",
    "Syrian Arab Republic",
    "Swaziland",
    "Turks and Caicos Islands",
    "Chad",
    "French Southern Territories",
    "Togo",
    "Thailand",
    "Tajikistan",
    "Tokelau",
    "Turkmenistan",
    "Tunisia",
    "Tonga",
    "Timor-Leste",
    "Turkey",
    "Trinidad and Tobago",
    "Tuvalu",
    "Taiwan",
    "Tanzania, United Republic of",
    "Ukraine",
    "Uganda",
    "United States Minor Outlying Islands",
    "United States",
    "Uruguay",
    "Uzbekistan",
    "Holy See (Vatican City State)",
    "Saint Vincent and the Grenadines",
    "Venezuela",
    "Virgin Islands, British",
    "Virgin Islands, U.S.",
    "Vietnam",
    "Vanuatu",
    "Wallis and Futuna",
    "Samoa",
    "Yemen",
    "Mayotte",
    "Serbia",
    "South Africa",
    "Zambia",
    "Montenegro",
    "Zimbabwe",
    "Anonymous Proxy",
    "Satellite Provider",
    "Other",
    "Aland Islands",
    "Guernsey",
    "Isle of Man",
    "Jersey",
    "Saint Barthelemy",
    "Saint Martin",
    "Bonaire, Saint Eustatius and Saba",
    "South Sudan",
    "Other" 
};

geoip_database_t geoip_database;

char *GeoIP_GetCountryName(int clientNum) {
	int depth, packed_ip;
	unsigned int x = 0;
	unsigned int step = 0;
	const unsigned char *buf = NULL;
	char ip[MAX_IPV4_LENGTH];
	static char country_name[GEOIP_COUNTRY_NAME_MAX_LEN];

	if (geoip_database.contents_size <= 0) {
		return "";
	}

	SV_GetClientIp(clientNum, ip, sizeof(ip));
	if (!strcmp(ip, "localhost")) {
		return UNKNOWN_COUNTRY_NAME;
	}

	packed_ip = Q_GetPackedIpAddress(ip);

	for (depth = 31; depth >= 0; depth--) {

		step = 6 * x;

		if (step + 6 >= geoip_database.contents_size) {
			Com_Printf("GeoIP_GetCountryName: Error traversing database for ip %s (%i)\n", ip, packed_ip);
			return UNKNOWN_COUNTRY_NAME;
		}

		buf = geoip_database.contents + step;

		if (packed_ip & (1 << depth)) {
			x = (buf[3] << 0) + (buf[4] << 8) + (buf[5] << 16);
		}
		else {
			x = (buf[0] << 0) + (buf[1] << 8) + (buf[2] << 16);
		}

		if (x >= 16776960) {
			Q_strncpyz(country_name, country_names[x - 16776960], sizeof(country_name));
			return country_name;
		}
	}

	Com_Printf("GeoIP_GetCountryName: Error traversing database for ip %s (%i)\n", ip, packed_ip);
	return UNKNOWN_COUNTRY_NAME;
}

void GeoIP_Initialize() {
	fileHandle_t database_file_handle;

	geoip_database.contents_size = FS_FOpenFileRead("GeoIP.dat", &database_file_handle, FS_READ);
	if (!database_file_handle) {
		Com_Printf("GeoIP_Initialize: Error opening database\n");
		return;
	}

	geoip_database.contents = (byte *)Z_Malloc(geoip_database.contents_size + 1);
	FS_Read(geoip_database.contents, geoip_database.contents_size, database_file_handle);
	FS_FCloseFile(database_file_handle);
}