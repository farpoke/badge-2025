
add_library(doom STATIC
        am_map.c
        d_items.c
        d_main.c
        d_net.c
        doomdef.c
        doomstat.c
        dstrings.c
        f_finale.c
        f_wipe.c
        g_game.c
        hu_lib.c
        hu_stuff.c
        # i_main.c
        # i_net.c
        # i_sound.c
        # i_system.c
        # i_video.c
        info.c
        m_argv.c
        m_bbox.c
        m_cheat.c
        m_fixed.c
        m_menu.c
        m_misc.c
        m_random.c
        m_swap.c
        p_ceilng.c
        p_doors.c
        p_enemy.c
        p_floor.c
        p_inter.c
        p_lights.c
        p_map.c
        p_maputl.c
        p_mobj.c
        p_plats.c
        p_pspr.c
        p_saveg.c
        p_setup.c
        p_sight.c
        p_spec.c
        p_switch.c
        p_telept.c
        p_tick.c
        p_user.c
        r_bsp.c
        r_data.c
        r_draw.c
        r_main.c
        r_plane.c
        r_segs.c
        r_sky.c
        r_things.c
        # s_sound.c
        # sounds.c
        st_lib.c
        st_stuff.c
        tables.c
        v_video.c
        w_wad.c
        wi_stuff.c
        z_zone.c

        rp2040/launch_doom.c
        rp2040/network.c
        rp2040/no_sound.c
        rp2040/system.cpp
        rp2040/video.cpp
)

target_include_directories(doom PRIVATE "${CMAKE_CURRENT_LIST_DIR}/rp2040")

target_link_libraries(doom PUBLIC
        pico_time
)

target_compile_options(doom PRIVATE -Os -g -Wall -Werror)
