// Copyright (C) 1999-2000 Team Mirage
// [ create 6-1-2000 ]
//
// all code in this file is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "g_local.h"

#define BRUSH_TEAM_RED		0
#define BRUSH_TEAM_BLUE		1
#define BRUSH_TEAM_ALL		2

// always keep in this order: 1, 2, 4 , 8, 10 , 20 , 30 , 40, 80, 100, 200 etc..

void InitMover(gentity_t*ent);
void LogExit(const char *string);
void InitTrigger ( gentity_t *self );

qboolean Is_OnBrushTeam(gentity_t *brush, gentity_t *client)
{
    if (brush->ns_team == BRUSH_TEAM_RED && client->client->sess.sessionTeam == TEAM_RED)
        return qtrue;
    else if (brush->ns_team == BRUSH_TEAM_BLUE && client->client->sess.sessionTeam == TEAM_BLUE)
        return qtrue;
    else if (brush->ns_team == BRUSH_TEAM_ALL)
        return qtrue;
    else
        return qfalse;
}


/*QUAKED  trigger_missionobjective (1 0 0) (-8 -8 -8) (8 8 8)
"team"			0 = seals / 1 = tangos
"optional"		it won't add a new mission objective but it will mark one as completed
"targetname"	when triggered, one mission objective for the team is marked as completed.

use this entity if you want to create your own type of mission. for example you could create
a mission where the tangos have to destroy a func_explosive in order to win.
to do this you have to target this entity using your func_explosive and set team to 1.
you can use as much of these as you need
*/

void trigger_points_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
    // BLUTENGEL XXX: removed 20040205 for b release
    /*	if ( !Is_OnBrushTeam( self, activator ) ) {
    //		G_Printf("not on brush team!\n");
    return;
    }*/

    if (g_gametype.integer == GT_LTS )
    {
        if ( self->ns_team == BRUSH_TEAM_RED )
            level.done_objectives[TEAM_RED]++;
        else if ( self->ns_team == BRUSH_TEAM_BLUE )
            level.done_objectives[TEAM_BLUE]++;
    }
}

void trigger_points_think(gentity_t *self) {
    return;
}

void SP_trigger_points( gentity_t *self ) {
    int optional = 0;

    G_SpawnInt( "team","0", &self->ns_team);
    G_SpawnInt( "optional", "0", &optional);


    // check for invalid values
    if ( self->ns_team != BRUSH_TEAM_RED && self->ns_team != BRUSH_TEAM_BLUE )
        G_Error( "%s: 'team' with invalid value %i\n", self->classname , self->ns_team);

    self->use = trigger_points_use;

    self->r.svFlags = SVF_NOCLIENT;

    // we're spawning this one so we got one more objective to do
    if (g_gametype.integer == GT_LTS && !optional )
    {
        if ( self->ns_team == BRUSH_TEAM_RED )
            level.num_objectives[TEAM_RED]++;
        else if ( self->ns_team == BRUSH_TEAM_BLUE )
            level.num_objectives[TEAM_BLUE]++;
    }
}


/*QUAKED trigger_counter (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) START_ON
Repeatedly fires its targets.
Can be turned on or off by using.

"wait"			base time between triggering all targets, default is 1
"loop"			(0) no loop, (1) always loop
"targetname"	can be targeted to be turned off
*/

void trigger_counter_think( gentity_t *self )
{

    if ( !self->activator )
        self->activator = self;

    G_UseTargets (self, self->activator );

    // off
    self->nextthink = 0;

    // loop?
    if (self->ns_flags & NS_FLAG_LOOP)
        self->nextthink = level.time + self->wait;
}

void trigger_counter_use( gentity_t *self, gentity_t *other, gentity_t *activator ) {
    self->activator = activator;

    // if on, turn it off
    if ( self->nextthink ) {
        self->nextthink = 0;
        return;
    }

    // turn it on
    self->nextthink = level.time + self->wait;
}

void SP_trigger_counter( gentity_t *self ) {
    int loop;

    G_SpawnFloat( "wait", "0", &self->wait );
    G_SpawnInt("loop", "0", &loop);

    self->use = trigger_counter_use;

    // in seconds please...
    if (!self->wait) {
        G_Printf("No waiting time set! Default: 10\n");
        self->wait = 10;
    }

    self->wait *= ONE_SECOND;

    if (loop)
        self->ns_flags |= NS_FLAG_LOOP;

    self->think = trigger_counter_think;
    self->nextthink = 0;

    // set initial think only when start_on and not in g_lts.
    if ( ( self->spawnflags & 1 ) && ( g_gametype.integer != GT_LTS) ) {
        self->nextthink = level.time + self->wait;
    }

    // no draw
    self->r.svFlags = SVF_NOCLIENT;
}

/*QUAKED trigger_explosion (1 0 0) (-8 -8 -8) (8 8 8)
As soon as it gets triggered an explosion will be shown
and dmg will be infliced within dmg_rad.
If time (in seconds) is set . it will trigger every X seconds

"dmg"			default 5 ( whole numbers only )
"radius"		default 50 ( whole numbers only )
"wait"          default 0 ( no loop , only trigger )
"delay"			default 0 ( delay when triggered in sec - only when wait=0)

*/

void explosion_explode( gentity_t *ent )
{
    G_TempEntity( ent->s.origin, EV_EXPLOSION );
    // splash damage
    if ( ent->splashDamage ) 
        G_RadiusDamage( ent->s.origin, ent, ent->splashDamage, ent->splashRadius, ent, ent->splashMethodOfDeath );
}

void explosion_use( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
    if ( ent->random )
    {
        ent->nextthink = level.time + (ent->random*ONE_SECOND);
        ent->think = explosion_explode;
        return;
    }
    //

    //	G_Printf("explosion_use\n");
    explosion_explode( ent );
}

void explosion_think( gentity_t *ent )
{
    explosion_explode( ent );

    /*if ( ent->wait == 100 && !Q_stricmp(NS_GetMapName(),"ns_bunker") )
    {
    G_Printf("applying bunker hack!\n");
    return;
    }*/
    ent->nextthink = level.time + ent->wait;

    //	G_Printf("explosion_think : %f\n", ent->wait);
    //	G_Printf("next: %i\n", ent->wait  );
}
void SP_trigger_explosion( gentity_t *self ) {
    // grab 2 variables
    G_SpawnInt( "radius", "0", &self->splashRadius );
    G_SpawnInt( "dmg", "0", &self->splashDamage );
    G_SpawnFloat( "wait", "0", &self->wait );
    G_SpawnFloat( "delay", "0", &self->random );

    self->s.weapon = WP_GRENADE;

    if ( !self->splashRadius ) {
        self->splashRadius = 50;
        G_Printf("%s at %s with no radius set! Default: 50\n", self->classname,vtos(self->s.origin) );
    }

    if ( !self->splashDamage ) {
        self->splashDamage = 5;
        G_Printf("%s at %s with no damage set! Default: 5\n", self->classname,vtos(self->s.origin) );
    }

    if ( self->wait && self->random )
    {
        G_Error("%s at %s with 'wait' and 'delay' set!\n", self->classname, vtos(self->s.origin ) );
        return;
    }

    if ( self->wait ) {
        self->wait *= ONE_SECOND;
        self->think = explosion_think;
        self->nextthink = level.time + self->wait;
    }

    // set use stats
    self->use = explosion_use;

}

/*QUAKED info_bomb_disarm (1 0 0) (-8 -8 -8) (8 8 8)
If player standing in radius from origin of ent
he is able to arm a bomb.

"radius"		default 50 ( whole numbers only )
"team"			team 0

*/
void info_bomb_disarm_use( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
    G_UseTargets( ent, activator );
}

void info_bomb_disarm_think( gentity_t *ent )
{
    int i;

    for ( i = 0; i < level.maxclients; i++ )
    {
        gclient_t *client = &level.clients[i];

        if (client->pers.connected != CON_CONNECTED)
            continue;

        if ( Is_OnBrushTeam( ent, &g_entities[ client - level.clients ] ) )
        {
            vec3_t origin;
            VectorCopy( client->ps.origin, origin );
            // this should fix the bug that you can only have one bombspot
            if ( client->ns.bomb_parent && client->ns.bomb_parent != ent )
                continue;

            if ( Distance( ent->s.origin, origin ) <= ent->splashRadius )
            {
                if ( !( client->ps.pm_flags & PMF_BOMBRANGE ) ) {
                    client->ps.pm_flags |= PMF_BOMBRANGE;
                    client->ns.bomb_parent = ent;
                }
                continue; // skip
            }
        }
        else
        {
            continue; // skip
        }

        if ( client->ps.pm_flags & PMF_BOMBRANGE || client->ns.bomb_parent ) {
            client->ps.pm_flags &=~ PMF_BOMBRANGE;
            client->ns.bomb_parent = NULL;
        }
    }


    ent->nextthink = level.time + 10;
}

void SP_info_bomb_disarm( gentity_t *self ) {
    int	optional, defusable;

    // grab 2 variables
    G_SpawnInt( "radius", "50", &self->splashRadius );
    G_SpawnInt( "team", "0", &self->ns_team );
    G_SpawnInt( "optional", "0", &optional );
    G_SpawnFloat( "time", "120", &self->wait );
    G_SpawnInt( "defusable", "1", &defusable);
    G_SpawnInt( "addmission", "1", &self->bot_chattime);


    if ( g_gametype.integer != GT_LTS )
        return;

    self->ns_flags = 0;

    if (defusable) self->ns_flags |= NS_FLAG_DEFUSABLE;

    /*if ( !Q_stricmp(NS_GetMapName(),"ns_snowcamp") )
    {
    G_Printf("applying snowcamp mapfix.\n");
    optional = 1;
    }
    if ( !Q_stricmp(NS_GetMapName(),"ns_sleepingwolf") )
    {
    G_Printf("applying sleepingwolf mapfix.\n" );
    self->splashRadius *= 3;
    }*/
    if ( !self->wait ) {
        self->wait = 120;
        G_Printf("%s at %s with no time set! Default: 120\n", self->classname,vtos(self->s.origin) );
    }

    if ( !self->splashRadius ) {
        self->splashRadius = 50;
        G_Printf("%s at %s with no radius set! Default: 50\n", self->classname,vtos(self->s.origin) );
    }

    if (!optional)
    {
        level.num_objectives[TEAM_RED]+=self->bot_chattime;
        level.num_objectives[TEAM_BLUE]+=self->bot_chattime;
    }

    //	G_Printf("spawning bomb: %i\n", self->ns_team );

    if ( self->ns_team == 0 )
        level.bombs[TEAM_RED]++;
    else if ( self->ns_team == 1 )
        level.bombs[TEAM_BLUE]++;
    else if ( self->ns_team == 2 )
        level.bombs[TEAM_FREE]++;

    g_bombTime.integer = (int)self->wait ;
    trap_Cvar_Set( "g_bombTime", va("%i", (int)self->wait ) );

    //	G_Printf("bomtime %i\n", g_bombTime.integer );
    // set use stats
    self->think = info_bomb_disarm_think;
    self->nextthink = level.time + 10;

    self->r.svFlags = SVF_NOCLIENT;
}

/*
===============================================================================

WALL

===============================================================================
*/

/*QUAKED func_wall (0 .5 .8) ? TRIGGER_SPAWN TOGGLE START_ON
This is just a solid wall if not inhibited

TRIGGER_SPAWN	the wall will not be present until triggered
it will then blink in to existance; it will
kill anything that was in it's way

TOGGLE			only valid for TRIGGER_SPAWN walls
this allows the wall to be turned on and off

START_ON		only valid for TRIGGER_SPAWN walls
the wall will initially be present

"model2"	.md3 model to also draw
"color"		constantLight color
"light"		constantLight radius
*/

void func_wall_use (gentity_t *self, gentity_t *other, gentity_t *activator)
{
    if (self->r.contents == 0)
    {
        self->r.contents = CONTENTS_SOLID;
        self->r.svFlags &= ~SVF_NOCLIENT;
        //		G_KillBox (self);
    }
    else
    {
        self->r.contents = 0;
        self->r.svFlags |= SVF_NOCLIENT;
    }
    trap_LinkEntity (self);

}

void SP_func_wall (gentity_t *self)
{
    trap_SetBrushModel( self, self->model );
    InitMover( self );
    VectorCopy( self->s.origin, self->s.pos.trBase );
    VectorCopy( self->s.origin, self->r.currentOrigin );

    // just a wall
    if ((self->spawnflags & 7) == 0)
    {
        self->r.contents = CONTENTS_SOLID;
        trap_LinkEntity( self );
        return;
    }

    // it must be TRIGGER_SPAWN
    if (!(self->spawnflags & 1))
    {
        G_Printf("func_wall missing TRIGGER_SPAWN\n");
        self->spawnflags |= 1;
    }

    // yell if the spawnflags are odd
    if (self->spawnflags & 4)
    {
        if (!(self->spawnflags & 2))
        {
            G_Printf("func_wall START_ON without TOGGLE\n");
            self->spawnflags |= 2;
        }
    }

    self->use = func_wall_use;
    if (self->spawnflags & 4)
    {
        self->r.contents = CONTENTS_SOLID;
    }
    else
    {
        self->r.contents = 0;
        self->r.svFlags |= SVF_NOCLIENT;
    }
    trap_LinkEntity (self);
}



#define WOOD		1
#define METAL		2
#define STONE		4

void model_die ( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
    vec3_t origin;
    gentity_t *temp;
    vec3_t dir;

    VectorSubtract(self->r.maxs, self->r.mins, origin);

    // define in : bg_public.h (oder so) bei den ganzen EV_ sachen das:
    // EV_FUNCEXPLOSIVE, (ganz am ende hinzufügen , vor der - aber dann dem vorletzen ein , hintendran falls keines da ist '}')
    temp = G_TempEntity( origin , EV_FUNCEXPLOSIVE );

    VectorCopy( self->r.maxs, temp->s.origin );
    VectorCopy( self->r.mins, temp->s.origin2 );


    if ( inflictor && !Q_stricmp( inflictor->classname , "reallead") )
    {
        VectorCopy( inflictor->movedir, dir );
    }
    else if ( inflictor && BG_IsGrenade( inflictor->s.weapon ) )
    {
        VectorSubtract (inflictor->r.currentOrigin, origin, dir);
        // push the center of mass higher than the origin so players
        // get knocked into the air more
        dir[2] += 24;

        VectorNormalize( dir );
    }
    else
    {
        vec3_t temp;

        VectorAdd ( inflictor->r.absmin, inflictor->r.absmax, temp);
        VectorScale ( temp, 0.5, temp);

        VectorSubtract ( origin, temp,  dir);

        VectorNormalize( dir );

    }

    VectorCopy( dir, temp->s.apos.trBase );

    //	G_Printf("spawned func explosive @ %s\n", vtos(origin ) );

    temp->s.eventParm = self->spawnflags;
    temp->r.svFlags |= SVF_BROADCAST;

    //
    if ( !self->fly_sound_debounce_time )
    {
        temp->s.frame = self->s.number; // copy entity number so we can remove all marks from this entity.
        temp->s.otherEntityNum = self->s.number;
        temp->s.otherEntityNum2 = self->s.number;
        temp->s.powerups = self->s.number;
    }

    SnapVector( temp->s.origin );
    //	G_Printf("Temp entitity: maxs: %s mins: %s , spawnflags: %i\n", vtos(temp->s.origin ), vtos(temp->s.origin2 ), temp->s.eventParm );
    return;
}


int vipSpawns = 0;

void SP_vip_spawn (gentity_t *ent ) {
    //	int numobjectives = level.num_objectives[TEAM_RED] + level.num_objectives[TEAM_BLUE];

    G_SpawnFloat( "stayalive", "0", &ent->wait );
    G_SpawnInt	( "team", "0", &ent->ns_team );
    G_SpawnInt	( "givebc", "0", &ent->ns_flags );

    if ( g_gametype.integer != GT_LTS )
        return;

    if ( vipSpawns <= 0 )
    {
        level.num_objectives[TEAM_RED]++;
        level.num_objectives[TEAM_BLUE]++;
    }
    //	G_Printf("spawning vipspawnpoint: %i %i %i\n", level.num_objectives[TEAM_RED],level.num_objectives[TEAM_BLUE], numobjectives);

    vipSpawns++;

    if ( ent->wait && !level.vipTime)
    {
        G_Printf("Vip has to stay alive... %i seconds\n", ent->wait );

        if ( ent->ns_team == 0 ) {
            level.vip[TEAM_RED] = VIP_STAYALIVE;
        }
        else if ( ent->ns_team == 1 ) {
            level.vip[TEAM_BLUE] = VIP_STAYALIVE;
        }
        else if ( ent->ns_team >= 2 || ent->ns_team < 0 ) {
            G_Error("%s: team > 1?", ent->classname );
            return;
        }
        level.vipTime = ent->wait * ONE_SECOND;
    }
}


/*QUAKED info_vip_rescue (1 0 0) (-8 -8 -8) (8 8 8)
If V.I.P. of the given team (0/1) is reaching this.
Team will

"radius"		default 50 ( whole numbers only )
"team"			team  0 seals, 1 tangos
"bconly"		only allow the vip to exit with the briefcase in his hand
*/
void info_vip_rescue_think( gentity_t *ent )
{
    int i;

    for ( i = 0; i < level.maxclients; i++ )
    {
        gclient_t *client = &level.clients[i];

        if (client->pers.connected != CON_CONNECTED)
            continue;

        if (
            Is_OnBrushTeam( ent, &g_entities[ client - level.clients ] )
            && client->ns.is_vip
            && ( Distance( ent->s.origin, client->ps.origin ) <= ent->splashRadius)
            && client->sess.waiting == qfalse
        )
        {
            // he hasn't got the breifcase and we're only allowed to exit with it
            if ( ent->ns_flags && client->ps.powerups[PW_BRIEFCASE] <= 0 )
                continue;

            if (ent->ns_team == 0)
                level.done_objectives[TEAM_RED]++;
            else
                level.done_objectives[TEAM_BLUE]++;

            client->sess.waiting = qtrue;

            G_LogPrintf( "OBJECTIVE: [%i] \"%s\" escaped as VIP\n",
                         client->ps.clientNum, client->pers.netname  );

            // fire targets, ie. close doors.
            G_UseTargets( ent, &g_entities[ client - level.clients ] );
        }
    }

    ent->nextthink = level.time + 100;
}

void SP_vip_rescue( gentity_t *self ) {

    // grab 2 variables
    G_SpawnInt( "radius", "50", &self->splashRadius );
    G_SpawnInt( "team", "0", &self->ns_team );
    G_SpawnInt( "bconly", "0", &self->ns_flags );

    if ( g_gametype.integer != GT_LTS )
        return;
    if ( level.vipTime )
        return;

    if ( !self->splashRadius ) {
        self->splashRadius = 50;
        G_Printf("%s at %s with no radius set! Default: 50\n", self->classname,vtos(self->s.origin) );
    }

    G_Printf("spawning vip rescue for team %s\n", TeamName(self->ns_team+1) );

    if ( self->ns_team == 0 ) {
        level.vip[TEAM_RED] = VIP_ESCAPE;
    }
    else if ( self->ns_team == 1 ) {
        level.vip[TEAM_BLUE] = VIP_ESCAPE;
    }
    else if ( self->ns_team >= 2 ) {
        G_Error("%s: team > 1?", self->classname );
        return;
    }

    //	level.num_objectives[TEAM_RED]++;
    //	level.num_objectives[TEAM_BLUE]++;

    // set use stats
    self->think = info_vip_rescue_think;
    self->nextthink = level.time + 10;

    self->r.svFlags = SVF_NOCLIENT;

}
#define MAX_ASSAULT_FIELDS 64

qboolean finished_assault_fields[TEAM_NUM_TEAMS][MAX_ASSAULT_FIELDS];
int assault_fields[TEAM_NUM_TEAMS];

qboolean in_assault_field( gentity_t *ent, int playerNum )
{
    int			 e;
    gentity_t	*check;
    int			entityList[MAX_GENTITIES];
    int			listedEntities;

    listedEntities = trap_EntitiesInBox( ent->r.mins, ent->r.maxs, entityList, MAX_GENTITIES );

    // see if any solid entities are inside the final position
    for ( e = 0 ; e < listedEntities ; e++ ) {
        check = &g_entities[ entityList[ e ] ];

        // only check players
        if ( check->s.eType != ET_PLAYER && !check->physicsObject ) {
            continue;
        }
        if ( check->s.clientNum != playerNum )
            continue;
        if ( check->client->ps.pm_flags & PMF_FOLLOW )
            continue;

        // see if the ent needs to be tested
        if ( check->r.absmin[0] <= ent->r.maxs[0]
                && check->r.absmin[1] <= ent->r.maxs[1]
                && check->r.absmin[2] <= ent->r.maxs[2]
                && check->r.absmax[0] >= ent->r.mins[0]
                && check->r.absmax[1] >= ent->r.mins[1]
                && check->r.absmax[2] >= ent->r.mins[2] ) {
            return qtrue;
        }
    }
    return qfalse;
}

void assault_field_stop( gentity_t *ent )
{
    gentity_t *target;
    int i;

    // restart time.
    if ( ent->health < 4 )
        trap_SetConfigstring( CS_ASSAULT_START_TIME+ent->health, "0" );

    for ( i = 0; i < level.maxclients; i++ ) {

        ent->ns_linkedClientsTime[ i ] = 0;
        target = &g_entities[i];

        if (target->client && (target->client->linkedto == ent->health) ) {

            if ( !target->client->sess.waiting && target->client->linkedto == ent->health ){
                // he was in the field... sorry ass!
                target->client->ps.pm_flags &=~PMF_ASSAULTRANGE;
                target->client->ps.pm_flags &=~PMF_BLOCKED;
                target->client->linkedto = -1;
            } // he was a blocker... he blocked it good.
            else if ( !target->client->sess.waiting && ( target->client->ps.pm_flags & PMF_ASSAULTRANGE || target->client->ps.pm_flags & PMF_ASSAULTRANGE )  ) {
                target->client->ps.pm_flags &=~PMF_ASSAULTRANGE;
                target->client->ps.pm_flags &=~PMF_BLOCKED;
            }
        }

    } // for

    //BLUTENGEL_XXX: debug
#if ASSAULT_FIELD_DEBUG
    PrintMsg(NULL, "An assault field has been stopped %i\n", ent->health );
#endif

    // next time send CS update
    ent->fly_sound_debounce_time = 1;
    ent->nextthink = 0;
    trap_UnlinkEntity(ent);
}

void assault_field_stopall( void )
{
    int			i;
    gentity_t	*ent;

    //	G_Printf("Restarting all assaultfields...\n" );


    ent = &g_entities[0];
    for (i=0 ; i<level.num_entities ; i++, ent++) {
        if (!Q_stricmp("trigger_assaultfield", ent->classname))
            // this is an assaultfield
            assault_field_stop( ent );
    }
}


void assault_link_all( qboolean unlink )
{
    int			i;
    gentity_t	*ent;

    ent = &g_entities[0];
    for (i=0 ; i<level.num_entities ; i++, ent++)
    {
        if (!Q_stricmp("trigger_assaultfield", ent->classname))
        {	// this is an assaultfield
            if ( unlink )
                trap_UnlinkEntity( ent );
            else
                trap_LinkEntity( ent );
        }
    }
}

void assault_field_think( gentity_t *ent );

void assault_field_start( gentity_t *ent )
{
    int i = 0;
    //	G_Printf("Starting Asasault field. %i\n", ent->health );
    // reset all values for this assault field
    for ( i = 0; i < MAX_CLIENTS; i++ )
    {
        ent->ns_linkedClientsTime[i] = 0;
    }
    ent->fly_sound_debounce_time = 1;


    // just relink into world, and start thinking
    ent->think = assault_field_think;
    ent->nextthink = level.time + 1;

    trap_LinkEntity(ent);
}

#define ASSAULT_FIELD_DEBUG	0

void assault_field_think( gentity_t *ent )
{
    int i;
    int a;


    //	G_Printf("An assault thinks... %i - ", ent->health );

    for ( i = 0; i < level.maxclients; i++ )
    {
        gclient_t *client = &level.clients[i];
        int clientnum = g_entities[client-level.clients].s.clientNum;

        // if client does not exist
        if (!client)
            continue;
        // if client isn't connected
        if (client->pers.connected != CON_CONNECTED)
            continue;
        // if the client is not linked to the assaultfield in question
        if ( (client->linkedto != ent->health) && (client->linkedto != -1) )
            continue;
        // if the client isn't following another one
        if ( client->ps.pm_flags & PMF_FOLLOW )
            continue;
        // if the client is dead
        //if ( g_entities[i].r.contents == CONTENTS_CORPSE )
        //	continue;

        // bugfix; this handles a bug if the player was in an assaultfield at the end of a round.
        if ( client->sess.waiting )// || client->ps.stats[STAT_HEALTH] <= 0 )
        {
            if ( client->ps.pm_flags & PMF_ASSAULTRANGE ) {
                client->ps.pm_flags &=~ PMF_ASSAULTRANGE;
                client->ps.pm_flags &=~ PMF_BLOCKED;
                client->linkedto = -1;

                if ( ent->ns_linkedClientsTime[ client->ps.clientNum ] > 0 )
                {
                    if ( ent->health < 4 )
                        trap_SetConfigstring( CS_ASSAULT_START_TIME+ent->health, "0" );

                    // send next configstring attemp
                    ent->fly_sound_debounce_time = 1;
                }

                ent->ns_linkedClientsTime[ client->ps.clientNum ] = 0;

                //BLUTENGEL_XXX: debug
#if ASSAULT_FIELD_DEBUG
                PrintMsg(NULL, "Field %i(team:%s) left by %s (because dead or left range)\n", ent->health , TeamName( client->sess.sessionTeam ), client->pers.netname );
#endif
                // next time send configstring
            }
            //	G_Printf("(%i w-cl)");
            continue;
        }
        if ( client->ps.eFlags & EF_WEAPONS_LOCKED)
            continue;

        if ( !Is_OnBrushTeam( ent, &g_entities[ client-level.clients] ) ) {

            // other player is in this field , so disable all active timers.
            if ( in_assault_field(ent, client->ps.clientNum ) ) {

                for ( a = 0;  a < level.maxclients; a++ ) {
                    if ( ent->ns_linkedClientsTime[a] <= 0 )
                        continue;
                    //client is not a member of this field
                    if ( g_entities[a].client->linkedto != ent->health )
                        continue;

                    // it's a tango.
                    if ( Is_OnBrushTeam( ent, &g_entities[a] ) ) {
                        // disable time for the other clients
                        gclient_t *cl;

                        ent->ns_linkedClientsTime[ a ] = 0;

                        cl = &level.clients[a];

                        if ( ! ( g_entities[a].client->ps.pm_flags & PMF_BLOCKED) &&
                                ( g_entities[a].client->ps.pm_flags & PMF_ASSAULTRANGE)) {
                            cl->ps.pm_flags |= PMF_BLOCKED;
                            cl->ps.pm_flags &=~PMF_ASSAULTRANGE;

                            if ( ent->health < 4 )
                                trap_SetConfigstring( CS_ASSAULT_START_TIME + ent->health, "0" );

                            // next time send configstring
                            ent->fly_sound_debounce_time = 1;

#if ASSAULT_FIELD_DEBUG
                            PrintMsg(NULL, "Field %i(team:%s) entered by %s (otherteam, stopped counting)\n", ent->health , TeamName( client->sess.sessionTeam ), client->pers.netname );
#endif

                        } // if ( ! (...
                    }	// if ( Is_OnBrush...
                }		// for ( a = 0...

                client->ps.pm_flags |= PMF_ASSAULTRANGE;
            } // if ( in_assault...
            else
                client->ps.pm_flags &= ~PMF_ASSAULTRANGE;

            continue;
        }

        // we should be blocked.see if the blocker left...
        if ( client->ps.pm_flags & PMF_BLOCKED ) {

            qboolean someoneinmyfield = qfalse;

            for ( a = 0;  a < level.maxclients ; a++ ) {
                gclient_t *cl;

                cl = &level.clients[a];

                if ( !cl )
                    continue;

                if ( cl->pers.connected != CON_CONNECTED)
                    continue;
                if ( g_entities[a].r.contents == CONTENTS_CORPSE )
                    continue;
                if ( cl->sess.waiting )
                    continue;
                if ( cl->ps.pm_flags & PMF_FOLLOW )
                    continue;
                if ( g_entities[a].health <= 0 )
                    continue;

                //client is not a member of this field
                if ( g_entities[a].client->linkedto != ent->health )
                    continue;

                // skip client if same team, since we're looking for enemys in our field
                if ( client->sess.sessionTeam == cl->sess.sessionTeam )
                    continue;

                // if there is someone in my field
                if ( in_assault_field(ent, g_entities[a].client->ps.clientNum ) ) {

                    someoneinmyfield = qtrue; 	// then it sucks nuts, because we still can't capture
#if ASSAULT_FIELD_DEBUG
                    PrintMsg(NULL, "Field %i(team:%s) somebody in field of %s (otherteam, blocked)\n", ent->health , TeamName( client->sess.sessionTeam ), client->pers.netname );
#endif

                    continue;
                }
            } // for ( a = 0...

            if ( someoneinmyfield ) {
                continue;
            }
            else {
#if ASSAULT_FIELD_DEBUG
                PrintMsg(NULL, "Field %i(team:%s) field opened for %s (otherteam left, counting)\n", ent->health , TeamName( client->sess.sessionTeam ), client->pers.netname );
#endif

                client->ps.pm_flags &= ~PMF_BLOCKED;
                client->ps.pm_flags |= PMF_ASSAULTRANGE;
                continue;
            }

        } // if ( client->ps.pm_flags

        // first time activated
        if ( ent->ns_linkedClientsTime[ clientnum ] <= 0 &&
                in_assault_field(ent, g_entities[client-level.clients].s.clientNum ) &&
                !(client->ps.pm_flags & PMF_BLOCKED) ) {

            qboolean failed = qfalse;

#if ASSAULT_FIELD_DEBUG
            PrintMsg(NULL, "Field %i(team:%s) tapped by by %s\n", ent->health , TeamName( client->sess.sessionTeam ), client->pers.netname );
#endif
            // before letting him in see if that biatch is valid
            for ( a = 0;  a < level.maxclients; a++ ) {
                gclient_t *cl;

                cl = &level.clients[a];

                if ( !cl )
                    continue;

                if ( cl->pers.connected != CON_CONNECTED)
                    continue;

                if ( cl->sess.waiting )
                    continue;

                if ( cl->sess.sessionTeam == client->sess.sessionTeam )
                    continue;

                if ( g_entities[a].client->ps.pm_flags & PMF_FOLLOW )
                    continue;

                if ( g_entities[a].health <= 0 )
                    continue;

                //client is not a member of this field
                if ( g_entities[a].client->linkedto != ent->health )
                    continue;

                //client is not a member of this field
                if ( !Is_OnBrushTeam( ent, &g_entities[a] ) ) {

                    // disable time for the other clients
                    if ( in_assault_field(ent, a ) ) {
                        client->ps.pm_flags &= ~PMF_ASSAULTRANGE;
                        client->ps.pm_flags |= PMF_BLOCKED;
                        failed = qtrue;
                        continue; // break
                    }

                }
            } // for ( a = 0...

            if ( failed )
                continue;

            if ( !failed ) {
                // set assault flag for client hud
                client->ps.pm_flags |= PMF_ASSAULTRANGE;
                client->linkedto = ent->health;
                ent->ns_linkedClientsTime[ clientnum ] = level.time + ent->wait * ONE_SECOND;
            }
        } // if ( ent->ns_linkedClients...
        if ( !(client->ps.pm_flags & PMF_ASSAULTRANGE) )
            continue;

        if ( client->ps.pm_flags & PMF_BLOCKED )
            continue;

        if ( client->linkedto != ent->health )
            continue;

        // if player died and had assaultrange setted:
        if ( client->ps.stats[STAT_HEALTH] <= 0 ||
                !in_assault_field(ent, g_entities[client-level.clients].s.clientNum )  ) {

            // reset time and disable client
            if ( client->ps.pm_flags & PMF_ASSAULTRANGE ) {
                client->ps.pm_flags &=~ PMF_ASSAULTRANGE;
                client->linkedto = -1;
                ent->ns_linkedClientsTime[ client->ps.clientNum ] = 0;
                if ( ent->health < 4 )
                    trap_SetConfigstring( CS_ASSAULT_START_TIME+ent->health, "0" );
                // next time send configstring
                ent->fly_sound_debounce_time = 1;
            }
            continue;
        }

        // another one was also waiting in the field. active HIS time
        if ( ent->fly_sound_debounce_time )
        {
            if ( ent->health < 4 )
                trap_SetConfigstring( CS_ASSAULT_START_TIME + ent->health , va("%i",ent->ns_linkedClientsTime[ client->ps.clientNum ]) );
            ent->fly_sound_debounce_time = 0;
        }
        // broke all limits
        if ( level.time > ent->ns_linkedClientsTime[ clientnum ] )
        {
            trap_SendServerCommand( -1, va("cp \"The %s captured Field %i\n\"", TeamName(g_entities[clientnum].client->sess.sessionTeam), ent->health+1 ));


            if ( ent->count == 1 )
            {
                int a;
                int tapped = 0;

                finished_assault_fields[client->sess.sessionTeam][ent->health] = qtrue;

                // check if all fields have been tapped
                for ( a = 0; a < level.fields[ client->sess.sessionTeam ]; a++ )
                {
                    if (finished_assault_fields[client->sess.sessionTeam][a] )
                        tapped++;
                }

                if ( tapped == level.fields[ client->sess.sessionTeam ] )
                {
                    for ( a = 0; a < level.fields [ client->sess.sessionTeam ]; a++ )
                    {
                        // set state of the field to untapped
                        finished_assault_fields[ client->sess.sessionTeam ][ ent->health ] = qfalse;
                    }
                    //	G_Printf("Team-Fields %i(team:%s) all finished by %s\n", ent->health , TeamName( client->sess.sessionTeam ), client->pers.netname );
                    G_UseTargets (ent, &g_entities[ client - level.clients ] );
                }
                else
                {
                    //	G_Printf("Team-Fields %i(team:%s) tapped by %s\n", ent->health , TeamName( client->sess.sessionTeam ), client->pers.netname );
                }
            }
            else
            {
                //	G_Printf("Field %i(team:%s) captured by %s\n", ent->health , TeamName( client->sess.sessionTeam ), client->pers.netname );
                // mark one objective as done
                if ( ent->bot_chattime )
                    level.done_objectives [ client->sess.sessionTeam ]++;
                G_UseTargets (ent, &g_entities[ client - level.clients ] );
            }

            // disable timer
            if ( ent->health < 4 )
                trap_SetConfigstring( CS_ASSAULT_START_TIME + ent->health, "taken" ); // means taken

            // taken one assault field
            if ( level.clients[i].ns.rewards & REWARD_3ASSAULT_TAKEN ) // 3 taken?
                client->ns.rewards |= REWARD_4ASSAULT_TAKEN;	  // add 4 taken
            else if ( level.clients[i].ns.rewards & REWARD_2ASSAULT_TAKEN ) // 2 taken?
                client->ns.rewards |= REWARD_3ASSAULT_TAKEN;	  // add 3 taken
            if ( level.clients[i].ns.rewards & REWARD_ASSAULT_TAKEN )
                client->ns.rewards |= REWARD_2ASSAULT_TAKEN; // add 2 taken
            else
                client->ns.rewards |= REWARD_ASSAULT_TAKEN;  // taken

            G_LogPrintf( "OBJECTIVE: [%i] \"%s\" captured assault field %i\n",
                         client->ps.clientNum, client->pers.netname , ent->health+1 );

            assault_field_stop( ent );

            return;
        }
    }

    ent->think = assault_field_think;
    ent->nextthink = level.time + 500;
}


int num_assaultfields_for_team( int team )
{
    int			i;
    gentity_t	*ent;
    int			num = 0;

    ent = &g_entities[0];
    for (i=0 ; i<level.num_entities ; i++, ent++) {

        if (!Q_stricmp("trigger_assaultfield", ent->classname))
        {
            if (ent->ns_team == BRUSH_TEAM_RED && team == TEAM_RED)
                num++;
            else if (ent->ns_team == BRUSH_TEAM_BLUE && team == TEAM_BLUE)
                num++;
        }
        else
            continue;
    }
    return num;
}
void SP_assault_field( gentity_t *self )
{
    int all = 0;
    char	*s;
    int	addmission;
    int	optional = 0;

    // grab variables
    G_SpawnInt( "all", "0", &all);
    G_SpawnInt( "radius", "15", &self->splashRadius );
    G_SpawnString( "fieldname", "unknown", &s  );
    G_SpawnFloat( "time", "10", &self->wait );
    G_SpawnInt( "team", "0", &self->ns_team );
    G_SpawnInt( "optional", "0", &optional);
    G_SpawnInt( "addmission", "1", &addmission);

    if ( g_gametype.integer != GT_LTS )
        return;

    //	G_Printf("spawning assault field for team %s\n", TeamName(self->ns_team) );

    InitTrigger (self);

    if ( all )
        self->count = 1;
    else if ( optional == 0 )
    {
        self->count = 0;

        // setup an objective
        if ( self->ns_team == BRUSH_TEAM_RED )
            level.num_objectives[TEAM_RED]++;
        else if ( self->ns_team == BRUSH_TEAM_BLUE )
            level.num_objectives[TEAM_BLUE]++;
    }

    self->bot_chattime = addmission;
    self->ns_brushname = s ;

    // num field
    if ( self->ns_team == BRUSH_TEAM_RED )
        self->health = level.fields[TEAM_RED];
    else if ( self->ns_team == BRUSH_TEAM_BLUE )
        self->health = level.fields[TEAM_BLUE];

    if ( self->ns_team == BRUSH_TEAM_RED )
    {
        level.fields[TEAM_RED]++;
    }
    else if ( self->ns_team == BRUSH_TEAM_BLUE )
        level.fields[TEAM_BLUE]++;
    else
        G_Error("Wrong Team for %s\n", self->classname );

    if ( level.fields[TEAM_RED] >= MAX_ASSAULT_FIELDS || level.fields[TEAM_BLUE] >= MAX_ASSAULT_FIELDS )
        G_Error("Reached Max_Fields.\n");

    for ( all = 0;  all < MAX_CLIENTS; all++ )
    {
        self->ns_linkedClientsTime[all] = 0;
    }
    // send configstring next time
    self->fly_sound_debounce_time = 1;
    trap_LinkEntity (self);
}

/*
=================
SpawnFlare

=================
*/
void NS_SpawnFlare ( gentity_t *self ) {
    // grab variables
    G_SpawnInt( "size", "15", &self->s.frame );

    G_SpawnInt( "r", "100", &self->s.generic1 );		// red
    G_SpawnInt( "g", "100", &self->s.otherEntityNum );		// green
    G_SpawnInt( "b", "100", &self->s.otherEntityNum2 );	// blue

    G_SpawnInt( "corona", "0", &self->s.eventParm ); // blue

    //	G_Printf("spawning flare\n" );

    self->s.eType = ET_FLARE;
    trap_LinkEntity (self);

}


void doorlock_die ( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
    gentity_t *target;

    target = self->target_ent;

    if (!target)
        return;

    // door is unlocked now.
    target->pain_debounce_time = qfalse;

    PrintMsg( attacker, "This door has been " S_COLOR_GREEN "opened" S_COLOR_WHITE ".\n");

    G_TempEntity( self->s.pos.trBase , EV_BREAKLOCK );

    G_FreeEntity( self );
}
void doorlock_use  (gentity_t *self, gentity_t *other, trace_t *trace )
{
    gentity_t *target;


    if (!other->client)
        return;

    if ( !(other->client->buttons & BUTTON_USE) )
        return;

    target = self->target_ent;

    if (!target)
        return;

    if ( other->client->sess.sessionTeam == TEAM_RED && self->ns_team == 1 )
    {
        PrintMsg( other, "This door has been " S_COLOR_GREEN "opened" S_COLOR_WHITE ".\n");

        // door is unlocked now.
        target->pain_debounce_time = qfalse;
    }
    else if ( other->client->sess.sessionTeam == TEAM_BLUE && self->ns_team == 2 )
    {
        PrintMsg( other, "This door has been " S_COLOR_GREEN "opened" S_COLOR_WHITE ".\n");

        // door is unlocked now.
        target->pain_debounce_time = qfalse;
    }
    else
        return;
}

/*
======================
SpawnLockTrigger

======================
*/
/*
================
Touch_DoorTrigger
================
*/
void Touch_LockTrigger( gentity_t *ent, gentity_t *other, trace_t *trace ) {
    if ( other->client->buttons & BUTTON_USE && !(other->client->oldbuttons & BUTTON_USE) )
    {
        NS_PlayerAnimation( other, TORSO_OPEN_DOOR, 750, qfalse );
    }
    doorlock_use( ent->parent , other, trace );
    //	ent->parent->touch(ent,other,trace);
}
void Think_SpawnNewDoorTrigger ( gentity_t *ent );

void SpawnLockTrigger( gentity_t *ent ) {
    gentity_t		*other;
    vec3_t		mins, maxs;
    int			i, best;

    // set all of the slaves as shootable
    ent->target_ent = G_PickTarget( ent->target );

    if ( ent->target_ent )
    {
        ent->target_ent->pain_debounce_time = qtrue; // this door is locked
        Think_SpawnNewDoorTrigger	( ent->target_ent );
    }

    // find the bounds of everything on the team
    VectorCopy (ent->r.absmin, mins);
    VectorCopy (ent->r.absmax, maxs);

    // find the thinnest axis, which will be the one we expand
    best = 0;
    for ( i = 1 ; i < 3 ; i++ ) {
        if ( maxs[i] - mins[i] < maxs[best] - mins[best] ) {
            best = i;
        }
    }
    maxs[best] += 20;
    mins[best] -= 20;

    // create a trigger with this size
    other = G_Spawn ();
    VectorCopy (mins, other->r.mins);
    VectorCopy (maxs, other->r.maxs);
    other->parent = ent;
    other->classname = "lock_trigger";
    other->r.contents = CONTENTS_TRIGGER;
    other->touch = Touch_LockTrigger;
    // remember the thinnest axis
    other->count = best;
    trap_LinkEntity (other);

}
// returns true if the entity can be damaged
qboolean doorlock_damage( gentity_t *ent , int attackerweapon)
{
    int locktype = ent->ns_flags;

    if (!ent)
        return qfalse;

    if ( BG_IsGrenade( attackerweapon ) )
        return qtrue;

    if ( locktype == 3 && !BG_IsShotgun( attackerweapon) )
        return qfalse;
    else if ( locktype == 2 && !BG_IsRifle( attackerweapon ) && !BG_IsShotgun( attackerweapon ) )
        return qfalse;
    else if ( locktype == 1 && !BG_IsSMG( attackerweapon ) && !BG_IsRifle( attackerweapon ) && !BG_IsShotgun( attackerweapon ) )
        return qfalse;
    else if ( BG_IsMelee( attackerweapon ) )
        return qfalse;

    return qtrue;
}

void SP_doorlock_spawn ( gentity_t *ent )
{
    char *test;
    int  locktype;
    float	mYaw,mPitch,mRoll;

    G_SpawnInt( "team", "0", &ent->ns_team ); // 0 = noteam | 1=open for seals | 2=open for tangos
    G_SpawnString( "target", "none", &test );
    G_SpawnInt( "locktype", "0", &locktype ); // 0 light 1 medium 2 heavy 3 shotgun only

    G_SpawnFloat( "yaw", "0", &mYaw );
    G_SpawnFloat( "pitch", "0", &mPitch );
    G_SpawnFloat( "roll", "0", &mRoll );


    if ( locktype == 3 )
        ent->s.modelindex = G_ModelIndex( "models/misc/locks/shotgun.md3" );
    else if ( locktype == 2 )
        ent->s.modelindex = G_ModelIndex( "models/misc/locks/heavy.md3" );
    else if ( locktype == 1 )
        ent->s.modelindex = G_ModelIndex( "models/misc/locks/medium.md3" );
    else
        ent->s.modelindex = G_ModelIndex( "models/misc/locks/light.md3" );

    ent->s.eType = ET_GENERAL;

    VectorClear ( ent->s.pos.trDelta );

    G_SetOrigin( ent, ent->s.origin );
    VectorCopy( ent->s.origin, ent->s.pos.trBase );

    VectorSet( ent->s.angles , mPitch, mYaw, mRoll );

    VectorCopy( ent->s.angles, ent->s.apos.trBase );

    ent->die = doorlock_die;
    ent->touch = doorlock_use;

    ent->health = 50;
    //c4->ns_team = ent->client->sess.sessionTeam;
    ent->s.number = ent - g_entities;

    VectorSet (ent->r.mins, -2, -1, -2);
    VectorSet (ent->r.maxs, 2, 1, 2);
    ent->r.contents = MASK_SHOT;

    ent->takedamage = qtrue;
    ent->s.pos.trType = TR_STATIONARY;
    ent->s.pos.trTime = level.time;
    ent->ns_flags = locktype;

    ent->physicsObject = qtrue;

    trap_LinkEntity( ent );

    ent->think = SpawnLockTrigger;
    ent->nextthink = level.time + 2000;

}
#if 0
void G_ActorIdle( gentity_t *ent )
{
    if ( ent-> actorState != ACTOR_IDLE )
        G_Error("Actor: In idle without permission\n");

}
void G_ActorChase( gentity_t *ent )
{
    gentity_t *other;
    vec3_t ownerPos;
    float distance;
    vec3_t	origin;
    vec3_t	movedir;
    vec3_t	neworigin;
    float	speed = ent->speed;

    if ( ent-> actorState != ACTOR_CHASE )
        G_Error("Actor: In chase without permission\n");

    if ( !ent->parent )
        G_Error("Actor: In chase without parent\n");

    VectorCopy( ent->s.origin , origin );

    other = ent->parent;

    if ( other->r.svFlags & SVF_USE_CURRENT_ORIGIN )
        VectorCopy( other->r.currentOrigin, ownerPos );
    else if ( !other->client )
        VectorCopy( other->s.origin , ownerPos );
    else if ( other->client )
        VectorCopy( other->client->ps.origin , ownerPos );

    distance = Distance( ownerPos, origin );

    VectorSubtract( origin,ownerPos,  movedir );

    VectorMA( origin , speed, movedir, neworigin );

#if 1
    {
        gentity_t *temp;

        temp = G_TempEntity( origin, EV_DEBUG_LINE );

        VectorCopy( origin, temp->s.pos.trBase );
        VectorCopy( neworigin, temp->s.origin2 );
    }
#endif
    G_Printf("Actor Chase s:%i d:%2f\n", speed, distance );
    VectorCopy( neworigin, ent->s.origin );
}

void G_RunActor( gentity_t *ent )
{
    // actor main thinking function
    if ( ent-> actorState >= MAX_ACTOR_STATES )
        G_Error("Actor: Illegal State\n");

    // ACTOR_IDLE - actor idles
    // ACTOR_CHASE - chases entity
    // ACTOR_OUTOFSIGHT - lost entity out of sight
    // ACTOR_PANIC - actor panics, runs around weierd or just stands still or ducks
    // ACTOR_DEAD - actor is dead
    G_Printf("Actor o:%s s:%i\n", vtos ( ent->s.origin ), ent-> actorState );

    switch ( ent-> actorState )
    {
    case ACTOR_IDLE:
        G_ActorIdle( ent );
        break;
    case ACTOR_CHASE:
        G_ActorChase( ent );
        break;
    case ACTOR_OUTOFSIGHT:
        break;
    case ACTOR_PANIC:
        break;
    case ACTOR_DEAD:
        break;
    default:
        break;
    }
}
#endif
void TestActor ( vec3_t origin , int clientNum )
{
    gentity_t *ent;

    ent = G_Spawn();

    // 	ent->s.eType = ET_ACTOR;

    //G_SetOrigin( ent, origin );
    /*
    VectorSet (ent->r.mins, -6, -6, -6);
    VectorSet (ent->r.maxs, 6, 6, 6);
    ent->r.contents = MASK_SHOT;
    */

    VectorCopy(  origin , ent->s.origin);

    //ent->takedamage = qtrue;
    ent->s.pos.trType = TR_INTERPOLATE;
    ent->s.pos.trTime = level.time;


    ent->speed = 50;
    //	ent->actorState = ACTOR_CHASE;



    ent->parent = &g_entities[clientNum];

    //ent->physicsObject = qtrue;

    trap_LinkEntity( ent );

}



//==========================================================

#define PARTFLAGS_DIRT			1
#define	PARTFLAGS_GLASS			2
#define PARTFLAGS_SAND			4
#define PARTFLAGS_SPARKS		8
#define PARTFLAGS_METALSPARKS	16
#define PARTFLAGS_SMOKE			32
#define PARTFLAGS_STARTOFF		64

/*QUAKED target_particle (1 0 0) (-8 -8 -8) (8 8 8) DIRT GLASS SAND SPARKS METALSPARKS SMOKE STARTOFF
"random"		randomizes amount of particles it spawns (full numbers).
"randomdir"		randomizes velocity of each particle ( x/y NOT Z(up) ) (full numbers).
"velocity"		velocity (full numbers).
"particles"		amount of particles randomized by 'random' key (full numbers).
"size"			size modifier (only for smoke) (full numbers).
"dlight"		add a dynamic light (0=none)-(255=biggest dlight) (full numbers).
"wait"			fire particles every X milliseconds.
"stop"			it stops spawning particles X milliseconds after it's been targeted.

It will spawn X particles( and a random amount between 0 and 'random' key ) every X seconds.
Can be targeted to be toggled on/off.
To set the type of particles spawned use the SPAWNFLAGS.
*/
void target_Particle_think (gentity_t *ent )
{
    // just toggle it off
    if ( ent->r.svFlags & SVF_NOCLIENT )
        ent->r.svFlags &=~SVF_NOCLIENT;
    else
        ent->r.svFlags |= SVF_NOCLIENT;
}

void Use_Target_Particle (gentity_t *ent, gentity_t *other, gentity_t *activator)
{
    // toggle it off
    if ( ent->r.svFlags & SVF_NOCLIENT )
        ent->r.svFlags &=~SVF_NOCLIENT;
    else
        ent->r.svFlags |= SVF_NOCLIENT;

    if ( ent->wait )
    {
        ent->think = target_Particle_think ;
        ent->nextthink = level.time + ent->wait;
    }
}

void G_ParticleSetup ( gentity_t *ent )
{
    ent->r.svFlags &=~SVF_NOCLIENT;

    // check for prestarted looping sound
    if (ent->target) {
        gentity_t *target = G_Find (NULL, FOFS(targetname), ent->target);
        if (!ent || !target) {
            G_Error ("%s at %s: %s is a bad target\n", ent->classname, vtos(ent->s.origin), ent->target);
        }
        else
            VectorSubtract( target->s.origin, ent->s.origin, ent->s.angles );
    }
}

// this entity is completly client-side except if it should get triggered
void SP_target_Particle( gentity_t *ent )
{
#ifdef PARTICLEHOST
    G_SpawnInt( "random", "4", &ent->s.powerups );
    G_SpawnInt( "randomdir", "2", &ent->s.time2 );
    G_SpawnInt( "velocity", "50", &ent->s.torsoAnim );
    G_SpawnInt( "particles", "10", &ent->s.generic1 );
    G_SpawnInt( "size", "0", &ent->s.legsAnim );
    G_SpawnInt( "dlight", "0", &ent->s.otherEntityNum2 );
    G_SpawnInt( "wait", "500", &ent->s.time );
    G_SpawnFloat( "stop", "0", &ent->wait );

    // set default.
    if ( ent->spawnflags == 0 )
        ent->spawnflags |= PARTFLAGS_SMOKE;

    ent->classname = "particle_host";

    ent->s.eType = ET_PARTICLEHOST;
    ent->s.eventParm = ent->spawnflags;
    ent->r.svFlags |= SVF_NOCLIENT;

    ent->use = Use_Target_Particle;

    if ( ent->spawnflags & PARTFLAGS_STARTOFF )
        ent->r.svFlags |= SVF_NOCLIENT;

    VectorCopy( ent->s.origin, ent->s.pos.trBase );

    ent->think = G_ParticleSetup;
    ent->nextthink = level.time + 1000;
#endif
    // must link the entity so we get areas and clusters so
    // the server can determine who to send updates to
    trap_LinkEntity( ent );
}

#define MXP_STAMINA		1
#define MXP_SPEED		2
#define	MXP_ACCURACY	4
#define	MXP_STEALTH		8
#define MXP_TECHNICAL	16
#define	MXP_STRENGTH	32

/*QUAKED target_modifyxp (1 0 0) (-8 -8 -8) (8 8 8) STAMINA SPEED ACCURACY STEALTH TECHNICAL STRENGTH
"setlevel" sets level of the selected abilities of the activator to a number
*/
void Use_ModifyXP (gentity_t *ent, gentity_t *other, gentity_t *activator) {
    int spawnflags = ent->spawnflags;
    int	level = ent->s.time;

    if ( !activator || !activator->client )
        return;

    if ( spawnflags & MXP_STAMINA )
        activator->client->pers.nsPC.stamina = level;

    if ( spawnflags & MXP_SPEED )
        activator->client->pers.nsPC.speed = level;

    if ( spawnflags & MXP_ACCURACY )
        activator->client->pers.nsPC.accuracy = level;

    if ( spawnflags & MXP_STEALTH )
        activator->client->pers.nsPC.stealth = level;

    if ( spawnflags & MXP_TECHNICAL )
        activator->client->pers.nsPC.technical = level;

    if ( spawnflags & MXP_STRENGTH )
        activator->client->pers.nsPC.strength = level;

}

void SP_target_modifyxp ( gentity_t *ent ) {
    G_SpawnInt( "setlevel", "0", &ent->s.time );

    if ( g_gametype.integer != 0 )
    {
        G_FreeEntity(ent);
        return;
    }


    ent->r.svFlags |= SVF_NOCLIENT;
    ent->use = Use_ModifyXP;
}


qboolean is_func_explosive ( gentity_t *ent )
{
    if ( !ent || !ent->classname )
        return qfalse;

    if (
        !Q_stricmp( ent->classname , "func_explosive_glass" ) ||
        !Q_stricmp( ent->classname , "func_explosive_wood" ) ||
        !Q_stricmp( ent->classname , "func_explosive_metal" ) ||
        !Q_stricmp( ent->classname , "func_explosive_stone" ) ||
        !Q_stricmp( ent->classname , "func_explosive" )
    )
        return qtrue;
    return qfalse;
}
/*
==============================================================================
*/

/*QUAKED func_explosive (.5 .5 .5) IS_WOOD IS_METAL

If targetname set, it will not be destroyable
To make it spawn an explosive let it trigger
default is: glass
an trigger_explosive

"target"	Target to trigger when destroyed
"targetname"	If set not destroyable
"health"	Health value


*/
void model_die ( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath );

void func_explosive_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath )
{
    // spawn modelparts
    model_die( self, inflictor, attacker, damage, meansOfDeath );

    // set invisible
    self->r.svFlags |= SVF_NOCLIENT;
    self->r.contents = 0;
    self->takedamage = qfalse;

    // link back into world
    trap_LinkEntity( self );

    // this has to be the last call, because else it will crash because
    // the engine is trynig to spawn a tempentity inside a brush
    // this is fatal and causes: G_NoFreeEntities
    G_UseTargets (self, attacker);
}

void func_explosive_use (gentity_t *self, gentity_t *other, gentity_t *activator )
{
    // just call the die function as it's the same and make sure that it's beeing killed.

    self->die ( self, other, activator, 1000, 0 );
}

void Touch_GlassTrigger (gentity_t *ent, gentity_t *other, trace_t *trace)
{
    gentity_t *glass = ent->parent;
    float length;
    vec3_t velocity;

    if ( !ent->parent && is_func_explosive(ent) )
        glass = ent;

    if ( other->client )
        VectorCopy( other->client->ps.velocity, velocity );

    if ( !other->client )
    {
        BG_EvaluateTrajectoryDelta( &other->s.pos, level.time, velocity );
    }

    if ( other->r.maxs[2] >= glass->r.maxs[2] - 2 )
    {// can 1be broken when stood upon

        //		G_Printf("broken because you stood on!\n");
        // play creaking sound here.
        //DamageSound();
        func_explosive_die( glass, other, other, glass->health, 0 );
        glass->fly_sound_debounce_time = qfalse;

        return;
    }

    length = VectorLength( velocity );

    if ( glass->health - ( length / 2) <= 0 )
    {
        func_explosive_die( glass, other, other, glass->health, 0 );
        glass->fly_sound_debounce_time = qfalse;
    }
}
void SpawnGlassTrigger( gentity_t *ent ) {
    gentity_t		*other;
    vec3_t		mins, maxs;
    int			i, best;

    // find the bounds of everything on the team
    VectorCopy (ent->r.absmin, mins);
    VectorCopy (ent->r.absmax, maxs);

    // find the thinnest axis, which will be the one we expand
    best = 0;
    for ( i = 1 ; i < 3 ; i++ ) {
        if ( maxs[i] - mins[i] < maxs[best] - mins[best] ) {
            best = i;
        }
    }
    maxs[best] += 0.5f;
    mins[best] -= 0.5f;

    // create a trigger with this size
    other = G_Spawn ();
    VectorCopy (mins, other->r.mins);
    VectorCopy (maxs, other->r.maxs);
    other->parent = ent;
    other->classname = "glass_trigger";
    other->r.contents = CONTENTS_TRIGGER;
    other->touch = Touch_GlassTrigger;
    // remember the thinnest axis
    other->count = best;

    trap_LinkEntity (other);
}

void SP_func_explosive (gentity_t *ent)
{
    G_SpawnInt( "health", "50", &ent->health );
    G_SpawnInt( "hits", "4", &ent->ns_flags );
    G_SpawnInt( "nomark", "0", &ent->fly_sound_debounce_time );

    InitMover( ent );
    ent->s.eType = ET_FUNCEXPLOSIVE;

    // classnames
    if ( ent->spawnflags & 1 ) {
        ent->classname = "func_explosive_wood";
    }
    else if ( ent->spawnflags & 2 ) {
        ent->classname = "func_explosive_metal";
    } else if ( ent->spawnflags & 4 ) {
        ent->classname = "func_explosive_stone";
    } else
        ent->classname = "func_explosive_glass";

    ent->timestamp = ent->ns_flags;

    if ( ent->targetname )
    {
        ent->use = func_explosive_use;
        ent->takedamage = qfalse;
        //		G_Printf("%s got target name... Can't shoot!\n", ent->classname );
    }
    else
    {
        ent->takedamage = qtrue;

        if ( ent->spawnflags & 8 )
            ; // other touch function?
        else
        {
            if ( !Q_stricmp( ent->classname, "func_explosive_glass") )
            {
                ent->touch = Touch_GlassTrigger;
                SpawnGlassTrigger( ent );
            }
        }
    }

    ent->die = func_explosive_die;
    ent->count = ent->health;

    trap_SetBrushModel( ent, ent->model );
}

/*QUAKED func_elevator_button (.5 .5 .5)

Button for the elevator

"target"	Target to the elvator
"team"    2 = for all, 0 = only for seals, 1 = only for tangos
"elevator_index" index of the elevator level where the elevator shall go after beeing triggered (0 - 7)


*/
void Touch_Elevator_Button(gentity_t *ent, gentity_t *other, trace_t *trace ) {
    trace_t tr;
    vec3_t end, forward, up, right, start;
    if ( !other->client ) {
        return;
    }
    // if game is open
    //if (GameState == STATE_OPEN && g_gametype.integer == GT_LTS)
    //	return;

    // e 2 - means free
    if (ent->ns_team != 2 && !Is_OnBrushTeam(ent,other) )
        return;

    if ( other->client && !( other->client->buttons & BUTTON_USE ) )
        return;

    if ( other->client && ( other->client->buttons & BUTTON_USE) && (other->client->oldbuttons & BUTTON_USE)) return;

    VectorCopy(other->client->ps.viewangles, end);
    AngleVectors(end, forward, right, up);
    VectorCopy(other->client->ps.origin, start);
    start[2] += other->client->ps.viewheight;
    VectorMA(start, 32, forward, end);
    VectorMA(start, -32, forward, start);

    trap_Trace( &tr, start, NULL, NULL, end, other->s.number, MASK_ALL);

    if (tr.entityNum == ent->s.number) G_UseTargets( ent, NULL);
}

void SP_func_elevator_button( gentity_t *ent ) {

    G_SpawnInt("team","2", &ent->ns_team);
    G_SpawnInt("elevator_index", "0", &ent->elevator_index);

    trap_SetBrushModel( ent, ent->model );

    InitMover(ent);

    VectorCopy( ent->s.origin, ent->s.pos.trBase );
    VectorCopy( ent->s.origin, ent->r.currentOrigin );


    ent->touch = Touch_Elevator_Button;

    ent->r.contents = CONTENTS_TRIGGER;

    ent->r.mins[0] -= 1.0;
    ent->r.mins[1] -= 1.0;
    ent->r.mins[2] -= 1.0;

    ent->r.maxs[0] += 1.0;
    ent->r.maxs[1] += 1.0;
    ent->r.maxs[2] += 1.0;

    ent->classname = "func_elevator_button";
    switch (ent->elevator_index) {
    case 0: ent->s.eType = ET_ELEVBUT0; break;
    case 1: ent->s.eType = ET_ELEVBUT1; break;
    case 2: ent->s.eType = ET_ELEVBUT2; break;
    case 3: ent->s.eType = ET_ELEVBUT3; break;
    case 4: ent->s.eType = ET_ELEVBUT4; break;
    case 5: ent->s.eType = ET_ELEVBUT5; break;
    case 6: ent->s.eType = ET_ELEVBUT6; break;
    case 7: ent->s.eType = ET_ELEVBUT7; break;
    default: ent->s.eType = ET_ELEVBUT0;
    }

    trap_LinkEntity(ent);

}


