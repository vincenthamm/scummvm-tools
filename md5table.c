/* md5table - Convert a MD5 table to either PHP or C++ code
 * Copyright (C) 2002, 2003  The ScummVM Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "util.h"

typedef struct {
	const char *key;
	const char *value;
} StringMap;

typedef struct {
	const char *desc;
	const char *platform;
	const char *language;
	const char *md5;
	const char *target;
	const char *infoSource;
} Entry;

/* Map MD5 table platform names to ScummVM constant names.
 * Note: Currently not many constants are defined within ScummVM. However, more
 * will probably be added eventually (see also commented out constants in
 * common/util.h).
 */
static const StringMap platformMap[] = {
	{ "3DO",		"kPlatformUnknown" },
	{ "Amiga",		"kPlatformAmiga" },
	{ "Atari",		"kPlatformAtariST" },
	{ "DOS",		"kPlatformPC" },
	{ "FM-TOWNS",	"kPlatformFMTowns" },
	{ "Mac",		"kPlatformMacintosh" },
	{ "SEGA",		"kPlatformUnknown" },
	{ "Windows",	"kPlatformPC" },

	{ "All?",		"kPlatformUnknown" },
	{ "All",		"kPlatformUnknown" },

	{ 0,			"kPlatformUnknown" }
};

static const StringMap langMap[] = {
	{ "en",		"EN_USA" },
	{ "de",		"DE_DEU" },
	{ "fr",		"FR_FRA" },
	{ "it",		"IT_ITA" },
	{ "pt",		"PT_BRA" },
	{ "es",		"ES_ESP" },
	{ "jp",		"JA_JPN" },
	{ "zh",		"ZH_TWN" },
	{ "ko",		"KO_KOR" },
	{ "se",		"SE_SWE" },
	{ "en",		"EN_GRB" },
	{ "hb",		"HB_HEB" },
	{ "ru",		"RU_RUS" },
	{ "cz",		"CZ_CZE" },
	{ "nl",		"NL_NOR" },

	{ "All",	"UNK_LANG" },
	{ "All?",	"UNK_LANG" },

	{ 0,		"UNK_LANG" }
};

static const char *php_header =
	"<!-- This file was generated by the md5table tool. DO NOT EDIT!\n"
	" -->\n"
	"\n";

static const char *c_header =
	"/* This file was generated by the md5table tool. DO NOT EDIT!\n"
	" */\n"
	"\n"
	"struct MD5Table {\n"
	"	const char *md5;\n"
	"	const char *target;\n"
	"	Common::Language language;\n"
	"	Common::Platform platform;\n"
	"};\n"
	"\n"
	"static const MD5Table md5table[] = {\n";

static const char *c_footer =
	"	{ 0, 0, Common::UNK_LANG, Common::kPlatformUnknown }\n"
	"};\n";

static void parseEntry(Entry *entry, char *line) {
	assert(entry);
	assert(line);
	
	/* Split at the tabs */
	entry->desc = strtok(line, "\t\n\r");
	entry->platform = strtok(NULL, "\t\n\r");
	entry->language = strtok(NULL, "\t\n\r");
	entry->md5 = strtok(NULL, "\t\n\r");
	entry->target = strtok(NULL, "\t\n\r");
	entry->infoSource = strtok(NULL, "\t\n\r");
}

static bool isEmptyLine(const char *line) {
	const char *whitespace = " \t\n\r";
	while (*line) {
		if (!strchr(whitespace, *line))
			return false;
		line++;
	}
	return true;
}

static const char *mapStr(const char *str, const StringMap *map) {
	assert(str);
	assert(map);
	while (map->key) {
		if (0 == strcmp(map->key, str))
			return map->value;
		map++;
	}
	warning("mapStr: unknown string '%s', defaulting to '%s'", str, map->value);
	return map->value;
}

void showhelp(const char *exename)
{
	printf("\nUsage: %s <params>\n", exename);
	printf("\nParams:\n");
	printf(" --c++   output C++ code for inclusion in ScummVM (default)\n");
	printf(" --php   output PHP code for the web site\n");
	exit(2);
}

int main(int argc, char *argv[])
{
	FILE *inFile = stdin;
	FILE *outFile = stdout;
	char buffer[1024];
	char section[1024];
	char *line;
	int err;
	int i;

	const int entrySize = 256;
	int numEntries = 0, maxEntries = 1;
	char *entriesBuffer = malloc(maxEntries * entrySize);

	bool phpOutput = false;

	if (argc != 2)
		showhelp(argv[0]);
	if (strcmp(argv[1], "--c++") == 0) {
		phpOutput = false;
	} else if (strcmp(argv[1], "--php") == 0) {
		phpOutput = true;
	} else {
		showhelp(argv[0]);
	}

	if (phpOutput)
		fprintf(outFile, php_header);

	while ((line = fgets(buffer, sizeof(buffer), inFile))) {
		/* Parse line */
		if (line[0] == '#' || isEmptyLine(line))
			continue;	/* Skip comments & empty lines */
		if (line[0] == '\t') {
			Entry entry;
			assert(section[0]);
			parseEntry(&entry, line+1);
			if (phpOutput) {
				fprintf(outFile, "\t\t<?php addEntry(");
				fprintf(outFile, "\"%s\", ", entry.desc);
				fprintf(outFile, "\"%s\", ", entry.platform);
				fprintf(outFile, "\"%s\", ", entry.language);
				fprintf(outFile, "\"%s\", ", entry.md5);
				fprintf(outFile, "\"%s\"", entry.target);
				if (entry.infoSource)
					fprintf(outFile, ", \"%s\"", entry.infoSource);
				fprintf(outFile, "); ?>\n");
			} else if (entry.md5) {
				if (numEntries >= maxEntries) {
					maxEntries *= 2;
					entriesBuffer = realloc(entriesBuffer, maxEntries * entrySize);
				}
				snprintf(entriesBuffer + numEntries * entrySize, entrySize, "\t{ \"%s\", \"%s\", Common::%s, Common::%s },\n",
					entry.md5, entry.target, mapStr(entry.language, langMap), mapStr(entry.platform, platformMap));
				numEntries++;
			}
		} else {
			for (i = 0; buffer[i] && buffer[i] != '\n'; ++i)
				section[i] = buffer[i];
			section[i] = 0;
			if (phpOutput) {
				fprintf(outFile, "\t<tr><td colspan='7'><strong>%s</strong></td></tr>\n", section);
			}
		}
	}

	err = ferror(inFile);
	if (err)
		error("Failed reading from input file, error %d", err);

	if (!phpOutput) {
		/* Printf header */
		fprintf(outFile, c_header);
		/* Now sort the MD5 table (this allows for binary searches) */
		qsort(entriesBuffer, numEntries, entrySize, strcmp);
		/* Output the table */
		for (i = 0; i < numEntries; ++i)
			fprintf(outFile, entriesBuffer + i * entrySize);
		/* Finally, print the footer */
		fprintf(outFile, c_footer);
	}
	
	free(entriesBuffer);

	return 0;
}
