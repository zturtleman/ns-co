// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_event.c -- handle entity events at snapshot or playerstate transitions

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"

void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum, int damage );
void CG_ReloadWeapon ( centity_t *cent, int last_rnd );
void CG_RealBloodTrail( vec3_t start, vec3_t end, float spacing );
void CG_DeleteDirectMark( int entityNum );
void CG_LockBreak( centity_t *cent );
void CG_DamageFeedback( int yawByte, int pitchByte, int damage, int damageDuration );

// for the voice chats
#include "../../ui/menudef.h"

//==========================================================================

/*
===================
CG_PlaceString

Also called by scoreboard drawing
===================
*/
const char	*CG_PlaceString( int rank ) {
    static char	str[64];
    char	*s, *t;


    if ( rank & RANK_TIED_FLAG ) {
        rank &= ~RANK_TIED_FLAG;
        t = "Tied for ";
    } else {
        t = "";
    }

    if ( rank == 1 ) {
        s = S_COLOR_BLUE "1st" S_COLOR_WHITE;		// draw in blue
    } else if ( rank == 2 ) {
        s = S_COLOR_RED "2nd" S_COLOR_WHITE;		// draw in red
    } else if ( rank == 3 ) {
        s = S_COLOR_YELLOW "3rd" S_COLOR_WHITE;		// draw in yellow
    } else if ( rank == 11 ) {
        s = "11th";
    } else if ( rank == 12 ) {
        s = "12th";
    } else if ( rank == 13 ) {
        s = "13th";
    } else if ( rank % 10 == 1 ) {
        s = va("%ist", rank);
    } else if ( rank % 10 == 2 ) {
        s = va("%ind", rank);
    } else if ( rank % 10 == 3 ) {
        s = va("%ird", rank);
    } else {
        s = va("%ith", rank);
    }

    Com_sprintf( str, sizeof( str ), "%s%s", t, s );
    return str;
}

/*
=============
CG_Obituary
=============
*/
static void CG_Obituary( entityState_t *ent ) {
    int			mod;
    int			target, attacker;
    char		*message;
    char		*message2;
    const char	*targetInfo;
    const char	*attackerInfo;
    char		targetName[64];
    char		attackerName[64];
    gender_t	gender;
    clientInfo_t	*ci;

    target = ent->otherEntityNum;

    // ns: only show obituary to the one that got killed
    if (target != cg.snap->ps.clientNum)
        return;

    attacker = ent->otherEntityNum2;
    mod = ent->eventParm;

    if ( target < 0 || target >= MAX_CLIENTS ) {
        CG_Error( "CG_Obituary: target out of range" );
    }
    ci = &cgs.clientinfo[target];

    if ( attacker < 0 || attacker >= MAX_CLIENTS ) {
        attacker = ENTITYNUM_WORLD;
        attackerInfo = NULL;
    } else {
        attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
    }

    targetInfo = CG_ConfigString( CS_PLAYERS + target );
    if ( !targetInfo ) {
        return;
    }
    Q_strncpyz( targetName, Info_ValueForKey( targetInfo, "n" ), sizeof(targetName) - 2);

    Com_sprintf( targetName, sizeof(targetName),"%s%s%s", S_COLOR_CYAN , targetName, S_COLOR_WHITE );


    message2 = "";

    // check for single client messages

    switch( mod ) {
    case MOD_SUICIDE:
        message = "tasted the sweet kiss of death";
        break;
    case MOD_FALLING:
        message = "forgot that this is reallife";
        break;
    case MOD_CRUSH:
        message = "got crushed";
        break;
    case MOD_WATER:
        message = "overestimated the size of his lungs";
        break;
    case MOD_SLIME:
        message = "melted";
        break;
    case MOD_LAVA:
        message = "does a back flip into the lava";
        break;
    case MOD_TARGET_LASER:
        message = "saw the light";
        break;
    case MOD_TRIGGER_HURT:
        message = "died";
        break;
    default:
        message = NULL;
        break;
    }

    if (attacker == target) {
        gender = ci->gender;
        switch (mod) {

        case MOD_GRENADE_SPLASH:
            if ( gender == GENDER_FEMALE )
                message = "was blown away by her own grenade";
            else if ( gender == GENDER_NEUTER )
                message = "was blown away by its own grenade";
            else
                message = "was blown away by his own grenade";
            break;

        default:
            message = "commited suicide";
            break;
        }
    }

    if (message) {
        CG_Printf( "%s %s.\n", targetName, message);
        return;
    }

    // check for double client messages
    if ( !attackerInfo ) {
        attacker = ENTITYNUM_WORLD;
        strcpy( attackerName, "noname" );
    } else {
        Q_strncpyz( attackerName, Info_ValueForKey( attackerInfo, "n" ), sizeof(attackerName) - 2);
        Com_sprintf( attackerName, sizeof(attackerName),"%s%s%s", S_COLOR_RED , attackerName, S_COLOR_WHITE );

        // check for kill messages about the current clientNum
        if ( target == cg.snap->ps.clientNum ) {
            Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
        }
    }

    if ( attacker != ENTITYNUM_WORLD ) {
        switch (mod) {
        case MOD_LEAD:
            message = "got shot by";
            break;
        case MOD_BLEED:
            message = "bleed to death because of";
            break;
        default:
            message = "was killed by";
            break;
        }

        if (message) {
            CG_Printf( "%s %s %s%s\n",
                       targetName, message, attackerName, message2);
            return;
        }
    }

    // we don't know what it was
    CG_Printf( "%s died.\n", targetName );
}

//==========================================================================

/*
===============
CG_UseItem
===============
*/
static void CG_UseItem( centity_t *cent ) {
    clientInfo_t *ci;
    int			itemNum, clientNum;
    gitem_t		*item;
    entityState_t *es;

    es = &cent->currentState;

    itemNum = (es->event & ~EV_EVENT_BITS) - EV_USE_ITEM0;
    if ( itemNum < 0 || itemNum > HI_NUM_HOLDABLE ) {
        itemNum = 0;
    }

    // print a message if the local player
    if ( es->number == cg.snap->ps.clientNum ) {
        if ( !itemNum ) {
            CG_CenterPrint( "No item to use", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
        } else {
            item = BG_FindItemForHoldable( itemNum );
            CG_CenterPrint( va("Use %s", item->pickup_name), SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
        }
    }

    switch ( itemNum ) {
    default:
    case HI_NONE:
        //		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useNothingSound );
        break;

    case HI_TELEPORTER:
        break;

    case HI_MEDKIT:
        clientNum = cent->currentState.clientNum;
        if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
            ci = &cgs.clientinfo[ clientNum ];
            ci->medkitUsageTime = cg.time;
        }
        //		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.medkitSound );
        break;


    }

}

/*
================
CG_ItemPickup

A new item was picked up this frame
================
*/
static void CG_ItemPickup( int itemNum ) {
    cg.itemPickup = itemNum;
    cg.itemPickupTime = cg.time;
    cg.itemPickupBlendTime = cg.time;
    // see if it should be the grabbed weapon
    if ( bg_itemlist[itemNum].giType == IT_WEAPON ) {
        // select it immediately
        if ( cg_autoswitch.integer && bg_itemlist[itemNum].giTag != WP_KHURKURI ) {
            cg.weaponSelectTime = cg.time + 1500;
            cg.weaponSelect = bg_itemlist[itemNum].giTag;
        }
    }

}

void CG_NewbieMessage( const char *str, int y, float charHeight );


/*
================
CG_PainEvent

Also called by playerstate transition
================
*/
void CG_PainEvent( centity_t *cent, int health ) {
    /*	char	*snd;

    // don't do more than two pain sounds a second
    if ( cg.time - cent->pe.painTime < 500 ) {
    return;
    }

    if ( health < 25 ) {
    snd = "*pain25_1.wav";
    } else if ( health < 50 ) {
    snd = "*pain50_1.wav";
    } else if ( health < 75 ) {
    snd = "*pain75_1.wav";
    } else {
    snd = "*pain100_1.wav";
    }
    trap_S_StartSound( NULL, cent->currentState.number, CHAN_VOICE,
    CG_CustomSound( cent->currentState.number, snd ) );
    */
    // save pain time for programitic twitch animation
    cent->pe.painTime = cg.time;
    cent->pe.painDirection ^= 1;
}



/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
#define	DEBUGNAME(x) if(cg_debugEvents.integer){CG_Printf(x"\n");}
void CG_EntityEvent( centity_t *cent, vec3_t position ) {
    entityState_t	*es;
    int				event;
    vec3_t			dir;
    const char		*s;
    int				clientNum;
    clientInfo_t	*ci;

    es = &cent->currentState;
    event = es->event & ~EV_EVENT_BITS;

    if ( cg_debugEvents.integer ) {
        CG_Printf( "ent:%3i  event:%3i", es->number, event );
    }

    if ( !event ) {
        DEBUGNAME("ZEROEVENT");
        return;
    }

    clientNum = es->clientNum;
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        clientNum = 0;
    }
    ci = &cgs.clientinfo[ clientNum ];

    // this is a client
    if ( clientNum < MAX_CLIENTS )
    {
        // generate footprints
        if ( event == EV_FOOTSTEP || event == EV_FOOTSTEP_METAL || event == EV_FOOTSTEP_WOOD ||
                event == EV_FOOTSTEP_SNOW || event == EV_FOOTSTEP_DIRT || event == EV_FOOTSPLASH ||
                event == EV_FOOTWADE || event == EV_FALL_SHORT || event == EV_FALL_LIGHT ||
                event == EV_FALL_MEDIUM || event == EV_FALL_FAR ||	event == EV_FALL_DEATH )
        {
            if ( cent->leftWaterTime + 60000 > cg.time && cent->leftWaterTime != 0 ) {
                cent->footstepSpawn = qtrue;
            }
        }
    }


    switch ( event ) {
        //
        // movement generated events
        //
    case EV_FOOTSTEP:
        DEBUGNAME("EV_FOOTSTEP");
        if (cg_footsteps.integer && (cg.DeafTime < cg.time) ) {
            trap_S_StartSoundExtended(NULL, 1.0, SEALS_FOOTSTEP_FALLOFF, 1.0f, es->number, CHAN_BODY,
                               cgs.media.footsteps[ ci->footsteps ][rand()&3] );
        }
        break;
    case EV_FOOTSTEP_METAL:
        DEBUGNAME("EV_FOOTSTEP_METAL");
        if (cg_footsteps.integer && (cg.DeafTime < cg.time)) {
            trap_S_StartSoundExtended(NULL, 1.0, SEALS_FOOTSTEP_FALLOFF, 1.0f, es->number, CHAN_BODY,
                               cgs.media.footsteps[ FOOTSTEP_METAL ][rand()&3] );
        }
        break;
    case EV_GAMESTATE:
        trap_Cvar_Set("ui_gamestate", va("%i", es->eventParm));
        break; 
    case EV_FOOTSTEP_WOOD:
        DEBUGNAME("EV_FOOTSTEP_WOOD");
        if (cg_footsteps.integer && (cg.DeafTime < cg.time)) {
            trap_S_StartSoundExtended(NULL, 1.0, SEALS_FOOTSTEP_FALLOFF, 1.0, es->number, CHAN_BODY,
                               cgs.media.footsteps[ FOOTSTEP_WOOD ][rand()&3] );
        }
        break;
    case EV_FOOTSTEP_SNOW:
        DEBUGNAME("EV_FOOTSTEP_SNOW");
        if (cg_footsteps.integer && (cg.DeafTime < cg.time)) {
            trap_S_StartSoundExtended(NULL, 1.0, SEALS_FOOTSTEP_FALLOFF, 1.0, es->number, CHAN_BODY,
                               cgs.media.footsteps[ FOOTSTEP_SNOW ][rand()&3] );
        }
        break;
    case EV_FOOTSTEP_DIRT:
        DEBUGNAME("EV_FOOTSTEP_DIRT");
        if (cg_footsteps.integer && (cg.DeafTime < cg.time)) {
            trap_S_StartSoundExtended(NULL, 1.0, SEALS_FOOTSTEP_FALLOFF, 1.0, es->number, CHAN_BODY,
                               cgs.media.footsteps[ FOOTSTEP_DIRT ][rand()&3] );
        }
        break; 
    case EV_FOOTSPLASH:
        DEBUGNAME("EV_FOOTSPLASH");
        if (cg_footsteps.integer && (cg.DeafTime < cg.time)) {
            trap_S_StartSoundExtended(NULL, 1.0, SEALS_FOOTSTEP_FALLOFF, 1.0, es->number, CHAN_BODY,
                               cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
        }
        break;
    case EV_FOOTWADE:
        DEBUGNAME("EV_FOOTWADE");
        if (cg_footsteps.integer && (cg.DeafTime < cg.time)) {
            trap_S_StartSoundExtended(NULL, 1.0, SEALS_FOOTSTEP_FALLOFF, 1.0,  es->number, CHAN_BODY,
                               cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
        }
        break;
    case EV_SWIM:
        DEBUGNAME("EV_SWIM");
        if (cg_footsteps.integer && (cg.DeafTime < cg.time)) {
            trap_S_StartSoundExtended(NULL, 1.0, SEALS_FOOTSTEP_FALLOFF, 1.0f, es->number, CHAN_BODY,
                               cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
        }
        break;
    case EV_FALL_SHORT:
        DEBUGNAME("EV_FALL_SHORT");
        if ((cg.DeafTime < cg.time)) trap_S_StartSoundExtended(NULL, 1.0, SEALS_FOOTSTEP_FALLOFF, 1.0f, es->number, CHAN_AUTO, cgs.media.landSound );
        if ( clientNum == cg.predictedPlayerState.clientNum ) {
            // smooth landing z changes
            cg.landChange = -8;
            cg.landTime = cg.time;
        }
        break;
    case EV_FALL_LIGHT:
        DEBUGNAME("EV_FALL_LIGHT");
        // use normal pain sound
        if ((cg.DeafTime < cg.time)) trap_S_StartSoundExtended(NULL, 1.0, SEALS_FOOTSTEP_FALLOFF, 1.0f, es->number, CHAN_AUTO, cgs.media.landSound );
        if ( clientNum == cg.predictedPlayerState.clientNum ) {
            // smooth landing z changes
            cg.landChange = -16;
            cg.landTime = cg.time;
        }
        break;
    case EV_FALL_MEDIUM:
        DEBUGNAME("EV_FALL_MEDIUM");
        // use normal pain sound
        if ((cg.DeafTime < cg.time)) trap_S_StartSoundExtended(NULL, 1.0, SEALS_FOOTSTEP_FALLOFF, 1.0f, es->number, CHAN_AUTO, cgs.media.landSound );
        if ( clientNum == cg.predictedPlayerState.clientNum ) {
            CG_DamageFeedback( 50+random()*50,50+random()*50, 40, 3000 );
            // smooth landing z changes
            cg.landChange = -16;
            cg.landTime = cg.time;
        }
        break;
    case EV_FALL_FAR:
    case EV_FALL_DEATH:
        DEBUGNAME("EV_FALL_FAR");
        if ((cg.DeafTime < cg.time)) trap_S_StartSoundExtended(NULL, 1.0, SEALS_FOOTSTEP_FALLOFF, 1.0f, es->number, CHAN_AUTO, trap_S_RegisterSound("sound/actors/player/fall1.wav", qfalse) );
        cent->pe.painTime = cg.time;	// don't play a pain sound right after this
        if ( clientNum == cg.predictedPlayerState.clientNum ) {
            CG_DamageFeedback( 75+random()*75,75+random()*75, 40, 5000 );
            // smooth landing z changes
            cg.landChange = -24;
            cg.landTime = cg.time;
        }
        break;

    case EV_STEP_4:
    case EV_STEP_8:
    case EV_STEP_12:
    case EV_STEP_16:		// smooth out step up transitions
        DEBUGNAME("EV_STEP");
        {
            float	oldStep;
            int		delta;
            int		step;

            if ( clientNum != cg.predictedPlayerState.clientNum ) {
                break;
            }
            // if we are interpolating, we don't need to smooth steps
            if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) ||
                    cg_nopredict.integer || cg_synchronousClients.integer ) {
                break;
            }
            // check for stepping up before a previous step is completed
            delta = cg.time - cg.stepTime;
            if (delta < STEP_TIME) {
                oldStep = cg.stepChange * (STEP_TIME - delta) / STEP_TIME;
            } else {
                oldStep = 0;
            }

            // add this amount
            step = 4 * (event - EV_STEP_4 + 1 );
            cg.stepChange = oldStep + step;
            if ( cg.stepChange > MAX_STEP_CHANGE ) {
                cg.stepChange = MAX_STEP_CHANGE;
            }
            cg.stepTime = cg.time;
            break;
        }

    case EV_STOLENWEAPON:
        DEBUGNAME("EV_STOLENWEAPON");
        if ( es->otherEntityNum != cg.snap->ps.clientNum )
            return;
        if ( BG_IsPrimary( es->weapon ) )
            trap_Cvar_Set( "inven_primary", va( "%i", es->weapon ) );
        else if ( BG_IsSecondary( es->weapon ) )
            trap_Cvar_Set( "inven_secondary", va( "%i", es->weapon ) );
        break;

    case EV_JUMP:
        DEBUGNAME("EV_JUMP"); 
        break;
    case EV_TAUNT:
        DEBUGNAME("EV_TAUNT"); 
        break;

    case EV_WATER_TOUCH:
        DEBUGNAME("EV_WATER_TOUCH");
        if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrInSound );
        break;
    case EV_WATER_LEAVE:
        DEBUGNAME("EV_WATER_LEAVE");
        if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrOutSound );
        break;
    case EV_WATER_UNDER:
        DEBUGNAME("EV_WATER_UNDER");
        if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrUnSound );
        break;
    case EV_WATER_CLEAR:
        DEBUGNAME("EV_WATER_CLEAR");
        if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, es->number, CHAN_AUTO, trap_S_RegisterSound("sound/actors/player/gasp.wav", qfalse));
        break;

    case EV_ITEM_PICKUP:
        DEBUGNAME("EV_ITEM_PICKUP");
        {
            gitem_t	*item;
            int		index;

            index = es->eventParm;		// player predicted

            if ( index < 1 || index >= bg_numItems ) {
                break;
            }
            item = &bg_itemlist[ index ];

            // powerups and team items will have a separate global sound, this one
            // will be played at prediction time
            if ( item->giType == IT_POWERUP || item->giType == IT_TEAM) {
                //				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.n_healthSound );
            } else {
                if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, es->number, CHAN_AUTO,	trap_S_RegisterSound( item->pickup_sound, qfalse ) );
            }

            // show icon and name on status bar
            if ( es->number == cg.snap->ps.clientNum ) {
                CG_ItemPickup( index );
            }
        }
        break;

    case EV_LOCALSOUND:
        DEBUGNAME("EV_LOCALSOUND");
        {
            if ( es->otherEntityNum != cg.snap->ps.clientNum )
                break;

            // es->frame			=		channel
            // es->otherEntityNum   =		client to play to
            // es->eventParm		=		soundfile

            if ( cgs.gameSounds[ es->eventParm ] ) {
                trap_S_StartLocalSound( cgs.gameSounds[ es->eventParm ] ,  es->frame );
            } else {
                s = CG_ConfigString( CS_SOUNDS + es->eventParm );
                trap_S_StartLocalSound( CG_CustomSound( es->number, s ) ,  es->frame );
            }
        }
        break;
    case EV_GLOBAL_ITEM_PICKUP:
        DEBUGNAME("EV_GLOBAL_ITEM_PICKUP");
        {
            gitem_t	*item;
            int		index;

            index = es->eventParm;		// player predicted

            if ( index < 1 || index >= bg_numItems ) {
                break;
            }
            item = &bg_itemlist[ index ];

            // powerup pickups are global
            if( item->pickup_sound ) {
                if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, trap_S_RegisterSound( item->pickup_sound, qfalse ) );
            }

            // show icon and name on status bar
            if ( es->number == cg.snap->ps.clientNum ) {
                CG_ItemPickup( index );
            }
        }
        break;
    case EV_BREAKLOCK:
        DEBUGNAME("EV_BREAKLOCK");
        {
            CG_Printf("broke lock\n");
            CG_LockBreak( cent );
        }
        break;
        //
        // weapon events
        //
    case EV_NOAMMO:
        DEBUGNAME("EV_NOAMMO");
        //		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.noAmmoSound );
#if 0
        if ( es->number == cg.snap->ps.clientNum ) {
            CG_OutOfAmmoChange();
        }
#endif
        break;
    case EV_CHANGE_WEAPON:
        DEBUGNAME("EV_CHANGE_WEAPON");
        cent->gunSmokeTime = 0; // reset this
        cent->pe.hand_weapon.frame = 0; // also reset frame counter
        // BLUTENGEL: 07.01.2004
        // this sound effect is ok
        if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.selectSound );
        break;
    case EV_FIRE_WEAPON_OTHER:
        DEBUGNAME("EV_FIRE_WEAPON_OTHER");
        CG_FireWeapon( cent, qtrue );
        break;
    case EV_FIRE_WEAPON:
        DEBUGNAME("EV_FIRE_WEAPON");
        CG_FireWeapon( cent, qfalse );
        break;

    case EV_USE_ITEM0:
        DEBUGNAME("EV_USE_ITEM0");
        CG_UseItem( cent );
        break;
    case EV_USE_ITEM1:
        DEBUGNAME("EV_USE_ITEM1");
        CG_UseItem( cent );
        break;
    case EV_USE_ITEM2:
        DEBUGNAME("EV_USE_ITEM2");
        CG_UseItem( cent );
        break;
    case EV_USE_ITEM3:
        DEBUGNAME("EV_USE_ITEM3");
        CG_UseItem( cent );
        break;
    case EV_USE_ITEM4:
        DEBUGNAME("EV_USE_ITEM4");
        CG_UseItem( cent );
        break;
    case EV_USE_ITEM5:
        DEBUGNAME("EV_USE_ITEM5");
        CG_UseItem( cent );
        break;
    case EV_USE_ITEM6:
        DEBUGNAME("EV_USE_ITEM6");
        CG_UseItem( cent );
        break;
    case EV_USE_ITEM7:
        DEBUGNAME("EV_USE_ITEM7");
        CG_UseItem( cent );
        break;
    case EV_USE_ITEM8:
        DEBUGNAME("EV_USE_ITEM8");
        CG_UseItem( cent );
        break;
    case EV_USE_ITEM9:
        DEBUGNAME("EV_USE_ITEM9");
        CG_UseItem( cent );
        break;
    case EV_USE_ITEM10:
        DEBUGNAME("EV_USE_ITEM10");
        CG_UseItem( cent );
        break;
    case EV_USE_ITEM11:
        DEBUGNAME("EV_USE_ITEM11");
        CG_UseItem( cent );
        break;
    case EV_USE_ITEM12:
        DEBUGNAME("EV_USE_ITEM12");
        CG_UseItem( cent );
        break;
    case EV_USE_ITEM13:
        DEBUGNAME("EV_USE_ITEM13");
        CG_UseItem( cent );
        break;
    case EV_USE_ITEM14:
        DEBUGNAME("EV_USE_ITEM14");
        CG_UseItem( cent );
        break;

        //=================================================================

        //
        // other events
        //
    case EV_PLAYER_TELEPORT_IN:
        DEBUGNAME("EV_PLAYER_TELEPORT_IN");
        if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleInSound );
        CG_SpawnEffect( position );
        break;

    case EV_PLAYER_TELEPORT_OUT:
        DEBUGNAME("EV_PLAYER_TELEPORT_OUT");
        if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleOutSound );
        CG_SpawnEffect(  position );
        break;

    case EV_ITEM_POP:
        DEBUGNAME("EV_ITEM_POP");
        break;
    case EV_ITEM_RESPAWN:
        DEBUGNAME("EV_ITEM_RESPAWN");
        cent->miscTime = cg.time;	// scale up from this
        break;

    case EV_GRENADE_BOUNCE:
        DEBUGNAME("EV_GRENADE_BOUNCE");
        if ( rand() & 1 ) {
            if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.hgrenb1aSound );
        } else {
            if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.hgrenb2aSound );
        }
        break;


    case EV_SCOREPLUM:
        DEBUGNAME("EV_SCOREPLUM");
        CG_ScorePlum( cent->currentState.otherEntityNum, cent->lerpOrigin, cent->currentState.time );
        break;

        //
        // missile impacts
        //
    case EV_MISSILE_HIT:
        DEBUGNAME("EV_MISSILE_HIT");
        ByteToDir( es->eventParm, dir );
        CG_MissileHitPlayer( es->weapon, position, dir, es->otherEntityNum , 50);
        break;

    case EV_MISSILE_MISS:
        DEBUGNAME("EV_MISSILE_MISS");
        ByteToDir( es->eventParm, dir );
        CG_MissileHitWall( es->weapon, 0, position, dir, IMPACTSOUND_DEFAULT, 0 );
        break;

    case EV_MISSILE_MISS_METAL:
        DEBUGNAME("EV_MISSILE_MISS_METAL");
        ByteToDir( es->eventParm, dir );
        CG_MissileHitWall( es->weapon, 0, position, dir, IMPACTSOUND_METAL, 0 );
        break;

    case EV_RAILTRAIL:
        DEBUGNAME("EV_RAILTRAIL");
        break;

    case EV_BULLET_HIT_WALL:
        DEBUGNAME("EV_BULLET_HIT_WALL");
        ByteToDir( es->eventParm, dir );
        if ( es->frame == 0 && cg.snap->ps.clientNum == es->otherEntityNum
                && !BG_IsGrenade( es->weapon )
                && !BG_IsMelee( es->weapon )
                && !BG_IsGrenade( es->weapon )
                && cg_antiLag.integer 
           )
            break;

        if ( es->otherEntityNum2 > 0 )
        {
            CG_Bullet( es->pos.trBase, es->apos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD, -1, es->otherEntityNum2, es->generic1, es->weapon );
            break;
        }
        CG_Bullet( es->pos.trBase, es->apos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD, -1, es->frame, es->generic1, es->weapon  );
        break;


    case EV_BULLET_HIT_FLESH:
        DEBUGNAME("EV_BULLET_HIT_FLESH");
        ByteToDir( es->torsoAnim , dir );
        CG_Bullet( es->pos.trBase, es->apos.trBase, es->otherEntityNum, dir, qtrue, es->eventParm, es->legsAnim, es->frame, 0, es->weapon  );
        break;

    case EV_SHOTGUN:
        DEBUGNAME("EV_SHOTGUN");
        CG_ShotgunFire( es );
        break;

    case EV_GENERAL_SOUND:
        DEBUGNAME("EV_GENERAL_SOUND");
        if ( cgs.gameSounds[ es->eventParm ] ) {
            if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
        } else {
            s = CG_ConfigString( CS_SOUNDS + es->eventParm );
            if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, s ) );
        }
        break;
    case EV_EXPLOSION:
        DEBUGNAME("EV_EXPLOSION");
        CG_Explosion( position, es->legsAnim );
        break;
    case EV_GLOBAL_SOUND:	// play from the player's head so it never diminishes
        DEBUGNAME("EV_GLOBAL_SOUND");
        if ( cgs.gameSounds[ es->eventParm ] ) {
            if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, cgs.gameSounds[ es->eventParm ] );
        } else {
            s = CG_ConfigString( CS_SOUNDS + es->eventParm );
            if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, CG_CustomSound( es->number, s ) );
        }
        break;

    case EV_GLOBAL_TEAM_SOUND:	// play from the player's head so it never diminishes
        {
            DEBUGNAME("EV_GLOBAL_TEAM_SOUND");
            switch( es->eventParm ) {
            case GTS_DRAW_ROUND:
                CG_AddBufferedSound( cgs.media.roundDrawSound );
                break;
            case GTS_REDTEAM_TOOK_LEAD:
                CG_AddBufferedSound( cgs.media.redLeadsSound );
                break;
            case GTS_BLUETEAM_TOOK_LEAD:
                CG_AddBufferedSound( cgs.media.blueLeadsSound );
                break;
            case GTS_REDTEAM_SCORED:
                CG_AddBufferedSound( cgs.media.redScoredSound );
                break;
            case GTS_BLUETEAM_SCORED:
                CG_AddBufferedSound( cgs.media.blueScoredSound );
                break;
            case GTS_BRIEFCASE_TAKEN:
                CG_AddBufferedSound( trap_S_RegisterSound("sound/commentary/bfc_stolen.wav", qfalse ) );
                if ( cg.snap->ps.powerups[PW_BRIEFCASE] <= 0 )
                    if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE && !cg.ns_newbiehelp.w_briefcaseTaken_Tango )
                    {
                        // tango player
                        cg.ns_newbiehelp.w_briefcaseTaken_Tango = qtrue;
                        CG_NewbieMessage(S_COLOR_GREEN "The Briefcase has been stolen.\nTry to kill the carrier..", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );


                    }
                    else if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED && !cg.ns_newbiehelp.w_briefcaseTaken_Seal )
                    {
                        cg.ns_newbiehelp.w_briefcaseTaken_Seal = qtrue;
                        CG_NewbieMessage(S_COLOR_GREEN "The Briefcase has been stolen by your Team.\nDefend the carrier at all cost.", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
                        // seal player
                    }
                break;
            case GTS_RED_BRIEFCASE_RETURN:
                CG_AddBufferedSound( trap_S_RegisterSound("sound/commentary/bfc_sls_captured.wav", qfalse ) );
                break;
            case GTS_BLUE_BRIEFCASE_RETURN:
                CG_AddBufferedSound( trap_S_RegisterSound("sound/commentary/bfc_tgs_captured.wav", qfalse ) );
                break;

            default:
                break;
            }
            break;
        }

    case EV_PAIN:
        // local player sounds are triggered in CG_CheckLocalSounds,
        // so ignore events on the player
        DEBUGNAME("EV_PAIN");
        if ( cent->currentState.number != cg.snap->ps.clientNum ) {
            CG_PainEvent( cent, es->eventParm );
        }
        break;

    case EV_BLOODPOOL:
        DEBUGNAME("EV_BLOODPOOL");
        CG_BloodPool( cent->lerpOrigin );
        break;
    case EV_DEATH:
        {
            int	rndnum = (random()*2);
            DEBUGNAME("EV_DEATHx");

            rndnum++;

            trap_S_StartSound( NULL, es->number, CHAN_VOICE, cgs.media.deathSounds[rndnum] );
        }
        break;
    case EV_OBITUARY:
        DEBUGNAME("EV_OBITUARY");
        CG_Obituary( es );
        break;

    case EV_GIB_PLAYER:
        DEBUGNAME("EV_GIB_PLAYER");
        // don't play gib sound when using the kamikaze because it interferes
        // with the kamikaze sound, downside is that the gib sound will also
        // not be played when someone is gibbed while just carrying the kamikaze
        /*		if ( !(es->eFlags & EF_KAMIKAZE) ) {
        trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
        }*/
        CG_GibPlayer( cent->lerpOrigin );
        break;

    case EV_STOPLOOPINGSOUND:
        DEBUGNAME("EV_STOPLOOPINGSOUND");
        trap_S_StopLoopingSound( es->number );
        es->loopSound = 0;
        break;
    case EV_DEBUG_LINE:
        DEBUGNAME("EV_DEBUG_LINE");
        CG_Beam( cent );
        break;
    case	EV_BLOODER:
        DEBUGNAME("EV_BLOODER");
        CG_CreateBleeder( cent->lerpOrigin , es->eventParm, es->otherEntityNum );

        //		CG_CreateBleeder(cent->lerpOrigin , es->torsoAnim  );
        break;
    case	EV_BLOOD_ON_WALL:
        DEBUGNAME("EV_BLOOD_ON_WALL");
        ByteToDir( es->eventParm, dir );

        if (es->legsAnim)
        {
            char head_sound[64];
            int	rndnum = (int)(random()*3);
            int	anim = cg_entities[ es->otherEntityNum ].currentState.legsAnim & ~ANIM_TOGGLEBIT;
            vec3_t headOrigin;
            vec3_t dir;

            VectorCopy( cg_entities[ es->otherEntityNum ].lerpOrigin, headOrigin );

            if ( anim == LEGS_WALKCR || anim == LEGS_IDLECR ) {
                headOrigin[2] += CROUCH_VIEWHEIGHT;
            } else {
                headOrigin[2] += DEFAULT_VIEWHEIGHT; // to get an accurate point of exuast
            }

            rndnum++;

            Com_sprintf(head_sound,sizeof(head_sound),"sound/actors/headblow_%i.wav", rndnum);

            ByteToDir(es->generic1, dir );
            //CG_HeadBlowGFX( headOrigin , dir);
            CG_RealBloodTrail( headOrigin, es->pos.trBase , 6);
            //CG_Printf("HEADSOUND^1 %s\n",head_sound );
            if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, es->otherEntityNum, CHAN_AUTO, trap_S_RegisterSound(head_sound, qfalse) );
        }
        CG_BloodOnWallMark( es->pos.trBase,dir, es->torsoAnim , es->legsAnim );
        break;
    case EV_EMPTYCLIP:
        DEBUGNAME("EV_EMPTYCLIP");
        if ( es->number == cg.snap->ps.clientNum )
            CG_ReloadClipWarning();
        break;
    case EV_FUNCEXPLOSIVE:
        // delete all marks of this object
        {
            int dmark = es->frame;

            CG_DeleteDirectMark( dmark );
            NS_CG_LaunchFuncExplosive( cent );
            break;
        }
    case EV_RELOAD:
        CG_ReloadWeapon( cent, 0 );
        break;
    case EV_RELOAD_EMPTY:
        CG_ReloadWeapon( cent, 1 );
        break;
    case EV_FLASHBANG:
        if ( es->otherEntityNum == cg.snap->ps.clientNum )
        {
            int blindtime = es->frame;
            int i = 0, j = 0;

            if (blindtime < 0) break;

            if (blindtime < 10 )
                blindtime = 10;

            // play the flashbangsound according to the time blinded
            if (blindtime > 6000)
                trap_S_StartLocalSound( cgs.media.flash_8sec , CHAN_AUTO );
            else if (blindtime > 4000)
                trap_S_StartLocalSound( cgs.media.flash_6sec , CHAN_AUTO );
            else if (blindtime > 2000)
                trap_S_StartLocalSound( cgs.media.flash_4sec , CHAN_AUTO );
            else
                trap_S_StartLocalSound( cgs.media.flash_2sec , CHAN_AUTO );

            // calculate the duration of deafeffect
            j = cg.time + blindtime - SEALS_FLASHBANGFADETIME;

            if ((cg.time + SEALS_CONCUSSIONFACTOR*blindtime) > cg.ConcussionTime)
                cg.ConcussionTime = cg.time + SEALS_CONCUSSIONFACTOR*blindtime;

            // recalc blindtime if blinded from behind
            if (es->eventParm)
                blindtime *= SEALS_FLASHEDBEHINDFACTOR;

            // calculate the duration of whiteouteffect
            i = cg.time + blindtime - SEALS_FLASHBANGFADETIME;

            // if it's bigger than the actual whiteouteffect, change it
            if (i > cg.FlashTime) cg.FlashTime = i;

            if (j > cg.DeafTime) cg.DeafTime = j;

            // calculate the duration of blindspot effect
            i = cg.time + blindtime*SEALS_BLINDSPOTFACTOR;

            // only change it if the player looked into the flashbang _AND_
            // the new effect duration if larger than the old one
            if ( !es->eventParm  && (i > cg.flashedVisionTime) )
                cg.flashedVisionTime = i;

        }
        break;
    case EV_C4DEPLOY:
        break;
    case EV_BANDAGING:
        //		CG_Printf("Bandaging!\n");
        break;
    case EV_CLIPWIRE_1:
    case EV_CLIPWIRE_2:
    case EV_CLIPWIRE_3:
    case EV_CLIPWIRE_4:
    case EV_CLIPWIRE_5:
    case EV_CLIPWIRE_6:
    case EV_CLIPWIRE_7:
    case EV_CLIPWIRE_8:
        break;

    default:
        DEBUGNAME("UNKNOWN");
        CG_Error( "Unknown event: %i", event );
        break;
    }

}


/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents( centity_t *cent ) {
    // check for event-only entities
    if ( cent->currentState.eType > ET_EVENTS ) {
        if ( cent->previousEvent ) {
            return;	// already fired
        }

        cent->previousEvent = 1;

        cent->currentState.event = cent->currentState.eType - ET_EVENTS;
    } else {
        // check for events riding with another entity
        if ( cent->currentState.event == cent->previousEvent ) {
            return;
        }
        cent->previousEvent = cent->currentState.event;
        if ( ( cent->currentState.event & ~EV_EVENT_BITS ) == 0 ) {
            return;
        }
    }

    // calculate the position at exactly the frame time
    BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin );
    CG_SetEntitySoundPosition( cent );

    CG_EntityEvent( cent, cent->lerpOrigin );
}

