B:=\
    $(DIR_SRC)/doom/doom \

O:=\
    $(DIR_SRC)/doom/am_map.o \
    $(DIR_SRC)/doom/cybermind.o \
    $(DIR_SRC)/doom/d_client.o \
    $(DIR_SRC)/doom/d_deh.o \
    $(DIR_SRC)/doom/d_items.o \
    $(DIR_SRC)/doom/d_main.o \
    $(DIR_SRC)/doom/dbopl.o \
    $(DIR_SRC)/doom/doomdef.o \
    $(DIR_SRC)/doom/doomstat.o \
    $(DIR_SRC)/doom/dstrings.o \
    $(DIR_SRC)/doom/e6y.o \
    $(DIR_SRC)/doom/f_finale.o \
    $(DIR_SRC)/doom/f_wipe.o \
    $(DIR_SRC)/doom/g_game.o \
    $(DIR_SRC)/doom/g_overflow.o \
    $(DIR_SRC)/doom/hu_lib.o \
    $(DIR_SRC)/doom/hu_stuff.o \
    $(DIR_SRC)/doom/hu_tracers.o \
    $(DIR_SRC)/doom/i_main.o \
    $(DIR_SRC)/doom/i_sound.o \
    $(DIR_SRC)/doom/i_system.o \
    $(DIR_SRC)/doom/i_video.o \
    $(DIR_SRC)/doom/info.o \
    $(DIR_SRC)/doom/lprintf.o \
    $(DIR_SRC)/doom/m_argv.o \
    $(DIR_SRC)/doom/m_bbox.o \
    $(DIR_SRC)/doom/m_cheat.o \
    $(DIR_SRC)/doom/m_menu.o \
    $(DIR_SRC)/doom/m_misc.o \
    $(DIR_SRC)/doom/m_random.o \
    $(DIR_SRC)/doom/md5.o \
    $(DIR_SRC)/doom/memio.o \
    $(DIR_SRC)/doom/midifile.o \
    $(DIR_SRC)/doom/mus2mid.o \
    $(DIR_SRC)/doom/opl.o \
    $(DIR_SRC)/doom/opl_queue.o \
    $(DIR_SRC)/doom/oplplayer.o \
    $(DIR_SRC)/doom/p_ceilng.o \
    $(DIR_SRC)/doom/p_checksum.o \
    $(DIR_SRC)/doom/p_doors.o \
    $(DIR_SRC)/doom/p_enemy.o \
    $(DIR_SRC)/doom/p_floor.o \
    $(DIR_SRC)/doom/p_genlin.o \
    $(DIR_SRC)/doom/p_inter.o \
    $(DIR_SRC)/doom/p_lights.o \
    $(DIR_SRC)/doom/p_map.o \
    $(DIR_SRC)/doom/p_maputl.o \
    $(DIR_SRC)/doom/p_mobj.o \
    $(DIR_SRC)/doom/p_plats.o \
    $(DIR_SRC)/doom/p_pspr.o \
    $(DIR_SRC)/doom/p_saveg.o \
    $(DIR_SRC)/doom/p_setup.o \
    $(DIR_SRC)/doom/p_sight.o \
    $(DIR_SRC)/doom/p_spec.o \
    $(DIR_SRC)/doom/p_switch.o \
    $(DIR_SRC)/doom/p_telept.o \
    $(DIR_SRC)/doom/p_tick.o \
    $(DIR_SRC)/doom/p_user.o \
    $(DIR_SRC)/doom/port.o \
    $(DIR_SRC)/doom/r_bsp.o \
    $(DIR_SRC)/doom/r_data.o \
    $(DIR_SRC)/doom/r_demo.o \
    $(DIR_SRC)/doom/r_draw.o \
    $(DIR_SRC)/doom/r_filter.o \
    $(DIR_SRC)/doom/r_fps.o \
    $(DIR_SRC)/doom/r_main.o \
    $(DIR_SRC)/doom/r_patch.o \
    $(DIR_SRC)/doom/r_plane.o \
    $(DIR_SRC)/doom/r_segs.o \
    $(DIR_SRC)/doom/r_sky.o \
    $(DIR_SRC)/doom/r_things.o \
    $(DIR_SRC)/doom/s_advsound.o \
    $(DIR_SRC)/doom/s_sound.o \
    $(DIR_SRC)/doom/sc_man.o \
    $(DIR_SRC)/doom/sounds.o \
    $(DIR_SRC)/doom/st_lib.o \
    $(DIR_SRC)/doom/st_stuff.o \
    $(DIR_SRC)/doom/tables.o \
    $(DIR_SRC)/doom/v_video.o \
    $(DIR_SRC)/doom/version.o \
    $(DIR_SRC)/doom/w_memcache.o \
    $(DIR_SRC)/doom/w_wad.o \
    $(DIR_SRC)/doom/wi_stuff.o \
    $(DIR_SRC)/doom/z_bmalloc.o \
    $(DIR_SRC)/doom/z_zone.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

DOOM_WARNINGS:=-Wno-misleading-indentation -Wno-pointer-sign -Wno-switch -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-value -Wno-unused-function -Wno-stringop-overflow -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -Wno-missing-braces -Wno-format

$(DIR_SRC)/doom/%.o: PP_FLAGS:=$(filter-out -std=c89 -pedantic, $(PP_FLAGS)) -std=c99 -DHAVE_CONFIG_H
$(DIR_SRC)/doom/%.o: CC_FLAGS:=$(filter-out -std=c89 -pedantic $(CC_FLAGS_FLOAT), $(CC_FLAGS)) -std=c99 -fcommon $(DOOM_WARNINGS)

include $(DIR_MK)/bin.mk

ROOTBIN:=$(ROOTBIN) $(B)
