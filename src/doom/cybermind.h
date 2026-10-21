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

#ifndef _CYBERMIND__
#define _CYBERMIND__

extern int stats_level2;
extern int dump_things;
extern  int totaltics;
extern  int totaldemotics;
extern  int alternative_kills_counter;
extern int demo_playvoice;

extern void cyb_Levelstat2(void);
extern void cyb_DumpThings(void);

typedef struct demo_comment_s
{
	int ticknum;
	char* message;
	struct demo_comment_s* prev;
	struct demo_comment_s* next;
}demo_comment_t;

extern demo_comment_t *demo_comments;
extern demo_comment_t *last_comment;

/*
---------------------------------------
Dump file handling
---------------------------------------
*/

enum dumpEvents {
	DE_NONE = 0,
	DE_THINGS,
	DE_PLAYERSTAT,
	DE_SPAWN,
	DE_DAMAGE,
	DE_KILL,
};

typedef struct dumpFile_s
{
	FILE *f;
	int recordCount;
} dumpFile_t;

extern dumpFile_t *dumpFile;

extern dumpFile_t* cyb_DumpStart(const char *file);
extern void cyb_DumpEnd(dumpFile_t * d);
extern void cyb_DumpEncodeThing(dumpFile_t * d, mobj_t *mobj);
extern void cyb_DumpEncodeThings(dumpFile_t *d);
extern void cyb_DumpEncodePlayers(dumpFile_t *d);
extern void cyb_DumpSpawn(dumpFile_t *d, mobj_t *s);
extern void cyb_DumpDamage(dumpFile_t *d, mobj_t *target, mobj_t *inflictor, int damage);
extern void cyb_DumpKill(dumpFile_t *d, mobj_t *target, mobj_t *who);

extern void write2bytes(FILE *file, unsigned short data);
extern void write4bytes(FILE *file, unsigned int data);

#endif
