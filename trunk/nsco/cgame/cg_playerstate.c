// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_playerstate.c -- this file acts on changes in a new playerState_t
// With normal play, this will be done after local prediction, but when
// following another player or playing back a demo, it will be checked
// when the snapshot transitions like all the other entities

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"


#if 0

// defcon-X:  I decided to leave this in, might be ofuse sometime later

/*
==============
CG_CheckAmmo

If the ammo has gone low enough to generate the warning, play a sound
==============
*/
void CG_CheckAmmo( void ) {
    //	int		i;
    //	int		total;
    //	int		previous;
    //	int		weapons;

    // see about how many seconds of ammo we have remaining
    weapons = cg.snap->ps.stats[ STAT_WEAPONS ];
    total = 0;
    for ( i = WP_MACHINEGUN ; i < WP_NUM_WEAPONS ; i++ ) {
        if ( ! ( weapons & ( 1 << i ) ) ) {
            continue;
        }
        switch ( i ) {
        case WP_ROCKET_LAUNCHER:
        case WP_GRENADE_LAUNCHER:
        case WP_RAILGUN:
        case WP_SHOTGUN:
        case WP_PROX_LAUNCHER:

            total += cg.snap->ps.ammo[i] * 1000;
            break;
        default:
            total += cg.snap->ps.ammo[i] * 200;
            break;
        }
        if ( total >= 5000 ) {
            cg.lowAmmoWarning = 0;
            return;
        }
    }

    previous = cg.lowAmmoWarning;

    if ( total == 0 ) {
        cg.lowAmmoWarning = 2;
    } else {
        cg.lowAmmoWarning = 1;
    }

    // play a sound on transitions
    if ( cg.lowAmmoWarning != previous ) {
        trap_S_StartLocalSound( cgs.media.noAmmoSound, CHAN_LOCAL_SOUND );
    }

}
#endif

/*
==============
CG_DamageFeedback
==============
*/
void CG_DamageFeedback( int yawByte, int pitchByte, int damage, int damageDuration ) {
    float		left, front, up;
    float		kick;
    int			health;
    float		scale;
    vec3_t		dir;
    vec3_t		angles;
    float		dist;
    float		yaw, pitch;

    // show the attacking player's head and name in corner
    cg.attackerTime = cg.time;

    // the lower on health you are, the greater the view kick will be
    health = cg.snap->ps.stats[STAT_HEALTH];
    if ( health < 40 ) {
        scale = 1;
    } else {
        scale = 40.0 / health;
    }
    kick = damage * scale;

    if (kick < 5)
        kick = 5;
    if (kick > 50)
        kick = 50;

    // if yaw and pitch are both 255, make the damage always centered (falling, etc)
    if ( yawByte == 255 && pitchByte == 255 ) {
        cg.damageX = 0;
        cg.damageY = 0;
        cg.v_dmg_roll = 0;
        cg.v_dmg_pitch = -kick;
    } else {
        // positional
        pitch = pitchByte / 255.0 * 360;
        yaw = yawByte / 255.0 * 360;

        angles[PITCH] = pitch;
        angles[YAW] = yaw;
        angles[ROLL] = 0;

        AngleVectors( angles, dir, NULL, NULL );
        VectorSubtract( vec3_origin, dir, dir );

        front = DotProduct (dir, cg.refdef.viewaxis[0] );
        left = DotProduct (dir, cg.refdef.viewaxis[1] );
        up = DotProduct (dir, cg.refdef.viewaxis[2] );

        dir[0] = front;
        dir[1] = left;
        dir[2] = 0;
        dist = VectorLength( dir );
        if ( dist < 0.1 ) {
            dist = 0.1f;
        }

        cg.v_dmg_roll = kick * left;

        cg.v_dmg_pitch = -kick * front;

        if ( front <= 0.1 ) {
            front = 0.1f;
        }
        cg.damageX = -left / front;
        cg.damageY = up / dist;
    }

    // clamp the position
    if ( cg.damageX > 1.0 ) {
        cg.damageX = 1.0;
    }
    if ( cg.damageX < - 1.0 ) {
        cg.damageX = -1.0;
    }

    if ( cg.damageY > 1.0 ) {
        cg.damageY = 1.0;
    }
    if ( cg.damageY < - 1.0 ) {
        cg.damageY = -1.0;
    }

    // don't let the screen flashes vary as much
    if ( kick > 10 ) {
        kick = 10;
    }
    cg.damageValue = kick;
    cg.v_dmg_time = cg.time + damageDuration;
    cg.damageTime = cg.snap->serverTime;
    cg.damageDuration = damageDuration;
}




/*
================
CG_Respawn

A respawn happened this snapshot
================
*/
void CG_Respawn( void ) {
    // no error decay on player movement
    cg.thisFrameTeleport = qtrue;

    // display weapons available
    cg.weaponSelectTime = cg.time;


    cg.damageDuration = 0;
    // clear all these
    cg.flashedVision_x = 0;
    cg.flashedVision_y = 0;
    cg.flashedVisionTime = 0;
    cg.FlashTime = 0;
    cg.ConcussionTime = 0;
    cg.damageTime = 0;
    cg.DeafTime = 0;

    cg.deathZoom = 0;
    cg.deathRotation = 0;


    // restart at idle frame
    cg.predictedPlayerEntity.pe.weapAnim = WANIM_IDLE;
    cg.predictedPlayerEntity.pe.nextweapAnim = WANIM_IDLE;

    cg.weaponPos = 1.0f;

    // select the weapon the server says we are using
    cg.weaponSelect = cg.snap->ps.weapon;

    cg.DeathBlendTime = cg.time + 500;

    trap_SendConsoleCommand( "; -movedown;" );
}

extern char *eventnames[];

/*
==============
CG_CheckPlayerstateEvents
==============
*/
void CG_CheckPlayerstateEvents( playerState_t *ps, playerState_t *ops ) {
    int			i;
    int			event;
    centity_t	*cent;

    if ( ps->externalEvent && ps->externalEvent != ops->externalEvent ) {
        cent = &cg_entities[ ps->clientNum ];
        cent->currentState.event = ps->externalEvent;
        cent->currentState.eventParm = ps->externalEventParm;
        CG_EntityEvent( cent, cent->lerpOrigin );
    }

    cent = &cg.predictedPlayerEntity; // cg_entities[ ps->clientNum ];
    // go through the predictable events buffer
    for ( i = ps->eventSequence - MAX_PS_EVENTS ; i < ps->eventSequence ; i++ ) {
        // if we have a new predictable event
        if ( i >= ops->eventSequence
                // or the server told us to play another event instead of a predicted event we already issued
                // or something the server told us changed our prediction causing a different event
                || (i > ops->eventSequence - MAX_PS_EVENTS && ps->events[i & (MAX_PS_EVENTS-1)] != ops->events[i & (MAX_PS_EVENTS-1)]) ) {

            event = ps->events[ i & (MAX_PS_EVENTS-1) ];
            cent->currentState.event = event;
            cent->currentState.eventParm = ps->eventParms[ i & (MAX_PS_EVENTS-1) ];
            CG_EntityEvent( cent, cent->lerpOrigin );

            cg.predictableEvents[ i & (MAX_PREDICTED_EVENTS-1) ] = event;

            cg.eventSequence++;
        }
    }
}

/*
==================
CG_CheckChangedPredictableEvents
==================
*/
void CG_CheckChangedPredictableEvents( playerState_t *ps ) {
    int i;
    int event;
    centity_t	*cent;

    cent = &cg.predictedPlayerEntity;
    for ( i = ps->eventSequence - MAX_PS_EVENTS ; i < ps->eventSequence ; i++ ) {
        //
        if (i >= cg.eventSequence) {
            continue;
        }
        // if this event is not further back in than the maximum predictable events we remember
        if (i > cg.eventSequence - MAX_PREDICTED_EVENTS) {
            // if the new playerstate event is different from a previously predicted one
            if ( ps->events[i & (MAX_PS_EVENTS-1)] != cg.predictableEvents[i & (MAX_PREDICTED_EVENTS-1) ] ) {

                event = ps->events[ i & (MAX_PS_EVENTS-1) ];
                cent->currentState.event = event;
                cent->currentState.eventParm = ps->eventParms[ i & (MAX_PS_EVENTS-1) ];
                CG_EntityEvent( cent, cent->lerpOrigin );

                cg.predictableEvents[ i & (MAX_PREDICTED_EVENTS-1) ] = event;

                if ( cg_showmiss.integer ) {
                    CG_Printf("WARNING: changed predicted event\n");
                }
            }
        }
    }
}

/*
==================
pushReward
==================
static void pushReward(sfxHandle_t sfx, qhandle_t shader, int rewardCount) {
if (cg.rewardStack < (MAX_REWARDSTACK-1)) {
cg.rewardStack++;
cg.rewardSound[cg.rewardStack] = sfx;
cg.rewardShader[cg.rewardStack] = shader;
cg.rewardCount[cg.rewardStack] = rewardCount;
}
}
*/

//void CG_SendInventory_f( );
/*
==================
CG_CheckLocalSounds
==================
*/

void CG_CheckLocalSounds( playerState_t *ps, playerState_t *ops ) {
    int			highScore,/* health,  armor*/  reward;
    char    buf[16];
    int     prim, sec;
    //	sfxHandle_t sfx;
    buf[0] = 'C';
    buf[1] = '1';
    buf[2] = '1';
    buf[3] = '1';
    buf[4] = '1';
    buf[5] = '1';
    buf[6] = '1';
    buf[7] = '\0';

    // don't play the sounds if the player just changed teams
    if ( ps->persistant[PERS_TEAM] != ops->persistant[PERS_TEAM]) {
        char var[MAX_TOKEN_CHARS];
        char model[256],skin[256];

        //CG_Printf(S_COLOR_RED"BLUTENGEL DEBUG MESSAGE: Teamchange occured [%i:%i:%i:%i:%i]\n", ps->persistant[PERS_TEAM], ops->persistant[PERS_TEAM], TEAM_RED, TEAM_BLUE, TEAM_SPECTATOR);

        // clear the XP for client, server and UI
        trap_Cvar_Set("character", buf);
        trap_Cvar_Set("ui_character", buf);

        // get the current weapons
        trap_Cvar_VariableStringBuffer("inven_primary", buf, sizeof(buf));
        prim = atoi(buf);
        trap_Cvar_VariableStringBuffer("inven_secondary", buf, sizeof(buf));
        sec = atoi(buf);

        // teamchange happend. update inventory
        if ( ps->persistant[PERS_TEAM] == TEAM_RED )
        {
            // we are seals now, check the weapons
            if (prim == WP_MAC10)
                trap_Cvar_Set("inven_primary", va("%i", WP_MP5));
            else if (prim == WP_AK47)
                trap_Cvar_Set("inven_primary", va("%i", WP_M4));
            else if (prim == WP_M590)
                trap_Cvar_Set("inven_primary", va("%i", WP_870));

            if (sec == WP_GLOCK)
                trap_Cvar_Set("inven_secondary", va("%i", WP_P9S));
            else if (sec == WP_SW40T)
                trap_Cvar_Set("inven_secondary", va("%i", WP_MK23));
            else if (sec == WP_DEAGLE)
                trap_Cvar_Set("inven_secondary", va("%i", WP_SW629));


            // update looks
            trap_Cvar_VariableStringBuffer("ui_s_e_eyes", var , sizeof(var ) );
            trap_Cvar_Set("e_eyes", var );
            trap_Cvar_VariableStringBuffer("ui_s_e_head", var , sizeof(var ) );
            trap_Cvar_Set("e_head", var );
            trap_Cvar_VariableStringBuffer("ui_s_e_mouth", var , sizeof(var ) );
            trap_Cvar_Set("e_mouth", var );

            // model
            trap_Cvar_VariableStringBuffer("ui_s_model", model , sizeof( model ) );
            trap_Cvar_VariableStringBuffer("ui_s_skin", skin , sizeof( skin ) );
            trap_Cvar_Set( "model", va("%s/%s",model,skin) );
        }
        else if ( ps->persistant[PERS_TEAM] == TEAM_BLUE )
        {
            // we are tangos now, check the weapons
            if (prim == WP_MP5)
                trap_Cvar_Set("inven_primary", va("%i", WP_MAC10));
            else if (prim == WP_M4)
                trap_Cvar_Set("inven_primary", va("%i", WP_AK47));
            else if (prim == WP_870)
                trap_Cvar_Set("inven_primary", va("%i", WP_M590));

            if (sec == WP_P9S)
                trap_Cvar_Set("inven_secondary", va("%i", WP_GLOCK));
            else if (sec == WP_MK23)
                trap_Cvar_Set("inven_secondary", va("%i", WP_SW40T));
            else if (sec == WP_SW629)
                trap_Cvar_Set("inven_secondary", va("%i", WP_DEAGLE));


            // update looks
            trap_Cvar_VariableStringBuffer("ui_t_e_eyes", var , sizeof(var ) );
            trap_Cvar_Set("e_eyes", var );
            trap_Cvar_VariableStringBuffer("ui_t_e_head", var , sizeof(var ) );
            trap_Cvar_Set("e_head", var );
            trap_Cvar_VariableStringBuffer("ui_t_e_mouth", var , sizeof(var ) );
            trap_Cvar_Set("e_mouth", var );

            // model
            trap_Cvar_VariableStringBuffer("ui_t_model", model , sizeof( model ) );
            trap_Cvar_VariableStringBuffer("ui_t_skin", skin , sizeof( skin ) );

            trap_Cvar_Set( "model", va("%s/%s",model,skin) );
        }

        return;
    }

    // health changes of more than -1 should call twitching screen
    if ( ps->stats[STAT_HEALTH] < ops->stats[STAT_HEALTH] - 1 ) {
        if ( ps->stats[STAT_HEALTH] > 0 ) {
            cg.predictedPlayerEntity.pe.painTime = cg.time;
            cg.predictedPlayerEntity.pe.painDirection ^= 1;
        }
    }


    // if we are going into the intermission, don't start any voices
    if ( cg.intermissionStarted ) {
        return;
    }

    // reward sounds
    reward = qfalse;
    /*
    if (ps->persistant[PERS_CAPTURES] != ops->persistant[PERS_CAPTURES]) {
    pushReward(cgs.media.captureAwardSound, cgs.media.medalCapture, ps->persistant[PERS_CAPTURES]);
    reward = qtrue;
    //Com_Printf("capture\n");
    }
    if (ps->persistant[PERS_IMPRESSIVE_COUNT] != ops->persistant[PERS_IMPRESSIVE_COUNT]) {

    sfx = cgs.media.impressiveSound;
    pushReward(sfx, cgs.media.medalImpressive, ps->persistant[PERS_IMPRESSIVE_COUNT]);
    reward = qtrue;
    //Com_Printf("impressive\n");
    }
    if (ps->persistant[PERS_EXCELLENT_COUNT] != ops->persistant[PERS_EXCELLENT_COUNT]) {
    #ifdef MISSIONPACK
    if (ps->persistant[PERS_EXCELLENT_COUNT] == 1) {
    sfx = cgs.media.firstExcellentSound;
    } else {
    sfx = cgs.media.excellentSound;
    }
    #else
    sfx = cgs.media.excellentSound;
    #endif
    pushReward(sfx, cgs.media.medalExcellent, ps->persistant[PERS_EXCELLENT_COUNT]);
    reward = qtrue;
    //Com_Printf("excellent\n");
    }
    if (ps->persistant[PERS_GAUNTLET_FRAG_COUNT] != ops->persistant[PERS_GAUNTLET_FRAG_COUNT]) {
    sfx = cgs.media.humiliationSound;
    pushReward(sfx, cgs.media.medalGauntlet, ps->persistant[PERS_GAUNTLET_FRAG_COUNT]);
    reward = qtrue;
    //Com_Printf("guantlet frag\n");
    }
    if (ps->persistant[PERS_DEFEND_COUNT] != ops->persistant[PERS_DEFEND_COUNT]) {
    pushReward(cgs.media.defendSound, cgs.media.medalDefend, ps->persistant[PERS_DEFEND_COUNT]);
    reward = qtrue;
    //Com_Printf("defend\n");
    }
    if (ps->persistant[PERS_ASSIST_COUNT] != ops->persistant[PERS_ASSIST_COUNT]) {
    pushReward(cgs.media.assistSound, cgs.media.medalAssist, ps->persistant[PERS_ASSIST_COUNT]);
    reward = qtrue;
    //Com_Printf("assist\n");
    }
    */
    // if any of the player event bits changed
#if 0
    if (ps->persistant[PERS_PLAYEREVENTS] != ops->persistant[PERS_PLAYEREVENTS]) {
        if ((ps->persistant[PERS_PLAYEREVENTS] & PLAYEREVENT_DENIEDREWARD) !=
                (ops->persistant[PERS_PLAYEREVENTS] & PLAYEREVENT_DENIEDREWARD)) {
            trap_S_StartLocalSound( cgs.media.deniedSound, CHAN_ANNOUNCER );
        }
        else if ((ps->persistant[PERS_PLAYEREVENTS] & PLAYEREVENT_GAUNTLETREWARD) !=
                 (ops->persistant[PERS_PLAYEREVENTS] & PLAYEREVENT_GAUNTLETREWARD)) {
            trap_S_StartLocalSound( cgs.media.humiliationSound, CHAN_ANNOUNCER );
        }
        else if ((ps->persistant[PERS_PLAYEREVENTS] & PLAYEREVENT_HOLYSHIT) !=
                 (ops->persistant[PERS_PLAYEREVENTS] & PLAYEREVENT_HOLYSHIT)) {
            trap_S_StartLocalSound( cgs.media.holyShitSound, CHAN_ANNOUNCER );
        }
        reward = qtrue;
    }
#endif
    // check for flag pickup
    /*
    if ( cgs.gametype >= GT_TEAM ) {
    if ((ps->powerups[PW_REDFLAG] != ops->powerups[PW_REDFLAG] && ps->powerups[PW_REDFLAG]) ||
    (ps->powerups[PW_BLUEFLAG] != ops->powerups[PW_BLUEFLAG] && ps->powerups[PW_BLUEFLAG])
    //(ps->powerups[PW_NEUTRALFLAG] != ops->powerups[PW_NEUTRALFLAG] && ps->powerups[PW_NEUTRALFLAG])
    )
    {
    trap_S_StartLocalSound( cgs.media.youHaveFlagSound, CHAN_ANNOUNCER );
    }
    }
    */

    // timelimit warnings
    if ( cgs.levelRoundStartTime > 0 )
    {
        int		msec;

        msec = cgs.levelRoundStartTime - cg.time;

        // break it down into seconds
        msec /= 1000;

        //fixme: precache sounds
        if ( msec <= 0 && !(cg.roundlimitWarnings & 8192 ) )
        {
            cg.roundlimitWarnings |= 1 | 2 | 4 | 8 | 16 | 32 | 64 | 128 | 256 | 512 | 1024 | 2048 | 4096 | 8192;
            // 1 seconds left
            trap_S_StartLocalSound( trap_S_RegisterSound( "sound/commentary/rt_limithit.wav", qfalse ) , CHAN_ANNOUNCER );
        }
        if ( msec <= 1 && !(cg.roundlimitWarnings & 4096 ) )
        {
            cg.roundlimitWarnings |= 1 | 2 | 4 | 8 | 16 | 32 | 64 | 128 | 256 | 512 | 1024 | 2048 | 4096;
            // 1 seconds left
            trap_S_StartLocalSound( trap_S_RegisterSound( "sound/commentary/1.wav", qfalse ) , CHAN_ANNOUNCER );
        }
        else if ( msec <= 2 && !(cg.roundlimitWarnings & 2048 ) )
        {
            cg.roundlimitWarnings |= 1 | 2 | 4 | 8 | 16 | 32 | 64 | 128 | 256 | 512 | 1024 | 2048;
            // 2 seconds left
            trap_S_StartLocalSound( trap_S_RegisterSound( "sound/commentary/2.wav", qfalse ) , CHAN_ANNOUNCER );
        }
        else if ( msec <= 3 && !(cg.roundlimitWarnings & 1024 ) )
        {
            cg.roundlimitWarnings |= 1 | 2 | 4 | 8 | 16 | 32 | 64 | 128 | 256 | 512 | 1024;
            // 3 seconds left
            trap_S_StartLocalSound( trap_S_RegisterSound( "sound/commentary/3.wav", qfalse ) , CHAN_ANNOUNCER );
        }
        else if ( msec <= 4 && !(cg.roundlimitWarnings & 512 ) )
        {
            cg.roundlimitWarnings |= 1 | 2 | 4 | 8 | 16 | 32 | 64 | 128 | 256 | 512;
            // 4 seconds left
            trap_S_StartLocalSound( trap_S_RegisterSound( "sound/commentary/4.wav", qfalse ) , CHAN_ANNOUNCER );
        }
        else if ( msec <= 5 && !(cg.roundlimitWarnings & 256 ) )
        {
            cg.roundlimitWarnings |= 1 | 2 | 4 | 8 | 16 | 32 | 64 | 128 | 256;
            // 5 seconds left
            trap_S_StartLocalSound( trap_S_RegisterSound( "sound/commentary/5.wav", qfalse ) , CHAN_ANNOUNCER );
        }
        else if ( msec <= 6 && !(cg.roundlimitWarnings & 128 ) )
        {
            cg.roundlimitWarnings |= 1 | 2 | 4 | 8 | 16 | 32 | 64 | 128;
            // 6 seconds left
            trap_S_StartLocalSound( trap_S_RegisterSound( "sound/commentary/6.wav", qfalse ) , CHAN_ANNOUNCER );
        }
        else if ( msec <= 7 && !(cg.roundlimitWarnings & 64 ) )
        {
            cg.roundlimitWarnings |= 1 | 2 | 4 | 8 | 16 | 32 | 64;
            // 7 seconds left
            trap_S_StartLocalSound( trap_S_RegisterSound( "sound/commentary/7.wav", qfalse ) , CHAN_ANNOUNCER );
        }
        else if ( msec <= 8 && !(cg.roundlimitWarnings & 32 ) )
        {
            cg.roundlimitWarnings |= 1 | 2 | 4 | 8 | 16 | 32;
            // 8 seconds left
            trap_S_StartLocalSound( trap_S_RegisterSound( "sound/commentary/8.wav", qfalse ) , CHAN_ANNOUNCER );
        }
        else if ( msec <= 9 && !(cg.roundlimitWarnings & 16 ) )
        {
            cg.roundlimitWarnings |= 1 | 2 | 4 | 8 | 16 ;
            // 9 seconds left
            trap_S_StartLocalSound( trap_S_RegisterSound( "sound/commentary/9.wav", qfalse ) , CHAN_ANNOUNCER );
        }
        else if ( msec <= 10 && !(cg.roundlimitWarnings & 8 ) )
        {
            cg.roundlimitWarnings |= 1 | 2 | 4 | 8  ;
            // 10 seconds left
            trap_S_StartLocalSound( trap_S_RegisterSound( "sound/commentary/10.wav", qfalse ) , CHAN_ANNOUNCER );
        }
        else if ( msec <= 30 && !(cg.roundlimitWarnings & 4 ) )
        {
            cg.roundlimitWarnings |= 1 | 2 | 4  ;
            // 30 seconds left
            trap_S_StartLocalSound( trap_S_RegisterSound( "sound/commentary/30seconds.wav", qfalse ) , CHAN_ANNOUNCER );
        }
        else if ( msec <= 60 && !(cg.roundlimitWarnings & 2 ) )
        {
            cg.roundlimitWarnings |= 1 | 2 ;
            // one minute left
            trap_S_StartLocalSound( trap_S_RegisterSound( "sound/commentary/oneminute.wav", qfalse ) , CHAN_ANNOUNCER );
        }
        else if ( msec <= 120 && !(cg.roundlimitWarnings & 1 ) )
        {
            cg.roundlimitWarnings |= 1 ;
            trap_S_StartLocalSound( trap_S_RegisterSound( "sound/commentary/twominutes.wav", qfalse ) , CHAN_ANNOUNCER );
            // two minutes left
        }

    }

    // pointlimit warning
    if ( cgs.teampointlimit > 0 && cgs.gametype >= GT_TEAM) {

        // only caculate for the team W/ higher points
        if ( cgs.scores1 > cgs.scores2 )
            highScore = cgs.scores1;
        else
            highScore = cgs.scores2;

        if ( !( cg.fraglimitWarnings & 4 ) && highScore == cgs.teampointlimit ) {
            cg.fraglimitWarnings |= 1 | 2 | 4;
            CG_AddBufferedSound( trap_S_RegisterSound("sound/commentary/pt_limithit.wav",qfalse) );
        }
        else if ( !( cg.fraglimitWarnings & 2 ) && highScore == (cgs.teampointlimit - 1) ) {
            cg.fraglimitWarnings |= 1 | 2 ;
            CG_AddBufferedSound( trap_S_RegisterSound("sound/commentary/one_ptleft.wav",qfalse) );
        }
        else if ( cgs.fraglimit > 2 && !( cg.fraglimitWarnings & 1 ) && highScore == (cgs.teampointlimit - 2) ) {
            cg.fraglimitWarnings |= 1 ;
            CG_AddBufferedSound( trap_S_RegisterSound("sound/commentary/two_ptleft.wav",qfalse) );
        }
    }

    // fraglimit warnings
    /*
    else if ( cgs.fraglimit > 0 && cgs.gametype < GT_TEAM) {
    highScore = cgs.scores1;
    if ( !( cg.fraglimitWarnings & 4 ) && highScore == (cgs.fraglimit - 1) ) {
    cg.fraglimitWarnings |= 1 | 2 | 4;
    CG_AddBufferedSound(cgs.media.oneFragSound);
    }
    else if ( cgs.fraglimit > 2 && !( cg.fraglimitWarnings & 2 ) && highScore == (cgs.fraglimit - 2) ) {
    cg.fraglimitWarnings |= 1 | 2;
    CG_AddBufferedSound(cgs.media.twoFragSound);
    }
    else if ( cgs.fraglimit > 3 && !( cg.fraglimitWarnings & 1 ) && highScore == (cgs.fraglimit - 3) ) {
    cg.fraglimitWarnings |= 1;
    CG_AddBufferedSound(cgs.media.threeFragSound);
    }
    }
    DISABLED */
}

int CG_ButtonPushed ( int button );
extern	vmCvar_t	cg_newbeeHeight;

/*
===============
CG_TransitionPlayerState

===============
*/
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops ) {
    // check for changing follow mode
    if ( ps->clientNum != ops->clientNum ) {
        cg.thisFrameTeleport = qtrue;
        cg.predictedPlayerEntity.pe.hand_weapon.animation = NULL;
        // clear all these flashbang related values
        cg.flashedVision_x = 0;
        cg.flashedVision_y = 0;
        cg.flashedVisionTime = 0;
        cg.FlashTime = 0;
        cg.ConcussionTime = 0;
        cg.damageTime = 0;
        cg.DeafTime = 0;
        // make sure we don't get any unwanted transition effects
        *ops = *ps;
    }

    // damage events (player is getting wounded)
    if ( ps->damageEvent != ops->damageEvent && ps->damageCount ) {
        CG_DamageFeedback( ps->damageYaw, ps->damagePitch, ps->damageCount, 500 );
    }

    if ( ps->pm_flags & PMF_FOLLOW && !(ops->pm_flags & PMF_FOLLOW) )
    {
        cg.predictedPlayerEntity.pe.hand_weapon.animation = NULL;
    }
    // respawning
    if ( ps->persistant[PERS_SPAWN_COUNT] != ops->persistant[PERS_SPAWN_COUNT] ) {
        CG_Respawn();
    }

    if( ps->stats[STAT_HEALTH] <= 0 && ops->stats[STAT_HEALTH] > 0 )
    {
        // set death time
        cg.deathTime = cg.time;
        // the game is over for us, allow to change xp
        trap_Cvar_Set("ui_gamestate", va("%i", STATE_OVER));
    }

    if ( cg.mapRestart ) {
        CG_Respawn();
        cg.mapRestart = qfalse;
    }

    if ( cg.snap->ps.pm_type != PM_INTERMISSION
            && ps->persistant[PERS_TEAM] != TEAM_SPECTATOR
            && !(ps->pm_flags & PMF_FOLLOW) ) {
        CG_CheckLocalSounds( ps, ops );
    }

    if ( ps->stats[STAT_CHEST_DAMAGE] != ops->stats[STAT_CHEST_DAMAGE] )
        cg.flashDmgLocTime[0] = cg.time + 1000;
    if ( ps->stats[STAT_STOMACH_DAMAGE] != ops->stats[STAT_STOMACH_DAMAGE] )
        cg.flashDmgLocTime[1] = cg.time + 1000;
    if ( ps->stats[STAT_ARM_DAMAGE] != ops->stats[STAT_ARM_DAMAGE] )
        cg.flashDmgLocTime[2] = cg.time + 1000;
    if ( ps->stats[STAT_LEG_DAMAGE] != ops->stats[STAT_LEG_DAMAGE] )
        cg.flashDmgLocTime[3] = cg.time + 1000;
    if ( ps->stats[STAT_HEAD_DAMAGE] != ops->stats[STAT_HEAD_DAMAGE] )
        cg.flashDmgLocTime[4] = cg.time + 1000;

    if ( ps->pm_type == PM_NORMAL )
    {
        if ( BG_GotWeapon( WP_C4, ps->stats ) && !BG_GotWeapon( WP_C4, ops->stats ) && !cg.ns_newbiehelp.w_gotBomb )
        {
            CG_NewbieMessage(S_COLOR_GREEN "You've got the c4-bomb.\nSelect it from your inventory and go to the defuse point and arm it.", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
            cg.ns_newbiehelp.w_gotBomb = qtrue;
        }
        if ( ps->pm_flags & PMF_BOMBRANGE && !(ops->pm_flags & PMF_BOMBRANGE) && !cg.ns_newbiehelp.w_bombRange )
        {
            cg.ns_newbiehelp.w_bombRange = qtrue;
            CG_NewbieMessage(S_COLOR_GREEN"You're in an bomb-field.\nThe bomb has to be placed here.", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
        }

        if ( ps->eFlags & EF_VIP && !(ops->eFlags & EF_VIP) && !cg.ns_newbiehelp.w_vipTime && cgs.mi_vipTime )
        {
            cg.ns_newbiehelp.w_vipTime = qtrue;
            CG_NewbieMessage(S_COLOR_GREEN"You are the V.I.P.\nHide yourself until the round is over.", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
        }
        else if ( ps->eFlags & EF_VIP && !(ops->eFlags & EF_VIP) && !cg.ns_newbiehelp.w_vipRescue && cgs.mi_vipRescue )
        {
            cg.ns_newbiehelp.w_vipRescue = qtrue;
            CG_NewbieMessage(S_COLOR_GREEN"You are the V.I.P.\nFollow your team to the extraction point.", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
        }

        if ( ps->stats[STAT_ROUNDS] >  ops->stats[STAT_ROUNDS] && ( ps->weaponstate == WEAPON_RELOADING || ps->weaponstate == WEAPON_RELOADING_EMPTY ) && BG_IsRifle( ps->weapon ) && BG_IsRifle( ops->weapon ) && ps->weapon == ops->weapon && !cg.ns_newbiehelp.w_reloadRifle )
        {
            cg.ns_newbiehelp.w_reloadRifle = qtrue;
            CG_NewbieMessage(S_COLOR_GREEN"You can change the firemode of this weapon.\nPush the WEAPONMODE1 key to switch through the avaiable Firemodes.", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
        }
        else if ( ps->stats[STAT_ROUNDS] > ops->stats[STAT_ROUNDS] && ps->weapon != WP_SPAS15 && BG_IsShotgun( ps->weapon ) && ps->weapon == ops->weapon && !cg.ns_newbiehelp.w_reloadShotgun )
        {
            cg.ns_newbiehelp.w_reloadShotgun = qtrue;
            CG_NewbieMessage(S_COLOR_GREEN "You can break the reloading sequence by holding the ATTACK key.", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
        }

        if ( ps->stats[STAT_ROUNDS] == 0 && ops->stats[STAT_ROUNDS] == 0 && CG_ButtonPushed( BUTTON_ATTACK ) && !cg.ns_newbiehelp.w_weaponEmpty
                && !BG_IsGrenade( cg.snap->ps.weapon )
                && !BG_IsMelee( cg.snap->ps.weapon ) )
        {
            cg.ns_newbiehelp.w_weaponEmpty = qtrue;
            CG_NewbieMessage(S_COLOR_GREEN "This weapon is out of rounds.\nTry to reload it using your RELOAD key.", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
        }

        if ( ps->powerups[PW_BRIEFCASE] && ops->powerups[PW_BRIEFCASE] == 0 && !(ps->eFlags & EF_VIP) && !cg.ns_newbiehelp.w_gotBriefcase )
        {
            cg.ns_newbiehelp.w_gotBriefcase = qtrue;
            CG_NewbieMessage(S_COLOR_GREEN "You've got the briefcase.\nBring it to a briefcase capture point.", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );

        }

        if ( ps->stats[STAT_HEALTH] > 0 && ps->stats[STAT_HEALTH] < ops->stats[STAT_HEALTH]-20 && !cg.ns_newbiehelp.w_firstHit )
        {
            cg.ns_newbiehelp.w_firstHit = qtrue;
            CG_NewbieMessage(S_COLOR_GREEN "You've been badly injured.\nUse the BANDAGE key to bandage yourself.", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
        }
        if ( cg.xpPoints > 15 && !cg.ns_newbiehelp.w_gotlotsXp )
        {
            cg.ns_newbiehelp.w_gotlotsXp = qtrue;
            CG_NewbieMessage( va( S_COLOR_GREEN "You've got %i XP.\nIt's time to open the playermenu (ESC->PLAYER) to spend them on your abilities", char_xp.integer ) , SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
        }
        if ( ps->pm_flags & PMF_ASSAULTRANGE && !(ops->pm_flags & PMF_ASSAULTRANGE) && !cg.ns_newbiehelp.w_assaultRange )
        {
            cg.ns_newbiehelp.w_assaultRange = qtrue;
            CG_NewbieMessage(S_COLOR_GREEN "You're in an assault field.\nTry to tap it by staying inside it until it's timer runs out.", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
        }
        if ( ps->pm_flags & PMF_BLOCKED && !(ops->pm_flags & PMF_BLOCKED) && !cg.ns_newbiehelp.w_assaultBlocked )
        {
            cg.ns_newbiehelp.w_assaultBlocked = qtrue;
            CG_NewbieMessage(S_COLOR_GREEN "This assault field has been blocked, because an enemy player is inside of it.", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
        }

    }
#if 0
    // check for going low on ammo
    CG_CheckAmmo();
#endif

    // run events
    CG_CheckPlayerstateEvents( ps, ops );

    // smooth the ducking viewheight change
    if ( ps->viewheight != ops->viewheight ) {
        cg.duckChange = ps->viewheight - ops->viewheight;
        cg.duckTime = cg.time;
    }

}

