//
// all code in this file is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "g_local.h"

int lastbombclient[TEAM_NUM_TEAMS];
int	lastvip[TEAM_NUM_TEAMS];

vmCvar_t	g_aimCorrect;

void NS_BotRadioMsg( gentity_t *ent, char *msg );
void NS_SendResetAllStatusToAllPlayers( void );
void NS_SendPlayersStatusToAllPlayers( int clientNum, int status );
void Touch_Multi( gentity_t *self, gentity_t *other, trace_t *trace );
qboolean InView ( vec3_t vViewer, vec3_t vViewAngles, vec3_t vTestPoint );

/*
============
G_FindEntityInRadius
============
*/
gentity_t *G_FindEntityInRadius ( vec3_t origin, float radius, gentity_t *ignore, char *classname) {
    float		dist;
    gentity_t	*ent;
    int			entityList[MAX_GENTITIES];
    int			numListedEntities;
    vec3_t		mins, maxs;
    vec3_t		v;
    int			i, e;

    if ( radius < 1 ) {
        radius = 1;
    }

    for ( i = 0 ; i < 3 ; i++ ) {
        mins[i] = origin[i] - radius;
        maxs[i] = origin[i] + radius;
    }

    numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

    for ( e = 0 ; e < numListedEntities ; e++ ) {
        ent = &g_entities[entityList[ e ]];

        if (ent == ignore)
            continue;
        if (ent->classname != classname && ( !is_func_explosive(ent) && !Q_stricmp(classname , "func_explosive")  ) )
            continue;
        if (ent->r.svFlags & SVF_NOCLIENT )
            continue;

        // find the distance from the edge of the bounding box
        for ( i = 0 ; i < 3 ; i++ ) {
            if ( origin[i] < ent->r.absmin[i] ) {
                v[i] = ent->r.absmin[i] - origin[i];
            } else if ( origin[i] > ent->r.absmax[i] ) {
                v[i] = origin[i] - ent->r.absmax[i];
            } else {
                v[i] = 0;
            }
        }

        dist = VectorLength( v );
        if ( dist >= radius ) {
            continue;
        }

        // we found it!
        return ent;
    }

    return NULL;
}


/*
=================
NSQ3 Get Weapon Time On Lower
author: Defcon-X
date:
description: returns the time a weapon needs to putaway
=================
*/
int GetWeaponTimeOnLower( gentity_t *ent ) {

    switch (ent->client->ps.weapon) {
    case WP_KHURKURI:
    case WP_SEALKNIFE:
        return 400;
    case WP_MAC10:
        return 400;
    case WP_MP5:
        return 400;
    case WP_M14:
    case WP_M4:
    case WP_M249:
        return 400;
    case WP_AK47:
        return 400;
    case WP_GLOCK:
    case WP_MK23:
    case WP_P9S:
    case WP_SW40T:
        return 400;
    case WP_MACMILLAN:
        return 500;
#ifdef SL8SD
    case WP_SL8SD:
        return 600;
#endif
    case WP_PSG1:
        return 840;
    case WP_870:
        return 600;
    case WP_M590:
        return 600;
    case WP_SPAS15:
        return 400;
    case WP_PDW:
        return 400;
    case WP_DEAGLE:
        return 400;
    case WP_SW629:
        return 400;
    default:
        return 100;
    }
}

/*
=================
NSQ3 Get Weapon Time On Raise
author: Defcon-X
date:
description: returns the time a weapon needs to raise
=================
*/
int GetWeaponTimeOnRaise (gentity_t *ent) {

    switch (ent->client->ps.weapon) {
    case WP_KHURKURI:
    case WP_SEALKNIFE:
        return 480;
    case WP_M249:
        return 800;
    case WP_MP5:
        return 640;
    case WP_M4:
        return 880;
    case WP_AK47:
        return 400;
    case WP_SW40T:
    case WP_DEAGLE:
    case WP_MAC10:
        return 600;
    case WP_MK23:
        return 600;
    case WP_MACMILLAN:
        return 750;
#ifdef	SL8SD
    case WP_SL8SD:
        return 800;
#endif
    case WP_PSG1:
        return 600;
    case WP_PDW:
    case WP_M14:
        return 400;
    case WP_870:
        return 600;
    case WP_SPAS15:
        return 600;
    case WP_GLOCK:
        return 400;
    case WP_M590:
        return 600;
    case WP_P9S:
        return 600;
    case WP_SW629:
        return 600;
    default:
        return 100;
    }
}

/*
=================
NSQ3 IsBot
author: dX
date: 15-02-99
description: returns true, if ent is bot
=================
*/
qboolean NS_IsBot(gentity_t *ent)
{
    if (!ent)
        return qfalse;

    if (ent->r.svFlags & SVF_BOT)
        return qtrue;
    return qfalse;
}



/*
=================
NSQ3 Reload
author: dX
date: 15-02-99
description: Reload weapon, if possible
=================
*/
void Cmd_Reload_f (gentity_t *ent)
{
    int _weapon = ent->s.weapon, ammo;
    qboolean say_error = qfalse;
    qboolean last_rnd = qfalse;
    int	addTime;
    char *message = "cp \"No Ammo\n\"";
    gitem_t		*item;

    //  \\__        _   ./\.   _
    //  //\_\__/\__/ \_/    \_| \_/
    //  \\/_/  \/  \_/ \.  ./ |_/ \_:
    //  //               \/

    // null isn't valid! (also prevents against world) --- and if dead also return
    if (_weapon <= 0 || ent->r.contents == CONTENTS_CORPSE)
        return;

    // find the item type for this weapon
    item = BG_FindItemForWeapon( _weapon );

    ammo = item->giAmmoTag;

    ent->client->ns.reload_tries = 0;
    // no khurkuri reloading
    if ( BG_IsMelee( _weapon) || BG_IsGrenade( _weapon) ||
            _weapon == WP_C4 )
        return;

    if ( ent->client->ps.pm_flags  & PMF_CLIMB )
    {
        PrintMsg( ent, "You can't reload while climbing.\n");
        return;
    }

    if ( ent->client->ps.weaponstate == WEAPON_RELOADING || ent->client->ps.weaponstate == WEAPON_RELOADING_EMPTY ||  ent->client->ps.weaponstate == WEAPON_RELOADING_CYCLE || ent->client->ps.weaponstate == WEAPON_RELOADING_STOP || ent->client->ps.weaponstate == WEAPON_BANDAGING || ent->client->ps.weaponstate == WEAPON_BANDAGING_START || ent->client->ps.weaponstate == WEAPON_BANDAGING_END )
        return;

    if ( ent->client->ps.eFlags & EF_IRONSIGHT )
    {
        PrintMsg(ent, "You can't reload while in Ironsight-Mode.\n");
        return;
    }

    if ( ( _weapon == WP_870 || _weapon == WP_M590 ) && ent->client->ns.rounds[_weapon] == BG_GetMaxRoundForWeapon( _weapon ) )
        return;

    if ( ent->client->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) &&  ent->client->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ) && ( _weapon == WP_M4 || _weapon == WP_AK47 ) )
    {
        // already full?
        if ( ent->client->ns.rounds[_weapon*3] >= 1 )
            return;
        if ( ent->client->ps.ammo[AM_40MMGRENADES] <= 0 )
            return;

        // max out our current rounds again
        ent->client->ns.rounds[_weapon*3] = 1;
        ent->client->ps.ammo[AM_40MMGRENADES]--;
        last_rnd = qfalse;
        addTime = 2000;
        goto end;
    }

    if ( _weapon == WP_PDW && ent->client->ns.weaponmode[_weapon] & ( 1 << WM_WEAPONMODE2 ) )
    {
        ent->client->ns.weaponmode[_weapon] &= ~( 1 << WM_WEAPONMODE2 );
        ent->client->ps.weaponstate = WEAPON_RELOADING_CYCLE;
        ent->client->ps.weaponTime = 400;
        return;
    }

    // no ammo?
    if ( ent->client->ps.ammo[ ammo ] <= 0 )
        say_error = qtrue;

    if (say_error)
    {
        trap_SendServerCommand( ent->client->ps.clientNum, message );
        return;
    }

    // is a lastrnd weapon?
    if (ent->client->ns.rounds[_weapon] <= 0 && (_weapon == WP_PDW || _weapon == WP_M249 || _weapon == WP_M14 || _weapon==WP_SW40T || _weapon==WP_SPAS15 || _weapon == WP_MK23 || _weapon == WP_GLOCK || _weapon == WP_MACMILLAN || _weapon == WP_MP5 || _weapon == WP_MAC10 || _weapon == WP_AK47 || _weapon == WP_P9S || _weapon == WP_DEAGLE || _weapon == WP_M4 || _weapon == WP_PSG1
#ifdef SL8SD
            || _weapon == WP_SL8SD
#endif
                                                ) )
        last_rnd = qtrue;

    // give full clips size
    if ( ( _weapon == WP_870 || _weapon == WP_M590 ) && ent->client->ns.rounds[_weapon] < BG_GetMaxRoundForWeapon( _weapon ) )
        ;
    else {
        // special case for m249
        if ( _weapon != WP_M249 )
            ent->client->ns.rounds[_weapon] = BG_GetMaxRoundForWeapon( _weapon );
        ent->client->ps.ammo[ ammo ]--;
    }


    // when reloading
    if ( ent->client->ns.weaponmode[_weapon] & ( 1 << WM_ZOOM4X) || ent->client->ns.weaponmode[_weapon] & (1 << WM_ZOOM2X))
    {
        // go out of zoom
        ent->client->ns.weaponmode[_weapon] &= ~( 1 << WM_ZOOM4X );
        ent->client->ns.weaponmode[_weapon] &= ~( 1 << WM_ZOOM2X );
    }

    // set state
    switch (_weapon) {
    case WP_MP5:
        addTime = 2000; // same as lastrnd
        break;
    case WP_P9S:
        addTime = 2400;

        if (last_rnd)
            addTime = 3000;
        break;
    case WP_MK23:
        addTime = 2400;

        if (last_rnd)
            addTime = 2464;
        break;
    case WP_MAC10:
        addTime = 3000;
        break;
    case WP_AK47:
        addTime = 2400;

        if (last_rnd)
            addTime = 2850;
        break;
    case WP_M249:
        addTime = 5200;
        break;
    case WP_M4:
        addTime = 2571;
        break;
    case WP_870:
        addTime = 400;
        break;
    case WP_M590:
        addTime = 400;
        break;
    case WP_SPAS15:
        addTime = 2400;
        break;
    case WP_PSG1:
        addTime = 2333;
        break;
    case WP_M14:
        addTime = 3400;
        break;
    case WP_PDW:
        addTime = 2000;
        break;
    case WP_GLOCK:
        addTime = 2800;
        break;
    case WP_MACMILLAN:
        addTime = 1800;
        break;
    case WP_DEAGLE:
        addTime = 2000;
        break;
    case WP_SW629:
        addTime = 3400;
        break;
    case WP_SW40T:
        addTime = 2400;
        if (last_rnd)
            addTime = 2760;
        break;
#ifdef SL8SD
    case WP_SL8SD:
        addTime = 2000;
        break;
#endif
    default:
        addTime = 2000;
        break;
    }

end:
    if (_weapon != WP_870 && _weapon != WP_M590 ) {
        if (last_rnd && _weapon != WP_MACMILLAN)
            G_AddEvent( ent, EV_RELOAD_EMPTY, 0 );
        else
            G_AddEvent( ent, EV_RELOAD, 0 );
    }

    ent->client->ps.weaponTime = addTime;
    // fix me to reload
    if (BG_IsPistol( _weapon ) || _weapon == WP_MAC10 )
        ent->client->ps.torsoAnim = ( ( ent->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
                                    | TORSO_RELOAD_PISTOL;
    else
        ent->client->ps.torsoAnim = ( ( ent->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
                                    | TORSO_RELOAD_RIFLE;

    ent->client->ps.torsoTimer = addTime;

    if ( last_rnd )
        ent->client->ps.weaponstate = WEAPON_RELOADING_EMPTY;
    else
        ent->client->ps.weaponstate = WEAPON_RELOADING;

    // stop recentering our screen if we're reloading
    ent->client->weaponangle_changed[0]= ent->client->weaponangle_changed[1]=ent->client->weaponangle_changed[2] = qfalse;
}

/*
==============

LOCATIONAL DAMAGE
( needs to be improved [damn new PPMs] )
+done

==============
*/
qboolean pointinback (gentity_t *self, vec3_t point)
{
    vec3_t	vec;
    float	dot;
    vec3_t	forward;

    if ( !self->client )
        return qfalse;

    AngleVectors (self->client->ps.viewangles, forward, NULL, NULL);
    VectorSubtract (point, self->r.currentOrigin, vec);
    VectorNormalize (vec);
    dot = DotProduct (vec, forward);

    if (dot < 0.3)
        return qtrue;
    return qfalse;
}
qboolean pointinfront (gentity_t *self, vec3_t point, float mod)
{
    vec3_t	vec;
    float	dot;
    vec3_t	forward;

    if ( !self->client )
        return qfalse;

    AngleVectors (self->client->ps.viewangles, forward, NULL, NULL);
    VectorCopy(self->r.currentOrigin, vec);
    vec[2]+=mod;
    VectorSubtract (point, vec, vec);
    VectorNormalize (vec);
    dot = DotProduct (vec, forward);

    if (dot < 0.3)
        return qfalse;
    return qtrue;
}
qboolean pointabove (gentity_t *self, vec3_t point)
{
    vec3_t	vec;
    float	dot;
    vec3_t	up;

    if ( !self->client )
        return qfalse;

    AngleVectors (self->client->ps.viewangles, NULL, NULL, up);
    VectorSubtract (point, self->r.currentOrigin, vec);
    VectorNormalize (vec);
    dot = DotProduct (vec, up);

    if (dot > 0.3)
        return qtrue;
    return qfalse;
}
qboolean pointbelow (gentity_t *self, vec3_t point)
{
    vec3_t	vec;
    float	dot;
    vec3_t	up;

    if ( !self->client )
        return qfalse;

    AngleVectors (self->client->ps.viewangles, NULL, NULL, up);
    VectorSubtract (point, self->r.currentOrigin, vec);
    VectorNormalize (vec);
    dot = DotProduct (vec, up);

    if (dot < -0.3)
        return qtrue;
    return qfalse;
}
qboolean inback (gentity_t *self, vec3_t point, float mod)
{
    vec3_t	vec;
    float	dot;
    vec3_t	forward;

    AngleVectors (self->s.angles, forward, NULL, NULL);
    VectorCopy(self->r.currentOrigin, vec);
    vec[2]+=mod;
    VectorSubtract (point, vec, vec);
    VectorNormalize (vec);
    dot = DotProduct (vec, forward);

    if (dot < -0.3)
        return qtrue;
    return qfalse;
}
/*
=================
NSQ3 GetLocationalDamage
by: dX
date: around 10th feburary 2k
return value: returns damage as #define
=================
*/


int NS_CheckLocationDamage ( gentity_t *targ, vec3_t point, int mod) {
    vec3_t bulletPath;
    vec3_t bulletAngle;

    float clientHeight;
    float clientFeetZ;
    float clientRotation;
    float bulletHeight;
    int bulletRotation;
    int impactRotation;
    int inback = 0;

    float headMod = 7.8f;
    float chestMod = 22.0f;
    float stomachMod = 14.0f;

    if ( targ->client && ( targ->client->ps.pm_flags & PMF_DUCKED ) )
    {
        headMod = 18.96f;
        chestMod = 31.16f;
        stomachMod = 26.19f; // where the stomach begins
    }


    // Point[2] is the REAL world Z. We want Z relative to the clients feet

    // Where the feet are at
    clientFeetZ  = targ->r.currentOrigin[2] + targ->r.mins[2];
    // How tall the client is
    clientHeight = targ->r.maxs[2] - targ->r.mins[2];
    // Where the bullet struck
    bulletHeight = point[2] - clientFeetZ;

    // Get a vector aiming from the client to the bullet hit
    VectorSubtract(targ->r.currentOrigin, point, bulletPath);
    // Convert it into PITCH, ROLL, YAW
    vectoangles(bulletPath, bulletAngle);

    clientRotation = targ->client->ps.viewangles[YAW];
    bulletRotation = bulletAngle[YAW];

    impactRotation = abs(clientRotation-bulletRotation);

    impactRotation += 45; // just to make it easier to work with
    impactRotation = impactRotation % 360; // Keep it in the 0-359 range

    if (impactRotation < 90)
        inback = 1;
    else if (impactRotation < 180)
        inback = 2;
    else if (impactRotation < 270)
        inback = 0;
    else if (impactRotation < 360)
        inback = 3;

    if (g_debugDamage.integer == 1) G_Printf("Bulletheight: %f || Clientheight: %f || mod: %i\n", bulletHeight, clientHeight, mod);

    // The upper body never changes height, just distance from the feet
    if ( (bulletHeight > clientHeight + headMod) ) {
        if (g_debugDamage.integer == 1)	G_Printf("Headshot\n");
        return LOC_HEAD;
    } else if ( (bulletHeight > clientHeight ) && (inback == 0) ) {
        if (g_debugDamage.integer == 1)	G_Printf("Faceshot\n");
        return  LOC_FACE;
    } else if ( (bulletHeight > clientHeight ) ) {
        if (g_debugDamage.integer == 1)	G_Printf("Headshot\n");
        return LOC_HEAD;
    } else if ( (bulletHeight > clientHeight - stomachMod) && (inback == 0) ) {
        if (g_debugDamage.integer == 1)	G_Printf("Chestshot\n");
        return LOC_CHEST;
    } else if ( (bulletHeight > clientHeight - stomachMod) && (inback == 2) ) {
        if (g_debugDamage.integer == 1)	G_Printf("Rightarmshot\n");
        return LOC_RIGHTARM;
    } else if ( (bulletHeight > clientHeight - stomachMod) && (inback == 3) ) {
        if (g_debugDamage.integer == 1)	G_Printf("Leftarmshot\n");
        return LOC_LEFTARM;
    } else if ( (bulletHeight > clientHeight - chestMod) && (inback == 1) ) {
        if (g_debugDamage.integer == 1)	G_Printf("Backshot\n");
        return LOC_BACK;
    } else if ( (bulletHeight > clientHeight - chestMod) ) {
        if (g_debugDamage.integer == 1)	G_Printf("Stomachshot\n");
        return LOC_STOMACH;
    } else if ( inback == 2 || inback == 0) {
        if (g_debugDamage.integer == 1)	G_Printf("Rightlegshot\n");
        return LOC_RIGHTLEG;
    } else {
        if (g_debugDamage.integer == 1)	G_Printf("Leftlegshot\n");
        return LOC_LEFTLEG;
    }

}

void NS_SetGameState(int state) {
    int i;

    if (state != STATE_OPEN &&
            state != STATE_LOCKED &&
            state != STATE_OVER) return;

    if (state == GameState) return;

    GameState = state;

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if ( level.clients[i].pers.connected == CON_DISCONNECTED ) continue;
        if ( g_gametype.integer == GT_LTS && level.clients[i].pers.connected != CON_CONNECTED ) continue;
        G_AddEvent(&g_entities[level.clients[i].ps.clientNum], EV_GAMESTATE, state);
    }

}

/*
=================
NSQ3 Cause Bleeding
author: dX
date: 13-02-99
description: causes bleeding...
=================
*/
void NS_CauseBleeding(gentity_t *ent)
{
    int damage = 0;
    int CURRENT = 1;

    // if dead or no wounds,
    if ( !(ent->client->bleed_num) || (ent->client->ps.pm_type == PM_DEAD)  )
        return; // return

    for (CURRENT = 1; CURRENT <= ent->client->bleed_num;) {
        // if wound should get to bleed now:
        if (ent->client->bleed_delay[CURRENT] < level.time)
        {
            gentity_t		*tent;
            vec3_t pos;// postition for bleeding
            vec3_t	dir;
            int dmg = -1;
            int	nextbleed;


            damage = ent->client->bleed_point[CURRENT]; // should really be fixed

            // inflict damage
            dmg = G_Damage(ent, ent->client->bleed_causer[CURRENT],ent->client->bleed_causer[CURRENT],NULL,NULL,damage,DAMAGE_NO_ARMOR, MOD_BLEED);

            // damage :)
            if ( damage > 1 )
                // if damage > 1 , turn screen red
                ent->client->damage_blood += damage; // for feedback

            // next bleed in +X secs
            if ( damage > 35 )
                damage = 35;

            nextbleed = 3500 - damage * 100 + ent->client->pers.nsPC.strength * 50 + random()*100;

            if ( nextbleed < 0 )
                nextbleed = 100;

            ent->client->bleed_delay[CURRENT] = level.time + nextbleed;

            VectorAdd(ent->client->bleed_loc[CURRENT], ent->r.absmax, pos);
#if DEBUG_BUILD
            G_Printf("%s[%i]: I bleed with %i , next time @ %f\n",ent->client->pers.netname, CURRENT, damage, ent->client->bleed_delay [CURRENT] ); // tell me that I bleed
#endif

            AngleVectors( ent->client->ps.viewangles , dir,NULL,NULL);

            SnapVector( pos );

            tent = G_TempEntity( pos, EV_BLOODER );
            tent->s.eventParm = dmg;
            tent->s.otherEntityNum = ent->s.number;

        }

        // current++
        CURRENT += 1;
    }
}

/*
=================
NSQ3 Find Radius
author: dX
date: 15-02-99
description: basically the same as the q2 function
=================
*/
gentity_t *NS_FindRadius (gentity_t *ent, vec3_t org, float rad) {

    vec3_t eorg;
    int j;

    if (!ent)
        ent = g_entities;
    else
        ent++;

    for (; ent < &g_entities[level.num_entities]; ent++)
    {
        if (!ent->inuse)
            continue;

        for (j=0; j<3; j++)
            eorg[j] = org[j] - (ent->r.currentOrigin[j] + (ent->r.mins[j] + ent->r.maxs[j])*0.5);

        if (VectorLength(eorg) > rad)
            continue;

        return ent;
    }
    return NULL;
}

/*
=================
NSQ3 Spray Blood
author: dX
date: 15-02-99
description: sprays blood, from vector start, facing dir
=================
*/
void NS_SprayBlood (gentity_t *ent, vec3_t start, vec3_t dir, int damage, qboolean brain ) {
    trace_t		tr;
    vec3_t		end,right,up;
    gentity_t		*tent;
    gentity_t		*traceEnt;
    float			length = 500;
    float		r;
    float		u;
    // spread it a little bit
    r = crandom()*12.5;
    u = crandom()*125;

    AngleVectors (ent->client->ps.viewangles, NULL, right, up);

    VectorMA (start, length, dir, end);

    VectorMA (end, r, right, end);
    VectorMA (end, u, up, end);

    trap_Trace (&tr, start, NULL, NULL, end, ent->s.number, MASK_SHOT);

    traceEnt = &g_entities[ tr.entityNum ];

    if (traceEnt->client) // if a client
        return;

    if (!tr.plane.normal)
        return;

    tent = G_TempEntity( tr.endpos, EV_BLOOD_ON_WALL );
    tent->s.eventParm = DirToByte( tr.plane.normal ); // plane vector
    if (brain) {
        tent->s.generic1 = DirToByte( dir );
        tent->s.otherEntityNum = ent->s.number; // who got shot
    }						  // only needed when brain damaged
    tent->s.torsoAnim = damage;		  // amount of damage inflicted
    tent->s.legsAnim = brain;			  // headshot

    SnapVector( tent->s.origin );
}

/*
=================
NSQ3 Clear Inventory
author: dX
date: 15-07-2k
description: Clears clients inventory
=================
*/
void NS_ClearInventory ( gentity_t *ent )
{
    int i;

    for ( i = AM_NUM_AMMO-1; i >= 0; i-- )
    {
        ent->client->pers.nsInven.ammo[i] = 0;
    }

    // take all xp
    NS_GiveXP( ent, 999 , qtrue);

    ent->client->pers.nsInven.primaryweapon = WP_NONE;
    ent->client->pers.nsInven.secondaryweapon = WP_NONE;

    ent->client->ps.stats[STAT_WEAPONS] = ent->client->ps.stats[STAT_WEAPONS2] = 0;
    memset(ent->client->ps.ammo, 0, sizeof(ent->client->ps.ammo ) );
}
void NS_HandleCreateClassMenu ( gentity_t *ent, int menuSlot );
void NS_HandleEquipmentMenu ( gentity_t *ent, int menuSlot );

#define SAY_TEAM 1
/*
==================
Bot Hacks... Adjust XP + stuff when spawning
==================
*/
void NS_BotHacks ( gentity_t *ent )
{
    int xp;
    float rnd = random();
    gitem_t	*weapon;

    if (!ent)
        return;

    xp = ent->client->pers.nsPC.xp;

    if (ent->client->sess.waiting)
        return;
    if (ent->health <= 0)
        return;
    if (!ent->client->ps.weapon)
        return;

    if (g_gametype.integer <= GT_TEAM)
        return;
    if (ent->client->ps.weapon == WP_NONE || !ent->client->ps.weapon )
        return;

    weapon = BG_FindItemForWeapon( ent->client->ps.weapon  );

    /*
    accuracy
    speed
    stamina
    stealth
    strength
    */

    if ( rnd < 0.2 || BG_WeaponMods( ent->client->ps.weapon ) & ( 1 << WM_LASER ) || BG_WeaponMods( ent->client->ps.weapon ) & ( 1 << WM_SCOPE )  && ent->client->pers.nsPC.accuracy < 7)
    {
        NS_HandleCreateClassMenu( ent, 1 );
    }
    else if ( rnd < 0.4  || BG_WeaponMods( ent->client->ps.weapon ) & ( 1 << WM_SILENCER )   && ent->client->pers.nsPC.stealth < 6)
    {
        NS_HandleCreateClassMenu( ent, 4 );
    }
    else if ( rnd < 0.6   && ent->client->pers.nsPC.speed < 10 )
    {
        NS_HandleCreateClassMenu( ent, 2 );
    }
    else if ( rnd < 0.8   && ent->client->pers.nsPC.stamina < 10 ) {
        NS_HandleCreateClassMenu( ent, 3 );
    }
    else if ( rnd < 1 || BG_WeaponMods( ent->client->ps.weapon ) & ( 1 << WM_BAYONET ) && ent->client->pers.nsPC.strength < 7) {
        NS_HandleCreateClassMenu( ent, 5 );
    }

    // primary silencer
    if ( ent->client->pers.nsPC.stealth >= 5 && !ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].silencer && BG_WeaponMods( ent->client->ps.weapon ) & ( 1 << WM_SILENCER ) )  {
        NS_HandleEquipmentMenu( ent, 3 );
        G_Say( ent, NULL, SAY_TEAM, va("I just equipped my %s with a %s", weapon->pickup_name, "SILENCER" ));
    }

    // secondary weapon silencer
    if ( ent->client->pers.nsPC.stealth >= 3 && !ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].silencer && BG_WeaponMods( ent->client->ps.weapon ) & ( 1 << WM_SILENCER )) {
        NS_HandleEquipmentMenu( ent, 7 );
        //	G_Say( ent, NULL, SAY_TEAM, va("I just equipped my %s with a %s", weapon->pickup_name, "SILENCER" );
    }

    if ( ent->client->pers.nsPC.strength >= 4 && !ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].bayonet && BG_WeaponMods( ent->client->ps.weapon ) & ( 1 << WM_BAYONET ) ) {
        G_Say( ent, NULL, SAY_TEAM, va("I just equipped my %s with a %s", weapon->pickup_name, "BAYONET") );
        NS_HandleEquipmentMenu( ent, 6 );
    }

    // accuracy
    if ( ent->client->pers.nsPC.accuracy >= 4 && !ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].lasersight && BG_WeaponMods( ent->client->ps.weapon ) & ( 1 << WM_LASER )) {
        NS_HandleEquipmentMenu( ent, 5 );
        G_Say( ent, NULL, SAY_TEAM, va("I just equipped my %s with a %s", weapon->pickup_name, "LASERSIGHT" ));
    }

    if ( ent->client->pers.nsPC.accuracy >= 6 && !ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].scope && BG_WeaponMods( ent->client->ps.weapon ) & ( 1 << WM_SCOPE )) {
        NS_HandleEquipmentMenu( ent, 4 );
        G_Say( ent, NULL, SAY_TEAM, va("I just equipped my %s with a %s", weapon->pickup_name, "SCOPE") );
    }

    // turn on lasersight
    if ( ent->client->pers.nsInven.weapon_mods[ent->client->ps.weapon].lasersight )
        NS_WeaponMode( ent, 1 );
}
/*
=================
NSQ3 Client init
author: dX
date: 15-02-99
description: sets client stats
=================
*/
void NS_NavySeals_ClientInit (gentity_t *ent, qboolean roundbegin)
{
    int i;

    if (!ent)
        return;

    if (!ent->client)
        return;

    for (i=0; i<32;i++)
    {
        // max out weaps
        ent->client->ns.rounds[i] = BG_GetMaxRoundForWeapon(i);
    }
    ent->client->ns.rounds[WP_AK47*3] = 1;
    ent->client->ns.rounds[WP_M4*3] = 1;

    ent->client->linkedto = -1;

    if ( roundbegin )
    {
        // return to open channel
        ent->client->ns.radio_channel = 0;

        // clear amount of people killed this round
        ent->client->ns.num_killed = 0;

        // resert locational damage values
        ent->client->ps.stats[STAT_LEG_DAMAGE] = 0;
        ent->client->ps.stats[STAT_ARM_DAMAGE] = 0;
        ent->client->ps.stats[STAT_STOMACH_DAMAGE] = 0;
        ent->client->ps.stats[STAT_CHEST_DAMAGE] = 0;
        ent->client->ps.stats[STAT_HEAD_DAMAGE] = 0;
        ent->client->ps.stats[STAT_STAMINA] = 300;
    }

    // copy stealth value to playerstate struct
    ent->client->ps.stats[STAT_STEALTH] = ent->client->pers.nsPC.stealth;

    // weapon loadout
    ent->client->ps.powerups[PW_VEST] = ent->client->pers.nsInven.powerups[PW_VEST];
    ent->client->ps.powerups[PW_HELMET] = ent->client->pers.nsInven.powerups[PW_HELMET];

    if ( ent->client->pers.nsPC.technical >= 2 )
        ent->client->ns.got_defusekit = qtrue;
    else
        ent->client->ns.got_defusekit = qfalse;

    // add weaps
    if (1/*g_gametype.integer >= GT_TEAM ||g_gametype.integer == GT_*/) {
        //	G_Printf("Loadout:\nPrimary: %i - Secondary: %i\n9MM Clips: %i - Shells: %i\n",ent->client->pers.nsInven.primaryweapon,ent->client->pers.nsInven.secondaryweapon,ent->client->pers.nsInven.ammo[AM_9MMCLIPS],ent->client->pers.nsInven.ammo[AM_SHELLS] );

        if (g_allowKnifes.integer) {
            if ( ent->client->sess.sessionTeam == TEAM_BLUE )
                BG_PackWeapon( WP_KHURKURI , ent->client->ps.stats );
            else
                BG_PackWeapon( WP_SEALKNIFE , ent->client->ps.stats );
        }

        // add desired weapons
        if ( !g_noPrimary.integer &&
                PI_CheckWeapon(ent->client->sess.sessionTeam,
                               ent->client->pers.nsInven.primaryweapon,
                               ent->client->pers.nsPC.accuracy,
                               ent->client->pers.nsPC.strength,
                               ent->client->pers.nsPC.stamina,
                               ent->client->pers.nsPC.stealth,
                               qfalse) )
            BG_PackWeapon( ent->client->pers.nsInven.primaryweapon , ent->client->ps.stats );
        if ( !g_noSecondary.integer &&
                PI_CheckWeapon(ent->client->sess.sessionTeam,
                               ent->client->pers.nsInven.secondaryweapon,
                               ent->client->pers.nsPC.accuracy,
                               ent->client->pers.nsPC.strength,
                               ent->client->pers.nsPC.stamina,
                               ent->client->pers.nsPC.stealth,
                               qfalse) )
            BG_PackWeapon( ent->client->pers.nsInven.secondaryweapon , ent->client->ps.stats );

        for ( i = AM_NUM_AMMO-1; i >= 0; i-- ) {
            ent->client->ps.ammo[i] = ent->client->pers.nsInven.ammo[i];
        }

        if ( ent->client->ps.ammo[AM_SHOTGUN] > 0 )
            ent->client->ps.ammo[AM_SHOTGUN] *= 7;

        // special m249 code
        if ( ent->client->pers.nsInven.primaryweapon == WP_M249 )
        {
            gitem_t *item = BG_FindItemForWeapon( WP_M249 );

            if ( ent->client->ps.ammo[item->giAmmoTag] > 1 )
                ent->client->ps.ammo[item->giAmmoTag] = 1;
        }
        // if we got flashbangs / grenades, equip us!
        if ( g_noGrenades.integer == 0 )
        {
            if ( ent->client->ps.ammo[AM_FLASHBANGS] > 0 )
                BG_PackWeapon( WP_FLASHBANG , ent->client->ps.stats );
            if ( ent->client->ps.ammo[AM_GRENADES] > 0 )
                BG_PackWeapon( WP_GRENADE , ent->client->ps.stats );
            if ( ent->client->ps.ammo[AM_SMOKE] > 0 )
                BG_PackWeapon( WP_SMOKE, ent->client->ps.stats );
        }

        // reset weapon mods
        for ( i = WP_NUM_WEAPONS; i > WP_NONE; i-- )
            ent->client->ns.weaponmode[ i ] = 0;

        if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].silencer &&
                (ent->client->pers.nsPC.stealth > 4) )
            ent->client->ns.weaponmode[ ent->client->pers.nsInven.primaryweapon ] |= ( 1 << WM_SILENCER );

        if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].lasersight &&
                (ent->client->pers.nsPC.accuracy > 3) )
            ent->client->ns.weaponmode[ ent->client->pers.nsInven.primaryweapon ] |= ( 1 << WM_LASER );

        if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].scope &&
                (ent->client->pers.nsPC.accuracy > 5) )
            ent->client->ns.weaponmode[ ent->client->pers.nsInven.primaryweapon ] |= ( 1 << WM_SCOPE);

        if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].bayonet &&
                (ent->client->pers.nsPC.strength > 3) )
            ent->client->ns.weaponmode[ ent->client->pers.nsInven.primaryweapon ] |= ( 1 << WM_BAYONET);

        if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].silencer &&
                (ent->client->pers.nsPC.stealth > 2) )
            ent->client->ns.weaponmode[ ent->client->pers.nsInven.secondaryweapon ] |= ( 1 << WM_SILENCER );

        if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].lasersight &&
                (ent->client->pers.nsPC.accuracy > 2) )
            ent->client->ns.weaponmode[ ent->client->pers.nsInven.secondaryweapon ] |= ( 1 << WM_LASER );

        if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].gl &&
                (ent->client->pers.nsPC.technical > 4) &&
                (ent->client->pers.nsPC.strength > 2) )
            ent->client->ns.weaponmode[ ent->client->pers.nsInven.primaryweapon ] |= ( 1 << WM_GRENADELAUNCHER );

        if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].duckbill &&
                (ent->client->pers.nsPC.strength > 4) )
            ent->client->ns.weaponmode[ ent->client->pers.nsInven.primaryweapon ] |= ( 1 << WM_DUCKBILL );

        if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].flashlight &&
                (ent->client->pers.nsPC.technical > 3) )
            ent->client->ns.weaponmode[ ent->client->pers.nsInven.primaryweapon ] |= ( 1 << WM_FLASHLIGHT );


        // force the base weapon up

        for ( i = WP_SMOKE-1; i > 0; i-- )
        {
            if (BG_GotWeapon( i, ent->client->ps.stats ) )
            {
                ent->client->ps.weapon = i;
                ent->s.weapon = i;

                ent->client->ps.weaponstate = WEAPON_RAISING;
                ent->client->ps.weaponTime = GetWeaponTimeOnRaise( ent );
                break;
            }
        }

    }
    else {
        int _wp = WP_MK23;
        int _close = WP_SEALKNIFE;
        gitem_t *sec;

        if (random() < 0.5)
            _wp = WP_GLOCK;
        if (random() > 0.5)
            _close = WP_KHURKURI;

        BG_PackWeapon(_wp, ent->client->ps.stats );
        BG_PackWeapon(_close, ent->client->ps.stats );

        sec = BG_FindItemForWeapon( _wp );

        ent->client->ps.ammo[sec->giAmmoTag] = 2;

        // force the base weapon up
        ent->client->ps.weapon = _wp;
        ent->s.weapon = _wp;

        ent->client->ps.weaponstate = WEAPON_RAISING;
        ent->client->ps.weaponTime = GetWeaponTimeOnRaise( ent );
    }

    if (NS_IsBot( ent ) )
        NS_BotHacks( ent );

    // remove blownhead flag!
    //	ent->client->ps.eFlags &= ~EF_HEADBLOWN;
}


/*
==============

COMMANDS

==============
*/

/*
=================
NSQ3 Open Door
author: dX
date: 15-02-99
description: Handles interaction with objects
=================
*/

#define NS_OBJECTUSE_RANGE	100

void NS_OpenDoor ( gentity_t *ent , qboolean direct ) {
    trace_t		tr;
    vec3_t		end,forward,start,right,up;
    qboolean	useanim = qfalse;
    //gentity_t	*traceEnt = NULL/*, *tent*/;
    vec3_t		mins = { -4,-4,-4 };
    vec3_t		maxs = { 4,4,4 };

    if ( (ent->client->ps.pm_flags & PMF_BOMBCASE) )
        return;

    if ( ent->client->ps.pm_type == PM_SPECTATOR || ent->client->ps.pm_type == PM_NOCLIP || ent->client->sess.waiting )
    {
        return;
    }

    AngleVectors( ent->client->ps.viewangles, forward, right,up );

    CalcMuzzlePoint( ent, forward, right, up, start );
    // fix 7-12
    VectorMA( ent->s.pos.trBase, NS_OBJECTUSE_RANGE, forward, end );

    // snap to integer coordinates for more efficient network bandwidth usage
    //	SnapVector( start );

    trap_Trace( &tr, start, mins, maxs, end, ent->s.number, MASK_ALL );



    if ( NS_GotWounds( ent ) && ent->client->pers.useBandage )
    {
        NS_StartBandage(ent);
        return;
    }
#if 0
    if (!useanim)
    {
        traceEnt = &g_entities[ tr.entityNum ];

        if (traceEnt->classname)
        { /*
              if (!Q_stricmp( traceEnt->classname, "turret_breach" ) )
              {
              traceEnt->parent = ent;
              }
              else if (!Q_stricmp( traceEnt->classname, "c4_placed") )
              {
              traceEnt->use(traceEnt, ent , ent );
              useanim = qtrue;
              }
              if ( traceEnt->s.eType == ET_ITEM && BG_CanItemBeGrabbed(g_gametype.integer, &traceEnt->s, &ent->client->ps) )
              {
              Pick_Item ( traceEnt, ent, &tr );
              useanim = qtrue;
              }   */
        }
        else
        {

        }

    }
#endif
    if (useanim)
        NS_PlayerAnimation( ent, TORSO_OPEN_DOOR, 750, qfalse );

}
/*
==============

TEAMPLAY CODE

==============
*/
/*
=================
NSQ3 Remove Items
author: Defcon-X
date: 10-06-2k
description: removes items in a map
=================
*/
char *remove_classnames[] =
    {
        "item_armor_shard",  // 0
        "item_armor_combat", // 1
        "item_armor_body",
        "item_health_small",
        "item_health",
        "item_health_large", // 5
        "item_health_mega",
        "weapon_gauntlet",
        "weapon_machinegun",
        "weapon_shotgun",
        "weapon_rocketlauncher", // 10
        "weapon_grenadelauncher",
        "weapon_ak47",
        "weapon_lightning",
        "weapon_railgun",
        "weapon_plasmagun",  // 15
        "weapon_bfg",
        "weapon_grapplinghook",
        "weapon_macmillan",
        "weapon_m4",
        "ammo_shells",
        "ammo_bullets",
        "ammo_grenades", //20
        "ammo_cells",
        "ammo_lightning",
        "ammo_rockets",
        "ammo_slugs",
        "ammo_bfg", //25
        "holdable_teleporter",
        "holdable_medkit",
        "item_quad",
        "item_enviro",
        "item_haste", //30
        "item_invis",
        "item_regen",
        "item_flight", // 35
        "c4_placed",
        "40mmgrenade",
        "reallead",
        "END"
    };

void NS_RemoveItems( void ) {
    int			i,j = 0;
    gentity_t	*ent;


    ent = &g_entities[0];
    for (i=0 ; i<level.num_entities ; i++, ent++) {
        if ( !ent->inuse ) {
            continue;
        }
        if ( ent->s.eType == ET_MISSILE )
        {
            G_FreeEntity( ent );
            continue;
        }
        if (!ent || !ent->classname)
            return;


        // dead bodys need special handling
        if (!Q_stricmp("bodyque", ent->classname))
        {
            if (ent->physicsObject == qtrue) // if it's a dead body , not something free in que
            {
                trap_UnlinkEntity( ent );
                ent->physicsObject = qfalse;
            }
            continue;
        }

        // scan
        while (!Q_stricmp(remove_classnames[j], "END") )
        {
            // if it's in the listeben ja schon um die 20 waffen
            if (!Q_stricmp(remove_classnames[j], ent->classname))
            {
                // remove it
                G_FreeEntity(ent);
            }
            j++;
        }

    }
    G_LogPrintf("Removed items...\n");
}

void assault_field_start( gentity_t *ent );
qboolean NS_GiveBombsToTeam( team_t team );


/*
=================
NSQ3 Launch Items
author: Defcon-X
date: 16-06-2k
description: relaunch entites, restart entities
=================
*/
void NS_LaunchEntities( void ) {

    // launch entities
    int			i;
    gentity_t	*ent;


    ent = &g_entities[0];
    for (i=0 ; i<level.num_entities ; i++, ent++) {

        // if it's in the list
        if (!Q_stricmp("trigger_counter", ent->classname))
            ent->use(ent,ent,ent); // fire up - start counting
        else if (
            !Q_stricmp(ent->classname , "func_explosive_glass" ) ||
            !Q_stricmp(ent->classname , "func_explosive_wood") ||
            !Q_stricmp(ent->classname , "func_explosive_metal") ||
            !Q_stricmp(ent->classname , "func_explosive_stone") ||
            !Q_stricmp(ent->classname , "func_explosive")
        )
        {
            ent->health = ent->count;
            ent->ns_flags = ent->timestamp;

            if ( !ent->targetname )
                ent->takedamage = qtrue;

            ent->r.svFlags &= ~SVF_NOCLIENT;
            ent->r.svFlags &= ~SVF_BROADCAST;
            ent->r.contents = CONTENTS_SOLID;

        }
        else if (!Q_stricmp("func_mission_object", ent->classname))
        {
            // just put back to original place
            VectorCopy (ent->pos2, ent->s.origin);
            ent->r.svFlags &= ~SVF_NOCLIENT;
        } else if (!Q_stricmp("team_briefcase", ent->classname))
        {
            Reset_Briefcase();
        } else if (!Q_stricmp("trigger_counter", ent->classname ) )
        {
            // stop thinking until we get triggered again
            ent->nextthink = 0;

        } else if (!Q_stricmp("func_wall", ent->classname ) )
        {
            ent->r.contents = CONTENTS_SOLID;
            ent->r.svFlags &= ~SVF_NOCLIENT;

            if (ent->spawnflags & 4)
            {
                ent->r.contents = CONTENTS_SOLID;
            }
            else
            {
                ent->r.contents = 0;
                ent->r.svFlags |= SVF_NOCLIENT;
            }
        } else if (!Q_stricmp("trigger_assaultfield", ent->classname ) )
        {
            assault_field_start( ent );
        } else if (!Q_stricmp("c4_placed", ent->classname ) )
        {
            G_FreeEntity( ent );
        } else if (!Q_stricmp("ball", ent->classname ) )
        {
            VectorCopy( ent->pos2 , ent->s.pos.trBase );
            VectorClear( ent->s.pos.trDelta );
        } else if (!Q_stricmp("trigger_fun_goal", ent->classname ) )
        {
            if ( !ent->inuse )
                trap_LinkEntity( ent );
        } else if ( (!Q_stricmp("func_door", ent->classname )) ||
                    !Q_stricmp("func_door_rotate", ent->classname ) )
        {
            Door_ResetState( ent );
        } else if ( (!Q_stricmp("trigger_toggle", ent->classname )) )
        {
            TriggerToggle_ResetState( ent );
        } else if (!Q_stricmp("func_train", ent->classname ) )
        {
            Think_SetupTrainTargets( ent );
        }
        else if (!Q_stricmp("target_particle", ent->classname ) )
        {
            if ( ent->spawnflags & 64 )
                ent->r.svFlags |= SVF_NOCLIENT;
        }
        else if (!Q_stricmp("trigger_multiple", ent->classname ) )
        {
            ent->touch = Touch_Multi;
            ent->nextthink = 0;
            trap_LinkEntity( ent );
        }
        else if (!Q_stricmp("target_delay", ent->classname) )
        {
            ent->nextthink = 0;
            ent->think = 0;
            ent->activator = 0;
        } else if (!Q_stricmp("func_elevator", ent->classname) ) {
            ElevatorReset( ent );
        }
    }
    G_LogPrintf("Relaunched entities...\n");
}




/*
=================
NSQ3 Remove dropped items
author: Defcon-X
date: 21-08-2000
description: remove all dropped enitites (including dropped flags, weapons, mission objectives)
=================
*/
void NS_RemoveDroppedEntities( void ) {

    // launch entities
    int			i;
    gentity_t	*ent;


    ent = &g_entities[0];
    for (i=0 ; i<level.num_entities ; i++, ent++) {
        if ( !ent->inuse ) {
            continue;
        }

        if (ent->flags & FL_DROPPED_ITEM)
            G_FreeEntity(ent);

    }
    G_LogPrintf("Relaunched entities...\n");
}

/*
=================
NSQ3 AliveTeamCount
author: Defcon-X
date: 19-06-2k
description: returns the amount of alive players on a team
=================
*/
int AliveTeamCount( int ignoreClientNum, team_t team ) {
    int		i;
    int		count = 0;

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if ( i == ignoreClientNum ) {
            continue;
        }
        if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
            continue;
        }
        if ( g_gametype.integer == GT_LTS && level.clients[i].pers.connected != CON_CONNECTED )
            continue;

        // not count players that haven't decided on their class yet...
        if ( level.clients[i].pers.nsPC.playerclass <= CLASS_NONE)
            continue;

        if ( level.clients[i].sess.sessionTeam == team && level.clients[i].sess.waiting == qfalse ) {
            count++;
        }

    }

    return count;
}

/*
=================
NSQ3 Is Vip Alive
author: Defcon-X
date: 19-06-2k
description: returns the amount of alive players on a team
=================
*/
int NS_IsVipAlive( int ignoreClientNum, team_t team ) {
    int		i;
    int		count = 0;

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if ( i == ignoreClientNum ) {
            continue;
        }
        if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
            continue;
        }
        if ( g_gametype.integer == GT_LTS && level.clients[i].pers.connected != CON_CONNECTED )
            continue;

        // not count players that haven't decided on their class yet...
        if ( level.clients[i].pers.nsPC.playerclass <= CLASS_NONE)
            continue;

        if ( level.clients[i].sess.sessionTeam == team && level.clients[i].sess.waiting == qfalse && level.clients[i].ns.is_vip ) {
            count++;
        }

    }

    return count;
}
/*
=================
NSQ3 Get Vip
author: Defcon-X
date: 19-06-2k
description: returns the clientnum of the current vip for team
=================
*/
int NS_GetVip( team_t team ) {
    int		i;

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
            continue;
        }
        if ( g_gametype.integer == GT_LTS && level.clients[i].pers.connected != CON_CONNECTED )
            continue;

        // not count players that haven't decided on their class yet...
        if ( level.clients[i].pers.nsPC.playerclass <= CLASS_NONE)
            continue;

        if ( level.clients[i].sess.sessionTeam == team && level.clients[i].sess.waiting == qfalse && level.clients[i].ns.is_vip == qtrue) {
            return i;
        }

    }

    return -1;
}


void NS_GiveXP ( gentity_t *ent , int num, qboolean take)
{

    if (!ent)
        return;

    /*
    if ( g_gametype.integer < GT_TEAM )
    return;
    */

    if (take)
        ent->client->pers.nsPC.xp -= num;
    else {
        if ( g_LockXP.integer ) {
            if ( num == g_baseXp.integer )
                ent->client->pers.nsPC.xp += num;
        }
        else
            ent->client->pers.nsPC.xp += num;

        ent->client->pers.nsPC.entire_xp += num;
    }

    if ( ent->client->pers.nsPC.xp < 0 )
        ent->client->pers.nsPC.xp = 0;

    // send xp
    trap_SendServerCommand( ent-g_entities, va("uxp %i", ent->client->pers.nsPC.xp ) );
}
/*
=================
NSQ3 Give Reward
author: Defcon-X
date: 19-06-2k
description: gives reward to ent
=================
*/
void NS_GiveReward( gentity_t *ent, int reward, int team )
{

    if ( !ent || !ent->client )
        return;

    if ( ent->client->sess.sessionTeam != team )
        return;

    ent->client->ns.rewards |= reward;
}


/*
=================
NSQ3 Give XP
author: Defcon-X
date: 19-06-2k
description: returns the amount of alive players on a team
=================
*/

#define ALIVE_XP_POINTS		2
#define DEAD_XP_POINTS		1

int NS_GiveBaseXP( void )
{
    int pointLimit	=	g_capturelimit.integer;
    int	timeLimit	=	g_timelimit.integer;

    if (
        ( pointLimit <= 15 && pointLimit >= 1 )
        ||
        (timeLimit <= 30 && timeLimit >= 1 )
    )
        return 2;

    if (
        ( pointLimit <= 24 && pointLimit >= 14 )
        ||
        (timeLimit <= 60 && timeLimit >= 31 )
    )
        return 1;

    return 0;
    /*
    bei roundlimit 24-14 oder timelimit 60-31	: jeder 1 XP mehr
    bei roundlimit 15-1 oder timelimit 30-1		: jeder 2 XP mehr
    */

}
void NS_GiveXPtoAlive( int ignoreClientNum, team_t team, qboolean winningTeam ) {
    int		i;
    //	int		count = 0;
    int		xp = 0;

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if ( i == ignoreClientNum ) {
            continue;
        }

        if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
            continue;
        }
        if ( level.clients[i].pers.connected != CON_CONNECTED )
            continue;

        // do not count players that haven't decided on their class yet...
        if ( level.clients[i].pers.nsPC.playerclass <= CLASS_NONE)
            continue;

        // check if it's the right team
        if ( level.clients[i].sess.sessionTeam != team )
            continue;

        // give XPs for killing people
        if ( level.clients[i].ns.num_killed > 0 )
        {
            // give xp
            if ( level.clients[i].ns.num_killed <= 1 )
                xp += 0;
            else
                xp += level.clients[i].ns.num_killed / 2;

            // remove our killed stats
            level.clients[i].ns.num_killed = 0;
        }

        // give XPs for killing people w/ knife
        if ( level.clients[i].knife_kills > 0 )
        {
            xp += level.clients[i].knife_kills;

            // remove our killed stats
            level.clients[i].knife_kills = 0;
        }

        if ( level.clients[i].ns.rewards != REWARD_NONE )
        {
            // vip
            if ( level.clients[i].ns.rewards & REWARD_VIP_ALIVE )
                xp += 3; // 4xp for beeing alive
            if ( level.clients[i].ns.rewards & REWARD_VIP_KILL )
                xp += 2; // 2xp for killing vip

            // bomb
            if ( level.clients[i].ns.rewards & REWARD_BOMB_EXPLODE )
                xp += 2;
            if ( level.clients[i].ns.rewards & REWARD_BOMB_DEFUSE )
                xp += 4;

            // briefcase
            if ( level.clients[i].ns.rewards & REWARD_BRIEFCASE_CAPTURE )
                xp += 2;
            if ( level.clients[i].ns.rewards & REWARD_BRIEFCASE_KILL )
                xp += 2;
            if ( level.clients[i].ns.rewards & REWARD_BRIEFCASE_2ND )
                xp += 2;
            if ( level.clients[i].ps.powerups[PW_BRIEFCASE] ) // even reward if you haven't caputred it
                xp += 2;

            // assault fields
            if ( level.clients[i].ns.rewards & REWARD_ASSAULT_TAKEN )
                xp += 2;
            if ( level.clients[i].ns.rewards & REWARD_2ASSAULT_TAKEN )
                xp += 2;
            if ( level.clients[i].ns.rewards & REWARD_3ASSAULT_TAKEN )
                xp += 2;
            if ( level.clients[i].ns.rewards & REWARD_4ASSAULT_TAKEN )
                xp += 2;

            // kills
            if ( level.clients[i].ns.rewards & REWARD_HS_KILL )
                xp += 1;
            if ( level.clients[i].ns.rewards & REWARD_2KILL_GRENADE )
                xp += 1;
            if ( level.clients[i].ns.rewards & REWARD_4KILL_GRENADE )
                xp += 3;


            level.clients[i].ns.rewards = REWARD_NONE;
        }

        if ( winningTeam )
            xp += 1;
        else  // not hanlding winner team.
        {
            if ( level.looseCount >= 8 ) // the team is really weak they get 5xtra XP
                xp += 5;
            else if ( level.looseCount >= 6 ) // already lost 6 rounds
                xp += 3;
            else if ( level.looseCount >= 4 ) // already lost 4 rounds
                xp += 2;
            else if ( level.looseCount >= 2 ) // already lost 2 rounds
                xp += 1;
        }

        // give 2 XP to any alive player from the winning team
        if ( level.clients[i].sess.waiting == qfalse) {
            xp += ALIVE_XP_POINTS;
        }
        if ( level.clients[i].sess.waiting == qtrue)
            xp += DEAD_XP_POINTS;

        xp += NS_GiveBaseXP( );

        G_LogPrintf( "gave %i xp to [%i] \"%s\" shots: %i hits: %i\n",
                     xp,
                     level.clients[i].ps.clientNum,
                     level.clients[i].pers.netname,
                     level.clients[i].accuracy_shots,
                     level.clients[i].accuracy_hits );

        NS_GiveXP( &g_entities[i] , xp, qfalse );
        level.clients[i].accuracy_hits = 0;
        level.clients[i].accuracy_shots = 0;
        xp = 0;
    }
}

int NS_DurchschnittXPforTeam( int ignoreClientNum, team_t team  ) {
    int		i;
    //	int		count = 0;
    int		xp = 0;
    int		validclients = 0;

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if ( i == ignoreClientNum ) {
            continue;
        }
        if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
            continue;
        }
        if ( level.clients[i].pers.connected != CON_CONNECTED )
            continue;

        // do not count players that haven't decided on their class yet...
        if ( level.clients[i].pers.nsPC.playerclass <= CLASS_NONE)
            continue;

        // do not count players that haven't joined for final yet
        if ( level.clients[i].pers.nsPC.entire_xp <= 0 )
            continue;

        // check if it's the right team
        if ( level.clients[i].sess.sessionTeam != team )
            continue;

        validclients++;
        xp += level.clients[i].pers.nsPC.entire_xp;
    }

    if ( xp <= 0 || validclients <= 0 )
        return g_baseXp.integer;

    xp = xp / validclients;

    return xp;
}

#define rndnum(y,z) ((random()*((z)-((y)+1)))+(y))

qboolean randomPlayers[MAX_CLIENTS];

gentity_t *NS_RandomPlayer( int ignoreClientNum, team_t team ) {
    int		i;
    int currvip = -1;

    // look for players which have a clientnumber bigger than the ignoreclientnum
    for ( i = ignoreClientNum+1 ; i < level.maxclients ; i++ ) {

        if ( level.clients[i].pers.connected != CON_CONNECTED ) continue;
        if ( level.clients[i].pers.nsPC.playerclass <= CLASS_NONE ) continue;
        if ( level.clients[i].sess.sessionTeam != team ) continue;
        if ( level.clients[i].sess.waiting ) continue;
        if ( level.clients[i].pers.lockedPlayer > 0 ) continue;

        currvip = i;

        break;

    }

    // look for players which have a clientnumber smaller than the ignoreclientnum
    if ( currvip < 0 ) for ( i = 0 ; i <= ignoreClientNum ; i++ ) {
            if ( level.clients[i].pers.connected != CON_CONNECTED ) continue;
            if ( level.clients[i].pers.nsPC.playerclass <= CLASS_NONE ) continue;
            if ( level.clients[i].sess.sessionTeam != team ) continue;
            if ( level.clients[i].sess.waiting ) continue;
            if ( level.clients[i].pers.lockedPlayer > 0 ) continue;

            currvip = i;

            break;
        }

    // return our new VIP if we have it
    if ( currvip >= 0 ) return &g_entities[ level.clients[currvip].ps.clientNum ];

    // if we have not found a new VIP, return the old one
    if (ignoreClientNum >= 0 && ignoreClientNum < level.maxclients)
        return &g_entities[ level.clients[ignoreClientNum].ps.clientNum ];
    else
        return NULL;
}

/*
=================
NSQ3 Won Round
author: KnighthawK / defcon-X
date: 30-05-2k
description: A Round has been won by a Team - add a point, and print a msg
=================
*/
void NS_WonRound ( team_t team )
{
    LTS_Rounds++;

    NS_SetGameState( STATE_OVER );

    if (team != TEAM_RED && team != TEAM_BLUE)
    {
        gentity_t	*te;

        te = G_TempEntity(vec3_origin, EV_GLOBAL_TEAM_SOUND );
        te->r.svFlags |= SVF_BROADCAST;
        te->s.eventParm = GTS_DRAW_ROUND;

        trap_SendServerCommand( -1, "cp \"The Round was tied!\n\"" );

        G_LogPrintf( "ROUND: Tied.\n" );
        return;
    }
    AddTeamScore( vec3_origin, team, 1 );

    trap_SendServerCommand( -1, va("cp \"The %s won the round.\n\"", TeamName( team ) ) );
    G_LogPrintf( "ROUND: %s won.\n", TeamName( team ) );

    // otherteam is still the looser
    if ( level.lastLooser == OtherTeam( team ) )
        level.looseCount++;
    else if ( level.lastLooser != OtherTeam( team ) )
    {
        level.looseCount = 1;
        level.lastLooser = OtherTeam( team );
    }

    CalculateRanks();
}

void assault_field_stopall( );

void NS_EndTimer ( void )
{
    // send the config string to the clients
    trap_SetConfigstring( CS_ROUND_START_TIME, "0" );
    trap_SetConfigstring( CS_VIP_START_TIME, "0" );
    trap_SetConfigstring( CS_BOMB_START_TIME, "0" );
    trap_SetConfigstring( CS_ASSAULT_START_TIME, "0" );
    trap_SetConfigstring( CS_ASSAULT2_START_TIME, "0" );
    trap_SetConfigstring( CS_ASSAULT3_START_TIME, "0" );
    trap_SetConfigstring( CS_ASSAULT4_START_TIME, "0" );


}
/*
=================
NSQ3 End Round
author: Defcon-X
date: 20-06-2k
description: End a Round, see which team won.
=================
*/
void NS_EndRound ( void )
{
    if (level.warmupTime != -1)
        return;

    // the round has just been won
    if ( AliveTeamCount( -1, TEAM_RED ) + AliveTeamCount( -1, TEAM_BLUE ) < 1)
        NS_WonRound(TEAM_FREE);
    else if (AliveTeamCount( -1, TEAM_RED ) > 0)
        NS_WonRound(TEAM_RED);
    else if (AliveTeamCount( -1, TEAM_BLUE ) > 0)
        NS_WonRound(TEAM_BLUE);
    else
        NS_WonRound(TEAM_FREE); // draw again

    level.done_objectives[TEAM_RED] = level.done_objectives[TEAM_BLUE] = 0;

    // buffer time until the next round starts
    level.warmupTime = -3;
    level.winTime = level.time + 10 * ONE_SECOND; // 10 seconds till next match
    level.xpTime = level.time + 5 * ONE_SECOND;
    level.roundstartTime = 0;


    assault_field_stopall( );


}

/*
=================
NSQ3 End Round For Team
author: Defcon-X
date: 20-06-2k
description: End a Round, see which team won.
=================
*/
void NS_EndRoundForTeam ( int team )
{
    if (level.warmupTime != -1)
        return;

    level.done_objectives[TEAM_RED] = level.done_objectives[TEAM_BLUE] = 0;


    // buffer time until the next round starts
    level.warmupTime = -3;
    level.winTime = level.time + ( 10 * ONE_SECOND ); // 10 seconds till next match
    level.xpTime = level.time + 5 * ONE_SECOND;
    level.roundstartTime = 0;

    assault_field_stopall( );

    // the round has just been won by a team
    NS_WonRound( team );  // give points & stuff

    // send the config string to the clients
    trap_SetConfigstring( CS_ROUND_START_TIME, "0" );
    trap_SetConfigstring( CS_VIP_START_TIME, "0" );
    trap_SetConfigstring( CS_BOMB_START_TIME, "0" );
    trap_SetConfigstring( CS_ASSAULT_START_TIME, "0" );
    trap_SetConfigstring( CS_ASSAULT2_START_TIME, "0" );
    trap_SetConfigstring( CS_ASSAULT3_START_TIME, "0" );
    trap_SetConfigstring( CS_ASSAULT4_START_TIME, "0" );

    NS_EndTimer();
}

/*
================
SelectRandomVipSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define	MAX_VIP_SPAWN_POINTS	4
gentity_t *SelectRandomVipSpawnPoint( void ) {
    gentity_t	*spot;
    int			count;
    int			selection;
    gentity_t	*spots[MAX_VIP_SPAWN_POINTS];

    count = 0;
    spot = NULL;

    while ((spot = G_Find (spot, FOFS(classname), "info_vip_spawn")) != NULL) {
        if ( SpotWouldTelefrag( spot ) ) {
            continue;
        }
        spots[ count ] = spot;
        count++;
    }

    if ( !count ) {	// no spots that won't telefrag
        return G_Find( NULL, FOFS(classname), "info_vip_spawn");
    }

    selection = rand() % count;
    return spots[ selection ];
}

/*
=================
NSQ3 Do Vip
author: Defcon-X
date: 07-10-2k
description: do necessary stuff for V.I.P. game play
=================
*/
void NS_DoVipStuff(int team, gentity_t *player)
{
    gentity_t	*vip;
    gitem_t		*sec;
    gentity_t	*spot;
    int			_wp = WP_SW629;
    int			_close = WP_SEALKNIFE;

    if ( player == NULL ) {
        // we need to do this for both seals and tanogs.
        if ( TeamCount( -1, team ) <= 1 )
            vip = NS_RandomPlayer( -1, team );
        else
            vip = NS_RandomPlayer( lastvip[team], team );
    }
    else
        vip = player;


    if ( !vip )
    {
        // blutengel_xxx:
        // seems to be this positionwhere the the server crashes!
        //
        // g_error("trying to spawn v.i.p.[%s] - without vip entity!", teamname(team) );
        // make the round win for the opposing team

        if (team == TEAM_RED) NS_EndRoundForTeam(TEAM_BLUE);
        else if (team == TEAM_BLUE) NS_EndRoundForTeam(TEAM_RED);
        else NS_EndRound();

        return;
    }

    vip->client->ns.is_vip = qtrue;
    lastvip[team] = vip->s.clientNum;

    // remove ammo & powerups
    memset( vip->client->ps.powerups, 0, sizeof(vip->client->ps.powerups) );
    memset( vip->client->ps.ammo, 0, sizeof(vip->client->ps.ammo) );

    vip->client->ps.stats[STAT_WEAPONS] = 0;
    vip->client->ps.stats[STAT_WEAPONS2] = 0;

    vip->health = 100;

    // set new powerups
    vip->client->ps.powerups[PW_VEST] = ARMOR_HEAVY;
    vip->client->ps.powerups[PW_HELMET] = ARMOR_NONE;

    // set new weapons
    if ( vip->client->sess.sessionTeam == TEAM_BLUE ) {
        _wp = WP_DEAGLE;
        _close = WP_KHURKURI;
    }

    BG_PackWeapon( _wp, vip->client->ps.stats );

    sec = BG_FindItemForWeapon( _wp );

    vip->client->ps.ammo[sec->giAmmoTag] = 6;
    vip->client->ps.weapon = _wp;
    vip->client->ps.eFlags |= EF_VIP;

    vip->s.weapon = _wp;

    spot = SelectRandomVipSpawnPoint();

    if (!spot)
    {
        G_Error("Trying to spawn V.I.P.[%s] - without spawnpoint!", TeamName(team) );
        return;
    }

    // give the briefcase to the vip
    if ( spot->ns_flags )
    {
        Pickup_Briefcase( spot, vip );
        vip->client->ns.is_vipWithBriefcase = qtrue;
    }

    vip->client->ns.rewards |= REWARD_VIP_ALIVE;

    G_SetOrigin( vip, spot->s.origin );
    VectorCopy (spot->s.origin, vip->client->ps.origin);

    SetClientViewAngle( vip, spot->s.angles );

    trap_SendServerCommand( vip->client->ps.clientNum, va("cp \""S_COLOR_GREEN"You are the V.I.P.\n\""));
    G_UseTargets( spot, vip );

    ClientUserinfoChanged( vip->s.clientNum );
    G_LogPrintf( "OBJECTIVE: [%i] \"%s\" spawned as VIP\n", vip->client->ps.clientNum, vip->client->pers.netname );
}

qboolean OriginWouldTelefrag( vec3_t origin, int ignoreClientnum );

/*
=================
NSQ3 Run TeamPlay
author: KnighthawK + defcon-X
date: 30-05-2k
description: runs the teamplay code
=================
*/
#define ROUND_WARMUP_TIME	6 * ONE_SECOND;

void CheckTeamplay( void ) {
    int i;
    int	minplayers = g_minPlayers.integer;
    gentity_t *bmbplayer;

    if ( minplayers < 1 )
        minplayers = 0;

    // not in LTS mode? what are we doing here?
    if (g_gametype.integer != GT_LTS )
        return; // argh

    if (g_doWarmup.integer != 0)
        trap_SendConsoleCommand( EXEC_INSERT, "g_doWarmup 0" );

    // never do round handling if intermission started
    if ( level.intermissiontime ) {
        return;
    }

    // what? no teams are full?
    if ( (TeamCount( -1, TEAM_RED ) < minplayers) || ( TeamCount( -1, TEAM_BLUE ) < minplayers)  )
    {
        // still waitin for them clients..
        NS_SetGameState( STATE_OPEN );

        if (level.time > i_sNextWaitPrint )
        {	// not enough players
            G_Printf("Waiting for players to join.\n");
            i_sNextWaitPrint = level.time + 5000;
        }

        for ( i = 0; i < level.maxclients; i++ )
        {
            gclient_t *client = &level.clients[i];

            if (client->pers.connected != CON_CONNECTED)
                continue;

            // don't drop in chasing players
            if ( client->ps.pm_flags & PMF_FOLLOW )
                continue;

            // if it's a client that is in game
            if ( client->sess.waiting == qtrue )
            {
                //		allow him to walk around
                if ( ( TeamCount( -1, TEAM_RED ) > 0 ) || ( TeamCount( -1, TEAM_BLUE ) > 0 ) )
                {
                    client->sess.waiting = qfalse;

                    ClientSpawn( &g_entities[ client - level.clients ] );

                    client->ps.eFlags &= ~EF_WEAPONS_LOCKED;
                }
            }
        }
        b_sWaitingForPlayers = qtrue;

        // reset loosecount
        level.looseCount = 0; // how often the team lost
        level.lastLooser = TEAM_FREE;	// which team lost

        level.warmupTime = -2;
        level.roundstartTime = 0;

        trap_SetConfigstring( CS_ROUND_START_TIME, "0" );
        trap_SetConfigstring( CS_VIP_START_TIME, "0" );
        trap_SetConfigstring( CS_BOMB_START_TIME, "0" );
        trap_SetConfigstring( CS_ASSAULT_START_TIME, "0" );
        trap_SetConfigstring( CS_ASSAULT2_START_TIME, "0" );
        trap_SetConfigstring( CS_ASSAULT3_START_TIME, "0" );
        trap_SetConfigstring( CS_ASSAULT4_START_TIME, "0" );
    }
    else
    {
        // give XPs
        if ( level.xpTime < level.time && level.xpTime > 0)
        {
            // now give our XPs
            NS_GiveXPtoAlive( -1, level.lastLooser, qfalse );
            NS_GiveXPtoAlive( -1, OtherTeam(level.lastLooser), qtrue );
            level.xpTime = 0;
        }

        // time to start a new warmup, initiate warmup - reset some variables
        if (level.winTime == level.time ) {
            level.warmupTime = -2;
            level.done_objectives[TEAM_RED] = level.done_objectives[TEAM_BLUE] = 0;
            NS_SetGameState( STATE_OPEN );
        }


        //
        // start the warmup
        //
        // clients are now beeing spawned
        // items are beeing respawned
        // missinoobjects / func are getting restartet.
        if (level.warmupTime == -2)
        {
            NS_SetGameState( STATE_OPEN );
            if ( LTS_Rounds == 1 )
                level.warmupTime = level.time + g_firstCountdown.integer * ONE_SECOND;
            else
                level.warmupTime = level.time + ROUND_WARMUP_TIME;

            trap_SendServerCommand( -1, va( "cp \"Round #%i will start in %i seconds.\nWeapons Locked.\"", LTS_Rounds, (level.warmupTime - level.time)/ONE_SECOND  ) );
            trap_SendServerCommand( -1, "roundst" );

            NS_EndTimer();

            NS_SendResetAllStatusToAllPlayers();

            for ( i = 0; i < level.maxclients; i++ )
            {
                gclient_t *client = &level.clients[i];

                if (client->pers.connected != CON_CONNECTED)
                    continue;

                if (client->pers.nsPC.playerclass <= CLASS_NONE)
                    continue;

                if ( client->sess.sessionTeam == TEAM_SPECTATOR )
                    continue;

                client->ps.eFlags &= ~EF_VIP;
                client->ns.is_vip = client->ns.is_vipWithBriefcase = qfalse;

                if ( client->pers.lockedPlayer > 0 )
                {
                    client->pers.lockedPlayer--;
                    client->sess.waiting = qtrue;

                    NS_SendPlayersStatusToAllPlayers( i, MS_DEAD );

                    if ( client->pers.lockedPlayer > 0 )
                        PrintMsg( &g_entities[ i ], "You are locked for %i more round%c. You may not play this round.\n", client->pers.lockedPlayer,(client->pers.lockedPlayer > 1 )?'s':' ' );
                    else
                        PrintMsg( &g_entities[ i ], "You are locked. You may not play this round.\n" );
                    continue;
                }
                client->sess.waiting = qfalse;

                if ( client->ps.pm_flags & PMF_FOLLOW )
                    StopFollowing( &g_entities[ client - level.clients ] );

                // refresh userinfo
                ClientSpawn( &g_entities[ client - level.clients ] );

                // for all clients in game... lock weapons
                client->ps.eFlags |= EF_WEAPONS_LOCKED;

                // so the vip model will get removed again and headstuff will get updated.
                ClientUserinfoChanged( client->ps.clientNum );
            }
            // we also spawned the clients now clean all items
            NS_RemoveItems(); // can't handle mission objectives like briefcase in here
            NS_RemoveDroppedEntities(); // cos we only remove the dropped versions

            // start the entites (spawn briefcase/resart func_explosives...)
            NS_LaunchEntities();

            if ( !g_overrideGoals.integer ) {
                if (level.vip[TEAM_RED] > 0)
                    NS_DoVipStuff(TEAM_RED, NULL);
                if (level.vip[TEAM_BLUE] > 0)
                    NS_DoVipStuff(TEAM_BLUE,NULL);
            }
        }

        // our round begins
        if (level.warmupTime == level.time)
        {
            // Time to start the next round
            level.warmupTime = -1;
            //NS_SetGameState( STATE_LOCKED );

            if ( ( /* level.vip[TEAM_RED] == VIP_ESCAPE || */ level.vip[TEAM_BLUE] == VIP_STAYALIVE ) && !g_overrideGoals.integer)
                level.roundstartTime = level.time + level.vipTime;
            else if ( g_roundTime.integer )
                level.roundstartTime = level.time + ( g_roundTime.integer * 60 * ONE_SECOND ); // round now started
            else
                level.roundstartTime = -1;

            trap_SetConfigstring( CS_ROUND_START_TIME, va("%i",level.roundstartTime) );

            // start timelimit
            if ( LTS_Rounds == 1 )
            {
                G_LogPrintf("Started Timelimit.\n");
                level.startTime = level.time;
                trap_SetConfigstring( CS_LEVEL_START_TIME, va("%i", level.startTime ) );

                // spawn all clients that are still waiting to get spawned
                for ( i = 0; i < level.maxclients; i++ )
                {
                    gclient_t *client = &level.clients[i];

                    if (client->pers.connected != CON_CONNECTED)
                        continue;
                    if (client->pers.nsPC.playerclass <= CLASS_NONE)
                        continue;
                    if ( client->sess.sessionTeam == TEAM_SPECTATOR )
                        continue;
                    if ( !client->sess.waiting )
                        continue;

                    client->sess.waiting = qfalse;

                    if ( client->ps.pm_flags & PMF_FOLLOW )
                        StopFollowing( &g_entities[ client - level.clients ] );

                    client->unstuck = 1;

                    // spawn
                    ClientSpawn( &g_entities[ client - level.clients ] );

                    // fix players state if it's a latetime joiner
                    NS_SendPlayersStatusToAllPlayers( i, MS_HEALTH5 );
                }
            }


            for ( i = 0; i < level.maxclients; i++ )
            {
                gclient_t *client = &level.clients[i];

                if (client->pers.connected != CON_CONNECTED)
                    continue;
                // if we're a waiting player don't ready our weapons
                if (client->sess.waiting != qfalse)
                    continue;
                if ( client->sess.sessionTeam == TEAM_SPECTATOR )
                    continue;

                // do we need to unstuck?
                if ( OriginWouldTelefrag( client->ps.origin, client->ps.clientNum ) != -1 )
                    client->unstuck = 1;
                else
                    client->unstuck = 0;

                // unlock weapons
                client->ps.eFlags &= ~EF_WEAPONS_LOCKED;
            }

            // give bombs to players that require it.
            if ( ! NS_GiveBombsToTeam( TEAM_RED ) ) {
                NS_EndRoundForTeam(TEAM_BLUE);
                return;
            }
            if ( ! NS_GiveBombsToTeam( TEAM_BLUE ) ) {
                NS_EndRoundForTeam(TEAM_RED);
                return;
            }

            // radiobroadcast the start radiomsg.
            bmbplayer = NS_RandomPlayer(-1, TEAM_RED);
            if ( random() < 0.5 )
                if (bmbplayer != NULL) NS_BotRadioMsg( bmbplayer , "gogo" );
                else
                    if (bmbplayer != NULL) NS_BotRadioMsg( bmbplayer , "locknload" );

            bmbplayer = NS_RandomPlayer(-1, TEAM_BLUE);
            if ( random() < 0.5 )
                if (bmbplayer != NULL) NS_BotRadioMsg( bmbplayer , "gogo" );
                else
                    if (bmbplayer != NULL) NS_BotRadioMsg( bmbplayer , "locknload" );

            // remove dropped weapons to prevent weapon stocking
            NS_RemoveDroppedEntities();
            // lock the game so no more people can join the round
            NS_SetGameState( STATE_LOCKED);
            // tell the player that they're ready to fire
            CenterPrintAll( "Go, Weapons Unlocked.\n" );
            // log print a start
            G_LogPrintf( "ROUND: Start.\n" );
        }

        if ( level.warmupTime != -3 )
        {

            // see if the round has to be ended this round.
            // it should be ended when all objectives have been accomplished.
            if ( !g_overrideGoals.integer )
            {
                if ( ( level.done_objectives[TEAM_RED] >= level.num_objectives[TEAM_RED] ) && level.num_objectives[TEAM_RED] > 0 )
                {
                    NS_EndRoundForTeam( TEAM_RED );
                    return;
                }
                else if ( ( level.done_objectives[TEAM_BLUE] >= level.num_objectives[TEAM_BLUE] ) && level.num_objectives[TEAM_BLUE] > 0 )
                { // don't go any further
                    NS_EndRoundForTeam( TEAM_BLUE );
                    return;
                }
            }

            // check if there is a team with no alive players
            if ( AliveTeamCount( -1, TEAM_RED ) + AliveTeamCount( -1, TEAM_BLUE ) <= 0 )
            {

                if ( NS_BombExistForTeam( TEAM_RED ) )
                    NS_EndRoundForTeam( TEAM_RED );
                else if ( NS_BombExistForTeam( TEAM_BLUE ) )
                    NS_EndRoundForTeam( TEAM_BLUE );
                else
                    NS_EndRound();

                return;
            }
            else if (AliveTeamCount( -1, TEAM_RED ) < 1 && !NS_BombExistForTeam(TEAM_RED) )
            {
                NS_EndRound();  // the round is finished , since some team got no members
                return;
            }
            else if ( AliveTeamCount( -1, TEAM_BLUE ) < 1 && !NS_BombExistForTeam(TEAM_BLUE) )
            {
                NS_EndRound(); // the round is finished , since some team got no members
                return;
            }

        }
    }

    // update warmup config string?
    {
        char buffer[512];
        int oldWarmup;

        trap_GetConfigstring( CS_WARMUP, buffer, sizeof(buffer) );
        oldWarmup = atoi(buffer);

        if (level.warmupTime > 0 )
            trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
        else if ( level.warmupTime == -2 )
            trap_SetConfigstring( CS_WARMUP, "-1" );
        else if ( oldWarmup  != 0 )
            trap_SetConfigstring( CS_WARMUP, "0" );
    }
}

/*
==============

TEAMPLAY SPAWNING

==============
*/

/*
=================
NSQ3 Select Random Spawn Point
author: Defcon-X
date:
description: selects a random team spawnpoint
=================
*/
#define	MAX_TEAM_SPAWN_POINTS	16

gentity_t *SelectRandomSpawnPoint( int teamstate, team_t team ) {
    gentity_t	*spot;
    int			count;
    int			selection;
    gentity_t	*spots[MAX_TEAM_SPAWN_POINTS];
    char		*classname;


    if (team == TEAM_RED)
        classname = "team_seal_player";
    else if (team == TEAM_BLUE)
        classname = "team_tango_player";
    else
        return NULL;

    count = 0;

    spot = NULL;

    while ( (spot = G_Find (spot, FOFS(classname), classname)) != NULL) {
        if ( SpotWouldTelefrag( spot ) ) {
            continue;
        }
        spots[ count ] = spot;
        if (++count == MAX_TEAM_SPAWN_POINTS)
            break;
    }

    if ( !count ) {	// no spots that won't telefrag
        spot = G_Find( NULL, FOFS(classname), classname);

        if ( !spot )
        {
            classname = "info_player_deathmatch";
            spot = G_Find( NULL, FOFS(classname), classname);
        }

        G_LogPrintf("Warning: All spawnspots for team %s full.\n", TeamName( team ) );

        return spot;
    }

    selection = rand() % count;
    return spots[ selection ];
}

/*
================
OriginWouldTelefrag

================
*/
static vec3_t	plMins = {-10, -10, MINS_Z};
static vec3_t	plMaxs = {10, 10, MAXS_Z};

qboolean OriginWouldTelefrag( vec3_t origin, int ignoreClientnum ) {
    int			i, num;
    int			touch[MAX_GENTITIES];
    gentity_t	*hit;
    vec3_t		mins, maxs;

    VectorAdd(  origin, plMins, mins );
    VectorAdd(  origin, plMaxs, maxs );
    num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

    for (i=0 ; i<num ; i++) {
        hit = &g_entities[touch[i]];
        //if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
        if ( ignoreClientnum != -1 && hit->client && hit->client->ps.clientNum == ignoreClientnum )
            continue;

        if ( hit->client) {
            return hit->client->ps.clientNum;
        }

    }

    return -1;
}

/*
=================
NSQ3 Select Team Spawn Point
author: Defcon-X
date:
description: selects a team Spawn point
=================
*/
gentity_t *SelectTeamSpawnPoint ( team_t team, int teamstate, vec3_t origin, vec3_t angles ) {
    gentity_t	*spot;
    //	trace_t		trace;
    //	int			test = 0;

    spot = SelectRandomSpawnPoint ( teamstate, team );

    if (!spot) {
        spot = SelectSpawnPoint( vec3_origin, origin, angles );
    }

    VectorCopy (spot->s.origin, origin);
    origin[2] += 9;
    VectorCopy (spot->s.angles, angles);
    /*

    if ( random() < 0.5 )
    origin[0] += 11;
    else
    origin[0] -= 11;

    if ( random() < 0.5 )
    origin[1] += 11;
    else
    origin[1] -= 11;

    trap_Trace( &trace, spot->s.origin, playerMins,playerMaxs, origin , -1, MASK_SOLID );

    while ( trace.fraction < 1.0f && test < 10 )
    {
    if ( random() < 0.5 )
    origin[0] += 11;
    else
    origin[0] -= 11;

    if ( random() < 0.5 )
    origin[1] += 11;
    else
    origin[1] -= 11;

    trap_Trace( &trace, spot->s.origin, playerMins,playerMaxs, origin , -1, MASK_SOLID );
    test++;
    }*/

    return spot;
}

/*
=================
NSQ3 Is Spectator
author: Defcon-X
date:
description: is the entity a spectator?
=================
*/
qboolean Is_Spectator ( gentity_t *ent)
{
    if (!ent)
        return qfalse;
    if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
        return qtrue;
    if ( ent->client->sess.waiting )
        return qtrue;

    if (ent->client->ps.pm_type == PM_SPECTATOR)
        return qtrue;
    return qfalse;
}

/*
==============

BANDAGING

==============
*/

/*
=================
NSQ3 Bandage Body Part
author: Defcon-X
date: 13-07-2000
description: adds health back to the bodypart and returns amount of healed points
=================
*/
int NS_BandageBodyPart ( gentity_t *ent , int loc)
{
    int		points = 0;

    points = 3;

    // heal body parts
    switch( loc )
    {
    case STAT_HEAD_DAMAGE:
        //			points /= 3;
        ent->client->ps.stats[STAT_HEAD_DAMAGE] -= points;
        break;
    case STAT_CHEST_DAMAGE:
        ent->client->ps.stats[STAT_CHEST_DAMAGE] -= points;
        break;
    case STAT_STOMACH_DAMAGE:
        ent->client->ps.stats[STAT_STOMACH_DAMAGE] -= points;
        break;
    case STAT_ARM_DAMAGE:
        ent->client->ps.stats[STAT_ARM_DAMAGE] -= points;
        break;
    case STAT_LEG_DAMAGE:
        ent->client->ps.stats[STAT_LEG_DAMAGE] -= points;
        break;
    default:
        break;
    }

    if (ent->client->ps.stats[STAT_ARM_DAMAGE] < 0 )
        ent->client->ps.stats[STAT_ARM_DAMAGE] = 0;
    if (ent->client->ps.stats[STAT_LEG_DAMAGE] < 0 )
        ent->client->ps.stats[STAT_LEG_DAMAGE] = 0;
    if (ent->client->ps.stats[STAT_CHEST_DAMAGE] < 0 )
        ent->client->ps.stats[STAT_CHEST_DAMAGE] = 0;
    if (ent->client->ps.stats[STAT_STOMACH_DAMAGE] < 0 )
        ent->client->ps.stats[STAT_STOMACH_DAMAGE] = 0;
    if (ent->client->ps.stats[STAT_HEAD_DAMAGE] < 0)
        ent->client->ps.stats[STAT_HEAD_DAMAGE] = 0;

    // regenerate a bit health
    ent->client->ps.stats[STAT_HEALTH] += 1;

    return points;
}

/*
=================
NSQ3 Got Wounds
author: Defcon-X
date:
description: return true if player got any wounds
=================
*/
int NS_GotWounds( gentity_t *ent )
{
    if (ent->client->ps.stats[STAT_ARM_DAMAGE] ||
            ent->client->ps.stats[STAT_LEG_DAMAGE] ||
            ent->client->ps.stats[STAT_CHEST_DAMAGE] ||
            ent->client->ps.stats[STAT_STOMACH_DAMAGE] ||
            ent->client->ps.stats[STAT_HEAD_DAMAGE] )
        return qtrue;

    if ( ent->client->bleed_num > 0 )
        return qtrue;

    return qfalse;
}

/*
=================
NSQ3 Bandage
author: Defcon-X
date:
description: Bandage an entity
=================
*/
void NS_Bandaging ( gentity_t *ent )
{
    // time for wound to bandage?
    int wounds = ent->client->bleed_num;
    int	time = 0;
    int loc = 0;

    if (ent->client->buttons & BUTTON_ATTACK)
    {
        ent->client->ps.weaponstate = WEAPON_BANDAGING_END;
        return;
    }

    if (wounds <= 0) {
        // no open bleeding wounds, now try bandaging our wounds
        if ( NS_GotWounds(ent) )
        {

            if (ent->client->ps.stats[STAT_ARM_DAMAGE])
                loc = STAT_ARM_DAMAGE;
            else if (ent->client->ps.stats[STAT_LEG_DAMAGE])
                loc = STAT_LEG_DAMAGE;
            else if (ent->client->ps.stats[STAT_CHEST_DAMAGE])
                loc = STAT_CHEST_DAMAGE;
            else if (ent->client->ps.stats[STAT_STOMACH_DAMAGE])
                loc = STAT_STOMACH_DAMAGE;
            else if (ent->client->ps.stats[STAT_HEAD_DAMAGE] )
                loc = STAT_HEAD_DAMAGE;
            else
                loc = 0;

            if (loc)
                time = NS_BandageBodyPart(ent, loc) / 4;
        }
        else {
            ent->client->ps.weaponstate = WEAPON_BANDAGING_END;
            return;
        }
    }

    if (!loc) {
        wounds--;
        time = ent->client->bleed_point[wounds];
    }

    // multiple so we get a time ( needs more testings )
    time *= ONE_SECOND/2;

    // add min time...
    time += BANDAGE_MINTIME - ent->client->pers.nsPC.technical * 25;

    if (!loc)
        ent->client->bleed_num--; // now remove one wound...

    ent->client->ps.weaponTime = time;
}



/*
=================
NSQ3 Start Bandage
author: Defcon-X
date:
description: client starts to bandage
=================
*/
void NS_StartBandage ( gentity_t *ent )
{
    int	_weapon = ent->client->ps.weapon;
    float	weaponTime	= GetWeaponTimeOnLower(ent) * BG_GetSpeedMod(ent->client->pers.nsPC.technical) ;

    // got any wounds?
    if ( !NS_GotWounds(ent) )
        return;
    if ( ent->client->ps.pm_flags & PMF_FOLLOW )
        return;
    // if not idling / endmag return
    if (ent->client->ps.weaponstate != WEAPON_READY && ent->client->ps.weaponstate != WEAPON_LASTRND)
        return;

    // when reloading
    if ( ent->client->ns.weaponmode[_weapon] & ( 1 << WM_ZOOM4X) || ent->client->ns.weaponmode[_weapon] & (1 << WM_ZOOM2X))
    {
        // go out of zoom
        ent->client->ns.weaponmode[_weapon] &= ~( 1 << WM_ZOOM4X );
        ent->client->ns.weaponmode[_weapon] &= ~( 1 << WM_ZOOM2X );
    }


    // send into bandage
    ent->client->ps.weaponTime = (int)weaponTime;
    ent->client->ps.weaponstate = WEAPON_BANDAGING_START;
    ent->client->ps.weaponTime = (int)weaponTime;
}

qboolean BG_IsPistol( int weapon );
qboolean BG_IsMelee( int weapon );

/*
=================
NSQ3 Make Touchable
author: Defcon-X
date:
description: makes a weapon touchable
=================
*/
void Weapon_MakeTouchable(gentity_t *self)
{
    int weapon;

    // now it's touchable
    self->touch = Touch_Item;
    //	self->s.generic1 = 0;

    weapon = self->item->giTag;

    // if a pistol or a knife , remove it after 5 seconds
    if ( BG_IsPistol ( weapon ) || BG_IsMelee ( weapon ) && g_gametype.integer < GT_TEAM)
    {
        self->think = G_FreeEntity;
        self->nextthink = level.time + 10000;
    }
    return;
}

/*
================
LaunchWeapon

spawns a weapon and tosses it forward . adding clips
================
*/
gentity_t *LaunchWeapon( gitem_t *item, vec3_t origin, vec3_t velocity, int clips, int weaponmode, qboolean silenced ) {
    gentity_t	*dropped;
    int			powerups[MAX_POWERUPS];
    int		i;

    dropped = G_Spawn();

    dropped->s.eType = ET_ITEM;
    dropped->s.modelindex = item - bg_itemlist;	// store item number in modelindex
    dropped->s.modelindex2 = 1;			// This is non-zero is it's a dropped item

    dropped->classname = item->classname;
    dropped->item = item;
    VectorSet (dropped->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -3);
    VectorSet (dropped->r.maxs, ITEM_RADIUS, ITEM_RADIUS, 3);
    dropped->r.contents = CONTENTS_TRIGGER;


    G_SetOrigin( dropped, origin );

    dropped->s.groundEntityNum = -1;
    dropped->s.pos.trType = TR_GRAVITY;
    dropped->s.pos.trTime = level.time;
    VectorCopy( velocity, dropped->s.pos.trDelta );

    dropped->s.eFlags |= EF_BOUNCE_HALF;

    dropped->flags = FL_DROPPED_ITEM;

    dropped->touch = Touch_Item;

    memset( &powerups, 0, sizeof(powerups) );

    //
    // make weapon addons visible for all clients
    //
    NS_AdjustClientVWeap(weaponmode, powerups );

    for ( i=0; i<MAX_POWERUPS; i++ )
    {
        if ( powerups[i] )
            dropped->s.powerups |= ( 1 << i );
    }
    // just one of the many hacks here.
    if ( silenced )
        dropped->s.eFlags |= EF_SILENCED;

    //
    // serverside usage; tell the items, and modes the item uses
    //
    dropped->s.generic1 = weaponmode;

    dropped->s.apos.trBase[YAW] = rand() % 360;
    dropped->s.apos.trBase[ROLL] = 90;
    // clips
    dropped->count = clips;

    if ( g_gametype.integer == GT_TEAM )
    {
        dropped->think = G_FreeEntity;
        dropped->nextthink = level.time + 10000; // remove after 10 seconds
    }
    trap_LinkEntity (dropped);

    return dropped;
}
/*
================
Drop_Weapon

Drops weapons, and adds the current clips to the spawned entity...
================
*/
gentity_t *Drop_Weapon( gentity_t *ent, gitem_t *item, float angle, int clips ) {
    vec3_t	velocity,forward;
    vec3_t	angles;
    vec3_t	origin;
    int		weaponmode = 0;
    qboolean	silencer = qfalse;

    VectorCopy( ent->s.apos.trBase, angles );
    angles[YAW] += angle;
    angles[PITCH] = 0;	// always forward

    AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
    VectorScale( forward, 250, velocity );
    velocity[2] += 200;

    VectorCopy(ent->client->ps.origin,origin );
    origin[2] += 10;


    weaponmode = ent->client->ns.weaponmode[ item->giTag ];

    ent->client->ns.weaponmode[ item->giTag ] = 0;

    if ( ent->client->ps.eFlags & EF_SILENCED )
        silencer = qtrue;

    // add this
    return LaunchWeapon( item, origin, velocity, clips, weaponmode, silencer );
}


qboolean BG_IsPrimary( int weapon);
qboolean BG_IsSecondary( int weapon);
/*
================
Drop_Weapon

Drops current Weapon
================
*/
void NS_DropWeapon ( gentity_t *ent)
{
    gclient_t	*client;
    usercmd_t	*ucmd;
    gitem_t		*item;
    gentity_t	*entity;

    qboolean	bomb=qfalse;
    byte i;
    int weapon; 

    client = ent->client;
    ucmd = &ent->client->pers.cmd;
    weapon = ent->client->ps.weapon;
 
    if ( ent->client->ps.eFlags & EF_VIP )
    {
        PrintMsg( ent, "The V.I.P. is " S_COLOR_RED "NOT" S_COLOR_WHITE " allowed to drop his weapon.\n");
        return;
    }
    if ( ent->client->ps.pm_flags & PMF_FOLLOW )
        return;

    if ( ent->client->ps.pm_flags & EF_WEAPONS_LOCKED)
        return;

    if (client->ps.pm_flags & PMF_BOMBCASE)
    {
        bomb_drop(ent);
        bomb = qtrue;
    }
    else if (client->ps.weaponstate != WEAPON_READY && client->ps.weaponstate != WEAPON_LASTRND)
        return;

    if (ent->s.weapon <= WP_NONE)
    {
        trap_SendServerCommand( client->ps.clientNum, "print \"No weapon to drop.\n\"" );
        return;
    }
    item = BG_FindItemForWeapon( client->ps.weapon );

    if ( BG_IsGrenade( weapon ) && client->ps.ammo[ item->giAmmoTag ] > 1 )
    {
        // more than one grenade
        // drop only ONE
        ent->client->ps.ammo[ item->giAmmoTag ]--;
    }
    else
    {
        // no ammo left
        if ( BG_IsGrenade( weapon ) )
            ent->client->ps.ammo[ item->giAmmoTag ] = 0;
        // remove the weapon
        BG_RemoveWeapon( client->ps.weapon, ent->client->ps.stats );

        // clear current weapon
        client->ps.weapon = WP_NONE;

        for ( i = WP_SMOKE - 1 ; i > 0 ; i-- ) {
            if ( BG_GotWeapon( i, client->ps.stats ) ) {
                ent->s.weapon = client->ps.weapon = i;
                break;
            }
        }
    }

    if ( !bomb )
    {
        entity = Drop_Weapon( ent, item, random()*360, ent->client->ns.rounds[weapon] );

        // only allow the team who dropped it to pick it up
        if ( weapon == WP_C4 )
            entity->ns_team = ent->client->sess.sessionTeam;
    }

    ent->client->ps.weaponstate = WEAPON_RAISING;
    ent->client->ps.weaponTime = GetWeaponTimeOnRaise(ent);

    ent->client->ps.torsoAnim = ( ( ent->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )	| TORSO_THROW;

}

/*
================
NS_DropMissionObjective

Drops the players current mission objective
================
*/
void NS_DropMissionObjective ( gentity_t *ent)
{
    gclient_t	*client;
    gitem_t		*item;

    client = ent->client;

    if ( ent->client->ps.pm_flags & PMF_FOLLOW )
        return;

    if (client->ps.pm_flags & PMF_BOMBCASE)
    {
        bomb_drop(ent);
        return;
    }
    if ( ent->client->ps.powerups[PW_BRIEFCASE] )
    {
        if ( ent->client->ns.is_vipWithBriefcase && ent->s.eFlags & EF_VIP )
        {
            PrintMsg(ent, "You can't drop your briefcase.\n");
            return;
        }

        item = BG_FindItemForPowerup(PW_BRIEFCASE);

        Drop_Item( ent, item, random()*360 );
        ent->client->ps.powerups[PW_BRIEFCASE] = 0;
    }
    else
    {
        trap_SendServerCommand( client->ps.clientNum, "print \"You got no Mission Objective.\n\"" );
        return;
    }

    ent->client->ps.torsoAnim = ( ( ent->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )	| TORSO_THROW;

}

/*
=================
NSQ3 Calculate Speed
author: Defcon-X
date: 13-07-2000
description: calculates players current speed
=================
*/
float NS_CalcSpeed ( gentity_t *ent )
{
    float	speed = g_speed.value;
    int		char_speed  = ent->client->pers.nsPC.speed;
    float	weight = ent->client->ns.weigth;

    speed *= 1.0 + SEALS_SPEED_FACT * ( ((float)char_speed - 1.0) / 9.0 );
    speed -= weight;

    return speed;
}
/*
=================
NSQ3 Calculate Weight
author: Defcon-X
date: 13-07-2000
description: calculates players current weight

normal soldier, level 1 on all values, with helmet and kevlar + m4 and 4 clips:
level 1.0 ~	   level 5.0   ~
57,95kg			49,40kg
=================
*/

float NS_CalcWeight ( gentity_t *ent )
{
    float weight = 0;
    int i;
    int	oldweapon = ent->s.weapon;

    if (!ent)
        return weight;

    if ( BG_IsMelee( ent->s.weapon ) || BG_IsGrenade( ent->s.weapon )  )
    {
        for ( i=WP_NUM_WEAPONS-1;i>WP_NONE;i--)
        {
            // so we have atleast one valid weapon.
            if ( BG_GotWeapon( i, ent->client->ps.stats ) )
            {
                if ( BG_IsSecondary( i ) )
                {
                    ent->s.weapon = i;
                    break;
                }
                else if ( BG_IsPrimary ( i ) )
                    ent->s.weapon = i;
            }
        }
    }

    //
    // Weapons
    //
    switch (ent->s.weapon) {
    case WP_M249:
        weight += SEALS_WEIGHT_M249;
        break;
    case WP_M14:
        weight += SEALS_WEIGHT_M14;
        break;
    case WP_MACMILLAN:
        weight += SEALS_WEIGHT_MACMILLAN;
        break;
    case WP_SPAS15:
        weight += SEALS_WEIGHT_SPAS15;
        break;
    case WP_AK47:
        weight += SEALS_WEIGHT_AK47;
        break;
    case WP_M4:
        weight += SEALS_WEIGHT_M4;
        break;
    case WP_870:
        weight += SEALS_WEIGHT_870;
        break;
    case WP_M590:
        weight += SEALS_WEIGHT_M590;
        break;
    case WP_PDW:
        weight += SEALS_WEIGHT_PDW;
        break;
    case WP_SL8SD:
        weight += SEALS_WEIGHT_SL8SD;
        break;
    case WP_PSG1:
        weight += SEALS_WEIGHT_PSG1;
        break;
    case WP_MAC10:
        weight += SEALS_WEIGHT_MAC10;
        break;
    case WP_MP5:
        weight += SEALS_WEIGHT_MP5;
        break;
    case WP_DEAGLE:
        weight += SEALS_WEIGHT_DEAGLE;
        break;
    case WP_SW629:
        weight += SEALS_WEIGHT_SW629;
        break;
    case WP_MK23:
        weight += SEALS_WEIGHT_MK23;
        break;
    case WP_SW40T:
        weight += SEALS_WEIGHT_SW40T;
        break;
    case WP_P9S:
        weight += SEALS_WEIGHT_P9S;
        break;
    case WP_GLOCK:
        weight += SEALS_WEIGHT_GLOCK;
        break;
    default:
        weight += SEALS_WEIGHT_DEFAULT;
    }

    ent->s.weapon = oldweapon;

    //
    // Ammo
    //
    if (ent->client->ps.ammo[AM_SHOTGUN] > 0) weight += ent->client->ps.ammo[AM_SHOTGUN]*SEALS_WEIGHT_SHOTGUN;
    if (ent->client->ps.ammo[AM_SHOTGUNMAG] > 0) weight += ent->client->ps.ammo[AM_SHOTGUNMAG]*SEALS_WEIGHT_SHOTGUNMAG;

    if (ent->client->ps.ammo[AM_LIGHT_PISTOL] > 0) weight += ent->client->ps.ammo[AM_LIGHT_PISTOL]*SEALS_WEIGHT_LIGHT_PISTOL;
    if (ent->client->ps.ammo[AM_MEDIUM_PISTOL] > 0) weight += ent->client->ps.ammo[AM_MEDIUM_PISTOL]*SEALS_WEIGHT_MEDIUM_PISTOL;
    if (ent->client->ps.ammo[AM_LARGE_PISTOL] > 0) weight += ent->client->ps.ammo[AM_LARGE_PISTOL]*SEALS_WEIGHT_LARGE_PISTOL;
    if (ent->client->ps.ammo[AM_SMG] > 0) weight += ent->client->ps.ammo[AM_SMG]*SEALS_WEIGHT_SMG;
    if (ent->client->ps.ammo[AM_RIFLE] > 0) weight += ent->client->ps.ammo[AM_RIFLE]*SEALS_WEIGHT_RIFLE;
    if (ent->client->ps.ammo[AM_MG] > 0) weight += ent->client->ps.ammo[AM_MG]*SEALS_WEIGHT_MG;
    if (ent->client->ps.ammo[AM_MEDIUM_SNIPER] > 0) weight += ent->client->ps.ammo[AM_MEDIUM_SNIPER]*SEALS_WEIGHT_MEDIUM_SNIPER;
    if (ent->client->ps.ammo[AM_LARGE_SNIPER] > 0) weight += ent->client->ps.ammo[AM_LARGE_SNIPER]*SEALS_WEIGHT_LARGE_SNIPER;
    if (ent->client->ps.ammo[AM_GRENADES] > 0) weight += ent->client->ps.ammo[AM_GRENADES]*SEALS_WEIGHT_GRENADE;
    if (ent->client->ps.ammo[AM_FLASHBANGS] > 0) weight += ent->client->ps.ammo[AM_FLASHBANGS]*SEALS_WEIGHT_FLASHBANG;
    if (ent->client->ps.ammo[AM_SMOKE] > 0) weight += ent->client->ps.ammo[AM_SMOKE]*SEALS_WEIGHT_SMOKE;
    if (ent->client->ps.ammo[AM_40MMGRENADES] > 0) weight += ent->client->ps.ammo[AM_40MMGRENADES]*SEALS_WEIGHT_40MMGRENADE;

    //
    // Kevlar Vest
    //

    if (ent->client->ps.powerups[PW_VEST] )
        weight += SEALS_WEIGHT_KEVLAR;

    //
    // Helmet
    //

    if (ent->client->ps.powerups[PW_HELMET] )
        weight += SEALS_WEIGHT_HELMET;

    // change the weight relative to the strength

    weight = weight * ( SEALS_WEIGHT_RATIO + (1.0 - SEALS_WEIGHT_RATIO) * (11.0 - (float)ent->client->pers.nsPC.strength)/10.0 );

    if ( (weight > 80.0) || (weight < 0.0))
        weight = 80.0;

    // update character if anything changed - so knows about any changes
    if ( ent->client->ps.persistant[PERS_STRENGTH] != ent->client->pers.nsPC.strength )
        ent->client->ps.persistant[PERS_STRENGTH] = ent->client->pers.nsPC.strength;

    if ( ent->client->ps.persistant[PERS_TECHNICAL] != ent->client->pers.nsPC.technical )
        ent->client->ps.persistant[PERS_TECHNICAL] = ent->client->pers.nsPC.technical;

    if ( ent->client->ps.persistant[PERS_STAMINA] != ent->client->pers.nsPC.stamina )
        ent->client->ps.persistant[PERS_STAMINA] = ent->client->pers.nsPC.stamina;

    if ( ent->client->ps.persistant[PERS_SPEED] != ent->client->pers.nsPC.speed )
        ent->client->ps.persistant[PERS_SPEED] = ent->client->pers.nsPC.speed;

    if ( ent->client->ps.persistant[PERS_STEALTH] != ent->client->pers.nsPC.stealth )
        ent->client->ps.persistant[PERS_STEALTH] = ent->client->pers.nsPC.stealth;

    if ( ent->client->ps.persistant[PERS_ACCURACY] != ent->client->pers.nsPC.accuracy )
        ent->client->ps.persistant[PERS_ACCURACY] = ent->client->pers.nsPC.accuracy;

    return weight;
}

/*
=================
NSQ3 Weapon Mode
author: Defcon-X
date: 13-07-2000
description: switches through all the different weapon modes
=================
*/
void NS_WeaponMode ( gentity_t *ent , int wmode )
{
    int weapon;
    char *mode = "none";

    weapon = ent->client->ps.weapon;

    if (!ent)
        return;

    if ( ent->client->ps.pm_flags & PMF_FOLLOW )
        return;
    if ( ent->client->ps.pm_flags & PMF_CLIMB ) {
        return;
    }

    if (weapon <= WP_NONE)
        return;

    if (ent->health <= 0)
        return;

    if ( ent->client->ps.weaponTime > 0 )
        return;

    ent->client->ns.weaponmode_tries[wmode] = 0;

    if ( wmode == 2 )
    {
        if ( ent->client->ps.eFlags & EF_IRONSIGHT )
        {
            // BLUTENGEL_XXX:
            // removed confusing message
            //PrintMsg(ent, "You can't use this mode while in Ironsight-Mode.\n");
            return;
        }
        if ( ( ent->client->ps.weapon == WP_M4 || ent->client->ps.weapon == WP_AK47 ) &&
                (ent->client->ns.weaponmode[weapon] & ( 1 << WM_GRENADELAUNCHER ) ) )
        {
            if ( ent->client->ns.weaponmode[weapon] & ( 1 << WM_WEAPONMODE2 ) ) {
                ent->client->ns.weaponmode[weapon] &= ~( 1 << WM_WEAPONMODE2 );
                mode = "Fire Mode";
                ent->client->ps.weaponstate = WEAPON_RELOADING_STOP;
                ent->client->ps.weaponTime = 300;
            }
            else
            {
                ent->client->ns.weaponmode[weapon] |= ( 1 << WM_WEAPONMODE2 );
                mode = "M-203!";
                ent->client->ps.weaponstate = WEAPON_RELOADING_CYCLE;
                ent->client->ps.weaponTime = 300;

            }
        }
        else if ( BG_WeaponMods( weapon ) & ( 1 << WM_FLASHLIGHT ) && ent->client->ns.weaponmode[weapon] & ( 1 << WM_FLASHLIGHT )  )
        {
            if ( ent->client->ns.weaponmode[weapon] & ( 1 << WM_WEAPONMODE2 ) ) {
                ent->client->ns.weaponmode[weapon] &= ~( 1 << WM_WEAPONMODE2 );
                mode = "Flashlight Disabled.";
            }
            else
            {
                ent->client->ns.weaponmode[weapon] |= ( 1 << WM_WEAPONMODE2 );
                mode = "Flashlight Enabled.";
            }
        }

        return;
    }

    // zooming
    if ( wmode == 1 )
    {
        if (
#ifdef SL8SD
            ent->client->ps.weapon == WP_SL8SD ||
#endif
            ent->client->ps.weapon == WP_PSG1 ||
            ent->client->ps.weapon == WP_MACMILLAN ) // got 4x zoom
        {
            // if i want to switch to
            if ( ent->client->ns.weaponmode[weapon] & ( 1 << WM_ZOOM4X) )
            {
                // go out of zoom
                ent->client->ns.weaponmode[weapon] &= ~( 1 << WM_ZOOM4X );
                mode = "Normal Zoom";
                G_LocalSound(ent, CHAN_WEAPON, G_SoundIndex( "sound/weapons/zoom.wav" ) );
            }
            else if ( ent->client->ns.weaponmode[weapon] & (1 << WM_ZOOM2X)  )
            {
                // set to 4x
                ent->client->ns.weaponmode[weapon] &= ~( 1 << WM_ZOOM2X );
                ent->client->ns.weaponmode[weapon] |= ( 1 << WM_ZOOM4X );
                mode = "4x Zoom";
                G_LocalSound(ent, CHAN_WEAPON, G_SoundIndex( "sound/weapons/zoom.wav" ) );
            }
            else
            {
                // set to 2x
                ent->client->ns.weaponmode[weapon] |= ( 1 << WM_ZOOM2X );
                mode = "2x Zoom";
                G_LocalSound(ent, CHAN_WEAPON, G_SoundIndex( "sound/weapons/zoom.wav" ) );
            }
        }
        else if ( ent->client->ns.weaponmode[weapon] & (1 << WM_SCOPE )  ) // scope add-on only2x
        {
            if ( ent->client->ns.weaponmode[weapon] & (1 << WM_ZOOM2X) )
            {
                // set back to normal
                ent->client->ns.weaponmode[weapon] &= ~( 1 << WM_ZOOM2X );
                mode = "Normal Zoom";
                G_LocalSound(ent, CHAN_WEAPON, G_SoundIndex( "sound/weapons/zoom.wav" ) );
            }
            else
            {
                // set to 2x if we're not in GL mode
                if ( ent->s.weapon == WP_M4 || ent->s.weapon == WP_AK47 ) {
                    if (! (ent->client->ns.weaponmode[weapon] & (1 << WM_GRENADELAUNCHER) && (ent->client->ps.stats[STAT_WEAPONMODE] & (1 << WM_WEAPONMODE2) ) ) ) {
                        ent->client->ns.weaponmode[weapon] |= ( 1 << WM_ZOOM2X );
                        mode = "2x Zoom";
                        G_LocalSound(ent, CHAN_WEAPON, G_SoundIndex( "sound/weapons/zoom.wav" ) );
                    }
                } else {
                    ent->client->ns.weaponmode[weapon] |= ( 1 << WM_ZOOM2X );
                    mode = "2x Zoom";
                    G_LocalSound(ent, CHAN_WEAPON, G_SoundIndex( "sound/weapons/zoom.wav" ) );
                }
            }
        }
        else if ( ent->client->ns.weaponmode[weapon] & (1 << WM_LASER ) ) // can't have scope + lasersight
        {
            if ( ent->client->ns.weaponmode[weapon] & (1 << WM_LACTIVE) )
            {
                // set back to normal
                ent->client->ns.weaponmode[weapon] &= ~( 1 << WM_LACTIVE );
                mode = "Laser Off";
            }
            else
            {
                // set to 2x
                ent->client->ns.weaponmode[weapon] |= ( 1 << WM_LACTIVE );
                mode = "Laser On";
            }
        }
        else if ( weapon == WP_GRENADE || weapon == WP_FLASHBANG || weapon == WP_SMOKE )
        {
            // if i want to switch to
            if ( ent->client->ns.weaponmode[weapon] & ( 1 << WM_SINGLE) )
            {
                // go to fullauto
                ent->client->ns.weaponmode[weapon] &= ~( 1 << WM_SINGLE );
                ent->client->ns.weaponmode[weapon] |= ( 1 << WM_WEAPONMODE2 ); // 5 sec priming
                mode = "5-Sec Priming";
            }
            else if ( ent->client->ns.weaponmode[weapon] & ( 1 << WM_WEAPONMODE2 ) )
            {
                ent->client->ns.weaponmode[weapon] &= ~( 1 << WM_WEAPONMODE2 );
                mode = "3-Sec Priming ( default )";
            }
            else
            {
                // set to burst
                ent->client->ns.weaponmode[weapon] |= ( 1 << WM_SINGLE ); // single = 4
                mode = "4-Sec Priming";
            }

        }
        else if ( weapon == WP_PDW )
        {

            if ( ent->client->ns.weaponmode[weapon] & ( 1 << WM_WEAPONMODE2 ) )
            {
                ent->client->ns.weaponmode[weapon] &= ~( 1 << WM_WEAPONMODE2 );
                ent->client->ps.weaponstate = WEAPON_RELOADING_CYCLE;
                ent->client->ps.weaponTime = 450;
                mode = "Recoilcatcher disabled";
            }
            else
            {
                // go to weaponmode2
                ent->client->ns.weaponmode[weapon] |= ( 1 << WM_WEAPONMODE2 );
                ent->client->ps.weaponstate = WEAPON_RELOADING_STOP;
                ent->client->ps.weaponTime = 450;
                mode = "Recoilcatcher enabled";
            }
        }
        else if ( BG_WeaponMods( weapon ) & ( 1 << WM_FLASHLIGHT ) && ent->client->ns.weaponmode[weapon] & ( 1 << WM_FLASHLIGHT )  )
        {
            if ( ent->client->ns.weaponmode[weapon] & ( 1 << WM_WEAPONMODE2 ) ) {
                ent->client->ns.weaponmode[weapon] &= ~( 1 << WM_WEAPONMODE2 );
                mode = "Flashlight Disabled.";
            }
            else
            {
                ent->client->ns.weaponmode[weapon] |= ( 1 << WM_WEAPONMODE2 );
                mode = "Flashlight Enabled.";
            }
        }

        //		if (mode != "none")
        //			PrintMsg(ent, "Setted mode to %s\n", mode );
        return;
    }

    if ( weapon == WP_GRENADE || weapon == WP_FLASHBANG || weapon == WP_SMOKE )
    {
        if ( ent->client->ns.weaponmode[weapon] & ( 1 << WM_GRENADEROLL ) )
        {
            // go to roll
            ent->client->ns.weaponmode[weapon] &= ~( 1 << WM_GRENADEROLL );
            mode = "Throw";
        }
        else
        {
            // set to burst
            ent->client->ns.weaponmode[weapon] |= ( 1 << WM_GRENADEROLL );
            mode = "Roll";
        }

        //		PrintMsg(ent, "Setted mode to %s\n", mode );
        return;
    }


    if (  weapon == WP_M14 || weapon == WP_M4 || weapon == WP_MAC10 || weapon == WP_AK47 || weapon == WP_MP5 || weapon == WP_PDW   )	{
        // if i want to switch to
        if ( ent->client->ns.weaponmode[weapon] & ( 1 << WM_SINGLE) )
        {
            // go to fullauto
            ent->client->ns.weaponmode[weapon] &= ~( 1 << WM_SINGLE );
            mode = "Full Auto";
        }
        else
        {
            // set to burst
            ent->client->ns.weaponmode[weapon] |= ( 1 << WM_SINGLE );
            mode = "Single Shot";
        }

        //		if (mode != "none")
        //			PrintMsg(ent, "Setted mode to %s\n", mode );
        return;
    }
}

/*
=================
NSQ3 Holster Weapon
author: Defcon-X
date: 24-08-2000
description: holsters current weapon
=================
*/
void NS_HolsterWeapon ( gentity_t *ent )
{
    if ( ent->client->ps.pm_flags & PMF_FOLLOW )
        return;

    // unholster ( pressed twice the holster button )
    if ( ent->client->ps.weaponstate == WEAPON_HOLSTERING )
    {
        ent->client->ps.weaponTime = GetWeaponTimeOnRaise(ent);
        ent->client->ps.weaponstate = WEAPON_RAISING;
        return;
    }
    else if ( ent->client->ps.weaponstate == WEAPON_READY )
    {
        ent->client->ps.weaponstate = WEAPON_HOLSTERING; // run raising frames
        ent->client->ps.weaponTime = GetWeaponTimeOnLower(ent);
    }
}

qboolean PI_CheckWeapon ( int team , int weapon, int acc, int str, int sta, int stl, qboolean stolen )
{

    if ( weapon == WP_C4 )
        return qtrue;
    if ( BG_IsMelee( weapon ) || BG_IsGrenade( weapon ) )
        return qtrue;

    if ( team == TEAM_RED || stolen )
    {
        switch ( weapon )
        {
        case WP_MP5:
        case WP_M4:
        case WP_MK23:
        case WP_P9S:
        case WP_870:
            return qtrue;
            break;
        case WP_SW629:
            if ( str > 3 )
                return qtrue;
            break;
        case WP_PDW:
            if ( str > 3 && acc > 4 )
                return qtrue;
            break;
        case WP_PSG1:
            if ( acc > 6 )
                return qtrue;
            break;
#ifdef SL8SD
        case WP_SL8SD:
            if ( acc > 5 && stl > 5 )
                return qtrue;
            break;
#endif
        case WP_MACMILLAN:
            if ( acc > 8 && str > 4 )
                return qtrue;
            break;
        case WP_SPAS15:
            if ( str > 4 )
                return qtrue;
            break;
        case WP_M14:
            if ( str >= 6 )
                return qtrue;
            break;
        case WP_M249:
            if ( str >= 4 && sta >= 5)
                return qtrue;
        default:
            break;
        }
    }

    if ( team == TEAM_BLUE || stolen )
    {
        switch ( weapon )
        {
        case WP_MAC10:
        case WP_AK47:
        case WP_GLOCK:
        case WP_SW40T:
        case WP_M590:
            return qtrue;
            break;
        case WP_DEAGLE:
            if ( str > 3 )
                return qtrue;
            break;
        case WP_PDW:
            if ( str > 3 && acc > 4 )
                return qtrue;
            break;
        case WP_PSG1:
            if ( acc > 6 )
                return qtrue;
            break;
#ifdef SL8SD
        case WP_SL8SD:
            if ( acc > 5 && stl > 5 )
                return qtrue;
            break;
#endif
        case WP_MACMILLAN:
            if ( acc > 8 && str > 4 )
                return qtrue;
            break;
        case WP_SPAS15:
            if ( str > 4 )
                return qtrue;
            break;
        case WP_M14:
            if ( str >= 6 )
                return qtrue;
            break;
        case WP_M249:
            if ( str >= 4 && sta >= 5)
                return qtrue;
        default:
            break;
        }
    }

    if ( team == TEAM_FREE )
        return qtrue;

    return qfalse;
}

/*
=================
NSQ3 Check end round
author: defcon-x
date: 12-07-2001
description: returns true if we should end the current round
=================
*/
qboolean NS_CheckEndRound ( void )
{
    int			i ;
    gentity_t	*ent;

    // round is still running
    if ( level.time < level.roundstartTime )
        return qfalse;

    ent = &g_entities[0];

    for (i=0 ; i<level.num_entities ; i++, ent++)
    {
        if ( !ent->inuse ) {
            continue;
        }
        if (!ent || !ent->classname)
            continue;

        if (!Q_stricmp("endround_holder", ent->classname ) )
        {
            return qfalse;
        }
        // find c4
        if (!Q_stricmp("c4_placed", ent->classname))
        {
            // if entity still has got time left
            if ( level.time <  ent->nextthink + 1000 )
                return qfalse; // don't end round
        }
    }
    // return
    return qtrue;



}
/*
=================
NSQ3 Bomb exist for team
author: defcon-x
date: 12-07-2001
description: returns true if we should end the current round
=================
*/
qboolean NS_BombExistForTeam ( team_t team )
{
    int			i;
    gentity_t	*ent;

    ent = &g_entities[0];

    // map hack
    if ( !Q_stricmp(NS_GetMapName(),"ns_snowcamp") )
        return qfalse;

    for (i=0 ; i<level.num_entities ; i++, ent++)
    {
        if ( !ent->inuse ) {
            continue;
        }
        if (!ent || !ent->classname)
            continue;

        // found roundholder? for whatever it called.
        if (!Q_stricmp("endround_holder", ent->classname ) )
            return qtrue;
        // find c4
        if (!Q_stricmp("c4_placed", ent->classname))
            // if entity still has got time left
            if ( ent->ns_team == team )
                return qtrue; // don't end round
    }
    // return
    return qfalse;

}
/*
=================
NSQ3 Recalc Character
author: defcon-x
date: 15-03-2003
description: recalc character depending on "character" cvar
=================
*/
#include "../../ui/menudef.h"

int NS_GetTotalXPforLevel( int level )
{
    int total=0;
    int i;

    for (i=0;i<=level;i++)
    {
        if ( i <= 1 )
            continue;

        total += i;
    }
    return total;
}
int NS_GetTotalXPforAllLevels( gentity_t *ent )
{
    int value = 0;

    value += NS_GetTotalXPforLevel( ent->client->pers.nsPC.strength );
    value += NS_GetTotalXPforLevel( ent->client->pers.nsPC.technical );
    value += NS_GetTotalXPforLevel( ent->client->pers.nsPC.stamina );
    value += NS_GetTotalXPforLevel( ent->client->pers.nsPC.accuracy );
    value += NS_GetTotalXPforLevel( ent->client->pers.nsPC.speed );
    value += NS_GetTotalXPforLevel( ent->client->pers.nsPC.stealth);

    return value;
}
void NS_RecalcCharacter ( gentity_t *ent )
{
    int acc,str,stl,tec,spd,sta;
    int totalxp = ent->client->pers.nsPC.entire_xp;

    if ( g_matchLockXP.integer )
    {
        PrintMsg(ent, "This command is not available in Match Mode.\n");
        return;
    }


    if ( NS_ActiveRound()
            && ( ent->client->sess.waiting == qfalse )
            && ( ent->health > 0 ) )
    {
        PrintMsg(ent, "You cannot use this command when you are alive.\n");
        return;
    }


    acc = ent->client->pers.character[PC_ACCURACY] - 48;
    if ( acc <= 0 )
        acc = 10;
    if ( acc > 10 )
        acc = 10;
    totalxp -= NS_GetTotalXPforLevel( acc );
    while ( totalxp < 0 && acc > 1 )
    {
        totalxp += NS_GetTotalXPforLevel( acc );
        acc--;
        totalxp -= NS_GetTotalXPforLevel( acc );
    }

    str = ent->client->pers.character[PC_STRENGTH] - 48;
    if ( str <= 0 )
        str = 10;
    if ( str > 10 )
        str = 10;
    totalxp -= NS_GetTotalXPforLevel( str );
    while ( totalxp < 0 && str > 1 )
    {
        totalxp += NS_GetTotalXPforLevel( str );
        str--;
        totalxp -= NS_GetTotalXPforLevel( str );
    }

    spd = ent->client->pers.character[PC_SPEED] - 48;
    if ( spd <= 0 )
        spd = 10;
    if ( spd > 10 )
        spd = 10;
    totalxp -= NS_GetTotalXPforLevel( spd );
    while ( totalxp < 0 && spd > 1 )
    {
        totalxp += NS_GetTotalXPforLevel( spd );
        spd--;
        totalxp -= NS_GetTotalXPforLevel( spd );
    }

    stl = ent->client->pers.character[PC_STEALTH] - 48;
    if ( stl <= 0 )
        stl = 10;
    if ( stl > 10 )
        stl = 10;
    totalxp -= NS_GetTotalXPforLevel( stl );
    while ( totalxp < 0 && stl > 1 )
    {
        totalxp += NS_GetTotalXPforLevel( stl );
        stl--;
        totalxp -= NS_GetTotalXPforLevel( stl );
    }

    sta = ent->client->pers.character[PC_STAMINA] - 48;
    if ( sta <= 0 )
        sta = 10;
    if ( sta > 10 )
        sta = 10;
    totalxp -= NS_GetTotalXPforLevel( sta );
    while ( totalxp < 0 && sta > 1 )
    {
        totalxp += NS_GetTotalXPforLevel( sta );
        sta--;
        totalxp -= NS_GetTotalXPforLevel( sta );
    }

    tec = ent->client->pers.character[PC_TECHNICAL] - 48;
    if ( tec <= 0 )
        tec = 10;
    if ( tec > 10 )
        tec = 10;
    totalxp -= NS_GetTotalXPforLevel( tec );
    while ( totalxp < 0 && tec > 1 )
    {
        totalxp += NS_GetTotalXPforLevel( tec );
        tec--;
        totalxp -= NS_GetTotalXPforLevel( tec );
    }

    ent->client->pers.nsPC.accuracy = acc;
    ent->client->pers.nsPC.strength = str;
    ent->client->pers.nsPC.speed = spd;
    ent->client->pers.nsPC.stealth = stl;
    ent->client->pers.nsPC.stamina = sta;
    ent->client->pers.nsPC.technical = tec;


    if ( !PI_CheckWeapon( ent->client->sess.sessionTeam, ent->client->pers.nsInven.primaryweapon , acc,str,sta,stl, qfalse ) )
    {
        int ammo = 0;
        int weapon = ent->client->pers.nsInven.primaryweapon;
        int newweapon = WP_M4;
        gitem_t *it = BG_FindItemForWeapon( weapon );

        if ( it ) {
            ammo = ent->client->pers.nsInven.ammo[ it->giAmmoTag ];
            ent->client->pers.nsInven.ammo[ it->giAmmoTag ] = 0;

            if ( ent->client->sess.sessionTeam == TEAM_BLUE )
                newweapon = WP_AK47;

            NS_SetPrimary( ent, newweapon );

            BG_RemoveWeapon( weapon, ent->client->ps.stats );

            it = BG_FindItemForWeapon( newweapon );
            ent->client->pers.nsInven.ammo[ it->giAmmoTag ] = ammo;

            BG_PackWeapon( newweapon, ent->client->ps.stats );
        }
    }
    if ( !PI_CheckWeapon( ent->client->sess.sessionTeam, ent->client->pers.nsInven.secondaryweapon , acc,str,sta,stl, qfalse ) )
    {
        int ammo = 0;
        int weapon = ent->client->pers.nsInven.secondaryweapon;
        int newweapon = WP_MK23;
        gitem_t *it = BG_FindItemForWeapon( weapon );

        if ( it ) {
            ammo = ent->client->pers.nsInven.ammo[ it->giAmmoTag ];
            ent->client->pers.nsInven.ammo[ it->giAmmoTag ] = 0;

            if ( ent->client->sess.sessionTeam == TEAM_BLUE )
                newweapon = WP_GLOCK;

            NS_SetPrimary( ent, newweapon );

            BG_RemoveWeapon( weapon, ent->client->ps.stats );

            it = BG_FindItemForWeapon( newweapon );
            ent->client->pers.nsInven.ammo[ it->giAmmoTag ] = ammo;

            BG_PackWeapon( newweapon, ent->client->ps.stats );
        }
    }

    G_LogPrintf( "[%i] \"%s\" recalculated char %s to: acc %i str %i spd %i stl %i sta %i tec %i\n",ent->client->ps.clientNum, ent->client->pers.netname, ent->client->pers.character ,acc,str,spd,stl,sta,tec );
}

/*
=================
NSQ3 Character Raise
author: defcon-x
date: 12-07-2001
description: raises the character abilities
=================
*/
void NS_RaiseCharacterLevel( gentity_t *ent )
{
    char	str[MAX_TOKEN_CHARS];
    int		i;

    if ( g_matchLockXP.integer )
    {
        PrintMsg(ent, "This command is not available in Match Mode.\n");
        return;
    }

    if ( NS_ActiveRound()
            && ( ent->client->sess.waiting == qfalse )
            && ( ent->health > 0 ) )
    {
        PrintMsg(ent, "You cannot use this command when you are alive.\n");
        return;
    }

    trap_Argv( 1, str, sizeof( str ) );
    i = atoi(str);

    switch ( i )
    {
    case PC_STRENGTH:
        if ( NS_GotEnoughXPfornextLevel ( ent, ent->client->pers.nsPC.strength ) &&
                ent->client->pers.nsPC.strength < 10 ) {
            ent->client->pers.nsPC.strength ++;

            NS_GiveXP( ent, ent->client->pers.nsPC.strength  , qtrue );
        }
        break;
    case PC_TECHNICAL:
        if ( NS_GotEnoughXPfornextLevel ( ent, ent->client->pers.nsPC.technical ) &&
                ent->client->pers.nsPC.technical < 10 ) {
            ent->client->pers.nsPC.technical ++;

            NS_GiveXP( ent, ent->client->pers.nsPC.technical  , qtrue );
        }
        break;
    case PC_STAMINA:
        if ( NS_GotEnoughXPfornextLevel ( ent, ent->client->pers.nsPC.stamina ) &&
                ent->client->pers.nsPC.stamina < 10 ) {
            ent->client->pers.nsPC.stamina ++;

            NS_GiveXP( ent, ent->client->pers.nsPC.stamina  , qtrue );
        }
        break;
    case PC_ACCURACY:
        if ( NS_GotEnoughXPfornextLevel ( ent, ent->client->pers.nsPC.accuracy  ) &&
                ent->client->pers.nsPC.accuracy < 10 ) {
            ent->client->pers.nsPC.accuracy ++;

            NS_GiveXP( ent, ent->client->pers.nsPC.accuracy  , qtrue );
        }
        break;
    case PC_STEALTH:
        if ( NS_GotEnoughXPfornextLevel ( ent, ent->client->pers.nsPC.stealth   ) &&
                ent->client->pers.nsPC.stealth < 10 ) {
            ent->client->pers.nsPC.stealth ++;

            NS_GiveXP( ent, ent->client->pers.nsPC.stealth  , qtrue );
        }
        break;
    case PC_SPEED:
        if ( NS_GotEnoughXPfornextLevel ( ent, ent->client->pers.nsPC.speed   ) &&
                ent->client->pers.nsPC.speed < 10 ) {
            ent->client->pers.nsPC.speed ++;

            NS_GiveXP( ent, ent->client->pers.nsPC.speed  , qtrue );
        }
        break;
    default:
        break;
    }

    if ( !PI_CheckWeapon( ent->client->sess.sessionTeam, ent->client->pers.nsInven.primaryweapon , ent->client->pers.nsPC.accuracy,ent->client->pers.nsPC.strength,ent->client->pers.nsPC.stamina,ent->client->pers.nsPC.stealth, qfalse ) )
    {
        int ammo = 0;
        int weapon = ent->client->pers.nsInven.primaryweapon;
        int newweapon = WP_M4;
        gitem_t *it = BG_FindItemForWeapon( weapon );

        if ( it ) {
            ammo = ent->client->pers.nsInven.ammo[ it->giAmmoTag ];
            ent->client->pers.nsInven.ammo[ it->giAmmoTag ] = 0;

            if ( ent->client->sess.sessionTeam == TEAM_BLUE )
                newweapon = WP_AK47;

            NS_SetPrimary( ent, newweapon );

            BG_RemoveWeapon( weapon, ent->client->ps.stats );

            it = BG_FindItemForWeapon( newweapon );
            ent->client->pers.nsInven.ammo[ it->giAmmoTag ] = ammo;

            BG_PackWeapon( newweapon, ent->client->ps.stats );
        }
    }
    if ( !PI_CheckWeapon( ent->client->sess.sessionTeam, ent->client->pers.nsInven.secondaryweapon ,ent->client->pers.nsPC.accuracy,ent->client->pers.nsPC.strength,ent->client->pers.nsPC.stamina,ent->client->pers.nsPC.stealth, qfalse ) )
    {
        int ammo = 0;
        int weapon = ent->client->pers.nsInven.secondaryweapon;
        int newweapon = WP_MK23;
        gitem_t *it = BG_FindItemForWeapon( weapon );

        if ( it ) {
            ammo = ent->client->pers.nsInven.ammo[ it->giAmmoTag ];
            ent->client->pers.nsInven.ammo[ it->giAmmoTag ] = 0;

            if ( ent->client->sess.sessionTeam == TEAM_BLUE )
                newweapon = WP_GLOCK;

            NS_SetPrimary( ent, newweapon );

            BG_RemoveWeapon( weapon, ent->client->ps.stats );

            it = BG_FindItemForWeapon( newweapon );
            ent->client->pers.nsInven.ammo[ it->giAmmoTag ] = ammo;

            BG_PackWeapon( newweapon, ent->client->ps.stats );
        }
    }

}

/*
=================
NSQ3 Calculate Starting Experience Points
author: defcon-x
date: 12-07-2001
description: calculates the amount of xp a player gets when he changes/joins a team
=================
*/
int NS_DurchschnittXPforTeam( int ignoreClientNum, team_t team );

int NS_CalculateStartingXP( int team )
{
    if ( g_gametype.integer == GT_TEAM )
        return g_teamXp.integer;
    //fixme
    return NS_DurchschnittXPforTeam( -1, team );
}
/*
=================
NSQ3 Inventory Handle
author: defcon-x
date: 12-07-2001
description: new way to handle player inventory
=================
*/

void NS_PlayerInventory( gentity_t *ent )
{
    char	str[MAX_TOKEN_CHARS];
    int		i;
    int		roundStartTime = 0;
    int		oldPrimary,oldSecondary;
    int   grennum = 0;

    oldPrimary = oldSecondary = WP_NONE;

    /*
    if (g_gametype.integer < GT_TEAM )
    {
    PrintMsg(ent, "This command is only avaiable in Team-Play.\n");
    return;
    }
    */

    trap_Argv(23, str, sizeof( str ) );
    if (Q_stricmp(ent->client->pers.character, str)) {
        Q_strncpyz( ent->client->pers.character, str, sizeof( ent->client->pers.character));
        NS_RecalcCharacter(ent);
    }

    trap_Argv( 1, str, sizeof( str ) );
    i = atoi(str);

    if (i > 0 &&  i < WP_NUM_WEAPONS &&
            i != ent->client->pers.nsInven.primaryweapon ) {

        // failed check.. default to mp5 for seals/mac10 for tangos
        if ( !PI_CheckWeapon ( ent->client->sess.sessionTeam, i,
                               ent->client->pers.nsPC.accuracy, ent->client->pers.nsPC.strength, ent->client->pers.nsPC.stamina, ent->client->pers.nsPC.stealth ,qfalse) || !BG_IsPrimary(i) )
        {
            if ( ent->client->sess.sessionTeam == TEAM_RED )
                i = WP_MP5;
            else
                i = WP_MAC10;
        }
        oldPrimary = ent->client->pers.nsInven.primaryweapon;

        ent->client->pers.nsInven.primaryweapon = i;
        /*
        it = BG_FindItemForWeapon(i);

        G_Printf("setted weapon %s as primary\n", it->pickup_name );
        */
    }

    trap_Argv( 2, str, sizeof( str ) );
    i = atoi(str);
    if (i > 0 && i < WP_NUM_WEAPONS &&
            i != ent->client->pers.nsInven.secondaryweapon )
    {
        if ( !PI_CheckWeapon ( ent->client->sess.sessionTeam, i,
                               ent->client->pers.nsPC.accuracy, ent->client->pers.nsPC.strength,
                               ent->client->pers.nsPC.stamina, ent->client->pers.nsPC.stealth , qfalse) || !BG_IsSecondary( i ) )
        {
            if ( ent->client->sess.sessionTeam == TEAM_RED )
                i = WP_P9S;
            else
                i = WP_GLOCK;
        }
        oldSecondary = ent->client->pers.nsInven.secondaryweapon;

        ent->client->pers.nsInven.secondaryweapon = i;
        /*
        it = BG_FindItemForWeapon(i);

        G_Printf("setted weapon %s as secondary\n", it->pickup_name );
        */
    }

    trap_Argv( 3, str, sizeof( str ) );
    i = atoi(str);
    if ( ent->client->pers.nsInven.primaryweapon )
    {
        gitem_t *it_primary = BG_FindItemForWeapon( ent->client->pers.nsInven.primaryweapon );


        if( i > 6 || i < 0 )
            i = 4;

        ent->client->pers.nsInven.ammo[ it_primary->giAmmoTag ] = i;

        //		G_Printf("setted %i ammo for primary\n", i);
    }
    trap_Argv( 4, str, sizeof( str ) );
    i = atoi(str);
    if ( ent->client->pers.nsInven.secondaryweapon )
    {
        gitem_t *it_primary = BG_FindItemForWeapon( ent->client->pers.nsInven.secondaryweapon );

        if( i > 4 || i < 0 )
            i = 2;

        ent->client->pers.nsInven.ammo[ it_primary->giAmmoTag ] = i;
        //		G_Printf("setted %i ammo for secondary\n", i);
    }
    trap_Argv( 5, str, sizeof( str ) );
    i = atoi(str);
    if ( i > -1 )
    {
        int num_gren = i;

        if ( num_gren > 2 || num_gren < 0 )
            num_gren = 1;

        ent->client->pers.nsInven.ammo[ AM_40MMGRENADES ] = num_gren;
        grennum += num_gren;
        //		G_Printf("setted %i ammo for 40mm grenades\n", i);
    }
    trap_Argv( 6, str, sizeof( str ) );
    i = atoi(str);
    if ( i > -1 )
    {
        int num_gren = atoi ( str );

        if ( num_gren > 2 || num_gren < 0 )
            num_gren = 1;

        ent->client->pers.nsInven.ammo[ AM_GRENADES ] = num_gren;
        grennum += num_gren;
        //		G_Printf("setted %i grenades\n", i);
    }

    trap_Argv( 7, str, sizeof( str ) );
    i = atoi(str);
    if ( i > -1 )
    {
        int num_gren = atoi ( str );

        if ( num_gren > 2 || num_gren < 0 )
            num_gren = 1;

        if ((grennum + num_gren) <= SEALS_MAX_GRENADES) ent->client->pers.nsInven.ammo[ AM_FLASHBANGS ] = num_gren;
        else {
            if (BG_GotWeapon(WP_FLASHBANG, ent->client->ps.stats) ) BG_RemoveWeapon(WP_FLASHBANG, ent->client->ps.stats);
            ent->client->pers.nsInven.ammo[ AM_FLASHBANGS ] = 0;
        }
        grennum += num_gren;
        //		G_Printf("setted %i flashbangs\n", i);
    }

    trap_Argv( 8, str, sizeof( str ) );
    i = atoi(str);
    if ( i > -1 )
    {
        int num_gren = atoi ( str );

        if ( num_gren > 2 || num_gren < 0 )
            num_gren = 1;

        if ( (grennum + num_gren) <= SEALS_MAX_GRENADES) ent->client->pers.nsInven.ammo[ AM_SMOKE ] = num_gren;
        else {
            if (BG_GotWeapon(WP_SMOKE, ent->client->ps.stats) ) BG_RemoveWeapon(WP_SMOKE, ent->client->ps.stats);
            ent->client->pers.nsInven.ammo[ AM_SMOKE ] = 0;
        }
        grennum += num_gren;

        //		G_Printf("setted %i smokegrenades\n", i);
    }

    trap_Argv( 9, str, sizeof( str ) );
    i = atoi(str);
    if ( i )
        ent->client->pers.nsInven.powerups[PW_VEST] = 1;
    else
        ent->client->pers.nsInven.powerups[PW_VEST] = 0;

    trap_Argv( 10, str, sizeof( str ) );
    i = atoi(str);
    if ( i )
        ent->client->pers.nsInven.powerups[PW_HELMET] = 1;
    else
        ent->client->pers.nsInven.powerups[PW_HELMET] = 0;

    trap_Argv( 11, str, sizeof( str ) );
    i = atoi(str);
    if ( i && BG_WeaponMods( ent->client->pers.nsInven.primaryweapon) & ( 1 << WM_SCOPE) && ent->client->pers.nsPC.accuracy >= 6 )
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].scope = 1;
    else
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].scope = 0;

    trap_Argv( 12, str, sizeof( str ) );
    i = atoi(str);
    if ( i && BG_WeaponMods( ent->client->pers.nsInven.primaryweapon) & ( 1 << WM_GRENADELAUNCHER ) && ent->client->pers.nsPC.technical >= 5 && ent->client->pers.nsPC.strength >= 3  )
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].gl = 1;
    else
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].gl = 0;

    trap_Argv( 13, str, sizeof( str ) );
    i = atoi(str);

    // only allow if no gl attached
    if ( i && BG_WeaponMods( ent->client->pers.nsInven.primaryweapon) & ( 1 << WM_BAYONET ) && ent->client->pers.nsPC.strength >= 4 && !ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].gl )
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].bayonet = 1;
    else
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].bayonet = 0;

    trap_Argv( 14, str, sizeof( str ) );
    i = atoi(str);
    if ( i && BG_WeaponMods( ent->client->pers.nsInven.primaryweapon) & ( 1 << WM_LASER ) && ent->client->pers.nsPC.accuracy >= 4 )
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].lasersight = 1;
    else
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].lasersight = 0;

    trap_Argv( 15, str, sizeof( str ) );
    i = atoi(str);

    if ( i && BG_WeaponMods( ent->client->pers.nsInven.primaryweapon) & ( 1 << WM_SILENCER ) && ent->client->pers.nsPC.stealth >= 5 )
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].silencer = 1;
    else
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].silencer = 0;


    trap_Argv( 16, str, sizeof( str ) );
    i = atoi(str);
    if ( i && BG_WeaponMods( ent->client->pers.nsInven.secondaryweapon) & ( 1 << WM_SCOPE) )
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].scope = 1;
    else
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].scope = 0;

    trap_Argv( 17, str, sizeof( str ) );
    i = atoi(str);
    if ( i && BG_WeaponMods( ent->client->pers.nsInven.secondaryweapon) & ( 1 << WM_GRENADELAUNCHER ) )
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].gl = 1;
    else
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].gl = 0;

    trap_Argv( 18, str, sizeof( str ) );
    i = atoi(str);
    if ( i && BG_WeaponMods( ent->client->pers.nsInven.secondaryweapon) & ( 1 <<  WM_BAYONET) )
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].bayonet = 1;
    else
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].bayonet = 0;

    trap_Argv( 19, str, sizeof( str ) );
    i = atoi(str);
    if ( i && BG_WeaponMods( ent->client->pers.nsInven.secondaryweapon) & ( 1 << WM_LASER ) && ent->client->pers.nsPC.accuracy >= 3 )
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].lasersight = 1;
    else
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].lasersight = 0;

    trap_Argv( 20, str, sizeof( str ) );
    i = atoi(str);

    if ( i && BG_WeaponMods( ent->client->pers.nsInven.secondaryweapon) & ( 1 << WM_SILENCER ) && ent->client->pers.nsPC.stealth >= 3 )
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].silencer = 1;
    else
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].silencer = 0;

    // fixme: update client
    trap_Argv( 21, str, sizeof( str ) );
    i = atoi(str);
    if ( i && BG_WeaponMods( ent->client->pers.nsInven.primaryweapon) & ( 1 << WM_DUCKBILL ) && ent->client->pers.nsPC.strength >= 5 )
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].duckbill = 1;
    else
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].duckbill = 0;

    trap_Argv( 22, str, sizeof( str ) );
    i = atoi(str);

    if ( i && BG_WeaponMods( ent->client->pers.nsInven.primaryweapon) & ( 1 << WM_FLASHLIGHT ) && ent->client->pers.nsPC.technical >= 4 )
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].flashlight = 1;
    else
        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].flashlight = 0;

    roundStartTime = level.roundstartTime;
    // remove roundtime to get time the round started.
    if ( ( /* level.vip[TEAM_RED] == VIP_ESCAPE || */ level.vip[TEAM_BLUE] == VIP_STAYALIVE ) && !g_overrideGoals.integer)
        roundStartTime -= level.vipTime * 60 * ONE_SECOND;
    else
        roundStartTime -= g_roundTime.value * 60 * ONE_SECOND;

    roundStartTime += g_invenTime.value * ONE_SECOND;

    // cheat fix
    if ( ent->client->ns.is_vip )
        return;
    if ( ent->client->ps.pm_flags & PMF_FOLLOW )
        return;

    //	NS_ClearInventory( ent );
    // update clientweapons / ammo if time is still in valid range

    if ( ( GameState == STATE_OPEN ) || ( GameState == STATE_LOCKED && level.time < roundStartTime ) )
    {
        if ( g_gametype.integer == GT_LTS ||
                ( g_gametype.integer == GT_TEAM && ent->client->respawnTime + RESPAWN_INVUNERABILITY_TIME > level.time )  )
        {
            BG_RemoveWeapon( oldPrimary, ent->client->ps.stats );
            BG_RemoveWeapon( oldSecondary, ent->client->ps.stats );

            NS_NavySeals_ClientInit( ent , qfalse );
        }
    }

    // the following is the order how the string gets send form the client to teh server.
    // all in <> is a cmd rest is decription
    // <cmd>  <primary> <secondary> <PriAmmo> <SecAmmo> <40mm grenades> <Grenades> <Fl Grenades> <kevlar> <helmet>
    //pri  <scope> <gl> <bayonet> <lasersight> <silencer>
    //sec  <scope> <gl> <bayonet> <lasersight> <silencer>
    //pri  <duckbill> <flashlight>
}
/*
=================
NSQ3 Gesture
author: Defcon-X
date: 24-08-2000
description: stars gesture animations (point/fist/wave)
=================
*/
void NS_Gesture( gentity_t *ent )
{
    char	str[MAX_TOKEN_CHARS];
    char	*anim = "wrong";
    int		i,anm = 0;

    trap_Argv( 1, str, sizeof( str ) );
    i = atoi( str );

    if (!i) {
        PrintMsg( ent, "Usage:\ncmd Gesture <1/2/3>\n");
        return;
    }
    if ( ent->client->ps.pm_flags & PMF_FOLLOW )
        return;
    if (ent->client->ps.torsoTimer)
        return;

    // if greater then 3 or smaller 1 default to 1
    if ( i > 3 || i < 1 )
        i = 1;

    switch (i) {
    case 1:
        anim = "Point";
        anm = TORSO_GESTURE1;
        break;
    case 2:
        anim = "Fist";
        anm = TORSO_GESTURE2;
        break;
    case 3:
        anim = "Wave";
        anm = TORSO_GESTURE3;
        break;
    }

    PrintMsg( ent, "%s\n", anim );

    NS_PlayerAnimation( ent, anm, 1500, qfalse );


}

/*
=================
NSQ3 Gesture
author: Defcon-X
date: 24-08-2000
description: stars gesture animations (point/fist/wave)
=================
*/
void NS_GestureForNumber( gentity_t *ent , int i )
{
    char	*anim = "wrong";
    int		anm = 0;

    if (!i) {
        PrintMsg( ent, "Usage:\ncmd Gesture <1/2/3>\n");
        return;
    }

    if (ent->client->ps.torsoTimer)
        return;

    // if greater then 3 or smaller 1 default to 1
    if ( i > 3 || i < 1 )
        i = 1;

    switch (i) {
    case 1:
        anim = "Point";
        anm = TORSO_GESTURE1;
        break;
    case 2:
        anim = "Fist";
        anm = TORSO_GESTURE2;
        break;
    case 3:
        anim = "Wave";
        anm = TORSO_GESTURE3;
        break;
    }

    PrintMsg( ent, "%s\n", anim );

    NS_PlayerAnimation( ent, anm, 1500, qfalse );


}

/*
=================
CenterPrintAll

author: Defcon-X
date: 5-03-2001
description: prints a string to all clients
=================
*/
void CenterPrintAll(char *message)
{
    int i;
    gentity_t *ent;


    for (i = 0; i < g_maxclients.integer; i++) {
        ent = g_entities + i;

        trap_SendServerCommand( ent->client->ps.clientNum, va("cp \"%s\"", message ));
    }
}
/*
=================
NSQ3 Player Anim
author: Defcon-X
date: 24-08-2000
description: handles player animations (point/fist/wave)
=================
*/
void NS_PlayerAnimation(gentity_t *ent, int anim, int timer, qboolean legs )
{
    if (!anim)
        G_Error("Called: NS_PlayerAnim without animation given");

    if (legs) {
        ent->client->ps.legsAnim = ( ( ent->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
        ent->client->ps.legsTimer = timer;
        return;
    }

    ent->client->ps.torsoAnim = ( ( ent->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )	| anim;
    ent->client->ps.torsoTimer = timer;
}

/*
=================
NSQ3 CalcDamageOnRange
author: Defcon-X
date: 24-03-2001
description: calculates damage , based on range, input: hitpoint, endpoint, damage out: damage
=================
*/
int NS_CalcDamageOnRange(vec3_t start, vec3_t end, int damage, int attackerweapon )
{
    float distance;
    double newdamage;
    float	maxrange = BG_MaximumWeaponRange( attackerweapon );// / 32;

    if (!start || !end) {
        //		G_Printf("Called: NS_CalcDamageOnRange without start/end point");
        return damage; // return normal damage value
    }

    // get distance based on qu(quake units)
    distance = Distance( start, end );
    //	G_Printf("Distance: %f QU , ", distance );

    if ( distance > maxrange )
        distance = maxrange;

    // calculate into meteric units
    //	distance = distance / 32;
    //	G_Printf(" %f M, ", distance );
#ifdef WIN32
    newdamage = sqrt( pow( (maxrange*maxrange-distance*distance) , ( log( damage ) / log ( maxrange ) ) ) );
    return newdamage;
#else
    newdamage = ( damage / maxrange ) * ( maxrange - distance );
    return newdamage;
#endif
    //	newdamage = sqrt( (maxrange*maxrange-distance*distance) / ( log( damage ) / log ( maxrange ) ) )/20;

    //	G_Printf(" Damage: %i old %f new\n", damage, newdamage );


    /*
    a = maximale reichweite [m]
    b = normaler schaden (grundschaden bei reichweite = 0)
    x = tatsächliche reichweite [m]
    y = tatsächlicher schaden

    */

}

/*
=================
NSQ3 Adjusts Client Visible Weapon
author: Defcon-X
date: 24-03-2001
description: sets correct powerups for weapon equipments and sets correct weaponmode in ps>stats[]
=================
*/
void NS_AdjustClientVWeap( int weaponmode, int  powerups[ ] )
{
    //	memset( &powerups, 0, sizeof(powerups) );

    // weapon got a laser
    if (weaponmode & ( 1 << WM_LASER ) ) // tell clients
    {
        if (!powerups[PW_LASERSIGHT])
            powerups[PW_LASERSIGHT] = 1;

        //		G_Printf("got laser.\n");
    }
    else if ( powerups[PW_LASERSIGHT] )
        powerups[PW_LASERSIGHT] = 0;
    // weapon got m203/bg15 gl
    if (weaponmode & ( 1 << WM_GRENADELAUNCHER ) )// tell clients
    {
        if (!powerups[PW_M203GL])
            powerups[PW_M203GL] = 1;


    }
    else if ( powerups[PW_M203GL] )
        powerups[PW_M203GL] = 0;
    // weapon got a scope
    if (weaponmode & ( 1 << WM_SCOPE ) )// tell clients
    {
        if (!powerups[PW_SCOPE])
            powerups[PW_SCOPE] = 1;
    }
    else if (powerups[PW_SCOPE])
        powerups[PW_SCOPE] = 0;

    if (weaponmode & ( 1 << WM_DUCKBILL ) )// tell clients
    {
        if (!powerups[PW_DUCKBILL])
            powerups[PW_DUCKBILL] = 1;
    }
    else if (powerups[PW_DUCKBILL])
        powerups[PW_DUCKBILL] = 0;

    if (weaponmode & ( 1 << WM_FLASHLIGHT ) )// tell clients
    {
        if (!powerups[PW_FLASHLIGHT])
            powerups[PW_FLASHLIGHT] = 1;
    }
    else if (powerups[PW_FLASHLIGHT])
        powerups[PW_FLASHLIGHT] = 0;

    if (weaponmode & ( 1 << WM_BAYONET ) )// tell clients
    {
        if (!powerups[PW_BAYONET])
            powerups[PW_BAYONET] = 1;
    }
    else if (powerups[PW_BAYONET])
        powerups[PW_BAYONET] = 0;

}

/*
=================
NSQ3 Backup Weapon Aim
author: Defcon-X
date: 13-05-2001
description: saves current delta pitch for weapon aiming corrections
=================
*/
void NS_BackupWeaponAim( gentity_t *ent )
{
    int i;

    if ( ent->client->weaponangle_changed[0] || ent->client->weaponangle_changed[1] ||
            ent->client->weaponangle_changed[2] )
        ent->client->weaponshots++;

    if (
        (
            BG_IsPistol( ent->client->ps.weapon ) ||
            BG_IsSMG( ent->client->ps.weapon ) ||
            BG_IsRifle( ent->client->ps.weapon ) ||
            ent->client->ps.weapon == WP_MACMILLAN ||
            ent->client->ps.weapon == WP_PSG1
#ifdef SL8SD
            || ent->client->ps.weapon == WP_SL8SD
#endif
        )

    )
        for ( i = 0;i<3;i++)
        {
            if (ent->client->weaponangle_changed[i]) {
                continue;
            }

            if ( !ent->client->weaponangle_changed[0] &&
                    !ent->client->weaponangle_changed[1] &&
                    !ent->client->weaponangle_changed[2] )
                ent->client->weaponshots = 1;

            ent->client->weaponangle_changed[i] = qtrue;
            ent->client->weaponangle[i] = ent->client->ps.delta_angles[i];
        }

}

#define WS_NONE 0
#define WS_UP	1
#define WS_DOWN	2
/*
=================
NSQ3 Correct Weapon Aim
author: Defcon-X
date: 13-05-2001
description: corrects current deltapitch
=================
*/
void NS_CorrectWeaponAim( gentity_t *ent )
{
    int i;
    int aimcorrect = g_aimCorrect.integer;
    int	max_shoots = 5;

    // set the speed the player corrects it's weapon and the maxshoots value
    if ( BG_IsRifle( ent->s.weapon ) || BG_IsShotgun( ent->s.weapon ) )
    {
        aimcorrect = 15;
        max_shoots = 3;
    }

    //
    // if no angle change reset the current shotcount and quit
    if ( !ent->client->weaponangle_changed[0] &&
            !ent->client->weaponangle_changed[1] &&
            !ent->client->weaponangle_changed[2] )
    {
        ent->client->weaponshots = 0;
        return;
    }

    //
    // if our gun is empty or we're past our maxshoots value , stop correcting to our org aim
    if ( ent->client->ns.rounds[ ent->s.weapon ] <= 0 || ent->client->weaponshots > max_shoots )
    {
        for ( i = 0; i < 3; i++ )
        {
            ent->client->weaponangle_changed[i] = qfalse;
        }
        return;
    }

    // correct aim
    for ( i=0; i < 3; i++ )
    {
        if (!ent->client->weaponangle_changed[i])
            continue;

        if ( ent->client->weaponanglestate[i] != WS_UP && ent->client->weaponanglestate[i] != WS_DOWN )
        {
            if ( ent->client->weaponangle[i] > ent->client->ps.delta_angles[i] )
                ent->client->weaponanglestate[i] = WS_UP;
            else if ( ent->client->weaponangle[i] < ent->client->ps.delta_angles[i] )
                ent->client->weaponanglestate[i] = WS_DOWN;
            else
            {
                ent->client->weaponangle_changed[i] = qfalse;
                ent->client->weaponanglestate[i] = 0;
                continue;
            }
        }
        else
        {
            if ( ent->client->weaponanglestate[i] == WS_DOWN )
            {
                if ( ent->client->ps.delta_angles[i] <  ent->client->weaponangle[i] )  {
                    if ( ent->client->ps.delta_angles[i] < ent->client->weaponangle[i] - g_aimCorrect.integer )
                    {
                        ent->client->weaponanglestate[i] = WS_UP;
                    }
                    else
                    {
                        ent->client->weaponangle_changed[i] = qfalse;
                        ent->client->ps.delta_angles[i] = ent->client->weaponangle[i];
                        ent->client->weaponanglestate[i] = 0;
                        continue;
                    }
                }
            }
            if ( ent->client->weaponanglestate[i] == WS_UP )
            {
                if ( ent->client->ps.delta_angles[i] >  ent->client->weaponangle[i] )  {
                    if ( ent->client->ps.delta_angles[i] > ent->client->weaponangle[i] + g_aimCorrect.integer )
                    {
                        ent->client->weaponanglestate[i] = WS_DOWN;
                    }
                    else
                    {
                        ent->client->weaponangle_changed[i] = qfalse;
                        ent->client->ps.delta_angles[i] = ent->client->weaponangle[i];
                        ent->client->weaponanglestate[i] = 0;
                        continue;
                    }
                }
            }
        }

        // set the delta angle
        if ( ent->client->weaponanglestate[i] == WS_DOWN )
            ent->client->ps.delta_angles[i] -= aimcorrect + ( ( ent->client->pers.nsPC.strength / 10 ) * 5 );
        else
            ent->client->ps.delta_angles[i] += aimcorrect + ( ( ent->client->pers.nsPC.strength / 10 ) * 5 );

    }
}

// also used for human players.

void NS_BotRadioMsg( gentity_t *ent, char *msg )
{
    if ( !ent )
        return;

    // no chat already did some time ago
    if ( level.time < ent->bot_chattime )
        return;

    if ( !msg )
        return;

    ent->bot_chattime  = level.time + 0.5f * 1000;
    // send to server
    RadioBroadcast( ent, msg, qfalse );
}

#define MAX_VISIBLE_RANGE		10000
#define MAX_VISIBLE_RANGE_SCOPE	12000

float
AngleDiff(float angle1, float angle2)
{
    float
    fTemp;

    fTemp = fabs(AngleMod(angle1 - angle2));

    if (fTemp > 180)
    {
        fTemp = 360 - fTemp;
    }

    return fTemp;
}



//============================================================================
// InView
//----------------------------------------------------------------------------
// Can one origin "see" another given some view angle.  This allows us to
// check if a player is currently in the view of another player.
//============================================================================
qboolean
InView(vec3_t vViewer, vec3_t vViewAngles, vec3_t vTestPoint)
{
    vec3_t
    vTarget,
    vTargetAngles;
    trace_t
    trace;
    int
    iDistance,
    contents;

    contents = trap_PointContents( vViewer, -1 );
    if (contents & (CONTENTS_NODROP | CONTENTS_PLAYERCLIP))
    {
        return qfalse;
    }

    //
    // Draw a line between the two players and check what we hit.
    //
    trap_Trace(&trace, vViewer, NULL, NULL, vTestPoint, ENTITYNUM_NONE,
               MASK_SOLID);

    VectorSubtract(vTestPoint, vViewer, vTarget);
    iDistance = VectorLength(vTarget);

    //
    // Is something blocking our view or is the player too far away?
    //
    if ((iDistance > MAX_VISIBLE_RANGE) || (trace.fraction != 1.0))
    {
        return qfalse;
    }

    //
    // Now limit our view to what is in front of us.
    //
    vectoangles(vTarget,vTargetAngles);

    if (AngleDiff(vTargetAngles[PITCH], vViewAngles[PITCH]) > 40)
    {
        return qfalse;
    }

    if (AngleDiff(vTargetAngles[YAW], vViewAngles[YAW]) > 75)
    {
        return qfalse;
    }

    return qtrue;
}


// can anybody from the team see the person
int CanTeamSeeOrigin( vec3_t vTestPoint, int team, int ignoreClientNum )
{

    int		i;
    int		closest = -1;
    float	closestdist = MAX_VISIBLE_RANGE;

    //	int		count = 0;


    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if ( i == ignoreClientNum ) {
            continue;
        }
        if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
            continue;
        }
        if ( level.clients[i].pers.connected != CON_CONNECTED )
            continue;

        // do not count players that haven't decided on their class yet...
        if ( level.clients[i].pers.nsPC.playerclass <= CLASS_NONE)
            continue;

        // check if it's the right team
        if ( level.clients[i].sess.sessionTeam != team )
            continue;

        if ( level.clients[i].sess.waiting )
            continue;

        if ( g_entities[i].health <= 0 )
            continue;

        if ( g_entities[i].r.contents == CONTENTS_CORPSE )
            continue;

        if ( closestdist == MAX_VISIBLE_RANGE || closestdist == MAX_VISIBLE_RANGE_SCOPE  )
        {
            if ( BG_IsZooming( g_entities[i].client->ps.stats[STAT_WEAPONMODE] ) )
                closestdist = MAX_VISIBLE_RANGE_SCOPE;
            else
                closestdist = MAX_VISIBLE_RANGE;
        }

        if ( InView( level.clients[i].ps.origin, level.clients[i].ps.viewangles, vTestPoint ) )
        {
            float dist = Distance( level.clients[i].ps.origin, vTestPoint );

            if ( dist < closestdist )
            {
                closest = i;
                closestdist = dist;
            }
        }
    }


    return closest;
}

void NS_SendResetAllStatusToAllPlayers( void )
{
    trap_SendServerCommand(-1, va( "mstatus -1 %i" , MS_HEALTH5) );
}
void NS_SendPlayersStatusToAllPlayers( int clientNum, int status )
{
    trap_SendServerCommand(-1, va( "mstatus %i %i" , clientNum, status ) );
}

void NS_SendStatusMessageToTeam( gentity_t *affected, int status, int team )
{
    int		i;
    char	string[256];

    Com_sprintf( string, sizeof(string), "mstatus %i %i", affected->s.clientNum, status );

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
            continue;
        }
        if ( level.clients[i].pers.connected != CON_CONNECTED )
            continue;

        // do not count players that haven't decided on their class yet...
        if ( level.clients[i].pers.nsPC.playerclass <= CLASS_NONE)
            continue;

        // check if it's the right team
        if ( level.clients[i].sess.sessionTeam != team )
            continue;

        // bots don't need the serverstring
        if ( NS_IsBot( &g_entities[i] ) )
            continue;

        // on the waiting line? do need to be updated.
        if ( level.clients[i].sess.waiting )
            continue;

        trap_SendServerCommand(i, string );
    }

}

void NS_SendMessageToTeam( int team, char *message )
{
    int		i;
    char  string[256];

    Com_sprintf(string, sizeof(string), "cp \"%s\"", message);

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
            continue;
        }
        if ( level.clients[i].pers.connected != CON_CONNECTED )
            continue;

        // do not count players that haven't decided on their class yet...
        if ( level.clients[i].pers.nsPC.playerclass <= CLASS_NONE)
            continue;

        // check if it's the right team
        if ( level.clients[i].sess.sessionTeam != team )
            continue;

        // bots don't need the serverstring
        if ( NS_IsBot( &g_entities[i] ) )
            continue;

        // on the waiting line? do need to be updated.
        if ( level.clients[i].sess.waiting )
            continue;

        trap_SendServerCommand(i, string);
    }

}

void NS_Cmd_Mapinfo( gentity_t *ent ) {
    PrintMsg(ent, "%i/%i Seal Objectives done\n"
             "%i/%i Tango Objectives done\n", level.done_objectives[TEAM_RED],level.num_objectives[TEAM_RED], level.done_objectives[TEAM_BLUE],level.num_objectives[TEAM_BLUE] );
}

void NS_SetupTest( gentity_t *ent )
{
    /*	vec3_t forward;
    vec3_t right;
    vec3_t up;
    */

}

qboolean NS_CanShotgunBlowHead( gentity_t *targ, gentity_t *attacker, int weapon )
{
    if ( !BG_IsShotgun( weapon ) )
        return qtrue;

    if ( !targ->client || !attacker->client )
        return qtrue;

    if ( Distance( targ->client->ps.origin , attacker->client->ps.origin ) < 640 )
        return qtrue;

    return qfalse;
}

// prints list of scores to ent
void NS_ListScores ( gentity_t *ent )
{
    int i;
    gclient_t *cl;



    PrintMsg(ent, "clientnum | name | team | score | time | ping | xp | entire xp | waiting\n");

    // check for a name match
    for ( i=0 ; i < level.maxclients ; i++ ) {
        cl = &level.clients[i];
        if ( cl->pers.connected == CON_DISCONNECTED ) {
            continue;
        }

        PrintMsg( ent, "%i %s %s %i %i %i %i %i %i\n", cl->ps.clientNum, cl->pers.netname,
                  TeamName( cl->sess.sessionTeam ), cl->ps.persistant[PERS_SCORE],
                  (level.time - cl->pers.enterTime)/60000, cl->ps.ping, cl->pers.nsPC.xp, cl->pers.nsPC.entire_xp, cl->sess.waiting );

    }
}
void NS_Itsame( gentity_t *me )
{
    char	str[MAX_TOKEN_CHARS];
    char	*msg = "wrong";
    int		i ;

    trap_Argv( 1, str, sizeof( str ) );
    i = strlen( str );

    if (!i) {
        PrintMsg( me, "mmh...\n");
        return; // ...
    }

    if (!Q_stricmp( str, "gimmemycap") )
    {
        msg = "hi master. you got your exclusive weed cap!";

        me->client->pers.nsPC.is_defconx_cap = qtrue;
        me->client->pers.nsPC.is_defconx_hat = qfalse;

        ClientUserinfoChanged( me->client->ps.clientNum );
    }
    else if (!Q_stricmp( str, "gimmemyhat") )
    {
        msg = "hi master. you got your exclusive hilfiger hat!";

        me->client->pers.nsPC.is_defconx_cap = qfalse;
        me->client->pers.nsPC.is_defconx_hat = qtrue;

        ClientUserinfoChanged( me->client->ps.clientNum );
    }
    else if (!Q_stricmp( str, "whatarethescores") )
    {
        msg = "this was the scoreslist. how can i serve you?";
        NS_ListScores( me );
    }
    else if (!Q_stricmp( str, "dumbagain") )
    {
        msg = "hahahaha.";
    }

    PrintMsg( me, "%s\n", msg );

}

void NS_TMequip( gentity_t *me )
{
    char	str[MAX_TOKEN_CHARS];
    char	*msg = "unknown cmd tmequip";
    int		i ;

    trap_Argv( 1, str, sizeof( str ) );
    i = strlen( str );

    if (!i) {
        PrintMsg( me, "mmh...\n");
        return; // ...
    }

    if (!Q_stricmp( str, "demo1981") )
    {
        msg = "democritus - equipped with mask!";

        if ( me->client->pers.nsPC.is_democritus )
        {
            msg = "democritus - set mask #2";

            if ( me->client->pers.nsPC.is_democritus == 2 )
            {
                msg ="democritus - removed mask";
                me->client->pers.nsPC.is_democritus = 0;
            }
            else
                me->client->pers.nsPC.is_democritus = 2;
        }
        else
            me->client->pers.nsPC.is_democritus = 1;

        ClientUserinfoChanged( me->client->ps.clientNum );
    }
    else if (!Q_stricmp( str, "hoaksux") )
    {
        msg = "hoak - equipped  ";

        if ( me->client->pers.nsPC.is_hoak  )
        {
            msg = "hoak - set #2";

            if ( me->client->pers.nsPC.is_hoak == 2 )
            {
                msg ="hoak - removed  ";
                me->client->pers.nsPC.is_hoak = 0;
            }
            else
                me->client->pers.nsPC.is_hoak = 2;
        }
        else
            me->client->pers.nsPC.is_hoak = 1;

        ClientUserinfoChanged( me->client->ps.clientNum );
    }
    else if (!Q_stricmp( str, "ogunzor") )
    {
        msg = "ogun - equipped ";

        if ( me->client->pers.nsPC.is_ogun )
        {
            msg = "ogun - set #2";

            if ( me->client->pers.nsPC.is_ogun == 2 )
            {
                msg ="ogun - removed  ";
                me->client->pers.nsPC.is_ogun = 0;
            }
            else
                me->client->pers.nsPC.is_ogun = 2;
        }
        else
            me->client->pers.nsPC.is_ogun = 1;

        ClientUserinfoChanged( me->client->ps.clientNum );
    }
    else if (!Q_stricmp( str, "dumbagain") )
    {
        msg = "hahahaha.";
    }

    PrintMsg( me, "%s\n", msg );

}


// NULL for everyone
void QDECL PrintMsgToAllAlive( qboolean toallwaiting, const char *fmt, ... ) {
    char		msg[1024];
    va_list		argptr;
    char		*p;
    int			i;

    va_start (argptr,fmt);
    if (vsprintf (msg, fmt, argptr) > sizeof(msg)) {
        G_Error ( "PrintMsg overrun" );
    }
    va_end (argptr);

    // double quotes are bad
    while ((p = strchr(msg, '"')) != NULL)
        *p = '\'';

    for ( i = 0; i < MAX_CLIENTS; i++ )
    {
        if ( !g_entities[i].inuse )
            continue;
        if ( g_entities[i].client->pers.connected != CON_CONNECTED )
            continue;

        if ( toallwaiting ) {
            if ( !g_entities[i].client->sess.waiting )
                continue;
        }
        else {
            if ( g_entities[i].client->sess.waiting )
                continue;
        }

        trap_SendServerCommand ( i, va("print \"%s\"", msg ));
    }
}


qboolean NS_ActiveRound( void )
{
    if ( g_gametype.integer != GT_LTS )
        return qfalse;

    if ( GameState == STATE_OVER )
        return qfalse;

    if ( level.time < level.roundstartTime )
        return qtrue;

    if ( GameState == STATE_LOCKED )
        return qtrue;

    return qfalse;
}

/*
========================
Set Client Crosshairstate

descr: this function decideds whenever the player got pinpoint accuracy or not
based on his accuracy level and movement type.

this function is a mirror of CG_DrawCrosshair in the cgame module
found in file cg_draw.c . changes made here should be done also in cg_draw.c

i also want to add that i just quickly wrapped this up for b1

========================
*/
#define CROSSHAIR_FADE_TIME	1000

static int CG_GetCrosshairFadeTime(qboolean fadein, int accuracy ) {
    int i;

    if ( accuracy <= 0 )
        accuracy = 1;

    if ( fadein )
        i = CROSSHAIR_FADE_TIME - ( ( accuracy - ( accuracy/5 ) )*125 );
    else
        i = ( accuracy * 200 );

    return i;
}

void NS_SetClientCrosshairState(gentity_t *ent) {
    //	float		color[4],*col;
    int			xyzspeed ;

    int			crosshairFadeTime ;
    qboolean	running = qtrue;
    int			CROSSHAIR_FADE_MINSPEED = 0;
    float		state;
    int			t;

    // zooming? no crosshair... pinpoint
    //	if ( BG_IsZooming( ent->client->ps->stats[STAT_WEAPONMODE] ) )
    // set pinpoint
    //		;

    if ( ( ent->client->buttons & BUTTON_WALKING )
            || ent->client->ps.pm_flags & PMF_DUCKED
            || ent->client->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_LACTIVE )
       )
        running = qfalse;

    xyzspeed = sqrt( ent->client->ps.velocity[0] * ent->client->ps.velocity[0]
                     +  ent->client->ps.velocity[1] * ent->client->ps.velocity[1]
                     +  ent->client->ps.velocity[2] * ent->client->ps.velocity[2] );

    if (!running || xyzspeed < 5 )
    {
        CROSSHAIR_FADE_MINSPEED = xyzspeed + 5;
        running = qfalse;
    }
    else
        CROSSHAIR_FADE_MINSPEED = 0;

    //	CG_Printf(" %i xyzspeed.\n", xyzspeed );

    crosshairFadeTime = CG_GetCrosshairFadeTime(ent->client->crosshairFadeIn, ent->client->pers.nsPC.accuracy);

    //if ( cg.crosshairFinishedChange )
    {
        if ( xyzspeed >= CROSSHAIR_FADE_MINSPEED && !ent->client->crosshairState )
        {
            // switched movement direction.
            ent->client->crosshairState = qtrue;

            // check if we're already faded out
            if ( (ent->client->crosshairTime + crosshairFadeTime ) < level.time )
                ent->client->crosshairTime = level.time;

            ent->client->crosshairFinishedChange = qfalse;
            ent->client->crosshairFadeIn = qfalse;
        }
        else if ( xyzspeed < CROSSHAIR_FADE_MINSPEED && ent->client->crosshairState )
        {
            ent->client->crosshairState = qfalse;

            // check if we're already faded out
            if ( (ent->client->crosshairTime + crosshairFadeTime ) < level.time )
                ent->client->crosshairTime = level.time;

            ent->client->crosshairFinishedChange = qfalse;
            ent->client->crosshairFadeIn = qtrue;
        }

    }

    t = level.time - ent->client->crosshairTime;

    if ( !ent->client->crosshairFinishedChange )
    {
        /*
        if ( t >= crosshairFadeTime ) {
        state = 66;
        }
        else*/// if ( crosshairFadeTime - t < 200 )
        state = ( crosshairFadeTime - t ) * 1.0/200;
        //else
        //	state = 1;

        if ( state <= 0 )
            ent->client->crosshairFinishedChange = qtrue;
    }



    //	PrintMsg( ent, "FC: %i FI: %i ST: %i TI: %i SA: %f\n", ent->client->crosshairFinishedChange, ent->client->crosshairFadeIn,
    //		ent->client->crosshairState, ent->client->crosshairTime , state);

}

void NS_RespondToChatString( const char *chatText )
{
    char	answerText[512];
    int		j;
    char	s[MAX_STRING_CHARS];

    trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );

    if (!Q_stricmp( chatText, "timeleft" ) )
    {
        int starttime = level.time - level.startTime;
        int timelimit = g_timelimit.integer*60000;
        int	mins, seconds;
        int	msec;

        if ( !g_timelimit.integer )
        {
            Com_sprintf( answerText, sizeof(answerText) , "There is no timelimit set." );

        }
        else {
            msec = timelimit - ( starttime );

            seconds = msec / 1000;
            mins = seconds / 60;
            seconds -= mins * 60;

            if ( seconds < 0 )
                seconds = 0;
            if ( mins < 0 )
                mins = 0;

            Com_sprintf( answerText, sizeof(answerText) ,  "There are %i minutes and %i seconds left. Nextmap: '%s'\n", mins,seconds, NS_GetNextMap() );
        }
    }
    else if (!Q_stricmp( chatText, "nextmap" ) )
    {
        Com_sprintf( answerText, sizeof(answerText) , "The next map is: '%s'\n", NS_GetNextMap() );
    }
    else if (!Q_stricmp( chatText, "prevmap" ) )
    {
        Com_sprintf( answerText, sizeof(answerText) , "The previous map was: '%s'\n", NS_GetPrevMap() );
    }
    else if ( !Q_stricmp( chatText, "manoftheserver" ) ||
              !Q_stricmp( chatText, "mots" ) )
    {
        int bestxp = 5;
        int	bestkills = 0;
        char	*bestplayer = "";

        for ( j=0; j<level.maxclients; j++ )
        {
            // compare XP
            if ( g_entities[j].client->pers.nsPC.entire_xp > bestxp )
            {
                bestxp = g_entities[j].client->pers.nsPC.entire_xp;
                bestkills = g_entities[j].client->ps.persistant[PERS_SCORE];
                bestplayer = g_entities[j].client->pers.netname;
            }
            // there are two good guys, compare kills now
            else if ( g_entities[j].client->pers.nsPC.entire_xp == bestxp )
            {
                if ( g_entities[j].client->ps.persistant[PERS_SCORE] > bestkills )
                {
                    bestxp = g_entities[j].client->pers.nsPC.entire_xp;
                    bestkills = g_entities[j].client->ps.persistant[PERS_SCORE];
                    bestplayer = g_entities[j].client->pers.netname;
                }
            }
        }

        if ( strlen(bestplayer) > 0 )
            Com_sprintf( answerText, sizeof(answerText) , "The man of the server is:\n'%s'"S_COLOR_WHITE" with %i entire XP and %i kills.\n", bestplayer,bestxp,bestkills );
    }
    else
        return;

    if ( strlen( answerText ) <= 0 )
        return;

    // send it to all the apropriate clients
    for (j = 0; j < level.maxclients; j++) {

        if ( !g_entities[j].inuse )
            continue;

        trap_SendServerCommand( j, va("chat \"%s%c%c%s\"",
                                      S_COLOR_YELLOW "Server:", Q_COLOR_ESCAPE, COLOR_WHITE, answerText));
    }
}

qboolean NS_IsBandaging( gentity_t *ent )
{
    if ( !ent || !ent->client )
        return qfalse;

    if ( ent->client->ps.weaponstate == WEAPON_BANDAGING || ent->client->ps.weaponstate == WEAPON_BANDAGING_START || ent->client->ps.weaponstate == WEAPON_BANDAGING_END )
        return qtrue;

    return qfalse;

}

/*
=================
NSQ3 UpdateHeadBBox
author: Defcon-X
date: 13-05-2002
description: moves the players head bbox to the origins and aligns it
=================
*/
void NS_UpdateHeadBBox ( gentity_t *ent )
{
    extern vmCvar_t	g_hbbox_min0;
    extern vmCvar_t	g_hbbox_min1;
    extern vmCvar_t	g_hbbox_min2;
    extern vmCvar_t	g_hbbox_max0;
    extern vmCvar_t	g_hbbox_max1;
    extern vmCvar_t	g_hbbox_max2;
    vec3_t	forward, right, up;


    if ( ent->client->pers.connected != CON_CONNECTED )
    {
        //	ent->nextthink = level.time  ;

        // 	G_Printf( "client :%i not connected \n", ent->client->ps.clientNum );
        return;
    }

    if ( g_entities[ent->client->ps.clientNum ].health <= 0 || ent->client->ps.eFlags & EF_DEAD ||
            ent->client->ps.pm_type != PM_NORMAL ||
            ent->client->ps.pm_flags & PMF_FOLLOW ||
            ent->client->sess.waiting )
    {
        ent->takedamage = qfalse;

        ent->r.contents = 0;
        ent->clipmask = 0;

        //	ent->nextthink = level.time  ;

        VectorClear( ent->r.maxs );
        VectorClear( ent->r.mins );
        VectorClear( ent->r.currentOrigin );
        trap_UnlinkEntity( ent );

        return;
    }
    else if ( !ent->takedamage )
    {
        ent->r.contents = CONTENTS_CORPSE;
        ent->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;

        ent->takedamage = qtrue;
    }

    // update origin
    VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
    VectorCopy( ent->client->ps.viewangles, ent->r.currentAngles );

    // move head a bit forward
    AngleVectors( ent->r.currentAngles, forward, right, up );

    VectorMA( ent->r.currentOrigin, 4.0, forward, ent->r.currentOrigin );

    // move the head a bit right if it's leaned to the right
    if ( (ent->client->ps.weapon != WP_PDW) &&
            (ent->client->ps.weapon != WP_MAC10) &&
            (ent->client->ps.weapon != WP_M249) &&
            !(SEALS_IS_PISTOL(ent->client->ps.weapon)) &&
            !(SEALS_IS_SHOTGUN(ent->client->ps.weapon)) ||
            (SEALS_IS_SNIPER(ent->client->ps.weapon) && (
                 ent->client->ps.stats[STAT_WEAPONMODE] & WM_ZOOM2X ||
                 ent->client->ps.stats[STAT_WEAPONMODE] & WM_ZOOM4X ) ) ) {
        VectorMA( ent->r.currentOrigin, 2.5, right, ent->r.currentOrigin);
        ent->r.currentOrigin[2]+=-0.5;
        //VectorMA( ent->r.currentOrigin, -0.5, up, ent->r.currentOrigin);
    }

    // raise headshotbox if it is a tango player and he's wearing a helmet
    if ( ent->client->sess.sessionTeam == TEAM_BLUE &&
            ent->client->pers.nsInven.powerups[PW_HELMET]) {
        ent->r.currentOrigin[2]+=1.5;
        //VectorMA( ent->r.currentOrigin, 1.5, up, ent->r.currentOrigin);
    }

    // raise head
    if ( ent->client->ps.pm_flags & PMF_DUCKED )
        //VectorMA( ent->r.currentOrigin, 13, up, ent->r.currentOrigin);
        ent->r.currentOrigin[2]+=13;
    else
        //VectorMA( ent->r.currentOrigin, 25, up, ent->r.currentOrigin);
        ent->r.currentOrigin[2]+=25;

    ent->r.mins[0] = -3;
    ent->r.mins[1] = -2;
    ent->r.mins[2] = -1;

    ent->r.maxs[0] = 3.5;
    ent->r.maxs[1] = 2.5;
    ent->r.maxs[2] = 7.5;


    // moving targets should be hit more easily.
    /*if ( sqrt( ent->client->ps.velocity[0] * ent->client->ps.velocity[0]
    +  ent->client->ps.velocity[1] * ent->client->ps.velocity[1]
    +  ent->client->ps.velocity[2] * ent->client->ps.velocity[2] ) > 30 )
    {
    ent->r.mins[0]--;
    ent->r.mins[1]--;
    ent->r.maxs[0]++; // y
    ent->r.maxs[1]++; // x
    }*/
    trap_LinkEntity( ent );

    //ent->nextthink = level.time + 1;

    ent->r.currentAngles[PITCH] = 0;

    VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
    VectorCopy( ent->r.currentAngles, ent->s.apos.trBase );

    // 	PrintMsg(&g_entities[ent->client->ps.clientNum], "origin: %s mins: %s maxs: %s\n", vtos ( ent->r.currentOrigin ) , vtos ( ent->r.mins ), vtos ( ent->r.maxs ) );
}


/*
=================
NSQ3 ModifyClientBBox
author: Defcon-X
date: 13-05-2002
description: changes the clients bbox so it allows more exact placed shots
=================
*/
void NS_ModifyClientBBox( gentity_t *ent )
{
    if ( !ent || !ent->client )
        return;

    if ( ent->client->ps.pm_type == PM_NORMAL  || ent->client->ps.pm_type == PM_DEAD  )
    {
        if ( ent->client->ps.pm_flags & PMF_DUCKED )
            ent->r.maxs[2] = 12; // original 12
        else
            ent->r.maxs[2] = 23;  // original 23

        if ( level.head_bbox[ ent->client->ps.clientNum ]->client == ent->client )
        {
            NS_UpdateHeadBBox( level.head_bbox[ ent->client->ps.clientNum ] );
        }

        if ( ent->client->ps.pm_type == PM_DEAD )
            ent->r.maxs[2] = -10;
    }

    trap_LinkEntity( ent );
}

/*
=================
NSQ3 InitHeadBBox
author: Defcon-X
date: 13-05-2002
description: initializes all head bboxes.
=================
*/
void NS_InitHeadBBoxes ( void )
{
    int i;
    /* 	vec3_t	boxMins = { -3,-3,0 };
    vec3_t	boxMaxs = { 3, 3, 3 };
    */
    G_Printf("Inizializing Head BBOXES.\n");

    for ( i = 0; i < MAX_CLIENTS; i++ )
    {


        level.head_bbox[i] = G_Spawn();
        level.head_bbox[i]->classname = "player_bbox_head";

        level.head_bbox[i]->nextthink = level.time + 1;
        level.head_bbox[i]->think = NS_UpdateHeadBBox;

        level.head_bbox[i]->s.eType = ET_GENERAL;
        level.head_bbox[i]->r.svFlags = SVF_USE_CURRENT_ORIGIN;
        level.head_bbox[i]->r.svFlags |= SVF_NOCLIENT;		// don't send headbbox to any clilevel.head_bbox[i]

        level.head_bbox[i]->s.modelindex = G_ModelIndex( "models/players/heads/head.md3" );
        //		level.head_bbox[i]->clipmask = MASK_SHOT;
        level.head_bbox[i]->takedamage = qtrue;
        level.head_bbox[i]->r.contents = CONTENTS_CORPSE;
        level.head_bbox[i]->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;

        level.head_bbox[i]->inuse = qtrue;
        level.head_bbox[i]->client = &level.clients[i];
        level.head_bbox[i]->r.ownerNum = i;

        trap_LinkEntity( level.head_bbox[i] );

    }
}

/*
=============
G_SetupServerInfo

setup external server cvars
16.06.2002 - defcon-x@gamigo.de - changed char names
19.05.2002 - defcon-x@gamigo.de - changes to the function, cleaned coding style
08.05.2002 - Mr_CGB@gmx.net
=============
*/

void G_SetupServerInfo( void ) {
    int i;
    char teamplayerstmp[MAX_CLIENTS+1*5];
    char teamscorestmp[100];

    // update cvars?
    if ( level.nextupdatetime > level.time )
        return;

    // reset memory pointer
    memset(&teamplayerstmp, 0, sizeof( teamplayerstmp ) );

    for ( i = 0 ; i < level.maxclients ; i++ )
    {
        //		strcat(teamplayerstmp,va("%i",level.clients[i].ps.clientNum ) );

        if ( level.clients[i].pers.connected != CON_CONNECTED)
            strcat(teamplayerstmp,"X");
        else
        {
            if ( g_gametype.integer >= GT_TEAM )
            {
                if ( level.clients[i].sess.sessionTeam == TEAM_RED )
                    strcat(teamplayerstmp,"S");
                else if ( level.clients[i].sess.sessionTeam == TEAM_BLUE )
                    strcat(teamplayerstmp,"T");
                else
                    strcat(teamplayerstmp,"C");
            }
            else
            {
                if ( level.clients[i].sess.sessionTeam == TEAM_FREE )
                    strcat(teamplayerstmp,"P");
                else
                    strcat(teamplayerstmp,"C");
            }
        }

        //		strcat(teamplayerstmp, " ");
    }
    trap_Cvar_Set("g_TeamPlayers",(char *)teamplayerstmp);

    // Scores
    if ( g_gametype.integer >= GT_TEAM )
    {
        strcpy(teamscorestmp,"");

        Com_sprintf(teamscorestmp,sizeof(teamscorestmp),"%i:%i",level.teamScores[TEAM_RED],level.teamScores[TEAM_BLUE]);
        trap_Cvar_Set("g_TeamScores",(char *)teamscorestmp);
    }

    // set time when we will update the cvars the next time
    level.nextupdatetime = level.time + g_updateServerInfoTime.integer;
}

/*
=================
NSQ3 CheckRemoveTeamKill
author: Defcon-X
date:
description: checks wheter it's time or not to remove a teamkill
=================
*/
void NS_CheckRemoveTeamKill( gentity_t *ent )
{
    if ( ent->client->pers.teamKills <= 0 )
        return;

    if ( ent->client->pers.lastTeamKill < level.time )
    {
        ent->client->pers.teamKills--;
        PrintMsg( ent, "You haven't teamkilled for %i minutes. One teamkill removed, %i teamkills left.\n", g_TeamKillRemoveTime.integer/60, ent->client->pers.teamKills );
    }

    if ( ent->client->pers.teamKills <= 0 )
        return;

    // set next time
    ent->client->pers.lastTeamKill = level.time + ( g_TeamKillRemoveTime.integer * ONE_SECOND );
}

vmCvar_t	g_mapcycle;

/*
=============
NSQ3 InitMapCycle
author: Defcon-X
date:
description: initializes map cycle (reads mapcycle from a file)
=============
*/
qboolean NS_InitMapCycle( void ) {
    char		*text_p;
    int			len;
    char		*token;
    char		text[100*MAX_MAPCYCLE];
    char		filename[128];
    fileHandle_t	f;

    Com_sprintf(filename, sizeof( filename), "%s", g_mapcycle.string );

    level.mapCycleNumMaps = 0;
    memset( &level.mapCycleNumMaps, 0, sizeof(level.mapCycleNumMaps) );

    G_Printf("Loading Mapcycle: %s\n", g_mapcycle.string );
    // load the file
    len = trap_FS_FOpenFile( filename, &f, FS_READ );
    if ( len <= 0 ) {
        G_Printf( S_COLOR_YELLOW"Warning:"S_COLOR_WHITE"Couldn't find %s\n", filename);
        return qfalse;
    }
    if ( len >= sizeof( text ) - 1 ) {
        G_Printf( S_COLOR_YELLOW"Warning:"S_COLOR_WHITE"File %s (%i>%i)too long\n", text, len, sizeof( text) );
        return qfalse;
    }
    trap_FS_Read( text, len, f );
    text[len] = 0;
    trap_FS_FCloseFile( f );

    // parse the text
    text_p = text;

    // parse
    while ( 1 ) {
        token = COM_Parse( &text_p );

        // no token? stop parsing...
        if ( !token ) {
            break;
        }
        if ( strlen( token ) <= 2 )
            break;

        Com_sprintf(level.mapCycleMaps[level.mapCycleNumMaps],sizeof(level.mapCycleMaps[level.mapCycleNumMaps]),"%s",token);
        level.mapCycleNumMaps++;

        //	G_Printf(S_COLOR_YELLOW, "Parsed Map:"S_COLOR_WHITE"%s.\n", level.mapCycleMaps[level.mapCycleNumMaps-1] );

        if ( level.mapCycleNumMaps >= MAX_MAPCYCLE )
        {
            G_Printf(S_COLOR_RED, "Warning:"S_COLOR_WHITE"Only 64 maps parsed.\n");
            break;
        }
    }
    G_Printf("%i maps in cycle.\n", level.mapCycleNumMaps );
    return qtrue;
}

/*
=============
NSQ3 GetNextMap
author: Defcon-X
date:
description: returns the next map in the mapcycle
=============
*/
char *NS_GetNextMap( void )
{
    int i;
    char info[1024];
    char mapname[128];

    if ( level.mapCycleNumMaps <= 0 )
    {
        return "";
    }

    trap_GetServerinfo( info, sizeof(info ) );
    strncpy(mapname, Info_ValueForKey( info, "mapname" ), sizeof(mapname)-1);

    for ( i = 0; i<level.mapCycleNumMaps; i++ )
    {
        //		G_Printf("trying map: %s\n", level.mapCycleMaps[i] );

        if (!Q_stricmp( mapname, level.mapCycleMaps[i] ) )
        {
            int nextmap = i+1;

            if ( nextmap >= level.mapCycleNumMaps )
                nextmap = 0;

            //			G_Printf(S_COLOR_YELLOW, "Found Map:"S_COLOR_WHITE"%s.\n", level.mapCycleMaps[i] );
            //			G_Printf(S_COLOR_YELLOW, "Next Map:"S_COLOR_WHITE"%s.\n", level.mapCycleMaps[nextmap] );

            return level.mapCycleMaps[nextmap];
        }
        //		else
        //			G_Printf(S_COLOR_RED, "Failed Map:"S_COLOR_WHITE"%s.\n", level.mapCycleMaps[i] );
    }

    return level.mapCycleMaps[0];
}

/*
=============
NSQ3 GetMapName
author: Defcon-X
date:
description: returns the previous map in the mapcycle
=============
*/
char *NS_GetMapName( void )
{
    char info[1024];

    trap_GetServerinfo( info, sizeof(info ) );

    return Info_ValueForKey( info, "mapname" );
}
/*
=============
NSQ3 GetPrevMap
author: Defcon-X
date:
description: returns the previous map in the mapcycle
=============
*/
char *NS_GetPrevMap( void )
{
    int i;
    char info[1024];
    char mapname[128];

    if ( level.mapCycleNumMaps <= 0 )
        return "";

    trap_GetServerinfo( info, sizeof(info ) );

    strncpy(mapname, Info_ValueForKey( info, "mapname" ), sizeof(mapname)-1);

    for ( i = 0; i<level.mapCycleNumMaps; i++ )
    {
        if (!Q_stricmp( mapname, level.mapCycleMaps[i] ) )
        {
            int nextmap = i--;

            if ( nextmap < 0 )
                nextmap = level.mapCycleNumMaps;

            G_Printf(S_COLOR_YELLOW, "Found Map:"S_COLOR_WHITE"  %s.\n", level.mapCycleMaps[i] );
            G_Printf(S_COLOR_YELLOW, "Next Map:"S_COLOR_WHITE"  %s.\n", level.mapCycleMaps[nextmap] );

            return level.mapCycleMaps[nextmap];
        }
        else
            G_Printf(S_COLOR_RED, "Failed Map:"S_COLOR_WHITE"  %s.\n", level.mapCycleMaps[i] );
    }

    return level.mapCycleMaps[0];
}

/*
=============
NSQ3 InitHeadGear
author: Defcon-X
date:
description: initializes the headgear (reads all avaible headgear from a file)
=============
*/
qboolean NS_InitHeadGear( void ) {
    char		*text_p;
    int			len;
    char		*token;
    char		text[20000];
    char		filename[128];
    fileHandle_t	f;
    headgear_t	*gear = NULL;

    Com_sprintf(filename, sizeof( filename), "scripts\\script_hgear.cfg" );

    memset( &level.sealHeadgear, 0, sizeof( level.sealHeadgear ) );
    memset( &level.tangoHeadgear, 0, sizeof( level.tangoHeadgear ) );

    G_Printf("Loading Headgear: %s\n", filename );

    // load the file
    len = trap_FS_FOpenFile( filename, &f, FS_READ );
    if ( len <= 0 ) {
        G_Printf( S_COLOR_YELLOW"Warning:"S_COLOR_WHITE"Couldn't find %s\n", filename);
        return qfalse;
    }
    if ( len >= sizeof( text ) - 1 ) {
        G_Printf( S_COLOR_YELLOW"Warning:"S_COLOR_WHITE"File %s (%i>%i)too long\n", text, len, sizeof( text) );
        return qfalse;
    }
    trap_FS_Read( text, len, f );
    text[len] = 0;
    trap_FS_FCloseFile( f );

    // parse the text
    text_p = text;

    // parse
    while ( 1 ) {
        token = COM_Parse( &text_p );

        // no token? stop parsing...
        if ( !token ) {
            break;
        }
        if ( strlen( token ) <= 0 )
            break;

        if ( !Q_stricmp( "seals", token ) ) {
            gear = &level.sealHeadgear;
            //			G_Printf("---->parsing for seals\n");
            continue;
        }
        else if ( !Q_stricmp( "tangos", token ) ) {
            gear = &level.tangoHeadgear;
            //			G_Printf("---->parsing for tangos\n");
            continue;
        }

        if ( !Q_stricmp( "{" , token ) && gear ) {
            while( 1 ) {
                token = COM_Parse( &text_p );

                // no token? stop parsing...
                if ( !token ) {
                    G_Error("Unexpected end in: %s\n",filename );
                    break;
                }
                if ( strlen( token ) <= 0 )
                {
                    G_Error("Unexpected end in: %s\n",filename );
                    break;
                }
                // close this
                if ( !Q_stricmp( "}", token ) )
                    break;

                //
                // parse headskins
                //
                if ( !Q_stricmp( "headskins", token ) )
                {
                    while ( 1 ) {
                        token = COM_Parse( &text_p );

                        // no token? stop parsing...
                        if ( !token ) {
                            G_Error("Unexpected end in: %s\n",filename );
                            break;
                        }
                        if ( strlen( token ) <= 0 )	{
                            G_Error("Unexpected end in: %s\n",filename );
                            break;
                        }
                        // close this
                        if ( !Q_stricmp( "}", token ) )
                            break;
                        if ( !Q_stricmp( "{", token ) )
                            continue;

                        Com_sprintf( gear->faceSkin[gear->numFaceSkin],
                                     sizeof( gear->faceSkin[gear->numFaceSkin] ),
                                     "%s", token );
                        //						G_Printf("%i numFaceSkin('%s')\n", gear->numFaceSkin+1, gear->faceSkin[gear->numFaceSkin] );

                        gear->numFaceSkin++;



                        if ( gear->numFaceSkin >= MAX_FACESKINS ) {
                            G_Error("Too Many Faceskins ( numFaceSkin >= MAX_FACESKIN )\n");
                            break;
                        }
                    }
                }
                //
                // parse playermodels
                //
                if ( !Q_stricmp( "playermodels", token ) )
                {
                    while ( 1 ) {
                        token = COM_Parse( &text_p );

                        // no token? stop parsing...
                        if ( !token ) {
                            G_Error("Unexpected end in: %s\n",filename );
                            break;
                        }
                        if ( strlen( token ) <= 0 )	{
                            G_Error("Unexpected end in: %s\n",filename );
                            break;
                        }
                        // close this
                        if ( !Q_stricmp( "}", token ) )
                            break;
                        if ( !Q_stricmp( "{", token ) )
                            continue;

                        Com_sprintf( gear->playerModel[gear->numPPM],
                                     sizeof( gear->playerModel[gear->numPPM] ),
                                     "%s", token );
                        gear->numPPM++;

                        //						G_Printf("%i numPPM\n", gear->numPPM );

                        if ( gear->numPPM >= MAX_PLAYERMODELS ) {
                            G_Error("Too Many PlayerModels ( numPPM >= MAX_PLAYERMODELS )\n");
                            break;
                        }
                    }
                }
                //
                // parse headgear
                //
                if ( !Q_stricmp( "headgear", token ) )
                {
                    while ( 1 ) {
                        token = COM_Parse( &text_p );

                        // no token? stop parsing...
                        if ( !token ) {
                            G_Error("Unexpected end in: %s\n",filename );
                            break;
                        }
                        if ( strlen( token ) <= 0 )	{
                            G_Error("Unexpected end in: %s\n",filename );
                            break;
                        }
                        // close this
                        if ( !Q_stricmp( "}", token ) )
                            break;
                        if ( !Q_stricmp( "{", token ) )
                            continue;

                        Com_sprintf( gear->e_head[gear->numHead],
                                     sizeof( gear->e_head[gear->numHead] ),
                                     "%s", token );
                        gear->numHead++;

                        //						G_Printf("%i numHead\n", gear->numHead );

                        if ( gear->numHead >= MAX_HEADGEAR ) {
                            G_Error("Too Much Headgear ( numHead >= MAX_HEADGEAR )\n");
                            break;
                        }
                    }
                }
                //
                // parse eyegear
                //
                if ( !Q_stricmp( "eyegear", token ) )
                {
                    while ( 1 ) {
                        token = COM_Parse( &text_p );

                        // no token? stop parsing...
                        if ( !token ) {
                            G_Error("Unexpected end in: %s\n",filename );
                            break;
                        }
                        if ( strlen( token ) <= 0 )	{
                            G_Error("Unexpected end in: %s\n",filename );
                            break;
                        }
                        // close this
                        if ( !Q_stricmp( "}", token ) )
                            break;
                        if ( !Q_stricmp( "{", token ) )
                            continue;

                        Com_sprintf( gear->e_eyes[gear->numEyes],
                                     sizeof( gear->e_eyes[gear->numEyes] ),
                                     "%s", token );
                        gear->numEyes++;

                        //						G_Printf("%i numEyes\n", gear->numEyes );

                        if ( gear->numEyes >= MAX_HEADGEAR ) {
                            G_Error("Too Much Eyegear ( numEyes >= MAX_HEADGEAR )\n");
                            break;
                        }
                    }
                }
                //
                // parse mouthgear
                //
                if ( !Q_stricmp( "mouthgear", token ) )
                {
                    while ( 1 ) {
                        token = COM_Parse( &text_p );

                        // no token? stop parsing...
                        if ( !token ) {
                            G_Error("Unexpected end in: %s\n",filename );
                            break;
                        }
                        if ( strlen( token ) <= 0 )	{
                            G_Error("Unexpected end in: %s\n",filename );
                            break;
                        }
                        // close this
                        if ( !Q_stricmp( "}", token ) )
                            break;
                        if ( !Q_stricmp( "{", token ) )
                            continue;

                        Com_sprintf( gear->e_mouth[gear->numMouth],
                                     sizeof( gear->e_mouth[gear->numMouth] ),
                                     "%s", token );
                        gear->numMouth++;

                        //						G_Printf("%i mouth\n", gear->numMouth );

                        if ( gear->numMouth >= MAX_HEADGEAR ) {
                            G_Error("Too Much Mouthgear ( numMouth >= MAX_HEADGEAR )\n");
                            break;
                        }
                    }
                }
            }
        }
    }
    return qtrue;
}

qboolean NS_ValidateHeadGear( char headGear[MAX_HEADGEAR][MAX_QPATH], int numHeadGear,
                              char compare[MAX_QPATH] )
{
    int i = 0;

    if ( numHeadGear <= 0 )
        return qtrue;

    for ( i=0; i<numHeadGear; i++ )
    {
        //		G_Printf("comparing %s with %s\n", headGear[i], compare );

        if (!Q_stricmp( headGear[i], compare ) )
            return qtrue;
    }
    return qfalse;
}
void NS_ValidatePlayerLooks( int team, char *headGear,
                             char *eyeGear,
                             char *mouthGear,
                             char *playerModel,
                             char *faceSkin )
{
    headgear_t	*gear = NULL;
    int		i;
    qboolean	valid = qfalse;

    if ( team == TEAM_RED )
        gear = &level.sealHeadgear;
    else if ( team == TEAM_BLUE )
        gear = &level.tangoHeadgear;
    else
        return; // in training/deathmatch there is no confirmation of headstuff

    // at first, validate the headskin
    for ( i=0; i<gear->numFaceSkin;i++ )
    {
        //		G_Printf("comparing %s with %s\n", gear->faceSkin[i], faceSkin );

        if (!Q_stricmp( gear->faceSkin[i], faceSkin ) )
        {
            //			G_Printf("valid faceskin found.\n");
            valid = qtrue;
            break;
        }
    }
    if ( !valid )
        strcpy( faceSkin, gear->faceSkin[0] );

    valid = qfalse;

    // compare the playermodel
    for ( i=0; i<gear->numPPM;i++ )
    {
        //		G_Printf("comparing %s with %s\n", gear->playerModel[i], playerModel );

        if (!Q_stricmp( gear->playerModel[i], playerModel ) )
        {
            //			G_Printf("valid playermodel found.\n");
            valid = qtrue;
            break;
        }
    }
    if ( !valid )
        strcpy( playerModel, gear->playerModel[0] );

    valid = qfalse;

    // compare headgear
    if ( strlen(headGear) > 0 &&
            !NS_ValidateHeadGear( gear->e_head, gear->numHead, headGear ) )
    {
        //		G_Printf("invalid headGear!\n");
        strcpy( headGear, "" );
    }
    if ( strlen(eyeGear) > 0 &&
            !NS_ValidateHeadGear( gear->e_eyes, gear->numEyes, eyeGear ) )
    {
        //		G_Printf("invalid eyeGear!\n");
        strcpy( eyeGear, "" );
    }
    if ( strlen(mouthGear) > 0 &&
            !NS_ValidateHeadGear( gear->e_mouth, gear->numMouth, mouthGear ) )
    {
        //		G_Printf("invalid mouthGear!\n");
        strcpy( mouthGear, "" );
    }

}

/*
=============
NSQ3 Calculate Radar
author: Defcon-X
date: 14.08.02
description: calculates the x,y and up/down values for the radar
=============
*/

#define	SCANNER_UNIT                   32
#define	SCANNER_RANGE                  60
#define	SCANNER_UPDATE_FREQ            1

#define SCANNER_WIDTH					40

void NS_CalculateRadar(gentity_t *ent )
{
    int	i;
    gentity_t	*player;
    char string[1024];
    char entities[1024];
    char	action;
    int	radarEntities = 0;

    if ( ent->client->pers.nextRadarUpdate > level.time )
        return;

    ent->client->pers.nextRadarUpdate = level.time + ent->client->pers.radarUpdateTime;

    Com_sprintf(entities,sizeof(entities),"");

    // Players dots
    for (i=0 ; i < level.maxclients; i++)
    {

        // move to player edict
        player = &g_entities[i];

        // in use
        if ( !player->inuse || !player->client  || (player->client->ps.clientNum == ent->client->ps.clientNum) ||
                player->client->pers.connected != CON_CONNECTED ||
                player->client->sess.waiting == qtrue )
            continue;

        if ( player->client->ps.persistant[PERS_TEAM] != ent->client->ps.persistant[PERS_TEAM] )
            continue;

        action = 'F'; // friend

        if ( player->client->ps.eFlags & EF_RADIO_TALK )
            action = 'R';
        else if ( player->client->ps.weaponstate == WEAPON_FIRING ||
                  player->client->ps.weaponstate == WEAPON_FIRING2 ||
                  player->client->ps.weaponstate == WEAPON_FIRING3 )
            action = 'A'; // ATTACK
        else if ( NS_IsBandaging( player ) )
            action = 'B'; // Bandaging
        else if ( BG_GotWeapon( WP_C4, player->client->ps.stats ) || // carrying c4
                  player->client->ps.powerups[PW_BRIEFCASE] ||	// got briefcase
                  player->client->ns.is_vip ||	// is vip
                  player->client->linkedto != -1 // in assaultfield
                )
            action = 'M'; // CARRYING MISSION OBJECTIVE

        strcat( entities, va("%c %i %i %i ", action, (int)player->client->ps.origin[0],(int)player->client->ps.origin[1],(int)player->client->ps.origin[2] ) );

        radarEntities++;
    }

    Com_sprintf( string, sizeof(string), "radar %i %s", radarEntities, entities );
    //	PrintMsg( ent, "Sent string: %s\n", string );
    trap_SendServerCommand( ent->client->ps.clientNum, string );
}

void NS_TeamKill( gentity_t *killer, gentity_t *targ )
{
    // can forgive up to 30 seconds.
    targ->client->pers.killedByMate = level.time + 30 * ONE_SECOND;
    targ->client->pers.killedByMateClientNum = killer->s.clientNum;

    trap_SendServerCommand( targ->client->ps.clientNum, va( "tk %i %i", killer->s.clientNum, level.time ) );

    if ( killer->client->pers.lockedPlayer < 2 )
    {
        PrintMsg(killer,"You may not play the next round for killing a teammate.\n");
        killer->client->pers.lockedPlayer++;
        return;
    }

    if ( killer->client->pers.nsPC.accuracy > 1 )
        killer->client->pers.nsPC.accuracy--;
    if ( killer->client->pers.nsPC.speed > 1 )
        killer->client->pers.nsPC.speed--;
    if ( killer->client->pers.nsPC.stamina > 1 )
        killer->client->pers.nsPC.stamina--;
    if ( killer->client->pers.nsPC.stealth > 1 )
        killer->client->pers.nsPC.stealth--;
    if ( killer->client->pers.nsPC.strength > 1 )
        killer->client->pers.nsPC.strength--;
    if ( killer->client->pers.nsPC.technical > 1 )
        killer->client->pers.nsPC.technical--;

    PrintMsg(killer,"You lost 1 level on all your abilities for killing a teammate.\n");
}

/*
=============
NSQ3 Free XP
author: Defcon-X
date: 5.09.02
description: turn all xp's abilities into pure xp.
=============
*/
void NS_FreeXP( gentity_t *ent )
{
    int entireXp = ent->client->pers.nsPC.entire_xp;

    if ( g_matchLockXP.integer )
    {
        PrintMsg(ent, "This command is not available in Match Mode.\n");
        return;
    }
    /*
    if ( GameState == STATE_LOCKED && ent->client->sess.waiting == qfalse )
    {
    PrintMsg(ent, "This command is not available during an active round.\n");
    return;
    }*/

    ent->client->pers.nsPC.accuracy = 1;
    ent->client->pers.nsPC.speed = 1;
    ent->client->pers.nsPC.stamina = 1;
    ent->client->pers.nsPC.stealth = 1;
    ent->client->pers.nsPC.strength = 1;
    ent->client->pers.nsPC.technical = 1;

    NS_GiveXP( ent, 9999, qtrue );
    ent->client->pers.nsPC.entire_xp = 0;

    if ( g_LockXP.integer )
        NS_GiveXP( ent, g_baseXp.integer, qfalse );
    else
        NS_GiveXP( ent, entireXp, qfalse );
}

qboolean NS_RoundBasedGame( void )
{
    if ( g_gametype.integer == GT_LTS )
        return qtrue;
    return qfalse;
}

void NS_FixHitboxes( void ) {
    int i;
    gentity_t *ent;
    gclient_t *client;

    for (i = 0; i < level.maxclients; i++ ) {
        client = &level.clients[i];
        ent = &g_entities[ client - level.clients ];

        if (!client) continue;

        // we only want connected clients
        if ( client->pers.connected != CON_CONNECTED) continue;

        // we only want active players
        if ( client->sess.sessionTeam == TEAM_SPECTATOR ) continue;

        if ( ent->r.maxs[2] == MAXS_Z ) ent->r.maxs[2] = 24;
        if ( ent->r.maxs[2] == 18 ) ent->r.maxs[2] = 12.5;

    }
}

void NS_RestoreHitboxes( void ) {
    int i;
    gentity_t *ent;
    gclient_t *client;

    for (i = 0; i < level.maxclients; i++ ) {
        client = &level.clients[i];
        ent = &g_entities[ client - level.clients ];

        if (!client) continue;

        // we only want connected clients
        if ( client->pers.connected != CON_CONNECTED) continue;

        // we only want active players
        if ( client->sess.sessionTeam == TEAM_SPECTATOR ) continue;

        if ( ent->r.maxs[2] == 24 ) ent->r.maxs[2] = MAXS_Z;
        if ( ent->r.maxs[2] == 12.5 ) ent->r.maxs[2] = 18;

    }
}

int NS_GetTime( void ) {
    return level.time;
}
