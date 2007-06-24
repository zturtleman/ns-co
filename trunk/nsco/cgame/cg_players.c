// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_players.c -- handle the media and animation for player entities

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"

void CG_AddPlayerWeapon( refEntity_t *leftArm, refEntity_t *rightArm, playerState_t *ps, centity_t *cent );

char	*cg_customSoundNames[MAX_CUSTOM_SOUNDS] = {
            "*death1.wav",
            "*death2.wav",
            "*death3.wav",
            "*jump1.wav",
            "*pain25_1.wav",
            "*pain50_1.wav",
            "*pain75_1.wav",
            "*pain100_1.wav",
            "*falling1.wav",
            "*gasp.wav",
            "*drown.wav",
            "*fall1.wav",
            "*taunt.wav"
        };

/*
================================
CG_IsPlayerInAnim( anim )

returns true if currentplayer is in anim
================================
*/
static qboolean CG_IsPlayerInAnim( centity_t *cent, int anim, qboolean legs ) {
    int curanim;

    if ( !cent )
        return qfalse;

    if ( legs )
        curanim = cent->currentState.legsAnim;
    else
        curanim = cent->currentState.torsoAnim;

    if ( curanim == anim )
        return qtrue;

    return qfalse;
} 
// Navy Seals --

/*
================
CG_CustomSound

================
*/
sfxHandle_t	CG_CustomSound( int clientNum, const char *soundName ) {
    //	clientInfo_t *ci;
    //	int			i;

    if ( soundName[0] != '*' ) {
        return trap_S_RegisterSound( soundName, qfalse );
    }
    /*
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
    clientNum = 0;
    }
    ci = &cgs.clientinfo[ clientNum ];

    for ( i = 0 ; i < MAX_CUSTOM_SOUNDS && cg_customSoundNames[i] ; i++ ) {
    if ( !strcmp( soundName, cg_customSoundNames[i] ) ) {
    return ci->sounds[i];
    }
    }
    */

    CG_Error( "Unknown custom sound: %s", soundName );
    return 0;
}


/*
=============================================================================

CLIENT INFO

=============================================================================
*/


/*
======================
CG_ParseAnimationFile

Read a configuration file containing animation coutns and rates
models/players/visor/animation.cfg, etc
======================
*/
static  qboolean	CG_ParseAnimationFile( const char *filename, clientInfo_t *ci ) {
    char		*text_p, *prev;
    int			len;
    int			i;
    char		*token;
    float		fps;
    int			skip;
    char		text[20000];
    fileHandle_t	f;
    animation_t *animations;

    animations = ci->animations;

    // load the file
    len = trap_FS_FOpenFile( filename, &f, FS_READ );
    if ( len <= 0 ) {
        return qfalse;
    }
    if ( len >= sizeof( text ) - 1 ) {
        CG_Printf( "File %s too long\n", filename );
        return qfalse;
    }
    trap_FS_Read( text, len, f );
    text[len] = 0;
    trap_FS_FCloseFile( f );

    // parse the text
    text_p = text;
    skip = 0;	// quite the compiler warning

    ci->footsteps = FOOTSTEP_NORMAL;
    VectorClear( ci->headOffset );
    ci->gender = GENDER_MALE;

    // read optional parameters
    while ( 1 ) {
        prev = text_p;	// so we can unget
        token = COM_Parse( &text_p );
        if ( !token ) {
            break;
        }
        if ( !Q_stricmp( token, "step_walk-l" ) ) {
            token = COM_Parse( &text_p );
            if ( !token ) {
                break;
            }
            ci->footprintFrameTimer.step_walkl = atoi(token);
            continue;
        } else if ( !Q_stricmp( token, "step_walk-r" ) ) {
            token = COM_Parse( &text_p );
            if ( !token ) {
                break;
            }
            ci->footprintFrameTimer.step_walkr = atoi(token);
            continue;
        } else if ( !Q_stricmp( token, "step_run-l" ) ) {
            token = COM_Parse( &text_p );
            if ( !token ) {
                break;
            }
            ci->footprintFrameTimer.step_runl = atoi(token);
            continue;
        } else if ( !Q_stricmp( token, "step_run-r" ) ) {
            token = COM_Parse( &text_p );
            if ( !token ) {
                break;
            }
            ci->footprintFrameTimer.step_runr = atoi(token);
            continue;
        } else if ( !Q_stricmp( token, "step_limp-l" ) ) {
            token = COM_Parse( &text_p );
            if ( !token ) {
                break;
            }
            ci->footprintFrameTimer.step_limpl = atoi(token);
            continue;
        } else if ( !Q_stricmp( token, "step_limp-r" ) ) {
            token = COM_Parse( &text_p );
            if ( !token ) {
                break;
            }
            ci->footprintFrameTimer.step_limpr = atoi(token);
            continue;
        } else if ( !Q_stricmp( token, "step_back-l" ) ) {
            token = COM_Parse( &text_p );
            if ( !token ) {
                break;
            }
            ci->footprintFrameTimer.step_backl = atoi(token);
            continue;
        } else if ( !Q_stricmp( token, "step_back-r" ) ) {
            token = COM_Parse( &text_p );
            if ( !token ) {
                break;
            }
            ci->footprintFrameTimer.step_backr = atoi(token);
            continue;
        }else if ( !Q_stricmp( token, "headoffset" ) ) {
            for ( i = 0 ; i < 3 ; i++ ) {
                token = COM_Parse( &text_p );
                if ( !token ) {
                    break;
                }
                ci->headOffset[i] = atof( token );
            }
            continue;
        } else if ( !Q_stricmp( token, "sex" ) ) {
            token = COM_Parse( &text_p );
            if ( !token ) {
                break;
            }
            if ( token[0] == 'f' || token[0] == 'F' ) {
                ci->gender = GENDER_FEMALE;
            } else if ( token[0] == 'n' || token[0] == 'N' ) {
                ci->gender = GENDER_NEUTER;
            } else {
                ci->gender = GENDER_MALE;
            }
            continue;
        }

        // if it is a number, start parsing animations
        if ( token[0] >= '0' && token[0] <= '9' ) {
            text_p = prev;	// unget the token
            break;
        }
        Com_Printf( "unknown token '%s' is %s\n", token, filename );
    }

    // read information for each frame
    for ( i = 0 ; i < MAX_ANIMATIONS ; i++ ) {

        token = COM_Parse( &text_p );
        if ( !token ) {
            break;
        }
        animations[i].firstFrame = atoi( token );

        // leg only frames are adjusted to not count the upper body only frames
        if ( i == LEGS_IDLECR ) {
            skip = animations[LEGS_IDLECR].firstFrame - animations[TORSO_GESTURE1].firstFrame;
        }
        if ( i >= LEGS_IDLECR ) {
            animations[i].firstFrame -= skip;
        }

        token = COM_Parse( &text_p );
        if ( !token ) {
            break;
        }
        animations[i].numFrames = atoi( token );

        token = COM_Parse( &text_p );
        if ( !token ) {
            break;
        }
        animations[i].loopFrames = atoi( token );

        token = COM_Parse( &text_p );
        if ( !token ) {
            break;
        }
        fps = atof( token );
        if ( fps == 0 ) {
            fps = 1;
        }
        animations[i].frameLerp = 1000 / fps;
        animations[i].initialLerp = 1000 / fps;
    }

    if ( i != MAX_ANIMATIONS ) {
        CG_Printf( "Error parsing animation file: %s", filename );
        return qfalse;
    }

    /*
    animations[TORSO_CLIMB_IDLE].firstFrame = animations[TORSO_CLIMB].firstFrame+1;
    animations[TORSO_CLIMB_IDLE].numFrames = 1;
    animations[TORSO_CLIMB_IDLE].loopFrames = 1;
    animations[TORSO_CLIMB_IDLE].frameLerp = animations[TORSO_CLIMB].frameLerp;
    animations[TORSO_CLIMB_IDLE].initialLerp = animations[TORSO_CLIMB].initialLerp;
    */
    //	memcpy(&animations[TORSO_RAISE_RIFLE], &animations[TORSO_DROP_RIFLE], sizeof(animation_t));
    //	animations[TORSO_RAISE_RIFLE].reversed = qtrue;

    // limb backwards animation
    memcpy(&animations[LEGS_BACKLIMB], &animations[LEGS_LIMP], sizeof(animation_t));
    animations[LEGS_BACKLIMB].reversed = qtrue;


    // crouch backwards animation
    memcpy(&animations[LEGS_BACKCR], &animations[LEGS_WALKCR], sizeof(animation_t));
    animations[LEGS_BACKCR].reversed = qtrue;

    animations[TORSO_ATTACK_MELEE].frameLerp = 1000 / 50;
    animations[TORSO_ATTACK_MELEE].initialLerp = 1000 / 25;

    animations[LEGS_IDLE_RIFLE_SCOPED].firstFrame = animations[LEGS_IDLE].firstFrame+1;
    animations[LEGS_IDLE_RIFLE_SCOPED].numFrames = 1;
    animations[LEGS_IDLE_RIFLE_SCOPED].loopFrames = 1;
    animations[LEGS_IDLE_RIFLE_SCOPED].frameLerp = animations[ LEGS_IDLE ].frameLerp;
    animations[LEGS_IDLE_RIFLE_SCOPED].initialLerp = animations[ LEGS_IDLE ].initialLerp;

    animations[LEGS_IDLECR_RIFLE_SCOPED].firstFrame = animations[ LEGS_IDLECR ].firstFrame+1;
    animations[LEGS_IDLECR_RIFLE_SCOPED].numFrames = 1;
    animations[LEGS_IDLECR_RIFLE_SCOPED].loopFrames = 1;
    animations[LEGS_IDLECR_RIFLE_SCOPED].frameLerp = animations[ LEGS_IDLECR ].frameLerp;
    animations[LEGS_IDLECR_RIFLE_SCOPED].initialLerp = animations[ LEGS_IDLECR ].initialLerp;



    //
    // new anims changes
    //
    //	animations[TORSO_GETFLAG].flipflop = qtrue;
    //	animations[TORSO_GUARDBASE].flipflop = qtrue;
    //	animations[TORSO_PATROL].flipflop = qtrue;
    //	animations[TORSO_AFFIRMATIVE].flipflop = qtrue;
    //	animations[TORSO_NEGATIVE].flipflop = qtrue;
    //
    return qtrue;
}



/*
==========================
CG_RegisterClientSkin
==========================
*/
static qboolean	CG_RegisterClientSkin( clientInfo_t *ci, const char *modelName, const char *headName ) {
    char		filename[MAX_QPATH];
    int camoType = cgs.camoType;

    // do not load camoSkins when player is a VIP
    if ( !Q_stricmp( modelName, "vip_male" ) )
        Com_sprintf( filename, sizeof( filename ), "models/players/%s/legs_%s.skin", modelName, cgs.vipType  );
    else
        Com_sprintf( filename, sizeof( filename ), "models/players/%s/legs%s.skin", modelName, CG_GetCamoStringForType( camoType )  );
    ci->legsSkin = trap_R_RegisterSkin( filename );

    if ( !ci->legsSkin )
        if ( cg_buildScript.integer )
            CG_Printf("Failed to load %s\n", filename);

    if ( !Q_stricmp( modelName, "vip_male" ) )
        Com_sprintf( filename, sizeof( filename ), "models/players/%s/torso_%s.skin", modelName, cgs.vipType  );
    else
        Com_sprintf( filename, sizeof( filename ), "models/players/%s/torso%s.skin", modelName, CG_GetCamoStringForType( camoType )  );
    ci->torsoSkin = trap_R_RegisterSkin( filename );

    if ( !ci->torsoSkin )
        if ( cg_buildScript.integer )
            CG_Printf("Failed to load %s\n", filename);

    if ( !Q_stricmp( modelName, "vip_male" ) )
        Com_sprintf( filename, sizeof( filename ), "models/players/%s/torso_%s_vest.skin", modelName, cgs.vipType  );
    else
        Com_sprintf( filename, sizeof( filename ), "models/players/%s/torso%s_vest.skin", modelName, CG_GetCamoStringForType( camoType )  );
    ci->torsoVestSkin = trap_R_RegisterSkin( filename );

    if ( !ci->torsoVestSkin )
        if ( cg_buildScript.integer )
            CG_Printf("Failed to load %s\n", filename);

    if ( !Q_stricmp( modelName, "vip_male" ) )
    {
        if (strlen(cgs.vipType) <= 0)
            Com_sprintf( filename, sizeof( filename ), "models/players/%s/head_%s.skin", modelName, headName  );
        else
            Com_sprintf( filename, sizeof( filename ), "models/players/heads/head_vip_%s.skin", cgs.vipType  );
    }
    else
        Com_sprintf( filename, sizeof( filename ), "models/players/heads/head_%s.skin", headName );
    ci->headSkin = trap_R_RegisterSkin( filename );

    if ( !ci->headSkin )
        if ( cg_buildScript.integer )
            CG_Printf("Failed to load %s\n", filename);


    if ( !ci->legsSkin || !ci->torsoSkin || !ci->headSkin || !ci->torsoVestSkin ) {
        return qfalse;
    }

    return qtrue;
}

/*
==========================
88
==========================
*/
static qboolean CG_RegisterClientModelname( clientInfo_t *ci, const char *modelName, const char *headName ) {
    char		filename[MAX_QPATH];

    // load the animations
    Com_sprintf( filename, sizeof( filename ), "models/players/%s/animation.cfg", modelName );
    if ( !CG_ParseAnimationFile( filename, ci ) ) {
        if ( cg_buildScript.integer )
            Com_Printf( "Failed to load animation file %s\n", filename );
        return qfalse;
    }

    // load cmodels before models so filecache works

    Com_sprintf( filename, sizeof( filename ), "models/players/%s/legs.md3", modelName );
    ci->legsModel = trap_R_RegisterPermanentModel( filename );
    if ( !ci->legsModel ) {
        if ( cg_buildScript.integer )
            Com_Printf( "Failed to load model file %s (wrong lower.md3)\n", filename );
        return qfalse;
    }

    Com_sprintf( filename, sizeof( filename ), "models/players/%s/torso.md3", modelName );
    ci->torsoModel = trap_R_RegisterPermanentModel( filename );
    if ( !ci->torsoModel ) {
        if ( cg_buildScript.integer )
            Com_Printf( "Failed to load model file %s (wrong torso.md3)\n", filename );
        return qfalse;
    }

    Com_sprintf( filename, sizeof( filename ), "models/players/%s/arml.md3", modelName );
    ci->leftArmModel = trap_R_RegisterPermanentModel( filename );
    if ( !ci->leftArmModel ) {
        if ( cg_buildScript.integer )
            Com_Printf( "Failed to load model file %s (wrong arml.md3)\n", filename );
        return qfalse;
    }
    Com_sprintf( filename, sizeof( filename ), "models/players/%s/armr.md3", modelName );
    ci->rightArmModel = trap_R_RegisterPermanentModel( filename );
    if ( !ci->rightArmModel ) {
        if ( cg_buildScript.integer )
            Com_Printf( "Failed to load model file %s (wrong armr.md3)\n", filename );
        return qfalse;
    }

    if ( !Q_stricmp( headName , "vip_seal" ) || !Q_stricmp( headName , "vip_tango" ) )
        Com_sprintf( filename, sizeof( filename ), "models/players/heads/head_vip_%s.md3", cgs.vipType );
    else
        Com_sprintf( filename, sizeof( filename ), "models/players/heads/head.md3", headName );

    ci->headModel = trap_R_RegisterPermanentModel( filename );

#if 0
    // i had this here to get the bounding boxes of our headmodel
    // for the serverside new hitdetection system
    {
        vec3_t mins,maxs;

        trap_R_ModelBounds( ci->headModel, mins, maxs );

        CG_Printf("headmodel bbox: %s %s\n", vtos( mins ), vtos ( maxs ) );
    }
#endif

    if ( !ci->headModel ) {
        if ( cg_buildScript.integer )
            Com_Printf( "Failed to load model file %s (wrong head.md3)\n", filename );
        return qfalse;
    }


    // if any skins failed to load, return failure
    if ( !CG_RegisterClientSkin( ci, modelName, headName ) ) {
        if ( cg_buildScript.integer )
            Com_Printf( "Failed to load skin file: %s : %s\n", modelName, headName );
        return qfalse;
    }

    // Com_sprintf( filename, sizeof( filename ), "models/players/%s/preview.tga", modelName );
    /*	ci->modelIcon = trap_R_RegisterShaderNoMip( filename );
    if ( !ci->modelIcon ) {
    Com_Printf( "Failed to load icon file: %s\n", filename );
    return qfalse;
    }*/

    return qtrue;
}

/*
==========================
88
==========================
*/
static qboolean CG_RegisterClientStyleModels( clientInfo_t *ci, const char *mouthName, const char *eyesName, const char *headName ) {
    char		filename[MAX_QPATH];

    // disabled.
    if ( cg_disableHeadstuff.integer )
        return qtrue;

    // load cmodels before models so filecache works
    Com_sprintf( filename, sizeof( filename ), "models/players/heads/accessoires/m_%s.md3", mouthName );
    ci->equipmentMouth = trap_R_RegisterPermanentModel( filename );

    Com_sprintf( filename, sizeof( filename ), "models/players/heads/accessoires/h_%s.md3", headName );
    ci->equipmentHead = trap_R_RegisterPermanentModel( filename );

    Com_sprintf( filename, sizeof( filename ), "models/players/heads/accessoires/e_%s.md3", eyesName );
    ci->equipmentEyes = trap_R_RegisterPermanentModel( filename );

    return qtrue;
}

/*
====================
CG_ColorFromString
====================
*/
static void CG_ColorFromString( const char *v, vec3_t color ) {
    int val;

    VectorClear( color );

    val = atoi( v );

    if ( val < 1 || val > 7 ) {
        VectorSet( color, 1, 1, 1 );
        return;
    }

    if ( val & 1 ) {
        color[2] = 1.0f;
    }
    if ( val & 2 ) {
        color[1] = 1.0f;
    }
    if ( val & 4 ) {
        color[0] = 1.0f;
    }
}

/*
===================
CG_LoadClientInfo

Load it now, taking the disk hits.
This will usually be deferred to a safe time
===================
*/
static void CG_LoadClientInfo( clientInfo_t *ci ) {
    //	const char	*dir, *fallback;
    //	int			i;
    //	const char	*s;
    //	int			clientNum;

    if ( !CG_RegisterClientModelname( ci, ci->modelName, ci->headName ) )
    {
        if ( cg_buildScript.integer ) {
            CG_Error( "CG_RegisterClientModelname( %s, %s ) failed", ci->modelName, ci->headName );
        }

        // fall back
        if ( cgs.gametype >= GT_TEAM )
        {
            char *modelName;
            char *headName;
            int oldcamo = cgs.camoType;


            if ( ci->team == TEAM_BLUE ) {
                modelName = "t_medium";
                headName = "jayant";
            }
            else
            {
                modelName = "s_medium";
                headName = "bruce";
            }

            // nothing worked... fall back
            if ( !CG_RegisterClientModelname( ci, modelName, headName ) ) {
                CG_Error( "DEFAULT_MODEL / skin (%s/%s) failed to register",
                          ci->modelName, ci->headName );
            }
            cgs.camoType = oldcamo;
        } else {
            char *modelName,*headName;

            // random default playermodels.

            if (random() < 0.5)
                modelName = "s_medium";
            else
                modelName = "t_medium";

            headName = "jayant";

            if ( !CG_RegisterClientModelname( ci, modelName, headName) ) {
                CG_Error( "DEFAULT_MODEL (%s/%s) failed to register", modelName, headName );
            }
        }
    }

    // player style
    /*	if ( !CG_RegisterClientStyleModels( ci, ci->equipmentMouthName,ci->equipmentEyesName,ci->equipmentHeadName ) )
    {
    if ( cg_buildScript.integer ) {
    CG_Error( "CG_RegisterClientStyleModels( %s %s/%s/%s ) failed", ci->name,ci->equipmentMouthName, ci->equipmentEyesName, ci->equipmentHeadName );
    }
    }*/

    // sounds
    //	dir = ci->modelName;
    //	fallback = DEFAULT_MODEL;

    /*
    for ( i = 0 ; i < MAX_CUSTOM_SOUNDS ; i++ ) {
    s = cg_customSoundNames[i];
    if ( !s ) {
    break;
    }
    ci->sounds[i] = trap_S_RegisterSound( va("sound/player/%s/%s", dir, s + 1),qfalse );
    if ( !ci->sounds[i] ) { // loadup defaults!
    ci->sounds[i] = trap_S_RegisterSound( va("sound/player/sarge/%s", s + 1),qfalse );
    }
    }
    */

    ci->deferred = qfalse;

    //	CG_Printf("%i equipment (loaded clientinfo)\n", ci->playerEquipment );

    // reset any existing players and bodies, because they might be in bad
    // frames for this new model
    /*	clientNum = ci - cgs.clientinfo;
    for ( i = 0 ; i < MAX_GENTITIES ; i++ ) {
    if ( cg_entities[i].currentState.clientNum == clientNum
    && cg_entities[i].currentState.eType == ET_PLAYER ) {
    CG_ResetPlayerEntity( &cg_entities[i] );
    }
    }*/
}

/*
======================
CG_CopyClientInfoModel
======================
*/
static void CG_CopyClientInfoModel( clientInfo_t *from, clientInfo_t *to ) {
    VectorCopy( from->headOffset, to->headOffset );
    to->footsteps = from->footsteps;
    to->gender = from->gender;

    to->legsModel = from->legsModel;
    to->legsSkin = from->legsSkin;
    to->torsoModel = from->torsoModel;
    to->torsoSkin = from->torsoSkin;
    to->headModel = from->headModel;
    to->headSkin = from->headSkin;
    //	to->modelIcon = from->modelIcon;
    to->torsoVestSkin = from->torsoVestSkin;

    to->leftArmModel = from->leftArmModel;
    to->rightArmModel = from->rightArmModel;

    to->footprintFrameTimer = from->footprintFrameTimer;
    //	memcpy( &to->footprintFrameTimer, &from->footprintFrameTimer , sizeof ( to->footprintFrameTimer ) );

    to->equipmentEyes = from->equipmentEyes;
    to->equipmentHead = from->equipmentHead;
    to->equipmentMouth = from->equipmentMouth;
    //	to->helmetModel = from->helmetModel;
    //	to->helmetSkin = from->helmetSkin;
    //	to->Equipment = from->Equipment;
    //	CG_Printf("%i equipment (copied clientinfo)\n", to->Equipment);


    to->newAnims = from->newAnims;

    memcpy( to->animations, from->animations, sizeof( to->animations ) );
    //	memcpy( to->sounds, from->sounds, sizeof( to->sounds ) );
}

/*
======================
CG_CopyModelInfo
======================
*/
static void CG_CopyModelInfo( clientInfo_t *from, clientInfo_t *to ) {
    VectorCopy( from->headOffset, to->headOffset );
    to->footsteps = from->footsteps;
    to->gender = from->gender;

    to->legsModel = from->legsModel;
    to->legsSkin = from->legsSkin;
    to->torsoModel = from->torsoModel;
    to->torsoSkin = from->torsoSkin;
    to->headModel = from->headModel;
    to->headSkin = from->headSkin;

    to->footprintFrameTimer = from->footprintFrameTimer;

    strcpy( to->modelName , from->modelName );

    to->torsoVestSkin = from->torsoVestSkin;

    to->leftArmModel = from->leftArmModel;
    to->rightArmModel = from->rightArmModel;

    to->newAnims = from->newAnims;

    memcpy( to->animations, from->animations, sizeof( to->animations ) );
}
/*
======================
CG_ScanForExistingClientInfo
======================
*/
static qboolean CG_ScanForExistingClientInfo( clientInfo_t *ci ) {
    int		i;
    clientInfo_t	*match;
    int clientNum = -1;

    clientNum = ci - cgs.clientinfo;

    if ( !Q_stricmp( ci->modelName, "vip_male" ) )
    {
        for ( i = 0; i < MAX_PLAYER_MODELS; i++ )
        {
            match = cgs.media.MiscPlayerModels + i;

            if ( !match->infoValid )
            {
                CG_Error("Could not load Model: %s\n", ci->modelName );
                continue;
            }
            if ( match->deferred )
                continue;

            CG_CopyModelInfo( match, ci );

            if ( !CG_RegisterClientSkin( ci, ci->modelName, ci->headName ) )
                if ( !CG_RegisterClientSkin( ci, ci->modelName, "vip_male" ) )
                {
                    CG_Error("Couldn't register default headskin %s for model %s", "vip_male", ci->modelName );
                    return qfalse;
                }


            CG_RegisterClientStyleModels( ci, ci->equipmentMouthName, ci->equipmentEyesName, ci->equipmentHeadName );

            return qtrue;
        }
    }

    if ( ci->team == TEAM_RED )
    {
seal:
        // see if we're in the memory already
        for ( i = 0; i < MAX_PLAYER_MODELS; i++ )
        {
            match = cgs.media.SealPlayerModels + i;

            if ( !match->infoValid )
                continue;
            if ( match->deferred )
                continue;

            CG_CopyModelInfo( match, ci );

            if ( !CG_RegisterClientSkin( ci, ci->modelName, ci->headName ) )
                if ( !CG_RegisterClientSkin( ci, ci->modelName, "bruce" ) )
                {
                    CG_Error("Couldn't register default headskin %s for model %s", "bruce", ci->modelName );
                    return qfalse;
                }

            CG_RegisterClientStyleModels( ci, ci->equipmentMouthName, ci->equipmentEyesName, ci->equipmentHeadName );

            return qtrue;
        }
    }
    else if ( ci->team == TEAM_BLUE )
    {
tango:
        for ( i = 0; i < MAX_PLAYER_MODELS; i++ )
        {
            match = cgs.media.TangoPlayerModels + i;

            if ( !match->infoValid )
                continue;
            if ( match->deferred )
                continue;

            CG_CopyModelInfo( match, ci );

            if ( !CG_RegisterClientSkin( ci, ci->modelName, ci->headName ) )
                if ( !CG_RegisterClientSkin( ci, ci->modelName, "jayant" ) )
                {
                    CG_Error("Couldn't register default headskin %s for model %s", "jayant", ci->modelName );
                    return qfalse;
                }


            CG_RegisterClientStyleModels( ci, ci->equipmentMouthName, ci->equipmentEyesName, ci->equipmentHeadName );

            return qtrue;
        }
    }
    else
    {
        qboolean tango;

        if ( !Q_stricmp( ci->modelName , "t_medium") )
            tango = qtrue;
        else
            tango = qfalse;

        if ( tango )
            goto tango;
        else
            goto seal;
    }

    // nothing matches, so defer the load
    return qfalse;
}

/*
======================
CG_SetDeferredClientInfo

We aren't going to load it now, so grab some other
client's info to use until we have some spare time.
======================
*/
static void CG_SetDeferredClientInfo( clientInfo_t *ci ) {
    int		i;
    clientInfo_t	*match;



    // if someone else is already the same models and skins we
    // can just load the client info
    for ( i = 0 ; i < cgs.maxclients ; i++ ) {
        match = &cgs.clientinfo[ i ];
        if ( !match->infoValid || match->deferred ) {
            continue;
        }
        if ( Q_stricmp( ci->skinName, match->skinName ) ||
                Q_stricmp( ci->modelName, match->modelName ) ||
                Q_stricmp( ci->headModelName, match->headModelName ) ||
                Q_stricmp( ci->headSkinName, match->headSkinName ) ||
                Q_stricmp( ci->equipmentMouthName, match->equipmentMouthName ) ||
                Q_stricmp( ci->equipmentEyesName, match->equipmentEyesName ) ||
                Q_stricmp( ci->equipmentHeadName, match->equipmentHeadName )
           ) {
            continue;
        }
        // just load the real info cause it uses the same models and skins
        CG_LoadClientInfo( ci );
        return;
    }

    // if we are in teamplay, only grab a model if the skin is correct
    if ( cgs.gametype >= GT_TEAM ) {
        for ( i = 0 ; i < cgs.maxclients ; i++ ) {
            match = &cgs.clientinfo[ i ];
            if ( !match->infoValid || match->deferred ) {
                continue;
            }
            if ( Q_stricmp( ci->skinName, match->skinName ) ) {
                continue;
            }
            ci->deferred = qtrue;
            CG_CopyClientInfoModel( match, ci );
            return;
        }
        // load the full model, because we don't ever want to show
        // an improper team skin.  This will cause a hitch for the first
        // player, when the second enters.  Combat shouldn't be going on
        // yet, so it shouldn't matter
        CG_LoadClientInfo( ci );
        return;
    }

    // find the first valid clientinfo and grab its stuff
    for ( i = 0 ; i < cgs.maxclients ; i++ ) {
        match = &cgs.clientinfo[ i ];
        if ( !match->infoValid ) {
            continue;
        }

        ci->deferred = qtrue;
        CG_CopyClientInfoModel( match, ci );
        return;
    }

    // we should never get here...
    CG_Printf( "CG_SetDeferredClientInfo: no valid clients!\n" );

    CG_LoadClientInfo( ci );
}


/*
======================
CG_NewClientInfo
======================
*/
void CG_NewClientInfo( int clientNum ) {
    clientInfo_t *ci;
    clientInfo_t newInfo;
    const char	*configstring;
    const char	*v;
    char		*slash;

    ci = &cgs.clientinfo[clientNum];

    configstring = CG_ConfigString( clientNum + CS_PLAYERS );
    if ( !configstring[0] ) {
        memset( ci, 0, sizeof( *ci ) );
        return;		// player just left
    }

    // build into a temp buffer so the defer checks can use
    // the old value
    memset( &newInfo, 0, sizeof( newInfo ) );

    // isolate the player's name
    v = Info_ValueForKey(configstring, "n");
    Q_strncpyz( newInfo.name, v, sizeof( newInfo.name ) );

    // colors
    v = Info_ValueForKey( configstring, "c1" );
    CG_ColorFromString( v, newInfo.color );

    // bot skill
    v = Info_ValueForKey( configstring, "skill" );
    newInfo.botSkill = atoi( v );

    // handicap
    v = Info_ValueForKey( configstring, "hc" );
    newInfo.handicap = atoi( v );

    // wins
    v = Info_ValueForKey( configstring, "w" );
    newInfo.wins = atoi( v );

    // losses
    v = Info_ValueForKey( configstring, "l" );
    newInfo.losses = atoi( v );

    // team
    v = Info_ValueForKey( configstring, "t" );
    newInfo.team = atoi( v );

    // team task
    v = Info_ValueForKey( configstring, "tt" );
    newInfo.teamTask = atoi(v);

    // team leader
    v = Info_ValueForKey( configstring, "tl" );
    newInfo.teamLeader = atoi(v);

    //	v = Info_ValueForKey( configstring, "eq" );
    //	newInfo.playerEquipment = atoi(v);

    //	CG_Printf("Equipment:%i (created userinfo)\n",newInfo.playerEquipment );

    v = Info_ValueForKey( configstring, "e_head" );
    Q_strncpyz(newInfo.equipmentHeadName, v, sizeof( newInfo.equipmentHeadName ) );

    v = Info_ValueForKey( configstring, "e_eyes" );
    Q_strncpyz(newInfo.equipmentEyesName, v, sizeof( newInfo.equipmentEyesName ));

    v = Info_ValueForKey( configstring, "e_mouth" );
    Q_strncpyz(newInfo.equipmentMouthName, v, sizeof( newInfo.equipmentMouthName ));

    //	CG_Printf("head: %s mouth: %s eyes : %s\n",newInfo.equipmentHeadName,newInfo.equipmentMouthName,newInfo.equipmentEyesName );

    // model
    v = Info_ValueForKey( configstring, "model" );

    if ( cg_disableHeadstuff.integer )
    {
        Q_strncpyz(newInfo.equipmentEyesName, "", sizeof( newInfo.equipmentEyesName ));
        Q_strncpyz(newInfo.equipmentHeadName, "", sizeof( newInfo.equipmentHeadName ));
        Q_strncpyz(newInfo.equipmentMouthName, "", sizeof( newInfo.equipmentMouthName ));
    }

    /*
    if ( cg_forceModel.integer ) {
    // forcemodel makes everyone use a single model
    // to prevent load hitches
    char modelStr[MAX_QPATH];
    char *skin;


    trap_Cvar_VariableStringBuffer( "model", modelStr, sizeof( modelStr ) );
    if ( ( skin = strchr( modelStr, '/' ) ) == NULL) {
    skin = "default";
    } else {
    *skin++ = 0;
    }

    Q_strncpyz( newInfo.headName, skin, sizeof( newInfo.headName ) );
    Q_strncpyz( newInfo.modelName, modelStr, sizeof( newInfo.modelName ) );

    //		Q_strncpyz( newInfo.modelName, DEFAULT_MODEL, sizeof( newInfo.modelName ) );
    //		Q_strncpyz( newInfo.skinName, "default", sizeof( newInfo.skinName ) );

    if ( cgs.gametype >= GT_TEAM )
    {
    // keep skin name
    slash = strchr( v, '/' );
    if ( slash ) {
    Q_strncpyz( newInfo.modelName, slash + 1, sizeof( newInfo.modelName ) );
    }
    }
    }
    else */
    {
        Q_strncpyz( newInfo.modelName, v, sizeof( newInfo.modelName ) );

        slash = strchr( newInfo.modelName, '/' );
        if ( !slash ) {
            // modelName didn not include a skin name
            Q_strncpyz( newInfo.headName, "bruce", sizeof( newInfo.headName ) );
        } else {
            Q_strncpyz( newInfo.headName, slash + 1, sizeof( newInfo.headName ) );
            // truncate modelName
            *slash = 0;
        }
    }

    // scan for an existing clientinfo that matches this modelname
    // so we can avoid loading checks if possible
    if ( !CG_ScanForExistingClientInfo( &newInfo ) ) {
        /*
        qboolean	forceDefer;

        forceDefer = trap_MemoryRemaining() < 4000000;

        // if we are defering loads, just have it pick the first valid
        if ( forceDefer || ( cg_deferPlayers.integer && !cg_buildScript.integer && !cg.loading ) ) {
        // keep whatever they had if it won't violate team skins
        if ( ci->infoValid &&
        ( cgs.gametype < GT_TEAM  ) ) {
        CG_CopyClientInfoModel( ci, &newInfo );
        newInfo.deferred = qtrue;
        } else {
        // use whatever is available
        CG_SetDeferredClientInfo( &newInfo );
        }
        // if we are low on memory, leave them with this model
        if ( forceDefer ) {
        CG_Error( "Memory is low. Couldn't load playermodels, please turn down your graphic detail.\n" );
        newInfo.deferred = qfalse;
        }
        } else {
        CG_LoadClientInfo( &newInfo );
        }*/
        CG_Printf("Couldn't find playermodel in ram.\n");

        newInfo.infoValid = qfalse;
        *ci = newInfo;
        return;
    }
    // replace whatever was there with the new one
    newInfo.infoValid = qtrue;
    *ci = newInfo;
}



/*
======================
CG_LoadDeferredPlayers

Called each frame when a player is dead
and the scoreboard is up
so deferred players can be loaded
======================
*/
void CG_LoadDeferredPlayers( void ) {
    int		i;
    clientInfo_t	*ci;

    // scan for a deferred player to load
    for ( i = 0, ci = cgs.clientinfo ; i < cgs.maxclients ; i++, ci++ ) {
        if ( ci->infoValid && ci->deferred ) {
            // if we are low on memory, leave it deferred
            /*f ( trap_MemoryRemaining() < 4000000 ) {
            CG_Printf( "Memory is low.  Using deferred model.\n" );
            ci->deferred = qfalse;
            continue;
            }*/
            CG_LoadClientInfo( ci );
            //			break;
        }
    }
}

void CG_CachingClient(char *model, char *skin)
{
    //	char iconName[MAX_QPATH];
    char *s;

    s = va("PPM [%s]", model);
    CG_LoadingString(s);
}

void CG_CacheAllModels(void)
{
    int i;//, m;
    clientInfo_t *ci, *PrevCI = NULL;

    cgs.media.SealPlayerModelNames[0] = "s_medium";
    cgs.media.SealPlayerModelNames[1] = NULL;

    cgs.media.TangoPlayerModelNames[0] = "t_medium";
    cgs.media.TangoPlayerModelNames[1] = NULL;

    cgs.media.MiscPlayerModelNames[0] = "vip_male";
    cgs.media.MiscPlayerModelNames[1] = NULL;

    for ( i = 0; i < MAX_PLAYER_MODELS; i++ )
    {
        if ( !cgs.media.SealPlayerModelNames[i] )
            break;

        ci = cgs.media.SealPlayerModels + i;

        strcpy( ci->modelName, cgs.media.SealPlayerModelNames[i] );
        strcpy( ci->headName, "jayant");

        CG_CachingClient(ci->modelName, ci->skinName);

        CG_LoadClientInfo(ci);

        ci->team = TEAM_RED;

        ci->deferred = qfalse;
        ci->infoValid = qtrue;

    }

    ci = NULL;

    if (trap_MemoryRemaining() < 4000000)
        CG_Error("Not enough memory to precache playermodels.\n");

    for ( i = 0; i < MAX_PLAYER_MODELS; i++ )
    {
        if ( !cgs.media.TangoPlayerModelNames[i] )
            break;

        ci = cgs.media.TangoPlayerModels + i;

        strcpy( ci->modelName, cgs.media.TangoPlayerModelNames[i] );
        strcpy( ci->headName, "bruce");

        ci->team = TEAM_BLUE;

        CG_CachingClient(ci->modelName, ci->skinName);

        CG_LoadClientInfo(ci);

        ci->deferred = qfalse;
        ci->infoValid = qtrue;

    }

    if (trap_MemoryRemaining() < 4000000)
        CG_Error("Not enough memory to precache playermodels.\n");

    for ( i = 0; i < MAX_PLAYER_MODELS; i++ )
    {
        if ( !cgs.media.MiscPlayerModelNames[i] )
            break;
        if ( !Q_stricmp( cgs.media.MiscPlayerModelNames[i], "vip_male") )
        {
            // if not in vip mode skip precaching the vip player model
            if ( !cgs.mi_vipRescue && !cgs.mi_vipTime )
                continue;
        }

        ci = cgs.media.MiscPlayerModels + i;

        strcpy( ci->modelName, cgs.media.MiscPlayerModelNames[i] );
        strcpy( ci->headName, "vip_seal");

        CG_CachingClient(ci->modelName, ci->skinName);

        CG_LoadClientInfo(ci);

        ci->team = TEAM_RED;

        ci->deferred = qfalse;
        ci->infoValid = qtrue;
    }

    ci = NULL;

    if (trap_MemoryRemaining() < 4000000)
        CG_Error("Not enough memory to precache playermodels.\n");

}
/*
=============================================================================

PLAYER ANIMATION

=============================================================================
*/


/*
===============
CG_SetLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
static void CG_SetLerpFrameAnimation( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation ) {
    animation_t	*anim;

    lf->animationNumber = newAnimation;
    newAnimation &= ~ANIM_TOGGLEBIT;

    if ( newAnimation < 0 || newAnimation >= MAX_ANIMATIONS ) {
        CG_Error( "Bad animation number: %i", newAnimation );
    }

    anim = &ci->animations[ newAnimation ];

    lf->animation = anim;
    lf->animationTime = lf->frameTime + anim->initialLerp;

    if ( cg_debugAnim.integer ) {
        CG_Printf( "Anim: %i\n", newAnimation );
    }
}

/*
===============
CG_RunLerpFrame

Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
static void CG_RunLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation, float speedScale ) {
    int			f, numFrames;
    animation_t	*anim;

    // debugging tool to get no animations
    if ( cg_animSpeed.integer == 0 ) {
        lf->oldFrame = lf->frame = lf->backlerp = 0;
        return;
    }

    // see if the animation sequence is switching
    if ( newAnimation != lf->animationNumber || !lf->animation ) {
        CG_SetLerpFrameAnimation( ci, lf, newAnimation );
    }

    if ( cg_animSpeed.value != 1.0f ) {
        speedScale = cg_animSpeed.value;
    }

    // if we have passed the current frame, move it to
    // oldFrame and calculate a new frame
    if ( cg.time >= lf->frameTime ) {
        lf->oldFrame = lf->frame;
        lf->oldFrameTime = lf->frameTime;

        // get the next frame based on the animation
        anim = lf->animation;
        if ( !anim->frameLerp ) {
            return;		// shouldn't happen
        }
        if ( cg.time < lf->animationTime ) {
            lf->frameTime = lf->animationTime;		// initial lerp
        } else {
            lf->frameTime = lf->oldFrameTime + anim->frameLerp;
        }
        f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;
        f *= speedScale;		// adjust for haste, etc


        numFrames = anim->numFrames;
        if (anim->flipflop) {
            numFrames *= 2;
        }
        if ( f >= numFrames ) {
            f -= numFrames;
            if ( anim->loopFrames ) {
                f %= anim->loopFrames;
                f += anim->numFrames - anim->loopFrames;
            } else {
                f = numFrames - 1;
                // the animation is stuck at the end, so it
                // can immediately transition to another sequence
                lf->frameTime = cg.time;
            }
        }
        if ( anim->reversed ) {
            lf->frame = anim->firstFrame + anim->numFrames - 1 - f;
        }
        else if (anim->flipflop && f>=anim->numFrames) {
            lf->frame = anim->firstFrame + anim->numFrames - 1 - (f%anim->numFrames);
        }
        else {
            lf->frame = anim->firstFrame + f;
        }
        if ( cg.time > lf->frameTime ) {
            lf->frameTime = cg.time;
            if ( cg_debugAnim.integer ) {
                CG_Printf( "Clamp lf->frameTime\n");
            }
        }
    }

    if ( lf->frameTime > cg.time + 200 ) {
        lf->frameTime = cg.time;
    }

    if ( lf->oldFrameTime > cg.time ) {
        lf->oldFrameTime = cg.time;
    }
    // calculate current lerp value
    if ( lf->frameTime == lf->oldFrameTime ) {
        lf->backlerp = 0;
    } else {
        lf->backlerp = 1.0 - (float)( cg.time - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
    }
}


/*
===============
CG_ClearLerpFrame
===============
*/
static void CG_ClearLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int animationNumber ) {
    lf->frameTime = lf->oldFrameTime = cg.time;
    CG_SetLerpFrameAnimation( ci, lf, animationNumber );
    lf->oldFrame = lf->frame = lf->animation->firstFrame;
}


/*
===============
CG_PlayerAnimation
===============
*/
static void CG_PlayerAnimation( centity_t *cent, int *legsOld, int *legs, float *legsBackLerp,
                                int *torsoOld, int *torso, float *torsoBackLerp ) {
    clientInfo_t	*ci;
    int				clientNum;
    float			speedScale;

    clientNum = cent->currentState.clientNum;

    if ( cg_noPlayerAnims.integer ) {
        *legsOld = *legs = *torsoOld = *torso = 0;
        return;
    }

#if 0
    if ( cent->currentState.powerups & ( 1 << PW_HASTE ) ) {
        speedScale = 1.5;
    } else
#endif
    {
        speedScale = 1;
    }

    ci = &cgs.clientinfo[ clientNum ];

    // do the shuffle turn frames locally
    if ( cent->pe.legs.yawing && ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_IDLE ) {
        CG_RunLerpFrame( ci, &cent->pe.legs, LEGS_TURN, speedScale );
    } else {
        CG_RunLerpFrame( ci, &cent->pe.legs, cent->currentState.legsAnim, speedScale );
    }

    *legsOld = cent->pe.legs.oldFrame;
    *legs = cent->pe.legs.frame;
    *legsBackLerp = cent->pe.legs.backlerp;

    CG_RunLerpFrame( ci, &cent->pe.torso, cent->currentState.torsoAnim, speedScale );

    *torsoOld = cent->pe.torso.oldFrame;
    *torso = cent->pe.torso.frame;
    *torsoBackLerp = cent->pe.torso.backlerp;
}

/*
=============================================================================

PLAYER ANGLES

=============================================================================
*/

/*
==================
CG_SwingAngles
==================
*/
static void CG_SwingAngles( float destination, float swingTolerance, float clampTolerance,
                            float speed, float *angle, qboolean *swinging ) {
    float	swing;
    float	move;
    float	scale;

    if ( !*swinging ) {
        // see if a swing should be started
        swing = AngleSubtract( *angle, destination );
        if ( swing > swingTolerance || swing < -swingTolerance ) {
            *swinging = qtrue;
        }
    }

    if ( !*swinging ) {
        return;
    }

    // modify the speed depending on the delta
    // so it doesn't seem so linear
    swing = AngleSubtract( destination, *angle );
    scale = fabs( swing );
    if ( scale < swingTolerance * 0.5 ) {
        scale = 0.5;
    } else if ( scale < swingTolerance ) {
        scale = 1.0;
    } else {
        scale = 2.0;
    }

    // swing towards the destination angle
    if ( swing >= 0 ) {
        move = cg.frametime * scale * speed;
        if ( move >= swing ) {
            move = swing;
            *swinging = qfalse;
        }
        *angle = AngleMod( *angle + move );
    } else if ( swing < 0 ) {
        move = cg.frametime * scale * -speed;
        if ( move <= swing ) {
            move = swing;
            *swinging = qfalse;
        }
        *angle = AngleMod( *angle + move );
    }

    // clamp to no more than tolerance
    swing = AngleSubtract( destination, *angle );
    if ( swing > clampTolerance ) {
        *angle = AngleMod( destination - (clampTolerance - 1) );
    } else if ( swing < -clampTolerance ) {
        *angle = AngleMod( destination + (clampTolerance - 1) );
    }
}

/*
=================
CG_AddPainTwitch
=================
*/
static void CG_AddPainTwitch( centity_t *cent, vec3_t torsoAngles ) {
    int		t;
    float	f;

    t = cg.time - cent->pe.painTime;
    if ( t >= PAIN_TWITCH_TIME ) {
        return;
    }

    f = 1.0 - (float)t / PAIN_TWITCH_TIME;

    if ( cent->pe.painDirection ) {
        torsoAngles[ROLL] += 20 * f;
    } else {
        torsoAngles[ROLL] -= 20 * f;
    }
}


/*
===============
CG_PlayerAngles

Handles seperate torso motion

legs pivot based on direction of movement

head always looks exactly at cent->lerpAngles

if motion < 20 degrees, show in head only
if < 45 degrees, also show in torso
===============
*/
static void CG_PlayerAngles( centity_t *cent, vec3_t legs[3], vec3_t torso[3] , vec3_t arms[3],  vec3_t head[3] ) {
    vec3_t		legsAngles, torsoAngles, headAngles   ;
    float		dest;
    static	int	movementOffsets[8] = { 0, 22, 45, -22, 0, 22, -45, -22 };
    vec3_t		velocity;
    float		speed;
    int			dir;

    VectorCopy( cent->lerpAngles, headAngles );
    headAngles[YAW] = AngleMod( headAngles[YAW] );
    VectorClear( legsAngles );
    VectorClear( torsoAngles );

    // --------- yaw -------------

    // allow yaw to drift a bit
    if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_IDLE
            || ( cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_STAND_RIFLE ||
            ( cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_STAND_ITEM
            || ( cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_STAND_PISTOL ) {
        // if not standing still, always point all in the same direction
        cent->pe.torso.yawing = qtrue;	// always center
        cent->pe.torso.pitching = qtrue;	// always center
        cent->pe.legs.yawing = qtrue;		// always center
    }

    // adjust legs for movement dir
    if ( cent->currentState.eFlags & EF_DEAD ) {
        // don't let dead bodies twitch
        dir = 0;
    } else {
        dir = cent->currentState.angles2[YAW];
        if ( dir < 0 || dir > 7 ) {
            CG_Error( "Bad player movement angle" );
        }
    }
    legsAngles[YAW] = headAngles[YAW] + movementOffsets[ dir ];
    torsoAngles[YAW] = headAngles[YAW] + 0.25 * movementOffsets[ dir ];

    // torso
    CG_SwingAngles( torsoAngles[YAW], 25, 90, cg_swingSpeed.value, &cent->pe.torso.yawAngle, &cent->pe.torso.yawing );
    CG_SwingAngles( legsAngles[YAW], 40, 90, cg_swingSpeed.value, &cent->pe.legs.yawAngle, &cent->pe.legs.yawing );

    torsoAngles[YAW] = cent->pe.torso.yawAngle;
    legsAngles[YAW] = cent->pe.legs.yawAngle;

    // --------- pitch -------------

    // only show a fraction of the pitch angle in the torso
    if ( headAngles[PITCH] > 180 ) {
        dest = (-360 + headAngles[PITCH]) * 0.75f;
    } else {
        dest = headAngles[PITCH] * 0.75f;
    }
    CG_SwingAngles( dest, 15, 30, 0.1f, &cent->pe.torso.pitchAngle, &cent->pe.torso.pitching );
    torsoAngles[PITCH] = cent->pe.torso.pitchAngle;


    if (
        CG_IsPlayerInAnim( cent, TORSO_DROP_RIFLE , qfalse ) ||
        CG_IsPlayerInAnim( cent, TORSO_DROP_PISTOL , qfalse ) )
        torsoAngles[PITCH] = 0;

    // --------- roll -------------


    // lean towards the direction of travel
    VectorCopy( cent->currentState.pos.trDelta, velocity );
    speed = VectorNormalize( velocity );
    if ( speed ) {
        vec3_t	axis[3];
        float	side;

        speed *= 0.05f;

        AnglesToAxis( legsAngles, axis );
        side = speed * DotProduct( velocity, axis[1] );
        legsAngles[ROLL] -= side;

        side = speed * DotProduct( velocity, axis[0] );
        legsAngles[PITCH] += side;
    }

    // pain twitch
    CG_AddPainTwitch( cent, torsoAngles );

    {
        vec3_t armAngles;

        VectorClear(armAngles);

        VectorCopy( headAngles, armAngles );
        VectorSubtract( armAngles, torsoAngles, armAngles );
        armAngles[ PITCH ] = torsoAngles[ PITCH ];

        if ( armAngles[PITCH] < -30.0f )
            armAngles[PITCH] = -30.0f;

        AnglesToAxis( armAngles, arms );
    }
    // pull the angles back out of the hierarchial chain
    AnglesSubtract( headAngles, torsoAngles, headAngles );
    AnglesSubtract( torsoAngles, legsAngles, torsoAngles );
    AnglesToAxis( legsAngles, legs );


    if ( torsoAngles[PITCH] < -5.0f )
        torsoAngles[PITCH] = -5.0f;
    else if ( torsoAngles[PITCH] > 5.0f )
        torsoAngles[PITCH] = 5.0f;

    AnglesToAxis( torsoAngles, torso );
    AnglesToAxis( headAngles, head );
}


//==========================================================================

/*
===============
CG_BreathPuffs
===============
*/
static void CG_BreathPuffs( centity_t *cent, refEntity_t *head) {
    clientInfo_t *ci;
    vec3_t up, origin;
    int contents;

    ci = &cgs.clientinfo[ cent->currentState.number ];

    if (!cg_enableBreath.integer) {
        return;
    }

    if ( cgs.camoType != CAMO_ARCTIC )
        return;

    if ( cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson) {
        return;
    }
    if ( cent->currentState.eFlags & EF_DEAD ) {
        return;
    }
    contents = trap_CM_PointContents( head->origin, 0 );
    if ( contents & CONTENTS_WATER ) {
        return;
    }
    if ( ci->breathPuffTime > cg.time ) {
        return;
    }
    VectorSet( up, 0, 0, 8 );
    VectorMA(head->origin, 8, head->axis[0], origin);
    VectorMA(origin, -4, head->axis[2], origin);

    CG_SmokePuff( origin, up,
                  4,
                  1, 1, 1, 0.33f,
                  1500,
                  cg.time, 0, LE_MOVE_SCALE_FADE, cgs.media.smokePuffShader );
    ci->breathPuffTime = cg.time + 2000;
}

/*
===============
CG_DustTrail
===============
*/
static void CG_DustTrail( centity_t *cent ) {
    int				anim;
    localEntity_t	*dust;
    vec3_t end, vel;
    trace_t tr;

    if (!cg_enableDust.integer)
        return;

    if ( cent->dustTrailTime > cg.time ) {
        return;
    }

    anim = cent->pe.legs.animationNumber & ~ANIM_TOGGLEBIT;
    if ( anim != LEGS_LAND ) {
        return;
    }

    cent->dustTrailTime += 40;
    if ( cent->dustTrailTime < cg.time ) {
        cent->dustTrailTime = cg.time;
    }

    VectorCopy(cent->currentState.pos.trBase, end);
    end[2] -= 64;
    CG_Trace( &tr, cent->currentState.pos.trBase, NULL, NULL, end, cent->currentState.number, MASK_PLAYERSOLID );

    if ( !(tr.surfaceFlags & SURF_DUST) )
        return;

    VectorCopy( cent->currentState.pos.trBase, end );
    end[2] -= 16;

    VectorSet(vel, 0, 0, -30);
    dust = CG_SmokePuff( end, vel,
                         24,
                         .8f, .8f, 0.7f, 0.33f,
                         500,
                         cg.time,
                         0,
                         0,
                         cgs.media.dustPuffShader );
}

#if 0
/*
===============
CG_TrailItem
===============
*/
static void CG_TrailItem( centity_t *cent, qhandle_t hModel ) {
    refEntity_t		ent;
    vec3_t			angles;
    vec3_t			axis[3];

    VectorCopy( cent->lerpAngles, angles );
    angles[PITCH] = 0;
    angles[ROLL] = 0;
    AnglesToAxis( angles, axis );

    memset( &ent, 0, sizeof( ent ) );
    VectorMA( cent->lerpOrigin, -16, axis[0], ent.origin );
    ent.origin[2] += 16;
    angles[YAW] += 90;
    AnglesToAxis( angles, ent.axis );

    ent.hModel = hModel;
    trap_R_AddRefEntityToScene( &ent );
}
#endif
/*
===============
CG_PlayerPowerups
===============
*/
#ifdef NEW_ANIMS
static void CG_PlayerPowerups( centity_t *cent, refEntity_t *torso ) {
#else
static void CG_PlayerPowerups( centity_t *cent ) {
#endif
    int		powerups;

    powerups = cent->currentState.powerups;
    if ( !powerups ) {
        return;
    }
#if 0
    if ( powerups & ( 1 << PW_BRIEFCASE ) ) {
        CG_TrailItem( cent, cgs.media.briefcaseModel );
    }
#endif


}


/*
===============
CG_PlayerFloatSprite

Float a sprite over the player's head
===============
*/
static void CG_PlayerFloatSprite( centity_t *cent, qhandle_t shader ) {
    int				rf;
    refEntity_t		ent;

    if ( cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson ) {
        rf = RF_THIRD_PERSON;		// only show in mirrors
    } else {
        rf = 0;
    }

    memset( &ent, 0, sizeof( ent ) );
    VectorCopy( cent->lerpOrigin, ent.origin );
    ent.origin[2] += 48;
    ent.reType = RT_SPRITE;
    ent.customShader = shader;
    ent.radius = 10;
    ent.renderfx = rf;
    ent.shaderRGBA[0] = 255;
    ent.shaderRGBA[1] = 255;
    ent.shaderRGBA[2] = 255;
    ent.shaderRGBA[3] = 255;
    trap_R_AddRefEntityToScene( &ent );
}



/*
===============
CG_PlayerSprites

Float sprites over the player's head
===============
*/
static void CG_PlayerSprites( centity_t *cent ) {
    int		team;

    team = cgs.clientinfo[ cent->currentState.clientNum ].team;

    if ( cent->currentState.eFlags & EF_CONNECTION ) {
        CG_PlayerFloatSprite( cent, cgs.media.connectionShader );
        return;
    }

    /*if ( cent->currentState.eFlags & EF_TALK ) {
    CG_PlayerFloatSprite( cent, cgs.media.balloonShader );
    return;
    }*/
    if (
        cent->currentState.eFlags & EF_RADIO_TALK &&
        cg.snap->ps.persistant[PERS_TEAM] == team
    ) {
        CG_PlayerFloatSprite( cent, cgs.media.radioIcon );
        return;
    }
#ifdef AWARDS
    if ( cent->currentState.eFlags & EF_AWARD_IMPRESSIVE ) {
        CG_PlayerFloatSprite( cent, cgs.media.medalImpressive );
        return;
    }

    if ( cent->currentState.eFlags & EF_AWARD_EXCELLENT ) {
        CG_PlayerFloatSprite( cent, cgs.media.medalExcellent );
        return;
    }

    if ( cent->currentState.eFlags & EF_AWARD_GAUNTLET ) {
        CG_PlayerFloatSprite( cent, cgs.media.medalGauntlet );
        return;
    }

    if ( cent->currentState.eFlags & EF_AWARD_DEFEND ) {
        CG_PlayerFloatSprite( cent, cgs.media.medalDefend );
        return;
    }

    if ( cent->currentState.eFlags & EF_AWARD_ASSIST ) {
        CG_PlayerFloatSprite( cent, cgs.media.medalAssist );
        return;
    }

    if ( cent->currentState.eFlags & EF_AWARD_CAP ) {
        CG_PlayerFloatSprite( cent, cgs.media.medalCapture );
        return;
    }
#endif
    /*
    if ( !(cent->currentState.eFlags & EF_DEAD) &&
    cg.snap->ps.persistant[PERS_TEAM] == team &&
    cgs.gametype >= GT_TEAM) {
    if (cg_drawFriend.integer) {
    CG_PlayerFloatSprite( cent, cgs.media.friendShader );
    }
    return;
    }*/
}

/*
===============
CG_PlayerShadow

Returns the Z component of the surface being shadowed

should it return a full plane instead of a Z?
===============
*/
#define	SHADOW_DISTANCE		128

void CG_DirectImpactMark( qhandle_t markShader, const vec3_t origin, const vec3_t dir, float orientation, float red, float green , float blue, float alpha, qboolean alphaFade, float radius, qboolean temporary, int entitynum );

static qboolean CG_PlayerShadow( centity_t *cent, vec3_t origin, float radius, float *shadowPlane ) {
    vec3_t		end, mins = {-5, -5, 5}, maxs = {5, 5, 5};
    trace_t		trace;
    float		alpha;
    vec3_t		start;

    if ( shadowPlane )
        *shadowPlane = 0;

    if ( cg_shadows.integer == 0 ) {
        return qfalse;
    }

    // send a trace down from the player to the ground
    VectorCopy( origin, end );
    VectorCopy( origin, start );

    start[2] += 5;
    end[2] -= SHADOW_DISTANCE;

    trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_SOLID );

    // no shadow if too high
    if ( trace.fraction == 1.0 ) {
        return qfalse;
    }

    if ( shadowPlane )
        *shadowPlane = trace.endpos[2] + 1;

    if ( cg_shadows.integer != 1 ) {	// no mark for stencil or projection shadows
        return qtrue;
    }

    // fade the shadow out with height
    alpha = 1.0 - trace.fraction;

    // add the mark as a temporary, so it goes directly to the renderer
    // without taking a spot in the cg_marks array
    CG_DirectImpactMark( cgs.media.shadowMarkShader, trace.endpos, trace.plane.normal,
                         cent->pe.legs.yawAngle, alpha,alpha,alpha,1, qfalse, radius, qtrue, ENTITYNUM_NONE );

    return qtrue;
}

/*
===============
CG_PlayerSplash

Draw a mark at the water surface
===============
*/
static void CG_PlayerSplash( centity_t *cent ) {
    vec3_t		start, end;
    trace_t		trace;
    int			contents;

    VectorCopy( cent->lerpOrigin, end );
    end[2] -= 25;

    // if the feet aren't in liquid, don't make a mark
    // this won't handle moving water brushes, but they wouldn't draw right anyway...
    contents = trap_CM_PointContents( end, 0 );
    if ( !( contents & CONTENTS_WATER ) ) {
        return;
    }

    // feet are in liquid that's more or less all we want to know
    cent->leftWaterTime = cg.time;


    if ( !cg_shadows.integer ) {
        return;
    }

    if ( cent->wakemarkTime > cg.time )
        return;

    VectorCopy( cent->lerpOrigin, start );
    start[2] += 32;

    // if the head isn't out of liquid, don't make a mark
    contents = trap_CM_PointContents( start, 0 );
    if ( contents & ( CONTENTS_SOLID | CONTENTS_WATER ) ) {
        return;
    }

    // trace down to find the surface
    trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, ( CONTENTS_WATER ) );

    if ( trace.fraction == 1.0 ) {
        return;
    }

    cent->wakemarkTime = cg.time + cg_wakemarkDistantTime.value;
    // Navy Seals ++
    // clear out smoking guns
    if ( cent->gunSmokeTime )
        cent->gunSmokeTime = 0;
    // Navy Seals --

    CG_DirectImpactMark( cgs.media.wakeMarkShader, trace.endpos,trace.plane.normal, rand()%360,1,1,1,0.4f,qfalse,cg_wakemarkTime.value, qfalse , -1 );
    //
    //	CG_ImpactMark( cgs.media.shadowMarkShader, trace.endpos, trace.plane.normal,
    //		cent->pe.legs.yawAngle, alpha,alpha,alpha,1, qfalse, 24, qtrue );


    /*
    // create a mark polygon
    VectorCopy( trace.endpos, verts[0].xyz );
    verts[0].xyz[0] -= 32;
    verts[0].xyz[1] -= 32;
    verts[0].st[0] = 0;
    verts[0].st[1] = 0;
    verts[0].modulate[0] = 255;
    verts[0].modulate[1] = 255;
    verts[0].modulate[2] = 255;
    verts[0].modulate[3] = 255;

    VectorCopy( trace.endpos, verts[1].xyz );
    verts[1].xyz[0] -= 32;
    verts[1].xyz[1] += 32;
    verts[1].st[0] = 0;
    verts[1].st[1] = 1;
    verts[1].modulate[0] = 255;
    verts[1].modulate[1] = 255;
    verts[1].modulate[2] = 255;
    verts[1].modulate[3] = 255;

    VectorCopy( trace.endpos, verts[2].xyz );
    verts[2].xyz[0] += 32;
    verts[2].xyz[1] += 32;
    verts[2].st[0] = 1;
    verts[2].st[1] = 1;
    verts[2].modulate[0] = 255;
    verts[2].modulate[1] = 255;
    verts[2].modulate[2] = 255;
    verts[2].modulate[3] = 255;

    VectorCopy( trace.endpos, verts[3].xyz );
    verts[3].xyz[0] += 32;
    verts[3].xyz[1] -= 32;
    verts[3].st[0] = 1;
    verts[3].st[1] = 0;
    verts[3].modulate[0] = 255;
    verts[3].modulate[1] = 255;
    verts[3].modulate[2] = 255;
    verts[3].modulate[3] = 255;
    */
    //	trap_R_AddPolyToScene( cgs.media.wakeMarkShader, 4, verts );
}

/*
===============
CG_PlayerSplash

Draw a mark at the water surface
===============
*/
void CG_BloodPool( vec3_t origin) {
    vec3_t		end;
    trace_t		trace;

    // copy endvector for traceline
    VectorCopy( origin, end );
    end[2] -= 128;

    // trace down to find the surface
    trap_CM_BoxTrace( &trace, origin, end, NULL, NULL, 0, CONTENTS_SOLID );

    // no valid groundplane ?
    if ( trace.fraction == 1.0 )
        return;

    VectorCopy( trace.endpos, end );

    // move it randomly around
    if (random() < 0.5)
        end[0] += random()*10;
    else
        end[0] -= random()*10;

    if (random() < 0.5)
        end[1] += random()*10;
    else
        end[1] -= random()*10;

    CG_ImpactMark( cgs.media.ns_bloodPool, end, trace.plane.normal, 0,1,1,1,0.8f,qfalse,2 + random()*4,qfalse );
}


/*
===============
CG_AddRefEntityWithPowerups

Adds a piece with modifications or duplications for powerups
Also called by CG_Missile for quad rockets, but nobody can tell...
===============
*/

void CG_AddRefEntityWithPowerups( refEntity_t *ent, entityState_t *state, int team ) {

    if (!state) {
        trap_R_AddRefEntityToScene( ent );
        return;
    }

    if (cg.snap->ps.stats[STAT_ACTIVE_ITEMS] & ( 1 << UI_NVG ) )
    {
        ent->customShader = cgs.media.thermalGogglesNoise; // TG Shader
        trap_R_AddRefEntityToScene( ent );
        return; // don't add any other shader
    }

    ent->customShader = 0;
    trap_R_AddRefEntityToScene( ent );
}

/*
=================
CG_LightVerts
=================
*/
int CG_LightVerts( vec3_t normal, int numVerts, polyVert_t *verts )
{
    int				i, j;
    float			incoming;
    vec3_t			ambientLight;
    vec3_t			lightDir;
    vec3_t			directedLight;

    trap_R_LightForPoint( verts[0].xyz, ambientLight, directedLight, lightDir );

    for (i = 0; i < numVerts; i++) {
        incoming = DotProduct (normal, lightDir);
        if ( incoming <= 0 ) {
            verts[i].modulate[0] = ambientLight[0];
            verts[i].modulate[1] = ambientLight[1];
            verts[i].modulate[2] = ambientLight[2];
            verts[i].modulate[3] = 255;
            continue;
        }
        j = ( ambientLight[0] + incoming * directedLight[0] );
        if ( j > 255 ) {
            j = 255;
        }
        verts[i].modulate[0] = j;

        j = ( ambientLight[1] + incoming * directedLight[1] );
        if ( j > 255 ) {
            j = 255;
        }
        verts[i].modulate[1] = j;

        j = ( ambientLight[2] + incoming * directedLight[2] );
        if ( j > 255 ) {
            j = 255;
        }
        verts[i].modulate[2] = j;

        verts[i].modulate[3] = 255;
    }
    return qtrue;
}


/*
// Camoflage Defines
#define CAMO_ARCTIC 0
#define CAMO_DESERT 1
#define CAMO_JUNGLE	2
#define CAMO_URBAN	3

// Player Equipment
#define EQ_STORMGOGGLES 1
#define EQ_JOINT		2
#define EQ_PIECEOFHAY	3
#define EQ_NVGOGGLES	4
#define EQ_SEALHAT		5
#define EQ_TURBAN		6
#define EQ_HELMET		7

CAMO_NONE, // 0

CAMO_DESERT, // 1
CAMO_JUNGLE, // 2
CAMO_ARCTIC,// 3
CAMO_URBAN,// 4

-1
*/

qhandle_t CG_GetEquipmentModel( int equip , int team)
{
    if (equip == EQ_HELMET )
    {
        switch (team)
        {
        case TEAM_RED:
            return cgs.media.playerSealHelmet[cgs.camoType-1];
            break;
        case TEAM_BLUE:
            return cgs.media.playerTangoHelmet[cgs.camoType-1];
            break;
        default:
            return cgs.media.playerSealHelmet[cgs.camoType-1];
            break;
        }
    }

    return 0;
}

static qboolean CG_InValidFootPrint( clientInfo_t *ci, int frame )
{
    if ( frame == ci->footprintFrameTimer.step_backl )
        return qtrue;
    if ( frame == ci->footprintFrameTimer.step_backr )
        return qtrue;
    if ( frame == ci->footprintFrameTimer.step_limpl )
        return qtrue;
    if ( frame == ci->footprintFrameTimer.step_limpr )
        return qtrue;
    if ( frame == ci->footprintFrameTimer.step_runl )
        return qtrue;
    if ( frame == ci->footprintFrameTimer.step_runr )
        return qtrue;
    if ( frame == ci->footprintFrameTimer.step_walkl )
        return qtrue;
    if ( frame == ci->footprintFrameTimer.step_walkr )
        return qtrue;

    return qfalse;
}
/*
===============
CG_Player
===============
*/
void CG_SpawnParticle( vec3_t org, vec3_t dir, float speed, float bouncefactor, float radius, float r, float g ,float b, float a, qboolean size );

void CG_Player( centity_t *cent ) {
    clientInfo_t	*ci;
    refEntity_t		legs;
    refEntity_t		torso;
    refEntity_t		head;
    refEntity_t		larm,rarm; // just one reference

    int				clientNum;
    int				renderfx;
    qboolean		shadow;
    float			shadowPlane;

    refEntity_t		primary;

    vec3_t			lO;

    // the client number is stored in clientNum.  It can't be derived
    // from the entity number, because a single client may have
    // multiple corpses on the level using the same clientinfo
    clientNum = cent->currentState.clientNum;
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        CG_Error( "Bad clientNum on player entity");
    }
    ci = &cgs.clientinfo[ clientNum ];

    // it isn't possible to see corpses from disconnected players that may
    // not have valid clientinfo
    if ( !ci->infoValid ) {
        return;
    }

    // get the player model information
    renderfx = 0;

    if ( cent->currentState.number == cg.snap->ps.clientNum)
    {
        if (!cg.renderingThirdPerson ){// || BG_IsZooming( cg.snap->ps.stats[STAT_WEAPONMODE] ) ) {
            renderfx = RF_THIRD_PERSON; // only draw in mirrors
        } else {
            if (cg_cameraMode.integer) {
                return;
            }
        }
    }

    /*
    if ( cent->currentState.number == cg.snap->ps.weapon && cg.snap->ps.pm_flags & PMF_FOLLOW )
    {
    //		CG_Printf("following %i\n", cent->currentState.number );
    VectorCopy( cent->lerpOrigin, cg.cameraOrigin );
    }
    */
    memset( &legs, 0, sizeof(legs) );
    memset( &torso, 0, sizeof(torso) );
    memset( &head, 0, sizeof(head) );
    memset( &primary, 0, sizeof(primary) );

    memset( &larm, 0, sizeof(larm) );
    memset( &rarm, 0, sizeof(rarm) );

    // Navy Seals ++
    if ( pmodel_o.integer != 0 ) {
        VectorCopy( cent->lerpOrigin, lO );

        cent->lerpOrigin[2] += pmodel_o.value;
        //	CG_Printf("Alinged %s\n", pmodel_o.string );
    }


    // Navy Seals --
    // get the rotation information
    CG_PlayerAngles( cent, legs.axis, torso.axis, larm.axis,  head.axis );

    // copy the same axis to the rightarm
    AxisCopy( larm.axis, rarm.axis );

    // get the animation state (after rotation, to allow feet shuffle)
    CG_PlayerAnimation( cent, &legs.oldframe, &legs.frame, &legs.backlerp,
                        &torso.oldframe, &torso.frame, &torso.backlerp );

#ifndef NEW_ANIMS
    // add powerups floating behind the player
    CG_PlayerPowerups( cent );
#endif

    // add the talk baloon or disconnect icon
    CG_PlayerSprites( cent );

    // add a water splash if partially in and out of water
    CG_PlayerSplash( cent );


    //
    // add the legs
    //
    legs.hModel = ci->legsModel;
    legs.customSkin = ci->legsSkin;

    VectorCopy( cent->lerpOrigin, legs.origin );
    VectorCopy( cent->lerpOrigin, legs.lightingOrigin );

    VectorCopy (legs.origin, legs.oldorigin);	// don't positionally lerp at all

    //
    // add new 2foot shadow
    //
    {
        vec3_t lfoot,rfoot;

        // add the shadow
        shadow = CG_PlayerShadow( cent, cent->lerpOrigin, 10, &shadowPlane );

        CG_GetOriginFromTag( &legs, legs.hModel, "tag_footl", lfoot );
        CG_GetOriginFromTag( &legs, legs.hModel, "tag_footr", rfoot );

        CG_PlayerShadow( cent, lfoot, 5, 0 );
        CG_PlayerShadow( cent, rfoot, 5, 0 );

        if ( cg_shadows.integer == 3 && shadow ) {
            renderfx |= RF_SHADOW_PLANE;
        }

        // use the same origin for all
        renderfx |= RF_LIGHTING_ORIGIN;

        legs.shadowPlane = shadowPlane;
        legs.renderfx = renderfx;

        // footprints
        {
            int t = cg.time - cent->leftWaterTime;
            float alpha = 0.0f;

            // if we're under the time
            if ( CG_InValidFootPrint( ci, cent->pe.legs.frame )  && cent->pe.legs.oldFrame != cent->pe.legs.frame )
            {
                int	  footprint = 0;
                trace_t trace;
                vec3_t start, end;
                int frame = cent->pe.legs.frame;


                //CG_Printf("frame: %i\n",frame);
                if ( frame == ci->footprintFrameTimer.step_backl ||
                        frame == ci->footprintFrameTimer.step_limpl ||
                        frame == ci->footprintFrameTimer.step_runl ||
                        frame == ci->footprintFrameTimer.step_walkl )
                {
                    // this means right foot just stepped with right foot.
                    footprint = 1;
                }
                else
                    footprint = 2;


                if ( footprint == 1 )
                    VectorCopy( lfoot, end );

                if ( footprint == 2 )
                    VectorCopy( rfoot, end );

                VectorCopy( end,start );

                start[2] += 15;
                end[2] -= 50;

                // measure distance from groundplane to both feet so we can decided
                // which feet's the closer one
                CG_Trace( &trace, start, NULL,NULL, end, cent->currentState.clientNum, CONTENTS_SOLID );

                if ( footprint > 0 && (trace.surfaceFlags & SURF_SNOWSTEPS || trace.surfaceFlags & SURF_DIRTSTEPS || trace.surfaceFlags & SURF_SANDSTEPS )  )
                {
                    int i;
                    int max = 4;


                    // spawn a bunch of particles
                    for ( i = 0; i < max; i++ ) {
                        vec3_t	movedir;
                        vec4_t	randomcolor;
                        float	spread = 2;

                        if ( trace.surfaceFlags & SURF_SNOWSTEPS )
                            randomcolor[0] = randomcolor[1] = randomcolor[2] = 0.5 + random()/3;
                        else if ( trace.surfaceFlags & SURF_DIRTSTEPS )
                        {
                            randomcolor[0] = 0.2 + random()/3.3; //we'll create our brown
                            randomcolor[1] = 0.2 + random()/10;
                            randomcolor[2] = 0.1 + random()/5;
                        }
                        else // sand
                        {
                            // sand color
                            randomcolor[0] = 0.6 + random()/6;
                            randomcolor[1] = 0.5 + random()/6;
                            randomcolor[2] = random()/10;
                            spread = 1.5;
                        }

                        VectorCopy( trace.plane.normal, movedir );

                        if ( random() < 0.33 )
                            movedir[0] += random()/spread;
                        else if ( random() < 0.66 )
                            movedir[0] -= random()/spread;

                        if ( random() < 0.33 )
                            movedir[1] += random()/spread;
                        else if ( random() < 0.66 )
                            movedir[1] -= random()/spread;

                        if ( trace.plane.normal[2] > 0 )
                            movedir[2] += random();
                        else if ( trace.plane.normal[2] < 0 )
                            movedir[2] -= random();
                        else
                            movedir[2] += random();

                        CG_SpawnParticle( trace.endpos, movedir, (trace.surfaceFlags & SURF_SANDSTEPS)?(30+random()*25):(50 + random()*20), random()/2.5,(trace.surfaceFlags & SURF_SANDSTEPS)?(random()/2+random()/3 ):( 0.1 + random() + random()/2), randomcolor[0],randomcolor[1],randomcolor[2], 1.0f, qfalse );
                    }
                }

            }

#define MARK_FOOTPRINT_TIME	60000 // you are printing footsteps at least 3mins

            if ( t < MARK_FOOTPRINT_TIME && t > 100 && cent->leftWaterTime > 0 )
                if ( cent->footstepSpawn && CG_InValidFootPrint( ci, cent->pe.legs.frame ) && cent->pe.legs.oldFrame == cent->pe.legs.frame ) // if we got to spawn a footprint
                {
                    int	  footprint = 0;
                    trace_t trace;
                    vec3_t start, end;
                    int frame = cent->pe.legs.frame;


                    //CG_Printf("frame: %i\n",frame);
                    if ( frame == ci->footprintFrameTimer.step_backl ||
                            frame == ci->footprintFrameTimer.step_limpl ||
                            frame == ci->footprintFrameTimer.step_runl ||
                            frame == ci->footprintFrameTimer.step_walkl )
                    {
                        // this means right foot just stepped with right foot.
                        footprint = 1;
                    }
                    else
                        footprint = 2;


                    if ( footprint == 1 )
                        VectorCopy( lfoot, end );

                    if ( footprint == 2 )
                        VectorCopy( rfoot, end );

                    VectorCopy( end,start );

                    start[2] += 15;
                    end[2] -= 50;

                    // measure distance from groundplane to both feet so we can decided
                    // which feet's the closer one
                    CG_Trace( &trace, start, NULL,NULL, end, cent->currentState.clientNum, CONTENTS_SOLID );

                    alpha =  1.0f - (float) ( (float)t / (float)MARK_FOOTPRINT_TIME );

                    if ( footprint > 0 )
                    {
                        vec3_t dir;
                        vec3_t angles;
                        vec3_t forward;

                        angles[ROLL] = 0;
                        angles[YAW] = cent->pe.legs.yawAngle;
                        angles[PITCH] = cent->pe.legs.pitchAngle;

                        AngleVectors( angles, forward, NULL,NULL );

                        VectorCopy( trace.plane.normal, dir );

                        VectorMA( trace.endpos, -1, forward, end );
                        CG_ImpactMark( cgs.media.bulletholes[0][0], end, dir, cent->pe.legs.yawAngle + 180, alpha,alpha,alpha,alpha,qfalse,3 ,qfalse );

                        VectorMA( trace.endpos, 2, forward, end );
                        CG_ImpactMark( cgs.media.bulletholes[0][0], end, dir, cent->pe.legs.yawAngle + 180, alpha,alpha,alpha,alpha,qfalse,3 ,qfalse );

                        // disable it
                        cent->footstepSpawn = 0;

                        //CG_Printf("footprint - t %i alpha %f po %i\n", t,alpha,footprint);
                    }

                }
        }
    }
    CG_AddRefEntityWithPowerups( &legs, &cent->currentState, ci->team );

    // if the model failed, allow the default nullmodel to be displayed
    if (!legs.hModel) {
        return;
    }


    //
    // add the torso
    //
    torso.hModel = ci->torsoModel;

    if ( cent->currentState.powerups & ( 1 << PW_VEST ) )
        torso.customSkin = ci->torsoVestSkin;
    else
        torso.customSkin = ci->torsoSkin;

    if (!torso.hModel) {
        return;
    }

    VectorCopy( cent->lerpOrigin, torso.lightingOrigin );


    CG_PositionRotatedEntityOnTag( &torso, &legs, ci->legsModel, "tag_torso");

    torso.shadowPlane = shadowPlane;
    torso.renderfx = renderfx;

    CG_AddRefEntityWithPowerups( &torso, &cent->currentState, ci->team );

    //
    // add the right arm
    //
    rarm.hModel = ci->rightArmModel;
    rarm.customSkin = ci->torsoSkin;

    if (!rarm.hModel) {
        return;
    }

    VectorCopy( cent->lerpOrigin, rarm.lightingOrigin );

    CG_PositionRotatedEntityOnTag( &rarm, &torso, ci->torsoModel, "tag_armr");

    rarm.shadowPlane = shadowPlane;
    rarm.renderfx = renderfx;

    // copy values from torso
    rarm.oldframe = torso.oldframe;
    rarm.frame = torso.frame;
    rarm.backlerp = torso.backlerp;

    // add it to the current scene
    CG_AddRefEntityWithPowerups( &rarm, &cent->currentState, ci->team );

    //
    // add the left arm
    //
    larm.hModel = ci->leftArmModel;
    larm.customSkin = ci->torsoSkin;

    if (!larm.hModel) {
        return;
    }

    VectorCopy( cent->lerpOrigin, larm.lightingOrigin );

    CG_PositionRotatedEntityOnTag( &larm, &torso, ci->torsoModel, "tag_arml");

    larm.shadowPlane = shadowPlane;
    larm.renderfx = renderfx;

    // copy values from torso
    larm.oldframe = torso.oldframe;
    larm.frame = torso.frame;
    larm.backlerp = torso.backlerp;

    // add it to the current scene
    CG_AddRefEntityWithPowerups( &larm, &cent->currentState, ci->team );


    //
    // add the head
    //
    head.hModel = ci->headModel;
    head.customSkin = ci->headSkin;

    //
    // render a head including a helmet
    //
    if (!head.hModel) {
        return;
    }

    VectorCopy( cent->lerpOrigin, head.lightingOrigin );

    CG_PositionRotatedEntityOnTag( &head, &torso, ci->torsoModel, "tag_head");

    head.shadowPlane = shadowPlane;
    head.renderfx = renderfx;

    CG_AddRefEntityWithPowerups( &head, &cent->currentState, ci->team );

    if ( !(cent->currentState.eFlags & EF_VIP) )
    {
        if (cent->currentState.powerups & ( 1 << PW_HELMET ) )
        {
            head.hModel = CG_GetEquipmentModel(EQ_HELMET, ci->team );
            head.customSkin = 0;
            trap_R_AddRefEntityToScene( &head );
        }
        // only render headequipment if we don't wear a helmet
        else if ( ci->equipmentHead )
        {
            head.hModel = ci->equipmentHead;
            head.customSkin = 0;
            trap_R_AddRefEntityToScene( &head );
        }

        if (ci->equipmentEyes)
        {
            head.hModel = ci->equipmentEyes;
            head.customSkin = 0;
            trap_R_AddRefEntityToScene( &head );
        }
        if (ci->equipmentMouth)
        {
            head.hModel = ci->equipmentMouth;
            head.customSkin = 0;
            trap_R_AddRefEntityToScene( &head );
        }
    }

    {
        int xxx;

        memset( &primary, 0, sizeof( primary ) );

        VectorCopy( cent->lerpOrigin, primary.lightingOrigin );
        primary.shadowPlane = shadowPlane;
        primary.renderfx = renderfx;

        if ( cent->currentState.powerups & ( 1 << PW_BRIEFCASE ) )
        {
            primary.hModel = cgs.media.briefcaseModel_vweap;

            CG_PositionEntityOnTag( &primary, &larm, ci->leftArmModel,"tag_weapon2");
            trap_R_AddRefEntityToScene( &primary );
        }

        for ( xxx = WP_NUM_WEAPONS - 1; xxx > 0; xxx-- )
        {
            if ( cent->currentState.frame & ( 1 << xxx ) )
            {
                weaponInfo_t	*weapon; 

                if ( BG_IsPrimary( xxx ) && cent->currentState.weapon != xxx)
                {
                    weapon = &cg_weapons[ xxx ];

                    primary.hModel = weapon->viewweaponModel;

                    CG_PositionEntityOnTag( &primary, &torso, ci->torsoModel,"tag_vitem1");
                    trap_R_AddRefEntityToScene( &primary );
                }
                if ( BG_IsSecondary( xxx ) && cent->currentState.weapon != xxx )
                {
                    weapon = &cg_weapons[ xxx ];

                    primary.hModel = weapon->viewweaponModel;

                    CG_PositionEntityOnTag( &primary, &legs, ci->legsModel,"tag_vitem2");
                    trap_R_AddRefEntityToScene( &primary );
                }
                /*
                if ( xxx == WP_C4 && cent->currentState.weapon != xxx )
                {
                weapon = &cg_weapons[ xxx ];

                primary.hModel = weapon->viewweaponModel;

                CG_PositionEntityOnTag( &primary, &torso, ci->torsoModel,test_h.string);
                trap_R_AddRefEntityToScene( &primary );
                }
                */
                if ( BG_IsMelee( xxx ) && cent->currentState.weapon != xxx )
                {
                    weapon = &cg_weapons[ xxx ];

                    primary.hModel = weapon->viewweaponModel;

                    CG_PositionEntityOnTag( &primary, &torso, ci->torsoModel,"tag_vitem5");
                    trap_R_AddRefEntityToScene( &primary );
                }
            }
        }
    }

    // if it's our playermodel

    if ( ( cgs.gametype != GT_TEAM ) &&
            cent->currentState.number == cg.snap->ps.clientNum &&
            cg.snap->ps.stats[STAT_HEALTH] <= 0  &&
            !cg.renderingThirdPerson )
    {
        VectorCopy( head.origin , cg.refdef.vieworg );
        AxisCopy( head.axis, cg.refdef.viewaxis );
        // position eye reletive to origin
        VectorCopy( cent->lerpAngles, cg.refdefViewAngles );
    }

#ifdef IS_REDUNDANT
    if ( cent->currentState.number == cg.snap->ps.clientNum && !(cent->currentState.eFlags & EF_DEAD) && !BG_IsZooming( cg.snap->ps.stats[STAT_WEAPONMODE]) &&
            !(cg.snap->ps.pm_flags & PMF_FOLLOW) )
        CG_Draw3rdPersonPlayerCrosshair( cent );
#endif

    //
    // add the gun / barrel / flash
    //
    CG_AddPlayerWeapon( &larm, &rarm, NULL, cent );

    //restore
    if ( pmodel_o.integer > 0 )
        VectorCopy( lO, cent->lerpOrigin );

    CG_DustTrail(cent);

    if ( ci->deferred )
        CG_LoadDeferredPlayers( );

#ifdef NEW_ANIMS
    // add powerups floating behind the player
    CG_PlayerPowerups( cent, &torso );
#endif
}


//=====================================================================

/*
===============
CG_ResetPlayerEntity

A player just came into view or teleported, so reset all animation info
===============
*/
void CG_ResetPlayerEntity( centity_t *cent ) {
    cent->errorTime = -99999;		// guarantee no error decay added
    cent->extrapolated = qfalse;

    CG_ClearLerpFrame( &cgs.clientinfo[ cent->currentState.clientNum ], &cent->pe.legs, cent->currentState.legsAnim );
    CG_ClearLerpFrame( &cgs.clientinfo[ cent->currentState.clientNum ], &cent->pe.torso, cent->currentState.torsoAnim );

    BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
    BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

    VectorCopy( cent->lerpOrigin, cent->rawOrigin );
    VectorCopy( cent->lerpAngles, cent->rawAngles );

    memset( &cent->pe.legs, 0, sizeof( cent->pe.legs ) );
    cent->pe.legs.yawAngle = cent->rawAngles[YAW];
    cent->pe.legs.yawing = qfalse;
    cent->pe.legs.pitchAngle = 0;
    cent->pe.legs.pitching = qfalse;

    memset( &cent->pe.torso, 0, sizeof( cent->pe.legs ) );
    cent->pe.torso.yawAngle = cent->rawAngles[YAW];
    cent->pe.torso.yawing = qfalse;
    cent->pe.torso.pitchAngle = cent->rawAngles[PITCH];
    cent->pe.torso.pitching = qfalse;

    cent->wakemarkTime = 0;
    cent->muzzleFlashTime = 0;

    // BLUTENGEL_XXX:
    // those 2 lines are the fix for the soundbug (ghost sounds giving
    // away player positions)
    if ( cg.nextSnap != NULL && cg.snap != NULL ) {
        cent->interpolate = qtrue;
        cent->previousEvent = cent->nextState.event;
    }

    if ( cg_debugPosition.integer ) {
        CG_Printf("(time %i) %i ResetPlayerEntity yaw=%i, event=(%i, %i)\n", cg.time, cent->currentState.number, cent->pe.torso.yawAngle, (&cent->currentState)->event, (&cent->nextState)->event );
    }
}



