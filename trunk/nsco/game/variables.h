#ifndef __VARIABLES_H__
#define __VARIABLES_H__

// ###########################################################################
//
// This file contains the most important configuration variables for
// Navy Seals - Covert Operations
//
// IMPORTANT:
// This file is not complete yet and therefore some configuration options may
// be misleading. All damage values are _BASIC_ damage values which will
// be modified by the hit system, mostly the basic values will grow!
// EXAMPLE:
// A grenade inflicting 75 damage to the chest will on close range inflict
// 75*2.5 damage. The code from Navy Seals - Cover Operations is a difficult
// beast, so just take this values, test them and if some value must be
// increased / decreased tell me.
//
// IMPORTANT 2:
// This file is property of Team Mirage. You are not allowed to copy it or
// even share it with others.
//
// Blutengel 2003/01/08
//
// ###########################################################################

// ###########################################################################
// clientgame sound specific
#define SEALS_FOOTSTEP_FALLOFF		0.12f

// ###########################################################################
// grenade specific
#define SEALS_GRENADEDAMAGE 160 // damage inflicted by a grenade
#define SEALS_GRENADESPLASHDAMAGE 130 // splash damage inflicted by a grenade
#define SEALS_GRENADERADIUS 362 // the radius of a grenade
#define SEALS_DEATHNADERADIUS 64 // the radius inflicting 999 damage
#define SEALS_MAXKNOCKBACK 128  // maximum knockback for grenades
#define SEALS_MAX_GRENADES 4   // maximum number of grenades

// ###########################################################################
// 40mm grenade specific
#define SEALS_40MMGRENDAMAGE 100 // damage inflicted by a grenade
#define SEALS_40MMGRENSPLASHDAMAGE 130 // splash damage inflicted by a grenade
#define SEALS_40MMGRENRADIUS 224 // the radius of a grenade
#define SEALS_40MMGREN_ARMEDTIME 350 // the time until the gren gets armed
#define SEALS_40MMGREN_SPEED 1500 // the speed (and therefore range) of GL

// ###########################################################################
// flashbang specific
//
// IMPORTANT:
// The actual time of the whiteout effect is SEALS_FLASHBANGTIME and _NOT_
// SEALS_FLASHBANGTIME + SEALS_FLASHBANGFADETIME
//
// So with SEALS_FLASHBANGTIME 6000 and SEALS_FLASHBANGFADETIME 2000 there
// are only 4000 msecs of 100% whiteout at max!
//
// however the BLINDSPOT is not affected by FLASHBANGFADETIME, so don't look
// into a flashbang!
#define SEALS_FLASHBANGTIME 9000 // time to get blinded at MAX
#define SEALS_FLASHBANGRADIUS 720 // the radius 32qu = 1m where a player can be hit by a flashbang
#define SEALS_BLINDSPOTFACTOR 1.5 // BLINDSPOTFACTOR*FLASHBANGTIME = time the blindspot is beeing rendered
#define SEALS_CONCUSSIONFACTOR 1.2 // CONCUSSIONFACTOR*FLASHBANGTIME = time the concussion effect is beeing rendered
#define SEALS_FLASHBANGFADETIME 3000 // time to fade out the flashbang
#define SEALS_FLASHEDBEHINDFACTOR 0.5 // amount of flash effect from behind

// ###########################################################################
// smokenade specific
#define SEALS_SMOKENADETIME 100 // time the smoke nade will spawn smoke for each numbr
#define SEALS_SMOKEPUFF_NUMBER 100 // number of smokepuffs to spawn from serverside
#define SEALS_SMOKEBLEND_RANGE 96.0f // range of the blend effect clientside
#define SEALS_SMOKEPUFF_TIME 4500 // duration of a single smokepuff clientside
#define SEALS_SMOKEPUFF_RADIUS 64.0 // radius of a single smokepuff clientside
#define SEALS_SMOKENADE_DISTANCE 320 // how far the smokepuffs can flow

// color of the smoke
#define SEALS_SMOKENADE_R_JUNGLE 0.66f
#define SEALS_SMOKENADE_G_JUNGLE 0.84f
#define SEALS_SMOKENADE_B_JUNGLE 0.31f

#define SEALS_SMOKENADE_R_URBAN 0.20f
#define SEALS_SMOKENADE_G_URBAN 0.31f
#define SEALS_SMOKENADE_B_URBAN 0.58f

#define SEALS_SMOKENADE_R_ARCTIC 0.70f
#define SEALS_SMOKENADE_G_ARCTIC 0.94f
#define SEALS_SMOKENADE_B_ARCTIC 1.00f

#define SEALS_SMOKENADE_R_DESERT 0.82f
#define SEALS_SMOKENADE_G_DESERT 0.72f
#define SEALS_SMOKENADE_B_DESERT 0.47f

// bitmasks
#define SEALS_SMOKEMASK_UP        0x0000c000
#define SEALS_SMOKEMASK_FORWARD   0x00003000
#define SEALS_SMOKEMASK_BACKWARD  0x00000c00
#define SEALS_SMOKEMASK_LEFT      0x00000300
#define SEALS_SMOKEMASK_RIGHT     0x000000c0
#define SEALS_SMOKEMASK_FLAGS     0x0000ffc0
#define SEALS_SMOKEMASK_RNDNUM    0x0000003f
#define SEALS_SMOKEMASK_SUP       14
#define SEALS_SMOKEMASK_SFORWARD  12
#define SEALS_SMOKEMASK_SBACKWARD 10
#define SEALS_SMOKEMASK_SLEFT     8
#define SEALS_SMOKEMASK_SRIGHT    6
#define SEALS_SMOKEMASK_VALUE     0x3

// ###########################################################################
// grenade / flashbang shared
#define SEALS_BASEGRENRANGE_THROW 350 // the range of a nade throwing it
#define SEALS_BASEGRENRANGE_ROLL  150 // the range of a nade rolling it
#define SEALS_BASEGRENRANGE_ADDSTRENGTH 0.5 // factor which is added to the range of nades. SEALS_BASEGRENRAGE_THROW + (strength/10*SEALS_BASEGRENRAGE_THROW*SEALS_BASEGRENRAGE_ADDSTRENGTH). So the maximum distance for throwing with SEALS_BASGRENRANGE_ADDSTRENGTH=0.5 is 350+350*.5 = 525

// ###########################################################################
// speed hacks
#define SEALS_SPEED_STANDING 5.0
#define SEALS_SPEED_CROUCHING 50.0
#define SEALS_SPEED_WALKING 139.0
#define SEALS_SPEED_RUNNING 230.0
#define SEALS_SPEED_SPRINTING 280.0
#define SEALS_SPEED_MAX 300.0

#define SEALS_SPEED_FACT 0.3      // speed factor for the speed attribute, 0.3 means a maximum of 30% + speed

#define SEALS_STANDING 0
#define SEALS_CROUCHING 1
#define SEALS_WALKING 2
#define SEALS_RUNNING 3
#define SEALS_SPRINTING 4
#define SEALS_JUMPING 5
#define SEALS_OTHER 6

// ###########################################################################
// weapons specific stuff
// describing accuracy

// ###########################################################################
// SNIPER specific stuff (MacMillan, PSG1, SL8SD)
#define SEALS_IS_SNIPER(blub) ( blub == WP_MACMILLAN || blub == WP_SL8SD || blub == WP_PSG1 )
#define SEALS_SNIPER_DEFLECTION_STANDING  2.0
#define SEALS_SNIPER_DEFLECTION_CROUCHING 3.0
#define SEALS_SNIPER_DEFLECTION_WALKING   3.0
#define SEALS_SNIPER_DEFLECTION_RUNNING   4.0
#define SEALS_SNIPER_DEFLECTION_SPRINTING 5.0
#define SEALS_SNIPER_DEFLECTION_JUMPING   5.0
#define SEALS_SNIPER_DEFLECTION_OTHER     5.0

// ###########################################################################
// RIFLE specific stuff (ak47, m4, m14)
#define SEALS_IS_RIFLE(blub) ( blub == WP_AK47 || blub == WP_M4 || blub == WP_M14 )
#define SEALS_RIFLE_DEFLECTION_STANDING  0.2
#define SEALS_RIFLE_DEFLECTION_CROUCHING 0.6
#define SEALS_RIFLE_DEFLECTION_WALKING   1.2
#define SEALS_RIFLE_DEFLECTION_RUNNING   2.4
#define SEALS_RIFLE_DEFLECTION_SPRINTING 4.0
#define SEALS_RIFLE_DEFLECTION_JUMPING   5.0
#define SEALS_RIFLE_DEFLECTION_OTHER     5.0

// ###########################################################################
// SMG specific stuff (mac10 mp5 pdw)
#define SEALS_IS_SMG(blub) ( blub == WP_MAC10 || blub == WP_MP5 || blub == WP_PDW )
#define SEALS_SMG_DEFLECTION_STANDING  0.8
#define SEALS_SMG_DEFLECTION_CROUCHING 1.2
#define SEALS_SMG_DEFLECTION_WALKING   2.4
#define SEALS_SMG_DEFLECTION_RUNNING   3.0
#define SEALS_SMG_DEFLECTION_SPRINTING 4.0
#define SEALS_SMG_DEFLECTION_JUMPING   5.0
#define SEALS_SMG_DEFLECTION_OTHER     5.0

// ###########################################################################
// PISTOL specific stuff (
#define SEALS_IS_PISTOL(blub) ( blub == WP_P9S || blub == WP_GLOCK || blub == WP_MK23 || blub == WP_SW40T || blub == WP_SW629 || blub == WP_DEAGLE )
#define SEALS_PISTOL_DEFLECTION_STANDING  0.6
#define SEALS_PISTOL_DEFLECTION_CROUCHING 1.2
#define SEALS_PISTOL_DEFLECTION_WALKING   1.4
#define SEALS_PISTOL_DEFLECTION_RUNNING   2.0
#define SEALS_PISTOL_DEFLECTION_SPRINTING 3.0
#define SEALS_PISTOL_DEFLECTION_JUMPING   3.0
#define SEALS_PISTOL_DEFLECTION_OTHER     4.0

// ###########################################################################
// SHOTGUN specific stuff (870 m590 spas15)
// this one is not deflection, this one is how the bullets are sprayed
#define SEALS_IS_SHOTGUN(blub) ( blub == WP_870 || blub == WP_M590 || blub == WP_SPAS15 )
#define SEALS_SHOTGUN_DEFLECTION_STANDING  2.0
#define SEALS_SHOTGUN_DEFLECTION_CROUCHING 2.0
#define SEALS_SHOTGUN_DEFLECTION_WALKING   2.0
#define SEALS_SHOTGUN_DEFLECTION_RUNNING   2.0
#define SEALS_SHOTGUN_DEFLECTION_SPRINTING 2.0
#define SEALS_SHOTGUN_DEFLECTION_JUMPING   2.0
#define SEALS_SHOTGUN_DEFLECTION_OTHER     2.0

// ###########################################################################
// MG specific stuff (m249)
#define SEALS_IS_MG(blub) ( blub == WP_M249 )
#define SEALS_MG_DEFLECTION_STANDING  0.3
#define SEALS_MG_DEFLECTION_CROUCHING 1.0
#define SEALS_MG_DEFLECTION_WALKING   2.5
#define SEALS_MG_DEFLECTION_RUNNING   4.0
#define SEALS_MG_DEFLECTION_SPRINTING 5.0
#define SEALS_MG_DEFLECTION_JUMPING   5.0
#define SEALS_MG_DEFLECTION_OTHER     5.0

// ###########################################################################
// Weapons general stuff
#define SEALS_WEAP_LASER_MOD 0.6 // factor multiplied with deflection degree if the weapon has a laser and is no pistol
#define SEALS_WEAP_PISTOLLASER_MOD 0.8 // factor multiplied with deflection degree if the weapon has a laser and is a pistol
#define SEALS_WEAP_CROUCH_MOD 0.7 // factor multiplied with deflection degree if the character is crouching
#define SEALS_WEAP_SCOPE_MOD 0.02 // factor multiplied with deflection degree if the weapon is in scope mode
#define SEALS_WEAP_PDW_MOD 0.9 // factor multiplied with deflection if the weapon is the pdw in secondary mode
#define SEALS_DUCKBILL_HOR_MOD 1.5 // factor multiplied with horizontal deflection if the weapon has a duckbill
#define SEALS_DUCKBILL_VER_MOD 0.3 // factor multiplied with vertical deflection if the weapon has a duckbill
#define SEALS_ACCURACY_MOD 0.7 // factor multiplied with deflection based on accuracy

// ###########################################################################
// damage inflicted by weapons
#define SEALS_DMG_GLOCK           13
#define SEALS_DMG_VEST_GLOCK     0.35
#define SEALS_DMG_P9S             13
#define SEALS_DMG_VEST_P9S       0.35
#define SEALS_DMG_MK23            15
#define SEALS_DMG_VEST_MK23      0.4
#define SEALS_DMG_SW40T           16
#define SEALS_DMG_VEST_SW40T     0.4
#define SEALS_DMG_SW629           24
#define SEALS_DMG_VEST_SW629     0.45
#define SEALS_DMG_DEAGLE          23
#define SEALS_DMG_VEST_DEAGLE    0.45

#define SEALS_DMG_MP5             17
#define SEALS_DMG_VEST_MP5       0.7
#define SEALS_DMG_MAC10           17
#define SEALS_DMG_VEST_MAC10     0.6
#define SEALS_DMG_PDW             10
#define SEALS_DMG_VEST_PDW       1.0

#define SEALS_DMG_M14             19
#define SEALS_DMG_VEST_M14       0.9
#define SEALS_DMG_M4              18
#define SEALS_DMG_VEST_M4        0.8
#define SEALS_DMG_AK47            18
#define SEALS_DMG_VEST_AK47      0.8

#define SEALS_DMG_M249            18
#define SEALS_DMG_VEST_M249      0.8

#define SEALS_DMG_SPAS15          7
#define SEALS_DMG_VEST_SPAS15   0.45
#define SEALS_DMG_M590            8
#define SEALS_DMG_VEST_M590     0.6
#define SEALS_DMG_870             8
#define SEALS_DMG_VEST_870      0.6

#define SEALS_DMG_MACMILLAN       65
#define SEALS_DMG_VEST_MACMILLAN 1.0
#define SEALS_DMG_PSG1            30
#define SEALS_DMG_VEST_PSG1      0.8
#define SEALS_DMG_SL8SD           20
#define SEALS_DMG_VEST_SL8SD     0.7

#define SEALS_DMG_SILENCER        3
#define SEALS_DMG_MUZZLEHIDER     1

// ###########################################################################
// basic weapon recoil factors
#define SEALS_WKICK_BASE          1.0	// base weapon recoil factor
#define SEALS_WKICK_SPEEDFACT     4.0	// maximum speed factor
#define SEALS_WKICK_JUMPFACT      1.2	// jump factor
#define SEALS_WKICK_2HANDLEFACT   0.5	// handle factor (PDW)
#define SEALS_WKICK_CROUCHNOSPD   0.8 // crouched and no speed factor
#define SEALS_WKICK_MAXSTRFACT    0.7 // the maximum factor one gained through strength
#define SEALS_WKICK_MAXRANDFACT   0.3 // the maximum random factor

// ###########################################################################
// weapon recoil base numbers for the different weapons
// the recoil of the WEAPON is given in degrees
#define SEALS_WKICK_PDW           1.5
#define SEALS_WKICK_MP5           1.1
#define SEALS_WKICK_MAC10         1.3

#define SEALS_WKICK_M4            1.3
#define SEALS_WKICK_AK47          1.3
#define SEALS_WKICK_M14           1.5

#define SEALS_WKICK_M249          1.7

#define SEALS_WKICK_M590          2.8
#define SEALS_WKICK_870           2.8
#define SEALS_WKICK_SPAS15        1.5

#define SEALS_WKICK_PSG1          1.3
#define SEALS_WKICK_MACMILLAN     2.4
#define SEALS_WKICK_SL8SD         1.5

#define SEALS_WKICK_P9S           1.1
#define SEALS_WKICK_GLOCK         1.1
#define SEALS_WKICK_SW40T         1.3
#define SEALS_WKICK_MK23          1.3
#define SEALS_WKICK_SW629         1.5
#define SEALS_WKICK_DEAGLE        1.5

#define SEALS_WKICK_DEFAULT       2.0

// ###########################################################################
// weapon knockback for the different weapons
#define SEALS_KNOCKBACK_PDW           8.0
#define SEALS_KNOCKBACK_MP5           10.0
#define SEALS_KNOCKBACK_MAC10         10.0

#define SEALS_KNOCKBACK_M4            16.0
#define SEALS_KNOCKBACK_AK47          16.0
#define SEALS_KNOCKBACK_M14           24.0

#define SEALS_KNOCKBACK_M249          16.0

#define SEALS_KNOCKBACK_M590          24.0
#define SEALS_KNOCKBACK_870           24.0
#define SEALS_KNOCKBACK_SPAS15        24.0

#define SEALS_KNOCKBACK_PSG1          24.0
#define SEALS_KNOCKBACK_MACMILLAN     32.0
#define SEALS_KNOCKBACK_SL8SD         24.0

#define SEALS_KNOCKBACK_P9S           8.0
#define SEALS_KNOCKBACK_GLOCK         8.0
#define SEALS_KNOCKBACK_SW40T         10.0
#define SEALS_KNOCKBACK_MK23          10.0
#define SEALS_KNOCKBACK_SW629         16.0
#define SEALS_KNOCKBACK_DEAGLE        16.0

#define SEALS_KNOCKBACK_DEFAULT       32.0

// ###########################################################################
// weapon fire rates

#define SEALS_FRATE_PDW               50
#define SEALS_FRATE_MP5               70
#define SEALS_FRATE_MAC10             85

#define SEALS_FRATE_AK47              100
#define SEALS_FRATE_M4                90
#define SEALS_FRATE_M14               125
#define SEALS_FRATE_M249              80

#define SEALS_FRATE_M590              400
#define SEALS_FRATE_870               400
#define SEALS_FRATE_SPAS15            125

#define SEALS_FRATE_PSG1              200
#define SEALS_FRATE_MACMILLAN         100
#define SEALS_FRATE_SL8SD             100

#define SEALS_FRATE_P9S               125
#define SEALS_FRATE_GLOCK             125
#define SEALS_FRATE_MK23              125
#define SEALS_FRATE_SW40T             125
#define SEALS_FRATE_DEAGLE            200
#define SEALS_FRATE_SW629             200

#define SEALS_FRATE_KHURKURI          380
#define SEALS_FRATE_SEALKNIFE         380

#define SEALS_FRATE_DEFAULT           175

// ###########################################################################
// Weight of the weapons in kg*10

#define SEALS_WEIGHT_PDW               17.0
#define SEALS_WEIGHT_MP5               15.0
#define SEALS_WEIGHT_MAC10             16.0

#define SEALS_WEIGHT_AK47              19.0
#define SEALS_WEIGHT_M4                19.0
#define SEALS_WEIGHT_M14               22.0
#define SEALS_WEIGHT_M249              35.0

#define SEALS_WEIGHT_M590              18.0
#define SEALS_WEIGHT_870               18.0
#define SEALS_WEIGHT_SPAS15            20.0

#define SEALS_WEIGHT_PSG1              20.0
#define SEALS_WEIGHT_MACMILLAN         25.0
#define SEALS_WEIGHT_SL8SD             18.0

#define SEALS_WEIGHT_P9S               8.0
#define SEALS_WEIGHT_GLOCK             8.0
#define SEALS_WEIGHT_MK23              10.0
#define SEALS_WEIGHT_SW40T             10.0
#define SEALS_WEIGHT_DEAGLE            12.0
#define SEALS_WEIGHT_SW629             12.0

#define SEALS_WEIGHT_KHURKURI          5.0
#define SEALS_WEIGHT_SEALKNIFE         5.0

#define SEALS_WEIGHT_DEFAULT           25.0

#define SEALS_WEIGHT_HELMET            2.0
#define SEALS_WEIGHT_KEVLAR            3.0

#define SEALS_WEIGHT_SHOTGUNMAG        0.45   // spas15
#define SEALS_WEIGHT_SHOTGUN           0.05   // normal shotgun

#define SEALS_WEIGHT_LIGHT_PISTOL      0.1
#define SEALS_WEIGHT_MEDIUM_PISTOL    0.1
#define SEALS_WEIGHT_LARGE_PISTOL      0.2

#define SEALS_WEIGHT_SMG               0.25
#define SEALS_WEIGHT_RIFLE             0.3
#define SEALS_WEIGHT_MG                0.4
#define SEALS_WEIGHT_MEDIUM_SNIPER      0.4
#define SEALS_WEIGHT_LARGE_SNIPER       0.5

#define SEALS_WEIGHT_GRENADE           0.5
#define SEALS_WEIGHT_FLASHBANG         0.5
#define SEALS_WEIGHT_SMOKE             0.5
#define SEALS_WEIGHT_40MMGRENADE       0.25

#define SEALS_WEIGHT_RATIO             0.6 // how much of the weight should cound in any case

// ###########################################################################
// General CONFIG stuff
#define SEALS_DRAW_NOT_ENEMY_NAME 1 // if set , the client will not tell you if you've spotted an enemy
#define SEALS_CHAT_COMMAND_TIME 250 // SEALS_CHAT_COMMAND_TIME msecs must be betweeen every chat command
#define SEALS_INCLUDE_NVG 0 // set to 1 if night vision gogles should be included
#define SEALS_NO_GRENADE_CROSSHAIR 1 // set to 1 if there should be no crosshair when grenades are selected

#define	NS_FLAG_BUSY_CARRIYNG	0x00000001
#define NS_FLAG_END_ROUND	    0x00000002
#define NS_FLAG_LOOP          0x00000004
#define NS_FLAG_DEFUSABLE     0x00000008

#endif
