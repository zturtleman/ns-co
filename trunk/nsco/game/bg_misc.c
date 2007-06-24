// Copyright (C) 1999-2000 Id Software, Inc.
//
// bg_misc.c -- both games misc functions, all completely stateless

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "q_shared.h"
#include "bg_public.h"

/*QUAKED item_***** ( 0 0 0 ) (-16 -16 -16) (16 16 16) suspended
DO NOT USE THIS CLASS, IT JUST HOLDS GENERAL INFORMATION.
The suspended flag will allow items to hang in the air, otherwise they are dropped to the next surface.

If an item is the target of another entity, it will not spawn in until fired.

An item fires all of its targets when it is picked up.  If the toucher can't carry it, the targets won't be fired.

"notfree" if set to 1, don't spawn in free for all games
"notteam" if set to 1, don't spawn in team games
"notsingle" if set to 1, don't spawn in single player games
"wait"	override the default wait before respawning.  -1 = never respawn automatically, which can be used with targeted spawning.
"random" random number of plus or minus seconds varied from the respawn time
"count" override quantity or duration on most items.
*/

gitem_t	bg_itemlist[] =
    {
        {
            NULL,
            NULL,
            { NULL,
              NULL,
              0, 0} ,
            /* icon */		NULL,
            /* pickup */	NULL,
            0,
            0,
            0,
            0,
            /* precache */ "",
            /* sounds */ ""
        },	// leave index 0 alone

        //
        // ARMOR
        //
#if 0
        /*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "item_armor_shard",
            "sound/misc/ar1_pkup.wav",
            { "models/powerups/armor/shard.md3",
              "models/powerups/armor/shard_sphere.md3",
              0, 0} ,
            /* icon */		"icons/iconr_shard",
            /* pickup */	"Armor Shard",
            5,
            IT_ARMOR,
            0,
            0,
            /* precache */ "",
            /* sounds */ ""
        }, */
#endif
        /*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "item_armor_combat",
            "",
            { "",
              0, 0, 0},
            /* icon */		"icons/armor/vest",
            /* pickup */	"Kevlar Vest",
            1,
            IT_ARMOR,
            PW_VEST,
            0,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "item_armor_body",
            "",
            { "", // fix me to: models/playe
              0, 0, 0},
            /* icon */		"icons/armor/helmet",
            /* pickup */	"Helmet",
            1,
            IT_ARMOR,
            PW_HELMET,
            0,
            /* precache */ "",
            /* sounds */ ""
        },

        //
        // health
        //

        {
            "item_botroam",
            "sound/items/s_health.wav",
            { "models/powerups/health/small_cross.md3",
              "models/powerups/health/small_sphere.md3",
              0, 0 },
            /* icon */		"icons/iconh_green",
            /* pickup */	"Bot Roam",
            5,
            IT_BOTROAM,
            0,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
#if 0
        /*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "item_health_small",
            "sound/items/s_health.wav",
            { "models/powerups/health/small_cross.md3",
              "models/powerups/health/small_sphere.md3",
              0, 0 },
            /* icon */		"icons/iconh_green",
            /* pickup */	"5 Health",
            5,
            IT_HEALTH,
            0,
            0,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "item_health",
            "sound/items/n_health.wav",
            { "models/powerups/health/medium_cross.md3",
              "models/powerups/health/medium_sphere.md3",
              0, 0 },
            /* icon */		"icons/iconh_yellow",
            /* pickup */	"25 Health",
            25,
            IT_HEALTH,
            0,
            0,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "item_health_large",
            "sound/items/l_health.wav",
            { "models/powerups/health/large_cross.md3",
              "models/powerups/health/large_sphere.md3",
              0, 0 },
            /* icon */		"icons/iconh_red",
            /* pickup */	"50 Health",
            50,
            IT_HEALTH,
            0,
            0,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "item_health_mega",
            "sound/items/m_health.wav",
            { "models/powerups/health/mega_cross.md3",
              "models/powerups/health/mega_sphere.md3",
              0, 0 },
            /* icon */		"icons/iconh_mega",
            /* pickup */	"Mega Health",
            100,
            IT_HEALTH,
            0,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
#endif

        //
        // WEAPONS
        //

        /*QUAKED weapon_mac10 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_mac10",
            "sound/misc/w_pkup.wav",
            { "models/weapons/mac10/mac10.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/mac10",
            /* pickup */	"Ingram Mac-10",
            0,
            IT_WEAPON,
            WP_MAC10,
            AM_SMG,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED misc_c4 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "misc_c4",
            "sound/misc/w_pkup.wav",
            { "models/weapons/c4/c4.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/c4",
            /* pickup */	"c4",
            0,
            IT_WEAPON,
            WP_C4,
            0,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED weapon_khurkuri (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_khurkuri",
            "sound/misc/w_pkup.wav",
            { "models/weapons/knife_t/knife_t.md3",
              0, 0, 0 },
            /* icon */		"icons/weapons/knife_t",
            /* pickup */	"Khurkuri",
            0,
            IT_WEAPON,
            WP_KHURKURI,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
        /*QUAKED weapon_sealknife (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_sealknife",
            "sound/misc/w_pkup.wav",
            { "models/weapons/knife_s/knife_s.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/knife_s",
            /* pickup */	"Seal Knife",
            0,
            IT_WEAPON,
            WP_SEALKNIFE,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
#if 0
        /*
        /*QUAKED weapon_rpg (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_rpg",
            "sound/misc/w_pkup.wav",
            { "models/weapons2/rocketl/rocketl.md3",
              0, 0, 0},
            /* icon */		"icons/iconw_rocket",
            /* pickup */	"Rocket Launcher",
            0,
            IT_WEAPON,
            WP_ROCKET_LAUNCHER,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
        */
#endif
        /*QUAKED weapon_mk23 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_mk23",
            "sound/misc/w_pkup.wav",
            { "models/weapons/mk23/mk23.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/mk23",
            /* pickup */	"H&K Mk 23",
            10,
            IT_WEAPON,
            WP_MK23,
            AM_MEDIUM_PISTOL,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED weapon_p9s (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_p9s",
            "sound/misc/w_pkup.wav",
            { "models/weapons/p9s/p9s.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/p9s",
            /* pickup */	"H&K P9s",
            10,
            IT_WEAPON,
            WP_P9S,
            AM_LIGHT_PISTOL,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED weapon_sw40t (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_sw40t",
            "sound/misc/w_pkup.wav",
            { "models/weapons/sw40t/sw40t.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/sw40t",
            /* pickup */	"S&W 40t",
            10,
            IT_WEAPON,
            WP_SW40T,
            AM_MEDIUM_PISTOL,
            /* precache */ "",
            /* sounds */ ""
        },


        /*QUAKED weapon_deagle (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_deagle",
            "sound/misc/w_pkup.wav",
            { "models/weapons/deagle/deagle.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/deagle",
            /* pickup */	"Desert Eagle",
            10,
            IT_WEAPON,
            WP_DEAGLE,
            AM_LARGE_PISTOL,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED weapon_sw629 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_sw629",
            "sound/misc/w_pkup.wav",
            { "models/weapons/sw629/sw629.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/sw629",
            /* pickup */	"S&W 629",
            10,
            IT_WEAPON,
            WP_SW629,
            AM_LARGE_PISTOL,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED weapon_glock (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_glock",
            "sound/misc/w_pkup.wav",
            { "models/weapons/glock30/glock30.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/glock30",
            /* pickup */	"Glock 26",
            10,
            IT_WEAPON,
            WP_GLOCK,
            AM_LIGHT_PISTOL,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED weapon_psg1 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_psg1",
            "sound/misc/w_pkup.wav",
            { "models/weapons/psg-1/psg-1.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/psg1",
            /* pickup */	"PSG-1",
            10,
            IT_WEAPON,
            WP_PSG1,
            AM_LARGE_SNIPER,
            /* precache */ "",
            /* sounds */ ""
        },
#ifdef SL8SD
        /*QUAKED weapon_sl8sd (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_sl8sd",
            "sound/misc/w_pkup.wav",
            { "models/weapons/sl8sd/sl8sd.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/sl8sd",
            /* pickup */	"Heckler & Koch Sl8sd",
            10,
            IT_WEAPON,
            WP_SL8SD,
            AM_MEDIUM_SNIPER,
            /* precache */ "",
            /* sounds */ ""
        },
#endif

        /*QUAKED weapon_macmillan (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_macmillan",
            "sound/misc/w_pkup.wav",
            { "models/weapons/macmillan/macmillan.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/macmillan",
            /* pickup */	"MacMillan",
            10,
            IT_WEAPON,
            WP_MACMILLAN,
            AM_LARGE_SNIPER,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED weapon_ak47 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_ak47",
            "sound/misc/w_pkup.wav",
            { "models/weapons/ak47/ak47.MD3",
              0, 0, 0},
            /* icon */		"icons/weapons/ak47",
            /* pickup */	"AK 47 .233",
            40,
            IT_WEAPON,
            WP_AK47,
            AM_RIFLE,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED weapon_m4 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_m4",
            "sound/misc/w_pkup.wav",
            {
                "models/weapons/m4/m4.md3",
                0, 0, 0 },
            "icons/weapons/m4",
            "SOCOM M4A1",
            10,
            IT_WEAPON,
            WP_M4,
            AM_RIFLE,
            "",
            "",
        },

        /*QUAKED weapon_m249 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_m249",
            "sound/misc/w_pkup.wav",
            {
                "models/weapons/m249/m249.md3",
                0, 0, 0 },
            "icons/weapons/m249",
            "Minimi Para M249",
            10,
            IT_WEAPON,
            WP_M249,
            AM_MG,
            "",
            "",
        },

        /*QUAKED weapon_spas15 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_spas15",
            "sound/misc/w_pkup.wav",
            { "models/weapons/spas15/spas15.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/spas15",
            /* pickup */	"Spas 15",
            10,
            IT_WEAPON,
            WP_SPAS15,
            AM_SHOTGUNMAG,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED weapon_m14 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */

        {
            "weapon_m14",
            "sound/misc/w_pkup.wav",
            {
                "models/weapons/m14/m14.md3",
                0,
                0,
                0
            },
            /* icon */ "icons/weapons/m14",
            /* pickup name */ "M14",
            1,
            IT_WEAPON,
            WP_M14,
            AM_MEDIUM_SNIPER,
            /* precache */ "",
            /* sounds */ ""
        },


        /*QUAKED weapon_870 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_870",
            "sound/misc/w_pkup.wav",
            { "models/weapons/870/870.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/870",
            /* pickup */	"Remington 870",
            10,
            IT_WEAPON,
            WP_870,
            AM_SHOTGUN,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED weapon_m590 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_m590",
            "sound/misc/w_pkup.wav",
            { "models/weapons/m590/m590.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/m590",
            /* pickup */	"Mossberg 590",
            10,
            IT_WEAPON,
            WP_M590,
            AM_SHOTGUN,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED weapon_uzi (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_pwd",
            "sound/misc/w_pkup.wav",
            { "models/weapons/pdw/pdw.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/pdw",
            /* pickup */	"PDW 90",
            2,
            IT_WEAPON,
            WP_PDW,
            AM_SMG,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED weapon_mp5sd (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_mp5sd",
            "sound/misc/w_pkup.wav",
            { "models/weapons/mp5/mp5.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/mp5",
            /* pickup */	"Heckler & Koch MP5N",
            2,
            IT_WEAPON,
            WP_MP5,
            AM_SMG,
            /* precache */ "",
            /* sounds */ ""
        },



        /*QUAKED weapon_mk26 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_mk26",
            "sound/misc/w_pkup.wav",
            { "models/weapons/mk26/mk26.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/mk26",
            /* pickup */	"Mk26 Grenade",
            1,
            IT_WEAPON,
            WP_GRENADE,
            AM_GRENADES,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED weapon_smoke (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_smoke",
            "sound/misc/w_pkup.wav",
            { "models/weapons/smoke/smoke.md3",
              //        { "models/weapons/smoke/smoke.md3",
              0, 0, 0},
            ///* icon */		"icons/weapons/flashbang",
            /* icon */		"icons/weapons/smoke",
            /* pickup */	"Smoke Grenade",
            1,
            IT_WEAPON,
            WP_SMOKE,
            AM_SMOKE,
            /* precache */ "sound/misc/gas.wav",
            /* sounds */ ""
        },


        /*QUAKED weapon_flash (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_flash",
            "sound/misc/w_pkup.wav",
            { "models/weapons/flashbang/flashbang.md3",
              0, 0, 0},
            /* icon */		"icons/weapons/flashbang",
            /* pickup */	"Flashbang",
            1,
            IT_WEAPON,
            WP_FLASHBANG,
            AM_FLASHBANGS,
            /* precache */ "",
            /* sounds */ ""
        },
#if 0 // old id shit
        /*QUAKED weapon_grapplinghook (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "weapon_grapplinghook",
            "sound/misc/w_pkup.wav",
            { "models/weapons2/grapple/grapple.md3",
              0, 0, 0},
            /* icon */		"icons/iconw_grapple",
            /* pickup */	"Grappling Hook",
            0,
            IT_WEAPON,
            WP_GRAPPLING_HOOK,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
#endif

        //
        // AMMO ITEMS
        //


        /*QUAKED ammo_9mmclips (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "ammo_shells",
            "sound/misc/am_pkup.wav",
            { "models/powerups/ammo/machinegunam.md3",
              0, 0, 0},
            /* icon */		"icons/ammo/9mmclip",
            /* pickup */	"9mm Pistol Clip",
            1,
            IT_AMMO,
            AM_LIGHT_PISTOL,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
        /*QUAKED ammo_9mmclips (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "ammo_45cal_pistol",
            "sound/misc/am_pkup.wav",
            { "models/powerups/ammo/machinegunam.md3",
              0, 0, 0},
            /* icon */		"icons/ammo/9mmclip",
            /* pickup */	".45 Pistol Clip",
            1,
            IT_AMMO,
            AM_LIGHT_PISTOL,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
        /*QUAKED ammo_9mmclips (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "ammo_40cal_pistol",
            "sound/misc/am_pkup.wav",
            { "models/powerups/ammo/machinegunam.md3",
              0, 0, 0},
            /* icon */		"icons/ammo/9mmclip",
            /* pickup */	".40 Pistol Clip",
            1,
            IT_AMMO,
            AM_MEDIUM_PISTOL,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
        /*QUAKED ammo_9mmclips (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "ammo_50cal_pistol",
            "sound/misc/am_pkup.wav",
            { "models/powerups/ammo/machinegunam.md3",
              0, 0, 0},
            /* icon */		"icons/ammo/9mmclip",
            /* pickup */	".50 Pistol Clip",
            1,
            IT_AMMO,
            AM_LARGE_PISTOL,
            0,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED ammo_9mmclips (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "ammo_44cal_pistol",
            "sound/misc/am_pkup.wav",
            { "models/powerups/ammo/machinegunam.md3",
              0, 0, 0},
            /* icon */		"icons/ammo/9mmclip",
            /* pickup */	".44 Pistol Clip",
            1,
            IT_AMMO,
            AM_MEDIUM_PISTOL,
            0,
            /* precache */ "",
            /* sounds */ ""
        },


        /*QUAKED ammo_45mm_clip (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "ammo_46mm_clip",
            "sound/misc/am_pkup.wav",
            { "models/powerups/ammo/machinegunam.md3",
              0, 0, 0},
            /* icon */		"icons/ammo/46mmclip",
            /* pickup */	"4.6mm Clip",
            1,
            IT_AMMO,
            AM_SMG,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
        /*QUAKED ammo_9mm_clip (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "ammo_9mm_clip",
            "sound/misc/am_pkup.wav",
            { "models/powerups/ammo/machinegunam.md3",
              0, 0, 0},
            /* icon */		"icons/ammo/9mmclip",
            /* pickup */	"9mm Clip",
            1,
            IT_AMMO,
            AM_SMG,
            0,
            /* precache */ "",
            /* sounds */ ""
        },

        /*QUAKED ammo_9mm_clip (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "ammo_556mm_clip",
            "sound/misc/am_pkup.wav",
            { "models/powerups/ammo/machinegunam.md3",
              0, 0, 0},
            /* icon */		"icons/ammo/556mmclip",
            /* pickup */	"5.56mm Clip",
            1,
            IT_AMMO,
            AM_RIFLE,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
        /*QUAKED ammo_9mm_clip (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "ammo_762mm_clip",
            "sound/misc/am_pkup.wav",
            { "models/powerups/ammo/machinegunam.md3",
              0, 0, 0},
            /* icon */		"icons/ammo/762mmclip",
            /* pickup */	"7.62mm Clip",
            1,
            IT_AMMO,
            AM_LARGE_SNIPER,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
        /*QUAKED ammo_45_clip (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "ammo_45_clip",
            "sound/misc/am_pkup.wav",
            { "models/powerups/ammo/machinegunam.md3",
              0, 0, 0},
            /* icon */		"icons/ammo/45calclip",
            /* pickup */	".45 Clip",
            1,
            IT_AMMO,
            AM_RIFLE,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
        /*QUAKED ammo_50_clip (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "ammo_50_clip",
            "sound/misc/am_pkup.wav",
            { "models/powerups/ammo/machinegunam.md3",
              0, 0, 0},
            /* icon */		"icons/ammo/50calclip",
            /* pickup */	".50 Clip",
            1,
            IT_AMMO,
            AM_LARGE_SNIPER,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
        /*QUAKED ammo_m257_clip (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "ammo_m257",
            "sound/misc/am_pkup.wav",
            { "models/powerups/ammo/machinegunam.md3",
              0, 0, 0},
            /* icon */		"icons/ammo/m257shells",
            /* pickup */	"M257 Shells",
            8,
            IT_AMMO,
            AM_MG,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
        /*QUAKED ammo_xm162_clip (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "ammo_xm162",
            "sound/misc/am_pkup.wav",
            { "models/powerups/ammo/machinegunam.md3",
              0, 0, 0},
            /* icon */		"icons/ammo/xm162shells",
            /* pickup */	"XM162 Shells",
            8,
            IT_AMMO,
            AM_SHOTGUNMAG,
            0,
            /* precache */ "",
            /* sounds */ ""
        },

#if 0
        /*QUAKED ammo_flechets_clip (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "ammo_flechets",
            "sound/misc/am_pkup.wav",
            { "models/powerups/ammo/machinegunam.md3",
              0, 0, 0},
            /* icon */		"icons/ammo/flechetshells",
            /* pickup */	"Flechet Shells",
            8,
            IT_AMMO,
            AM_FLECHETS,
            0,
            /* precache */ "",
            /* sounds */ ""
        },
#endif

        /*QUAKED ammo_556mmclips (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
        */
        {
            "ammo_cells",
            "sound/misc/am_pkup.wav",
            { "models/powerups/ammo/shotgunam.md3",
              0, 0, 0},
            /* icon */		"icons/ammo/40mmgrenade",
            /* pickup */	"40mm Grenades",
            1,
            IT_AMMO,
            AM_40MMGRENADES,
            0,
            /* precache */ "",
            /* sounds */ ""
        },



        /*QUAKED team_briefcase (1 0 0) (-16 -16 -16) (16 16 16)
        intelligence to capture for seals....
        */
        {
            "team_briefcase",
            "sound/misc/am_pkup.wav", // briefcase stolen sound
            { "models/misc/suitcase/suitcase.md3",
              0, 0, 0 },
            /* icon */		"icons/weapons/icon_suitcase",
            /* pickup */	"Briefcase",
            0,
            IT_TEAM,
            PW_BRIEFCASE,
            0,
            /* precache */ "",
            /* sounds */ "",
        },

        /*QUAKED team_briefcase2 (1 0 0) (-16 -16 -16) (16 16 16)
        intelligence to capture for seals....
        */
        {
            "team_briefcase2",
            "sound/misc/am_pkup.wav", // briefcase stolen sound
            { "models/misc/suitcase/suitcase2.md3",
              0, 0, 0 },
            /* icon */		"icons/weapons/icon_suitcase2",
            /* pickup */	"Briefcase",
            0,
            IT_TEAM,
            PW_BRIEFCASE,
            0,
            /* precache */ "",
            /* sounds */ "",
        },


        /*QUAKED team_briefcase_return (1 0 0) (-16 -16 -16) (16 16 16)
        point where it needs to get returned
        */
        {
            "team_briefcase_return",
            "sound/misc/am_pkup.wav",
            { "null.md3",
              0, 0, 0 },
            /* icon */		"icons/weapons/icon_suitcase",
            /* pickup */	"Briefcase Returnpoint",
            0,
            IT_TEAM,
            PW_BRIEFCASE_RETURN,
            0,
            /* precache */ "",
            /* sounds */ ""
        },

        // end of list marker
        {NULL}
    };

int		bg_numItems = sizeof(bg_itemlist) / sizeof(bg_itemlist[0]) - 1;

float BG_GetSpeedMod( int techLevel )
{
    float speedMod = 1.0f;

    switch ( techLevel ) {
    case 1:
        speedMod = 1.0f;
        break;
    case 2:
    case 3:
        speedMod = 0.8f;
        break;
    case 4:
        speedMod = 0.7f;
        break;
    case 5:
    case 6:
        speedMod = 0.6f;
        break;
    case 7:
    case 8:
    case 9:
        speedMod = 0.5f;
        break;
    case 10:
        speedMod = 0.4f;
        break;
    }

    return speedMod;
}

qboolean BG_IsInGLMode( int weaponmode )
{
    if (
        weaponmode & ( 1 << WM_WEAPONMODE2 ) &&
        weaponmode & ( 1 << WM_GRENADELAUNCHER )
    )
        return qtrue;
    return qfalse;

}
qboolean BG_IsZooming( int weaponmode )
{
    if ( weaponmode & ( 1 << WM_ZOOM4X) || weaponmode & (1 << WM_ZOOM2X))
        return qtrue;

    return qfalse;
}
qboolean BG_HasLaser( int weaponmode )
{
    if ( weaponmode & ( 1 << WM_LASER ) )
        return qtrue;

    return qfalse;
}


// BLUTENGEL
// gives a flag how fast a player is
// used to calculate weapon recoil
int BG_CalcSpeed( playerState_t ps ) {
    float spd = sqrt( ps.velocity[0]*ps.velocity[0] +
                      ps.velocity[1]*ps.velocity[1] +
                      ps.velocity[2]*ps.velocity[2]);

    // jumping or falling
    if ( (ps.velocity[2] > 2.0 ||
            ps.velocity[2] < -2.0 ) &&
            (spd >= SEALS_SPEED_WALKING) &&
            ps.groundEntityNum == ENTITYNUM_NONE) return SEALS_JUMPING;

    // standing
    if (spd < SEALS_SPEED_STANDING) return SEALS_STANDING;

    // crouching
    if (ps.pm_flags & PMF_DUCKED) return SEALS_CROUCHING;

    // walking
    if ( spd < SEALS_SPEED_WALKING ) return SEALS_WALKING;

    // sprinting
    if ( (spd >= SEALS_SPEED_RUNNING) &&
            (spd < SEALS_SPEED_SPRINTING) ) return SEALS_SPRINTING;

    // running
    return SEALS_RUNNING;
}

/*
=================
NSQ3 MaximumWeaponRange
author: Defcon-X
date: 24-03-2001
description: calculates damage , based on range, input: hitpoint, endpoint, damage out: damage
================
*/
float BG_MaximumWeaponRange( int weapon )
{
    gitem_t *it_weapon;
    int ammotype;
    float range;

    if ( !weapon )
        return -1;

    it_weapon = BG_FindItemForWeapon( weapon );

    ammotype = it_weapon->giAmmoTag;

    switch ( ammotype )
    {

        // pistols
    case AM_LIGHT_PISTOL:
        range = 3200;
        break;
    case AM_MEDIUM_PISTOL:
        range = 3840;
        break;
    case AM_LARGE_PISTOL:
        range = 2880;
        break;

        // others
    case AM_SMG:
        range = 4800;
        break;
    case AM_RIFLE:
        range = 12800;
        break;
    case AM_MEDIUM_SNIPER:
        range = 19200;
        break;
    case AM_LARGE_SNIPER:
        range = 32000;
        break;
    case AM_MG:
        range = 9600;
        break;
    case AM_SHOTGUNMAG:
        range = 3200;
        break;
    case AM_SHOTGUN:
        range = 3200;
        break;
        //	case AM_FLECHETS:
        //	range = 6400;
        //		break;
    default:
        range = -1;
        break;
    }

    return range;
    /*
    - 9mm Pistole/Revolver		100		3200
    - .45 Pistole/Revolver		90		2880
    - .40 Pistole/Revolver		100		3200
    - .50 Pistole/Revolver 		90		2880
    - .44 Pistole/Revolver 		120		3840
    - 4.6mm normal				200		6400
    - 9mm normal				125		4000
    - 5,56mm normal				400		12800
    - 7,62mm normal				600		19200
    - .45mm normal				100		3200
    - .50 normal				2000	64000
    - M257 shotgun		  		150		4800
    - XM162 shotgun				80		2560
    - flechets shotgun			200		6400

    */
}
void BG_PackWeapon( int weapon, int stats[ ] )
{
    unsigned int  weaponList;
    // create one big list with from our 2
    weaponList = (unsigned int)stats[ STAT_WEAPONS ] | ((unsigned int)stats[ STAT_WEAPONS2 ] << 16 );
    // add that bit to our "bigger list"
    weaponList |= ( 1 << weapon );
    // statweapons 1 is the lower half part ( 0-15)
    stats[ STAT_WEAPONS ] = weaponList & 0x0000FFFF;
    // weapons2 is the upper part ( 17-31 )
    stats[ STAT_WEAPONS2 ] = ( weaponList & 0xFFFF0000 ) >> 16;
}

void BG_ClearWeapons( int stats[ ] )
{
    stats[ STAT_WEAPONS ] = stats[ STAT_WEAPONS2 ] = 0;
}

// the old version didn't work with qvms but it works w/o problems in dll mode.
// we have to erase the old weaponstats and have
// to add all weapons except the one we don't want
void BG_RemoveWeapon( int weapon, int stats[ ] )
{
    int  weaponList;
    int  i;

    // create the big list
    weaponList = (unsigned int)stats[ STAT_WEAPONS ] | ( (unsigned int)stats[ STAT_WEAPONS2 ] << 16 );

    BG_ClearWeapons( stats );

    //	Com_Printf("cleared stats:%i , adding previous except %i\n", weaponList, weapon );
    for ( i = 0; i < WP_NUM_WEAPONS; i++ )
    {
        if ( weaponList & ( 1 << i ) && i != weapon )
        {
            //			Com_Printf("packed weapon: %i\n",i );
            BG_PackWeapon( i, stats );
        }
    }

    //stats[ STAT_WEAPONS ] = weaponList & 0x0000FFFF;
    //stats[ STAT_WEAPONS2 ] = ( weaponList & 0xFFFF0000 ) >> 16;
}

// returns surfacetype as string
char *BG_SurfaceToString( int surfaceFlags )
{
    if ( surfaceFlags & SURF_METALSTEPS	)
        return "Metal";
    else if ( surfaceFlags & SURF_WOODSTEPS )
        return "Wood";
    else if ( surfaceFlags & SURF_DIRTSTEPS )
        return "Dirt/Grass";
    else if ( surfaceFlags & SURF_SNOWSTEPS )
        return "Snow";
    else if ( surfaceFlags & SURF_SANDSTEPS )
        return "Sand";
    else if ( surfaceFlags & SURF_GLASS )
        return "Glass";
    else if ( surfaceFlags & SURF_SOFTSTEPS )
        return "Soft";
    else
        return "Concrete";
}

qboolean BG_GotWeapon( int weapon, int stats[ ] )
{
    if (weapon < 16 ) return (unsigned int)stats[STAT_WEAPONS] & (1 << weapon);

    return (unsigned int) stats[STAT_WEAPONS2] & ( 1 << (weapon - 16) );
}

int		BG_GetPrimary( int stats [ ] )
{
    int i;

    for ( i=WP_NUM_WEAPONS-1;i>WP_NONE;i--)
    {
        // so we have atleast one valid weapon.
        if ( BG_GotWeapon( i, stats ) )
        {
            if ( BG_IsPrimary( i ) )
                return i;
        }
    }
    return WP_NONE;
}
int		BG_GetSecondary( int stats [ ] )
{
    int i;

    for ( i=WP_NUM_WEAPONS-1;i>WP_NONE;i--)
    {
        // so we have atleast one valid weapon.
        if ( BG_GotWeapon( i, stats ) )
        {
            if ( BG_IsSecondary( i ) )
                return i;
        }
    }
    return WP_NONE;
}

/*
=================
BG_LeadGetBreakValueForSurface
author: dX
date: 25-05-02
return value: returns the breakvalue for the surface
=================
*/
float BG_LeadGetBreakValueForSurface( trace_t *tr )
{

    if ( tr->surfaceFlags & SURF_WOODSTEPS )
        return 6;
    else if ( tr->surfaceFlags & SURF_DIRTSTEPS )
        return 8;
    else if ( tr->surfaceFlags & SURF_SNOWSTEPS )
        return 16;
    else if ( tr->surfaceFlags & SURF_SANDSTEPS )
        return 6;
    else if ( tr->surfaceFlags & SURF_GLASS )
        return 12;
    else if ( tr->surfaceFlags & SURF_SOFTSTEPS )
        return 8;
    else if ( tr->surfaceFlags & SURF_METALSTEPS )
        return 2;
    else // concrete
        return 3;

}
/*
=================
NSQ3 Get Max Round For Weapon
author: dX
date: 15-02-99
return value: returns maxrounds for weap
=================
*/
int BG_GetMaxRoundForWeapon( int weapon )
{
    // fire the specific weapon
    switch( weapon ) {
    case WP_GLOCK:
        return 20;
    case WP_P9S:
        return 14;
    case WP_MK23:
        return 12;
    case WP_PDW:
        return 20;
    case WP_MP5:
        return 30;
    case WP_MAC10:
        return 45;
    case WP_M14:
        return 20;
    case WP_M4:
    case WP_AK47:
        return 30;
    case WP_SPAS15:
        return 9;
    case WP_870:
        return 7;
    case WP_MACMILLAN:
        return 5;
#ifdef SL8SD
    case WP_SL8SD:
        return 10;
#endif
    case WP_PSG1:
        return 5;
    case WP_SMOKE:
        return -1;
    case WP_GRENADE :
        return -1;
    case WP_FLASHBANG :
        return -1;
    case WP_SW40T:
        return 11;
    case WP_SW629:
        return 6;
    case WP_DEAGLE:
        return 7;
    case WP_M590:
        return 7;
    case WP_M249:
        return 200;
    }
    return 0;
}

qboolean BG_IsSemiAutomatic( int weapon )
{
    if (
        (BG_IsMelee( weapon ) ||
         BG_IsPistol( weapon ) ||
         BG_IsShotgun( weapon ) ||
         weapon == WP_PSG1 ||
#ifdef SL8SD
         weapon == WP_SL8SD ||
#endif
         weapon == WP_MACMILLAN
        ) /*&& weapon != WP_SPAS15*/ )
        return qtrue;
    return qfalse;
}

int BG_WeaponMods( int weapon )
{
    int wMods = 0;

    switch (weapon)
    {
    case WP_M14:
        wMods |= ( 1 << WM_SCOPE );
        break;
    case WP_M4:
    case WP_AK47:
        wMods |= ( 1 << WM_BAYONET );
        wMods |= ( 1 << WM_SCOPE );
        wMods |= ( 1 << WM_SILENCER );
        wMods |= ( 1 << WM_GRENADELAUNCHER );
        wMods |= ( 1 << WM_MUZZLEHIDER );
        break;
    case WP_PDW:
        //	wMods |= ( 1 << WM_SILENCER );
        break;
    case WP_MAC10:
    case WP_MP5: // mp5 got silencer
        wMods |= ( 1 << WM_SILENCER );
        wMods |= ( 1 << WM_LASER );
        break;
    case WP_P9S:
    case WP_GLOCK:
        wMods |= ( 1 << WM_SILENCER );
        wMods |= ( 1 << WM_LASER );
        break;
    case WP_SW40T:
    case WP_MK23:
        wMods |= ( 1 << WM_SILENCER );
        break;
    case WP_M590:
    case WP_870:
        wMods |= ( 1 << WM_FLASHLIGHT );
        wMods |= ( 1 << WM_DUCKBILL );
        break;
    default:
        wMods = 0;
        break;
    }
    return wMods;
}
/*
==============
BG_FindItemForPowerup
==============
*/
gitem_t	*BG_FindItemForPowerup( powerup_t pw ) {
    int		i;

    for ( i = 0 ; i < bg_numItems ; i++ ) {
        if ( bg_itemlist[i].giType == IT_POWERUP ||
                bg_itemlist[i].giType == IT_TEAM ) {
            return &bg_itemlist[i];
        }
    }

    return NULL;
}


/*
==============
BG_FindItemForHoldable
==============
*/
gitem_t	*BG_FindItemForHoldable( holdable_t pw ) {
    int		i;

    for ( i = 0 ; i < bg_numItems ; i++ ) {
        if ( bg_itemlist[i].giType == IT_HOLDABLE && bg_itemlist[i].giTag == pw ) {
            return &bg_itemlist[i];
        }
    }

    Com_Error( ERR_DROP, "HoldableItem not found" );

    return NULL;
}


/*
=================
NSQ3 BulletHoleTypeForSurface
author: Defcon-X
date: 25-03-2001
description: input: surface, output: bhole type
=================
*/
int NS_BulletHoleTypeForSurface( int surface )
{
    if ( surface & SURF_WOODSTEPS )
        return BHOLE_WOOD;
    else if ( surface & SURF_DIRTSTEPS )
        return BHOLE_DIRT;
    else if ( surface & SURF_SNOWSTEPS )
        return BHOLE_SNOW;
    else if ( surface & SURF_SANDSTEPS )
        return BHOLE_SAND;
    else if ( surface & SURF_METALSTEPS )
        return BHOLE_METAL;
    else if ( surface & SURF_GLASS )
        return BHOLE_GLASS;
    else if ( surface & SURF_SOFTSTEPS )
        return BHOLE_SOFT;

    return BHOLE_NORMAL;
}

/*
===============
BG_FindItemForWeapon

===============
*/
gitem_t	*BG_FindItemForWeapon( int weapon ) {
    gitem_t	*it;

    for ( it = bg_itemlist + 1 ; it->classname ; it++) {
        if ( it->giType == IT_WEAPON && it->giTag == weapon ) {
            return it;
        }
    }

    //Com_Printf( "Couldn't find item for weapon %i", weapon);
    return NULL;
}

/*
===============
BG_FindItem

===============
*/
gitem_t	*BG_FindItem( const char *pickupName ) {
    gitem_t	*it;

    for ( it = bg_itemlist + 1 ; it->classname ; it++ ) {
        if ( !Q_stricmp( it->pickup_name, pickupName ) )
            return it;
    }

    return NULL;
}

/*
============
BG_PlayerTouchesItem

Items can be picked up without actually touching their physical bounds to make
grabbing them easier
============
*/
qboolean	BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime ) {
    vec3_t		origin;

    BG_EvaluateTrajectory( &item->pos, atTime, origin );

    // we are ignoring ducked differences here
    if ( ps->origin[0] - origin[0] > 44
            || ps->origin[0] - origin[0] < -50
            || ps->origin[1] - origin[1] > 36
            || ps->origin[1] - origin[1] < -36
            || ps->origin[2] - origin[2] > 36
            || ps->origin[2] - origin[2] < -36 ) {
        return qfalse;
    }

    return qtrue;
}
// Navy Seals ++
/*
================
BG_GotPrimary

Returns true if player got a Primary weapon
================
*/

qboolean BG_GotPrimary ( const playerState_t *ps ) {
    if (BG_GotWeapon(WP_AK47 , (int*) ps->stats ) )
        return qtrue; // ak47 is an primary weapon
    if (BG_GotWeapon(WP_MP5 , (int*)ps->stats ) )
        return qtrue;
    if (BG_GotWeapon(WP_MAC10 , (int*)ps->stats ) )
        return qtrue;
    if (BG_GotWeapon(WP_PDW , (int*)ps->stats ) )
        return qtrue;
    if (BG_GotWeapon(WP_PSG1 , (int*)ps->stats ) )
        return qtrue;
#ifdef SL8SD
    if (BG_GotWeapon(WP_SL8SD, (int*)ps->stats ) )
        return qtrue;
#endif
    if (BG_GotWeapon(WP_MACMILLAN , (int*)ps->stats ) )
        return qtrue;
    if (BG_GotWeapon(WP_M4 , (int*)ps->stats ) )
        return qtrue;
    if (BG_GotWeapon(WP_870 , (int*)ps->stats ) )
        return qtrue;
    if (BG_GotWeapon(WP_M590, (int*)ps->stats ) )
        return qtrue;
    if (BG_GotWeapon(WP_SPAS15 , (int*)ps->stats ) )
        return qtrue;
    if (BG_GotWeapon(WP_M14, (int*)ps->stats ) )
        return qtrue;
    if (BG_GotWeapon(WP_M249, (int*)ps->stats ) )
        return qtrue;

    return qfalse;
}
/*
================
BG_GotSecondary

Returns true if player got a Secondary weapon
================
*/

qboolean BG_GotSecondary ( const playerState_t *ps ) {
    if (BG_GotWeapon(WP_GLOCK , (int*)ps->stats ) )
        return qtrue; // ak47 is an primary weapon
    if (BG_GotWeapon(WP_MK23 , (int*)ps->stats ) )
        return qtrue;
    if (BG_GotWeapon(WP_SW40T , (int*)ps->stats ) )
        return qtrue;
    if (BG_GotWeapon(WP_P9S, (int*)ps->stats ) )
        return qtrue;
    if (BG_GotWeapon(WP_DEAGLE, (int*)ps->stats ) )
        return qtrue;
    if (BG_GotWeapon(WP_SW629, (int*)ps->stats ) )
        return qtrue;

    return qfalse;
}

/*
================
BG_IsGrenade

Returns true if weapon is a grenade
================
*/

qboolean BG_IsGrenade ( int weapon ) {
    switch (weapon) {
    case WP_SMOKE:
    case WP_GRENADE:
    case WP_FLASHBANG:
        return qtrue;
    default:
        return qfalse;
    }
}


/*
================
BG_IsPrimary

Returns true if weapon is a Primary weapon
================
*/

qboolean BG_IsPrimary ( int weapon ) {
    switch (weapon) {
    case WP_AK47:
    case WP_MP5:
    case WP_PDW:
    case WP_MAC10:
    case WP_M4:
    case WP_PSG1:
    case WP_MACMILLAN:
    case WP_870:
    case WP_SPAS15:
    case WP_M14:
#ifdef SL8SD
    case WP_SL8SD:
#endif
    case WP_M590:
    case WP_M249:
        return qtrue;
    default:
        return qfalse;
    }
}

/*
================
BG_IsSMG

Returns true if weapon is a Primary weapon
================
*/

qboolean BG_IsSMG ( int weapon ) {
    switch (weapon) {
    case WP_MP5:
    case WP_MAC10:
        return qtrue;
    default:
        return qfalse;
    }
}

/*
================
BG_IsShotgun

Returns true if weapon is a Shotgun
================
*/
qboolean BG_IsShotgun ( int weapon ) {
    switch (weapon) {
    case WP_870:
    case WP_SPAS15:
    case WP_M590:
        return qtrue;
    default:
        return qfalse;
    }
}

/*
================
BG_IsRifle

Returns true if weapon is a Rifle
================
*/
qboolean BG_IsRifle ( int weapon ) {
    switch (weapon) {
    case WP_AK47:
    case WP_M14:
    case WP_M4:
    case WP_PSG1:
    case WP_MACMILLAN:
#ifdef SL8SD
    case WP_SL8SD:
#endif
    case WP_PDW:
    case WP_M249:
        return qtrue;
    default:
        return qfalse;
    }
}
/*
================
BG_IsPistol

Returns true if weapon is a Rifle
================
*/
qboolean BG_IsPistol ( int weapon ) {
    switch (weapon) {
    case WP_GLOCK:
    case WP_MK23:
    case WP_SW40T:
    case WP_P9S:
    case WP_SW629:
    case WP_DEAGLE:
        return qtrue;
    default:
        return qfalse;
    }
}
/*
================
BG_IsMelee

Returns true if weapon is a Rifle
================
*/
qboolean BG_IsMelee ( int weapon ) {
    switch (weapon) {
    case WP_KHURKURI:
    case WP_SEALKNIFE:
        return qtrue;
    default:
        return qfalse;
    }
}

/*
================
BG_IsSecondary

Returns true if weapon is a Secondary weapon
================
*/

qboolean BG_IsSecondary ( int weapon) {
    switch (weapon) {
    case WP_GLOCK:
    case WP_MK23:
    case WP_SW40T:
    case WP_P9S:
    case WP_SW629:
    case WP_DEAGLE:
        return qtrue;
    default:
        return qfalse;
    }
}
// Navy Seals --



/*
================
BG_CanItemBeGrabbed

Returns false if the item should not be picked up.
This needs to be the same for client side prediction and server use.
================
*/
qboolean BG_CanItemBeGrabbed( int gametype, const entityState_t *ent, const playerState_t *ps ) {
    gitem_t	*item;

    if ( ent->modelindex < 1 || ent->modelindex >= bg_numItems ) {
        Com_Error( ERR_DROP, "BG_CanItemBeGrabbed: index out of range" );
    }

    item = &bg_itemlist[ent->modelindex];

    // Navy Seals ++
    // if vip return
    if ( ps->eFlags & EF_VIP ) {
        if ( BG_IsPistol( item->giTag ) && !BG_GotSecondary( ps ))
            return qtrue;

        if ( item->giTag == PW_BRIEFCASE && !ps->powerups[PW_BRIEFCASE])
            return qtrue;

        return qfalse;
    }

    // Navy Seals --

    switch( item->giType ) {
        // Navy Seals ++
    case IT_WEAPON:
        {
            if ( BG_IsPrimary ( item->giTag ) && BG_GotPrimary( ps ) )
                return qfalse;
            if ( BG_IsSecondary ( item->giTag  ) && BG_GotSecondary( ps ) )
                return qfalse;

            if ( item->giTag == WP_FLASHBANG ) {
                if ( ps->ammo[AM_FLASHBANGS] < 2 )
                    return qtrue;
                else
                    return qfalse;
            }
            else if ( item->giTag == WP_SMOKE ) {
                if ( ps->ammo[AM_SMOKE] < 2 )
                    return qtrue;
                else
                    return qfalse;
            }
            else if ( item->giTag == WP_GRENADE ) {
                if ( ps->ammo[AM_GRENADES] < 2 )
                    return qtrue;
                else
                    return qfalse;
            }

            // already got a c4 - max is one
            if ( item->giTag == WP_C4 && ( BG_GotWeapon( WP_C4, (int*)ps->stats ) ) )
                return qfalse;

            return qtrue;
        }
        // Navy Seals --

    case IT_AMMO:
        // Navy Seals ++
        {
            int max = 3;

            switch( item->giTag ) {
            default:
                max = MAX_DEFAULT;
                break;
            }

            if ( ps->ammo[ item->giTag ] >= max ) {
                return qfalse;	// can't hold any more
            }

            return qtrue;
        }
        // Navy Seals --

    case IT_ARMOR:
        // Navy Seals ++
        {
            if ( ps->powerups[ item->giTag ] )
                return qfalse;

            return qtrue;
        }// Navy Seals --

    case IT_BOTROAM:
        return qfalse;
    case IT_HEALTH:
    case IT_POWERUP:
        // Navy Seals ++
        return qfalse;
    case IT_TEAM: // team items, such as flags
        if (item->giTag == PW_BRIEFCASE_RETURN) {
            if ( ps->powerups[PW_BRIEFCASE] && ps->persistant[PERS_TEAM] == TEAM_RED)
                return qtrue;
            return qfalse;
        }
        if (item->giTag == PW_BRIEFCASE) {
            if (ps->persistant[PERS_TEAM] == TEAM_RED && !ps->powerups[PW_BRIEFCASE])
                return qtrue;
            else
                return qfalse;
        }
        return qfalse;
    case IT_HOLDABLE:
        return qfalse;
    case IT_BAD:
        Com_Error( ERR_DROP, "BG_CanItemBeGrabbed: IT_BAD" );
    }

    return qfalse;
}

//======================================================================

/*
================
BG_EvaluateTrajectory

================
*/
void BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result ) {
    float		deltaTime;
    float		phase;

    switch( tr->trType ) {
    case TR_STATIONARY:
    case TR_INTERPOLATE:
        VectorCopy( tr->trBase, result );
        break;
    case TR_LINEAR:
        deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
        VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
        break;
    case TR_SINE:
        deltaTime = ( atTime - tr->trTime ) / (float) tr->trDuration;
        phase = sin( deltaTime * M_PI * 2 );
        VectorMA( tr->trBase, phase, tr->trDelta, result );
        break;
    case TR_LINEAR_STOP:
        if ( atTime > tr->trTime + tr->trDuration ) {
            atTime = tr->trTime + tr->trDuration;
        }
        deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
        if ( deltaTime < 0 ) {
            deltaTime = 0;
        }
        VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
        break;
    case TR_MOREGRAVITY:
        deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
        VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
        result[2] -= 0.5 * 1200 * deltaTime * deltaTime;		// FIXME: local gravity...
        break;
    case TR_GRAVITY:
        deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
        VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
        result[2] -= 0.5 * DEFAULT_GRAVITY * deltaTime * deltaTime;		// FIXME: local gravity...
        break;
    default:
        Com_Error( ERR_DROP, "BG_EvaluateTrajectory: unknown trType: %i", tr->trTime );
        break;
    }
}

/*
================
BG_EvaluateTrajectoryDelta

For determining velocity at a given time
================
*/
void BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result ) {
    float	deltaTime;
    float	phase;

    switch( tr->trType ) {
    case TR_STATIONARY:
    case TR_INTERPOLATE:
        VectorClear( result );
        break;
    case TR_LINEAR:
        VectorCopy( tr->trDelta, result );
        break;
    case TR_SINE:
        deltaTime = ( atTime - tr->trTime ) / (float) tr->trDuration;
        phase = cos( deltaTime * M_PI * 2 );	// derivative of sin = cos
        phase *= 0.5;
        VectorScale( tr->trDelta, phase, result );
        break;
    case TR_LINEAR_STOP:
        if ( atTime > tr->trTime + tr->trDuration ) {
            VectorClear( result );
            return;
        }
        VectorCopy( tr->trDelta, result );
        break;
    case TR_MOREGRAVITY:
        deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
        VectorCopy( tr->trDelta, result );
        result[2] -= 1200 * deltaTime;		// FIXME: local gravity...
        break;
    case TR_GRAVITY:
        deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
        VectorCopy( tr->trDelta, result );
        result[2] -= DEFAULT_GRAVITY * deltaTime;		// FIXME: local gravity...
        break;
    default:
        Com_Error( ERR_DROP, "BG_EvaluateTrajectoryDelta: unknown trType: %i", tr->trTime );
        break;
    }
}

char *eventnames[] = {
                         "EV_NONE",

                         "EV_FOOTSTEP",
                         "EV_FOOTSTEP_METAL",
                         "EV_FOOTSPLASH",
                         "EV_FOOTWADE",
                         "EV_SWIM",

                         "EV_STEP_4",
                         "EV_STEP_8",
                         "EV_STEP_12",
                         "EV_STEP_16",

                         "EV_FALL_SHORT",
                         "EV_FALL_MEDIUM",
                         "EV_FALL_FAR",

                         "EV_STOLENWEAPON",
                         "EV_JUMP_PAD",			// boing sound at origin", jump sound on player

                         "EV_JUMP",
                         "EV_WATER_TOUCH",	// foot touches
                         "EV_WATER_LEAVE",	// foot leaves
                         "EV_WATER_UNDER",	// head touches
                         "EV_WATER_CLEAR",	// head leaves

                         "EV_ITEM_PICKUP",			// normal item pickups are predictable
                         "EV_GLOBAL_ITEM_PICKUP",		// powerup / team sounds are broadcast to everyone

                         "EV_NOAMMO",
                         "EV_CHANGE_WEAPON",
                         "EV_FIRE_WEAPON",
                         "EV_FIRE_WEAPON_OTHER",

                         "EV_USE_ITEM0",
                         "EV_USE_ITEM1",
                         "EV_USE_ITEM2",
                         "EV_USE_ITEM3",
                         "EV_USE_ITEM4",
                         "EV_USE_ITEM5",
                         "EV_USE_ITEM6",
                         "EV_USE_ITEM7",
                         "EV_USE_ITEM8",
                         "EV_USE_ITEM9",
                         "EV_USE_ITEM10",
                         "EV_USE_ITEM11",
                         "EV_USE_ITEM12",
                         "EV_USE_ITEM13",
                         "EV_USE_ITEM14",
                         "EV_USE_ITEM15",

                         "EV_ITEM_RESPAWN",
                         "EV_ITEM_POP",
                         "EV_PLAYER_TELEPORT_IN",
                         "EV_PLAYER_TELEPORT_OUT",

                         "EV_GRENADE_BOUNCE",		// eventParm will be the soundindex

                         "EV_GENERAL_SOUND",
                         "EV_GLOBAL_SOUND",		// no attenuation
                         "EV_GLOBAL_TEAM_SOUND",

                         "EV_BULLET_HIT_FLESH",
                         "EV_BULLET_HIT_WALL",

                         "EV_MISSILE_HIT",
                         "EV_MISSILE_MISS",
                         "EV_MISSILE_MISS_METAL",
                         "EV_RAILTRAIL",
                         "EV_SHOTGUN",
                         "EV_BULLET",				// otherEntity is the shooter

                         "EV_PAIN",
                         "EV_DEATH1",
                         "EV_DEATH2",
                         "EV_DEATH3",
                         "EV_OBITUARY",

                         "EV_POWERUP_QUAD",
                         "EV_POWERUP_BATTLESUIT",
                         "EV_POWERUP_REGEN",

                         "EV_GIB_PLAYER",			// gib a previously living player
                         "EV_SCOREPLUM",			// score plum

                         //#ifdef MISSIONPACK
                         "EV_PROXIMITY_MINE_STICK",
                         "EV_PROXIMITY_MINE_TRIGGER",
                         "EV_KAMIKAZE",			// kamikaze explodes
                         "EV_OBELISKEXPLODE",		// obelisk explodes
                         "EV_INVUL_IMPACT",			// invulnerability sphere impact
                         "EV_JUICED",			// invulnerability juiced effect
                         "EV_LIGHTNINGBOLT",		// lightning bolt bounced of invulnerability sphere
                         //#endif

                         "EV_DEBUG_LINE",
                         "EV_STOPLOOPINGSOUND",
                         "EV_TAUNT"

                     };

/*
===============
BG_AddPredictableEventToPlayerstate

Handles the sequence numbers
===============
*/

void	trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

void BG_AddPredictableEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps ) {

#ifdef _DEBUG
    {
        char buf[256];
        trap_Cvar_VariableStringBuffer("showevents", buf, sizeof(buf));
        if ( atof(buf) != 0 ) {
#ifdef QAGAME
            Com_Printf(" game event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount/*ps->commandTime*/, ps->eventSequence, eventnames[newEvent], eventParm);
#else
            Com_Printf("Cgame event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount/*ps->commandTime*/, ps->eventSequence, eventnames[newEvent], eventParm);
#endif
        }
    }
#endif
    ps->events[ps->eventSequence & (MAX_PS_EVENTS-1)] = newEvent;
    ps->eventParms[ps->eventSequence & (MAX_PS_EVENTS-1)] = eventParm;
    ps->eventSequence++;
}

/*
========================
BG_TouchJumpPad
========================
*/
void BG_TouchJumpPad( playerState_t *ps, entityState_t *jumppad ) {
    // spectators don't use jump pads
    if ( ps->pm_type != PM_NORMAL ) {
        return;
    }

    // flying characters don't hit bounce pads
#if 0
    if ( ps->powerups[PW_FLIGHT] ) {
        return;
    }
#endif

    // remember hitting this jumppad this frame
    ps->jumppad_ent = jumppad->number;
    ps->jumppad_frame = ps->pmove_framecount;
    // give the player the velocity from the jumppad
    VectorCopy( jumppad->origin2, ps->velocity );
}

/*
========================
BG_PlayerStateToEntityState

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap ) {
    int		i;

    if ( ps->pm_type == PM_INTERMISSION /*|| ps->pm_type == PM_FREEZE*/ || ps->pm_type == PM_NOCLIP || ps->pm_type == PM_SPECTATOR ) {
        s->eType = ET_INVISIBLE;
    }
    // Navy Seals ++else if ( ps->stats[STAT_HEALTH] <= GIB_HEALTH ) {
    //	s->eType = ET_INVISIBLE;
    //	} // Navy Seals --
    else {
        s->eType = ET_PLAYER;
    }

    s->number = ps->clientNum;

    s->pos.trType = TR_INTERPOLATE;
    VectorCopy( ps->origin, s->pos.trBase );
    if ( snap ) {
        SnapVector( s->pos.trBase );
    }
    // set the trDelta for flag direction
    VectorCopy( ps->velocity, s->pos.trDelta );

    s->apos.trType = TR_INTERPOLATE;
    VectorCopy( ps->viewangles, s->apos.trBase );
    if ( snap ) {
        SnapVector( s->apos.trBase );
    }

    s->angles2[YAW] = ps->movementDir;
    s->legsAnim = ps->legsAnim;
    s->torsoAnim = ps->torsoAnim;
    s->clientNum = ps->clientNum;		// ET_PLAYER looks here instead of at number
    // so corpses can also reference the proper config
    s->eFlags = ps->eFlags;
    if ( ps->stats[STAT_HEALTH] <= 0 ) {
        s->eFlags |= EF_DEAD;
    } else {
        s->eFlags &= ~EF_DEAD;
    }
    // Navy Seals --

    if ( ps->externalEvent ) {
        s->event = ps->externalEvent;
        s->eventParm = ps->externalEventParm;
    } else {
        int		seq;

        if ( ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS) {
            ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;
        }
        seq = (ps->entityEventSequence-1) & (MAX_PS_EVENTS-1);
        s->event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
        s->eventParm = ps->eventParms[ seq ];
        if ( ps->entityEventSequence < ps->eventSequence ) {
            ps->entityEventSequence++;
        }
    }

    s->weapon = ps->weapon;
    s->groundEntityNum = ps->groundEntityNum;
    // Navy Seals ++
    s->powerups = s->frame = 0;

    // copy array


    for (i = WP_NUM_WEAPONS - 1; i > 0; i-- )
    {
        if ( ps->weapon == i )
            continue;

        if (BG_GotWeapon( i , ps->stats ) &&
                ( BG_IsPrimary(i) || BG_IsSecondary(i) )
           )
            s->frame |= 1 << i;
    }

    for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
        if ( ps->powerups[ i ] ) {
            s->powerups |= 1 << i;
        }
    }
    // Navy Seals --

    s->loopSound = ps->loopSound;
    s->generic1 = ps->generic1;
}

/*
========================
BG_PlayerStateToEntityStateExtraPolate

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, qboolean snap ) {
    int		i;

    if ( ps->pm_type == PM_INTERMISSION /*|| ps->pm_type == PM_FREEZE*/ || ps->pm_type == PM_NOCLIP || ps->pm_type == PM_SPECTATOR ) {
        s->eType = ET_INVISIBLE;
    }
    // Navy Seals ++
    //	} else if ( ps->stats[STAT_HEALTH] <= GIB_HEALTH ) {
    //		s->eType = ET_INVISIBLE;
    /*	} */
    // Navy Seals --
    else {
        s->eType = ET_PLAYER;
    }

    s->number = ps->clientNum;

    s->pos.trType = TR_LINEAR_STOP;
    VectorCopy( ps->origin, s->pos.trBase );
    if ( snap ) {
        SnapVector( s->pos.trBase );
    }
    // set the trDelta for flag direction and linear prediction
    VectorCopy( ps->velocity, s->pos.trDelta );
    // set the time for linear prediction
    s->pos.trTime = time;
    // set maximum extra polation time
    s->pos.trDuration = 50; // 1000 / sv_fps (default = 20)

    s->apos.trType = TR_INTERPOLATE;
    VectorCopy( ps->viewangles, s->apos.trBase );
    if ( snap ) {
        SnapVector( s->apos.trBase );
    }

    s->angles2[YAW] = ps->movementDir;
    s->legsAnim = ps->legsAnim;
    s->torsoAnim = ps->torsoAnim;
    s->clientNum = ps->clientNum;		// ET_PLAYER looks here instead of at number
    // so corpses can also reference the proper config
    s->eFlags = ps->eFlags;
    if ( ps->stats[STAT_HEALTH] <= 0 ) {
        s->eFlags |= EF_DEAD;
    } else {
        s->eFlags &= ~EF_DEAD;
    }
    // Navy Seals ++
    /*
    if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_SILENCER ) )
    s->eFlags |= EF_SILENCED;
    else
    s->eFlags &= ~EF_SILENCED;

    // flashlight activated.
    if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ) && ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_FLASHLIGHT ) ) && (ps->weaponstate == WEAPON_READY || ps->weaponstate == WEAPON_FIRING || ps->weaponstate == WEAPON_FIRING2 || ps->weaponstate == WEAPON_FIRING3 ) )
    s->eFlags |= EF_WEAPONMODE3;
    else
    s->eFlags &=~ EF_WEAPONMODE3;

    if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_LACTIVE )  && (ps->weaponstate == WEAPON_READY || ps->weaponstate == WEAPON_FIRING || ps->weaponstate == WEAPON_FIRING2 || ps->weaponstate == WEAPON_FIRING3 ) )
    s->eFlags |= EF_LASERSIGHT;
    else
    s->eFlags &= ~EF_LASERSIGHT;
    */
    // Navy Seals --

    if ( ps->externalEvent ) {
        s->event = ps->externalEvent;
        s->eventParm = ps->externalEventParm;
    } else {
        int		seq;

        if ( ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS) {
            ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;
        }
        seq = (ps->entityEventSequence-1) & (MAX_PS_EVENTS-1);
        s->event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
        s->eventParm = ps->eventParms[ seq ];
        if ( ps->entityEventSequence < ps->eventSequence ) {
            ps->entityEventSequence++;
        }
    }

    s->weapon = ps->weapon;
    s->groundEntityNum = ps->groundEntityNum;
    // Navy Seals ++
    s->powerups = s->frame = 0;

    // copy array
    for (i = WP_NUM_WEAPONS - 1; i > 0; i-- )
    {
        if ( ps->weapon == i )
            continue;

        if ( BG_GotWeapon( i , ps->stats ) &&
                ( BG_IsPrimary(i) || BG_IsSecondary(i) )
           )
            s->frame |= 1 << i;
    }
    for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
        if ( ps->powerups[ i ] ) {
            s->powerups |= 1 << i;
        }
    }
    // Navy Seals --
    s->loopSound = ps->loopSound;
    s->generic1 = ps->generic1;
}
