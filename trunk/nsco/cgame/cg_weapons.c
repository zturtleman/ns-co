// Copyright (C) 1999-2000 Id Software, Inc.
// u
//
// cg_weapons.c -- events and effects dealing with weapons

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"



//#include "cg_seals.h"
#define METALSPARK ( WP_NUM_WEAPONS + 1)

#define MACMILLAN_BRASS_EJECTTIME	1000
#define SW629_BRASS_EJECTTIME		1545
extern vmCvar_t cg_smallGuns;

// first shot
qboolean firstshot;

static void CG_EjectBrass( centity_t *cent, vec3_t origin, brasstype_t bType, qboolean firstperson  ) {
    localEntity_t	*le;
    refEntity_t		*re;
    vec3_t			velocity, xvelocity;
    //	vec3_t			offset, xoffset;
    float			waterScale = 1.0f;
    qboolean		left = qfalse;
    vec3_t			v[3];

    if ( cg_brassTime.integer <= 0 ) {
        return;
    }

    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    if ( bType == BRASS_PISTOL || bType == BRASS_PISTOL_LEFT || bType == BRASS_COLT )
        re->hModel = cgs.media.shellPistol;
    else if ( bType == BRASS_RIFLE || bType == BRASS_RIFLE_LEFT)
        re->hModel = cgs.media.shellRifle;
    else if ( bType == BRASS_SHOTGUN || bType == BRASS_SHOTGUN_LEFT)
        re->hModel = cgs.media.shellShotgun;
    else
        return;

    if ( bType == BRASS_PISTOL_LEFT || bType == BRASS_RIFLE_LEFT || bType == BRASS_SHOTGUN_LEFT )
        left = qtrue;

    velocity[0] = 0;
    velocity[1] = -30 + 5 * crandom();
    velocity[2] = 100 + 5 * crandom();
    /// 	velocity[1] = -30 + 20 * crandom();
    //	velocity[2] = 100 + 25 * crandom();

    if ( bType == BRASS_COLT )
        VectorClear( velocity );

    if ( left )
        velocity[1] *= -1;

    le->leType = LE_FRAGMENT;
    le->startTime = cg.time;
    if ( cg_brassTime.integer == -1 )
        le->endTime = le->startTime + 65535;// a buncha
    else
        le->endTime = le->startTime + cg_brassTime.integer ;


    le->pos.trType = TR_GRAVITY;
    le->pos.trTime = cg.time - (rand()&15);


    AnglesToAxis( cent->lerpAngles, v );

    VectorCopy( origin, re->origin );
    VectorCopy( re->origin, le->pos.trBase );

    if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
        waterScale = 0.10f;
    }

    xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
    xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
    xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];


    VectorScale( xvelocity, waterScale, le->pos.trDelta );

    AxisCopy( axisDefault, re->axis );

    le->bounceFactor = 0.4 * waterScale;

    VectorCopy( cent->lerpAngles , le->angles.trBase );

    le->angles.trType = TR_GRAVITY;
    le->angles.trTime = cg.time;

    le->angles.trDuration = 0;

    // let them spin around
    le->angles.trDelta[0] = crandom()*200;
    le->angles.trDelta[1] = crandom()*200;

    // make them look real
    le->leFlags = LEF_TUMBLE;
    le->leBounceSoundType = LEBS_BRASS;
    le->leMarkType = LEMT_NONE;

    if ( firstperson )
    {
        le->leFlags |= LEF_3RDPERSON;

        // "real" physics
        VectorAdd( le->pos.trDelta, cg.snap->ps.velocity, le->pos.trDelta );
    }
}


static void CG_SmokeGrenade( centity_t *ent, const weaponInfo_t *wi )
{
    int		step;
    vec3_t	origin, lastPos;
    int		t;
    int		startTime, contents;
    int		lastContents;
    entityState_t	*es;
    vec3_t	up;
    localEntity_t	*smoke;

    up[0] = random()*2;
    up[1] = random()*2;
    up[2] = random()*10;

    step = 35;

    es = &ent->currentState;
    startTime = ent->trailTime;
    t = step * ( (startTime + step) / step );

    BG_EvaluateTrajectory( &es->pos, cg.time, origin );
    contents = CG_PointContents( origin, -1 );

    BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
    lastContents = CG_PointContents( lastPos, -1 );

    ent->trailTime = cg.time;

    if ( contents & CONTENTS_WATER  ) {
        if ( contents & lastContents & CONTENTS_WATER ) {
            CG_BubbleTrail( lastPos, origin, 4 );
        }
        return;
    }

    for ( ; t <= ent->trailTime ; t += step ) {
        BG_EvaluateTrajectory( &es->pos, t, lastPos );


        smoke = CG_SmokePuff( lastPos, up,
                              300,
                              1, 1, 1, 1,
                              3000,
                              cg.time,
                              0, 0,
                              cgs.media.smokePuffShader );
        // use the optimized local entity add
        smoke->leType = LE_SCALE_FADE;
    }
}

/*
==========================
CG_40mmGrenadeTrail
==========================
*/
void CG_40mmGrenadeTrail( centity_t *ent, const weaponInfo_t *wi ) {
    int		step;
    vec3_t	origin, lastPos;
    int		t;
    int		startTime, contents;
    int		lastContents;
    entityState_t	*es;
    vec3_t	up;
    localEntity_t	*smoke;

    up[0] = 0;
    up[1] = 0;
    up[2] = 0;

    step = 35;

    es = &ent->currentState;
    startTime = ent->trailTime;
    t = step * ( (startTime + step) / step );

    BG_EvaluateTrajectory( &es->pos, cg.time, origin );
    contents = CG_PointContents( origin, -1 );

    // if object (e.g. grenade) is stationary, don't toss up smoke
    if ( es->pos.trType == TR_STATIONARY ) {
        ent->trailTime = cg.time;
        return;
    }

    BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
    lastContents = CG_PointContents( lastPos, -1 );

    ent->trailTime = cg.time;

    if ( contents & CONTENTS_WATER ) {
        if ( contents & lastContents & CONTENTS_WATER ) {
            CG_BubbleTrail( lastPos, origin, 4 );
        }
        return;
    }

    for ( ; t <= ent->trailTime ; t += step ) {
        BG_EvaluateTrajectory( &es->pos, t, lastPos );


        smoke = CG_SmokePuff( lastPos, up,
                              wi->trailRadius,
                              1, 1, 1, 0.33f,
                              wi->wiTrailTime,
                              cg.time,
                              0, 0,
                              cgs.media.smokePuffShader );
        // use the optimized local entity add
        smoke->leType = LE_SCALE_FADE;
    }

}
#if 0
/*
==========================
CG_GrenadeTrail
==========================
*/
static void CG_GrenadeTrail( centity_t *ent, const weaponInfo_t *wi ) {
    CG_RocketTrail( ent, wi );
}
#endif


/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapon( int weaponNum )
{
    weaponInfo_t	*weaponInfo;
    gitem_t			*item;
    char			path[MAX_QPATH];
    //	vec3_t			mins, maxs;
    int				i;

    weaponInfo = &cg_weapons[weaponNum];

    if ( weaponNum == 0 ) {
        return;
    }

    if ( weaponInfo->registered ) {
        return;
    }

    memset( weaponInfo, 0, sizeof( *weaponInfo ) );
    weaponInfo->registered = qtrue;

    for ( item = bg_itemlist + 1 ; item->classname ; item++ ) {
        if ( item->giType == IT_WEAPON && item->giTag == weaponNum ) {
            weaponInfo->item = item;
            break;
        }
    }
    if ( !item->classname ) {
        CG_Error( "Couldn't find weapon %i", weaponNum );
    }
    CG_RegisterItemVisuals( item - bg_itemlist );

    // load cmodel before model so filecache works 
    weaponInfo->weaponIcon = trap_R_RegisterShader( item->icon );  
    weaponInfo->reloadSound = 0;
    weaponInfo->reloadEmptySound = 0;
    weaponInfo->ejectBrassNum = 0;
    weaponInfo->kickBack = 2;
 
    /* ==========
    Muzzle Flash
    ==========
    */
    if ( BG_IsPrimary( weaponNum) || BG_IsSecondary( weaponNum ) && weaponNum != WP_SL8SD )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_flash.md3" );
        weaponInfo->flashModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->flashModel ) {
            // new default ;
            CG_Printf("Couldn't register: Flash Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    Scope
    ==========
    */
    if ( BG_WeaponMods( weaponNum ) & ( 1 << WM_SCOPE ) )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_scope.md3" );
        weaponInfo->scopeModel = trap_R_RegisterPermanentModel( path );
        // vweap
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_scope_vweap.md3" );
        weaponInfo->v_scopeModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->scopeModel  ) {
            // new default ;
            CG_Printf("Couldn't register: Scope Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    Laser sight Model
    ==========
    */
    if ( BG_WeaponMods( weaponNum ) & ( 1 << WM_LASER ) )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_laser.md3" );
        weaponInfo->lasersightModel = trap_R_RegisterPermanentModel( path );
        // vweap
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_laser_vweap.md3" );
        weaponInfo->v_lasersightModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->lasersightModel  ) {
            // new default ;
            CG_Printf("Couldn't register: Lasersight Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    Silencer Model
    ==========
    */
    if ( BG_WeaponMods( weaponNum ) & ( 1 << WM_SILENCER ) )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_silencer.md3" );
        weaponInfo->silencerModel = trap_R_RegisterPermanentModel( path );
        // vweap
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_silencer_vweap.md3" );
        weaponInfo->v_silencerModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->silencerModel ) {
            // new default ;
            CG_Printf("Couldn't register: Silencer Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    Flashlight Model
    ==========
    */
    if ( BG_WeaponMods( weaponNum ) & ( 1 << WM_FLASHLIGHT ) )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_flashlight.md3" );
        weaponInfo->glModel = trap_R_RegisterPermanentModel( path );
        // vweap
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_flashlight_vweap.md3" );
        weaponInfo->v_glModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->glModel ) {
            // new default ;
            CG_Printf("Couldn't register: Flashlight Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    Duckbill Model
    ==========
    */
    if ( BG_WeaponMods( weaponNum ) & ( 1 << WM_DUCKBILL ) )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_duckbill.md3" );
        weaponInfo->silencerModel = trap_R_RegisterPermanentModel( path );
        // vweap
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_duckbill_vweap.md3" );
        weaponInfo->v_silencerModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->silencerModel ) {
            // new default ;
            CG_Printf("Couldn't register: Duckbill Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    Bayonet Model
    ==========
    */
    if ( BG_WeaponMods( weaponNum ) & ( 1 << WM_BAYONET ) )
    {

        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_bayonet.md3" );
        weaponInfo->bayonetModel = trap_R_RegisterPermanentModel( path );
        // vweap
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_bayonet_vweap.md3" );
        weaponInfo->v_bayonetModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->bayonetModel ) {
            // new default ;
            CG_Printf("Couldn't register: Bayonet Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    GrenadeLauncher Model
    ==========
    */

    if ( BG_WeaponMods( weaponNum ) & ( 1 << WM_GRENADELAUNCHER ) )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        if ( item->giTag == WP_M4 )
            strcat( path, "_m203_1stperson.md3" );
        else
            strcat( path, "_bg15_1stperson.md3" );

        weaponInfo->glModel  = trap_R_RegisterPermanentModel( path );

        // vweap
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        if ( item->giTag == WP_M4 )
            strcat( path, "_m203_vweap.md3" );
        else
            strcat( path, "_bg15_vweap.md3" );

        weaponInfo->v_glModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->glModel ) {
            // new default ;
            CG_Printf("Couldn't register: GrenadeLauncher Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }



    /* ==========
    1st Person Model [ Weapon ]
    ==========
    */
    if ( weaponNum != WP_C4 )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_1stperson_body.md3" );
        weaponInfo->weaponModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->weaponModel ) {
            CG_Printf("Couldn't register: 1rd Person Model[WEAPON] (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    1st Person Model [ Hands ]
    ==========
    */
    strcpy( path, item->world_model[0] );
    COM_StripExtension( path, path );
    strcat( path, "_1stperson.md3" );
    weaponInfo->handsModel = trap_R_RegisterPermanentModel( path );

    if ( !weaponInfo->handsModel ) {
        CG_Printf("Couldn't register: 1rd Person Model (for: %s)\n", weaponInfo->item->pickup_name);
    }

    /* ==========
    1st Person Model Skins [ Tango Hand Skin]
    ==========
    */
    if ( !cg_disableTangoHandSkin.integer )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_t.skin" );
        weaponInfo->t_viewweaponSkin = trap_R_RegisterSkin( path );

        if ( !weaponInfo->t_viewweaponSkin ) {
            CG_Printf("Couldn't register Tango Hand Skin for: %s\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    View Weapon
    ==========
    */
    strcpy( path, item->world_model[0] );
    COM_StripExtension( path, path );
    strcat( path, "_vweap.md3" );
    weaponInfo->viewweaponModel  = trap_R_RegisterPermanentModel( path );

    if ( !weaponInfo->viewweaponModel ) {
        CG_Printf("Couldn't register: View Weapon Model (for: %s)\n", weaponInfo->item->pickup_name);
    }

    // calc midpoint for rotation
    /*	trap_R_ModelBounds( weaponInfo->viewweaponModel, mins, maxs );
    for ( i = 0 ; i < 3 ; i++ ) {
    weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * ( maxs[i] - mins[i] );
    }
    */
    if ( !cgs.media.bulletExplosionShader )
        cgs.media.bulletExplosionShader = trap_R_RegisterShader( "bulletExplosion" );

    weaponInfo->partTags[0] = "";

    switch ( weaponNum ) {
    case WP_SEALKNIFE:
    case WP_KHURKURI:
        //		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/melee/fstrun.wav" );
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/melee/knifeslash1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/melee/knifeslash2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/melee/knifeslash3.wav",qfalse );
        break;
        // NS +
    case WP_MACMILLAN:
        MAKERGB( weaponInfo->flashDlightColor, 1, 1, 1 );
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/macmillan/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/macmillan/fire2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/macmillan/fire3.wav",qfalse );

        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/macmillan/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/macmillan/boltaction.wav",qfalse );

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "";

        weaponInfo->ejectBrassType = BRASS_RIFLE;
        weaponInfo->ejectBrassNum = 1;
        weaponInfo->kickBack = 6;
        break;
#ifdef SL8SD
    case WP_SL8SD: // WP_SL8SD
        MAKERGB( weaponInfo->flashDlightColor, 0.9f, 0.9f, 0.9f );
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/sl8sd/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/sl8sd/fire2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/sl8sd/fire3.wav",qfalse );
        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/sl8sd/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/sl8sd/reload_empty.wav",qfalse );

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[3] = "mageject";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[5] = "cock2";
        weaponInfo->partTags[4] = "";


        weaponInfo->ejectBrassType = BRASS_RIFLE;
        weaponInfo->ejectBrassNum = 1;
        weaponInfo->kickBack = 4;
        break;
#endif
    case WP_PSG1:
        MAKERGB( weaponInfo->flashDlightColor, 0.9f, 0.9f, 0.9f );
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/psg-1/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/psg-1/fire2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/psg-1/fire3.wav",qfalse );
        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/psg-1/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/psg-1/reload_empty.wav",qfalse );

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = "";


        weaponInfo->ejectBrassType = BRASS_RIFLE;
        weaponInfo->ejectBrassNum = 1;
        weaponInfo->kickBack = 6;
        break;
    case WP_MAC10:
        MAKERGB( weaponInfo->flashDlightColor, 0.8f, 0.8f, 0.7f );
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/mac10/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/mac10/fire2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/mac10/fire3.wav",qfalse );

        weaponInfo->sil_flashSound[0] = trap_S_RegisterSound( "sound/weapons/mac10/fire1_sil.wav",qfalse );
        weaponInfo->sil_flashSound[1] = trap_S_RegisterSound( "sound/weapons/mac10/fire2_sil.wav",qfalse );
        weaponInfo->sil_flashSound[2] = trap_S_RegisterSound( "sound/weapons/mac10/fire3_sil.wav",qfalse );

        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/mac10/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/mac10/reload_empty.wav",qfalse );

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = "";

        weaponInfo->ejectBrassType = BRASS_PISTOL;
        weaponInfo->ejectBrassNum = 1;
        weaponInfo->kickBack = 2;
        break;
    case WP_PDW:
        MAKERGB( weaponInfo->flashDlightColor, 0.7f, 0.7f, 0.7f );
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/pdw/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/pdw/fire2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/pdw/fire3.wav",qfalse );
        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/pdw/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/pdw/reload_empty.wav",qfalse );
        weaponInfo->ejectBrassType = BRASS_PISTOL;
        weaponInfo->ejectBrassNum = 1;
        weaponInfo->kickBack = 3;

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = "catcher";
        weaponInfo->partTags[4] = "";


        break;
    case WP_MP5:
        MAKERGB( weaponInfo->flashDlightColor, 0.7f, 0.7f, 0.7f );
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/mp5/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/mp5/fire2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/mp5/fire3.wav",qfalse );

        weaponInfo->sil_flashSound[0] = trap_S_RegisterSound( "sound/weapons/mp5/fire1_sil.wav",qfalse );
        weaponInfo->sil_flashSound[1] = trap_S_RegisterSound( "sound/weapons/mp5/fire2_sil.wav",qfalse );
        weaponInfo->sil_flashSound[2] = trap_S_RegisterSound( "sound/weapons/mp5/fire3_sil.wav",qfalse );

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = "";

        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/mp5/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/mp5/reload_empty.wav",qfalse );
        weaponInfo->ejectBrassType = BRASS_PISTOL;
        weaponInfo->ejectBrassNum = 1;
        weaponInfo->kickBack = 2;
        break;
    case WP_AK47:
        MAKERGB( weaponInfo->flashDlightColor, 0.9f, 0.7f, 0.7f );
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/ak47/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/ak47/fire2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/ak47/fire3.wav",qfalse );

        weaponInfo->otherflashSound[0] = trap_S_RegisterSound( "sound/weapons/ak47/gl.wav",qfalse );

        weaponInfo->sil_flashSound[0] = weaponInfo->flashSound[0];
        weaponInfo->sil_flashSound[1] = weaponInfo->flashSound[1];
        weaponInfo->sil_flashSound[2] = weaponInfo->flashSound[2];

        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/ak47/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/ak47/reload_empty.wav",qfalse );
        weaponInfo->ejectBrassType = BRASS_RIFLE;
        weaponInfo->ejectBrassNum = 1;

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = "gl";
        weaponInfo->partTags[4] = "";

        weaponInfo->missileModel = trap_R_RegisterPermanentModel( "models/misc/bullets/bullet_40mm.md3" ); 
        weaponInfo->wiTrailTime = 200;
        weaponInfo->missileDlight = 0;
        weaponInfo->trailRadius = 4.5f;

        weaponInfo->kickBack = 3;

        break;
    case WP_M14:
        MAKERGB( weaponInfo->flashDlightColor, 0.90f, 0.80f, 0.80f );
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/m14/fire1.wav", qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/m14/fire2.wav", qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/m14/fire3.wav", qfalse ),
                                    weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/m14/reload.wav", qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/m14/reload_empty.wav", qfalse );

        weaponInfo->partTags[0] = "cock";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "";

        weaponInfo->kickBack = 4;
        weaponInfo->ejectBrassType = BRASS_RIFLE;
        weaponInfo->ejectBrassNum = 1;
        break;

    case WP_M249:
        MAKERGB( weaponInfo->flashDlightColor, 0.85f, 0.85f, 0.85f );

        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/m249/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/m249/fire2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/m249/fire3.wav",qfalse );

        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/m249/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/m249/reload_empty.wav",qfalse );

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "lid";
        weaponInfo->partTags[3] = "bullet";
        weaponInfo->partTags[4] = "belt";
        weaponInfo->partTags[5] = "";

        weaponInfo->kickBack = 4;

        weaponInfo->ejectBrassType = BRASS_RIFLE;
        weaponInfo->ejectBrassNum = 1;
        break;
    case WP_M4:
        MAKERGB( weaponInfo->flashDlightColor, 0.85f, 0.85f, 0.85f );

        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/m4/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/m4/fire2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/m4/fire3.wav",qfalse );

        weaponInfo->sil_flashSound[0] = weaponInfo->flashSound[0];
        weaponInfo->sil_flashSound[1] = weaponInfo->flashSound[1];
        weaponInfo->sil_flashSound[2] = weaponInfo->flashSound[2];

        weaponInfo->otherflashSound[0] = trap_S_RegisterSound( "sound/weapons/m4/gl.wav",qfalse );

        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/m4/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/m4/reload_empty.wav",qfalse );

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "m203gren";
        weaponInfo->partTags[3] = "m203rel";
        weaponInfo->partTags[4] = "m203";
        weaponInfo->partTags[5] = "handle";
        weaponInfo->partTags[6] = "";


        weaponInfo->missileModel = trap_R_RegisterPermanentModel( "models/misc/bullets/bullet_40mm.md3" ); 
        weaponInfo->wiTrailTime = 1000;
        weaponInfo->missileDlight = 0;
        weaponInfo->trailRadius = 20.0f;

        weaponInfo->kickBack = 3;

        weaponInfo->ejectBrassType = BRASS_RIFLE;
        weaponInfo->ejectBrassNum = 1;
        break;


    case WP_GLOCK:
        MAKERGB( weaponInfo->flashDlightColor, 0.7f, 0.7f, 0.7f );

        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/glock30/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/glock30/fire2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/glock30/fire3.wav",qfalse );

        weaponInfo->sil_flashSound[0] = trap_S_RegisterSound( "sound/weapons/glock30/fire1_sil.wav",qfalse );
        weaponInfo->sil_flashSound[1] = trap_S_RegisterSound( "sound/weapons/glock30/fire2_sil.wav",qfalse );

        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/glock30/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/glock30/reload_empty.wav",qfalse );
        weaponInfo->ejectBrassType = BRASS_PISTOL;
        weaponInfo->ejectBrassNum = 1;

        weaponInfo->kickBack = 2;

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = "sled";
        weaponInfo->partTags[4] = "";


        break;
    case WP_P9S:
        MAKERGB( weaponInfo->flashDlightColor, 0.7f, 0.7f, 0.7f );
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/p9s/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/p9s/fire2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/p9s/fire3.wav",qfalse );

        weaponInfo->sil_flashSound[0] = trap_S_RegisterSound( "sound/weapons/glock30/fire1_sil.wav",qfalse );
        weaponInfo->sil_flashSound[1] = trap_S_RegisterSound( "sound/weapons/glock30/fire2_sil.wav",qfalse );

        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/p9s/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/p9s/reload_empty.wav",qfalse );
        weaponInfo->ejectBrassType = BRASS_PISTOL;
        weaponInfo->ejectBrassNum = 1;

        weaponInfo->kickBack = 2;

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = "sled";
        weaponInfo->partTags[4] = "";
        break;
    case WP_MK23:
        MAKERGB( weaponInfo->flashDlightColor, 0.8f, 0.8f, 0.8f);
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/mk23/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/mk23/fire2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/mk23/fire3.wav",qfalse );

        weaponInfo->sil_flashSound[0] = trap_S_RegisterSound( "sound/weapons/mk23/fire1_sil.wav",qfalse );
        weaponInfo->sil_flashSound[1] = trap_S_RegisterSound( "sound/weapons/mk23/fire2_sil.wav",qfalse );

        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/mk23/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/mk23/reload_empty.wav",qfalse );
        weaponInfo->ejectBrassType = BRASS_PISTOL;
        weaponInfo->ejectBrassNum = 1;

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "bolt";
        weaponInfo->partTags[3] = "sled";
        weaponInfo->partTags[4] = "";

        weaponInfo->kickBack = 3;
        break;
    case WP_SW629:
        MAKERGB( weaponInfo->flashDlightColor, 0.9f, 0.6f, 0.6f );
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/sw629/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/sw629/fire2.wav",qfalse );
        weaponInfo->ejectBrassType = BRASS_COLT;

        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/sw629/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/sw629/reload.wav",qfalse );

        weaponInfo->partTags[0] = "barrel";
        weaponInfo->partTags[1] = "bolt";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = "mag";
        weaponInfo->partTags[4] = "sled";
        weaponInfo->partTags[5] = "";


        weaponInfo->kickBack = 3;
        break;

    case WP_DEAGLE:
        MAKERGB( weaponInfo->flashDlightColor, 0.8f, 0.7f, 0.7f );
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/deagle/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/deagle/fire2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/deagle/fire3.wav",qfalse );

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "sled";
        weaponInfo->partTags[3] = "";

        weaponInfo->ejectBrassType = BRASS_PISTOL;
        weaponInfo->ejectBrassNum = 1;
        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/deagle/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/deagle/reload_empty.wav",qfalse );

        weaponInfo->kickBack = 4;
        break;
    case WP_SW40T:
        MAKERGB( weaponInfo->flashDlightColor, 0.8f, 0.8f, 0.8f );
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/sw40t/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/sw40t/fire2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/sw40t/fire3.wav",qfalse );

        weaponInfo->sil_flashSound[0] = trap_S_RegisterSound( "sound/weapons/sw40t/fire1_sil.wav",qfalse );
        weaponInfo->sil_flashSound[1] = trap_S_RegisterSound( "sound/weapons/sw40t/fire2_sil.wav",qfalse );
        weaponInfo->sil_flashSound[2] = trap_S_RegisterSound( "sound/weapons/sw40t/fire3_sil.wav",qfalse );
        weaponInfo->ejectBrassType = BRASS_PISTOL;
        weaponInfo->ejectBrassNum = 1;

        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/sw40t/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/sw40t/reload_empty.wav",qfalse );

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = "sled";
        weaponInfo->partTags[4] = "";

        weaponInfo->kickBack = 3;
        break;
    case WP_SPAS15:
        MAKERGB( weaponInfo->flashDlightColor, 0.8f, 0.8f, 0.8f );
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/spas15/fire1.wav",qfalse );
        weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/spas15/fire2.wav",qfalse );
        weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/spas15/fire3.wav",qfalse );
        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/spas15/reload.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/spas15/reload_empty.wav",qfalse );
        weaponInfo->ejectBrassType = BRASS_SHOTGUN;
        weaponInfo->ejectBrassNum = 1;

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = "";


        weaponInfo->kickBack = 6;
        break;
    case WP_M590:
        MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0.7f );
        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/870/fire1.wav",qfalse );
        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/870/shellin.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/870/cock.wav",qfalse );
        weaponInfo->ejectBrassType = BRASS_SHOTGUN_LEFT;
        weaponInfo->ejectBrassNum = 1;

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "pump";
        weaponInfo->partTags[2] = "mag";
        weaponInfo->partTags[3] = "";

        weaponInfo->kickBack = 4;
        break;
    case WP_870:
        MAKERGB( weaponInfo->flashDlightColor, 1, 0.7f, 0.7f );

        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "pump";
        weaponInfo->partTags[2] = "";

        weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/870/fire1.wav",qfalse );
        weaponInfo->reloadSound = trap_S_RegisterSound( "sound/weapons/870/shellin.wav",qfalse );
        weaponInfo->reloadEmptySound = trap_S_RegisterSound( "sound/weapons/870/cock.wav",qfalse );
        weaponInfo->ejectBrassType = BRASS_SHOTGUN_LEFT;
        weaponInfo->ejectBrassNum = 1;

        weaponInfo->kickBack = 4;
        break;
        // NS -
    case WP_FLASHBANG:
        weaponInfo->missileModel = trap_R_RegisterPermanentModel( "models/weapons/flashbang/flashbang_thrown.md3" );
        weaponInfo->kickBack = 0;

        weaponInfo->partTags[0] = "pin";
        weaponInfo->partTags[1] = "";

        break;
    case WP_GRENADE:
        weaponInfo->missileModel = trap_R_RegisterPermanentModel( "models/weapons/mk26/mk26_thrown.md3" );
        weaponInfo->kickBack = 0;

        weaponInfo->partTags[0] = "pin";
        weaponInfo->partTags[1] = "";
        break;
    case WP_SMOKE:
        weaponInfo->missileModel = trap_R_RegisterPermanentModel( "models/weapons/smoke/smoke_thrown.md3" );
        weaponInfo->kickBack = 0;

        weaponInfo->partTags[0] = "pin";
        weaponInfo->partTags[1] = "";
        break;


    default:
        weaponInfo->kickBack = 0;
        MAKERGB( weaponInfo->flashDlightColor, 1, 1, 1 );
        break;
    }

    i = 0;

    while ( strlen(weaponInfo->partTags[i]) > 0 && i < MAX_WEAPONPARTS )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );

        if ( !Q_stricmp( "handle", weaponInfo->partTags[i] ) ) {
            weaponInfo->v_flashModel = trap_R_RegisterPermanentModel( va("%s_handle_vweap.md3",path) );
            strcat( path, va("_%s.md3", weaponInfo->partTags[i] )  );
        }
        else
            strcat( path, va("_1stperson_%s.md3", weaponInfo->partTags[i] )  );

        weaponInfo->weaponParts[i] = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->partTags[i]  /*|| 1*/ ) {

            if ( !weaponInfo->partTags[i] )
                Com_Error( ERR_DROP, "Couldn't register Modelpart %s (for: %s)\n", path, weaponInfo->item->pickup_name);
        }
        i++;
    }
}

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals( int itemNum ) {
    itemInfo_t		*itemInfo;
    gitem_t			*item;

    itemInfo = &cg_items[ itemNum ];
    if ( itemInfo->registered ) {
        return;
    }

    item = &bg_itemlist[ itemNum ];

    memset( itemInfo, 0, sizeof( &itemInfo ) );
    itemInfo->registered = qtrue;

    if ( item->giType != IT_WEAPON )
        itemInfo->models[0] = trap_R_RegisterModel( item->world_model[0] );

    itemInfo->icon = trap_R_RegisterShader( item->icon );  
}


/*
========================================================================================

VIEW WEAPON

========================================================================================
*/

void CG_Spark( vec3_t org, vec3_t dir, float width );

/*
==============
CG_CalculateWeaponPosition
==============
*/
extern vmCvar_t	cg_isgun_z;
extern vmCvar_t	cg_isgun_y;
extern vmCvar_t	cg_isgun_x;
extern vmCvar_t	cg_isgun_roll;
extern vmCvar_t	cg_isgun_yaw;
extern vmCvar_t	cg_isgun_pitch;
extern vmCvar_t	cg_isgun_step;
extern vmCvar_t	cg_weaponYaw;
extern vmCvar_t	cg_weaponPitch;
extern vmCvar_t	cg_weaponRoll;

void CG_CalculateWeaponPosition( vec3_t origin, vec3_t angles ) {
    float		scale;
    int			delta;
    float		fracsin;
    vec3_t		forward,right,up;
    vec3_t		end;
    trace_t		trace;
    //	qboolean	ladder = qfalse;

    VectorCopy( cg.refdef.vieworg, origin );
    VectorCopy( cg.weaponAngles, angles );

    AngleVectors( angles, forward,right,up );

    VectorMA( origin, 50, forward, end );

    // trace a line from previous position to new position
    CG_Trace( &trace, origin, NULL, NULL, end, cg.predictedPlayerState.clientNum, CONTENTS_SOLID );

    if (trace.fraction < 1 && cg.weaponPos > 0.0f ) {
        if ( cg.weaponPos > trace.fraction )
            cg.weaponPos -= 0.1f;

        if ( cg.weaponPos < trace.fraction )
            cg.weaponPos = trace.fraction;
    } else if ( cg.weaponPos < 1.0f && trace.fraction > cg.weaponPos )
        // get close if we're close
        cg.weaponPos += 0.1f;

    if (cg.weaponPos > 1.0f) cg.weaponPos = 1.0;
    if (cg.weaponPos < 0.0f) cg.weaponPos = 0.0;

    if (cg.weaponPos != 1.0f)
    {
        // BLUTENGEL:
        // here seems to be the part of the code where the weapon
        // drawback is calculated. I reduced the weapondrawback from
        // primary weapons from a maximum of -10 to a maximum of -5
        switch (cg.snap->ps.weapon) {
        case WP_PSG1:
        case WP_MACMILLAN:
        case WP_SL8SD:
            VectorMA( origin, -1 + cg.weaponPos*1, forward, origin);
            break;
        case WP_P9S:
        case WP_GLOCK:
        case WP_MK23:
        case WP_SW40T:
        case WP_DEAGLE:
        case WP_SW629:
            VectorMA( origin, -2 + cg.weaponPos*2, forward, origin);
            break;
        default:
            VectorMA( origin, -3 + cg.weaponPos*3, forward, origin );
        }
    }


    // on odd legs, invert some angles
    if ( cg.bobcycle & 1 ) {
        scale = -cg.xyspeed;
    } else {
        scale = cg.xyspeed;
    }

    // gun angles from bobbing
    angles[ROLL] += scale * cg.bobfracsin * 0.005 * cg_weaponRoll.value;
    angles[YAW] += scale * cg.bobfracsin * 0.01 * cg_weaponYaw.value;
    angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.005 * cg_weaponPitch.value;

    // drop the weapon when landing
    delta = cg.time - cg.landTime;
    if ( delta < LAND_DEFLECT_TIME ) {
        origin[2] += cg.landChange*0.25 * delta / LAND_DEFLECT_TIME;
    } else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
        origin[2] += cg.landChange*0.25 *
                     (LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
    }

#if 1
    // drop the weapon when stair climbing
    delta = cg.time - cg.stepTime;
    if ( delta < STEP_TIME/2 ) {
        origin[2] -= cg.stepChange*0.15 * delta / (STEP_TIME/2);
    } else if ( delta < STEP_TIME ) {
        origin[2] -= cg.stepChange*0.15 * (STEP_TIME - delta) / (STEP_TIME/2);
    }
#endif


    if ( cg.ns_ironsightState )
    {
        if ( cg.ns_ironsightX < cg_isgun_x.value )
            cg.ns_ironsightX += cg_isgun_step.value;
        else if ( cg.ns_ironsightX > cg_isgun_x.value )
            cg.ns_ironsightX -= cg_isgun_step.value;

        if ( cg.ns_ironsightY < cg_isgun_y.value )
            cg.ns_ironsightY += cg_isgun_step.value;
        else if ( cg.ns_ironsightY > cg_isgun_y.value )
            cg.ns_ironsightY -= cg_isgun_step.value;

        if ( cg.ns_ironsightZ < cg_isgun_z.value )
            cg.ns_ironsightZ += cg_isgun_step.value;
        else if ( cg.ns_ironsightZ > cg_isgun_z.value )
            cg.ns_ironsightZ -= cg_isgun_step.value;

        // new gun angles.
        if ( cg.ns_ironsightAngles[PITCH] < cg_isgun_pitch.value )
            cg.ns_ironsightAngles[PITCH] += cg_isgun_step.value;
        else if ( cg.ns_ironsightAngles[PITCH] > cg_isgun_pitch.value )
            cg.ns_ironsightAngles[PITCH] -= cg_isgun_step.value;

        if ( cg.ns_ironsightAngles[YAW] < cg_isgun_yaw.value )
            cg.ns_ironsightAngles[YAW] += cg_isgun_step.value;
        else if ( cg.ns_ironsightAngles[YAW] > cg_isgun_yaw.value )
            cg.ns_ironsightAngles[YAW] -= cg_isgun_step.value;

        if ( cg.ns_ironsightAngles[ROLL] < cg_isgun_roll.value )
            cg.ns_ironsightAngles[ROLL] += cg_isgun_step.value;
        else if ( cg.ns_ironsightAngles[ROLL] > cg_isgun_roll.value )
            cg.ns_ironsightAngles[ROLL] -= cg_isgun_step.value;


        VectorMA( origin, cg.ns_ironsightX , forward, origin );
        VectorMA( origin, cg.ns_ironsightY , right, origin );
        VectorMA( origin, cg.ns_ironsightZ , up, origin );

        angles[YAW] += cg.ns_ironsightAngles[YAW];
        angles[ROLL] += cg.ns_ironsightAngles[ROLL];
        angles[PITCH] += cg.ns_ironsightAngles[PITCH];


        //	CG_Printf("IS: %f %f %f\n", cg.ns_ironsightX,cg.ns_ironsightY, cg.ns_ironsightZ );
    }
    else // idle drift
    {

        if (cg.xyspeed == 0) {
            scale = cg.xyspeed + 40;
            fracsin = sin( cg.time * 0.001 );
            angles[ROLL] += scale * fracsin * 0.01;
            angles[YAW] += scale * fracsin * 0.01;
            angles[PITCH] += scale * fracsin * 0.01;
        }

        if ( cg.ns_ironsightX != 0.0f || cg.ns_ironsightY != 0.0f || cg.ns_ironsightZ != 0.0f )
        {

            if ( cg.ns_ironsightAngles[PITCH] != 0.0f && cg.ns_ironsightAngles[PITCH] >= cg_isgun_step.value )
                cg.ns_ironsightAngles[PITCH] -= cg_isgun_step.value;
            else if ( cg.ns_ironsightAngles[PITCH] != 0.0f && cg.ns_ironsightAngles[PITCH] <= -cg_isgun_step.value )
                cg.ns_ironsightAngles[PITCH] += cg_isgun_step.value;

            if ( cg.ns_ironsightAngles[YAW] != 0.0f && cg.ns_ironsightAngles[YAW] >= cg_isgun_step.value )
                cg.ns_ironsightAngles[YAW] -= cg_isgun_step.value;
            else if ( cg.ns_ironsightAngles[YAW] != 0.0f && cg.ns_ironsightAngles[YAW] <= -cg_isgun_step.value )
                cg.ns_ironsightAngles[YAW] += cg_isgun_step.value;

            if ( cg.ns_ironsightAngles[ROLL] != 0.0f && cg.ns_ironsightAngles[ROLL] >= cg_isgun_step.value )
                cg.ns_ironsightAngles[ROLL] -= cg_isgun_step.value;
            else if ( cg.ns_ironsightAngles[ROLL] != 0.0f && cg.ns_ironsightAngles[ROLL] <= -cg_isgun_step.value )
                cg.ns_ironsightAngles[ROLL] += cg_isgun_step.value;

            if ( cg.ns_ironsightX >= cg_isgun_step.value )
                cg.ns_ironsightX -= cg_isgun_step.value;
            else if ( cg.ns_ironsightX <= -cg_isgun_step.value )
                cg.ns_ironsightX += cg_isgun_step.value;

            if ( cg.ns_ironsightY >= cg_isgun_step.value )
                cg.ns_ironsightY -= cg_isgun_step.value;
            else if ( cg.ns_ironsightY <= -cg_isgun_step.value )
                cg.ns_ironsightY += cg_isgun_step.value;

            if ( cg.ns_ironsightZ >= cg_isgun_step.value )
                cg.ns_ironsightZ -= cg_isgun_step.value;
            else if ( cg.ns_ironsightZ <= -cg_isgun_step.value )
                cg.ns_ironsightZ += cg_isgun_step.value;

            VectorMA( origin, cg.ns_ironsightX , forward, origin );
            VectorMA( origin, cg.ns_ironsightY , right, origin );
            VectorMA( origin, cg.ns_ironsightZ , up, origin );

            angles[YAW] += cg.ns_ironsightAngles[YAW];
            angles[ROLL] += cg.ns_ironsightAngles[ROLL];
            angles[PITCH] += cg.ns_ironsightAngles[PITCH];

        }

    }


    if ( cg.snap->ps.pm_flags & PMF_CLIMB ) {

        if ( cg.ladderWeaponTime < 35 )
            cg.ladderWeaponTime +=2;

    } else if ( cg.ladderWeaponTime > 0 )
        cg.ladderWeaponTime-=2;

    VectorMA( origin, -( (float)(cg.ladderWeaponTime) / 1.0f ), up, origin );

#if DEBUG_BUILD
    if ( cg.ladderWeaponTime || cg.snap->ps.pm_flags & PMF_CLIMB)
        CG_Printf("laddertime: %f origin: %s\n",(float)(cg.ladderWeaponTime) , vtos(origin) );
#endif

}

/*
==============
CG_CalculateWeaponPosition
==============
*/
void CG_CalculateWeaponPosition2( vec3_t origin, vec3_t angles ) {
    float	scale;
    int		delta;
    float	fracsin;


    // on odd legs, invert some angles
    if ( cg.bobcycle & 1 ) {
        scale = -cg.xyspeed;
    } else {
        scale = cg.xyspeed;
    }

    // gun angles from bobbing
    angles[ROLL] += scale * cg.bobfracsin * 0.005;
    angles[YAW] += scale * cg.bobfracsin * 0.01;
    angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.005;

    // drop the weapon when landing
    delta = cg.time - cg.landTime;
    if ( delta < LAND_DEFLECT_TIME ) {
        origin[2] += cg.landChange*0.25 * delta / LAND_DEFLECT_TIME;
    } else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
        origin[2] += cg.landChange*0.25 *
                     (LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
    }

#if 1
    // drop the weapon when stair climbing
    delta = cg.time - cg.stepTime;
    if ( delta < STEP_TIME/2 ) {
        origin[2] -= cg.stepChange*0.25 * delta / (STEP_TIME/2);
    } else if ( delta < STEP_TIME ) {
        origin[2] -= cg.stepChange*0.25 * (STEP_TIME - delta) / (STEP_TIME/2);
    }
#endif

    // idle drift
    scale = cg.xyspeed + 40;
    fracsin = sin( cg.time * 0.001 );
    angles[ROLL] += scale * fracsin * 0.01;
    angles[YAW] += scale * fracsin * 0.01;
    angles[PITCH] += scale * fracsin * 0.01;
}



/*
========================
CG_AddWeaponWithPowerups

This is used for worldmodel rendering.

Very sad that we have to do so many hacks to make ns-co run on the limited resources we got
========================
*/
void CG_AddWeaponWithPowerups( refEntity_t gun, weaponInfo_t	*weapon,int i_equipment, int eFlags )
{
    refEntity_t equipment;

    // fix me: move this into addweapon w/p
    if ( eFlags & EF_SILENCED )
    {
        memset( &equipment, 0, sizeof( equipment ) );
        VectorCopy( gun.lightingOrigin, equipment.lightingOrigin );
        equipment.shadowPlane = gun.shadowPlane;
        equipment.renderfx = gun.renderfx;

        equipment.hModel = weapon->v_silencerModel;

        if ( gun.customSkin )
            equipment.customSkin = gun.customSkin;

        CG_PositionEntityOnTag( &equipment, &gun, weapon->viewweaponModel,"tag_flash");

        if ( equipment.hModel )
            trap_R_AddRefEntityToScene( &equipment );
    }

    // add powerup effects
    if ( i_equipment & ( 1 << PW_DUCKBILL ) )
    {
        memset( &equipment, 0, sizeof( equipment ) );
        VectorCopy( gun.lightingOrigin, equipment.lightingOrigin );
        equipment.shadowPlane = gun.shadowPlane;
        equipment.renderfx = gun.renderfx;

        equipment.hModel = weapon->v_silencerModel;

        if ( gun.customSkin )
            equipment.customSkin = gun.customSkin;

        CG_PositionEntityOnTag( &equipment, &gun, weapon->viewweaponModel,"tag_flash");

        if ( equipment.hModel )
            trap_R_AddRefEntityToScene( &equipment );

    }

    if ( i_equipment & ( 1 << PW_LASERSIGHT ) )
    {
        memset( &equipment, 0, sizeof( equipment ) );
        VectorCopy( gun.lightingOrigin, equipment.lightingOrigin );
        equipment.shadowPlane = gun.shadowPlane;
        equipment.renderfx = gun.renderfx;

        equipment.hModel = weapon->v_lasersightModel;

        if ( gun.customSkin )
            equipment.customSkin = gun.customSkin;

        CG_PositionEntityOnTag( &equipment, &gun, weapon->viewweaponModel,"tag_weapon");

        if ( equipment.hModel )
            trap_R_AddRefEntityToScene( &equipment );

    }
    if ( i_equipment & ( 1 << PW_SCOPE ) ) //
    {
        memset( &equipment, 0, sizeof( equipment ) );
        VectorCopy( gun.lightingOrigin, equipment.lightingOrigin );
        equipment.shadowPlane = gun.shadowPlane;
        equipment.renderfx = gun.renderfx;

        equipment.hModel = weapon->v_scopeModel;

        if ( gun.customSkin )
            equipment.customSkin = gun.customSkin;

        CG_PositionEntityOnTag( &equipment, &gun, weapon->viewweaponModel,"tag_weapon");

        if ( equipment.hModel )
            trap_R_AddRefEntityToScene( &equipment );
    } else if ( weapon->item->giTag == WP_M4 ) {
        memset( &equipment, 0, sizeof( equipment ) );
        VectorCopy( gun.lightingOrigin, equipment.lightingOrigin );
        equipment.shadowPlane = gun.shadowPlane;
        equipment.renderfx = gun.renderfx;

        equipment.hModel = weapon->v_flashModel;

        if ( gun.customSkin )
            equipment.customSkin = gun.customSkin;

        CG_PositionEntityOnTag( &equipment, &gun, weapon->viewweaponModel,"tag_weapon");

        if ( equipment.hModel )
            trap_R_AddRefEntityToScene( &equipment );
    }

    if ( i_equipment & ( 1 << PW_M203GL ) ) //
    {
        memset( &equipment, 0, sizeof( equipment ) );
        VectorCopy( gun.lightingOrigin, equipment.lightingOrigin );
        equipment.shadowPlane = gun.shadowPlane;
        equipment.renderfx = gun.renderfx;

        equipment.hModel = weapon->v_glModel;

        if ( gun.customSkin )
            equipment.customSkin = gun.customSkin;

        CG_PositionEntityOnTag( &equipment, &gun, weapon->viewweaponModel,"tag_weapon");

        if ( equipment.hModel )
            trap_R_AddRefEntityToScene( &equipment );

    }
    else if ( i_equipment & ( 1 << PW_FLASHLIGHT ) ) //
    {
        memset( &equipment, 0, sizeof( equipment ) );
        VectorCopy( gun.lightingOrigin, equipment.lightingOrigin );
        equipment.shadowPlane = gun.shadowPlane;
        equipment.renderfx = gun.renderfx;

        equipment.hModel = weapon->v_glModel;

        if ( gun.customSkin )
            equipment.customSkin = gun.customSkin;

        CG_PositionEntityOnTag( &equipment, &gun, weapon->viewweaponModel,"tag_weapon");

        if ( equipment.hModel )
            trap_R_AddRefEntityToScene( &equipment );

    }
    if ( i_equipment & ( 1 << PW_BAYONET ) ) //
    {
        memset( &equipment, 0, sizeof( equipment ) );
        VectorCopy( gun.lightingOrigin, equipment.lightingOrigin );
        equipment.shadowPlane = gun.shadowPlane;
        equipment.renderfx = gun.renderfx;

        equipment.hModel = weapon->v_bayonetModel;

        if ( gun.customSkin )
            equipment.customSkin = gun.customSkin;

        CG_PositionEntityOnTag( &equipment, &gun, weapon->viewweaponModel,"tag_weapon");

        if ( equipment.hModel )
            trap_R_AddRefEntityToScene( &equipment );

    }
    trap_R_AddRefEntityToScene( &gun );
}

/*
========================
CG_AddWeaponParts

This is used for worldmodel rendering.

draws all the models parts on the weapon
========================
*/
void CG_AddWeaponParts( refEntity_t hands, weaponInfo_t	*weapon )
{
    refEntity_t part;
    char		*tagname;
    int			i = 0;

    if ( cg.predictedPlayerState.weapon == WP_C4 )
        return;

    memset( &part, 0, sizeof( part ) );
    VectorCopy( hands.lightingOrigin, part.lightingOrigin );
    part.shadowPlane = hands.shadowPlane;
    part.renderfx = hands.renderfx;

    if ( hands.customSkin )
        part.customSkin = hands.customSkin;

    // render base weapon body
    part.hModel = weapon->weaponModel;


    CG_PositionEntityOnTag( &part, &hands, hands.hModel, "tag_weapon");

    trap_R_AddRefEntityToScene( &part );

    // no weaponparts to render
    if ( strlen( weapon->partTags[0] ) <= 1 )
        return;

    // render all parts for this weapon
    while ( strlen( weapon->partTags[i] ) > 1 )
    {
        tagname = va("tag_%s",weapon->partTags[i] );

        // special handling for belt feeded weapons
        if ( !Q_stricmp( "belt" , weapon->partTags[i] ) ||
                !Q_stricmp( "bullet" , weapon->partTags[i] )
           )
        {
            int rounds = cg.predictedPlayerState.stats[STAT_ROUNDS];
            int j = 0;

            if ( rounds > 8 )
                rounds = 8;

            rounds--;

            if ( rounds > 0 )
                for ( j=0; j<rounds; j++ )
                {
                    if ( !Q_stricmp( "bullet" , weapon->partTags[i] ) )
                        tagname = va( "tag_bullet0%i", j+1 );
                    else
                        tagname = va( "tag_belt0%i", j+1 );

                    part.hModel = weapon->weaponParts[i];

                    CG_PositionEntityOnTag( &part, &hands, hands.hModel, tagname);

                    trap_R_AddRefEntityToScene( &part );
                }


            i++;
            continue;
        }

        if ( !Q_stricmp( "gl" , weapon->partTags[i] ) ||
                !Q_stricmp( "m203gren" , weapon->partTags[i] ) ||
                !Q_stricmp( "m203rel" , weapon->partTags[i] ) ||
                !Q_stricmp( "m203" , weapon->partTags[i] ))
        {
            if( !(cg.predictedPlayerState.stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) ) ) {
                i++;
                continue;
            }

            if ( !Q_stricmp( "gl" , weapon->partTags[i] ) ||
                    !Q_stricmp( "m203" , weapon->partTags[i] ))
                tagname = "tag_weapon";
        }


        if ( !Q_stricmp( "handle" , weapon->partTags[i] ) )
        {
            if( cg.predictedPlayerState.stats[STAT_WEAPONMODE] & ( 1 << WM_SCOPE ) ) {
                i++;
                continue;
            }

            tagname = "tag_weapon";
        }

        part.hModel = weapon->weaponParts[i];

        CG_PositionEntityOnTag( &part, &hands, hands.hModel, tagname);

        trap_R_AddRefEntityToScene( &part );
        i++;
    }

}


/*
=============
CG_AddPlayerWeapon

Used for the world modelother character models (ps is NULL)
The main player will have this called for BOTH cases, so effects like light and
sound should only be done on the world model case.
=============
*/
qboolean pointinfrontmindot (vec3_t angles, vec3_t origin, vec3_t point, float mindot)
{
    vec3_t	vec;
    float	dot;
    vec3_t	forward;

    AngleVectors (angles, forward, NULL, NULL);
    VectorSubtract (point, origin, vec);
    VectorNormalize (vec);
    dot = DotProduct (vec, forward);

    if (dot > mindot)
        return qtrue;
    return qfalse;
}

void _CG_Flare( centity_t *cent, vec3_t lerpOrigin, float size, float r, float g, float b, qboolean showCorona );
void CG_AddPlayerWeapon( refEntity_t *leftArm, refEntity_t *rightArm, playerState_t *ps, centity_t *cent ) {
    refEntity_t	gun;
    refEntity_t	flash;
    vec3_t		angles;
    int	weaponNum;
    weaponInfo_t	*weapon;
    centity_t	*nonPredictedCent;

    weaponNum = cent->currentState.weapon;
 
    weapon = &cg_weapons[weaponNum];

    // add the weapon
    memset( &gun, 0, sizeof( gun ) );
    VectorCopy( rightArm->lightingOrigin, gun.lightingOrigin );
    gun.shadowPlane = rightArm->shadowPlane;
    gun.renderfx = rightArm->renderfx;

    gun.hModel = weapon->viewweaponModel;
    //
    // apply tango skin
    //
    if ( !cg_disableTangoHandSkin.integer )
    {
        clientInfo_t	*ci = &cgs.clientinfo[ cent->currentState.number ];


        if ( ci->team  == TEAM_BLUE)
            gun.customSkin = weapon->t_viewweaponSkin;

        // we're caching this here because it's only one weapon that uses this
        // it's not worth adding a _v.skin for every weapon
        if ( cent->currentState.eFlags & EF_VIP && BG_IsPistol( weaponNum ) )
        {
            char			path[MAX_QPATH];
            gitem_t			*item;

            item = BG_FindItemForWeapon( weaponNum );

            if ( item )
            {
                strcpy( path, item->world_model[0] );
                COM_StripExtension( path, path );
                strcat( path, "_v.skin" );

                gun.customSkin =  trap_R_RegisterSkin( path );
            }
        }


    }
    if (!gun.hModel) {
        return;
    }

    CG_PositionEntityOnTag( &gun, rightArm, rightArm->hModel, "tag_weapon");

    CG_AddWeaponWithPowerups( gun, weapon, cent->currentState.powerups, cent->currentState.eFlags );

    // make sure we aren't looking at cg.predictedPlayerEntity for LG
    nonPredictedCent = &cg_entities[cent->currentState.clientNum];

    // if the index of the nonPredictedCent is not the same as the clientNum
    // then this is a fake player (like on teh single player podiums), so
    // go ahead and use the cent
    if( ( nonPredictedCent - cg_entities ) != cent->currentState.clientNum ) {
        nonPredictedCent = cent;
    }

    //
    // flashlight
    //
    if ( cent->currentState.eFlags & EF_WEAPONMODE3 ) // flashlight
    {

        vec3_t forward,right;
        trace_t		trace;
        vec3_t			muzzlePoint, endPoint;
        refEntity_t		beam;
        //		qhandle_t laser;
        //		int	rf;
        //		vec4_t	rgba;
#define FLASHLIGHT_RANGE	500

        memset( &beam, 0, sizeof( beam ) );
        memset( &flash, 0, sizeof( flash ) );

        VectorCopy( cent->currentState.pos.trBase , muzzlePoint );

        AngleVectors( cent->currentState.apos.trBase , forward, right, NULL );

        // project forward by the lightning range
        VectorMA( cent->currentState.pos.trBase , FLASHLIGHT_RANGE, forward, endPoint );

        {
            int anim;

            anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;

            if ( anim == LEGS_WALKCR || anim == LEGS_BACKCR || anim == LEGS_IDLECR ) {
                endPoint[2] += CROUCH_VIEWHEIGHT;
                muzzlePoint[2] += CROUCH_VIEWHEIGHT;
            }
            else {
                endPoint[2] += DEFAULT_VIEWHEIGHT;
                muzzlePoint[2] += DEFAULT_VIEWHEIGHT;
            }
        }

        // see if it hit a wall
        CG_Trace( &trace, muzzlePoint, vec3_origin, vec3_origin, endPoint,
                  cent->currentState.number, MASK_SHOT );

        // BLUTENGEL 20040207
        // def 2006-01-24, fixed dlights - removed junk code
        VectorMA(trace.endpos, -20 , forward, trace.endpos);
        trap_R_AddLightToScene( trace.endpos, 100, 1.0f-trace.fraction,1.0f-trace.fraction,1.0f-trace.fraction);

        // render the lensflare effect
        CG_GetOriginFromTag(&gun, weapon->viewweaponModel , "tag_flash", endPoint );

        VectorMA( endPoint, 1, right, endPoint );

        // this is ugly. we have to trace back from org to the player model to see wheter
        // the flash would be sticking through a wall or not...
        CG_Trace( &trace, endPoint, vec3_origin, vec3_origin, cent->lerpOrigin, -1, MASK_SHOT );

        if ( trace.entityNum == cent->currentState.clientNum )
        {
            if ( pointinfrontmindot( cent->currentState.apos.trBase,endPoint, cg.refdef.vieworg , -0.3f ) )
            {
                _CG_Flare( cent, endPoint, 10, 100 ,100 ,100 ,qtrue );
            }
        }
    }

    //
    // laserbeam
    //
    if ( cent->currentState.eFlags & EF_LASERSIGHT )
    {

        vec3_t forward;
        trace_t		trace;
        vec3_t			muzzlePoint, endPoint;
        refEntity_t		beam;
        qhandle_t laser;
        int	rf;

        memset( &beam, 0, sizeof( beam ) );
        memset( &flash, 0, sizeof( flash ) );


        CG_PositionRotatedEntityOnTag( &flash, &gun, weapon->viewweaponModel, "tag_laser");
        // find muzzle point for this frame
        VectorCopy ( flash.origin,muzzlePoint );

        AngleVectors( cent->currentState.apos.trBase , forward, NULL, NULL );

        // project forward by the lightning range
        VectorMA( cent->currentState.pos.trBase , LIGHTNING_RANGE, forward, endPoint );

        { // 0711-9970713
            int anim;

            anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;

            if ( anim == LEGS_WALKCR  || anim == LEGS_BACKCR || anim == LEGS_IDLECR )
                endPoint[2] += CROUCH_VIEWHEIGHT;
            else
                endPoint[2] += DEFAULT_VIEWHEIGHT;
        }

        laser = trap_R_RegisterShader( "gfx/misc/ns_laserbeam" );
        // see if it hit a wall
        CG_Trace( &trace, muzzlePoint, vec3_origin, vec3_origin, endPoint,
                  cent->currentState.number, MASK_SHOT );

        VectorCopy( trace.endpos , endPoint );

        if ( cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson ) {
            rf = RF_THIRD_PERSON;		// only show in mirrors
        } else {
            rf = 0;
        }

        if (! (CG_PointContents( muzzlePoint, cent->currentState.number ) & CONTENTS_SOLID) &&
                !trace.startsolid )
        {
            if ( !rf) {
                vec4_t rgba;

                rgba[0] = rgba[1] = rgba[2] = 1;
                rgba[3] = 0.6f;

                CG_Tracer( muzzlePoint, endPoint, 0.1f, laser , rgba );
            }

            // add the impact flare if it hit something
            if ( trace.fraction < 1.0 ) {
                vec3_t	angles;


                beam.customShader = cgs.media.laserShader;
                beam.reType = RT_SPRITE;
                beam.radius = 0.5;

                beam.renderfx = rf;
                VectorMA( trace.endpos, -0.4, forward, beam.origin );

                // make a random orientation
                angles[0] = rand() % 360;
                angles[1] = rand() % 360;
                angles[2] = rand() % 360;
                AnglesToAxis( angles, beam.axis );
                trap_R_AddRefEntityToScene( &beam );
            }
        }
    }
    //
    // shell ejection
    //
    if ( cg_brassTime.value > 0 && weapon->ejectBrassNum && cent->brassEjected > 0 )
    {
        // only draw these if we're not our client
        if ( cent->currentState.number != cg.snap->ps.clientNum || cg.renderingThirdPerson )
        {
            //		CG_Printf("%i \n",cg.time - cent->muzzleFlashTime);
            if ( weaponNum == WP_SW629 && cg.time - cent->muzzleFlashTime < ( SW629_BRASS_EJECTTIME + MUZZLE_FLASH_TIME ) )
                ;
            else if ( weaponNum == WP_MACMILLAN && cg.time - cent->muzzleFlashTime < MACMILLAN_BRASS_EJECTTIME )
                ;
            else {
                vec3_t origin;

                if ( weaponNum == WP_SW629 )
                {
                    char *tag_name = "tag_ejection";

                    if ( cent->brassEjected == 6 )
                        tag_name = "tag_ejection6";
                    else if ( cent->brassEjected == 5 )
                        tag_name = "tag_ejection5";
                    else if ( cent->brassEjected == 4 )
                        tag_name = "tag_ejection4";
                    else if ( cent->brassEjected == 3 )
                        tag_name = "tag_ejection3";
                    else if ( cent->brassEjected == 2 )
                        tag_name = "tag_ejection2";

                    // sw629 special rule
                    CG_GetOriginFromTag( &gun, weapon->viewweaponModel, tag_name, origin );
                }
                else
                    CG_GetOriginFromTag( &gun, weapon->viewweaponModel, "tag_ejection", origin );

                CG_EjectBrass( cent, origin , weapon->ejectBrassType, qfalse );

                cent->brassEjected--;
            }
        }
    }

    if ( cent->gunSmokePuff )
    {
        /*
        vec3_t up;
        localEntity_t *smoke;
        vec3_t	forward;

        AngleVectors( cent->currentState.apos.trBase , forward, NULL, NULL );

        memset( &flash, 0, sizeof( flash ) );

        // find muzzle point for this frame
        CG_CalculateWeaponPosition( flash.origin, angles );
        AngleVectors( angles  , forward, NULL, NULL );
        VectorMA( up, 15, forward, up );

        up[2] = 5;

        if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_SILENCER ) )
        CG_PositionRotatedEntityOnTag( &flash, &gun, weapon->viewweaponModel, "tag_flash2");
        else
        CG_PositionRotatedEntityOnTag( &flash, &gun, weapon->viewweaponModel, "tag_flash");

        cent->gunSmokePuff = qfalse;

        smoke = CG_SmokePuff( flash.origin, up, 7.5f,1,1,1,1,225,cg.time,0,LE_MOVE_SCALE_FADE, cgs.media.smokePuffShader  );
        */
    }
    //
    // gun smoke
    //
    if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME )
    {
        if ( cent->gunSmokeTime - cg.time > 0 && ( !BG_IsMelee( weaponNum ) && !BG_IsGrenade( weaponNum ) && weaponNum != WP_C4 ) && ( cg_gunSmoke.integer > 1 || cg_gunSmoke.integer == -1 ) )
            if ( cent->currentState.number != cg.snap->ps.clientNum || cg.renderingThirdPerson )
            {
                vec3_t up;
                localEntity_t *a;
                memset( &flash, 0, sizeof( flash ) );

                if ( cent->currentState.eFlags & EF_SILENCED &&
                        (weaponNum != WP_AK47 && weaponNum != WP_M4 ) )
                    CG_PositionRotatedEntityOnTag( &flash, &gun, weapon->viewweaponModel, "tag_flash2");

                else
                    CG_PositionRotatedEntityOnTag( &flash, &gun, weapon->viewweaponModel, "tag_flash");

                up[0] = random()*3;
                up[1] = random()*3;
                up[2] = 2 + random()*3;

                if ( cent->gunSmokeTime - cg.time < 1000 )
                {
                    float t;

                    t = (float)((float)( cent->gunSmokeTime - cg.time ) / 2000 );

                    a = CG_SmokePuff(  flash.origin, up, cg_gunSmokeTime.integer / 333 + 1,1,1,1,t,cg_gunSmokeTime.integer,cg.time,0,LE_MOVE_SCALE_FADE, cgs.media.smokePuffShader  );
                }
                else
                    a = CG_SmokePuff(  flash.origin, up, cg_gunSmokeTime.integer / 333 + 1,1,1,1,0.5,cg_gunSmokeTime.integer,cg.time,0,LE_MOVE_SCALE_FADE, cgs.media.smokePuffShader  );

            }
        return;
    }
    // no muzzle flash when a silencer attached.
    if ( cent->currentState.eFlags & EF_SILENCED )
        return;

    //
    // impulse flash
    //
    memset( &flash, 0, sizeof( flash ) );
    VectorCopy( rightArm->lightingOrigin, flash.lightingOrigin );
    flash.shadowPlane = rightArm->shadowPlane;
    flash.renderfx = rightArm->renderfx;

    flash.hModel = weapon->flashModel;
    if (!flash.hModel) {
        return;
    }
    //
    // randomly scale the muzzle flash

    angles[YAW] = 0;
    angles[PITCH] = 0;
    angles[ROLL] = 0;

    {
        float size;
        int i;
        int a;

        size = random() + 0.5f;

        if (!BG_IsRifle(weaponNum))
            angles[ROLL] = rand() % 360;

        AnglesToAxis( angles, flash.axis );

        if ( size > 1.50f )
            size = 1.50f;

        for ( i = 0; i < 3; i++ )
            for ( a = 0; a < 3; a++ )
                flash.axis[i][a] *= size;
    }

    CG_PositionRotatedEntityOnTag( &flash, &gun, weapon->viewweaponModel, "tag_flash");

    trap_R_AddRefEntityToScene( &flash );

#if 0 // ENABLE TO MAKE THE MUZZLEFLASH CREATE LENSFLARES
    // this is ugly. we have to trace back from org to the player model to see wheter
    // the flash would be sticking through a wall or not...
    {
        trace_t	trace;

        CG_Trace( &trace, flash.origin, vec3_origin, vec3_origin, cent->lerpOrigin, -1, MASK_SHOT );

        if ( trace.entityNum == cent->currentState.clientNum )
        {
            if ( pointinfrontmindot( cent->currentState.apos.trBase,flash.origin, cg.refdef.vieworg , -0.3 + test_x.value ) )
            {
                _CG_Flare( cent, flash.origin, 15, 100 ,100 ,100 ,qfalse );
            }
        }
    }
#endif
    // make a dlight for the flash
    if ( weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2] ) {
        trap_R_AddLightToScene( flash.origin, 200 + (rand()&31), weapon->flashDlightColor[0],
                                weapon->flashDlightColor[1], weapon->flashDlightColor[2] );
    }
}


/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/

/*
===============
CG_WeaponSelectable
===============
*/
static qboolean CG_WeaponSelectable( int i ) {

    if ( ! (BG_GotWeapon( i, cg.snap->ps.stats) ) ) {
        return qfalse;
    }

    return qtrue;
}

/*
===================
CG_DrawWeaponSelect
===================
*/
#define IN_INVENTORYSPACER 8
#define IN_INACTIVE_SMALL_W	16
#define IN_INACTIVE_BIG_W	128
#define IN_INACTIVE_HEIGHT	16

#define IN_ACTIVE_W			128
#define IN_ACTIVE_H			16

void CG_DrawWeaponSelect( void ) {
    int		x, y;
    int		xpos[5];
    vec4_t	color;
    int		axy;
    int		s;
    float	alpha;

    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = 1.0f;

    if ( cg.InventoryFadeTime )
    {
        alpha = (float)(cg.InventoryFadeTime - cg.time)/1000.0f;

        color[3] = alpha;

        trap_R_SetColor( color );

        if ( alpha <= 0.0f )
        {
            cg.curInventory = 666;
            cg.activeInventory = 0;
            cg.InventoryFadeTime = 0;
            return;
        }
    } else	trap_R_SetColor( color );

    // don't display if dead
    if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 )
        return;

    if ( !cg.activeInventory )
        return;


    x = 8;

    if ( cg.activeInventory == 1 ) // primary
        trap_R_DrawStretchPic(x,32, IN_ACTIVE_W,IN_ACTIVE_H, 0, 0, 1, 1,cgs.media.weaponMenuActive[0] );
    else  {

        trap_R_DrawStretchPic(x,32, IN_INACTIVE_SMALL_W,IN_ACTIVE_H, 0, 0, 1, 1,cgs.media.weaponMenuInactive[0] );

    }

    xpos[0] = x;
    xpos[1] = BIGCHAR_WIDTH + xpos[0];
    xpos[2] = BIGCHAR_WIDTH + xpos[1];
    xpos[3] = BIGCHAR_WIDTH + xpos[2];
    xpos[4] = BIGCHAR_WIDTH + xpos[3];
    xpos[5] = BIGCHAR_WIDTH + xpos[4];

    for (axy = cg.activeInventory; axy <= 4; axy++)
        xpos[axy] += 128 ;

    axy = 0;

    for ( x = 0; x <= 4 ; x ++ )
    {
        y = 32 + TINYCHAR_HEIGHT + IN_INVENTORYSPACER;

        if ( x == cg.activeInventory-1 )
        {

            for ( s = 0;  s <  cg.Inventorypos[cg.activeInventory-1]; s++ )
            {
                if ( s == cg.curInventory-1 )
                {
                    // only powerup in inventory?
                    if ( cg.Inventory[ cg.activeInventory -1][s] <= 0 ) {
                        gitem_t *item;
                        int pw = 0;
                        qhandle_t shader;
 
                        pw = cg.Inventory[ cg.activeInventory -1][s] * -1;

                        if ( pw == PW_BRIEFCASE )
                        {
                            shader = trap_R_RegisterShader("icons/weapons/icon_suitcase");
                        }
                        else
                        {
                            item = BG_FindItemForPowerup( pw );

                            shader = cg_items[ ITEM_INDEX(item) ].icon;
                        }

                        trap_R_DrawStretchPic(xpos[x],y,IN_INACTIVE_BIG_W,IN_INACTIVE_HEIGHT*4, 0, 0, 1, 1,shader );
                    }
                    else {
                         trap_R_DrawStretchPic(xpos[x],y,IN_INACTIVE_BIG_W,IN_INACTIVE_HEIGHT*4, 0, 0, 1, 1,cg_weapons[ cg.Inventory[ cg.activeInventory-1 ] [ s ] ].weaponIcon  );
                    }

                    y += IN_INACTIVE_HEIGHT*4 + IN_INVENTORYSPACER;
                }
                else
                {
                    // render inactive inventory (but in active list )
                    if ( cg.curInventory > 0 )
                        trap_R_DrawStretchPic( xpos[ x ], y , IN_INACTIVE_BIG_W,IN_INACTIVE_HEIGHT, 0, 0, 1, 1,cgs.media.weaponSelInactiveBig );
                    else
                        trap_R_DrawStretchPic( xpos[ x ],y, IN_INACTIVE_SMALL_W,IN_INACTIVE_HEIGHT, 0, 0, 1, 1,cgs.media.weaponSelInactiveSmall );

                    y += IN_INACTIVE_HEIGHT + IN_INVENTORYSPACER;
                }
            }
        }
        else
        {
            //
            // render inactive inventory
            //
            for ( s = 0;  s <  cg.Inventorypos[x]; s++ )
            {
                trap_R_DrawStretchPic(xpos[ x ],y, IN_INACTIVE_SMALL_W,IN_INACTIVE_HEIGHT, 0, 0, 1, 1,cgs.media.weaponSelInactiveSmall );

                y += IN_INACTIVE_HEIGHT + IN_INVENTORYSPACER;
            }
        }
    }

    //
    // render the rest of the titlebars
    //
    for ( s = 1; s < 5;s++)
    {
        if ( s == cg.activeInventory-1 ) {
            trap_R_DrawStretchPic(xpos[s],32, IN_ACTIVE_W,IN_ACTIVE_H, 0, 0, 1, 1,cgs.media.weaponMenuActive[s] );
        }
        else {
            trap_R_DrawStretchPic(xpos[s],32, IN_INACTIVE_SMALL_W,IN_ACTIVE_H, 0, 0, 1, 1,cgs.media.weaponMenuInactive[s] );
        }
    }

    trap_R_SetColor( NULL );

}


#define PRIMARY			0
#define SECONDARY		1
#define MELEE			2
#define EXPLO			3
#define MISC			4
#define INVENTORYITEMS	5

/*
==================
CG_Invenselect(...)

uses selected menu item
==================
*/

void CG_InvenSelect ( void )
{
    if (!cg.activeInventory)
        return;
    if ( cg.curInventory >  cg.Inventorypos[ cg.activeInventory - 1 ] )
        return;
    if ( cg.curInventory < 0 )
        return;
    if ( cg.Inventorypos[ cg.activeInventory-1] == 0 )
        return;

    if ( 1 )
    {
        int pw = cg.Inventory[ cg.activeInventory -1][ cg.curInventory - 1] * -1;

        if ( pw == PW_BRIEFCASE )
        {
            // don't select this weapon. just ignore.
            trap_SendClientCommand( "dropmissionobjective\n" );
            cg.curInventory = 666;
            cg.activeInventory = 0;
            // play select sound
            if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_ITEM, trap_S_RegisterSound("sound/hud/select.wav",qfalse) );
            return;
        }
    }

    if ( cg.Inventory[cg.activeInventory-1][cg.curInventory-1] <= 0 )
        return;

    cg.InventoryTime = 0;

    if (cg.curInventory == 666 ) {
        if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_ITEM, trap_S_RegisterSound("sound/hud/close.wav",qfalse) );
        cg.curInventory = 666;
        cg.activeInventory = 0;
        cg.weaponSelect = cg.snap->ps.weapon;
        return;
    }

    // select that weapon
    cg.weaponSelect = cg.Inventory[ cg.activeInventory-1 ] [ cg.curInventory - 1];
    /*
    cg.curInventory = 666;
    cg.activeInventory = 0;
    cg.weaponSelectTime = cg.time + 1000;
    */
    cg.InventoryFadeTime = cg.time + 1000;

    cg.weaponSelectTime = cg.time + 1000;
    // play select sound
    if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_ITEM, trap_S_RegisterSound("sound/hud/select.wav",qfalse) );
}

void CG_MWheel_f ( int prev ) {
    int		num = 1;
    int i;
    char var[MAX_TOKEN_CHARS];
    int  count = 0;

    if ( !cg.snap ) {
        return;
    }
    if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
        return;
    }
    if ( cg.snap->ps.pm_type == PM_SPECTATOR || cg.snap->ps.pm_type == PM_NOCLIP )
        return;

    // new time at any keypress ( for auto weapon select )
    cg.InventoryTime = cg.time + cg_enableTimeSelect.integer * 1000;

    //
    // update list on key press
    //

    // clear old inventory
    cg.Inventorypos[PRIMARY] = cg.Inventorypos[SECONDARY] = cg.Inventorypos[MELEE] = cg.Inventorypos[EXPLO] = cg.Inventorypos[MISC] = 0;

    for ( i = 1 ; i < WP_NUM_WEAPONS ; i++ )
    {
        if ( BG_GotWeapon( i , cg.snap->ps.stats ) )
        {
			if ( BG_IsPrimary( i ) )
            {
                cg.Inventory[PRIMARY][ cg.Inventorypos[PRIMARY] ] = i;
                cg.Inventorypos[PRIMARY]++;
            }
            else if ( BG_IsSecondary( i ) )
            {
                cg.Inventory[SECONDARY][ cg.Inventorypos[SECONDARY] ] = i;
                cg.Inventorypos[SECONDARY]++;
            }
            else if ( BG_IsMelee( i ) )
            {
                cg.Inventory[MELEE][ cg.Inventorypos[MELEE] ] = i;
                cg.Inventorypos[MELEE]++;
            }
            else if ( i == WP_GRENADE ) {
                trap_Cvar_VariableStringBuffer("inven_ammo_mk26", var, sizeof(var));
                count = atoi(var);
                if (count > 0) {
                    cg.Inventory[EXPLO][ cg.Inventorypos[EXPLO] ] = i;
                    cg.Inventorypos[EXPLO]++;
                }
            }
            else if ( i == WP_FLASHBANG ) {
                trap_Cvar_VariableStringBuffer("inven_ammo_flash", var, sizeof(var));
                count = atoi(var);
                if (count > 0) {
                    cg.Inventory[EXPLO][ cg.Inventorypos[EXPLO] ] = i;
                    cg.Inventorypos[EXPLO]++;
                }
            }
            else if ( i == WP_SMOKE ) {
                trap_Cvar_VariableStringBuffer("inven_ammo_smoke", var, sizeof(var));
                count = atoi(var);
                if (count > 0) {
                    cg.Inventory[EXPLO][ cg.Inventorypos[EXPLO] ] = i;
                    cg.Inventorypos[EXPLO]++;
                }
            }
            else if ( i == WP_C4 ) {
                cg.Inventory[MISC][ cg.Inventorypos[MISC] ] = i;
                cg.Inventorypos[MISC]++;
            }
        }
    }

    // add powerups here:
    if( cg.snap->ps.powerups[PW_BRIEFCASE] )
    {
        cg.Inventory[MISC][ cg.Inventorypos[MISC] ] = PW_BRIEFCASE *-1;
        cg.Inventorypos[MISC]++;
    }

    // still in valid range?
    if ( cg.activeInventory < 0 || cg.activeInventory > 5 )
        cg.activeInventory = 0;

    // not opened any menu
    if ( !cg.activeInventory )
    {
        if ( num < 0 || num > 5 ) // primary,secondary,melee,explo,misc
            return;

        // fixme: play open sound
        if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_ITEM, trap_S_RegisterSound("sound/hud/open.wav",qfalse) );

        if ( prev )
            cg.activeInventory = 5;
        else
            cg.activeInventory = 1;

        cg.curInventory = 0;
        return;
    }

    if ( prev )
    {
        num = cg.curInventory--;

        if ( cg.curInventory <= 0)
        {
            if (cg.activeInventory-1 == 0) // skip to misc
                cg.curInventory = cg.Inventorypos[ MISC ];
            else
                cg.curInventory = cg.Inventorypos[ cg.activeInventory - 2 ];

            cg.activeInventory--;

            if ( cg.activeInventory <= 0)
                cg.activeInventory = 5;

            if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_ITEM, trap_S_RegisterSound("sound/hud/open.wav",qfalse) );
            return;
        }

        if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_ITEM, trap_S_RegisterSound("sound/hud/moveselect.wav",qfalse) );
        return;

    }
    // open next
    else if ( num > cg.Inventorypos[ cg.activeInventory-1 ] )
    {
        if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_ITEM, trap_S_RegisterSound("sound/hud/open.wav",qfalse) );

        if ( cg.activeInventory < 5 && !prev)
            cg.activeInventory++;
        else if ( cg.activeInventory > 1 && prev)
            cg.activeInventory--;
        else if ( !prev )
            cg.activeInventory = 1;
        else if ( prev )
            cg.activeInventory = 5;

        cg.curInventory = 1;
        return;
    }
    else
    {
        num = cg.curInventory++;

        if ( cg.curInventory > cg.Inventorypos[ cg.activeInventory - 1 ])
        {
            cg.curInventory = 1;
            cg.activeInventory++;
            if ( cg.activeInventory > 5)
                cg.activeInventory = 1;
            if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_ITEM, trap_S_RegisterSound("sound/hud/open.wav",qfalse) );
            return;
        }

        if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_ITEM, trap_S_RegisterSound("sound/hud/moveselect.wav",qfalse) );
        return;
    }

}

/*
CG_Weapon_f
===============
*/
void CG_Weapon_f( void ) {
    int		num;
    int		i;
    char var[MAX_TOKEN_CHARS];
    int  count = 0;

    if ( !cg.snap ) {
        return;
    }
    if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
        return;
    }
    if ( cg.snap->ps.pm_type == PM_SPECTATOR || cg.snap->ps.pm_type == PM_NOCLIP )
        return;
    if ( cg.viewCmd )
        return;
    num = atoi( CG_Argv( 1 ) );

    if (cg.inMenu)
    {
        // not a valid slot
        if ( num < 1 || num > cg.menuValidSlots )
            return;

        cg.inMenu = qfalse;
        cg.menuValidSlots = 0;

        //then send the commmand
        trap_SendClientCommand( va("menuselect %i",num) );

        if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_LOCAL, cgs.media.noAmmoSound);

        return;
    }

    // give the user more time to choose
    cg.InventoryTime = cg.time + cg_enableTimeSelect.integer * 1000;
    // update list on key press

    // clear
    cg.Inventorypos[PRIMARY] = cg.Inventorypos[SECONDARY] = cg.Inventorypos[MELEE] = cg.Inventorypos[EXPLO] = cg.Inventorypos[MISC] = 0;

    for ( i = 1 ; i < WP_NUM_WEAPONS ; i++ ) {
        if ( BG_GotWeapon( i , cg.snap->ps.stats ) )
        { 
            if ( BG_IsPrimary( i ) )
            {
                cg.Inventory[PRIMARY][ cg.Inventorypos[PRIMARY] ] = i;
                cg.Inventorypos[PRIMARY]++;
            }
            else if ( BG_IsSecondary( i ) )
            {
                cg.Inventory[SECONDARY][ cg.Inventorypos[SECONDARY] ] = i;
                cg.Inventorypos[SECONDARY]++;
            }
            else if ( BG_IsMelee( i ) )
            {
                cg.Inventory[MELEE][ cg.Inventorypos[MELEE] ] = i;
                cg.Inventorypos[MELEE]++;
            }
            else if ( i == WP_GRENADE ) {
                trap_Cvar_VariableStringBuffer("inven_ammo_mk26", var, sizeof(var));
                count = atoi(var);
                if (count > 0) {
                    cg.Inventory[EXPLO][ cg.Inventorypos[EXPLO] ] = i;
                    cg.Inventorypos[EXPLO]++;
                }
            }
            else if ( i == WP_FLASHBANG ) {
                trap_Cvar_VariableStringBuffer("inven_ammo_flash", var, sizeof(var));
                count = atoi(var);
                if (count > 0) {
                    cg.Inventory[EXPLO][ cg.Inventorypos[EXPLO] ] = i;
                    cg.Inventorypos[EXPLO]++;
                }
            }
            else if ( i == WP_SMOKE ) {
                trap_Cvar_VariableStringBuffer("inven_ammo_smoke", var, sizeof(var));
                count = atoi(var);
                if (count > 0) {
                    cg.Inventory[EXPLO][ cg.Inventorypos[EXPLO] ] = i;
                    cg.Inventorypos[EXPLO]++;
                }
            }
            else if ( i == WP_C4 ) {
                cg.Inventory[MISC][ cg.Inventorypos[MISC] ] = i;
                cg.Inventorypos[MISC]++;
            }
        }
    }

    // the user should know that he got it
    if( cg.snap->ps.powerups[PW_BRIEFCASE] )
    {
        // he can't activate it, so it set it to -1
        cg.Inventory[MISC][ cg.Inventorypos[MISC] ] = PW_BRIEFCASE*-1;
        cg.Inventorypos[MISC]++;
    }

    // currently the inventory isn't open so open the desired one
    if ( cg.activeInventory < 0 || cg.activeInventory > 5 ) {
        cg.activeInventory = 0;
    }

    // inventory contains one spot so quick select
    if ( /*cg.Inventorypos[ num-1 ] <= 1*/1 )
    {
        // if key is not in range of valid slots
        if ( num < 0 || num > 5 ) // primary,secondary,melee,explo,misc
            return;

        // nothing in inventory, return
        if ( cg.Inventorypos[ num-1 ] <= 0 )  {
            return;
        }

        // fixme: play open sound
        if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_ITEM, trap_S_RegisterSound("sound/hud/open.wav",qfalse) );

        // set the active inventory
        if ( !cg.activeInventory )
            cg.curInventory = 1;



        if ( cg.Inventorypos[ num-1 ] != 1 ) // only one item here? the go for it
        {
            if (cg.activeInventory == num )
                cg.curInventory++;
        }
        if ( cg.activeInventory != num )
            cg.curInventory = 1;

        cg.activeInventory = num;

        if ( cg.curInventory > cg.Inventorypos[ cg.activeInventory - 1 ] )
            cg.curInventory = 1;

        CG_InvenSelect( );

        return;
    }
    if ( !cg.activeInventory )
    {
        if ( cg.Inventorypos[ num-1] > 0 ) {
            cg.activeInventory = num;
            cg.curInventory = 1;
        }
        if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_ITEM, trap_S_RegisterSound("sound/hud/open.wav",qfalse) );
        return;
    }

    // now we pressed the key twice ( we're already on slot#num )
    if (cg.curInventory == num)
    {
        CG_InvenSelect();
        return;
    }

    // just set this slot as the active one
    cg.curInventory = num;

    // play sound
    if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_ITEM, trap_S_RegisterSound("sound/hud/moveselect.wav",qfalse) );
}

/*
===================
CG_OutOfAmmoChange

The current weapon has just run out of ammo
===================
*/
void CG_OutOfAmmoChange( void ) {
    int		i;

    cg.weaponSelectTime = cg.time;

    for ( i = 15 ; i > 0 ; i-- ) {
        if ( CG_WeaponSelectable( i ) ) {
            cg.weaponSelect = i;
            break;
        }
    }
}



/*
===================================================================================================

WEAPON EVENTS

===================================================================================================
*/

/*
================
CG_ReloadWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_ReloadWeapon( centity_t *cent, int last_rnd ) {
    entityState_t *ent;
    //	int				c;
    weaponInfo_t	*weap;

    ent = &cent->currentState;
    if ( ent->weapon == WP_NONE ) {
        return;
    }
    if ( ent->weapon >= WP_NUM_WEAPONS ) {
        CG_Error( "CG_ReloadWeapon: ent->weapon >= WP_NUM_WEAPONS" );
        return;
    }
    weap = &cg_weapons[ ent->weapon ];

    if ( ent->weapon == WP_SW629 )
    {
        cent->muzzleFlashTime = MUZZLE_FLASH_TIME;
        cent->brassEjected = 6;
    }

    if ( weap->reloadSound && !last_rnd) {
        if ((cg.DeafTime < cg.time)) trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->reloadSound );
        return;
    }

    if ( last_rnd && !weap->reloadEmptySound )
        return;

    if ((cg.DeafTime < cg.time)) trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->reloadEmptySound );
}

void CG_PredictFireLead();
void CG_PredictFireShotgun();
/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon( centity_t *cent, qboolean othermode ) {
    entityState_t *ent;
    int				c;
    weaponInfo_t	*weap;

    ent = &cent->currentState;
    if ( ent->weapon == WP_NONE ) {
        return;
    }
    if ( ent->weapon >= WP_NUM_WEAPONS ) {
        CG_Error( "CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS" );
        return;
    }
    weap = &cg_weapons[ ent->weapon ];

    // mark the entity as muzzle flashing, so when it is added it will
    // append the flash to the weapon model
    cent->muzzleFlashTime = cg.time;

    // is this 'our' client then increase crosshair size
    if ( weap->kickBack && ( cent->currentState.number == cg.snap->ps.clientNum ) )
        cg.crosshairMod += weap->kickBack;

    if ( cg.crosshairMod > 50 )
        cg.crosshairMod = 50;
    // firing using a silenced weapon , play a silenced sound then
    if ( othermode )
    {
        for ( c = 0 ; c < 4 ; c++ ) {
            if ( !weap->otherflashSound[c] ) {
                break;
            }
        }

        if ( c > 0 ) {
            c = rand() % c;
            if ( weap->otherflashSound[c] )
            {
                if ((cg.DeafTime < cg.time))
                    trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->otherflashSound[c] );
            }
        }
    }
    else if ( cent->currentState.eFlags & EF_SILENCED )
    {
        for ( c = 0 ; c < 4 ; c++ ) {
            if ( !weap->sil_flashSound[c] ) {
                break;
            }
        }

        if ( c > 0 ) {
            c = rand() % c;
            if ( weap->sil_flashSound[c] )
            {
                if ((cg.DeafTime < cg.time))
                    trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->sil_flashSound[c] );
            }
        }
    }
    else {
        // play a normal firing sound
        for ( c = 0 ; c < 4 ; c++ ) {
            if ( !weap->flashSound[c] ) 
                break; 
        }

        if ( c > 0 ) {
            c = rand() % c;
            if ( weap->flashSound[c] )
            {
			     if ((cg.DeafTime < cg.time))
					trap_S_StartSoundExtended( NULL, 1.0f, 0.5f+(random()/10.0f), 0.95f + random()/10, ent->number, CHAN_WEAPON, weap->flashSound[c] );
            }
        }
    }

    // do brass ejection
    if ( weap->ejectBrassNum > 0 && weap->ejectBrassType > BRASS_NONE && cg_brassTime.value > 0)
    {
        if ( BG_IsZooming( cg.snap->ps.stats[STAT_WEAPONMODE] ) && cg.snap->ps.clientNum == cent->currentState.clientNum && cg.snap->ps.weapon != WP_MACMILLAN )
            cent->brassEjected = 0;
        else
            cent->brassEjected = weap->ejectBrassNum;
    } 

    if ( !BG_IsGrenade( ent->weapon ) && !BG_IsMelee(ent->weapon) && ent->weapon != WP_C4 )
    {
        cent->gunSmokePuff = qtrue;

        if ( cent->gunSmokeTime < cg.time )
            cent->gunSmokeTime = cg.time + 500;
        else if ( cent->gunSmokeTime - cg.time < 2000 )
            cent->gunSmokeTime += 400;

    }
    if ( cent->currentState.clientNum == cg.snap->ps.clientNum &&
            BG_IsShotgun( cg.snap->ps.weapon ) )
    {
        if (cg_antiLag.integer) 
			CG_PredictFireShotgun( );
    }
    else if ( cent->currentState.clientNum == cg.snap->ps.clientNum
              && !BG_IsGrenade( cg.snap->ps.weapon )
              && !BG_IsMelee( cg.snap->ps.weapon )
              && !BG_IsGrenade( cg.snap->ps.weapon )
              && !BG_IsShotgun( cg.snap->ps.weapon )
            )
    {

        if (cg_antiLag.integer) {
            CG_PredictFireLead( );
        }
    }

}




void CG_GrenadeShrapnel( vec3_t org, vec3_t dir ) {
    localEntity_t	*le;
    refEntity_t		*re;

    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    VectorCopy( org, le->pos.trBase );
    VectorScale(dir, 800, le->pos.trDelta);
    le->pos.trType = TR_GRAVITY;
    le->pos.trTime = cg.time;
    le->leType = LE_SHRAPNEL;
    le->startTime = cg.time;
    le->endTime = cg.time + random() * 2000;
    le->lifeRate = 1.0 / ( le->endTime - le->startTime );
    le->radius = 0.7f;
    le->leFlags = cgs.media.sparkShader;
    le->bounceFactor = 0.25f;
}

void CG_RocketShrapnel( vec3_t org, vec3_t dir ) {
    localEntity_t	*le;
    refEntity_t		*re;

    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    VectorCopy( org, le->pos.trBase );
    VectorScale(dir, random() * 400 + 200, le->pos.trDelta);
    le->pos.trType = TR_GRAVITY;
    le->pos.trTime = cg.time;
    le->leType = LE_SHRAPNEL;
    le->startTime = cg.time;
    le->endTime = cg.time + random() * 5000;
    le->lifeRate = 1.0 / ( le->endTime - le->startTime );
    le->radius = 1;
    le->leFlags = cgs.media.sparkShader;
    le->bounceFactor = 0.3f;
}

void CG_Spark( vec3_t org, vec3_t dir, float width ) {
    localEntity_t	*le;
    refEntity_t		*re;

    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    VectorCopy( org, le->pos.trBase );
    VectorScale(dir, 125 + crandom()*30, le->pos.trDelta);
    le->pos.trType = TR_GRAVITY;
    le->pos.trTime = cg.time;
    le->leType = LE_SHRAPNEL;
    le->startTime = cg.time;
    le->endTime = cg.time + 400;
    le->lifeRate = 1.0 / ( le->endTime - le->startTime );
    le->radius = width;
    le->leFlags = cgs.media.sparkShader;//sparkShader;
    le->bounceFactor = 0.5f;
}


void CG_SpawnTracer( vec3_t start, vec3_t end ) {
    localEntity_t	*le;
    refEntity_t		*re;

    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    // rape those vectors
    VectorCopy(start, le->pos.trBase );
    VectorCopy(end, le->angles.trBase );

    le->leType = LE_TRACER;
    le->startTime = cg.time;
    le->endTime = cg.time + 250;
    le->lifeRate = 1.0 / ( le->endTime - le->startTime );
    le->leFlags = cgs.media.sparkShader;
}

void CG_MetalSpark( vec3_t org, vec3_t dir, float width ) {
    localEntity_t	*le;
    refEntity_t		*re;

    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    VectorCopy( org, le->pos.trBase );
    VectorScale(dir, 200 + crandom()*50, le->pos.trDelta);

    /*
    if ( width < 0 ) // is negative ~ the circles have to be linear
    le->pos.trType = TR_GRAVITY;
    else*/
    le->pos.trType = TR_LINEAR;

    le->pos.trTime = cg.time;
    le->leType = LE_SHRAPNEL;
    le->startTime = cg.time;
    le->endTime = cg.time + 300;
    le->lifeRate = 1.0 / ( le->endTime - le->startTime );
    le->radius = width;
    le->leFlags = cgs.media.metalsparkShader;
    le->bounceFactor = 1.0f;

}

/*
=================
CG_LightParticles
=================
*/
void CG_LightParticles( vec3_t origin, vec4_t hcolor, float limit )
{
    vec3_t			ambientLight;
    vec3_t			lightDir;
    vec3_t			directedLight;

    trap_R_LightForPoint( origin, ambientLight, directedLight, lightDir );

    directedLight[0] /= 255;
    directedLight[1] /= 255;
    directedLight[2] /= 255;

    if ( limit > 0.0f )
    {
        if ( directedLight[0] < limit )
            directedLight[0] = limit;
        if ( directedLight[1] < limit )
            directedLight[1] = limit;
        if ( directedLight[2] < limit )
            directedLight[2] = limit;
    }
    hcolor[0] = (directedLight[0] * hcolor[0]);
    hcolor[1] = (directedLight[1] * hcolor[1]);
    hcolor[2] = (directedLight[2] * hcolor[2]);

    return;
}
localEntity_t *CG_SpawnParticle( vec3_t org, vec3_t dir, float speed, float bouncefactor, float radius, float r,float g,float b,float a, qboolean size ) {
    localEntity_t	*le;
    refEntity_t		*re;

    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    VectorCopy( org, le->pos.trBase ); // move to origin vector org
    VectorCopy( org, le->refEntity.origin ); // move to origin vector org
    VectorScale(dir, speed, le->pos.trDelta); // add velocity vector based on speed
    le->pos.trType = TR_GRAVITY;	// movement type
    le->pos.trTime = cg.time;		// set start time of calculation
    le->leType = LE_PARTICLE;		// render as particle
    le->startTime = cg.time;		// start time
    le->endTime = cg.time + cg_particleTime.integer; // time it will be removed from the scene
    le->lifeRate = 1.0 / ( le->endTime - le->startTime );
    le->radius = radius * 0.75f; //
    le->color[0] = r;
    le->color[1] = g;
    le->color[2] = b;
    le->color[3] = a;


    // light particles
    CG_LightParticles( org, le->color, 0.2f );

    le->bounceFactor = bouncefactor;
    le->refEntity.reType = RT_SPRITE;
    le->leBounceSoundType = 0;
    if (!size)
        le->leFlags = LEF_PUFF_DONT_SCALE;
    le->refEntity.customShader = trap_R_RegisterShader("gfx/misc/particle_08.tga");

    return le;

    //	trap_R_AddLightToScene( org, 20, 0.5f, 0.5f, 0.5f );

}
localEntity_t *CG_SpawnBloodParticle( vec3_t org, vec3_t dir, float speed, float bouncefactor, float radius, float r,float g,float b,float a, qboolean size ) {
    localEntity_t	*le;
    refEntity_t		*re;

    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    VectorCopy( org, le->pos.trBase ); // move to origin vector org
    VectorCopy( org, le->refEntity.origin ); // move to origin vector org
    VectorScale(dir, speed, le->pos.trDelta); // add velocity vector based on speed
    le->pos.trType = TR_GRAVITY;	// movement type
    le->pos.trTime = cg.time;		// set start time of calculation
    le->leType = LE_PARTICLE;		// render as particle
    le->startTime = cg.time;		// start time
    le->endTime = cg.time + 600;	// time it will be removed from the scene
    le->lifeRate = 1.0 / ( le->endTime - le->startTime );
    le->radius = radius; //
    le->color[0] = r;
    le->color[1] = g;
    le->color[2] = b;
    le->color[3] = a;
    le->bounceFactor = bouncefactor;
    le->refEntity.reType = RT_SPRITE;
    if ( random() < ( 0.25 * cg_goreLevel.value ) )
        le->leMarkType = LEMT_BLOOD;
    else
        le->leMarkType = LEMT_NONE;
    le->leBounceSoundType = 0;
    if (!size)
        le->leFlags = LEF_PUFF_DONT_SCALE;
    le->refEntity.customShader = cgs.media.bloodparticleShaders[0];

    return le;

    //	trap_R_AddLightToScene( org, 20, 0.5f, 0.5f, 0.5f );

}

void CG_SurfaceEffect( vec3_t origin, vec3_t dir , int surface, int weapon, float radius)
{
    int max = 2 + random()*2;
    int i = 0;

    // se disabled.
    if ( cg_particleTime.integer <= 0 )
        return;

    if ( BG_IsGrenade( weapon ) && cg_grenadeSparks.integer < 2 )
        return;
    /*
    vec3_t testorg;
    qboolean up = qtrue; // add upwards velocity

    VectorCopy( origin, testorg );

    testorg[2] += 1;

    // is the point a solid, if yeah then we can't go up
    if ( trap_CM_PointContents( testorg, 0 ) == CONTENTS_SOLID )
    up = qfalse;
    */
    if ( surface == BHOLE_NORMAL && !BG_IsShotgun( weapon ) )
    {
        // add a strong but small dynamic light on impact
        float spread = 1;
        vec3_t	movedir;
        vec3_t up;
        localEntity_t *smoke;

        VectorCopy ( dir, up );
        up[2] += 2;

        smoke = CG_SmokePuff( origin, up, 2.5 + radius + random(), 1, 1, 1, 1, 1000 + random()*500, cg.time-250, cg.time-250,LE_MOVE_SCALE_FADE, cgs.media.smokePuffShader );

        //		smoke = CG_SmokePuff( origin, up, 0.5 + radius + random(), 1, 1, 1, 1, 1000 + random()*500, cg.time, 0,LE_MOVE_SCALE_FADE, cgs.media.smokePuffShader );

        VectorScale( dir, 5, smoke->pos.trDelta );

        smoke->pos.trDelta[2] += 5;
        smoke->refEntity.renderfx = 0;

        VectorMA( origin, 0.5, dir, smoke->pos.trBase);

        for ( i = 0; i < 4; i++ )
        {
            spread = 1 + random();

            VectorCopy( dir, movedir );

            if ( random() < 0.33 )
                movedir[0] += random()/spread;
            else if ( random() < 0.66 )
                movedir[0] -= random()/spread;

            if ( random() < 0.33 )
                movedir[1] += random()/spread;
            else if ( random() < 0.66 )
                movedir[1] -= random()/spread;

            if ( dir[2] > 0 )
                movedir[2] += 2 + random();
            else if ( dir[2] < 0 )
                movedir[2] -= 2 + random();
            else
                movedir[2] += 1 + random();

            VectorScale( movedir, 0.25f + random()/10 , movedir );

            CG_Spark(origin, movedir, radius/5 );
        }
    }


    if ( surface == BHOLE_METAL )
    {
        // add a strong but small dynamic light on impact
        float value;
        float intensity = 18;
        float spread = 0.1f;
        int		max = 10;
        vec3_t	movedir;

        value = 0.6f + random()/5;

        intensity += random()*4;

        if ( BG_IsShotgun(weapon) )
        {
            max = 2;
        }
        else
            trap_R_AddLightToScene( origin, intensity, value, value, value );

        for ( i = 0; i < 10; i++ )
        {
            spread = 1 + random();

            VectorCopy( dir, movedir );

            if ( random() < 0.33 )
                movedir[0] += random()/spread;
            else if ( random() < 0.66 )
                movedir[0] -= random()/spread;

            if ( random() < 0.33 )
                movedir[1] += random()/spread;
            else if ( random() < 0.66 )
                movedir[1] -= random()/spread;
            /*
            if ( dir[2] > 0 )
            movedir[2] += 2 + random();
            else if ( dir[2] < 0 )
            movedir[2] -= 2 + random();
            else
            movedir[2] += 1 + random();	 */

            VectorScale( movedir, 0.25f + random()/10, movedir );

            if ( random() < 0.4 )
                CG_MetalSpark(origin, movedir, 0.1 + random()/4 );
            else
                CG_MetalSpark(origin, movedir, -(0.2+random()/5) );
        }
        return;
    }
    else if ( surface == BHOLE_SNOW || surface == BHOLE_DIRT || surface == BHOLE_SAND )
    {
        max = 14 + random()*6;

        VectorMA( origin, 0.5, dir, origin);

        if ( BG_IsShotgun( weapon ) )
            max /= 4;

        // launch 4x grenades.
        if ( BG_IsGrenade( weapon ) )
            max *= 4;

        for ( i = 0; i < max; i++ ) {
            vec3_t	movedir;
            float	randomcolor[3];
            float	spread = 3;

            if ( surface == BHOLE_SNOW )
                randomcolor[0] = randomcolor[1] = randomcolor[2] = 0.5 + random()/3;
            else if ( surface == BHOLE_DIRT )
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

            VectorCopy( dir, movedir );

            if ( random() < 0.33 )
                movedir[0] += random()/spread;
            else if ( random() < 0.66 )
                movedir[0] -= random()/spread;

            if ( random() < 0.33 )
                movedir[1] += random()/spread;
            else if ( random() < 0.66 )
                movedir[1] -= random()/spread;

            if ( dir[2] > 0 )
                movedir[2] += 2 + random();
            else if ( dir[2] < 0 )
                movedir[2] -= 2 + random();
            else
                movedir[2] += 1 + random();

            if ( BG_IsGrenade ( weapon ) )
                CG_SpawnParticle( origin, movedir, (surface == BHOLE_SAND)?(300+random()*200):(500 + random()*200), random()/2.5,(surface == BHOLE_SAND)?(random()/2+random()/3 ):( 0.1 + random() + random()/2), randomcolor[0],randomcolor[1],randomcolor[2], 1.0f, qfalse );
            else
                CG_SpawnParticle( origin, movedir, (surface == BHOLE_SAND)?(30+random()*25):(50 + random()*20), random()/2.5,(surface == BHOLE_SAND)?(random()/2+random()/3 ):( 0.1 + random() + random()/2), randomcolor[0],randomcolor[1],randomcolor[2], 1.0f, qfalse );
        }
        return;
    }
    else if ( surface == BHOLE_WOOD || surface == BHOLE_GLASS || surface == BHOLE_SOFT)
    {
        max = 1 + random()*2;

        if ( surface == BHOLE_SOFT )
            max = 3 + random()*2;

        if ( BG_IsShotgun( weapon ) || weapon == WP_PDW )
            max = 1;

        if ( surface == BHOLE_WOOD )
        {
            vec3_t movedir;

            VectorCopy( dir, movedir );

            VectorScale(movedir, 3, movedir);

            CG_SmokePuff( origin, movedir, 14, 0.4f, 0.2f, 0.0, 0.9f, 1000, cg.time-250,cg.time-250,LE_MOVE_SCALE_FADE,cgs.media.smokePuffShader );
        }

        for ( i = 0; i < max; i++ ) {
            localEntity_t *bp;
            vec3_t movedir;

            if ( surface == BHOLE_WOOD )
            {
                bp = CG_MakeExplosion( origin, dir, cgs.media.woodSplinter, 0, cg_particleTime.integer + random()*1000, qfalse );
            }
            else if ( surface == BHOLE_GLASS )
                bp = CG_MakeExplosion( origin, dir, cgs.media.glassSplinter, 0, cg_particleTime.integer + random()*1000, qfalse );
            else if ( surface == BHOLE_SOFT )
                bp = CG_MakeExplosion( origin, dir, cgs.media.softSplinter, 0, cg_particleTime.integer + random()*1000, qfalse );
            else
                return;

            VectorCopy( dir, movedir );

            if ( random() < 0.33 )
                movedir[0] += random();
            else if ( random() < 0.66 )
                movedir[0] -= random();

            if ( random() < 0.33 )
                movedir[1] += random();
            else if ( random() < 0.66 )
                movedir[1] -= random();

            if ( dir[2] > 0 )
                movedir[2] += 2 + random();
            else if ( dir[2] < 0 )
                movedir[2] -= 2 + random();
            else
                movedir[2] += 1 + random();

            VectorMA( origin, 0.5, dir, bp->pos.trBase );

            VectorScale( movedir, 50, bp->pos.trDelta );

            bp->leType = LE_FRAGMENT;
            bp->pos.trType = TR_GRAVITY;
            bp->pos.trTime = cg.time;
            bp->leFlags = LEF_TUMBLE;
            bp->angles.trType = TR_LINEAR;
            bp->angles.trTime = cg.time;
            bp->angles.trBase[0] = rand()&360;
            bp->angles.trBase[1] = rand()&360;
            bp->angles.trBase[2] = rand()&360;

            bp->angles.trDelta[0] = rand() % 150;
            bp->angles.trDelta[1] = rand() % 150;
            bp->angles.trDelta[2] = rand() % 150;

            {
                float size = 0.3 + random() / 2 + random() / 2;

                if ( BG_IsPistol( weapon ) )
                    size /= 3;
                else if ( BG_IsRifle( weapon ) )
                    size /= 2;

                bp->refEntity.axis[0][0] *= size;
                bp->refEntity.axis[1][0] *= size;
                bp->refEntity.axis[2][0] *= size;
                bp->refEntity.axis[0][1] *= size;
                bp->refEntity.axis[1][1] *= size;
                bp->refEntity.axis[2][1] *= size;
                bp->refEntity.axis[0][2] *= size;
                bp->refEntity.axis[1][2] *= size;
                bp->refEntity.axis[2][2] *= size;
            }

            bp->leMarkType = LEMT_NONE;

        }

        max = 4 + random()*2;

        if ( BG_IsShotgun( weapon ) )
            max /= 2;

        for ( i = 0; i < max; i++ ) {
            vec3_t movedir;
            float randombrown[3];

            if ( surface == BHOLE_GLASS ) {
                randombrown[0] = randombrown[1] = randombrown[2] = 0.1 + random()/3;
            }
            else if ( surface == BHOLE_SOFT ) {
                randombrown[0] = 0.45 + random()/4; //we'll create our brown
                randombrown[1] = 0.5 + random()/4;
                randombrown[2] = 0.4 + random()/4;
            }
            else {
                randombrown[0] = 0.3 + random()/3.3; //we'll create our brown
                randombrown[1] = 0.2 + random()/10;
                randombrown[2] = 0;
            }

            VectorCopy( dir, movedir );

            if ( random() < 0.33 )
                movedir[0] += random();
            else if ( random() < 0.66 )
                movedir[0] -= random();

            if ( random() < 0.33 )
                movedir[1] += random();
            else if ( random() < 0.66 )
                movedir[1] -= random();

            if ( dir[2] > 0 )
                movedir[2] += 2 + random();
            else if ( dir[2] < 0 )
                movedir[2] -= 2 + random();
            else
                movedir[2] += 1 + random();

            if ( surface == BHOLE_SOFT )
                CG_SpawnParticle( origin, movedir, 40 + random()*10, random()/2.5, 0.7 - random()/3, randombrown[0],randombrown[1],randombrown[2], 1.0f,qfalse );
            else
                CG_SpawnParticle( origin, movedir, 20 + random()*10, random()/2.5, 0.7 - random()/3, randombrown[0],randombrown[1],randombrown[2], 1.0f,qfalse );
        }
    }
    else
    {
        max = 5 + random()*5;

        if ( BG_IsShotgun( weapon ) )
            max /= 3;

        if ( BG_IsGrenade( weapon ) )
            max = 50;

        for ( i = 0; i < max; i++ ) {
            vec3_t movedir;
            float randomdarkgrey = random()/3;

            VectorCopy( dir, movedir );

            if ( random() < 0.33 )
                movedir[0] += random();
            else if ( random() < 0.66 )
                movedir[0] -= random();

            if ( random() < 0.33 )
                movedir[1] += random();
            else if ( random() < 0.66 )
                movedir[1] -= random();

            if ( dir[2] > 0 )
                movedir[2] += 2 + random();
            else if ( dir[2] < 0 )
                movedir[2] -= 2 + random();
            else
                movedir[2] += 1 + random();

            if ( BG_IsGrenade( weapon ) )
                CG_SpawnParticle( origin, movedir, 100 + random()*25, random()/2.5, 2.0f - random(), randomdarkgrey,randomdarkgrey,randomdarkgrey, 1.0f,qtrue );
            else
                CG_SpawnParticle( origin, movedir, 20 + random()*10, random()/2.5, 0.7 - random()/3, randomdarkgrey,randomdarkgrey,randomdarkgrey, 1.0f,qfalse );
        }
    }
}

int NS_BulletHoleTypeForSurface( int surface );
void CG_DirectImpactMark( qhandle_t markShader, const vec3_t origin, const vec3_t dir,
                          float orientation, float red, float green, float blue, float alpha,
                          qboolean alphaFade, float radius, qboolean temporary, int entityNum );

/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir, int soundtype, int surfaceparms) {
    qhandle_t		mod;
    qhandle_t		mark;
    qhandle_t		shader;
    sfxHandle_t		sfx = 0;
    float			radius;
    float			mradius;
    float			light;
    vec3_t			lightColor;
    localEntity_t	*le; 
    qboolean		isSprite;
    int				duration;
    qboolean		water = ( CG_PointContents( origin, -1 ) & CONTENTS_WATER );
    qboolean		metal = ( surfaceparms == BHOLE_METAL );

    mark = 0;
    mradius = 0;
    radius = 32;
    sfx = 0;
    mod = 0;
    shader = 0;
    light = 0;
    lightColor[0] = 1;
    lightColor[1] = 1;
    lightColor[2] = 0;

    // set defaults
    isSprite = qfalse;
    duration = 120;

    memset( &le, 0, sizeof(le) );

    switch ( weapon  )
    {
    case WP_FLASHBANG:
        mod = cgs.media.sphereFlashModel;
        shader = cgs.media.grenadeExplosionShader;

        if ( water )
            shader = cgs.media.waterExplosionShader;

        sfx = cgs.media.flashbang_explode;
        radius = 128;
        light = 600;
        duration = 500;
        mradius = 64;

        // raise a bit if onground
        origin[2] += 1;
        {
            int i;
            vec3_t	org;

            VectorCopy(origin, org );

            for(i = 0; i < 64; i++)
            {
                vec3_t dir;

                dir[0] = crandom();
                dir[1] = crandom();
                dir[2] = crandom();


                org[2] += i / 64;

                CG_RocketShrapnel(org, dir);
            }
        }
        lightColor[0] = 1;
        lightColor[1] = 1;
        lightColor[2] = 1;

        break;
    case WP_M4*3:
    case WP_AK47*3:
    case WP_GRENADE:
        mod = cgs.media.sphereFlashModel;
        shader = cgs.media.grenadeExplosionShader;
        sfx = cgs.media.mk26_explode;
        radius = 128;
        light = 300;
        duration = 1000;
        mradius = 64;

        if ( water )
            shader = cgs.media.waterExplosionShader;

        // raise a bit
        origin[2] += 1;
        {
            int i;
            vec3_t	org;

            VectorCopy(origin, org );

            for(i = 0; i < 64; i++)
            {
                vec3_t dir;

                dir[0] = crandom();
                dir[1] = crandom();
                dir[2] = crandom();

                org[2] += i / 64;

                CG_GrenadeShrapnel(org, dir);
            }
        }
        lightColor[0] = 1;
        lightColor[1] = 0.75;
        lightColor[2] = 0.0;
        break;
    case WP_MACMILLAN:
#ifdef SL8SD
    case WP_SL8SD:
#endif
    case WP_PSG1:
        mod = cgs.media.bulletFlashModel;
        shader = cgs.media.bulletExplosionShader; 
        radius = 3;
        break;

    case WP_SW629:
    case WP_DEAGLE:
        mod = cgs.media.bulletFlashModel;
        shader = cgs.media.bulletExplosionShader; 
        if (weapon == WP_DEAGLE)
            radius = 3.3f;
        else
            radius = 3;
        break;

    case WP_MAC10:
    case WP_GLOCK:
    case WP_MK23:
    case WP_SW40T:
    case WP_PDW:
    case WP_P9S:
    case WP_MP5:
        mod = cgs.media.bulletFlashModel;
        shader = cgs.media.bulletExplosionShader; 
        radius = 1.5; 
        break;
    case WP_M249:
    case WP_M14:
    case WP_M4:
    case WP_AK47:
        mod = cgs.media.bulletFlashModel;
        shader = cgs.media.bulletExplosionShader; 
        if ( weapon == WP_M14 )
            radius = 2.85f;
        else
            radius = 2.5;
        break;
    case WP_SPAS15:
    case WP_M590:
    case WP_870: // default shotgun
        mod = cgs.media.bulletFlashModel;
        shader = cgs.media.bulletExplosionShader;  
        radius = 2+random()*2.5;
        break;
    default:
        return;
        break;

    }

    if ( soundtype == 1 ) // kevlar
    {
        int rndnum = (int)(random()*2);

        if ((cg.DeafTime < cg.time)) trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.bulletHitKevlar[rndnum] );
    }
    else if ( soundtype == 2 )
    {
        int rndnum = (int)(random()*3);

        if ( cg.DeafTime < cg.time ) 
			trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.bulletHitHelmet[rndnum] );
    }
	else if ( BG_IsGrenade(weapon) ) 
	{
        if ( cg.DeafTime < cg.time )
			trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
    }
    else  
    {
		sfxHandle_t impactSfx = cgs.media.sfx_ric[IMPACTSOUND_DEFAULT]; 
		float		rollOff = 0.12f;
		float		volume =  1.0f + random()/5.0f;

        if ((cg.DeafTime < cg.time)) 
		{
			switch ( surfaceparms )
			{
			case BHOLE_NORMAL:
			case BHOLE_STONE:
				impactSfx = cgs.media.sfx_ric[IMPACTSOUND_DEFAULT];
				break;
			case BHOLE_METAL:
				impactSfx = cgs.media.sfx_ric[IMPACTSOUND_METAL]; 
				volume = 1.1 + random()/4.0;
				break;
			case BHOLE_GLASS:
				impactSfx = cgs.media.sfx_ric[IMPACTSOUND_GLASS];
				break;
			case BHOLE_SOFT:
			case BHOLE_SNOW:
				impactSfx = cgs.media.sfx_ric[IMPACTSOUND_SNOW];
				break;
			case BHOLE_DIRT:
			case BHOLE_SAND:
				impactSfx = cgs.media.sfx_ric[IMPACTSOUND_DIRT];
				break;
			case BHOLE_WOOD:
				impactSfx = cgs.media.sfx_ric[IMPACTSOUND_WOOD];
				break;  
			}
			trap_S_StartSoundExtended( origin , volume, rollOff, 0.9f+random()/4.0f, ENTITYNUM_WORLD, CHAN_AUTO, impactSfx );
		}
    }
    

    //
    // create the explosion
    //
    if ( mod )
    {

        // the flash
        if( weapon == WP_FLASHBANG )
        {
            le = CG_MakeExplosion( origin, dir, mod, shader, duration, isSprite );
            le->light = light;
            le->lightColor[0] = 0;
            le->lightColor[1] = 0.5;
            le->lightColor[2] = 1;
            le->refEntity.renderfx |= RF_EXPANDING;
            le->radius = 64;
        }
        else if( weapon == WP_GRENADE || weapon == WP_AK47*3 || weapon == WP_M4*3 )
        {
            le = CG_MakeExplosion( origin, dir, mod, shader, duration, isSprite );
            le->light = 0;
            le->lightColor[0] = 0;
            le->lightColor[1] = 0;
            le->lightColor[2] = 0;
            le->refEntity.renderfx |= RF_EXPANDING;
            le->radius = 32;
            le->angles.trBase[YAW] = random()*360;
            le->startTime = cg.time - 250;
        }

        if ( !metal && ( surfaceparms != BHOLE_SNOW || surfaceparms != BHOLE_GLASS ) )
        {

            le = CG_MakeExplosion( origin, dir, mod, shader, duration, isSprite );

            if ( !le )
                return;

            if(mradius)
                le->refEntity.radius = mradius;
            le->light = light;

            VectorCopy( lightColor, le->lightColor );

            if ( weapon == WP_FLASHBANG || weapon == WP_GRENADE )
                CG_ExplosionSparks( origin );
            // size the explosion smaller depending on caliber
            else if ( !metal )
            {
                le->refEntity.axis[0][0] *= radius * 2 / 10;
                le->refEntity.axis[1][0] *= radius * 2 / 10;
                le->refEntity.axis[2][0] *= radius * 2 / 10;
                le->refEntity.axis[0][1] *= radius * 2 / 10;
                le->refEntity.axis[1][1] *= radius * 2 / 10;
                le->refEntity.axis[2][1] *= radius * 2 / 10;
                le->refEntity.axis[0][2] *= radius * 2 / 10;
                le->refEntity.axis[1][2] *= radius * 2 / 10;
                le->refEntity.axis[2][2] *= radius * 2 / 10;
            }
        }

        if( BG_IsGrenade( weapon ) && ( !metal && ( surfaceparms != BHOLE_SNOW || surfaceparms != BHOLE_GLASS )  ) )
        {
            vec3_t up;
            vec3_t org2;
            int i;

            up[0] = up[1] = 0;
            up[2] = 10 + random()*20;

            if ( !le || le->refEntity.origin == NULL)
                return;

            le->refEntity.renderfx |= RF_EXPANDING;
            le->startTime -= duration * 0.3;

            if ( water )
            {
                for ( i = 0; i < 4; i++ ) {
                    localEntity_t *smoke;

                    VectorCopy( origin, org2 );

                    org2[0] += random()*60 - random()*60;
                    org2[1] += random()*60 - random()*60;
                    org2[2] += random()*5;

                    up[0] = random();
                    up[1] = random();
                    up[2] = random();

                    smoke = CG_SmokePuff( org2, up, 1 + random()*3, 1, 1, 1, 1, 2500 + random()*500, cg.time, 0,LE_MOVE_SCALE_FADE, cgs.media.waterBubbleShader  );
                    smoke->startTime -= 300;
                }
            }
            else
                for ( i = 0; i < 3; i++ ) {
                    localEntity_t *smoke;

                    VectorCopy( origin, org2 );

                    org2[0] += 30 - random()*60;
                    org2[1] += 30 - random()*60;
                    org2[2] += random()*5;

                    smoke = CG_SmokePuff( org2, up, 150 + random()*30, 1, 1, 1, 1, 5000 + random()*2000, cg.time-1000, cg.time-1000,LE_MOVE_SCALE_FADE, cgs.media.smokePuffShader );
                }

        }
    }

    //
    // surface depending effects
    //
    {

        CG_SurfaceEffect( origin, dir, surfaceparms , weapon, radius );
    }

    //
    // impact mark
    //

    // don't spawn impact mark, if we're only here for sparks / debris
    if( soundtype > 0 )
    {
        if ( soundtype > MAX_CLIENTS && soundtype < ENTITYNUM_MAX_NORMAL )
        {
            if (!mark)
            {
                int type;
                int number;
                int orientation = random()*360;

                type = surfaceparms;
                number = random()*5;

                if ( type == BHOLE_METAL )
                    orientation = ( random()*60 ) - 30 ;
                if ( type == BHOLE_GLASS )
                    radius = 12 + radius;
				if ( type == BHOLE_SAND || type == BHOLE_SNOW )
					type = BHOLE_SOFT;

                CG_DirectImpactMark( cgs.media.bulletholes[surfaceparms][number] , origin, dir, orientation, 1,1,1,1, qfalse, radius, qfalse, soundtype);
            }
            else
            {
                CG_DirectImpactMark( mark , origin, dir, random()*360, 1,1,1,1, qfalse, radius, qfalse, soundtype );
            }
        }
        return;
    }


    if (!mark)
    {
        int type;
        int number;
        int orientation = random()*360;

        type = surfaceparms;

        number = random()*5;

        if ( BG_IsGrenade( weapon ) )
        {
            number = random()*2;

            CG_ImpactMark( cgs.media.burnMarkShaders[number], origin,dir, orientation, 0,0,0,1, qfalse, radius, qfalse );
            return;
        }

        if ( type == BHOLE_METAL )
            orientation = ( random()*60 ) - 30 ;
        if ( type == BHOLE_GLASS )
            radius = 12;
		if ( type == BHOLE_SAND || type == BHOLE_SNOW )
			type = BHOLE_SOFT;

        CG_ImpactMark( cgs.media.bulletholes[type][number] , origin, dir, orientation, 1,1,1,1, qfalse, radius, qfalse );
    }
    else
        CG_ImpactMark( mark , origin, dir, random()*360, 1,1,1,1, qfalse, radius, qfalse );

}

/*
=================
CG_Explosion

Caused by an EV_EXPLOSION event, or directly
=================
*/
void CG_Explosion( vec3_t origin, int c4explo ) {
    qhandle_t		mod;
    qhandle_t		mark;
    qhandle_t		shader;
    sfxHandle_t		sfx;
    float			radius;
    float			mradius;
    float			light;
    vec3_t			lightColor;
    localEntity_t	*le;
    qboolean		isSprite;
    int				duration;
    vec3_t			dir;
    int i, r;
    float			alpha;
    trace_t		trace;
    int				number;
    qboolean		water = ( CG_PointContents( origin, -1 ) & CONTENTS_WATER );

    mark = 0;
    mradius = 0;
    radius = 32;
    sfx = 0;
    mod = 0;
    shader = 0;
    light = 0;
    lightColor[0] = 1;
    lightColor[1] = 1;
    lightColor[2] = 0;

    // set defaults
    if (c4explo) {
        isSprite = qfalse;
        mod = cgs.media.sphereFlashModel;
        shader = cgs.media.grenadeExplosionShader;
        if ( water )
            shader = cgs.media.waterExplosionShader;
        sfx = cgs.media.c4_explode;
        radius = 128;
        light = 600;
        duration = 1000;
        mradius = 256 / 2;
    } else {
        isSprite = qfalse;
        mod = cgs.media.sphereFlashModel;
        shader = cgs.media.grenadeExplosionShader;
        if ( water )
            shader = cgs.media.waterExplosionShader;
        sfx = cgs.media.mk26_explode;
        radius = 32;
        light = 400;
        duration = 1000;
        mradius = 128 / 2;
    }

    if (!water)
        // alot fewer chunks than the origin explosion
        for(i = 0; i < 16; i++)
        {
            r = rand() & 255;
            CG_GrenadeShrapnel(origin, bytedirs[r]);
        }

    if ( sfx ) {
        if ((cg.DeafTime < cg.time)) trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
    }

    le = CG_MakeExplosion( origin, origin,
                           mod,	shader,
                           duration, isSprite );

    le->light = light;

    VectorCopy( lightColor, le->lightColor );

    //le->refEntity.customShader = ;//cgs.media.plasmaBallShader;
    le->refEntity.renderfx |= RF_EXPANDING;
    le->startTime -= duration * 0.3; // make it start out with some size

    // send a trace down from the origin to the ground
    VectorCopy( origin, dir );
    dir[2] -= 128; // trace down max 128

    trap_CM_BoxTrace( &trace, origin, dir, NULL, NULL, 0, MASK_PLAYERSOLID );

    // no mark if we're too high
    if ( trace.fraction == 1.0 ) {
        return;
    }

    // fade the mark out with height
    alpha = 1.0 - trace.fraction;

    number = random()*2;


    CG_ImpactMark( cgs.media.burnMarkShaders[number], trace.endpos,trace.plane.normal,
                   random()*360, 0,0,0,1, qfalse, 128, qfalse );
}

#define BLOOD_MINI		0
#define BLOOD_SMALL		1
#define BLOOD_NORMAL	2
#define	BLOOD_NORMAL2	3
#define BLOOD_BIG		4

static void CG_BloodParticle( vec3_t org, vec3_t dir, int size ) {
    localEntity_t	*le;
    refEntity_t		*re;

    if (!cg_blood.integer)
        return;

    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    VectorCopy( org, le->pos.trBase );

    VectorScale(dir, 75 + random()*20*cg_goreLevel.value, le->pos.trDelta);
    le->pos.trType = TR_GRAVITY;
    le->pos.trTime = cg.time;
    le->leType = LE_SHRAPNEL;
    le->startTime = cg.time;
    le->endTime = cg.time + 1500;
    le->lifeRate = 1.0 / ( le->endTime - le->startTime );

    switch (size) {
    case BLOOD_MINI:
        le->radius = 0.5 + random(); // base is 0.5 max is 1.5
        le->leFlags = cgs.media.bloodparticleShaders[BLOOD_MINI];
        break;
    case BLOOD_SMALL:
        le->radius = 1 + random();   // base is 1 max is 2
        le->leFlags = cgs.media.bloodparticleShaders[BLOOD_SMALL];
        break;
    case BLOOD_NORMAL:
        le->radius = 1.5 + random(); // base is 1.5 max is 2.5
        le->leFlags = cgs.media.bloodparticleShaders[BLOOD_NORMAL];
        break;
    case BLOOD_NORMAL2:
        le->radius = 2 + random();   // base is 2 max is 3
        le->leFlags = cgs.media.bloodparticleShaders[BLOOD_NORMAL2];
        break;
    case BLOOD_BIG:
        le->radius = 2.5 + random(); // base is 2.5 max is 3.5
        le->leFlags = cgs.media.bloodparticleShaders[BLOOD_BIG];
        break;
    default:
        break;
    }

    le->bounceFactor = 0.1f;

    le->leMarkType = LEMT_BLOOD;



}

/*
=================
CG_PlayerBleed

Caused by ev_bullet
=================
*/
void CG_PlayerBleed( int weapon, int clientNum, int damage, vec3_t origin, vec3_t dir ) {
    //	int				r;
    int			i;//, s;
    qboolean	headshot = qfalse;
    vec3_t dir2;

    if (!cg_blood.integer)
        return;

    if ( damage >= 100 )
        headshot = qtrue;


    if (damage > 30)
        damage = 30;


    if (headshot)
        damage = 100;

    damage *= cg_goreLevel.value;


    CG_SmokePuff( origin, dir, 5, 1.0f, 0.2f, 0.2f, 0.8f, 750, cg.time-250,cg.time-250,LE_MOVE_SCALE_FADE,cgs.media.smokePuffShader );

    for(i = 0; i < damage; i++)
    {
        if ( headshot )
        {
            int r,s;

            r = rand() & 255;
            VectorCopy(bytedirs[r], dir2);
            s = DotProduct(dir, dir2);

            if(s < 0)
                VectorMA(dir2, -2 * s, dir, dir2);
            VectorNormalize(dir2);

            dir[2] += 2;

            CG_SpawnBloodParticle( origin, dir2, 100 + random()*50 , 0.0f, 4.0f +random(), 0.4+random()/10,0.1f,0.1f,1.0f, qtrue );
        }
        else
        {
            VectorCopy( dir, dir2 );

            dir2[0] += -0.50 + random();
            dir2[1] += -0.50 + random();
            dir2[2] -= random();

            CG_SpawnBloodParticle( origin, dir2, 60 + i*3, 0.0f, 3.0f +random()/2.0f, 0.4+random()/10,0.1f,0.1f,1.0f, qtrue );
        }
    }
}

/*
=================
CG_MissileHitPlayer
=================
*/
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum, int damage ) {
    CG_Bleed( origin, entityNum );
    CG_PlayerBleed( weapon, entityNum, damage, origin, dir );

    switch ( weapon ) {
    case WP_FLASHBANG:
    case WP_GRENADE:
    case WP_SMOKE:
        CG_MissileHitWall( weapon, 0, origin, dir, 0, 0 );
        break;
    default:
        break;
    }
}



/*
============================================================================

SHOTGUN TRACING

============================================================================
*/

/*
================
CG_ShotgunPellet
================
*/
static void CG_ShotgunPellet( vec3_t start, vec3_t end, vec3_t forward, int skipNum ) {
    trace_t		tr;
    int sourceContentType, destContentType;

    CG_Trace( &tr, start, NULL, NULL, end, skipNum, MASK_SHOT );

    sourceContentType = trap_CM_PointContents( start, 0 );
    destContentType = trap_CM_PointContents( tr.endpos, 0 );

    // FIXME: should probably move this cruft into CG_BubbleTrail
    if ( sourceContentType == destContentType ) {
        if ( sourceContentType & CONTENTS_WATER ) {
            CG_BubbleTrail( start, tr.endpos, 8 );
        }
    } else if ( sourceContentType & CONTENTS_WATER ) {
        trace_t trace;

        trap_CM_BoxTrace( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
        CG_BubbleTrail( start, trace.endpos, 8 );

        CG_ImpactMark(cgs.media.wakeMarkShader, trace.endpos, trace.plane.normal , random()*360,1,1,1,1,qfalse, 4, qfalse );
    } else if ( destContentType & CONTENTS_WATER ) {
        trace_t trace;

        trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
        CG_BubbleTrail( tr.endpos, trace.endpos, 8 );

        CG_ImpactMark(cgs.media.wakeMarkShader, trace.endpos, trace.plane.normal , random()*360,1,1,1,1,qfalse, 4, qfalse );
    }

    // really thin shotgun tracers
    if ( random() < cg_tracerChance.value ) {
        vec4_t rgba;
        vec3_t	forward;
        vec3_t	trace_start;

        // tracer
        VectorSubtract( end, start, forward );
        VectorMA( start, 32.0, forward, trace_start );

        rgba[0] = rgba[1] = rgba[2] = rgba[3] = 1;

        CG_Tracer(trace_start, end, 1, cgs.media.tracerShader, rgba);
    }

    if (  tr.surfaceFlags & SURF_NOIMPACT ) {
        return;
    }

    if ( cg_entities[tr.entityNum].currentState.eType == ET_PLAYER ) {
        // if the player got a vest
        if ( cg_entities[tr.entityNum].currentState.powerups & ( 1 << PW_VEST ) || cg_entities[tr.entityNum].currentState.powerups & ( 1 << PW_HELMET ) )
        {
            int viewheight;
            int anim;
            float pos;
            float h_1 = 1.125;

            anim = cg_entities[tr.entityNum].currentState.legsAnim & ~ANIM_TOGGLEBIT;

            if ( anim == LEGS_WALKCR || anim == LEGS_IDLECR )
                viewheight = CROUCH_VIEWHEIGHT;
            else
                viewheight = DEFAULT_VIEWHEIGHT;

            pos = cg_entities[tr.entityNum].currentState.origin[2] + viewheight - tr.endpos[2];

            // if above legs and below head hit the wall(the vest - just same gfx)
            if ( cg_entities[tr.entityNum].currentState.powerups & ( 1 << PW_HELMET ) )
                h_1 = 0.1f;

            if ( pos < 18 && pos > h_1 )  {
                // reverse
                forward[0] *= -1;
                forward[1] *= -1;
                forward[2] *= -1;

                CG_MissileHitWall( WP_870, 0, tr.endpos, forward, 1, NS_BulletHoleTypeForSurface(tr.surfaceFlags) );
            }
            else
                CG_MissileHitPlayer( WP_870, tr.endpos, forward, tr.entityNum, 4 );
        }
        else
            CG_MissileHitPlayer( WP_870, tr.endpos, forward, tr.entityNum, 4 );
    } else {
        if ( tr.surfaceFlags & SURF_NOIMPACT ) {
            // SURF_NOIMPACT will not make a flame puff or a mark
            return;
        }
        CG_MissileHitWall( WP_870, 0, tr.endpos, tr.plane.normal, 0, NS_BulletHoleTypeForSurface(tr.surfaceFlags));
    }
}
float BG_MaximumWeaponRange( int weapon );


/*
================
CG_BounceProjectile
================
*/
void CG_BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout ) {
    vec3_t v, newv;
    float dot;

    VectorSubtract( impact, start, v );
    dot = DotProduct( v, dir );
    VectorMA( v, -2*dot, dir, newv );

    VectorNormalize(newv);
    VectorMA(impact, 8192, newv, endout);
}

void CG_PredictFireLead ( void )
{
    vec3_t		end;
    trace_t		trace,trace2;
    int			hits;
    vec3_t		tracefrom, start, temp, forward,right,up;
    int			wallhits, bulletHits;
    float		bulletThickn;
    int			ignoreent;//,savedent;
    double		r = 0,u = 0;
    float		max_range = BG_MaximumWeaponRange ( cg.predictedPlayerState.weapon );
    int			*seed = &cg.predictedPlayerState.stats[STAT_SEED];
    int			entityNum;

    gitem_t		*item = BG_FindItemForWeapon( cg.predictedPlayerState.weapon );
    int			caliber = item->giAmmoTag;

    hits = wallhits = 0;

    if ( BG_WeaponMods( cg.predictedPlayerState.weapon ) & WM_GRENADELAUNCHER )
    {
        // no prediction if firing a grenade
        if ( cg.predictedPlayerState.stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ) )
            return;
    }

    ignoreent = cg.predictedPlayerState.clientNum;
    bulletThickn = MAX_9MM_BULLET_THICKN;
    bulletHits = MAX_BULLET_HITS;

    // BLUTENGEL:
    // modified whole code for calculating weapon recoil

    // BLUTENGEL:
    //  SYNCHRONIZE THIS CODE ALWAYS WITH THE CODE IN g_weapons.c:

    // snipers
    if (SEALS_IS_SNIPER(cg.predictedPlayerState.weapon)) {
        switch(BG_CalcSpeed(cg.predictedPlayerState)) {
        case SEALS_STANDING:
            r = SEALS_SNIPER_DEFLECTION_STANDING - 2.0 * Q_random(seed) * SEALS_SNIPER_DEFLECTION_STANDING;
            u = SEALS_SNIPER_DEFLECTION_STANDING - 2.0 * Q_random(seed) * SEALS_SNIPER_DEFLECTION_STANDING;
            break;
        case SEALS_CROUCHING:
            r = SEALS_SNIPER_DEFLECTION_CROUCHING - 2.0 * Q_random(seed) * SEALS_SNIPER_DEFLECTION_CROUCHING;
            u = SEALS_SNIPER_DEFLECTION_CROUCHING - 2.0 * Q_random(seed) * SEALS_SNIPER_DEFLECTION_CROUCHING;
            break;
        case SEALS_WALKING:
            r = SEALS_SNIPER_DEFLECTION_WALKING - 2.0 * Q_random(seed) * SEALS_SNIPER_DEFLECTION_WALKING;
            u = SEALS_SNIPER_DEFLECTION_WALKING - 2.0 * Q_random(seed) * SEALS_SNIPER_DEFLECTION_WALKING;
            break;
        case SEALS_RUNNING:
            r = SEALS_SNIPER_DEFLECTION_RUNNING - 2.0 * Q_random(seed) * SEALS_SNIPER_DEFLECTION_RUNNING;
            u = SEALS_SNIPER_DEFLECTION_RUNNING - 2.0 * Q_random(seed) * SEALS_SNIPER_DEFLECTION_RUNNING;
            break;
        case SEALS_SPRINTING:
            r = SEALS_SNIPER_DEFLECTION_SPRINTING - 2.0 * Q_random(seed) * SEALS_SNIPER_DEFLECTION_SPRINTING;
            u = SEALS_SNIPER_DEFLECTION_SPRINTING - 2.0 * Q_random(seed) * SEALS_SNIPER_DEFLECTION_SPRINTING;
            break;
        case SEALS_JUMPING:
            r = SEALS_SNIPER_DEFLECTION_JUMPING - 2.0 * Q_random(seed) * SEALS_SNIPER_DEFLECTION_JUMPING;
            u = SEALS_SNIPER_DEFLECTION_JUMPING - 2.0 * Q_random(seed) * SEALS_SNIPER_DEFLECTION_JUMPING;
            break;
        default:
            r = SEALS_SNIPER_DEFLECTION_OTHER - 2.0 * Q_random(seed) * SEALS_SNIPER_DEFLECTION_OTHER;
            u = SEALS_SNIPER_DEFLECTION_OTHER - 2.0 * Q_random(seed) * SEALS_SNIPER_DEFLECTION_OTHER;
            break;
        }
        // rifles
    } else if (SEALS_IS_RIFLE(cg.predictedPlayerState.weapon)) {
        switch(BG_CalcSpeed(cg.predictedPlayerState)) {
        case SEALS_STANDING:
            r = SEALS_RIFLE_DEFLECTION_STANDING - 2.0 * Q_random(seed) * SEALS_RIFLE_DEFLECTION_STANDING;
            u = SEALS_RIFLE_DEFLECTION_STANDING - 2.0 * Q_random(seed) * SEALS_RIFLE_DEFLECTION_STANDING;
            break;
        case SEALS_CROUCHING:
            r = SEALS_RIFLE_DEFLECTION_CROUCHING - 2.0 * Q_random(seed) * SEALS_RIFLE_DEFLECTION_CROUCHING;
            u = SEALS_RIFLE_DEFLECTION_CROUCHING - 2.0 * Q_random(seed) * SEALS_RIFLE_DEFLECTION_CROUCHING;
            break;
        case SEALS_WALKING:
            r = SEALS_RIFLE_DEFLECTION_WALKING - 2.0 * Q_random(seed) * SEALS_RIFLE_DEFLECTION_WALKING;
            u = SEALS_RIFLE_DEFLECTION_WALKING - 2.0 * Q_random(seed) * SEALS_RIFLE_DEFLECTION_WALKING;
            break;
        case SEALS_RUNNING:
            r = SEALS_RIFLE_DEFLECTION_RUNNING - 2.0 * Q_random(seed) * SEALS_RIFLE_DEFLECTION_RUNNING;
            u = SEALS_RIFLE_DEFLECTION_RUNNING - 2.0 * Q_random(seed) * SEALS_RIFLE_DEFLECTION_RUNNING;
            break;
        case SEALS_SPRINTING:
            r = SEALS_RIFLE_DEFLECTION_SPRINTING - 2.0 * Q_random(seed) * SEALS_RIFLE_DEFLECTION_SPRINTING;
            u = SEALS_RIFLE_DEFLECTION_SPRINTING - 2.0 * Q_random(seed) * SEALS_RIFLE_DEFLECTION_SPRINTING;
            break;
        case SEALS_JUMPING:
            r = SEALS_RIFLE_DEFLECTION_JUMPING - 2.0 * Q_random(seed) * SEALS_RIFLE_DEFLECTION_JUMPING;
            u = SEALS_RIFLE_DEFLECTION_JUMPING - 2.0 * Q_random(seed) * SEALS_RIFLE_DEFLECTION_JUMPING;
            break;
        default:
            r = SEALS_RIFLE_DEFLECTION_OTHER - 2.0 * Q_random(seed) * SEALS_RIFLE_DEFLECTION_OTHER;
            u = SEALS_RIFLE_DEFLECTION_OTHER - 2.0 * Q_random(seed) * SEALS_RIFLE_DEFLECTION_OTHER;
            break;
        }
        // smgs
    } else if (SEALS_IS_SMG(cg.predictedPlayerState.weapon)) {
        switch(BG_CalcSpeed(cg.predictedPlayerState)) {
        case SEALS_STANDING:
            r = SEALS_SMG_DEFLECTION_STANDING - 2.0 * Q_random(seed) * SEALS_SMG_DEFLECTION_STANDING;
            u = SEALS_SMG_DEFLECTION_STANDING - 2.0 * Q_random(seed) * SEALS_SMG_DEFLECTION_STANDING;
            break;
        case SEALS_CROUCHING:
            r = SEALS_SMG_DEFLECTION_CROUCHING - 2.0 * Q_random(seed) * SEALS_SMG_DEFLECTION_CROUCHING;
            u = SEALS_SMG_DEFLECTION_CROUCHING - 2.0 * Q_random(seed) * SEALS_SMG_DEFLECTION_CROUCHING;
            break;
        case SEALS_WALKING:
            r = SEALS_SMG_DEFLECTION_WALKING - 2.0 * Q_random(seed) * SEALS_SMG_DEFLECTION_WALKING;
            u = SEALS_SMG_DEFLECTION_WALKING - 2.0 * Q_random(seed) * SEALS_SMG_DEFLECTION_WALKING;
            break;
        case SEALS_RUNNING:
            r = SEALS_SMG_DEFLECTION_RUNNING - 2.0 * Q_random(seed) * SEALS_SMG_DEFLECTION_RUNNING;
            u = SEALS_SMG_DEFLECTION_RUNNING - 2.0 * Q_random(seed) * SEALS_SMG_DEFLECTION_RUNNING;
            break;
        case SEALS_SPRINTING:
            r = SEALS_SMG_DEFLECTION_SPRINTING - 2.0 * Q_random(seed) * SEALS_SMG_DEFLECTION_SPRINTING;
            u = SEALS_SMG_DEFLECTION_SPRINTING - 2.0 * Q_random(seed) * SEALS_SMG_DEFLECTION_SPRINTING;
            break;
        case SEALS_JUMPING:
            r = SEALS_SMG_DEFLECTION_JUMPING - 2.0 * Q_random(seed) * SEALS_SMG_DEFLECTION_JUMPING;
            u = SEALS_SMG_DEFLECTION_JUMPING - 2.0 * Q_random(seed) * SEALS_SMG_DEFLECTION_JUMPING;
            break;
        default:
            r = SEALS_SMG_DEFLECTION_OTHER - 2.0 * Q_random(seed) * SEALS_SMG_DEFLECTION_OTHER;
            u = SEALS_SMG_DEFLECTION_OTHER - 2.0 * Q_random(seed) * SEALS_SMG_DEFLECTION_OTHER;
            break;
        }
        // pistols
    } else if (SEALS_IS_PISTOL(cg.predictedPlayerState.weapon)) {
        switch(BG_CalcSpeed(cg.predictedPlayerState)) {
        case SEALS_STANDING:
            r = SEALS_PISTOL_DEFLECTION_STANDING - 2.0 * Q_random(seed) * SEALS_PISTOL_DEFLECTION_STANDING;
            u = SEALS_PISTOL_DEFLECTION_STANDING - 2.0 * Q_random(seed) * SEALS_PISTOL_DEFLECTION_STANDING;
            break;
        case SEALS_CROUCHING:
            r = SEALS_PISTOL_DEFLECTION_CROUCHING - 2.0 * Q_random(seed) * SEALS_PISTOL_DEFLECTION_CROUCHING;
            u = SEALS_PISTOL_DEFLECTION_CROUCHING - 2.0 * Q_random(seed) * SEALS_PISTOL_DEFLECTION_CROUCHING;
            break;
        case SEALS_WALKING:
            r = SEALS_PISTOL_DEFLECTION_WALKING - 2.0 * Q_random(seed) * SEALS_PISTOL_DEFLECTION_WALKING;
            u = SEALS_PISTOL_DEFLECTION_WALKING - 2.0 * Q_random(seed) * SEALS_PISTOL_DEFLECTION_WALKING;
            break;
        case SEALS_RUNNING:
            r = SEALS_PISTOL_DEFLECTION_RUNNING - 2.0 * Q_random(seed) * SEALS_PISTOL_DEFLECTION_RUNNING;
            u = SEALS_PISTOL_DEFLECTION_RUNNING - 2.0 * Q_random(seed) * SEALS_PISTOL_DEFLECTION_RUNNING;
            break;
        case SEALS_SPRINTING:
            r = SEALS_PISTOL_DEFLECTION_SPRINTING - 2.0 * Q_random(seed) * SEALS_PISTOL_DEFLECTION_SPRINTING;
            u = SEALS_PISTOL_DEFLECTION_SPRINTING - 2.0 * Q_random(seed) * SEALS_PISTOL_DEFLECTION_SPRINTING;
            break;
        case SEALS_JUMPING:
            r = SEALS_PISTOL_DEFLECTION_JUMPING - 2.0 * Q_random(seed) * SEALS_PISTOL_DEFLECTION_JUMPING;
            u = SEALS_PISTOL_DEFLECTION_JUMPING - 2.0 * Q_random(seed) * SEALS_PISTOL_DEFLECTION_JUMPING;
            break;
        default:
            r = SEALS_PISTOL_DEFLECTION_OTHER - 2.0 * Q_random(seed) * SEALS_PISTOL_DEFLECTION_OTHER;
            u = SEALS_PISTOL_DEFLECTION_OTHER - 2.0 * Q_random(seed) * SEALS_PISTOL_DEFLECTION_OTHER;
            break;
        }
        // shotguns
    } else if (SEALS_IS_SHOTGUN(cg.predictedPlayerState.weapon)) {
        switch(BG_CalcSpeed(cg.predictedPlayerState)) {
        case SEALS_STANDING:
            r = SEALS_SHOTGUN_DEFLECTION_STANDING - 2.0 * Q_random(seed) * SEALS_SHOTGUN_DEFLECTION_STANDING;
            u = SEALS_SHOTGUN_DEFLECTION_STANDING - 2.0 * Q_random(seed) * SEALS_SHOTGUN_DEFLECTION_STANDING;
            break;
        case SEALS_CROUCHING:
            r = SEALS_SHOTGUN_DEFLECTION_CROUCHING - 2.0 * Q_random(seed) * SEALS_SHOTGUN_DEFLECTION_CROUCHING;
            u = SEALS_SHOTGUN_DEFLECTION_CROUCHING - 2.0 * Q_random(seed) * SEALS_SHOTGUN_DEFLECTION_CROUCHING;
            break;
        case SEALS_WALKING:
            r = SEALS_SHOTGUN_DEFLECTION_WALKING - 2.0 * Q_random(seed) * SEALS_SHOTGUN_DEFLECTION_WALKING;
            u = SEALS_SHOTGUN_DEFLECTION_WALKING - 2.0 * Q_random(seed) * SEALS_SHOTGUN_DEFLECTION_WALKING;
            break;
        case SEALS_RUNNING:
            r = SEALS_SHOTGUN_DEFLECTION_RUNNING - 2.0 * Q_random(seed) * SEALS_SHOTGUN_DEFLECTION_RUNNING;
            u = SEALS_SHOTGUN_DEFLECTION_RUNNING - 2.0 * Q_random(seed) * SEALS_SHOTGUN_DEFLECTION_RUNNING;
            break;
        case SEALS_SPRINTING:
            r = SEALS_SHOTGUN_DEFLECTION_SPRINTING - 2.0 * Q_random(seed) * SEALS_SHOTGUN_DEFLECTION_SPRINTING;
            u = SEALS_SHOTGUN_DEFLECTION_SPRINTING - 2.0 * Q_random(seed) * SEALS_SHOTGUN_DEFLECTION_SPRINTING;
            break;
        case SEALS_JUMPING:
            r = SEALS_SHOTGUN_DEFLECTION_JUMPING - 2.0 * Q_random(seed) * SEALS_SHOTGUN_DEFLECTION_JUMPING;
            u = SEALS_SHOTGUN_DEFLECTION_JUMPING - 2.0 * Q_random(seed) * SEALS_SHOTGUN_DEFLECTION_JUMPING;
            break;
        default:
            r = SEALS_SHOTGUN_DEFLECTION_OTHER - 2.0 * Q_random(seed) * SEALS_SHOTGUN_DEFLECTION_OTHER;
            u = SEALS_SHOTGUN_DEFLECTION_OTHER - 2.0 * Q_random(seed) * SEALS_SHOTGUN_DEFLECTION_OTHER;
            break;
        }
        // mgs
    } else if (SEALS_IS_MG(cg.predictedPlayerState.weapon)) {
        switch(BG_CalcSpeed(cg.predictedPlayerState)) {
        case SEALS_STANDING:
            r = SEALS_MG_DEFLECTION_STANDING - 2.0 * Q_random(seed) * SEALS_MG_DEFLECTION_STANDING;
            u = SEALS_MG_DEFLECTION_STANDING - 2.0 * Q_random(seed) * SEALS_MG_DEFLECTION_STANDING;
            break;
        case SEALS_CROUCHING:
            r = SEALS_MG_DEFLECTION_CROUCHING - 2.0 * Q_random(seed) * SEALS_MG_DEFLECTION_CROUCHING;
            u = SEALS_MG_DEFLECTION_CROUCHING - 2.0 * Q_random(seed) * SEALS_MG_DEFLECTION_CROUCHING;
            break;
        case SEALS_WALKING:
            r = SEALS_MG_DEFLECTION_WALKING - 2.0 * Q_random(seed) * SEALS_MG_DEFLECTION_WALKING;
            u = SEALS_MG_DEFLECTION_WALKING - 2.0 * Q_random(seed) * SEALS_MG_DEFLECTION_WALKING;
            break;
        case SEALS_RUNNING:
            r = SEALS_MG_DEFLECTION_RUNNING - 2.0 * Q_random(seed) * SEALS_MG_DEFLECTION_RUNNING;
            u = SEALS_MG_DEFLECTION_RUNNING - 2.0 * Q_random(seed) * SEALS_MG_DEFLECTION_RUNNING;
            break;
        case SEALS_SPRINTING:
            r = SEALS_MG_DEFLECTION_SPRINTING - 2.0 * Q_random(seed) * SEALS_MG_DEFLECTION_SPRINTING;
            u = SEALS_MG_DEFLECTION_SPRINTING - 2.0 * Q_random(seed) * SEALS_MG_DEFLECTION_SPRINTING;
            break;
        case SEALS_JUMPING:
            r = SEALS_MG_DEFLECTION_JUMPING - 2.0 * Q_random(seed) * SEALS_MG_DEFLECTION_JUMPING;
            u = SEALS_MG_DEFLECTION_JUMPING - 2.0 * Q_random(seed) * SEALS_MG_DEFLECTION_JUMPING;
            break;
        default:
            r = SEALS_MG_DEFLECTION_OTHER - 2.0 * Q_random(seed) * SEALS_MG_DEFLECTION_OTHER;
            u = SEALS_MG_DEFLECTION_OTHER - 2.0 * Q_random(seed) * SEALS_MG_DEFLECTION_OTHER;
            break;
        }
    }

    //CG_Printf("client: %f %f\n", Q_random(seed), Q_random(seed));

    // modify the according to the accuracy
    if (! SEALS_IS_SHOTGUN(cg.predictedPlayerState.weapon) ) {
        float acc = cg.snap->ps.persistant[PERS_ACCURACY];

        r *=  SEALS_ACCURACY_MOD + (1.0 - SEALS_ACCURACY_MOD) * (1.0 - (acc-1.0)/9.0);
        u *=  SEALS_ACCURACY_MOD + (1.0 - SEALS_ACCURACY_MOD) * (1.0 - (acc-1.0)/9.0);
    }

    // modify if the weapon is in scope mode
    if (BG_IsZooming( cg.predictedPlayerState.stats[STAT_WEAPONMODE])) {
        r *= SEALS_WEAP_SCOPE_MOD;
        u *= SEALS_WEAP_SCOPE_MOD;
    }

    // modify if the weapon uses an attached laser and is a smg
    if ( BG_HasLaser(cg.predictedPlayerState.stats[STAT_WEAPONMODE]) &&
            SEALS_IS_SMG(cg.predictedPlayerState.weapon) &&
            ( cg.predictedPlayerState.stats[STAT_WEAPONMODE] & (1 << WM_LACTIVE) ) ) {
        r *= SEALS_WEAP_LASER_MOD;
        u *= SEALS_WEAP_LASER_MOD;
    }

    // modify if the weapon uses an attached laser and is a pistol
    if ( BG_HasLaser(cg.predictedPlayerState.stats[STAT_WEAPONMODE]) &&
            SEALS_IS_PISTOL(cg.predictedPlayerState.weapon) &&
            ( cg.predictedPlayerState.stats[STAT_WEAPONMODE] & (1 << WM_LACTIVE) ) ) {
        r *= SEALS_WEAP_PISTOLLASER_MOD;
        u *= SEALS_WEAP_PISTOLLASER_MOD;
    }

    // modify if the weapon is a pdw in secondary mode
    if ( (cg.predictedPlayerState.stats[STAT_WEAPONMODE] & (1 << WM_WEAPONMODE2)) && (cg.predictedPlayerState.weapon == WP_PDW) ) {
        r *= SEALS_WEAP_PDW_MOD;
        u *= SEALS_WEAP_PDW_MOD;
    }

    // modify if the weapon is shooted crouched
    if ( (cg.predictedPlayerState.viewheight == CROUCH_VIEWHEIGHT) &&
            !SEALS_IS_SHOTGUN(cg.predictedPlayerState.weapon) &&
            !SEALS_IS_PISTOL(cg.predictedPlayerState.weapon) &&
            !(cg.predictedPlayerState.weapon == WP_PDW) ) {
        r *= SEALS_WEAP_CROUCH_MOD;
        u *= SEALS_WEAP_CROUCH_MOD;
    }

    // modifiy if the weapon has a duckbill
    if ( SEALS_IS_SHOTGUN(cg.predictedPlayerState.weapon) && ( cg.predictedPlayerState.stats[STAT_WEAPONMODE] & ( 1 << WM_DUCKBILL ) ) ) {
        r *= SEALS_DUCKBILL_HOR_MOD;
        u *= SEALS_DUCKBILL_VER_MOD;
    }



    // copy viewangles to end
    VectorCopy(cg.predictedPlayerState.viewangles, end);
    // modify viewangles if recoil should be added
    if ( r || u ) {
        end[YAW] += r;
        end[PITCH] += u;
    }

    AngleVectors( end, forward, right, up );

    VectorCopy( cg.predictedPlayerState.origin, start );
    start[2] += cg.predictedPlayerState.viewheight;

    VectorMA (start, max_range, forward, end);

    VectorCopy ( start,tracefrom);

    //	CG_Printf("Lead: Predicting bullet seed: %f\n", Q_crandom( seed ) );

    firstshot = qtrue;

    do {
        //savedent = 0;

        CG_Trace (&trace, tracefrom, NULL, NULL, end, ignoreent, MASK_SHOT );
        if ( trace.entityNum >= ENTITYNUM_MAX_NORMAL )
        {
            if (trace.surfaceFlags & SURF_SKY)
                break;
            if (trace.allsolid || trace.startsolid)// it should start free
                break;
            if (wallhits >= bulletHits)
                break;
            // gone?
            if ( trace.fraction >= 1 )
                break;

            //			CG_Printf("Lead: Render FX at: %s\n", vtos(trace.endpos) );
            // hit! now render the effect
            CG_Bullet( trace.endpos,
                       tracefrom,
                       cg.predictedPlayerState.clientNum,
                       trace.plane.normal,
                       qfalse,-1,10,0,
                       NS_BulletHoleTypeForSurface( trace.surfaceFlags ),
                       cg.predictedPlayerState.weapon );

            // only 9mm bounces off the wall, and not every projectile
            if ( trace.surfaceFlags & SURF_METALSTEPS &&
                    !BG_IsShotgun( cg.predictedPlayerState.weapon ) &&
                    ( caliber == AM_SMG || caliber == AM_LIGHT_PISTOL || caliber == AM_MEDIUM_PISTOL )  && Q_crandom( seed ) < 0.5f )
            {
                VectorCopy( trace.endpos, temp);

                CG_BounceProjectile( tracefrom, temp,  trace.plane.normal, end );
                VectorCopy( temp, tracefrom );
                VectorSubtract( end, temp, forward );
                VectorNormalize(forward);
                // now you can hit yourself with your projectile
                ignoreent = ENTITYNUM_NONE;
                wallhits++;

                // after rebounced... break, this won't go through metal then...
                continue;
            }

            // 9mm only goes through 'soft' materials like dirt/snow/wood etc.
            if ( BG_IsShotgun( cg.predictedPlayerState.weapon ) )
                break;
            if (  ( caliber == AM_SMG || caliber == AM_LIGHT_PISTOL || caliber == AM_MEDIUM_PISTOL ) && !(trace.surfaceFlags & SURF_WOODSTEPS) && !(trace.surfaceFlags & SURF_SOFTSTEPS) && !(trace.surfaceFlags & SURF_GLASS)
                    && !(trace.surfaceFlags & SURF_DIRTSTEPS) && !(trace.surfaceFlags & SURF_SNOWSTEPS) && !(trace.surfaceFlags & SURF_SANDSTEPS) )
                break;

            /*
            ============
            Break through wall ~ get break value
            ============
            */
            bulletThickn = BG_LeadGetBreakValueForSurface( &trace );// ;

            switch (cg.predictedPlayerState.weapon)
            {
            case WP_MACMILLAN:
                bulletThickn *= 1.75; // uber tuff :>
                break;
            case WP_M14:
            case WP_PSG1:
            case WP_SL8SD:
                bulletThickn *= 1.5; // really tuff
                break;
            default:
                break;
            }
            VectorMA (trace.endpos, bulletThickn, forward, temp);

            // stuck in a wall?
            if ( CG_PointContents( temp, -1 ) & CONTENTS_SOLID )
                break;

            // get exit position...
            CG_Trace (&trace2, temp, NULL, NULL, trace.endpos, ignoreent, MASK_SOLID );

            // get the point where the bullet leaves the wall
            VectorCopy (trace2.endpos, tracefrom);

            // we actually found a plane that might be a window
            if(trace2.entityNum < ENTITYNUM_MAX_NORMAL)
            {
                // do a trace back again, but this time ignore the entity.
                CG_Trace (&trace2, temp, NULL, NULL, trace.endpos, trace2.entityNum, MASK_SOLID );

                // get the point where the bullet leaves the wall
                VectorCopy (trace2.endpos, tracefrom);
            }

            // deflect the bullet
            VectorSubtract(end, tracefrom, temp);
            r = VectorLength(temp);
            // by right/up direction (maximum of 5 degrees (5*2*PI)/360)
            u = 0.086 - 0.172 * Q_random(seed);
            VectorMA(end, u, right, end);
            u = 0.086 - 0.172 * Q_random(seed);
            VectorMA(end, u, up, end);


            wallhits ++;

            CG_MissileHitWall( cg.predictedPlayerState.weapon,cg.predictedPlayerState.clientNum,
                               trace2.endpos, trace2.plane.normal, 0, NS_BulletHoleTypeForSurface(trace.surfaceFlags) );

            continue;
        }

        wallhits++;

        if ( cg_entities[trace.entityNum].currentState.eType == ET_PLAYER )
        {
            // (FIX ME?) NO BLOOD FX

            // these weapons don't go through bodies
            if ( caliber == AM_SMG || caliber == AM_LIGHT_PISTOL || caliber == AM_MEDIUM_PISTOL ||
                    BG_IsShotgun( cg.predictedPlayerState.weapon ) )
                break;

            ignoreent = trace.entityNum; // ignore this entity

            // deflect the bullet
            VectorSubtract(end, tracefrom, temp);
            r = VectorLength(temp);
            // by right/up direction (maximum of 5 degrees (5*2*PI)/360)
            u = 0.086 - 0.172 * Q_random(seed);
            VectorMA(end, u, right, end);
            u = 0.086 - 0.172 * Q_random(seed);
            VectorMA(end, u, up, end);

            continue;
        }

        entityNum = trace.entityNum;

        // fixme add player
        if ( cg_entities[trace.entityNum].currentState.eType == ET_MOVER ||
                cg_entities[trace.entityNum].currentState.eType == ET_DOOR )
        {

            entityNum = 0;
        }
        else if ( cg_entities[trace.entityNum].currentState.eType != ET_FUNCEXPLOSIVE )
            break;

        //G_Printf("hitted enitity: %i[%i]\n", entityNum, trace.entityNum );

        //CG_Printf("tracefrom %s\n", vtos( tracefrom ) );

        // create FX
        CG_Bullet( trace.endpos,
                   tracefrom,
                   cg.predictedPlayerState.clientNum,
                   trace.plane.normal,
                   qfalse,-1,10,
                   entityNum,/* trace.entityNum ~ so the created marks will be removed */
                   NS_BulletHoleTypeForSurface( trace.surfaceFlags ),
                   cg.predictedPlayerState.weapon );

        // only 9mm bounces off the wall, and not every projectile
        if ( trace.surfaceFlags & SURF_METALSTEPS &&
                !BG_IsShotgun( cg.predictedPlayerState.weapon ) &&
                ( caliber == AM_SMG || caliber == AM_LIGHT_PISTOL || caliber == AM_MEDIUM_PISTOL )  && Q_crandom( seed ) < 0.5f )
        {
            VectorCopy( trace.endpos, temp);
            CG_BounceProjectile( tracefrom, temp,  trace.plane.normal, end );
            VectorCopy( temp, tracefrom );
            VectorSubtract( end, temp, forward );
            VectorNormalize(forward);
            // now you can hit yourself with your projectile
            ignoreent = trace.entityNum;

            wallhits++;

            // after rebounced... break, this won't go through metal then...
            continue;
        }

        // 9mm / shotgun only goes through 'soft' materials like dirt/snow/wood etc.
        if (  ( caliber == AM_SMG || caliber == AM_LIGHT_PISTOL || caliber == AM_MEDIUM_PISTOL ||
                BG_IsShotgun( cg.predictedPlayerState.weapon )  ) && !(trace.surfaceFlags & SURF_WOODSTEPS) && !(trace.surfaceFlags & SURF_SOFTSTEPS) && !(trace.surfaceFlags & SURF_GLASS)
                && !(trace.surfaceFlags & SURF_DIRTSTEPS) && !(trace.surfaceFlags & SURF_SNOWSTEPS) && !(trace.surfaceFlags & SURF_SANDSTEPS) )
            break;

        /*
        ============
        Break through wall ~ get break value
        ============
        */
        bulletThickn = BG_LeadGetBreakValueForSurface( &trace );// ;

        switch (cg.predictedPlayerState.weapon)
        {
        case WP_MACMILLAN:
            bulletThickn *= 1.75; // uber tuff :>
            break;
        case WP_M14:
        case WP_PSG1:
#ifdef SL8SD
        case WP_SL8SD:
#endif
            bulletThickn *= 1.5; // really tuff
            break;
        default:
            break;
        }

        VectorMA (trace.endpos, bulletThickn, forward, temp);

        // stuck in a wall?
        if ( CG_PointContents( temp, -1 ) & CONTENTS_SOLID && !(trace.surfaceFlags & SURF_GLASS) )
            break;

        // get exit position...
        CG_Trace (&trace2, temp, NULL, NULL, trace.endpos, ignoreent, MASK_SOLID );

        // get the point where the bullet leaves the wall
        VectorCopy (trace2.endpos, tracefrom);

        //	CG_Printf("exit for entity: %i [exit ent: %i]\n", trace.entityNum, trace2.entityNum );
        // we actually are free again, but we hit something different from our original entity?
        // go back ignore the current
        if(trace2.entityNum != trace.entityNum )
        {
            int checks = 5;

            while ( checks > 0 )
            {
                // do a trace back again, but this time ignore the entity.
                CG_Trace (&trace2, temp, NULL, NULL, trace.endpos, trace2.entityNum, MASK_SOLID );

                // get the point where the bullet leaves the wall
                VectorCopy (trace2.endpos, tracefrom);

                if ( trace2.entityNum == entityNum )
                    break;

                //	CG_Printf("new exit for entity: %i\n", trace2.entityNum );

                checks--;

            }
            // something must have been terribly wrong, so now avoid spawning multiple effects
            // and consuming unneccesary cpu power
            if ( checks == 0 && trace2.entityNum != entityNum )
                break;
        }

        CG_MissileHitWall( cg.predictedPlayerState.weapon,
                           cg.predictedPlayerState.clientNum,
                           tracefrom,
                           trace2.plane.normal,
                           entityNum,
                           NS_BulletHoleTypeForSurface(trace2.surfaceFlags)
                         );

        ignoreent = entityNum;

        // deflect the bullet
        VectorSubtract(end, tracefrom, temp);
        r = VectorLength(temp);
        // by right/up direction (maximum of 5 degrees (5*2*PI)/360)
        u = 0.086 - 0.172 * Q_random(seed);
        VectorMA(end, u, right, end);
        u = 0.086 - 0.172 * Q_random(seed);
        VectorMA(end, u, up, end);

    } while ( wallhits < bulletHits );

}

void CG_PredictFireShotgun( void )
{
    int i = 0;
    int pellets;
    int weapon = cg.predictedPlayerState.weapon;

    switch ( weapon ) {
    case WP_SPAS15:
        pellets = SPAS15_PELLETS;
        break;
    case WP_M590:
        pellets = M590_PELLETS;
        break;
    case WP_870:
    default:
        pellets = M870_PELLETS;
        break;
    }

    for ( i=0; i<pellets; i++ )
    {
        CG_PredictFireLead( );
    }
}
/*
================
CG_ShotgunPattern

Perform the same traces the server did to locate the
hit splashes (FIXME: ranom seed isn't synce anymore)
================
*/
static void CG_ShotgunPattern( vec3_t origin, vec3_t origin2, int otherEntNum, int spread, int count, int u2,int r2 ) {
    int			i;
    float		r, u;
    vec3_t		end;
    vec3_t		forward, right, up;

    // derive the right and up vectors from the forward vector, because
    // the client won't have any other information
    VectorNormalize2( origin2, forward );
    PerpendicularVector( right, forward );
    CrossProduct( forward, right, up );

    // generate the "random" spread pattern
    for ( i = 0 ; i < count ; i++ ) {
        if ( u2 && r2 )
        {
            r = crandom() * (spread*r2);
            u = crandom() * (spread/u2);
        }
        else
        {
            r = crandom() * spread;
            u = crandom() * spread;
        }
        VectorMA( origin, 8192, forward, end);
        VectorMA (end, r, right, end);
        VectorMA (end, u, up, end);

        CG_ShotgunPellet( origin, end, forward, otherEntNum );
    }
}

/*
==============
CG_ShotgunFire
==============
*/
void CG_ShotgunFire( entityState_t *es ) {
    vec3_t	v;
    int		contents;

    VectorSubtract( es->origin2, es->pos.trBase, v );
    VectorNormalize( v );
    VectorScale( v, 32, v );
    VectorAdd( es->pos.trBase, v, v );
    if ( cgs.glconfig.hardwareType != GLHW_RAGEPRO ) {
        // ragepro can't alpha fade, so don't even bother with smoke
        vec3_t			up;

        contents = trap_CM_PointContents( es->pos.trBase, 0 );
        if ( !( contents & CONTENTS_WATER ) ) {
            VectorSet( up, 0, 0, 8 );

            CG_SmokePuff( v, up, 32, 1, 1, 1, 0.5, cg_gunSmokeTime.integer, cg.time,0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader );
        }
    }
    if ( es->frame && es->otherEntityNum2 )
        CG_ShotgunPattern( es->pos.trBase, es->origin2, es->otherEntityNum, es->eventParm, es->generic1, es->frame, es->otherEntityNum2  );
    else
        CG_ShotgunPattern( es->pos.trBase, es->origin2, es->otherEntityNum, es->eventParm, es->generic1, 0, 0 );
}

/*
============================================================================

BULLETS

============================================================================
*/


/*
===============
CG_Tracer
===============
*/
void CG_Tracer( vec3_t source, vec3_t dest, float width, qhandle_t shader, vec4_t rgba ) {
    vec3_t		forward, right;
    polyVert_t	verts[4];
    vec3_t		line;
    float		len, begin, end;
    vec3_t		start, finish;
    //	vec3_t		midpoint;

    // tracer
    VectorSubtract( dest, source, forward );
    len = VectorNormalize( forward );

    // start at least a little ways from the muzzle
    // begin = width * -0.5;
    begin = width * -0.5; // put 1 meter in front of the attacker
    end = len + width * 0.5;

    VectorMA( source, begin, forward, start );
    VectorMA( source, end, forward, finish );
    line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
    line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

    VectorScale( cg.refdef.viewaxis[1], line[1], right );
    VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
    VectorNormalize( right );

    VectorMA( finish, width, right, verts[0].xyz );
    verts[0].st[0] = 0;
    verts[0].st[1] = 1;
    verts[0].modulate[0] = 255 * rgba[0];
    verts[0].modulate[1] = 255 * rgba[1];
    verts[0].modulate[2] = 255 * rgba[2];
    verts[0].modulate[3] = 255 * rgba[3];

    VectorMA( finish, -width, right, verts[1].xyz );
    verts[1].st[0] = 1;
    verts[1].st[1] = 1;
    verts[1].modulate[0] = 255 * rgba[0];
    verts[1].modulate[1] = 255 * rgba[1];
    verts[1].modulate[2] = 255 * rgba[2];
    verts[1].modulate[3] = 255 * rgba[3];

    VectorMA( start, -width, right, verts[2].xyz );
    verts[2].st[0] = 1;
    verts[2].st[1] = 0;
    verts[2].modulate[0] = 255 * rgba[0];
    verts[2].modulate[1] = 255 * rgba[1];
    verts[2].modulate[2] = 255 * rgba[2];
    verts[2].modulate[3] = 255 * rgba[3];

    VectorMA( start, width, right, verts[3].xyz );
    verts[3].st[0] = 0;
    verts[3].st[1] = 0;
    verts[3].modulate[0] = 255 * rgba[0];
    verts[3].modulate[1] = 255 * rgba[1];
    verts[3].modulate[2] = 255 * rgba[2];
    verts[3].modulate[3] = 255 * rgba[3];

    trap_R_AddPolyToScene( shader, 4, verts );

}

/*
======================
CG_Bullet

Renders bullet effects.
======================
*/
void CG_Bullet( vec3_t end, vec3_t start, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum, int damage, int soundtype, int bholetype, int weapon ) {
    trace_t trace;
    int sourceContentType, destContentType;
    centity_t	*cent;
    vec3_t up;
    vec4_t rgba;

    rgba[0] = rgba[1] = rgba[2] = rgba[3] = 1;

    up[0] = up[1] = 0;
    up[2] = 2;

    cent = &cg_entities[sourceEntityNum];

    if ( random() < cg_tracerChance.value &&
            ( weapon == WP_M4 || weapon == WP_AK47 || weapon == WP_M14 || weapon == WP_M249 ) )
    {
        vec3_t right,up,forward;


        // tracer
        if ( sourceEntityNum == cg.clientNum && firstshot )
        {

            AngleVectors( cg.refdefViewAngles, forward,right,up );

            VectorMA( start, 30, forward, start );
            VectorMA( start, 2.0f, right, start );
            VectorMA( start, -3.0f, up, start );

            /*			if ( cg_smallGuns.integer || cg_drawGun.integer == 3 || cg_drawGun.integer == 4 ) {
            VectorMA( start, -2.5f, up, start );
            VectorMA( start, 2.6, forward, start );
            VectorMA( start, 0.2, right, start );
            }*/

            firstshot = qfalse;
        } else {

            // BLUTENGEL:
            // put tracer lil bit to front
            VectorSubtract(end, start, forward);
            VectorNormalize(forward);
            VectorMA(start, 28.0, forward, start);
            start[2] -= 3.0;

        }

        CG_SpawnTracer( start, end );
    }

    // if the shooter is currently valid, calc a source point and possibly
    // do trail effects
    if ( /*start[0] != 0 && start[1] != 0 && start[2] != 0*/ 1 )
    {
        if ( sourceEntityNum >= 0 && cg_tracerChance.value > 0 ) {
            //	if ( CG_CalcMuzzlePoint( sourceEntityNum, start ) ) {
            sourceContentType = trap_CM_PointContents( start, 0 );
            destContentType = trap_CM_PointContents( end, 0 );

            // do a complete bubble trail if necessary
            if ( ( sourceContentType == destContentType ) && ( sourceContentType & CONTENTS_WATER ) ) {
                CG_BubbleTrail( start, end, 8 );
            }
            // bubble trail from water into air
            else if ( ( sourceContentType & CONTENTS_WATER ) ) {
                trap_CM_BoxTrace( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
                CG_BubbleTrail( start, trace.endpos, 8 );
            }
            // bubble trail from air into water
            else if ( ( destContentType & CONTENTS_WATER ) ) {
                trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
                CG_BubbleTrail( end, trace.endpos, 8 );
            }
        }
    }

    //	if (  trace.surfaceFlags & SURF_NOIMPACT )
    //		return;

    // impact splash and mark
    if ( flesh ) {
        int r = (int)(random()*3);

        CG_PlayerBleed( cent->currentState.weapon, cent->currentState.clientNum, damage, end, normal );

        if ((cg.DeafTime < cg.time)) trap_S_StartSound (end, fleshEntityNum, CHAN_AUTO, cgs.media.bulletHitFlesh[r] );

    } else {
        CG_MissileHitWall( weapon, 0,end,  normal, soundtype, bholetype );
    }

}


animation_t	weaponAnimations[WP_NUM_WEAPONS*2][MAX_WEAPON_ANIMATIONS];
//weaponframestuff_t	weaponStuff[MAX_WEAPONS];

//
// rather important :D
//

int CG_StringToAnimation( char *token )
{
    if ( !Q_stricmp( token, "WANIM_IDLE" ) )
        return WANIM_IDLE;
    else if ( !Q_stricmp( token, "WANIM_ATTACK" ) )
        return WANIM_ATTACK;
    else if ( !Q_stricmp( token, "WANIM_PUTAWAY" ) )
        return WANIM_PUTAWAY;
    else if ( !Q_stricmp( token, "WANIM_PUTUP" ) )
        return WANIM_PUTUP;
    else if ( !Q_stricmp( token, "WANIM_RELOAD" ) )
        return WANIM_RELOAD;
    else if ( !Q_stricmp( token, "WANIM_LASTRND" ) )
        return WANIM_LASTRND;
    else if ( !Q_stricmp( token, "WANIM_ATTACK2" ) )
        return WANIM_ATTACK2;
    else if ( !Q_stricmp( token, "WANIM_ATTACK3" ) )
        return WANIM_ATTACK3;
    else if ( !Q_stricmp( token, "WANIM_SPIN1" ) )
        return WANIM_SPIN1;
    else if ( !Q_stricmp( token, "WANIM_SPIN2" ) )
        return WANIM_SPIN2;
    else if ( !Q_stricmp( token, "WANIM_THROW" ) )
        return WANIM_THROW;
    else if ( !Q_stricmp( token, "WANIM_IDLE_EMPTY" ) )
        return WANIM_IDLE_EMPTY;
    else if ( !Q_stricmp( token, "WANIM_RELOAD_EMPTY" ) )
        return WANIM_RELOAD_EMPTY;
    else if ( !Q_stricmp( token, "WANIM_RELOAD_CYCLE" ) )
        return WANIM_RELOAD_CYCLE;
    else if ( !Q_stricmp( token, "WANIM_MELEE" ) )
        return WANIM_MELEE;
    else if ( !Q_stricmp( token, "WANIM_RELOAD_STOP" ) )
        return WANIM_RELOAD_STOP;
    else if ( !Q_stricmp( token, "WANIM_ATTACKMODE21" ) )
        return WANIM_ATTACKMODE21;
    else if ( !Q_stricmp( token, "WANIM_ATTACKMODE22" ) )
        return WANIM_ATTACKMODE22;
    else if ( !Q_stricmp( token, "WANIM_ATTACKMODE23" ) )
        return WANIM_ATTACKMODE23;

    else if ( !Q_stricmp( token, "WANIM_IRONSIGHT_UP" ) )
        return WANIM_IRONSIGHT_UP;
    else if ( !Q_stricmp( token, "WANIM_IRONSIGHT_DN" ) )
        return WANIM_IRONSIGHT_DN;
    else if ( !Q_stricmp( token, "WANIM_IRONSIGHTIDLE" ) )
        return WANIM_IRONSIGHTIDLE;

    else if ( !Q_stricmp( token, "WANIM_IRONSIGHT_ATK1" ) )
        return WANIM_IRONSIGHT_ATK1;
    else if ( !Q_stricmp( token, "WANIM_IRONSIGHT_ATK2" ) )
        return WANIM_IRONSIGHT_ATK2;
    else if ( !Q_stricmp( token, "WANIM_IRONSIGHT_ATK3" ) )
        return WANIM_IRONSIGHT_ATK3;

    return WANIM_UNKNOWN;
}

vec3_t	weaponOffsets[WP_NUM_WEAPONS*2];

/*
======================
CG_ParseWeaponAnimationFile

Read a configuration file containing animation coutns and rates
models/players/visor/animation.cfg, etc
======================
*/
qboolean	CG_ParseWeaponAnimationFile( const char *filename, int weapon_num ) {
    char		*text_p;
    int			len;
    //	int			lines = 0;
    char		*token;
    float		fps;
    int			skip;
    char		text[20000];
    fileHandle_t	f;
    int animation = WANIM_UNKNOWN;

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

    // clear diz
    VectorClear( weaponOffsets[weapon_num] );


    // parse animation information
    while ( 1 ) {

        // first thing we do, read which animation it's for
        token = COM_Parse( &text_p );

        // no token? stop parsing...
        if ( !token ) {
            break;
        }

        if ( !Q_stricmp( token, "W_OFFSET" ) && weaponOffsets[weapon_num][0] == 0 && weaponOffsets[weapon_num][1] == 0 && weaponOffsets[weapon_num][2] == 0 ) {
            vec3_t offset;

            VectorClear(offset);

            token = COM_Parse( &text_p );

            if ( !token ) {
                VectorClear(offset);
                break;
            }

            offset[0] = atof(token);

            token = COM_Parse( &text_p );

            if ( !token ) {
                VectorClear(offset);
                break;
            }
            offset[1] = atof(token);

            token = COM_Parse( &text_p );

            if ( !token ) {
                VectorClear(offset);
                break;
            }

            offset[2] = atof(token);

            VectorCopy( offset, weaponOffsets[weapon_num] );

            token = COM_Parse( &text_p );

            //			Com_Printf("parsed weaponoffset %s for weapon %i\n", vtos(offset),weapon_num );
        }

        // if found the end string
        if (token[0] == '$' && token[1] == 'E' )
            break; // stop parsing

        // good token? then set it as current anim.
        animation = CG_StringToAnimation(token);

        // check if it's valid
        if (animation == WANIM_UNKNOWN)
        {
            Com_Printf( "unknown animation '%s' in %s\n", token, filename );
            break;
        }

        // then loadup everything into the current animation
        token = COM_Parse( &text_p );

        if ( !token ) {
            break;
        }
        weaponAnimations[weapon_num][animation].firstFrame = atoi( token );


        token = COM_Parse( &text_p );
        if ( !token ) {
            break;
        }
        weaponAnimations[weapon_num][animation].numFrames = atoi( token );

        token = COM_Parse( &text_p );
        if ( !token ) {
            break;
        }
        weaponAnimations[weapon_num][animation].loopFrames = atoi( token );

        token = COM_Parse( &text_p );
        if ( !token ) {
            break;
        }
        fps = atof( token );

        // default frames per second to zero
        if ( fps == 0 ) {
            fps = 1;
        }
        weaponAnimations[weapon_num][animation].frameLerp = 1000 / fps;
        weaponAnimations[weapon_num][animation].initialLerp = 1000 / fps;
    }

    //	weaponOffsets[weapon_num][0] -= 5;
    return qtrue;
}

/*
===============
CG_SetWeaponLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
void CG_SetWeaponLerpFrameAnimation( lerpFrame_t *lf, int weapon_num, int newAnimation ) {
    animation_t	*anim;

    lf->animationNumber = newAnimation;
    newAnimation &= ~ANIM_TOGGLEBIT;

    if ( newAnimation < 0 || newAnimation >= MAX_WEAPON_ANIMATIONS ) {
        CG_Error( "Bad animation number: %i", newAnimation );
    }

    anim = &weaponAnimations[weapon_num][ newAnimation ];

    lf->animation = anim;
    lf->animationTime = lf->frameTime + anim->initialLerp;

    if ( cg_debugAnim.integer == -1 ) {
        CG_Printf( "Setted WeaponAnim: %i\n", newAnimation );
    }
}

// old
#if 0
/*
===============
CG_CheckPause

Checks for a Pause , if correct frame, it makes a random break
===============
*/
static qboolean CG_CheckPause ( int cur_frame, int weapon_num )
{
    int i = 0;

    for ( i = 0 ; i < weaponStuff[weapon_num].num_pauseframes ; i++ )
    {
        if (cur_frame == weaponStuff[weapon_num].pauseframes[i] )
        {
            if (random() < 0.3) {
#if 0
                CG_Printf("Breaking...\n");
#endif
                return qtrue;
            }
        }
    }
    return qfalse;

}
#endif


/*
===============
CG_GetSpeedScaleForWeaponSwitch

returns the speedscale for the weaponswitching based on
the techincal level of the player
===============
*/
float	CG_GetSpeedScaleForWeaponSwitch( playerState_t *ps, centity_t *cent ) {
    int weapAnim = cent->pe.weapAnim;
    int	techLevel = ps->persistant[PERS_TECHNICAL];
    float	speedMod = 1.0f;

    if ( weapAnim != WANIM_PUTUP && weapAnim != WANIM_PUTAWAY )
        return 1.0f;


    speedMod = 1.0f + ( 1.0f - BG_GetSpeedMod( techLevel ) ) * 2;

    //	CG_Printf("speed mod: %3f tech: %i\n", speedMod, techLevel );

    return speedMod;
}

/*
===============
CG_RunWeaponLerpFrame

Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
void CG_RunWeaponLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int weapon_num, int newAnimation, float speedScale, playerState_t *ps ,centity_t	*cent) {
    int			f;
    qboolean	loopanim = qfalse;
    animation_t	*anim;

    // debugging tool to get no animations
    if ( cg_animSpeed.integer == 0 ) {
        lf->oldFrame = lf->frame = lf->backlerp = 0;
        return;
    }

    if ( cg_animSpeed.value != 1.0f ) {
        speedScale = cg_animSpeed.value;
    }
    /*	if ( CG_CheckPause(lf->frame, weapon_num) ) {
    loopanim = qtrue;
    return;
    }
    */
    // see if the animation sequence is switching
    if ( newAnimation != lf->animationNumber || !lf->animation ) {
#if 0
        CG_Printf("Animation Sequence Switching!\n");
#endif
        CG_SetWeaponLerpFrameAnimation( lf, weapon_num, newAnimation );
    }

    // if we have passed the current frame, move it to
    // oldFrame and calculate a new frame
    if ( cg.time >= lf->frameTime ) {

        if (loopanim)
            lf->frame--;

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
        if ( f >= anim->numFrames ) {
            f -= anim->numFrames;
            if ( anim->loopFrames ) {
                f %= anim->loopFrames;
                f += anim->numFrames - anim->loopFrames;
            } else {
                f = anim->numFrames - 1;
                // the animation is stuck at the end, so it
                // can immediately transition to another sequence
                lf->frameTime = cg.time;

            }
        }

        lf->frame = anim->firstFrame + f;

        if ( cg.time > lf->frameTime ) {
            lf->frameTime = cg.time;
            if ( cg_debugAnim.integer == -1) {
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
CG_WeaponAnimation
===============
*/
/*
=================
AxistoAngles
TA: takes an axis (forward + right + up)
and returns angles -- including a roll
=================
*/
void AxistoAngles( vec3_t axis[3], vec3_t angles )
{
    float length1;
    float yaw, pitch, roll;

    roll = pitch = yaw = 0;

    if ( axis[0][1] == 0 && axis[0][0] == 0 )
    {

        if ( axis[0][2] > 0 ) {
            pitch = 90;
        }
        else {
            pitch = 270;
        }
    }
    else {
        if ( axis[0][0] ) {
            yaw = ( atan2 ( axis[0][1], axis[0][0] ) * 180 / M_PI );
        }
        else if ( axis[0][1] > 0 ) {
            yaw = 90;
        }
        else {
            yaw = 270;
        }
        if ( yaw < 0 ) {
            yaw += 360;
        }

        length1 = sqrt ( axis[0][0]*axis[0][0] + axis[0][1]*axis[0][1] );
        pitch = ( atan2(axis[0][2], length1) * 180 / M_PI );

        if ( pitch < 0 ) {
            pitch += 360;
        }

        roll = ( atan2( axis[1][2], axis[2][2] ) * 180 / M_PI );
        if ( roll < 0 )
        {
            roll += 360;
        }
    }

    angles[PITCH] = -pitch;
    angles[YAW] = yaw;
    angles[ROLL] = roll;
}

/*
================
CG_SetupRender

sets up explicit weapon rendering
================
*/
qboolean CG_SetupRender( void ) {

#ifdef SAME_WEAPONPIPE
    return qfalse;
#endif

    // copy values from cg.refdef to cg.weaponrefdef
    memset( &cg.weaponrefdef, 0, sizeof( cg.weaponrefdef ) );

    // setup refdef
    cg.weaponrefdef.fov_x = cg_correctgunFov.value;
    cg.weaponrefdef.fov_y = cg_correctgunFov.value;
    cg.weaponrefdef.height = cg.refdef.height;
    cg.weaponrefdef.width = cg.refdef.width;
    cg.weaponrefdef.x = 0;
    cg.weaponrefdef.y = 0;

    cg.weaponrefdef.rdflags = RDF_NOWORLDMODEL;;


    //	cg.weaponrefdef.text
    VectorCopy( cg.refdef.vieworg , cg.weaponrefdef.vieworg );
    AxisCopy( cg.refdef.viewaxis, cg.weaponrefdef.viewaxis );

    cg.weaponrefdef.time = cg.time;

    trap_R_ClearScene();
    return qtrue;
}
void CG_RenderWeapon( void )
{
    trap_R_RenderScene( &cg.weaponrefdef );
}

void CG_WeaponAnimation( playerState_t *ps ) {
    refEntity_t	hand;
    centity_t	*cent;
    clientInfo_t	*ci;
    float		fovOffset;
    vec3_t		angles;
    weaponInfo_t	*weapon;
    refEntity_t	flash, silencer, scope, lasersight,bayonet;
    int	weaponNum;
    centity_t	*nonPredictedCent;
    vec3_t	origin;
    vec3_t		weaponMod;

    //
    if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_NOCLIP || ps->pm_type == PM_SPECTATOR )
        return;

    // no gun if bandaging
    if ( ps->weaponstate == WEAPON_BANDAGING)
        return;

    // no gun if in third person view
    if ( cg.renderingThirdPerson )
        return;

    // don't draw if testing a gun model
    if ( cg.testGun )
        return;

    if ( !cg_drawGun.integer )
        return;

    if (  cg_viewsize.integer < 100 )
        return;

    // no weapon if we're at a camera.
    if ( cg.cameraActive )
        return;

    // drop gun lower at higher fov
    if ( cg_fov.integer > 90 ) {
        fovOffset = -0.2 * ( cg_fov.integer - 90 );
    } else {
        fovOffset = 0;
    }

    cent = &cg.predictedPlayerEntity;	// &cg_entities[cg.snap->ps.clientNum];

	weapon = &cg_weapons[ ps->weapon ];

    memset (&hand, 0, sizeof(hand));

    // set up gun position
    CG_CalculateWeaponPosition( hand.origin, angles );

    if ( cg.ladderWeaponTime == 23 )
        return;

    VectorClear( weaponMod );
 
    AnglesToAxis( angles, hand.axis );

    // map torso animations to weapon animations
    // get clientinfo for animation map
    ci = &cgs.clientinfo[ cent->currentState.clientNum ];

    if ( ps->weaponstate == WEAPON_READY && ( BG_IsPistol(ps->weapon) ) && ps->stats[STAT_ROUNDS] <= 0)
        cent->pe.weapAnim = WANIM_IDLE_EMPTY;
    else if (ps->weaponstate == WEAPON_READY)
        cent->pe.weapAnim = WANIM_IDLE;
    else if (ps->weaponstate == WEAPON_RAISING)
        cent->pe.weapAnim = WANIM_PUTUP;
    else if (ps->weaponstate == WEAPON_DROPPING || ps->weaponstate == WEAPON_BANDAGING_START)
        cent->pe.weapAnim = WANIM_PUTAWAY;
    else if (ps->weaponstate == WEAPON_FIRING)
        cent->pe.weapAnim = WANIM_ATTACK;
    else if (ps->weaponstate == WEAPON_FIRING2)
        cent->pe.weapAnim = WANIM_ATTACK2;
    else if (ps->weaponstate == WEAPON_FIRING3)
        cent->pe.weapAnim = WANIM_ATTACK3;
    else if (ps->weaponstate == WEAPON_FIREEMPTY)
        cent->pe.weapAnim = WANIM_LASTRND;
    else if (ps->weaponstate == WEAPON_LASTRND)
        cent->pe.weapAnim = WANIM_LASTRND;
    else if (ps->weaponstate == WEAPON_RELOADING_EMPTY)
        cent->pe.weapAnim = WANIM_RELOAD_EMPTY;
    else if (ps->weaponstate == WEAPON_RELOADING)
        cent->pe.weapAnim = WANIM_RELOAD;
    else if (ps->weaponstate == WEAPON_LASTRND)
        cent->pe.weapAnim = WANIM_LASTRND;
    else if (ps->weaponstate == WEAPON_HOLSTERING)
        cent->pe.weapAnim = WANIM_PUTAWAY;
    else if (ps->weaponstate == WEAPON_RELOADING_STOP)
        cent->pe.weapAnim = WANIM_RELOAD_STOP;
    else if (ps->weaponstate == WEAPON_RELOADING_CYCLE)
        cent->pe.weapAnim = WANIM_RELOAD_CYCLE;
    else if (ps->weaponstate == WEAPON_MELEE )
        cent->pe.weapAnim = WANIM_MELEE;
    else
        cent->pe.weapAnim = WANIM_IDLE;



    // c4 special handling
    if ( ps->weapon == WP_C4 && ps->pm_flags & PMF_BOMBCASE )
    {
        switch (ps->weaponstate) {
        case WEAPON_LASTRND:
            cent->pe.weapAnim = WANIM_IDLE_EMPTY;
            break;
        case WEAPON_BANDAGING_START:
            cent->pe.weapAnim = WANIM_SPIN1;
            break;
        case WEAPON_BANDAGING_END:
            cent->pe.weapAnim = WANIM_SPIN2;
            break;
        case WEAPON_BANDAGING:
            cent->pe.weapAnim = WANIM_ATTACKMODE23;
            break;
        case WEAPON_MELEE:
            cent->pe.weapAnim = WANIM_MELEE;
            break;
        case WEAPON_THROW:
            cent->pe.weapAnim = WANIM_THROW;
            break;
        case WEAPON_FIRING21:
            cent->pe.weapAnim = WANIM_ATTACKMODE21;
            break;
        case WEAPON_FIRING22:
            cent->pe.weapAnim = WANIM_ATTACKMODE22;
            break;
        }

    }

    // pdw special handling
    if ( ps->weapon == WP_PDW )
    {
        if (ps->weaponstate == WEAPON_RELOADING_CYCLE && ps->weapon == WP_PDW )
            cent->pe.weapAnim = WANIM_THROW;
        else if (ps->weaponstate == WEAPON_RELOADING_STOP && ps->weapon == WP_PDW )
            cent->pe.weapAnim = WANIM_MELEE;
        else if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ) )
        {
            if ( ps->weaponstate == WEAPON_FIRING )
                cent->pe.weapAnim = WANIM_ATTACKMODE21;
            else if ( ps->weaponstate == WEAPON_FIRING2 )
                cent->pe.weapAnim = WANIM_ATTACKMODE22;
            else if ( ps->weaponstate == WEAPON_FIRING3 )
                cent->pe.weapAnim = WANIM_ATTACKMODE23;
            else if ( ps->weaponstate == WEAPON_READY )
                cent->pe.weapAnim = WANIM_IDLE_EMPTY;
        }
    }

    // grenadelauncher requires special handling
    if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) )
    {
        // gl attached.
        if (ps->weaponstate == WEAPON_RELOADING_STOP )
            cent->pe.weapAnim = WANIM_SPIN2; // back to normal mode
        else if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ) )
        {
            if ( ps->weaponstate == WEAPON_READY )
                cent->pe.weapAnim = WANIM_IDLE_EMPTY; // idle gl
            else if (ps->weaponstate == WEAPON_RELOADING_CYCLE )
                cent->pe.weapAnim = WANIM_THROW;	// to gl
            else if (ps->weaponstate == WEAPON_RELOADING )
                cent->pe.weapAnim = WANIM_SPIN1; // reload gl
            else if (ps->weaponstate == WEAPON_FIRING )
                cent->pe.weapAnim = WANIM_ATTACKMODE23; // fire gl
        }
    }

    //
    // dead... no render for weaponmodel - this is also a cheap fix for corrupt
    // weapon frames in the idleframes.
    //
    if ( ps->stats[STAT_HEALTH] <= 0 ) {
        cent->pe.weapAnim = WANIM_PUTAWAY;
    }

    if ( cg.ns_ironsightState == IS_PUTUP || cg.ns_ironsightState == IS_PUTAWAY )
    {
        // time to switch to next state
        if ( cg.ns_ironsightTimer < cg.time )
        {
            switch ( cg.ns_ironsightState ) {
            case IS_PUTUP:
                cg.ns_ironsightState = IS_IDLE;
                break;
            case IS_PUTAWAY:
                cg.ns_ironsightState = IS_NONE;
                break;
            default:
                break;
            }
        }
    }
    else if ( cg.ns_ironsightState == IS_IDLE )
    {
        if ( ps->weaponstate == WEAPON_RELOADING || ps->weaponstate == WEAPON_RELOADING_EMPTY )
        {

        }
        // somebody is trying to deactivate me.
        if ( cg.ns_ironsightDeactivate )
        {
            cg.ns_ironsightState = IS_PUTAWAY;

            if ( BG_IsRifle( ps->weapon ) )
                cg.ns_ironsightTimer = cg.time + IS_RIFLE_TIME;
            else
                cg.ns_ironsightTimer = cg.time + IS_TIME;
        }
    }

    if ( cg.ns_ironsightState == IS_PUTUP || cg.ns_ironsightState == IS_PUTAWAY || cg.ns_ironsightState == IS_IDLE )
    {
        if ( cg.ns_ironsightState == IS_PUTUP )
            cent->pe.weapAnim = WANIM_IRONSIGHT_UP;
        else if ( cg.ns_ironsightState == IS_PUTAWAY )
            cent->pe.weapAnim = WANIM_IRONSIGHT_DN;
        else if (ps->weaponstate == WEAPON_FIRING)
            cent->pe.weapAnim = WANIM_IRONSIGHT_ATK1;
        else if (ps->weaponstate == WEAPON_FIRING2)
            cent->pe.weapAnim = WANIM_IRONSIGHT_ATK2;
        else if (ps->weaponstate == WEAPON_FIRING3)
            cent->pe.weapAnim = WANIM_IRONSIGHT_ATK3;

        else
            cent->pe.weapAnim = WANIM_IRONSIGHTIDLE;
    }


    if ( ps->weapon == WP_C4 && ps->pm_flags & PMF_BOMBCASE )
    {

        CG_RunWeaponLerpFrame ( ci, &cent->pe.hand_weapon, 28, cent->pe.weapAnim, 1.0f, ps, cent);
        hand.hModel = cgs.media.bombCaseModel;
    }
    else if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) )
    {
        CG_RunWeaponLerpFrame ( ci, &cent->pe.hand_weapon, ps->weapon, cent->pe.weapAnim, CG_GetSpeedScaleForWeaponSwitch(ps, cent), ps, cent);
        hand.hModel = weapon->glModel;
    }
    else
    {

        CG_RunWeaponLerpFrame ( ci, &cent->pe.hand_weapon, ps->weapon, cent->pe.weapAnim, CG_GetSpeedScaleForWeaponSwitch(ps, cent), ps, cent);
        hand.hModel = weapon->handsModel;		/////////////
    }

    hand.frame = cent->pe.hand_weapon.frame;
    hand.oldframe = cent->pe.hand_weapon.oldFrame;
    hand.backlerp = cent->pe.hand_weapon.backlerp;
    hand.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON | RF_LIGHTING_ORIGIN | RF_NOSHADOW;
    VectorCopy( cg.predictedPlayerState.origin, hand.lightingOrigin );
    VectorCopy( hand.origin, hand.oldorigin );
    weaponNum = cent->currentState.weapon;

    //
    // apply tango skin
    //
    if ( !cg_disableTangoHandSkin.integer )
    {
        if ( ps->persistant[PERS_TEAM] == TEAM_BLUE )
            hand.customSkin = weapon->t_viewweaponSkin;
        else if ( weaponNum == WP_M249 )
            hand.customSkin = trap_R_RegisterSkin("models/weapons/m249/m249.skin");

        if ( ps->persistant[PERS_TEAM] == TEAM_BLUE && hand.hModel == cgs.media.bombCaseModel )
            hand.customSkin = cgs.media.bombCaseTangoSkin;

        // we're caching this here because it's only one weapon that uses this
        // it's not worth adding a _v.skin for every weapon
        if ( ps->eFlags & EF_VIP && BG_IsPistol( weaponNum ) )
        {
            char			path[MAX_QPATH];
            gitem_t			*item;

            item = BG_FindItemForWeapon( weaponNum );

            if ( item )
            {
                strcpy( path, item->world_model[0] );
                COM_StripExtension( path, path );
                strcat( path, "_v.skin" );

                hand.customSkin =  trap_R_RegisterSkin( path );
            }
        }
    }

    //
    // if we're scoping forget calculating handmodel
    //
    if ( (BG_IsZooming( ps->stats[STAT_WEAPONMODE] ) ) &&
            ( ps->weaponstate == WEAPON_READY ||
              ps->weaponstate == WEAPON_FIRING ||
              ps->weaponstate == WEAPON_FIRING2 ||
              ps->weaponstate == WEAPON_FIRING3 ||
              ps->weaponstate == WEAPON_LASTRND ) )
        return;


    if (!hand.hModel) {
        return;
    }

    CG_SetupRender();

    if ( ps->weapon == WP_C4 && ps->pm_flags & PMF_BOMBCASE )
    {
        refEntity_t	model;
        int i;
        int			mins, seconds, tens;
        int			msec;

        msec = ( cgs.levelBombStartTime + 1000 ) - cg.time;

        seconds = msec / 1000;
        mins = seconds / 60;
        seconds -= mins * 60;
        tens = seconds / 10;
        seconds -= tens * 10;

        if( mins <= 0 )
            mins = 0;
        if ( seconds <= 0 )
            seconds = 0;
        if ( tens <= 0 )
            tens = 0;

        memset( &model, 0, sizeof( model ) );
        //VectorCopy( hand.lightingOrigin, model.lightingOrigin );
        //model.shadowPlane = hand.shadowPlane;

        // i gave it a minlight so the wires are always visible
        model.renderfx = RF_MINLIGHT | RF_DEPTHHACK | RF_FIRST_PERSON | RF_NOSHADOW;// | RF_MINLIGHT;

        //	model.renderfx = hand.renderfx;

        CG_PositionEntityOnTag( &model, &hand, hand.hModel,"tag_weapon");

        VectorCopy( model.origin, model.oldorigin );

        for ( i = 0; i < 8; i++ )
        {
            model.hModel = cgs.media.bombCaseWireModels[i];

            if ( cg.bombcaseWires[i] == -1 )
                model.customShader = cgs.media.bombCaseWireShaders[1];
            else if ( cg.bombcaseWires[i] == 1 )
                model.customShader = cgs.media.bombCaseWireShaders[2];
            else
                model.customShader = cgs.media.bombCaseWireShaders[0];

            trap_R_AddRefEntityToScene( &model );
        }

        model.hModel = cgs.media.bombCaseDigitModels[0];
        model.customShader = cgs.media.digitalNumberShaders[mins];
        trap_R_AddRefEntityToScene( &model );

        model.hModel = cgs.media.bombCaseDigitModels[1];
        model.customShader = cgs.media.digitalNumberShaders[tens];
        trap_R_AddRefEntityToScene( &model );

        model.hModel = cgs.media.bombCaseDigitModels[2];
        model.customShader = cgs.media.digitalNumberShaders[seconds];
        trap_R_AddRefEntityToScene( &model );

        trap_R_AddRefEntityToScene( &hand );
        return;
    }

    if ( cg_drawGun.integer != 2 && cg_drawGun.integer != 4 )
        CG_AddRefEntityWithPowerups( &hand, &cent->currentState, ci->team );

    CG_AddWeaponParts( hand, weapon );

    //
    // add the silencer
    //
    if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_SILENCER ) || ps->stats[STAT_WEAPONMODE] & ( 1 << WM_DUCKBILL ) )
    {
        memset( &silencer, 0, sizeof( silencer ) );
        VectorCopy( hand.lightingOrigin, silencer.lightingOrigin );
        silencer.shadowPlane = hand.shadowPlane;
        silencer.renderfx = hand.renderfx;

        silencer.hModel = weapon->silencerModel;

        if ( hand.customSkin )
            silencer.customSkin = hand.customSkin;

        CG_PositionEntityOnTag( &silencer, &hand, hand.hModel,"tag_flash");

        if ( silencer.hModel )
            trap_R_AddRefEntityToScene( &silencer );

    }

    //
    // add the bayonet
    //
    if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_FLASHLIGHT ) )
    {
        memset( &bayonet, 0, sizeof( bayonet ) );
        VectorCopy( hand.lightingOrigin, bayonet.lightingOrigin );
        bayonet.shadowPlane = hand.shadowPlane;
        bayonet.renderfx = hand.renderfx;

        bayonet.hModel = weapon->glModel;

        if ( hand.customSkin )
            bayonet.customSkin = hand.customSkin;

        CG_PositionEntityOnTag( &bayonet, &hand, hand.hModel,"tag_weapon");

        if ( bayonet.hModel )
            trap_R_AddRefEntityToScene( &bayonet );
    }

    //
    // add the bayonet
    //
    if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_BAYONET ) )
    {
        memset( &bayonet, 0, sizeof( bayonet ) );
        VectorCopy( hand.lightingOrigin, bayonet.lightingOrigin );
        bayonet.shadowPlane = hand.shadowPlane;
        bayonet.renderfx = hand.renderfx;

        bayonet.hModel = weapon->bayonetModel;

        if ( hand.customSkin )
            bayonet.customSkin = hand.customSkin;

        CG_PositionEntityOnTag( &bayonet, &hand, hand.hModel,"tag_weapon");

        if ( bayonet.hModel )
            trap_R_AddRefEntityToScene( &bayonet );
    }

    //
    // Add Scope
    //
    if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_SCOPE ) )
    {
        memset( &scope, 0, sizeof( scope ) );
        VectorCopy( hand.lightingOrigin, scope.lightingOrigin );
        scope.shadowPlane = hand.shadowPlane;
        scope.renderfx = hand.renderfx;

        scope.hModel = weapon->scopeModel;

        if ( hand.customSkin )
            scope.customSkin = hand.customSkin;

        CG_PositionEntityOnTag( &scope, &hand, hand.hModel,"tag_weapon");

        if ( scope.hModel )
            trap_R_AddRefEntityToScene( &scope );
    }

    //
    // add the lasersight
    //
    if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_LASER ) )
    {
        memset( &lasersight, 0, sizeof( lasersight ) );
        VectorCopy( hand.lightingOrigin, lasersight.lightingOrigin );
        lasersight.shadowPlane = hand.shadowPlane;
        lasersight.renderfx = hand.renderfx;

        lasersight.hModel = weapon->lasersightModel;

        if ( hand.customSkin )
            lasersight.customSkin = hand.customSkin;

        CG_PositionEntityOnTag( &lasersight, &hand, hand.hModel,"tag_weapon");

        if ( lasersight.hModel )
            trap_R_AddRefEntityToScene( &lasersight );
    }

    // make sure we aren't looking at cg.predictedPlayerEntity for LG
    nonPredictedCent = &cg_entities[cent->currentState.clientNum];

    // if the index of the nonPredictedCent is not the same as the clientNum
    // then this is a fake player (like on teh single player podiums), so
    // go ahead and use the cent
    if( ( nonPredictedCent - cg_entities ) != cent->currentState.clientNum ) {
        nonPredictedCent = cent;
    }

    //
    // shell ejection
    //
    if ( cg_brassTime.value > 0 && cent->brassEjected > 0 )
    {
        if ( weaponNum == WP_SW629 && cg.time - cent->muzzleFlashTime < ( SW629_BRASS_EJECTTIME+MUZZLE_FLASH_TIME ) )
            ;
        else if ( weaponNum == WP_MACMILLAN && cg.time - cent->muzzleFlashTime < MACMILLAN_BRASS_EJECTTIME )
            ;
        else {
            CG_GetOriginFromTag( &hand, weapon->handsModel, "tag_ejection", origin );

            CG_EjectBrass( cent, origin , weapon->ejectBrassType, qtrue );

            cent->brassEjected--;
        }
    }

    //
    // Laser Beam
    //
    if ( cent->currentState.eFlags & EF_LASERSIGHT )
    {
        trace_t		trace;
        vec3_t			muzzlePoint, endPoint;
        refEntity_t		beam;
        vec3_t		angles;
        vec3_t		f_org;
        qhandle_t laser;
        vec4_t rgba;
        vec3_t forward;

        rgba[0] = rgba[1] = rgba[2] = rgba[3] = 1;

        memset( &beam, 0, sizeof( beam ) );
        memset( &flash, 0, sizeof( flash ) );

        CG_PositionEntityOnTag(&flash,&hand, hand.hModel ,"tag_laser");

        // find muzzle point for this frame
        VectorCopy ( flash.origin,muzzlePoint );

        CG_CalculateWeaponPosition( f_org, angles );
        AngleVectors( angles  , forward, NULL, NULL );

        // FIXME: crouch
        VectorMA( cent->currentState.pos.trBase , 200, forward, endPoint );
        {
            int anim;

            anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;

            if ( anim == LEGS_WALKCR || anim == LEGS_IDLECR )
                endPoint[2] += CROUCH_VIEWHEIGHT;
            else
                endPoint[2] += DEFAULT_VIEWHEIGHT;
        }

        laser =  trap_R_RegisterShader( "gfx/misc/ns_laserbeam" );
        // see if it hit a wall
        CG_Trace( &trace, muzzlePoint, vec3_origin, vec3_origin, endPoint,
                  cent->currentState.number, MASK_SHOT );

        VectorCopy( trace.endpos , endPoint );

        if (! (CG_PointContents( muzzlePoint, cent->currentState.number ) & CONTENTS_SOLID)
                &&
                !trace.startsolid )
        {
            vec3_t	angles;

            // draw the beam
            CG_Tracer( muzzlePoint, endPoint, 0.1f, laser , rgba );

            // add the impact flare
            beam.customShader = cgs.media.laserShader;
            beam.reType = RT_SPRITE;
            beam.radius = 0.5;

            VectorMA( endPoint, -0.4, forward, beam.origin );

            // make a random orientation
            angles[0] = rand() % 360;
            angles[1] = rand() % 360;
            angles[2] = rand() % 360;
            AnglesToAxis( angles, beam.axis );
            trap_R_AddRefEntityToScene( &beam );
        }
    }

    //
    // Gun fire Puff
    //
    if ( cent->gunSmokePuff && cg_gunSmoke.integer == 1 )
    {
        vec3_t up;
        localEntity_t *smoke;
        vec3_t forward;

        memset( &flash, 0, sizeof( flash ) );

        VectorClear( up );

        // find muzzle point for this frame
        CG_CalculateWeaponPosition( flash.origin, angles );
        AngleVectors( angles  , forward, NULL, NULL );
        VectorMA( up, 15, forward, up );

        up[2] = 5;

        if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_SILENCER ) &&
                ( weaponNum != WP_AK47 && weaponNum != WP_M4 ) )
            CG_PositionRotatedEntityOnTag( &flash, &hand, hand.hModel, "tag_flash2");
        else
            CG_PositionRotatedEntityOnTag( &flash, &hand, hand.hModel, "tag_flash");

        cent->gunSmokePuff = qfalse;

        smoke = CG_SmokePuff( flash.origin, up, 7.5f,1,1,1,1,225,cg.time,0,LE_MOVE_SCALE_FADE, cgs.media.smokePuffShader  );
        smoke->leFlags |= LEF_3RDPERSON;

        smoke->refEntity.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON | RF_LIGHTING_ORIGIN | RF_NOSHADOW;

    }
#if 1
    //
    // Gunsmoke
    //
    if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME )
    {

        /*
        if ( cent->gunSmokeTime - cg.time > 0 &&
        ( !BG_IsGrenade(weaponNum) && !BG_IsMelee( weaponNum) && weaponNum != WP_C4 ) && cg_gunSmoke.integer > 0 )
        {
        vec3_t up;
        localEntity_t *smoke;

        memset( &flash, 0, sizeof( flash ) );

        if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_SILENCER ) )
        CG_PositionRotatedEntityOnTag( &flash, &hand, hand.hModel, "tag_flash2");
        else
        CG_PositionRotatedEntityOnTag( &flash, &hand, hand.hModel, "tag_flash");

        up[0] = crandom()*5;
        up[1] = crandom()*5;

        up[2] = 20 + crandom()*5;

        if ( cent->gunSmokeTime - cg.time < 1000 )
        {
        float t;

        t= (float)((float)( cent->gunSmokeTime - cg.time ) / 1000 );  //  / 2000

        smoke = CG_SmokePuff(  flash.origin, up, cg_gunSmokeTime.integer / 333 + 1,1,1,1,t,cg_gunSmokeTime.integer,cg.time,0,LE_MOVE_SCALE_FADE, cgs.media.smokePuffShader  );


        //smoke = CG_SmokePuff(  flash.origin, up, 3,1,1,1,t,350,cg.time,0,LE_MOVE_SCALE_FADE, cgs.media.smokePuffShader );
        }
        else
        smoke = CG_SmokePuff(  flash.origin, up, cg_gunSmokeTime.integer / 333 + 1,1,1,1,1,cg_gunSmokeTime.integer,cg.time,0,LE_MOVE_SCALE_FADE, cgs.media.smokePuffShader  );
        //smoke = CG_SmokePuff(  flash.origin, up, 3,1,1,1,0.5,350,cg.time,0,LE_MOVE_SCALE_FADE, cgs.media.smokePuffShader );
        smoke->leFlags |= LEF_3RDPERSON;
        }*/
        return;
    }
#endif
    // Got a silencer, no muzzle flash
    if ( ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_SILENCER ) ||
            ps->weapon == WP_SL8SD ) &&
            !( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) && ps->stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ))
       )
    {
        if ( cent->gunSmokePuff )
        {
            vec3_t up;
            vec3_t forward;
            localEntity_t *smoke;

            memset( &flash, 0, sizeof( flash ) );


            VectorClear( up );
            // find muzzle point for this frame
            CG_CalculateWeaponPosition( flash.origin, angles );
            AngleVectors( angles  , forward, NULL, NULL );
            VectorMA( up, 15, forward, up );

            up[2] = 5;

            if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_SILENCER ) && ( weaponNum != WP_AK47 && weaponNum != WP_M4 ) )
                CG_PositionRotatedEntityOnTag( &flash, &hand, hand.hModel, "tag_flash2");
            else
                CG_PositionRotatedEntityOnTag( &flash, &hand, hand.hModel, "tag_flash");

            cent->gunSmokePuff = qfalse;

            smoke = CG_SmokePuff( flash.origin, up, 7.5f,1,1,1,1,225,cg.time,0,LE_MOVE_SCALE_FADE, cgs.media.smokePuffShader  );
            smoke->leFlags |= LEF_3RDPERSON;
        }
        return;
    }


    //
    // impulse flash
    //
    memset( &flash, 0, sizeof( flash ) );
    VectorCopy( hand.lightingOrigin, flash.lightingOrigin );
    flash.shadowPlane = hand.shadowPlane;
    flash.renderfx = hand.renderfx;

    flash.hModel = weapon->flashModel;

    if (!flash.hModel) {
        return;
    }

    angles[YAW] = 0;
    angles[PITCH] = 0;
    angles[ROLL] = 0;

    {
        float size;
        int i;
        int a;

        size = random() + 0.5f;

        if (!BG_IsRifle(weaponNum))
            angles[ROLL] = rand() % 360;

        AnglesToAxis( angles, flash.axis );

        if ( size > 1.50f )
            size = 1.50f;

        for ( i = 0; i < 3; i++ )
            for ( a = 0; a < 3; a++ )
                flash.axis[i][a] *= size;
    }


    if ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) && ps->stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ))
        CG_PositionRotatedEntityOnTag( &flash, &hand, weapon->handsModel, "tag_flash2");
    else
        CG_PositionRotatedEntityOnTag( &flash, &hand, weapon->handsModel, "tag_flash");

    trap_R_AddRefEntityToScene( &flash );

    // make a dlight for the flash
    if ( weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2] ) {
        trap_R_AddLightToScene( flash.origin, 200 + (rand()&31), weapon->flashDlightColor[0],
                                weapon->flashDlightColor[1], weapon->flashDlightColor[2] );
    }
}
