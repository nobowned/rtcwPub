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

const char country_names[GEOIP_MAX_COUNTRY_NAMES][GEOIP_COUNTRY_NAME_MAX_LEN] = {
	{ UNKNOWN_COUNTRY_NAME                           },
	{ UNKNOWN_COUNTRY_NAME                           },
	{ "Europe"                                       },
	{ "Andorra"                                      },
	{ "United Arab Emirates"                         },
	{ "Afghanistan"                                  },
	{ "Antigua and Barbuda"                          },
	{ "Anguilla"                                     },
	{ "Albania"                                      },
	{ "Armenia"                                      },
	{ "Curacao"                                      },
	{ "Angola"                                       },
	{ "Antarctica"                                   },
	{ "Argentina"                                    },
	{ "American Samoa"                               },
	{ "Austria"                                      },
	{ "Australia"                                    },
	{ "Aruba"                                        },
	{ "Azerbaijan"                                   },
	{ "Bosnia and Herzegovina"                       },
	{ "Barbados"                                     },
	{ "Bangladesh"                                   },
	{ "Belgium"                                      },
	{ "Burkina Faso"                                 },
	{ "Bulgaria"                                     },
	{ "Bahrain"                                      },
	{ "Burundi"                                      },
	{ "Benin"                                        },
	{ "Bermuda"                                      },
	{ "Brunei"                                       },
	{ "Bolivia"                                      },
	{ "Brazil"                                       },
	{ "Bahamas"                                      },
	{ "Bhutan"                                       },
	{ "Bouvet Island"                                },
	{ "Botswana"                                     },
	{ "Belarus"                                      },
	{ "Belize"                                       },
	{ "Canada"                                       },
	{ "Cocos (Keeling) Islands"                      },
	{ "Democratic Republic of the Congo"             },
	{ "Central African Republic"                     },
	{ "Republic of the Congo"                        },
	{ "Switzerland"                                  },
	{ "Ivory Coast"                                  },
	{ "Cook Islands"                                 },
	{ "Chile"                                        },
	{ "Cameroon"                                     },
	{ "China"                                        },
	{ "Colombia"                                     },
	{ "Costa Rica"                                   },
	{ "Cuba"                                         },
	{ "Cape Verde"                                   },
	{ "Christmas Island"                             },
	{ "Cyprus"                                       },
	{ "Czechia"                                      },
	{ "Germany"                                      },
	{ "Djibouti"                                     },
	{ "Denmark"                                      },
	{ "Dominica"                                     },
	{ "Dominican Republic"                           },
	{ "Algeria"                                      },
	{ "Ecuador"                                      },
	{ "Estonia"                                      },
	{ "Egypt"                                        },
	{ "Western Sahara"                               },
	{ "Eritrea"                                      },
	{ "Spain"                                        },
	{ "Ethiopia"                                     },
	{ "Finland"                                      },
	{ "Fiji"                                         },
	{ "Falkland Islands"                             },
	{ "Micronesia"                                   },
	{ "Faroe Islands"                                },
	{ "France"                                       },
	{ "Sint Maarten"                                 },
	{ "Gabon"                                        },
	{ "United Kingdom"                               },
	{ "Grenada"                                      },
	{ "Georgia"                                      },
	{ "French Guiana"                                },
	{ "Ghana"                                        },
	{ "Gibraltar"                                    },
	{ "Greenland"                                    },
	{ "Gambia"                                       },
	{ "Guinea"                                       },
	{ "Guadeloupe"                                   },
	{ "Equatorial Guinea"                            },
	{ "Greece"                                       },
	{ "South Georgia and the South Sandwich Islands" },
	{ "Guatemala"                                    },
	{ "Guam"                                         },
	{ "Guinea-Bissau"                                },
	{ "Guyana"                                       },
	{ "Hong Kong"                                    },
	{ "Heard Island and McDonald Islands"            },
	{ "Honduras"                                     },
	{ "Croatia"                                      },
	{ "Haiti"                                        },
	{ "Hungary"                                      },
	{ "Indonesia"                                    },
	{ "Ireland"                                      },
	{ "Israel"                                       },
	{ "India"                                        },
	{ "British Indian Ocean Territory"               },
	{ "Iraq"                                         },
	{ "Iran"                                         },
	{ "Iceland"                                      },
	{ "Italy"                                        },
	{ "Jamaica"                                      },
	{ "Jordan"                                       },
	{ "Japan"                                        },
	{ "Kenya"                                        },
	{ "Kyrgyzstan"                                   },
	{ "Cambodia"                                     },
	{ "Kiribati"                                     },
	{ "Comoros"                                      },
	{ "Saint Kitts and Nevis"                        },
	{ "North Korea"                                  },
	{ "South Korea"                                  },
	{ "Kuwait"                                       },
	{ "Cayman Islands"                               },
	{ "Kazakhstan"                                   },
	{ "Laos"                                         },
	{ "Lebanon"                                      },
	{ "Saint Lucia"                                  },
	{ "Liechtenstein"                                },
	{ "Sri Lanka"                                    },
	{ "Liberia"                                      },
	{ "Lesotho"                                      },
	{ "Lithuania"                                    },
	{ "Luxembourg"                                   },
	{ "Latvia"                                       },
	{ "Libya"                                        },
	{ "Morocco"                                      },
	{ "Monaco"                                       },
	{ "Moldova"                                      },
	{ "Madagascar"                                   },
	{ "Marshall Islands"                             },
	{ "North Macedonia"                              },
	{ "Mali"                                         },
	{ "Myanmar"                                      },
	{ "Mongolia"                                     },
	{ "Macau"                                        },
	{ "Northern Mariana Islands"                     },
	{ "Martinique"                                   },
	{ "Mauritania"                                   },
	{ "Montserrat"                                   },
	{ "Malta"                                        },
	{ "Mauritius"                                    },
	{ "Maldives"                                     },
	{ "Malawi"                                       },
	{ "Mexico"                                       },
	{ "Malaysia"                                     },
	{ "Mozambique"                                   },
	{ "Namibia"                                      },
	{ "New Caledonia"                                },
	{ "Niger"                                        },
	{ "Norfolk Island"                               },
	{ "Nigeria"                                      },
	{ "Nicaragua"                                    },
	{ "Netherlands"                                  },
	{ "Norway"                                       },
	{ "Nepal"                                        },
	{ "Nauru"                                        },
	{ "Niue"                                         },
	{ "New Zealand"                                  },
	{ "Oman"                                         },
	{ "Panama"                                       },
	{ "Peru"                                         },
	{ "French Polynesia"                             },
	{ "Papua New Guinea"                             },
	{ "Philippines"                                  },
	{ "Pakistan"                                     },
	{ "Poland"                                       },
	{ "Saint Pierre and Miquelon"                    },
	{ "Pitcairn Islands"                             },
	{ "Puerto Rico"                                  },
	{ "Palestine"                                    },
	{ "Portugal"                                     },
	{ "Palau"                                        },
	{ "Paraguay"                                     },
	{ "Qatar"                                        },
	{ "Réunion"                                      },
	{ "Romania"                                      },
	{ "Russia"                                       },
	{ "Rwanda"                                       },
	{ "Saudi Arabia"                                 },
	{ "Solomon Islands"                              },
	{ "Seychelles"                                   },
	{ "Sudan"                                        },
	{ "Sweden"                                       },
	{ "Singapore"                                    },
	{ "Saint Helena"                                 },
	{ "Slovenia"                                     },
	{ "Svalbard and Jan Mayen"                       },
	{ "Slovakia"                                     },
	{ "Sierra Leone"                                 },
	{ "San Marino"                                   },
	{ "Senegal"                                      },
	{ "Somalia"                                      },
	{ "Suriname"                                     },
	{ "São Tomé and Príncipe"                        },
	{ "El Salvador"                                  },
	{ "Syria"                                        },
	{ "Eswatini"                                     },
	{ "Turks and Caicos Islands"                     },
	{ "Chad"                                         },
	{ "French Southern Territories"                  },
	{ "Togo"                                         },
	{ "Thailand"                                     },
	{ "Tajikistan"                                   },
	{ "Tokelau"                                      },
	{ "Turkmenistan"                                 },
	{ "Tunisia"                                      },
	{ "Tonga"                                        },
	{ "East Timor"                                   },
	{ "Turkey"                                       },
	{ "Trinidad and Tobago"                          },
	{ "Tuvalu"                                       },
	{ "Taiwan"                                       },
	{ "Tanzania"                                     },
	{ "Ukraine"                                      },
	{ "Uganda"                                       },
	{ "U.S. Minor Outlying Islands"                  },
	{ "United States"                                },
	{ "Uruguay"                                      },
	{ "Uzbekistan"                                   },
	{ "Vatican City"                                 },
	{ "Saint Vincent and the Grenadines"             },
	{ "Venezuela"                                    },
	{ "British Virgin Islands"                       },
	{ "U.S. Virgin Islands"                          },
	{ "Vietnam"                                      },
	{ "Vanuatu"                                      },
	{ "Wallis and Futuna"                            },
	{ "Samoa"                                        },
	{ "Yemen"                                        },
	{ "Mayotte"                                      },
	{ "Serbia"                                       },
	{ "South Africa"                                 },
	{ "Zambia"                                       },
	{ "Montenegro"                                   },
	{ "Zimbabwe"                                     },
	{ UNKNOWN_COUNTRY_NAME                           },
	{ UNKNOWN_COUNTRY_NAME                           },
	{ UNKNOWN_COUNTRY_NAME                           },
	{ "Åland"                                        },
	{ "Guernsey"                                     },
	{ "Isle of Man"                                  },
	{ "Jersey"                                       },
	{ "Saint Barthélemy"                             },
	{ "Saint Martin"                                 },
	{ "Unknown"                                      },
	{ "South Sudan"                                  },
	{ UNKNOWN_COUNTRY_NAME                           },
};

geoip_database_t geoip_database;
qboolean geoip_database_initialized;

char *GeoIP_GetCountryName(int clientNum) {
	int depth, packed_ip;
	unsigned int x = 0;
	unsigned int step = 0;
	const unsigned char *buf = NULL;
	char ip[MAX_IPV4_LENGTH];
	static char country_name[GEOIP_COUNTRY_NAME_MAX_LEN];

	if (!geoip_database_initialized) {
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
			Com_Printf("GeoIP: Error Traversing Database for ip %s (%i) - Perhaps database is corrupt?\n", ip, packed_ip);
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

	Com_Printf("GeoIP: Error Traversing Database for ip %s (%i) - Perhaps database is corrupt?\n", ip, packed_ip);
	return UNKNOWN_COUNTRY_NAME;
}

void GeoIP_Initialize() {
	fileHandle_t database_file_handle;

	geoip_database.contents_size = FS_FOpenFileRead("GeoIP.dat", &database_file_handle, FS_READ);
	if (!database_file_handle) {
		Com_Printf("GeoIP: Error opening database\n");
		return;
	}

	geoip_database.contents = (byte *)Z_Malloc(geoip_database.contents_size + 1);
	FS_Read(geoip_database.contents, geoip_database.contents_size, database_file_handle);
	FS_FCloseFile(database_file_handle);

	geoip_database_initialized = geoip_database.contents_size > 0;
}