// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_weapon.c
// perform the server side effects of a weapon firing

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation

// defcon-x@ns-co.net
#include "g_local.h"

static	float	s_quadFactor;
static	vec3_t	forward, right, up;
static	vec3_t	muzzle;
static  qboolean wshotty = qfalse;

#define NUM_NAILSHOTS 10
gentity_t *fire_reallead (gentity_t *self, vec3_t start,vec3_t dir, int weapon, int caliber, int damage, float muzzlevelocity);
gentity_t *fire_realshotgun (gentity_t *self, vec3_t start,vec3_t dir, int weapon, int caliber, int damage, float muzzlevelocity);

void NS_BotRadioMsg ( gentity_t *ent, char *msg );
/*
================
G_KnockPlayer
================
*/
void G_KnockPlayer( gentity_t *ent, vec3_t dir, float value )
{
    vec3_t	kvel;

    if ( !ent->client )
        return;

    // reverse
    VectorScale (dir, value, kvel);
    VectorAdd (ent->client->ps.velocity, kvel, ent->client->ps.velocity);

    // set the timer so that the other client can't cancel
    // out the movement immediately
    if ( !ent->client->ps.pm_time ) {
        int		t;

        t = value * 0.75f;

        ent->client->ps.pm_time = t;
        ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
    }
}
/*
================
G_BounceProjectile
================
*/
void G_BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout ) {
    vec3_t v, newv;
    float dot;


    // check to see if it's a 180° ricochet
    VectorSubtract( start, impact , v );

    VectorNormalize( v );
    dot = DotProduct (v, dir);

    // prevent 180° ricochets
    if( dot >= 0.75 )
        return;

    VectorSubtract( impact, start, v );
    dot = DotProduct( v, dir );

    VectorMA( v, -2*dot, dir, newv );

    VectorNormalize(newv);
    VectorMA(impact, 8192, newv, endout);
}


/*
===============
CheckMeleeAttack
===============
*/
#define KNIFE_RANGE		25
#define BAYONET_RANGE	25

#define MELEE_DAMAGE	10

qboolean CheckMeleeAttack( gentity_t *ent , qboolean longrange ) {
    trace_t		tr;
    vec3_t		end;
    gentity_t	*tent;
    gentity_t	*traceEnt;
    int			damage;
    vec3_t		origin;
    int			oldHealth;

    int			weapontime = ent->client->ps.weaponTime;
    int			maxweapontime = 380;
    int			weaponstate = ent->client->ps.weaponstate;
    float		t;

    if ( longrange )
        maxweapontime = 400;

    t = 1.0f - (float)weapontime/maxweapontime;

    // set aiming directions
    AngleVectors (ent->client->ps.viewangles, forward, right, up);

    VectorCopy( ent->client->ps.origin , origin );
    origin[2] += ent->client->ps.viewheight;

    //	CalcMuzzlePoint ( ent, forward, right, up, muzzle );

    if ( longrange )
        VectorMA ( origin, BAYONET_RANGE + (12.5 * t), forward, end);
    else
        VectorMA ( origin, KNIFE_RANGE + (5*t), forward, end);


    if ( !longrange )
    {
        if ( weaponstate == WEAPON_FIRING || weaponstate == WEAPON_FIRING3 )
        {
            VectorMA (end, 1 - ( 10 * t), right, end);
            VectorMA (end, 1 - ( 8 * t), up, end);
        } else if ( weaponstate == WEAPON_FIRING2 )
        {
            VectorMA (end, -4 + ( 10 * t), right, end);
            VectorMA (end, 1 - ( 8 * t), up, end);
        }
    }

    // BLUTENGEL: introduced hitbox fix
    NS_FixHitboxes();
    trap_Trace (&tr, origin, NULL, NULL, end, ent->s.number, MASK_SHOT );
    NS_RestoreHitboxes();

    if ( tr.surfaceFlags & SURF_NOIMPACT ) {
        return qfalse;
    }

    traceEnt = &g_entities[ tr.entityNum ];

    damage = MELEE_DAMAGE;



    // G_Printf("wt: %i mwt: %i ws: %i t: %f fr: %f\n", weapontime,maxweapontime,weaponstate,t, tr.fraction );


    if ( !traceEnt->client && tr.fraction < 1.0f/* && traceEnt*/ )
    {
        //	G_Printf("wallspawn!\n");
        /*
        tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_WALL );

        tent->s.eventParm = DirToByte( tr.plane.normal );
        tent->s.otherEntityNum = ent->s.number;
        tent->s.weapon = WP_MK23;
        */
        return qfalse;
    }
    else if ( traceEnt->client && tr.endpos )
    {
        if ( traceEnt->takedamage && traceEnt->client && random() < 0.3 )
        {
            tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
            tent->s.otherEntityNum = traceEnt->s.number;
            tent->s.eventParm = DirToByte( tr.plane.normal );
            tent->s.weapon = ent->s.weapon;
        }
    }

    if ( !traceEnt->takedamage) {
        return qfalse;
    }

    // fix for the new hitdetection system.
    if ( traceEnt->client )
        oldHealth = g_entities[ traceEnt->client->ps.clientNum ].health;
    else
        oldHealth = traceEnt->health;

    if ( oldHealth <= 0 )
        return qfalse;

    if ( pointinback( traceEnt, ent->client->ps.origin ) )
        damage *= 2;

    G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, 0, MOD_GAUNTLET );



    return qtrue;
}


/*
======================
SnapVectorTowards

Round a vector to integers for more efficient network
transmission, but make sure that it rounds towards a given point
rather than blindly truncating.  This prevents it from truncating
into a wall.
======================
*/
void SnapVectorTowards( vec3_t v, vec3_t to ) {
    int		i;

    for ( i = 0 ; i < 3 ; i++ ) {
        if ( to[i] <= v[i] ) {
            v[i] = (int)v[i];
        } else {
            v[i] = (int)v[i] + 1;
        }
    }
}


/*
======================================================================

SHOTGUN

======================================================================
*/

// DEFAULT_SHOTGUN_SPREAD and DEFAULT_SHOTGUN_COUNT	are in bg_public.h, because
// client predicts same spreads
#define	DEFAULT_SHOTGUN_DAMAGE	10

int ShotgunPellet( vec3_t start, vec3_t end, gentity_t *ent, int damage ) {
    trace_t		tr;
    int			d_damage;
    gentity_t		*traceEnt;

    NS_FixHitboxes();
    trap_Trace (&tr, start, NULL, NULL, end, ent->s.number, MASK_SHOT);
    NS_RestoreHitboxes();
    traceEnt = &g_entities[ tr.entityNum ];

    // send bullet impact
    if (  tr.surfaceFlags & SURF_NOIMPACT ) {
        return qfalse;
    }

    if ( traceEnt->takedamage) {
        d_damage = damage * s_quadFactor;

        G_Damage( traceEnt, ent, ent, forward, tr.endpos,
                  d_damage, 0, MOD_SHOTGUN);
        if( LogAccuracyHit( traceEnt, ent ) ) {
            if ( tr.entityNum < MAX_CLIENTS )
                return tr.entityNum;
            else
                return -5;
        }
    }
    // didn't hit anything at all
    return -5;
}

#define DUCKBILL_HOR_MOD	3
#define DUCKBILL_VERT_DIV	1

// this should match CG_ShotgunPattern
void ShotgunPattern( vec3_t origin, int seed, gentity_t *ent, int pellets, int damage, int spread  ) {
    int			i;
    float		r, u;
    vec3_t		end;
    vec3_t		forward, right, up;
    int			maxrange = BG_MaximumWeaponRange( ent->s.weapon );

    // derive the right and up vectors from the forward vector, because
    // the client won't have any other information
    AngleVectors( ent->client->ps.viewangles, forward,right,up );


    // generate the "random" spread pattern
    for ( i = 0 ; i < pellets ; i++ ) {
        vec3_t dir;

        if ( ent->client->ns.weaponmode[ent->s.weapon] & ( 1 << WM_DUCKBILL ) )
        {
            r = Q_crandom( &seed ) * (spread*DUCKBILL_HOR_MOD);
            u = Q_crandom( &seed ) * (spread/DUCKBILL_VERT_DIV);
        }
        else {
            r = Q_crandom( &seed ) * spread;
            u = Q_crandom( &seed ) * spread;
        }
        VectorMA( origin, maxrange, forward, end);
        VectorMA (end, r, right, end);
        VectorMA (end, u, up, end);

        VectorSubtract( end,origin,  dir );
        fire_realshotgun( ent, origin, dir, ent->s.weapon, AM_SHOTGUNMAG, 6+(-2+random()*3), g_shotgunleedvelocity.value );
    }
}


void Weapon_Shotgun (gentity_t *ent, int caliber, int damageperpellet, int pellets )
{
    int i = 0;
    extern vmCvar_t	g_test;

    ent->client->ns.rounds[ent->s.weapon]--;
    // workaround for shotgun recoil
    // wshotty is only true, when the recoil of the shotgun has
    // been calculated already
    wshotty = qfalse;
    for ( i=0; i<pellets; i++ )
    {
        Fire_Lead( ent, caliber, damageperpellet );
        wshotty = qtrue;
    }
    wshotty = qfalse;

    // only do shotgun knockback when not crouching
    /*	if (!(ent->client->ps.pm_flags & PMF_DUCKED) && sqrt( ent->client->ps.velocity[2] * ent->client->ps.velocity[2]) < 5)
    {
    vec3_t	kvel;
    float	mass;
    // get it from the strength of the shotgun
    float knockback = damageperpellet * pellets / 25.0;
    float	final_knockback;
    mass = 200 + ent->client->pers.nsPC.strength * 15;
    final_knockback = ( ( 1200 * knockback ) / mass );

    // reverse
    forward[0] *= -1;
    forward[1] *= -1;
    forward[2] *= -1;

    VectorScale (forward, final_knockback, kvel);
    VectorAdd (ent->client->ps.velocity, kvel, ent->client->ps.velocity);

    // set the timer so that the other client can't cancel
    // out the movement immediately
    if ( !ent->client->ps.pm_time ) {
    int		t;

    t = knockback * 1.5;

    ent->client->ps.pm_time = t;
    ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
    }
    }
    */
}

///////////////////////////////////////////////////////////////////////////////////////
//																				     //
// the bombcode																		 //
//
// author: defcon-x
// description: entire bombcode gameplay
//
///////////////////////////////////////////////////////////////////////////////////////
gentity_t *NS_RandomPlayer( int ignoreClientNum, team_t team );

static int lastbmbplayer = -1;

qboolean NS_GiveBombsToTeam( team_t team ) {
    gentity_t *bmbplayer;

    if ( g_overrideGoals.integer )
        return qtrue;

    if ( team != TEAM_RED && team != TEAM_BLUE ) {
        G_Error("NS_GiveBombsToTeam. Unallowed team.\n");
        return qtrue;
    }

    if (level.bombs[team] <= 0) return qtrue;

    bmbplayer = NS_RandomPlayer(lastbmbplayer, team);

    if ( bmbplayer) {
        // give the player the bomb
        BG_PackWeapon( WP_C4, bmbplayer->client->ps.stats );
        // notify everyone via radio message about the bomb
        NS_BotRadioMsg( bmbplayer, "bgot" );
        // remember this player
        lastbmbplayer = bmbplayer->s.clientNum;

        G_LogPrintf("OBJECTIVE: [%i] \"%s\" got the C4\n",
                    bmbplayer->s.clientNum, bmbplayer->client->pers.netname);
    } else {
        return qfalse;
    }

    return qtrue;
}


void bomb_detonate ( gentity_t *ent ) {
    gentity_t *temp;

    G_UseTargets(ent->target_ent, ent->parent );

    temp = G_TempEntity(ent->s.pos.trBase, EV_EXPLOSION );

    temp->s.legsAnim = 1;

    G_Printf("BOOOM %i\n", ent->bot_chattime);
    if (g_gametype.integer == GT_LTS ) {
        level.done_objectives[ent->ns_team]+=ent->bot_chattime;

        // so the seals will definitivly win the round
        if ( level.roundstartTime < level.time )
        {
            trap_LinkEntity( ent );
            if (level.done_objectives[ent->ns_team] >= level.num_objectives[ent->ns_team]) {
                NS_EndRoundForTeam( ent->ns_team );
            } else {
                if (ent->ns_team == TEAM_RED) NS_EndRoundForTeam(TEAM_BLUE);
                else NS_EndRoundForTeam(TEAM_RED);
            }
        }
    }
    ent->r.svFlags |= SVF_NOCLIENT;
    ent->nextthink = level.time + 5000;
    ent->think = G_FreeEntity;

    trap_LinkEntity(ent);
}

void bomb_explode_instantly ( gentity_t *ent ) {

    //	G_Printf("bombexplode: %s netname\n", ent->parent->client->pers.netname );
    ent->s.loopSound = 0;

    // got a parent
    if (ent->parent)
    {
        // reward him for placing the bomb
        ent->parent->client->ns.rewards |= REWARD_BOMB_EXPLODE;
    }

    bomb_detonate( ent );

    // remove c4
    trap_SetConfigstring( CS_BOMB_START_TIME, "0" );

}
void bomb_explode ( gentity_t *ent ) {

    //	G_Printf("bombexplode: %s netname\n", ent->parent->client->pers.netname );
    ent->s.loopSound = 0;

    G_AddEvent( ent, EV_GENERAL_SOUND,  G_SoundIndex( "sound/weapons/c4/bomb-detonate.wav" ) );

    ent->nextthink = level.time + ( 3 * ONE_SECOND );
    ent->think = bomb_detonate ;

    // remove c4
    trap_SetConfigstring( CS_BOMB_START_TIME, "0" );
}

int bomb_getDiffTime( gentity_t *ent , int defender_tec )
{	// defender = person who planted bomb
    // attacker = person who disarms bomb
    int attacker_tec = ent->client->pers.nsPC.technical;
    int diff = defender_tec - attacker_tec;

    if ( diff ==  -9 )
        return level.time + 1000;
    else if ( diff ==  -8 )
        return level.time + 1500;
    else if ( diff ==  -7 )
        return level.time + 2000;
    else if ( diff ==  -6 )
        return level.time + 2000;
    else if ( diff ==  -5 )
        return level.time + 2500;
    else if ( diff ==  -4 )
        return level.time + 2700;
    else if ( diff ==  -3 )
        return level.time + 3000;
    else if ( diff ==  -2 )
        return level.time + 3000;
    else if ( diff ==  -1 )
        return level.time + 4000;
    else if ( diff ==  0 )
        return level.time + 4500;
    else if ( diff ==  1 )
        return level.time + 5000;
    else if ( diff ==  2 )
        return level.time + 6000;
    else if ( diff ==  3 )
        return level.time + 6000;
    else if ( diff ==  4 )
        return level.time + 6000;
    else if ( diff ==  5 )
        return level.time + 6000;
    else if ( diff ==  6 )
        return level.time + 7000;
    else if ( diff ==  7 )
        return level.time + 7000;
    else if ( diff ==  8 )
        return level.time + 7000;
    else if ( diff ==  9 )
        return level.time + 8000;

    return level.time + 1000;
}

int bomb_getwireTime( gentity_t *ent )
{
    int technical = ent->client->pers.nsPC.technical;
    int time;
    int addTime;

    /*
    Here are the times;
    Techincal  1 - no defusing possible
    Technical  2 :  10 seconds per wire
    Technical  3 :   9 seconds per wire
    Technical  4 :   8 seconds per wire
    Technical  5 : 7.5 seconds per wire
    Technical  6 :   7 seconds per wire
    Technical  7 : 6.5 seconds per wire
    Technical  8 :   6 seconds per wire
    Technical  9 : 5.5 seconds per wire
    Technical 10 :   5 seconds per wire
    */
    switch ( technical )
    {
    case 1:
        addTime = 15000;
        break;
    case 2:
        addTime = 10000;
        break;
    case 3:
        addTime = 9000;
        break;
    case 4:
        addTime = 8000;
        break;
    case 5:
        addTime = 7500;
        break;
    case 6:
        addTime = 7000;
        break;
    case 7:
        addTime = 6500;
        break;
    case 8:
        addTime = 6000;
        break;
    case 9:
        addTime = 5500;
        break;
    case 10:
        addTime = 5000;
        break;
    default:
        addTime = 20000;
        break;
    }

    time = level.time + addTime;

    return time;
}
void bomb_changewirestate( gentity_t *ent, int wire, int state )
{
    if ( !ent || !wire )
        return;

    ent->client->ns.bomb_wires[wire-1] = state;
    trap_SendServerCommand( ent-g_entities, va("bombwire %i %i", wire, state ) );
}
int bomb_onewireleft( gentity_t *ent )
{
    // defuse wire
    int i;
    int count = 0;

    for ( i =0;i<8;i++)
    {
        if ( ent->client->ns.bomb_wires[i] == 0 )
            count++;
    }

    return count;

}
int bomb_getdefusablewire( gentity_t *ent )
{
    int i;
    gentity_t *bomb = ent->client->ns.bomb_world;

    // loop through all wires
    for ( i = random()*7; i < 8 ; i++ )
    {
        // this is the defuse wire
        if ( i == bomb->health - 1 )
            continue;
        if ( ent->client->ns.bomb_wires[i] != 0 ) // already been defused
            continue;

        return i;
    }
    return -1;
}

void bomb_checkremovewire( gentity_t *ent )
{
    int wire = ent->client->ns.bomb_world->health;
    int defuseWire = random()*7;

    if ( g_gametype.integer < GT_TEAM )
        return;
    if ( !ent || !ent->client )
        return;
    if ( !(ent->client->ps.pm_flags & PMF_BOMBCASE ) )
        return;
    if ( level.time < ent->client->ns.bomb_wireTime || !ent->client->ns.bomb_wireTime )
        return;
    if ( bomb_onewireleft(ent) == 0 )
        return;

    // get a new wire that can be defused
    defuseWire = bomb_getdefusablewire( ent );

    if( defuseWire < 0 )
        return;

    bomb_changewirestate( ent, defuseWire+1, -1 );
    ent->client->ns.bomb_wireTime = bomb_getDiffTime( ent, ent->client->ns.bomb_world->damage );

    if ( bomb_onewireleft(ent) == 1 )
    {
        bomb_changewirestate( ent, wire, 1 );
        ent->client->ns.bomb_wireTime = 0;
    }
}

//
// handles what happens if a player tries to drop the bomb.
//

void bomb_drop( gentity_t *ent )
{
    gentity_t *bomb = ent->client->ns.bomb_world;

    //  _  _   _  _   _  _
    //  \\// _ \\// _ \\//
    //  //\\   //\\   //\\

    if ( !bomb || !ent->client )
        return;

    // remove flag
    ent->client->ps.pm_flags &= ~PMF_BOMBCASE;

    bomb->count = 0;
    ent->client->ns.bomb_world = 0;

    // remove weapon bombcase
    BG_RemoveWeapon( WP_C4, ent->client->ps.stats );

}

//
// trying to defuse.
//
void bomb_touch (gentity_t *ent, gentity_t *other, trace_t *trace )
{
    int technical = 0;

    if (!other->client)
        return;

    if ( !(other->client->buttons & BUTTON_USE) )
        return;

    technical = other->client->pers.nsPC.technical;

    // this means that the bomb is already beeing defused by another player.
    if ( ent->count )
        return;

    if ( ent->ns_team == other->client->sess.sessionTeam )
    {
        if ( (level.time - other->client->pers.lmt) > 1000) {
            PrintMsg( other , "This is bomb that your team placed.\n");
            other->client->pers.lmt = level.time;
        }
        return;
    }

    if ( ! (ent->ns_flags & NS_FLAG_DEFUSABLE ) ) {
        if ( (level.time - other->client->pers.lmt) > 1000) {
            PrintMsg( other ,S_COLOR_WHITE"This bomb can "S_COLOR_RED"NOT"S_COLOR_WHITE" be defused!\n");
            other->client->pers.lmt = level.time;
        }
        return;
    }

    if ( !other->client->ns.got_defusekit )
    {
        if ( (level.time - other->client->pers.lmt) > 1000) {
            PrintMsg( other , "You need to have at least "S_COLOR_RED"TECH 2"S_COLOR_WHITE" at the beginning of the round to defuse a bomb.\n");
            other->client->pers.lmt = level.time;
        }
        return;
    }

    other->client->ps.pm_flags |= PMF_BOMBCASE;
    BG_PackWeapon( WP_C4, other->client->ps.stats );
    other->client->ps.weapon = other->s.weapon = WP_C4;

    // ent->r.svFlags |= SVF_NOCLIENT;

    other->client->ns.bomb_world = ent;
    other->client->ps.weaponstate = WEAPON_RAISING;
    other->client->ps.weaponTime = 500;
    other->client->ns.bomb_wireTime = bomb_getDiffTime( other, ent->damage );

    ent->count = 1;


}


void bomb_use( gentity_t *ent, gentity_t *other, gentity_t *activator )
{
    int technical = activator->client->pers.nsPC.technical;

    if (!activator->client)
        return;
    // this means that the bomb is already beeing defused by another player.
    if ( ent->count )
        return;

    if ( ent->ns_team == activator->client->sess.sessionTeam )
    {
        if ( (level.time - activator->client->pers.lmt) > 1000) {
            PrintMsg( activator , "This is team bomb that your team placed.\n");
            activator->client->pers.lmt = level.time;
        }
        return;
    }

    if ( ! (ent->ns_flags & NS_FLAG_DEFUSABLE ) ) {
        if ( (level.time - activator->client->pers.lmt) > 1000) {
            PrintMsg( activator , "This bomb can"S_COLOR_RED"not"S_COLOR_WHITE" be defused!\n");
            activator->client->pers.lmt = level.time;
        }
        return;
    }

    if ( technical < 2)
    {
        if ( (level.time - activator->client->pers.lmt) > 1000) {
            PrintMsg( activator , "You need to have at least "S_COLOR_RED"TECH 2"S_COLOR_WHITE" at the beginning of the round to defuse a bomb.\n");
            activator->client->pers.lmt = level.time;
        }
        return;
    }

    activator->client->ps.pm_flags |= PMF_BOMBCASE;
    BG_PackWeapon( WP_C4, activator->client->ps.stats );
    activator->client->ps.weapon = activator->s.weapon = WP_C4;

    // ent->r.svFlags |= SVF_NOCLIENT;

    activator->client->ns.bomb_world = ent;
    activator->client->ps.weaponstate = WEAPON_RAISING;
    activator->client->ps.weaponTime = 500;
    activator->client->ns.bomb_wireTime = bomb_getDiffTime( activator, technical );

    ent->count = 1;

}

void bomb_defused( gentity_t *ent,  gentity_t *activator )
{

    if (!activator->client)
        return;

    if ( g_gametype.integer == GT_LTS ) {
        level.done_objectives[OtherTeam( ent->ns_team )]+=ent->bot_chattime	;
    }
    // got a parent
    if (ent->parent)
    {
        // reward him for placing the bomb ... remove his points.
        ent->parent->client->ns.rewards &= ~REWARD_BOMB_EXPLODE;
    }

    activator->client->ns.rewards |= REWARD_BOMB_DEFUSE;
    G_LogPrintf( "OBJECTIVE: [%i] \"%s\" defused the c4\n",
                 activator->client->ps.clientNum, activator->client->pers.netname  );

    G_FreeEntity( ent );
    trap_SetConfigstring( CS_BOMB_START_TIME, "0" );
    return;
}


void Weapon_C4 ( gentity_t *ent ) {
    gentity_t *c4;
    vec3_t forward;
    vec3_t hans;
    trace_t tr;

    AngleVectors( ent->client->ps.viewangles , forward, NULL,NULL );

    VectorMA( ent->client->ps.origin , 10, forward, hans );

    hans[2] -= 300;

    trap_Trace( &tr, ent->client->ps.origin,  NULL,NULL,hans, ent->client->ps.clientNum , MASK_SOLID );

    c4 = G_Spawn();

    c4->s.modelindex = G_ModelIndex( "models/weapons/c4/c4_placed.md3" );
    c4->s.eType = ET_GENERAL;

    VectorClear ( c4->s.pos.trDelta );

    VectorCopy( tr.endpos, c4->s.pos.trBase );
    G_SetOrigin( c4, tr.endpos );

    c4->classname = "c4_placed";
    c4->parent = ent;
    // remember the points for mission
    c4->bot_chattime = c4->parent->client->ns.bomb_parent->bot_chattime;
    // remember the flags
    c4->ns_flags = c4->parent->client->ns.bomb_parent->ns_flags;

    // set defuse state to 0
    c4->count = 0;

    {
        int random = random()*7;
        random++;
        c4->health = random;
    }

    c4->target_ent = ent->client->ns.bomb_parent;

    vectoangles( tr.plane.normal, c4->s.angles );

    VectorCopy( c4->s.angles, c4->s.apos.trBase );

    c4->use = bomb_use;
    c4->touch = bomb_touch;
    c4->ns_team = ent->client->sess.sessionTeam;
    c4->s.number = c4 - g_entities;

    VectorSet (c4->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
    VectorSet (c4->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
    c4->r.contents = CONTENTS_TRIGGER;

    c4->s.loopSound = G_SoundIndex( "sound/weapons/c4/bomb-tick.wav" );

    c4->damage  = ent->client->pers.nsPC.technical;

    ent->client->ns.bomb_world = c4;

    if (!c4->target_ent)
    {
        G_Error("D'oh! Trying to place C4 without C4 Bombing Place!\n");
        return;
    }

    c4->think = bomb_explode;

    c4->s.pos.trType = TR_STATIONARY;
    c4->s.pos.trTime = level.time;

    c4->physicsObject = qtrue;

    c4->nextthink = level.time + ( g_bombTime.integer * ONE_SECOND );

    trap_SetConfigstring( CS_BOMB_START_TIME, va("%i", c4->nextthink ) );
    trap_LinkEntity( c4 );

    NS_BotRadioMsg( ent, "bplaced" );
    if ( ! (c4->ns_flags & NS_FLAG_DEFUSABLE))
        NS_SendMessageToTeam( c4->ns_team == TEAM_RED ? TEAM_BLUE : TEAM_RED, "An "S_COLOR_RED"undefusable bomb"S_COLOR_WHITE" has been planted");
    // got a parent
    if (c4->parent)
    {
        // reward him for placing the bomb
        c4->parent->client->ns.rewards |= REWARD_BOMB_EXPLODE;
    }
    G_LogPrintf( "OBJECTIVE: [%i] \"%s\" planted the c4\n",
                 c4->parent->client->ps.clientNum, c4->parent->client->pers.netname  );
}

/*
======================================================================

Flashbang

======================================================================
*/

gentity_t *fire_flashbang( gentity_t *self, vec3_t start, vec3_t dir, int firestrength );

void weapon_flashbang_throw (gentity_t *ent) {
    gentity_t	*m;
    vec3_t start;

    VectorCopy(ent->s.pos.trBase, start);

    start[2] += ent->client->ps.viewheight+8.0f;

    VectorMA( start, 10, right , start );
    // extra vertical velocity
    forward[2] += 0.2f;

    if ( ent->client && ent->client->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADEROLL ) )
    {
        forward[2] -= 0.3f;
    }
    VectorNormalize( forward );

    m = fire_flashbang (ent, start, forward, ent->client->ps.weaponTime * -1);

    NS_BotRadioMsg( ent, "outgrenade" );
}



/*
======================================================================

GRENADES

======================================================================
*/
gentity_t *fire_smoke( gentity_t *self, vec3_t start, vec3_t dir, int firestrength );

void weapon_smoke_throw (gentity_t *ent) {
    gentity_t	*m;
    vec3_t start;

    VectorCopy(ent->s.pos.trBase, start);

    start[2] += ent->client->ps.viewheight+8.0f;


    // extra vertical velocity
    forward[2] += 0.2f;
    if ( ent->client && ent->client->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADEROLL ) )
    {
        forward[2] -= 0.3f;
    }
    VectorNormalize( forward );

    m = fire_smoke( ent, start, forward, /*this sucks, that we can't modify ps state another q3engine hack*/ ent->client->ps.weaponTime*-1);

    //	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics

    NS_BotRadioMsg( ent, "outgrenade" );
}

void weapon_grenade_throw (gentity_t *ent) {
    gentity_t	*m;
    vec3_t start;

    VectorCopy(ent->s.pos.trBase, start);

    start[2] += ent->client->ps.viewheight+8.0f;

    // extra vertical velocity
    forward[2] += 0.2f;
    if ( ent->client && ent->client->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADEROLL ) )
    {
        forward[2] -= 0.3f;
    }
    VectorNormalize( forward );

    m = fire_grenade (ent, start, forward, /*this sucks, that we can't modify ps state another q3engine hack*/ ent->client->ps.weaponTime*-1);

    //	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics

    NS_BotRadioMsg( ent, "outgrenade" );
}
/*
======================================================================

M203/GL GRENADE LAUNCHER

======================================================================
*/
gentity_t *fire_40mmgrenade ( gentity_t *self, vec3_t start, vec3_t dir, int speed );

void weapon_m203gl_fire (gentity_t *ent) {
    gentity_t	*m;
    vec3_t newmuzzle;

    VectorMA( muzzle, 4, right , newmuzzle );
    VectorMA( newmuzzle, -2, up , newmuzzle );

    if ( ent->client->crosshairFinishedChange && !ent->client->crosshairFadeIn )
    {/*
         float r,u;

         r = -3.5 + random() * 7;
         u = -3.5 + random() * 7;

         forward[PITCH] += u;
         forward[YAW] += r;
         /*
         VectorMA( newmuzzle, r, right, newmuzzle );
         VectorMA( newmuzzle, u, up, newmuzzle );
         */
    }
    m = fire_40mmgrenade (ent, newmuzzle, forward, SEALS_40MMGREN_SPEED);

    VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}



/*
======================================================================

ROCKET

======================================================================
*/

void Weapon_RocketLauncher_Fire (gentity_t *ent) {
    gentity_t	*m;

    m = fire_rocket (ent, muzzle, forward);
    m->damage *= s_quadFactor;
    m->splashDamage *= s_quadFactor;

    //	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}

//======================================================================


/*
===============
LogAccuracyHit
===============
*/
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker ) {
    if( !target->takedamage ) {
        return qfalse;
    }

    if ( target == attacker ) {
        return qfalse;
    }

    if( !target->client ) {
        return qfalse;
    }

    if( !attacker->client ) {
        return qfalse;
    }

    if( target->client->ps.stats[STAT_HEALTH] <= 0 ) {
        return qfalse;
    }

    if ( OnSameTeam( target, attacker ) ) {
        return qfalse;
    }

    return qtrue;
}


/*
===============
CalcMuzzlePoint

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePoint ( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
    VectorCopy( ent->s.pos.trBase, muzzlePoint );
    muzzlePoint[2] += ent->client->ps.viewheight;
    VectorMA( muzzlePoint, 14, forward, muzzlePoint );
    // snap to integer coordinates for more efficient network bandwidth usage
    SnapVector( muzzlePoint );
}

void Apply_Weapon_Kick ( gentity_t *ent, int weapon, int damage )
{
    qboolean	r_left = qfalse;
    qboolean	r_down = qfalse;
    int			xyzspeed;
    float		strength;
    double   kickfact;
    double   kickangle = 0.0;

    // if i already calculated shotgun recoil, return
    if (wshotty) return;

    // calculate speed of the character
    xyzspeed = sqrt( ent->client->ps.velocity[0] * ent->client->ps.velocity[0]
                     +  ent->client->ps.velocity[1] * ent->client->ps.velocity[1]
                     +  ent->client->ps.velocity[2] * ent->client->ps.velocity[2] );

    if (BG_CalcSpeed(ent->client->ps) == SEALS_STANDING) {
        // we are standing (by definition), no extra recoil
        kickfact = SEALS_WKICK_BASE;
    } else if (xyzspeed > SEALS_SPEED_MAX ) {
        // there shall be at highest a kickfactor and it shall depend
        // on xyzspeed maximum kick factor
        kickfact = SEALS_WKICK_SPEEDFACT;
    } else {
        //  0.0 <= 1.0 - (MAX - speed ) / MAX <= 1.0
        kickfact = SEALS_WKICK_BASE + SEALS_WKICK_SPEEDFACT * ( 1.0 - (SEALS_SPEED_MAX - xyzspeed ) / SEALS_SPEED_MAX ) ;
    }

    // if the character is jumping or falling, we have more weapon kickback
    if (BG_CalcSpeed(ent->client->ps) == SEALS_JUMPING)
        kickfact *= SEALS_WKICK_JUMPFACT;

    // if the character is not moving and is ducked, this is a good position
    // to sniper, weapon kick reduced
    if ( (BG_CalcSpeed(ent->client->ps) == SEALS_STANDING) &&
            (ent->client->ps.pm_flags & PMF_DUCKED) &&
            !SEALS_IS_PISTOL( ent->s.weapon ) )
        kickfact *= SEALS_WKICK_CROUCHNOSPD;

    // if the client is useing a PDW in secondary weaponmode (meaning he's holding
    // the handle), the weapon kick is reduced
    if ( (ent->s.weapon == WP_PDW) &&
            (ent->client->ns.weaponmode[ent->s.weapon] & (1 << WM_WEAPONMODE2)) )
        kickfact *= SEALS_WKICK_2HANDLEFACT;

    // set the strength
    if ( g_gametype.integer < GT_TEAM )
        strength = 2;
    else
        strength = ent->client->pers.nsPC.strength;

    // adopt the strength to the weapons kick, the factor should be between 1.0 and
    // SEALS_WKICK_MAXSTRFACT
    //
    kickfact *= 1.0 - (1.0 - SEALS_WKICK_MAXSTRFACT) * ((strength-1.0)/9.0);

    // the weapon kick should move the weapon slowly to the upper right
    if (random() < 0.3)
        r_left = qtrue;
    if (random() < 0.1)
        r_down = qtrue;

    // calculate the kick angle depending on the weapon
    switch (ent->s.weapon) {
    case WP_PDW: kickangle = SEALS_WKICK_PDW; break;
    case WP_MP5: kickangle = SEALS_WKICK_MP5; break;
    case WP_MAC10: kickangle = SEALS_WKICK_MAC10; break;

    case WP_M4: kickangle = SEALS_WKICK_M4; break;
    case WP_AK47: kickangle = SEALS_WKICK_AK47; break;
    case WP_M14: kickangle = SEALS_WKICK_M14; break;

    case WP_M249: kickangle = SEALS_WKICK_M249; break;

    case WP_M590: kickangle = SEALS_WKICK_M590; break;
    case WP_870: kickangle = SEALS_WKICK_870; break;
    case WP_SPAS15: kickangle = SEALS_WKICK_SPAS15; break;

    case WP_PSG1: kickangle = SEALS_WKICK_PSG1; break;
    case WP_MACMILLAN: kickangle = SEALS_WKICK_MACMILLAN; break;
    case WP_SL8SD: kickangle = SEALS_WKICK_SL8SD; break;

    case WP_P9S: kickangle = SEALS_WKICK_P9S; break;
    case WP_GLOCK: kickangle = SEALS_WKICK_GLOCK; break;
    case WP_SW40T: kickangle = SEALS_WKICK_SW40T; break;
    case WP_MK23: kickangle = SEALS_WKICK_MK23; break;
    case WP_SW629: kickangle = SEALS_WKICK_SW629; break;
    case WP_DEAGLE: kickangle = SEALS_WKICK_DEAGLE; break;

    default: kickangle = SEALS_WKICK_DEFAULT; break;
    }

    // modify the kickangle by the factor calculated before
    kickangle *= kickfact;
    //PrintMsg(NULL, "%f %f %f\n", kickangle, kickfact,
    //		((1.0-SEALS_WKICK_MAXRANDFACT)*random() + SEALS_WKICK_MAXRANDFACT));

    NS_BackupWeaponAim( ent );

    // apply the weapon kick
    //      0.0 <= random() <= 1.0
    // <--> t <= (1.0-t)*random() + t <= 1.0
    if (r_down)
        ent->client->ps.delta_angles[PITCH] += ANGLE2SHORT( kickangle *
                                               ((1.0-SEALS_WKICK_MAXRANDFACT)*random() + SEALS_WKICK_MAXRANDFACT)  );
    else
        ent->client->ps.delta_angles[PITCH] -= ANGLE2SHORT( kickangle *
                                               ((1.0-SEALS_WKICK_MAXRANDFACT)*random() + SEALS_WKICK_MAXRANDFACT)  );

    if (r_left)
        ent->client->ps.delta_angles[YAW] += ANGLE2SHORT( kickangle *
                                             ((1.0-SEALS_WKICK_MAXRANDFACT)*random() + SEALS_WKICK_MAXRANDFACT)  );
    else
        ent->client->ps.delta_angles[YAW] -= ANGLE2SHORT( kickangle *
                                             ((1.0-SEALS_WKICK_MAXRANDFACT)*random() + SEALS_WKICK_MAXRANDFACT)  );

}

/*
===============
CalcMuzzlePointOrigin

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePointOrigin ( gentity_t *ent, vec3_t origin, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
    VectorCopy( ent->s.pos.trBase, muzzlePoint );
    muzzlePoint[2] += ent->client->ps.viewheight;
    VectorMA( muzzlePoint, 14, forward, muzzlePoint );
    // snap to integer coordinates for more efficient network bandwidth usage
    SnapVector( muzzlePoint );
}


/*
=================
NSQ3 Rifle Bullet Fire
author: defcon-x (defcon-x@team-mirage.de)
date: 04-16-2k
description: fires special bullet (that goes through walls)
=================
*/


extern vmCvar_t g_bulletDamage;


void Lead_SpawnWallFX( trace_t	*trace, vec3_t tracefrom, int weapon, int otherEntityNum, qboolean funcExplosiveNum )
{
    gentity_t *tent;

    tent = G_TempEntity( trace->endpos, EV_BULLET_HIT_WALL );
    tent->s.eventParm = DirToByte( trace->plane.normal );
    tent->s.otherEntityNum = otherEntityNum;
    tent->s.weapon = weapon;
    tent->s.generic1 = NS_BulletHoleTypeForSurface( trace->surfaceFlags );

    VectorCopy( tracefrom, tent->s.apos.trBase );

    if ( funcExplosiveNum )
        tent->s.otherEntityNum2 = trace->entityNum;
    else
        tent->s.frame = 0; // leave no mark... if we're dead
}

void Fire_Lead ( gentity_t *ent, int caliber , int damage )
{
    vec3_t		end;
    trace_t		trace,trace2;
    gentity_t	*traceEnt;
    int			i;
    int			hits;
    int			unlinked;
    gentity_t	*unlinkedEntities[6];
    vec3_t		tracefrom;
    vec3_t		start;
    vec3_t		temp;
    int			wallhits;
    float		bulletThickn;
    int			bulletHits;
    int			ignoreent;
    double		r = 0,u = 0;
    float		max_range = BG_MaximumWeaponRange ( ent->client->ps.weapon ) ;
    int			*seed = &ent->client->ps.stats[STAT_SEED];

    //	G_Printf("maxrange: %f\n",max_range );
    unlinked = 0;
    hits = wallhits = 0;

    ignoreent = ent->s.number;
    if ( !BG_IsShotgun( ent->client->ps.weapon ) )
        ent->client->ns.rounds[ent->s.weapon]--;

    bulletThickn = MAX_9MM_BULLET_THICKN;
    bulletHits = MAX_BULLET_HITS;

    // BLUTENGEL:
    // modified whole code for calculating weapon recoil

    // BLUTENGEL:
    //  SYNCHRONIZE THIS CODE ALWAYS WITH THE CODE IN cg_weapons.c

    // snipers
    if (SEALS_IS_SNIPER(ent->s.weapon)) {
        switch(BG_CalcSpeed(ent->client->ps)) {
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
    } else if (SEALS_IS_RIFLE(ent->s.weapon)) {
        switch(BG_CalcSpeed(ent->client->ps)) {
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
    } else if (SEALS_IS_SMG(ent->s.weapon)) {
        switch(BG_CalcSpeed(ent->client->ps)) {
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
    } else if (SEALS_IS_PISTOL(ent->s.weapon)) {
        switch(BG_CalcSpeed(ent->client->ps)) {
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
    } else if (SEALS_IS_SHOTGUN(ent->s.weapon)) {
        switch(BG_CalcSpeed(ent->client->ps)) {
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
    } else if (SEALS_IS_MG(ent->s.weapon)) {
        switch(BG_CalcSpeed(ent->client->ps)) {
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

    //PrintMsg(NULL, "server: %f %f\n", Q_random(seed), Q_random(seed));

    // modify the according to the accuracy
    if (! SEALS_IS_SHOTGUN(ent->s.weapon) ) {
        float acc = ent->client->pers.nsPC.accuracy;

        if (acc < 1.0 || acc > 10.0) acc = 1.0;

        r *=  SEALS_ACCURACY_MOD + (1.0 - SEALS_ACCURACY_MOD) * (1.0 - (acc-1.0)/9.0);
        u *=  SEALS_ACCURACY_MOD + (1.0 - SEALS_ACCURACY_MOD) * (1.0 - (acc-1.0)/9.0);
    }

    // modify if the weapon is in scope mode
    if (BG_IsZooming( ent->client->ns.weaponmode[ent->s.weapon])) {
        r *= SEALS_WEAP_SCOPE_MOD;
        u *= SEALS_WEAP_SCOPE_MOD;
    }

    // modify if the weapon uses an attached laser and is a smg
    if ( BG_HasLaser(ent->client->ns.weaponmode[ent->s.weapon]) &&
            SEALS_IS_SMG(ent->s.weapon) &&
            ( ent->client->ns.weaponmode[ent->s.weapon] & (1 << WM_LACTIVE) ) ) {
        r *= SEALS_WEAP_LASER_MOD;
        u *= SEALS_WEAP_LASER_MOD;
    }

    // modify if the weapon uses an attached laser and is a pistol
    if ( BG_HasLaser(ent->client->ns.weaponmode[ent->s.weapon]) &&
            SEALS_IS_PISTOL(ent->s.weapon) &&
            ( ent->client->ns.weaponmode[ent->s.weapon] & (1 << WM_LACTIVE) ) ) {
        r *= SEALS_WEAP_PISTOLLASER_MOD;
        u *= SEALS_WEAP_PISTOLLASER_MOD;
    }

    // modify if the weapon is a pdw in secondary mode
    if ( (ent->client->ns.weaponmode[ent->s.weapon] & (1 << WM_WEAPONMODE2)) && (ent->s.weapon == WP_PDW) ) {
        r *= SEALS_WEAP_PDW_MOD;
        u *= SEALS_WEAP_PDW_MOD;
    }

    // modify if the weapon is shooted crouched / not for shotguns / not for pistols
    if ( (ent->client->ps.viewheight == CROUCH_VIEWHEIGHT) && !SEALS_IS_SHOTGUN(ent->s.weapon) && !SEALS_IS_PISTOL(ent->s.weapon) && !(ent->s.weapon == WP_PDW)) {
        r *= SEALS_WEAP_CROUCH_MOD;
        u *= SEALS_WEAP_CROUCH_MOD;
    }

    // modifiy if the weapon has a duckbill
    if ( SEALS_IS_SHOTGUN(ent->s.weapon) && ( ent->client->ns.weaponmode[ent->s.weapon] & ( 1 << WM_DUCKBILL ) ) ) {
        r *= SEALS_DUCKBILL_HOR_MOD;
        u *= SEALS_DUCKBILL_VER_MOD;
    }


    // copy viewangles to end
    VectorCopy(ent->client->ps.viewangles, end);
    // modify viewangles if recoil should be added
    if ( r || u ) {
        end[YAW] += r;
        end[PITCH] += u;
    }

    AngleVectors( end, forward, right, up );

    VectorCopy( ent->client->ps.origin, start );
    start[2] += ent->client->ps.viewheight;


    // prestep trace where we ignore our own entity
    // this is used to ignore the player in next traces
    VectorMA(start, 64, forward, end);
    trap_Trace( &trace, start, NULL, NULL, end, ent->s.number, MASK_SHOT );

    VectorCopy( trace.endpos, start);
    ignoreent = ENTITYNUM_NONE;
    // end hax

    VectorMA (start, max_range, forward, end);

    ent->client->accuracy_shots++;

    Apply_Weapon_Kick( ent, ent->s.weapon, damage );

    VectorCopy ( start,tracefrom);

    if ( g_bulletDamage.integer )
        damage = g_bulletDamage.integer;

    // Anti-Lag
    if (g_antilag.integer) G_ApplyAntiLag( ent );

    // BLUTENGEL: Introduced hitbox fix
    // see end of while for closing RestoreHitboxes
    NS_FixHitboxes();
    do {
        // trap_Trace(&trace, tracefrom, NULL, NULL, end, ignoreent, MASK_SHOT );
        trap_Trace( &trace, tracefrom, NULL, NULL, end, ignoreent, MASK_SHOT );

        if ( trace.entityNum >= ENTITYNUM_MAX_NORMAL ) {
            if (trace.surfaceFlags & SURF_SKY)
                break;
            if (trace.allsolid || trace.startsolid)// it should start free
                break;
            if (wallhits >= bulletHits)
                break;
            // gone?
            if ( trace.fraction >= 1 )
                break;

            // hit! now render the effect
            Lead_SpawnWallFX( &trace, tracefrom, ent->s.weapon, ent->s.number, is_func_explosive( &g_entities[trace.entityNum] ) );

            // only 9mm bounces off the wall, and not every projectile
            if ( trace.surfaceFlags & SURF_METALSTEPS &&
                    !BG_IsShotgun( ent->client->ps.weapon ) &&
                    ( caliber == AM_LIGHT_PISTOL || caliber == AM_MEDIUM_PISTOL || caliber == AM_SMG )  && Q_crandom( seed ) < 0.5f )
            {
                VectorCopy( trace.endpos, temp);
                G_BounceProjectile( tracefrom, temp,  trace.plane.normal, end );
                VectorCopy( temp, tracefrom );
                VectorSubtract( end, temp, forward );
                VectorNormalize(forward);
                // now you can hit yourself with your projectile
                ignoreent = ENTITYNUM_NONE;
                wallhits++;

                damage *= 0.7;
                // after rebounced... break, this won't go through metal then...
                continue;
            }

            if ( BG_IsShotgun( ent->client->ps.weapon ) )
                break;
            // 9mm only goes through 'soft' materials like dirt/snow/wood etc.
            if (  ( caliber == AM_SMG || caliber == AM_LIGHT_PISTOL || caliber == AM_MEDIUM_PISTOL ) && !(trace.surfaceFlags & SURF_WOODSTEPS) && !(trace.surfaceFlags & SURF_SOFTSTEPS) && !(trace.surfaceFlags & SURF_GLASS)
                    && !(trace.surfaceFlags & SURF_DIRTSTEPS) && !(trace.surfaceFlags & SURF_SNOWSTEPS) && !(trace.surfaceFlags & SURF_SANDSTEPS) )
                break;

            /*
            ============
            Break through wall ~ get break value
            ============
            */
            bulletThickn = BG_LeadGetBreakValueForSurface( &trace );// ;

            switch (ent->s.weapon)
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
            if ( trap_PointContents( temp, -1 ) & CONTENTS_SOLID )
                break;

            // get exit position...
            trap_Trace (&trace2, temp, NULL, NULL, trace.endpos, ignoreent, MASK_SHOT );

            // get the point where the bullet leaves the wall
            VectorCopy (trace2.endpos, tracefrom);

            // we actually found a plane that might be a window
            if(trace2.entityNum < ENTITYNUM_MAX_NORMAL)
            {
                // do a trace back again, but this time ignore the entity.
                trap_Trace (&trace2, temp, NULL, NULL, trace.endpos, trace2.entityNum, MASK_SHOT );

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

            // reduce the damage
            damage *= 0.7;

            wallhits ++;

            Lead_SpawnWallFX( &trace2, tracefrom, ent->s.weapon, ent->s.number, is_func_explosive( &g_entities[trace2.entityNum] ) );
            continue;
        }

        wallhits++;

        traceEnt = &g_entities[ trace.entityNum ];

        if ( traceEnt->takedamage )
        {
            int dmg;

            dmg = G_Damage (traceEnt, ent, ent, forward, trace.endpos, damage, 0, 0);

            ent->client->accuracy_hits++;

            if ( g_debugDamage.integer )
            {
                Com_Printf("fire_lead: hitted %s\n", traceEnt->classname );
            }
            if ( traceEnt->client )
            {
                int HitLocation = LOC_NULL;
                gentity_t *tent;

                HitLocation = NS_CheckLocationDamage( traceEnt, trace.endpos, WP_MK23 );

                // covered by a vest?
                if ( ( HitLocation == LOC_BACK || HitLocation == LOC_STOMACH || HitLocation == LOC_CHEST ) && traceEnt->client->ps.powerups[PW_VEST] )
                {
                    tent = G_TempEntity( trace.endpos, EV_BULLET_HIT_WALL );
                    tent->s.eventParm = DirToByte( trace.plane.normal );
                    tent->s.otherEntityNum = ent->s.number;
                    tent->s.weapon = ent->s.weapon;
                    tent->s.frame = 1; // kevlar vest

                    // this is used for the bullet tracer
                    // i use the angle vector as start point
                    // and the base as the endpoint for the tracer!
                    VectorCopy( tracefrom, tent->s.apos.trBase );

                } // or even a helmet , call sparks then
                else if ( HitLocation == LOC_HEAD && traceEnt->client->ps.powerups[PW_HELMET] )
                {
                    tent = G_TempEntity( trace.endpos, EV_BULLET_HIT_WALL );
                    tent->s.eventParm = DirToByte( trace.plane.normal );
                    tent->s.otherEntityNum = ent->s.number;
                    tent->s.weapon = ent->s.weapon;
                    tent->s.frame = 2; // helmet hit
                    // this is used for the bullet tracer
                    // i use the angle vector as start point
                    // and the base as the endpoint for the tracer!
                    VectorCopy( tracefrom, tent->s.apos.trBase );
                } // call blood
                else if ( traceEnt->takedamage && traceEnt->client )
                {
                    tent = G_TempEntity( trace.endpos, EV_BULLET_HIT_FLESH );
                    tent->s.eventParm = traceEnt->s.number;
                    tent->s.torsoAnim = DirToByte( forward );
                    tent->s.legsAnim = dmg;
                    // this is used for the bullet tracer
                    // i use the angle vector as start point
                    // and the base as the endpoint for the tracer!
                    VectorCopy( tracefrom, tent->s.apos.trBase );
                }

                // these weapons don't go through bodies - but they might
                // break through a func explosive
                if ( caliber == AM_LIGHT_PISTOL || caliber == AM_MEDIUM_PISTOL || caliber == AM_SMG ||
                        BG_IsShotgun( ent->s.weapon ) )
                    break;

                VectorCopy( trace.endpos, tracefrom );
                ignoreent = trace.entityNum;

                // deflect the bullet
                VectorSubtract(end, tracefrom, temp);
                r = VectorLength(temp);
                // by right/up direction (maximum of 5 degrees (5*2*PI)/360)
                u = 0.086 - 0.172 * Q_random(seed);
                VectorMA(end, u, right, end);
                u = 0.086 - 0.172 * Q_random(seed);
                VectorMA(end, u, up, end);

                // reduce the damage
                damage *= 0.7;

                continue;
            }
        }

        // 	G_Printf("hitted brushentity: %i\n", trace.entityNum );

        // create FX
        Lead_SpawnWallFX( &trace, tracefrom, ent->s.weapon, ent->s.number, is_func_explosive( &g_entities[trace.entityNum] ) );

        // only 9mm bounces off the wall, and not every projectile
        if ( trace.surfaceFlags & SURF_METALSTEPS &&
                !BG_IsShotgun( ent->s.weapon ) &&
                ( caliber == AM_SMG || caliber == AM_LIGHT_PISTOL || caliber == AM_MEDIUM_PISTOL )  && Q_crandom( seed ) < 0.5f )
        {
            VectorCopy( trace.endpos, temp);
            G_BounceProjectile( tracefrom, temp,  trace.plane.normal, end );
            VectorCopy( temp, tracefrom );
            VectorSubtract( end, temp, forward );
            VectorNormalize(forward);
            // now you can hit yourself with your projectile
            ignoreent = trace.entityNum;
            wallhits++;

            damage *= 0.7;

            // after rebounced... break, this won't go through metal then...
            continue;
        }

        // 9mm / shotgun only goes through 'soft' materials like dirt/snow/wood etc.
        if (  ( caliber == AM_SMG || caliber == AM_LIGHT_PISTOL || caliber == AM_MEDIUM_PISTOL ||
                BG_IsShotgun( ent->s.weapon )  ) && !(trace.surfaceFlags & SURF_WOODSTEPS) && !(trace.surfaceFlags & SURF_SOFTSTEPS) && !(trace.surfaceFlags & SURF_GLASS)
                && !(trace.surfaceFlags & SURF_DIRTSTEPS) && !(trace.surfaceFlags & SURF_SNOWSTEPS) && !(trace.surfaceFlags & SURF_SANDSTEPS) )
        {
            break;
        }

        /*
        ============
        Break through wall ~ get break value
        ============
        */
        bulletThickn = BG_LeadGetBreakValueForSurface( &trace );// ;

        switch ( ent->s.weapon )
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
        if ( trap_PointContents( temp, -1 ) & CONTENTS_SOLID && !(trace.surfaceFlags & SURF_GLASS) )
            break;

        // get exit position...
        trap_Trace (&trace2, temp, NULL, NULL, trace.endpos, ignoreent, MASK_SOLID );

        // get the point where the bullet leaves the wall
        VectorCopy (trace2.endpos, tracefrom);

        //	G_Printf("exit brushentity: %i\n", trace2.entityNum );

        // we actually are free again, but we hit something different from our original entity?
        // go back ignore the current
        if(trace2.entityNum != trace.entityNum )
        {
            // do a trace back again, but this time ignore the entity.
            trap_Trace (&trace2, temp, NULL, NULL, trace.endpos, trace2.entityNum, MASK_SOLID );

            // get the point where the bullet leaves the wall
            VectorCopy (trace2.endpos, tracefrom);

            //		G_Printf("new exit for brushentity: %i\n", trace.entityNum );
        }

        if ( trace2.entityNum < ENTITYNUM_MAX_NORMAL )
        {
            // create FX
            Lead_SpawnWallFX( &trace2, tracefrom, ent->s.weapon, ent->s.number, is_func_explosive( &g_entities[trace2.entityNum] ) );

            ignoreent = trace2.entityNum;
        }

        // unlink this entity, so the next trace will go past it
        trap_UnlinkEntity( traceEnt );

        // deflect the bullet
        VectorSubtract(end, tracefrom, temp);
        r = VectorLength(temp);
        // by right/up direction (maximum of 5 degrees (5*2*PI)/360)
        u = 0.086 - 0.172 * Q_random(seed);
        VectorMA(end, u, right, end);
        u = 0.086 - 0.172 * Q_random(seed);
        VectorMA(end, u, up, end);

        // reduce the damage
        damage *= 0.7;

        unlinkedEntities[unlinked] = traceEnt;
        unlinked++;
    } while ( unlinked < bulletHits && wallhits < bulletHits );
    NS_RestoreHitboxes();

    // link back in any entities we unlinked
    for ( i = 0 ; i < unlinked ; i++ ) {
        trap_LinkEntity( unlinkedEntities[i] );
    }

    if (g_antilag.integer) G_UndoAntiLag ( );

}
/*
===============
FireWeapon
===============
*/
void FireWeapon( gentity_t *ent ) {
    gitem_t *weapon;
    vec3_t knockvec;
    float kbfact = 0.0f;

    weapon = BG_FindItemForWeapon (ent->s.weapon );

    if (!weapon)
        return;

    // set aiming directions
    AngleVectors (ent->client->ps.viewangles, forward, right, up);

    CalcMuzzlePoint ( ent, forward, right, up, muzzle );

    // calculate knockback vector
    if ( g_gravity.value >= 0 && g_gravity.value < 800) {
        knockvec[0] = -forward[0];
        knockvec[1] = -forward[1];
        knockvec[2] = -forward[2];

        kbfact = (800-g_gravity.value)/800.0 * ( 0.7 + 0.3*(150.0 - ent->client->pers.nsPC.strength*10.0)/150.0);
    }

    // fire the specific weapon
    switch( ent->s.weapon ) {

        // SEALS +
    case WP_GLOCK:
        if ( ent->client->ps.stats[STAT_WEAPONMODE] & WM_SILENCER )
            Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_GLOCK - SEALS_DMG_SILENCER );
        else
            Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_GLOCK );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_GLOCK);
        break;
    case WP_P9S:
        if ( ent->client->ps.stats[STAT_WEAPONMODE] & WM_SILENCER )
            Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_P9S - SEALS_DMG_SILENCER );
        else
            Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_P9S );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_P9S);
        break;
    case WP_MK23:
        if ( ent->client->ps.stats[STAT_WEAPONMODE] & WM_SILENCER )
            Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_MK23 - SEALS_DMG_SILENCER );
        else
            Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_MK23 );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_MK23);
        break;
    case WP_SW40T:
        if ( ent->client->ps.stats[STAT_WEAPONMODE] & WM_SILENCER )
            Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_SW40T - SEALS_DMG_SILENCER );
        else
            Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_SW40T );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_SW40T);
        break;
    case WP_SW629:
        Fire_Lead( ent, weapon->giAmmoTag , SEALS_DMG_SW629 );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_SW629);
        break;
    case WP_DEAGLE:
        Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_DEAGLE );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_DEAGLE);
        break;

    case WP_MP5:
        if ( ent->client->ps.stats[STAT_WEAPONMODE] & WM_SILENCER )
            Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_MP5 - SEALS_DMG_SILENCER );
        else
            Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_MP5 );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_MP5);
        break;
    case WP_MAC10:
        if ( ent->client->ps.stats[STAT_WEAPONMODE] & WM_SILENCER )
            Fire_Lead( ent, weapon->giAmmoTag,  SEALS_DMG_MAC10 - SEALS_DMG_SILENCER );
        else
            Fire_Lead( ent, weapon->giAmmoTag,  SEALS_DMG_MAC10 );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_MAC10);
        break;
    case WP_PDW:
        Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_PDW );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_PDW);
        break;

    case WP_M14:
        Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_M14 );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_M14);
        break;
    case WP_M4:
        if (ent->client->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ) && ent->client->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) ) {
            weapon_m203gl_fire( ent );
            ent->client->ns.rounds[ent->s.weapon*3]--;
        }
        else
            if ( ent->client->ps.stats[STAT_WEAPONMODE] & WM_MUZZLEHIDER )
                Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_M4 - SEALS_DMG_MUZZLEHIDER );
            else
                Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_M4 );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_M4);
        break;
    case WP_AK47:
        if (ent->client->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ) && ent->client->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) ) {
            weapon_m203gl_fire( ent );
            ent->client->ns.rounds[ent->s.weapon*3]--;
        }
        else
            if ( ent->client->ps.stats[STAT_WEAPONMODE] & WM_MUZZLEHIDER )
                Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_AK47 - SEALS_DMG_MUZZLEHIDER );
            else
                Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_AK47 );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_AK47);
        break;
    case WP_M249:
        Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_M249 );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_M249);
        break;

    case WP_SPAS15:
        Weapon_Shotgun( ent, weapon->giAmmoTag, SEALS_DMG_SPAS15, SPAS15_PELLETS );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_SPAS15);
        break;
    case WP_M590:
        Weapon_Shotgun( ent, weapon->giAmmoTag, SEALS_DMG_M590, M590_PELLETS );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_M590);
        break;
    case WP_870:
        Weapon_Shotgun( ent, weapon->giAmmoTag, SEALS_DMG_870, M870_PELLETS );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_870);
        break;

    case WP_MACMILLAN:
        Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_MACMILLAN );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_MACMILLAN);
        break;
    case WP_PSG1:
        Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_PSG1 );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_PSG1);
        break;
    case WP_SL8SD:
        Fire_Lead( ent, weapon->giAmmoTag, SEALS_DMG_SL8SD );
        if (g_gravity.value >= 0 && g_gravity.value < 800)
            G_KnockPlayer(ent, knockvec, kbfact*SEALS_KNOCKBACK_SL8SD);
        break;

    case WP_SEALKNIFE:
    case WP_KHURKURI:
        break;

    case WP_SMOKE:
        weapon_smoke_throw ( ent );
        break;
    case WP_GRENADE:
        weapon_grenade_throw ( ent );
        break;
    case WP_FLASHBANG:
        weapon_flashbang_throw ( ent );
        break;
    default:
        // FIXME		G_Error( "Bad ent->s.weapon" );
        break;
    }
}
