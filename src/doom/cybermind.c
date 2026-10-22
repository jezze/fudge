/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko, cybermind
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *
 *-----------------------------------------------------------------------------
 */

#include "config.h"
#include "port.h"

#include "hu_lib.h"

#include "doomtype.h"
#include "doomstat.h"
#include "d_main.h"
#include "d_think.h"
#include "s_sound.h"
#include "i_system.h"
#include "i_main.h"
#include "i_sound.h"
#include "m_menu.h"
#include "lprintf.h"
#include "m_argv.h"
#include "m_misc.h"
#include "i_system.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_tick.h"
#include "i_video.h"
#include "info.h"
#include "r_main.h"
#include "r_things.h"
#include "r_sky.h"
#include "am_map.h"
#include "hu_tracers.h"
#include "g_game.h"
#include "r_demo.h"
#include "d_deh.h"
#include "e6y.h"
#include "cybermind.h"

const int dump_things_version = 1;
int stats_level2;
int dump_things;
int alternative_kills_counter;
int demo_playvoice;

inline void write2bytes(FILE *file, unsigned short data)
{
	int i;
	for (i = 0; i < 2; ++i) {
		fputc((data & (0xFF << (0x08 * i))) >> (0x08 * i), file);
	}
}

inline void write4bytes(FILE *file, unsigned int data)
{
	int i;
	for (i = 0; i < 4; ++i) {
		fputc((data & (0xFF << (0x08 * i))) >> (0x08 * i), file);
	}
}

/*
---------------------------------------
Dump file functions
---------------------------------------
*/

dumpFile_t *dumpFile = NULL;

dumpFile_t* cyb_DumpStart(const char *file)
{
	dumpFile = (dumpFile_t*)malloc(sizeof(dumpFile_t));
	dumpFile->f = fopen(file, "wb");
	dumpFile->recordCount = 0;
	// header
	fputc(0x50, dumpFile->f);
	fputc(0x52, dumpFile->f);
	fputc(0x42, dumpFile->f);
	fputc(0x44, dumpFile->f);
	write4bytes(dumpFile->f, dump_things_version);
	write4bytes(dumpFile->f, dump_things);
	return dumpFile;
}

void cyb_DumpEnd(dumpFile_t *d)
{
	write4bytes(d->f, d->recordCount);
	fclose(d->f);
	free(dumpFile);
}


void cyb_DumpEncodePlayers(dumpFile_t *d)
{
	int i, j;

	++d->recordCount;
	write2bytes(d->f, (short)DE_PLAYERSTAT);
	write4bytes(d->f, totaltics);
	for (i = 0; i < MAXPLAYERS; ++i) {
		player_t *p = &players[i];
		if (!playeringame[i]) {
			fputc(0xFF, d->f);
			continue;
		}
		fputc(i, d->f);
		// coordinates
		write2bytes(d->f, p->mo->x >> 16);
		write2bytes(d->f, p->mo->y >> 16);
		write2bytes(d->f, p->mo->z >> 16);
		// health and armor
		write2bytes(d->f, p->health);
		write2bytes(d->f, p->armorpoints);
		fputc(p->armortype, d->f);
		// all ammo stock
		for (j = 0; j < NUMAMMO; ++j) {
			write2bytes(d->f, p->ammo[j]);
			write2bytes(d->f, p->maxammo[j]);
		}
		// powers
		for (j = 0; j < NUMPOWERS; ++j) {
			write2bytes(d->f, p->powers[j]);
		}
		
		write2bytes(d->f, p->killcount);
		write2bytes(d->f, p->itemcount);
		write2bytes(d->f, p->secretcount);
		write2bytes(d->f, p->deathscount);
		for (j = 0; j < NUMCARDS; ++j) {
			fputc(p->cards[j], d->f);
		}
	}
}


void cyb_DumpSpawn(dumpFile_t *d, mobj_t *s)
{
	++d->recordCount;
	write2bytes(d->f, (short)DE_SPAWN);
	write4bytes(d->f, totaltics);
	write2bytes(d->f, s->x >> FRACBITS);
	write2bytes(d->f, s->y >> FRACBITS);
	write2bytes(d->f, s->z >> FRACBITS);
	fputc((unsigned char)s->type, d->f);

}


void cyb_DumpDamage(dumpFile_t *d, mobj_t *target, mobj_t *inflictor, int damage)
{
	++d->recordCount;
	write2bytes(d->f, (short)DE_DAMAGE);
	write4bytes(d->f, totaltics);
	cyb_DumpEncodeThing(d, target);
	cyb_DumpEncodeThing(d, inflictor);
	write2bytes(d->f, damage);
}


void cyb_DumpKill(dumpFile_t *d, mobj_t *target, mobj_t *who)
{
	++d->recordCount;
	write2bytes(d->f, (short)DE_KILL);
	write4bytes(d->f, totaltics);
	cyb_DumpEncodeThing(d, target);
	cyb_DumpEncodeThing(d, who);
}


void cyb_DumpEncodeThing(dumpFile_t * d, mobj_t *mobj)
{
	if (!mobj) {
		write4bytes(d->f, -2);
		return;
	}
	
	write4bytes(d->f, mobj->index);
	write2bytes(d->f, mobj->x >> FRACBITS);
	write2bytes(d->f, mobj->y >> FRACBITS);
	write2bytes(d->f, mobj->z >> FRACBITS);
	write2bytes(d->f, mobj->angle / ANG1);
	fputc((unsigned char)mobj->type, d->f);
	write2bytes(d->f, (short)mobj->info->doomednum);
	write2bytes(d->f, (short)mobj->health);
	write2bytes(d->f, mobj->spawnpoint.x);
	write2bytes(d->f, mobj->spawnpoint.y);
	write2bytes(d->f, mobj->spawnpoint.angle);
	write2bytes(d->f, mobj->spawnpoint.options);
	write2bytes(d->f, mobj->spawnpoint.type);
}

// Dump real-time thing information
void cyb_DumpEncodeThings(dumpFile_t *d)
{
	thinker_t *currentthinker = NULL;
	int count = 0, c = 0;

	++d->recordCount;
	write2bytes(d->f, (short)DE_THINGS);
	write4bytes(d->f, totaltics);

	while ((currentthinker = P_NextThinker(currentthinker,th_all)) != NULL) {
		if ((currentthinker->function == P_MobjThinker)) {
			++count;
		}
	}

	write4bytes(d->f, count);
	currentthinker = NULL;
	while ((currentthinker = P_NextThinker(currentthinker,th_all)) != NULL) {
		if (currentthinker->function == P_MobjThinker ) {
			mobj_t *mobj = (mobj_t *)currentthinker;
			
			cyb_DumpEncodeThing(d, mobj);

		}
	}
}

// cybermind
// Alternative levelstat with deaths and totaltime count
void cyb_Levelstat2(void)
{
	int i;
	FILE *f;
	char str[200];
	char map[16];

	if (doSkip && (demo_stoponend || demo_warp))
		G_SkipDemoStop();

	if (!stats_level2)
		return;

	f = fopen("levelstat2.txt", "ab");
	fprintf(f, "\r\n");

	if (gamemode == commercial)
		sprintf(map, "MAP%02i   ", gamemap);
	else
		sprintf(map, "E%iM%i    ", gameepisode, gamemap);

	sprintf(str, "%%s%%5d:%%02d %%5d:%%02d %%4d %%5d / %%3d / %%3d\r\n");

	fprintf(f, str, map,
		totaltics / TICRATE / 60,
		(totaltics / TICRATE) % 60,
		leveltime / TICRATE / 60,
		(leveltime / TICRATE) % 60,
		players[0].deathscount,
		totalkills ? ((players[0].killcount - players[0].resurectedkillcount) * 100) / totalkills : 100,
		totalitems ? (players[0].itemcount * 100) / totalitems : 100,
		totalsecret ? (players[0].secretcount * 100) / totalsecret : 100
		);

	fclose(f);

	// clean-up
	for (i = 0; i < MAXPLAYERS; i++)
	{
		players[i].deathscount = 0;
	}
}

demo_comment_t *demo_comments;
demo_comment_t *last_comment;

void Cyb_AddNewComment(int tick, char* message)
{
	demo_comment_t *c = (demo_comment_t*)malloc(sizeof(demo_comment_t));
	c->ticknum = tick;
	c->message = message;
	c->next = NULL;
	if (last_comment) {
		c->prev = last_comment;
		last_comment = c;
	} else {
		c->prev = NULL;
		demo_comments = last_comment = c;
	}
}