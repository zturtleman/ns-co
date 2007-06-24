#include "cg_local.h"


//
// all code in this file is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

/*
=============
VectorToString

This is just a convenience function
for printing vectors
=============
*/
char	*vtos( const vec3_t v ) {
    static	int		index;
    static	char	str[8][32];
    char	*s;

    // use an array so that multiple vtos won't collide
    s = str[index];
    index = (index + 1)&7;

    Com_sprintf (s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

    return s;
}


#define	CRATE_PARTICLES	250

/*
===================
NSQ3 CG_LaunchParticle
by: dX
date:  6.feb.2000
function: Generates One particle (model) with the given values
===================
*/

void CG_LaunchParticle( vec3_t origin, vec3_t baseangle, vec3_t dir, qhandle_t hModel, int sound, int bouncefactor ) {
    localEntity_t	*le;
    refEntity_t		*re;

    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    le->leType = LE_FRAGMENT;
    le->startTime = cg.time;
    le->endTime = le->startTime + cg_particleTime.integer ;


    VectorCopy( origin, re->origin );
    AxisCopy( axisDefault, re->axis );
    re->hModel = hModel;

    le->pos.trType = TR_GRAVITY;
    VectorCopy( origin, le->pos.trBase );
    le->pos.trTime = cg.time + 100;
    le->leFlags = LEF_TUMBLE;
    le->angles.trType = TR_GRAVITY;
    le->angles.trTime = cg.time + 50;

    le->angles.trBase[0] = baseangle[0];
    le->angles.trBase[1] = baseangle[1];
    le->angles.trBase[2] = baseangle[2];

    le->angles.trDelta[0] = rand() % 150;
    le->angles.trDelta[1] = rand() % 150;
    le->angles.trDelta[2] = rand() % 150;

    re->rotation = rand() % 360;

    //	VectorCopy( dir, le->pos.trDelta );
    if (dir)
        VectorScale( dir, 140 + random()*30, le->pos.trDelta );

    if (bouncefactor == BOUNCE_LIGHT)
        le->bounceFactor = 0.2f;
    else if (bouncefactor == BOUNCE_MEDIUM)
        le->bounceFactor = 0.4f;
    else if (bouncefactor == BOUNCE_HEAVY)
        le->bounceFactor = 0.6f;


    le->leMarkType = LEMT_NONE;

}

/*
================
NSQ3 CG_BleederTrail
by: dX
date: 14-feb-2k
desc: Mostly the same as CG_Bloodtrail, but with smaller drops
================
*/
void CG_BleederTrail( localEntity_t *le ) {
    int		t;
    int		t2;
    int		step;
    vec3_t	newOrigin;
    localEntity_t	*blood;

    if (!cg_blood.integer)
        return;

    step = 20; // original *=3
    t = step * ( (cg.time - cg.frametime + step ) / step );
    t2 = step * ( cg.time / step );

    for ( ; t <= t2; t += step ) {
        BG_EvaluateTrajectory( &le->pos, t, newOrigin );


        blood = CG_SmokePuff( newOrigin, vec3_origin,
                              2,		// radius : original = 20
                              1, 1, 1, 0.4f,	// color
                              250,		// trailTime [²000 = 2scs]
                              cg.time,		// startTime
                              0,0,		// flags
                              cgs.media.bloodExplosionShader );
        // use the optimized version
        blood->leType = LE_FALL_SCALE_FADE;
        // drop a total of 40 units over its lifetime
        blood->pos.trDelta[2] = 40;
    }
}

/*
================
NSQ3 CG_SparkTrail
by: dX
date: 14-feb-2k
desc: Creates sparks
================
*/
void CG_SparkTrail( localEntity_t *le ) {
    int num,i;
    vec3_t	origin;

    num = (int)(random()*4);

    for (i=0;i<num;i++)
    {
        vec3_t dir;
        refEntity_t		*re;

        dir[0] = crandom();
        dir[1] = crandom();
        dir[2] = crandom();

        BG_EvaluateTrajectory( &le->pos, cg.time, origin );

        le = CG_AllocLocalEntity();
        re = &le->refEntity;

        VectorCopy( origin, le->pos.trBase );
        VectorScale(dir, 150 + crandom()*30, le->pos.trDelta);
        le->pos.trType = TR_GRAVITY;
        le->pos.trTime = cg.time;
        le->leType = LE_SHRAPNEL;
        le->startTime = cg.time;
        le->endTime = cg.time + 150;
        le->lifeRate = 1.0 / ( le->endTime - le->startTime );
        le->radius = 1;
        le->leFlags = cgs.media.sparkShader;
        le->bounceFactor = 0.7f;
    }
}

/*
=================
NS CreateBleeder
author: dX
date: 11-02-99
function: creates bleeder @ origin (Called over cg_event.c / CG_EntityEvent (...) )
=================
*/
void CG_PlayerBleed( int weapon, int clientNum, int damage, vec3_t origin, vec3_t dir );

void CG_CreateBleeder( vec3_t origin , int damage, int playerNum ) {
    //	localEntity_t	*le;
    //	refEntity_t		*re;
    //	int a;
    centity_t	*cent;
    vec3_t dir;

    cent = &cg_entities[ playerNum ];

    if (!cent)
        return;

    if (!cg_blood.integer)
        return;

    AngleVectors( cent->lerpAngles, dir, NULL,NULL );

    // add a little random factor
    dir[0] += -0.5 + random();
    dir[1] += -0.5 + random();

    CG_PlayerBleed( WP_M4, playerNum, damage*2, origin, dir );

    // old bloodcode goes here
    /*
    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    VectorCopy( origin, le->pos.trBase );
    //	VectorScale(dir, 125, le->pos.trDelta);
    le->pos.trType = TR_GRAVITY;
    le->pos.trTime = cg.time;
    le->leType = LE_SHRAPNEL;
    le->startTime = cg.time;
    le->endTime = cg.time + 500;
    le->lifeRate = 1.0 / ( le->endTime - le->startTime );

    damage *= 3;


    if ( damage < 5) {
    le->radius = 0.5  * damage + random(); // base is 0.5 max is 1.5
    le->leFlags = cgs.media.bloodparticleShaders[0];
    }
    else if ( damage < 10 ) {
    le->radius = 1 * damage + random();   // base is 1 max is 2
    le->leFlags = cgs.media.bloodparticleShaders[1];
    }
    else if ( damage < 15) {
    le->radius = 1.5  * damage + random(); // base is 1.5 max is 2.5
    le->leFlags = cgs.media.bloodparticleShaders[2];
    }
    else {
    le->radius = 2 * damage + random();   // base is 2 max is 3
    le->leFlags = cgs.media.bloodparticleShaders[3];
    }

    le->radius = re->radius ;

    le->bounceFactor = 0.1f;

    le->leMarkType = LEMT_BLOOD;
    */
}


/*
=================
NSQ3 Blood On Wall Mark
author: dX
date: 15-02-99
description: renders Mark on wall
=================
*/

void CG_BloodOnWallMark( vec3_t end, vec3_t normal, int damage, qboolean brain ) {
    int i;
    qhandle_t shader;
    int max = 14;

    if (!cg_blood.integer)
        return;

    if (damage > 20)
        damage = 20;
    if (damage <= 6)
        max = 4;

    i = random()*max;

    if (i>max)
        i=max;
    else if (i<0)
        i=0;

    if (damage <= 6)
        shader = cgs.media.ns_bloodStainSmall[i];
    else
        shader = cgs.media.ns_bloodStain[i];

#if DEBUG_BUILD
    CG_Printf("Blood Mark On Wall: damage = %i shader# %i brain: %s\n",damage, i, (brain)?"yes":"no");
#endif

    if (brain) {
        int i = random()*4;

        if (i>4)
            i=random()*4;

        damage = 24 - random()*4;

        shader = cgs.media.ns_brainStain[i];
    }

    CG_ImpactMark( shader, end, normal, random()*360,1,1,1,0.6 + random()/3,qtrue,damage,qfalse);
}

/*
=================
NSQ3 Reload Empty Clip Warning
author: dX
date: 23-02-2k
descr: prints a reload string and plays a sound
=================
*/
void CG_ReloadClipWarning( void ) {

    // play 'click' sound
    if ((cg.DeafTime < cg.time)) {
        trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, cgs.media.noAmmoSound );

        CG_Printf(S_COLOR_RED"Reload...\n");
    }

}

/*
=================
CG_GetColorForBodyPart
=================
*/
void CG_GetColorForBodyPart( int health, int armor, vec4_t hcolor ) {
    int		count;
    int		max;

    // calculate the total points of damage that can
    // be sustained at the current health / armor level
    if ( health <= 0 ) {
        VectorClear( hcolor );	// black
        hcolor[3] = 1;
        return;
    }
    count = armor;
    max = health;
    if ( max < count ) {
        count = max;
    }
    health += count;

    // set the color based on health
    hcolor[0] = 1.0;
    hcolor[3] = 1.0;
    if ( health <= 0 ) {
        hcolor[2] = 1.0;
    } else if ( health > 66 ) {
        hcolor[2] = 0;
    } else {
        hcolor[2] = ( health - 66 ) / 33.0;
    }

    if ( health < 30 ) {
        hcolor[1] = 1.0;
    } else if ( health > 60 ) {
        hcolor[1] = 0;
    } else {
        hcolor[1] = ( health - 30 ) / 30.0;
    }
}
void CG_DeleteDirectMark( int entityNum );

#define LOCKBREAK_PIECES	4

void CG_LockBreak( centity_t *cent )
{
    vec3_t  origin;
    vec3_t	dir;
    int i;

    VectorCopy( cent->currentState.pos.trBase , cent->currentState.origin );

    for ( i=0; i < LOCKBREAK_PIECES; i++ )
    {
        vec3_t randangle;
        VectorCopy( cent->currentState.origin , origin );

        randangle[0] = (rand() % 360) - 180;
        randangle[1] = (rand() % 360) - 180;
        randangle[2] = (rand() % 360) - 180;

        dir[0] = crandom()*100;
        dir[1] = crandom()*100;
        dir[2] = crandom()*100;

        origin[0] += -0.25 + random()/2;
        origin[1] += -0.25 + random()/2;
        origin[2] += -0.25 + random()/2;

        VectorNormalize( dir );

        CG_LaunchParticle( origin, randangle, dir, cgs.media.metalSmall,SOUND_NONE, 0.2f );
    }

    VectorCopy( cent->currentState.origin , origin );

    if ((cg.DeafTime < cg.time)) trap_S_StartSound( origin, cent->currentState.number, CHAN_AUTO, cgs.media.sfxMetal[ (int)(random()*5) ] );
}


void NS_CG_LaunchFuncExplosive ( centity_t *cent )
{
    sfxHandle_t		sfx = 0;
    qhandle_t	model;
    vec3_t dir, origin,maxs,mins;
    qboolean	flatobject = qfalse;
    float *xpos, *ypos, miny;
    int			bouncefactor, best, border, height, width,i, step = 5,random = 0;
    int		spawnflags = cent->currentState.eventParm;
    vec3_t randangle;

    VectorCopy( cent->currentState.origin , maxs );
    VectorCopy( cent->currentState.origin2 , mins );

    // find the thinnest axis, which will be the one we expand
    best = 0;
    for ( i = 1 ; i < 3 ; i++ ) {
        if ( maxs[i] - mins[i] < maxs[best] - mins[best] ) {
            best = i;
        }
    }

    if ( maxs[best] - mins[best] < 6.0f )
    {
        flatobject = qtrue;
        //		CG_Printf("flatobject: %i - %f %f %f\n", best, maxs[0]-mins[0], maxs[1]-mins[1], maxs[2]-mins[2] );
    }
    i = 0;


    VectorSubtract( maxs, mins, origin);

    height = (int)origin[2];

    if(origin[0] > origin[1]){
        xpos = &origin[0];
        ypos = &origin[1];

        width = (int)origin[1];
        miny = mins[1];

        border = *xpos/step;

        VectorScale(origin, 0.5, origin);
        VectorAdd(mins, origin, origin);

        *xpos = mins[0];
    } else {
        xpos = &origin[1];
        ypos = &origin[0];

        width = (int)origin[0];
        miny = mins[0];

        border = *xpos/step; // xpos/step

        VectorScale(origin, 0.5, origin);
        VectorAdd  (mins, origin, origin);

        *xpos = mins[1];
    }
    VectorClear(dir);

    VectorCopy( cent->currentState.apos.trBase , dir );

    for(i = 0; i < border; i++){

        //	VectorClear(dir);
        if ( flatobject )
        {
            randangle[0] = 	randangle[1] = randangle[2] = 0;

            switch ( best )
            {
            case 0:
                randangle[0] = 80 + random()*20;
                randangle[2] = -30 + random()*(30*2);
                break;
            case 1:
                randangle[2] = 80 + random()*20;
                randangle[0] = -30 + random()*(30*2);
                break;
            case 2:
                randangle[1] = 80 + random()*20;
                randangle[2] = -30 + random()*(30*2);
                break;
            default:
                break;
            }
        }
        else
        {
            randangle[0] = (rand() % 360) - 180;
            randangle[1] = (rand() % 360) - 180;
            randangle[2] = (rand() % 360) - 180;
        }

        //	dir[0] = (rand() % 360) - 180;
        //	dir[1] = (rand() % 360) - 180;
        //	dir[2] = (rand() % 360) - 180;

        //		VectorNormalize( dir );

        *xpos += step;
        *ypos = miny + rand()%width;
        origin[2] = mins[2] + rand()%height;

        if(spawnflags & 1) {
            sfx = cgs.media.sfxWood[ (int)(random()*3) ];
            random = (int)(random()*2);

            if ( flatobject && random > 1 )
                random = 1;
            switch (random ) {
            case 0:
                model = cgs.media.woodSmall;
                bouncefactor = BOUNCE_MEDIUM;
                break;
            case 1:
                model = cgs.media.woodMedium;
                bouncefactor = BOUNCE_LIGHT;
                break;
            case 2:
            default:
                model = cgs.media.woodBig;
                bouncefactor = BOUNCE_LIGHT;
                break;
            }
            // fire_woodgib(self, origin, dir);
        } else if(spawnflags & 2) {
            sfx = cgs.media.sfxMetal[ (int)(random()*3) ];
            random = (int)(random()*2);

            if ( flatobject && random > 1)
                random = 1;

            switch ( random ) {
            case 0:
                model = cgs.media.metalSmall;
                bouncefactor = BOUNCE_MEDIUM;
                break;
            case 1:
                model = cgs.media.metalMedium;
                bouncefactor = BOUNCE_LIGHT;
                break;
            case 2:
            default:
                model = cgs.media.metalBig;
                bouncefactor = BOUNCE_LIGHT;
                break;
            }
            //fire_metalgib(self, origin, dir);
        }  else if(spawnflags & 4) {
            sfx = cgs.media.sfxMetal[ (int)(random()*3) ];
            random = (int)(random()*2);

            if ( flatobject && random > 1 )
                random = 1;

            switch ( random ) {
            case 0:
                model = cgs.media.stoneSmall;
                bouncefactor = 0;
                break;
            case 1:
                model = cgs.media.stoneMedium;
                bouncefactor = 0;
                break;
            case 2:
            default:
                model = cgs.media.stoneBig;
                bouncefactor = 0;
                break;
            }
            //fire_metalgib(self, origin, dir);
        }
        else{
            sfx = cgs.media.sfxGlass[ (int)(random()*3)];
            random = (int)(random()*2);

            if ( flatobject && random > 1 )
                random = 1;

            switch ( random ) {
            case 0:
                model = cgs.media.glassSmall;
                bouncefactor = BOUNCE_MEDIUM;
                break;
            case 1:
                model = cgs.media.glassMedium;
                bouncefactor = BOUNCE_MEDIUM;
                break;
            case 2:
            default:
                model = cgs.media.glassBig;
                bouncefactor = BOUNCE_LIGHT;
                break;
            }
        }

        CG_LaunchParticle( origin, randangle, dir, model,SOUND_NONE, bouncefactor );
        //	CG_Printf("spawned particle @ %s\n", vtos(origin ) );

    }


    if ((cg.DeafTime < cg.time)) trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
}

void CG_ForceCvar( const char *cvar, int value )
{
    char var[128];

    trap_Cvar_VariableStringBuffer( cvar, var , sizeof( var ) );

    if( atoi(var) != value )
        trap_Cvar_Set( cvar, va("%i",value) );
}

qboolean CG_ButtonPushed(int button)
{
    usercmd_t	cmd;
    int			cmdNum;

    cmdNum = trap_GetCurrentCmdNumber();
    trap_GetUserCmd( cmdNum, &cmd );


    if (cmd.buttons & button)
        return qtrue;

    return qfalse;
}
qboolean CG_LastButtonPushed(int button)
{
    usercmd_t	cmd;
    int			cmdNum;

    cmdNum = trap_GetCurrentCmdNumber();
    trap_GetUserCmd( cmdNum-1, &cmd );


    if (cmd.buttons & button)
        return qtrue;

    return qfalse;
}
int CG_FollowCycle( int dir ) {
    int		clientnum;
    int		original;

    if ( dir != 1 && dir != -1 ) {
        return cg.cameraFollowNumber;
    }

    clientnum = cg.cameraFollowNumber;
    original = clientnum;
    do {
        clientnum += dir;
        if ( clientnum >= cgs.maxclients ) {
            clientnum = 0;
        }
        if ( clientnum < 0 ) {
            clientnum = cgs.maxclients - 1;
        }


        // how can we detect dead players?
        /*
        if ( cg_entities[clientnum].currentState.eFlags & EF_DEAD )
        continue;*/

        // can only follow connected clients
        if ( cg_entities[clientnum].currentState.eType != ET_PLAYER ) {
            continue;
        }

        // this is good, we can use it
        return clientnum;
    } while ( clientnum != original );

    return original;
    // leave it where it was
}


int CG_GoChase( void )
{
    int chasetarget;


    cg.cameraFollowNumber = cg.snap->ps.clientNum;

    chasetarget = CG_FollowCycle(1);

    if (chasetarget == cg.snap->ps.clientNum )
        return 0;

    if ( cg.cameraActive ) {
        cg.cameraActive = qfalse;
        CG_Printf("Camera Disabled.\n");
    }
    else {
        cg.cameraActive = qtrue;
        CG_Printf("Camera Enabled.\n");
    }

    cg.cameraFollowNumber = chasetarget;
    cg.cameraZoom = 35.0f;
    cg.cameraUsed = qfalse;
    cg.cameraRemainTime = cg.time + 300000; // remain 5m at the chase

    return 0;

}

/*
===============
Qcmd Showmenu

shows the menu, saves 1-0 key bindings to a buffer and binds these keys new
===============
*/
#include "../ui/ui_shared.h"

extern vmCvar_t	cg_qcmd_close;

void CG_QCmd_HandleMenu( void )
{
    int		x		= cg_qcmd_posx.integer,
              y		= cg_qcmd_posy.integer;
    float	size	= cg_qcmd_size.value;
    int		cur_y;
    vec4_t	color;
    int		i;
    char	command[128];


    if ( trap_Key_IsDown( '1' ) ) {
        trap_SendConsoleCommand( va("%s ; %s",cg_qcmd_cmd1.string, cg_qcmd_close.string ) );
        cg.viewCmd = qfalse; return;
    }
    else if ( trap_Key_IsDown( '2' ) ) {
        trap_SendConsoleCommand( va("%s ; %s ;", cg_qcmd_cmd2.string, cg_qcmd_close.string ) );
        cg.viewCmd = qfalse; return;
    }
    else if ( trap_Key_IsDown( '3' ) ) {
        trap_SendConsoleCommand( va("%s ; %s ;", cg_qcmd_cmd3.string, cg_qcmd_close.string ) );
        cg.viewCmd = qfalse; return;
    }
    else if ( trap_Key_IsDown( '4' ) ) {
        trap_SendConsoleCommand( va("%s ; %s ;", cg_qcmd_cmd4.string, cg_qcmd_close.string ) );
        cg.viewCmd = qfalse; return;
    }
    else if ( trap_Key_IsDown( '5' ) ) {
        trap_SendConsoleCommand( va("%s ; %s ;", cg_qcmd_cmd5.string, cg_qcmd_close.string ) );
        cg.viewCmd = qfalse; return;
    }
    else if ( trap_Key_IsDown( '6' ) ) {
        trap_SendConsoleCommand( va("%s ; %s ;", cg_qcmd_cmd6.string, cg_qcmd_close.string ) );
        cg.viewCmd = qfalse; return;
    }
    else if ( trap_Key_IsDown( '7' ) ) {
        trap_SendConsoleCommand( va("%s ; %s ;", cg_qcmd_cmd7.string, cg_qcmd_close.string ) );
        cg.viewCmd = qfalse; return;
    }
    else if ( trap_Key_IsDown( '8' ) ) {
        trap_SendConsoleCommand( va("%s ; %s ;", cg_qcmd_cmd8.string, cg_qcmd_close.string ) );
        cg.viewCmd = qfalse; return;
    }
    else if ( trap_Key_IsDown( '9' ) ) {
        trap_SendConsoleCommand( va("%s ; %s ;", cg_qcmd_cmd9.string, cg_qcmd_close.string ) );
        cg.viewCmd = qfalse; return;
    }
    else if ( trap_Key_IsDown( '0' ) ) {
        trap_SendConsoleCommand( va("%s ; %s ;", cg_qcmd_cmd0.string, cg_qcmd_close.string ) );
        cg.viewCmd = qfalse;
        return;
    }

    cur_y = y;

    color[0] = cg_qcmd_r.value;
    color[1] = cg_qcmd_g.value;
    color[2] = cg_qcmd_b.value;
    color[3] = cg_qcmd_a.value;

    for (i=0;i<10;i++)
    {
        trap_Cvar_VariableStringBuffer(va("cg_qcmd_dscr%i",(i==9)?0:i+1), command,sizeof(command) );

        if ( strlen ( command ) <= 0 )
        {
            if ( i == 9 )
                CG_Text_Paint( x, cur_y, size, color, va("%i. Quit", (i==9)?0:i+1 ) , 0,0, 0 );//ITEM_TEXTSTYLE_OUTLINED);

            continue;
        }

        CG_Text_Paint( x, cur_y, size, color, va("%i. %s", (i==9)?0:i+1, command) , 0,0, 0 );//ITEM_TEXTSTYLE_OUTLINED);

        cur_y += CG_Text_Height("A", size, 0 ) + 2;
    }

}

extern	vmCvar_t	cg_timerPosY;
extern	vmCvar_t	cg_timerPosX;
extern	vmCvar_t	cg_hudScale;

/*
===============
Draw Pic2

another version of cg_drawpic
===============
*/
void CG_DrawPic2( float x, float y, float width, float height, qhandle_t hShader )
{
    //	if ( cgs.glconfig.vidWidth < 1024 )
    {
        x = x * ( (float)cgs.glconfig.vidWidth/1024.0f ) * cg_hudScale.value;
        y = y * ( (float)cgs.glconfig.vidHeight/768.0f ) * cg_hudScale.value;
        width = width * ( (float)cgs.glconfig.vidWidth/1024.0f ) * cg_hudScale.value;
        height = height * ( (float)cgs.glconfig.vidHeight/768.0f ) * cg_hudScale.value;
    }

    trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}
/*
===============
Draw Pic3

another version of cg_drawpic

just because demo is too dumb to do shit right.

angegebene X coordinate ist VON rechts!
gesamte breite des imaginären rechtecks - width - angegebenes X = richtiges X

höhe eines BACKGROUND STREIFEN mit zwischenabstand ist (24+2)=26

Schiebt sich also ein streifen nach oben, dann wird so gerechnet:
angebenes Y - (anzahl der verschiebungen nach oben x 26) = neues Y

read this and accept his dumbness.he's doing it, but he's never doing it right.
omg he sucks badly.

basturd

instead of going from left to right it's going from fucking right to fucking left
things like this are just fucking not necessary.

===============
*/
void CG_DrawPic3( float x, float y, float width, float height, qhandle_t hShader )
{

    x = 1024 - width - x;
    //	y = 768 - height - y;

    //	if ( cgs.glconfig.vidWidth < 1024 )
    {
        x = x * ( (float)cgs.glconfig.vidWidth/1024.0f );// * cg_hudScale.value;
        y = y * ( (float)cgs.glconfig.vidHeight/768.0f );// * cg_hudScale.value;
        width = width * ( (float)cgs.glconfig.vidWidth/1024.0f ) * cg_hudScale.value;
        height = height * ( (float)cgs.glconfig.vidHeight/768.0f ) * cg_hudScale.value;
    }


    trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}

#define SMALLDIGIT_WIDTH	12
#define SMALLDIGIT_HEIGHT	20

static void CG_DrawSmallField (int x, int y, int width, int value) {
    char	num[16], *ptr;
    int		l;
    int		frame;

    if ( width < 1 ) {
        return;
    }

    // draw number string
    if ( width > 5 ) {
        width = 5;
    }

    if ( value < 0 )
        value = 0;

    switch ( width ) {
    case 1:
        value = value > 9 ? 9 : value;
        value = value < 0 ? 0 : value;
        Com_sprintf (num, sizeof(num), "%i", value);
        break;
    case 2:
        value = value > 99 ? 99 : value;
        value = value < -9 ? -9 : value;

        if ( value < 10 )
            Com_sprintf (num, sizeof(num), "0%i", value);
        else
            Com_sprintf (num, sizeof(num), "%i", value);
        break;
    case 3:
        value = value > 999 ? 999 : value;
        value = value < -99 ? -99 : value;
        if ( value < 10 )
            Com_sprintf (num, sizeof(num), "00%i", value);
        else if ( value < 100 )
            Com_sprintf (num, sizeof(num), "0%i", value);
        else
            Com_sprintf (num, sizeof(num), "%i", value);
        break;
    case 4:
        value = value > 9999 ? 9999 : value;
        value = value < -999 ? -999 : value;
        if ( value < 10 )
            Com_sprintf (num, sizeof(num), "000%i", value);
        else if ( value < 100 )
            Com_sprintf (num, sizeof(num), "00%i", value);
        else if ( value < 1000 )
            Com_sprintf (num, sizeof(num), "0%i", value);
        else
            Com_sprintf (num, sizeof(num), "%i", value);
        break;
    }

    l = strlen(num);
    if (l > width)
        l = width;
    /* x += 2 + SMALLDIGIT_WIDTH*(width - l); */

    ptr = num;
    while (*ptr && l)
    {
        if (*ptr == '-')
            frame = STAT_MINUS;
        else
            frame = *ptr -'0';

        CG_DrawPic3( x,y, SMALLDIGIT_WIDTH, SMALLDIGIT_HEIGHT, cgs.media.smalldigitalNumberShaders[frame] );
        x -= SMALLDIGIT_WIDTH;
        ptr++;
        l--;
    }

}

/*
=================
CG_DrawSmallTimer
=================
*/
static void CG_DrawSmallTimer( int x, int y, int timer ) {
    int			mins, seconds;
    int			msec;
    qhandle_t	smallColon = trap_R_RegisterShader( "gfx/2d/hud/timer/number_colon.tga" );
    /*
    if ( cgs.gametype == GT_LTS ) {
    if ( !cgs.levelRoundStartTime )
    return y;

    msec = ( cgs.levelRoundStartTime ) - cg.time;
    }
    else

    msec = cg.time - cgs.levelStartTime;
    */
    msec = timer - cg.time;

    seconds = msec / 1000;
    mins = seconds / 60;
    seconds -= mins * 60;


    CG_DrawSmallField( x + SMALLDIGIT_WIDTH * 4 - 2, y, 1, mins );
    CG_DrawPic3( x + SMALLDIGIT_WIDTH * 3, y, 8, 20, smallColon );
    CG_DrawSmallField( x + SMALLDIGIT_WIDTH * 2 , y, 2, seconds );
    //	CG_DrawPic( 640 - 2 * 16, y, 32,32, cgs.media.clockIcon );

}

static int CC_DrawStatusBar_AssaultField( int y, int number )
{
    //	int misc;
    //	int	y2;
    int base_x = cg_timerPosX.integer;

    qhandle_t layer_back		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/background.tga" );
    qhandle_t timer_back		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/background_timer.tga" );

    qhandle_t assault_blocked	=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/mission_assault_blocked.tga" );
    qhandle_t assault_icon		=	trap_R_RegisterShaderNoMip( va("gfx/2d/hud/timer/mission_assault_%i.tga",number+1) );

    if ( cgs.mi_assaultFields == 0 )
        return y;

    if ( cgs.mi_assaultFields == 1 && number > 0 )
        return y;
    if ( cgs.mi_assaultFields == 2 && number > 1 )
        return y;
    if ( cgs.mi_assaultFields == 3 && number > 2 )
        return y;
    if ( cgs.mi_assaultFields == 4 && number > 3 )
        return y;

    CG_DrawPic3( base_x, y, 26,24, layer_back );

    if ( cgs.assaultFieldsCaptured[number] ) {
        CG_DrawPic3( base_x + 4, y+2, 20,20, assault_blocked );
        return y + 26;
    }
    else
        CG_DrawPic3( base_x + 4, y+2, 20,20, assault_icon );

    CG_DrawPic3( base_x+26, y, 55,24, timer_back );

    CG_DrawSmallTimer( base_x+20, y+2, cgs.levelAssaultStartTime[number] );

    y += 26;

    return y;
    /* TODO: ADD TIMER */

    /*
    MISSION - AFIELD1 BACKGROUND STREIFEN
    0, 110, 26, 24, gfx/2d/hud/timer/background.tga

    MISSION - AFIELD1 ICON
    4, 112, 20, 20, gfx/2d/hud/timer/mission_assault_1.tga
    4, 112, 20, 20, gfx/2d/hud/timer/mission_assault_blocked.tga

    MISSION - AFIELD1 TIMER BACKGROUND
    6, 110, 55, 24, gfx/2d/hud/timer/background_timer.tga

    MISSION - AFIELD1 TIMER
    liegt auf timer background
    */
}
static int CC_DrawStatusBar_Bomb( int y )
{
    //	int misc;
    //	int	y2;
    int base_x = cg_timerPosX.integer;

    qhandle_t layer_back		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/background.tga" );
    qhandle_t timer_back		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/background_timer.tga" );
    qhandle_t bomb_icon		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/mission_bomb.tga" );

    if ( cgs.levelBombStartTime <= 0 )
        return y;

    CG_DrawPic3( base_x, y, 26, 24, layer_back );
    CG_DrawPic3( base_x + 4, y+2, 20, 20, bomb_icon );

    CG_DrawPic3( base_x+26, y, 55, 24, timer_back );

    CG_DrawSmallTimer( base_x+20, y+2, cgs.levelBombStartTime );

    return y + 26;
}
static int CC_DrawStatusBar_MissionItems( int y )
{
    int misc = 0;
    //	int	y2;
    int base_x = cg_timerPosX.integer;

    qhandle_t layer_back		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/background.tga" );
    qhandle_t timer_back		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/background_timer.tga" );
    qhandle_t briefcase_icon	=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/mission_briefcase.tga" );
    qhandle_t bomb_icon			=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/mission_bomb.tga" );
    qhandle_t layer_end			=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/background_end.tga" );

    if ( ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR || cg.snap->ps.pm_type == PM_SPECTATOR || cg.snap->ps.pm_type == PM_NOCLIP ) )
        return y;

    if ( cg.predictedPlayerState.powerups[PW_BRIEFCASE] > 0 )
        misc++;
    if ( BG_GotWeapon( WP_C4, cg.predictedPlayerState.stats ) || ( cg.snap->ps.pm_flags & PMF_BOMBRANGE ) )
        misc++;

    if ( misc == 1 ) {
        CG_DrawPic3( base_x, y, 28, 24, layer_back );
        CG_DrawPic3( base_x+28, y, 4, 24, layer_end );
    }
    if ( misc == 2 ) {
        CG_DrawPic3( base_x, y, 56, 24, layer_back );
        CG_DrawPic3( base_x+56, y, 4, 24, layer_end );
    }

    if ( misc == 0 )
        return y;

    misc = 0;

    if ( cg.predictedPlayerState.powerups[PW_BRIEFCASE] > 0 )
    {
        trap_R_SetColor( colorRed );
        CG_DrawPic3( base_x+4, y+2, 20, 20, briefcase_icon );
        trap_R_SetColor( NULL );

        misc++;
    }
    if ( BG_GotWeapon( WP_C4, cg.predictedPlayerState.stats ) ||
            ( cg.snap->ps.pm_flags & PMF_BOMBRANGE ) )
    {
        float pulse = sin( cg.time / 25  );

        if ( cg.snap->ps.pm_flags & PMF_BOMBRANGE )
        {
            if ( pulse < 0.0 )
                trap_R_SetColor( colorBlack );
            else
                trap_R_SetColor( colorRed );
        }
        else
            trap_R_SetColor( colorRed );

        if ( misc == 0 )
            CG_DrawPic3( base_x+4, y+2, 20, 20, bomb_icon );
        else
            CG_DrawPic3( base_x+32, y+2, 20, 20, bomb_icon );
        trap_R_SetColor( NULL );

        misc++;
    }

    if ( misc <= 0 )
        return y;

    return y + 26;
}

static int CG_DrawStatusBar_Grenades( int y )
{
    int misc;
    int	y2;
    int base_x = cg_timerPosX.integer;


    qhandle_t layer_back	=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/background.tga" );

    qhandle_t ammo_40mm		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/grenade_40mm.tga" );
    qhandle_t ammo_frag		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/grenade_frag.tga" );
    qhandle_t ammo_flash	=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/grenade_flash.tga" );
    qhandle_t layer_end			=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/background_end.tga" );

    misc = 0;

    if ( ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR || cg.snap->ps.pm_type == PM_SPECTATOR || cg.snap->ps.pm_type == PM_NOCLIP ) )
        return y;

    if ( cg.predictedPlayerState.ammo[AM_GRENADES] > 0 )
        misc++;
    if ( cg.predictedPlayerState.ammo[AM_FLASHBANGS] > 0 )
        misc++;
    if ( cg.predictedPlayerState.ammo[AM_40MMGRENADES] > 0 )
        misc++;

    if ( misc == 0 )
        return y;

    if ( misc == 1 ) {
        CG_DrawPic3( base_x, y, 52, 24, layer_back );
        CG_DrawPic3( base_x + 52, y, 4, 24, layer_end );
    }
    if ( misc == 2 ) {
        CG_DrawPic3( base_x, y, 100, 24, layer_back );
        CG_DrawPic3( base_x + 100, y, 4, 24, layer_end );
    }
    if ( misc == 3 ) {
        CG_DrawPic3( base_x, y, 148, 24, layer_back );
        CG_DrawPic3( base_x + 148, y, 4, 24, layer_end );
    }

    y2 = 52;

    if ( cg.predictedPlayerState.ammo[AM_GRENADES] > 0 )
    {
        int ammo = cg.predictedPlayerState.ammo[AM_GRENADES];

        CG_DrawPic3( base_x+28, y+2, 20, 20, ammo_frag );
        CG_DrawSmallField( base_x+28 - 20, y+2, 1, ammo );

        y2 += 48;
    }

    if ( cg.predictedPlayerState.ammo[AM_FLASHBANGS] > 0  )
    {
        int ammo = cg.predictedPlayerState.ammo[AM_FLASHBANGS];

        if ( y2 == 52 )
        {
            CG_DrawPic3( base_x+28, y+2, 20, 20, ammo_flash );
            CG_DrawSmallField( base_x+28 - 20, y+2, 1, ammo );
        }
        else
        {
            CG_DrawPic3( base_x+76, y+2, 20, 20, ammo_flash );
            CG_DrawSmallField( base_x+76 - 20, y+2, 1, ammo );
        }

        y2 += 48;
    }

    if ( cg.predictedPlayerState.ammo[AM_40MMGRENADES] > 0 )
    {
        int ammo = cg.predictedPlayerState.ammo[AM_40MMGRENADES];

        if ( y2 == 52 )
        {
            CG_DrawPic3( base_x+28, y+2, 20, 20, ammo_40mm );
            CG_DrawSmallField( base_x+28 - 20, y+2, 1, ammo );
        }
        else if ( y2 == 100 )
        {
            CG_DrawPic3( base_x+76, y+2, 20, 20, ammo_40mm );
            CG_DrawSmallField( base_x+76 - 20, y+2, 1, ammo );
        }
        else
        {
            CG_DrawPic3( base_x+124, y+2, 20, 20, ammo_40mm );
            CG_DrawSmallField( base_x+124 - 20, y+2, 1, ammo );
        }

        y2 += 48;
    }
    if ( y2 == 52 )
        return y;

    return y + 26;
}

int		BG_GetPrimary( int stats [ ] );
int		BG_GetSecondary( int stats [ ] );

int CG_GetPrimaryAmmoCount( void )
{
    int weapon = BG_GetPrimary( cg.snap->ps.stats );
    gitem_t	*item;

    if ( weapon == WP_NONE )
        return -1;

    item = BG_FindItemForWeapon( weapon );

    if ( !item )
        return -1;

    return cg.predictedPlayerState.ammo[ item->giAmmoTag ];
}
int CG_GetSecondaryAmmoCount( void )
{
    int weapon = BG_GetSecondary( cg.snap->ps.stats );
    gitem_t	*item;

    if ( weapon == WP_NONE )
        return -1;

    item = BG_FindItemForWeapon( weapon );

    if ( !item )
        return -1;

    return cg.predictedPlayerState.ammo[ item->giAmmoTag ];
}
int CG_GetAmmoCountForWeapon( int weapon )
{
    gitem_t	*item;

    if ( weapon == WP_NONE || weapon >= WP_NUM_WEAPONS )
        return -1;

    item = BG_FindItemForWeapon( weapon );

    if ( !item )
        return -1;

    return cg.predictedPlayerState.ammo[ item->giAmmoTag ];
}

static int	CG_DrawStatusBar_Ammo ( int y )
{
    qhandle_t ammo_prim		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/ammo_primary.tga" );
    qhandle_t ammo_seco		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/ammo_secondary.tga" );
    qhandle_t layer_back	=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/background.tga" );
    qhandle_t layer_end			=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/background_end.tga" );

    int base_x = cg_timerPosX.integer;
    int base_y = cg_timerPosY.integer;

    if ( ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR || cg.snap->ps.pm_type == PM_SPECTATOR || cg.snap->ps.pm_type == PM_NOCLIP ) )
        return y;

    CG_DrawPic3( base_x, y, 100, 24, layer_back );
    CG_DrawPic3( base_x+100, y, 4, 24, layer_end );

    CG_DrawPic3( base_x+76, y+2, 20, 20, ammo_prim );
    if ( CG_GetPrimaryAmmoCount() < 10 )
        CG_DrawSmallField( base_x+76 - 20, y+2, 1, CG_GetPrimaryAmmoCount() );
    else
        CG_DrawSmallField( base_x+76 - 15, y+2, 2, CG_GetPrimaryAmmoCount() );

    CG_DrawPic3( base_x+28, y+2, 20, 20, ammo_seco );
    CG_DrawSmallField( base_x+28 - 20, y+2, 1, CG_GetSecondaryAmmoCount() );

    y += 26;

    return y;
}

static int CG_DrawStatusBar_Armor ( int y )
{
    qhandle_t item_helm		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/item_helmet.tga" );
    qhandle_t item_vest		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/item_vest.tga" );
    qhandle_t layer_back	=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/background.tga" );
    qhandle_t layer_end			=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/background_end.tga" );
    int base_x = cg_timerPosX.integer;
    int base_y = cg_timerPosY.integer;
    int	misc;

    misc = 0;

    if ( ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR || cg.snap->ps.pm_type == PM_SPECTATOR || cg.snap->ps.pm_type == PM_NOCLIP ) )
        return y;

    if ( cg.predictedPlayerState.powerups[PW_VEST] )
        misc++;
    if ( cg.predictedPlayerState.powerups[PW_HELMET] )
        misc++;

    if ( misc == 1 ) {
        CG_DrawPic3( base_x, y, 28, 24, layer_back );
        CG_DrawPic3( base_x + 28, y, 4, 24, layer_end );
    }
    if ( misc == 2 ) {
        CG_DrawPic3( base_x, y, 56, 24, layer_back );
        CG_DrawPic3( base_x + 56, y, 4, 24, layer_end );
    }

    if ( misc == 0 )
        return y;

    misc = 0;

    if ( cg.predictedPlayerState.powerups[PW_VEST] )
    {
        CG_DrawPic3( base_x+4, y+2, 20, 20, item_vest );
        misc = 1;
    }
    if ( cg.predictedPlayerState.powerups[PW_HELMET] )
    {
        if ( misc == 0 )
            CG_DrawPic3( base_x+4, y+2, 20, 20, item_helm );
        else
            CG_DrawPic3( base_x+32, y+2, 20, 20, item_helm );
    }

    return y + 26;
}

static int CG_DrawStatusBar_Timer ( int y )
{
    int base_x = cg_timerPosX.integer;
    int base_y = cg_timerPosY.integer;
    qhandle_t layer_back	=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/background.tga" );
    qhandle_t timer_clock	=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/timer_clock.tga" );
    qhandle_t timer_vip		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/timer_vip.tga" );
    qhandle_t timer_back	=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/timer/background_timer.tga" );

    CG_DrawPic3( base_x, y, 26, 24, layer_back );

    if ( cgs.mi_vipTime )
        CG_DrawPic3( base_x+4, y+2, 20, 20, timer_vip );
    else
        CG_DrawPic3( base_x+4, y+2, 20, 20, timer_clock );

    CG_DrawPic3( base_x+26, y, 55, 24, timer_back );

    if ( cgs.gametype != GT_LTS )
        CG_DrawSmallTimer( base_x+20, y+2, cgs.levelStartTime );
    else
        CG_DrawSmallTimer( base_x+20, y+2, cgs.levelRoundStartTime );

    y += 26;

    return y;
}



extern vmCvar_t	cg_timerCustom;

static void  CG_DrawHudStatusBar ( void )
{
    int y;

    if ( cg_timerCustom.integer == -1 )
        return;


    y = 12 + cg_timerPosY.integer;

    y = CG_DrawStatusBar_Timer( y );

    if ( cg_timerCustom.integer == 1 ||
            cg_timerCustom.integer == 2 ||
            cg_timerCustom.integer == 3 )
        y = CG_DrawStatusBar_Ammo( y );

    if ( cg_timerCustom.integer == 2 ||
            cg_timerCustom.integer == 3 )
        y = CG_DrawStatusBar_Grenades( y );

    if ( cg_timerCustom.integer == 3 )
        y = CG_DrawStatusBar_Armor( y );

    // mission helpers
    if ( cgs.gametype == GT_LTS ) {
        y = CC_DrawStatusBar_AssaultField( y , 0 );
        y = CC_DrawStatusBar_AssaultField( y , 1 );
        y = CC_DrawStatusBar_AssaultField( y , 2 );
        y = CC_DrawStatusBar_AssaultField( y , 3 );

        y = CC_DrawStatusBar_Bomb( y );

        y = CC_DrawStatusBar_MissionItems( y );
    }
}

/*
===============
Scrutch Hud

draws scrutchs version of the HUD
===============
*/
int BG_GetMaxRoundForWeapon( int weapon );
void CG_Text_Paint2(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style);

// 96x192
static void  CG_DrawHudLocator ( int x, int y, int w, int h )
{
    vec4_t		hcolor;
    playerState_t	*ps;
    float pulse = sin( cg.time / 25  );

    ps = &cg.snap->ps;

    CG_GetColorForHealth( /* 1 0 0 - */ ps->stats[STAT_HEALTH] - ps->stats[STAT_CHEST_DAMAGE] , 0, hcolor );
    if ( cg.flashDmgLocTime[0] > cg.time && ( cg.flashDmgLocTime[0] - cg.time ) > 0  )
    {
        if ( pulse < 0.0 )
        {
            /*
            hcolor[0] = colorWhite[0];
            hcolor[1] = colorWhite[1];
            hcolor[2] = colorWhite[2];
            */
        }
        else
        {
            hcolor[0] = colorBlack[0];
            hcolor[1] = colorBlack[1];
            hcolor[2] = colorBlack[2];
        }
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic2( x, y, w, h ,cgs.media.loc_chestIcon);
    trap_R_SetColor( NULL );

    CG_GetColorForHealth( /* 1 0 0 - */ ps->stats[STAT_HEALTH]  - ps->stats[STAT_STOMACH_DAMAGE] ,0, hcolor );
    if ( cg.flashDmgLocTime[1] > cg.time && ( cg.flashDmgLocTime[1] - cg.time ) > 0  )
    {
        if ( pulse < 0.0 )
        {
            /*
            hcolor[0] = colorWhite[0];
            hcolor[1] = colorWhite[1];
            hcolor[2] = colorWhite[2]; */
        }
        else
        {
            hcolor[0] = colorBlack[0];
            hcolor[1] = colorBlack[1];
            hcolor[2] = colorBlack[2];
        }
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic2( x, y, w, h ,cgs.media.loc_stomachIcon);
    trap_R_SetColor( NULL );

    CG_GetColorForHealth( /* 1 0 0 - */ ps->stats[STAT_HEALTH]  - ps->stats[STAT_ARM_DAMAGE] ,0, hcolor );
    if ( cg.flashDmgLocTime[2] > cg.time && ( cg.flashDmgLocTime[2] - cg.time ) > 0  )
    {
        if ( pulse < 0.0 )
        {
            /*
            hcolor[0] = colorWhite[0];
            hcolor[1] = colorWhite[1];
            hcolor[2] = colorWhite[2]; */
        }
        else
        {
            hcolor[0] = colorBlack[0];
            hcolor[1] = colorBlack[1];
            hcolor[2] = colorBlack[2];
        }
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic2( x, y, w, h ,cgs.media.loc_leftArmIcon);
    CG_DrawPic2( x, y, w, h ,cgs.media.loc_rightArmIcon);
    trap_R_SetColor( NULL );

    CG_GetColorForHealth( /* 100 - */ ps->stats[STAT_HEALTH]  - ps->stats[STAT_LEG_DAMAGE] ,0, hcolor );
    if ( cg.time < cg.flashDmgLocTime[3] && ( cg.flashDmgLocTime[3] - cg.time ) > 0 )
    {
        if ( pulse < 0.0 )
        {
            /*
            hcolor[0] = colorWhite[0];
            hcolor[1] = colorWhite[1];
            hcolor[2] = colorWhite[2]; */
        }
        else
        {
            hcolor[0] = colorBlack[0];
            hcolor[1] = colorBlack[1];
            hcolor[2] = colorBlack[2];
        }
        /*
        hcolor[0] *= sin( cg.time / PULSE_DIVISOR );
        hcolor[1] *= sin( cg.time / PULSE_DIVISOR );
        hcolor[2] *= sin( cg.time / PULSE_DIVISOR );
        */
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic2( x, y, w, h ,cgs.media.loc_leftLegIcon);
    CG_DrawPic2( x, y, w, h ,cgs.media.loc_rightLegIcon);
    trap_R_SetColor( NULL );

    CG_GetColorForHealth( /* 100 - */ ps->stats[STAT_HEALTH]  - ps->stats[STAT_HEAD_DAMAGE] ,0, hcolor );
    if ( cg.flashDmgLocTime[4] > cg.time && ( cg.flashDmgLocTime[4] - cg.time ) > 0 )
    {
        if ( pulse < 0.0 )
        {
            /*
            hcolor[0] = colorWhite[0];
            hcolor[1] = colorWhite[1];
            hcolor[2] = colorWhite[2]; */
        }
        else
        {
            hcolor[0] = colorBlack[0];
            hcolor[1] = colorBlack[1];
            hcolor[2] = colorBlack[2];
        }
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic2( x, y, w, h ,cgs.media.loc_headIcon);
    trap_R_SetColor( NULL );

    trap_R_SetColor( g_color_table[ColorIndex(COLOR_BLACK)] );
    CG_DrawPic2( x, y, w, h ,cgs.media.loc_bodyLines);
    trap_R_SetColor( NULL );
}

static void  CG_DrawHudLocatorSmall ( int x, int y, int w, int h )
{
    qhandle_t loc_small_headIcon		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/defcon/loc/head.tga" );
    qhandle_t loc_small_leftArmIcon		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/defcon/loc/larm.tga" );
    qhandle_t loc_small_rightArmIcon	=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/defcon/loc/rarm.tga" );
    qhandle_t loc_small_leftLegIcon		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/defcon/loc/lleg.tga" );
    qhandle_t loc_small_rightLegIcon	=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/defcon/loc/rleg.tga" );
    qhandle_t loc_small_stomachIcon		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/defcon/loc/stomach.tga" );
    qhandle_t loc_small_chestIcon		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/defcon/loc/chest.tga" );
    qhandle_t loc_small_bodyLines		=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/defcon/loc/loc_lines.tga" );

    vec4_t		hcolor;
    playerState_t	*ps;
    float pulse = sin( cg.time / 25  );

    ps = &cg.snap->ps;

    CG_GetColorForHealth( /* 1 0 0 - */ ps->stats[STAT_HEALTH] - ps->stats[STAT_CHEST_DAMAGE] , 0, hcolor );
    if ( cg.flashDmgLocTime[0] > cg.time && ( cg.flashDmgLocTime[0] - cg.time ) > 0  )
    {
        if ( pulse < 0.0 )
        {
            /*
            hcolor[0] = colorWhite[0];
            hcolor[1] = colorWhite[1];
            hcolor[2] = colorWhite[2];
            */
        }
        else
        {
            hcolor[0] = colorBlack[0];
            hcolor[1] = colorBlack[1];
            hcolor[2] = colorBlack[2];
        }
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic2( x, y, w, h ,loc_small_chestIcon);
    trap_R_SetColor( NULL );

    CG_GetColorForHealth( /* 1 0 0 - */ ps->stats[STAT_HEALTH]  - ps->stats[STAT_STOMACH_DAMAGE] ,0, hcolor );
    if ( cg.flashDmgLocTime[1] > cg.time && ( cg.flashDmgLocTime[1] - cg.time ) > 0  )
    {
        if ( pulse < 0.0 )
        {
            /*
            hcolor[0] = colorWhite[0];
            hcolor[1] = colorWhite[1];
            hcolor[2] = colorWhite[2]; */
        }
        else
        {
            hcolor[0] = colorBlack[0];
            hcolor[1] = colorBlack[1];
            hcolor[2] = colorBlack[2];
        }
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic2( x, y, w, h ,loc_small_stomachIcon);
    trap_R_SetColor( NULL );

    CG_GetColorForHealth( /* 1 0 0 - */ ps->stats[STAT_HEALTH]  - ps->stats[STAT_ARM_DAMAGE] ,0, hcolor );
    if ( cg.flashDmgLocTime[2] > cg.time && ( cg.flashDmgLocTime[2] - cg.time ) > 0  )
    {
        if ( pulse < 0.0 )
        {
            /*
            hcolor[0] = colorWhite[0];
            hcolor[1] = colorWhite[1];
            hcolor[2] = colorWhite[2]; */
        }
        else
        {
            hcolor[0] = colorBlack[0];
            hcolor[1] = colorBlack[1];
            hcolor[2] = colorBlack[2];
        }
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic2( x, y, w, h ,loc_small_leftArmIcon);
    CG_DrawPic2( x, y, w, h ,loc_small_rightArmIcon);
    trap_R_SetColor( NULL );

    CG_GetColorForHealth( /* 100 - */ ps->stats[STAT_HEALTH]  - ps->stats[STAT_LEG_DAMAGE] ,0, hcolor );
    if ( cg.time < cg.flashDmgLocTime[3] && ( cg.flashDmgLocTime[3] - cg.time ) > 0 )
    {
        if ( pulse < 0.0 )
        {
            /*
            hcolor[0] = colorWhite[0];
            hcolor[1] = colorWhite[1];
            hcolor[2] = colorWhite[2]; */
        }
        else
        {
            hcolor[0] = colorBlack[0];
            hcolor[1] = colorBlack[1];
            hcolor[2] = colorBlack[2];
        }
        /*
        hcolor[0] *= sin( cg.time / PULSE_DIVISOR );
        hcolor[1] *= sin( cg.time / PULSE_DIVISOR );
        hcolor[2] *= sin( cg.time / PULSE_DIVISOR );
        */
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic2( x, y, w, h ,loc_small_leftLegIcon);
    CG_DrawPic2( x, y, w, h ,loc_small_rightLegIcon);
    trap_R_SetColor( NULL );

    CG_GetColorForHealth( /* 100 - */ ps->stats[STAT_HEALTH]  - ps->stats[STAT_HEAD_DAMAGE] ,0, hcolor );
    if ( cg.flashDmgLocTime[4] > cg.time && ( cg.flashDmgLocTime[4] - cg.time ) > 0 )
    {
        if ( pulse < 0.0 )
        {
            /*
            hcolor[0] = colorWhite[0];
            hcolor[1] = colorWhite[1];
            hcolor[2] = colorWhite[2]; */
        }
        else
        {
            hcolor[0] = colorBlack[0];
            hcolor[1] = colorBlack[1];
            hcolor[2] = colorBlack[2];
        }
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic2( x, y, w, h ,loc_small_headIcon);
    trap_R_SetColor( NULL );

    trap_R_SetColor( g_color_table[ColorIndex(COLOR_BLACK)] );
    CG_DrawPic2( x, y, w, h ,loc_small_bodyLines);
    trap_R_SetColor( NULL );
}
static void CG_DrawWeaponStatus2( int x,int y, float scale ) {
    playerState_t	*ps;
    char *mode = "";
    int	weaponmode ;
    int	weapon;

    ps = &cg.snap->ps;
    weaponmode = ps->stats[STAT_WEAPONMODE];
    weapon = ps->weapon;


    if ( ps->weaponstate == WEAPON_BANDAGING_START ||
            ps->weaponstate == WEAPON_BANDAGING_END ||
            ps->weaponstate == WEAPON_BANDAGING )
        mode = "Bandaging";	/*
    	else if ( (weapon == WP_M4 || weapon == WP_AK47) && (weaponmode & ( 1 << WM_GRENADELAUNCHER ) ) && ( weaponmode & ( 1 << WM_WEAPONMODE2 ) ) )
    	{
    	mode = "";
    	}*/
    else if ( (weapon == WP_M4 || weapon == WP_AK47) && (weaponmode & ( 1 << WM_BAYONET ) ) && ( weaponmode & ( 1 << WM_WEAPONMODE2 ) ) )
    {
        mode = "Stab Mode";
    }
    else if ( BG_IsGrenade( weapon ) )
    {
        int sec = 3;

        if ( weapon == WP_FLASHBANG )
        {
            if ( weaponmode & ( 1 << WM_SINGLE) )
                sec = 2;
            else if ( weaponmode & ( 1 << WM_WEAPONMODE2 ) )
                sec = 1;
        } else if ( weapon == WP_SMOKE ) {
            if ( weaponmode & ( 1 << WM_SINGLE) )
                sec = 2;
            else if ( weaponmode & ( 1 << WM_WEAPONMODE2 ) )
                sec = 1;
        }
        else
        {
            if ( weaponmode & ( 1 << WM_SINGLE) )
                sec = 4;
            else if ( weaponmode & ( 1 << WM_WEAPONMODE2 ) )
                sec = 5;
        }

        if ( weaponmode & ( 1 << WM_GRENADEROLL ) )
            mode = va( "%is Roll", sec );
        else
            mode = va( "%is Throw", sec );
    }
    else if ( weapon == WP_M4 || weapon == WP_M249 || weapon == WP_M14 || weapon == WP_MAC10 || weapon == WP_AK47 || weapon == WP_MP5 || weapon == WP_PDW ||
              weapon == WP_SPAS15 )
    {
        // if i want to switch to
        if ( weaponmode & ( 1 << WM_SINGLE) )
            mode = "Semi Auto";
        else
            mode = "Full Auto";
    }
    else if ( !BG_IsMelee(weapon) )
        mode = "Single Shot";
    else
        return;


    CG_Text_Paint2(x,y,scale,colorWhite, mode, 0, 0, 0 );//ITEM_TEXTSTYLE_OUTLINED);
}
extern	vmCvar_t cg_hudStyle;

static void CG_DrawWeaponStatusIcon( int x, int y, int width, int height )
{
    qhandle_t firemode[5];
    qhandle_t	shader;
    int weapon = cg.snap->ps.weapon;
    int weaponmode = cg.snap->ps.stats[STAT_WEAPONMODE];

    firemode[0] = trap_R_RegisterShader( "gfx/2d/hud/default/firemode_1.tga" );
    firemode[1] = trap_R_RegisterShader( "gfx/2d/hud/default/firemode_5.tga" );
    firemode[3] = trap_R_RegisterShader( "gfx/2d/hud/default/firemode_roll.tga" );
    firemode[4] = trap_R_RegisterShader( "gfx/2d/hud/default/firemode_throw.tga" );

    if ( BG_IsMelee( weapon ) || weapon == WP_NONE || weapon == WP_C4 )
        return;

    if ( BG_IsGrenade( weapon ) )
    {
        int sec = 3;

        if ( weapon == WP_FLASHBANG )
        {
            if ( weaponmode & ( 1 << WM_SINGLE) )
                sec = 2;
            else if ( weaponmode & ( 1 << WM_WEAPONMODE2 ) )
                sec = 1;
        } else if ( weapon == WP_SMOKE ) {
            if ( weaponmode & ( 1 << WM_SINGLE) )
                sec = 2;
            else if ( weaponmode & ( 1 << WM_WEAPONMODE2 ) )
                sec = 1;
        }
        else
        {
            if ( weaponmode & ( 1 << WM_SINGLE) )
                sec = 4;
            else if ( weaponmode & ( 1 << WM_WEAPONMODE2 ) )
                sec = 5;
        }

        if ( cg_hudStyle.integer == 1 )
            CG_Text_Paint2( x + 64, y + 42, 0.35f, colorWhite, va("%is",sec), 0,0, ITEM_TEXTSTYLE_SHADOWED  );
        else
            CG_Text_Paint2( x + 105, y + 17, 0.41f, colorWhite, va("%is",sec), 0,0, ITEM_TEXTSTYLE_SHADOWED  );

        if ( weaponmode & ( 1 << WM_GRENADEROLL ) )
            shader = firemode[3];
        else
            shader = firemode[4];
    }
    else if ( weapon == WP_M4 || weapon == WP_M249 || weapon == WP_M14 || weapon == WP_MAC10 || weapon == WP_AK47 || weapon == WP_MP5 || weapon == WP_PDW )
    {
        // if i want to switch to
        if ( weaponmode & ( 1 << WM_SINGLE) )
            shader = firemode[0];
        else
            shader = firemode[1];
    }
    else if ( !BG_IsMelee(weapon) )
        shader = firemode[0];

    CG_DrawPic2( x, y, width, height, shader );
}
#define HUD_STAMINA_MAX		300

void CG_DrawDefconHud( void );

extern	vmCvar_t	cg_hudStyle;
extern	vmCvar_t	cg_hud1PosX;
extern	vmCvar_t	cg_hud1PosY;
extern	vmCvar_t	cg_hud2PosX;
extern	vmCvar_t	cg_hud2PosY;
extern	vmCvar_t	cg_hudAlpha1;
extern	vmCvar_t	cg_hudAlpha2;

void CG_DrawSpectatorHud( void )
{
    CG_DrawHudStatusBar( );
}
void CG_DrawScrutchHud( void )
{
    //	int x;
    //	int y;
    int base_x = cg_hud1PosX.integer;
    int base_y = cg_hud1PosY.integer;
    int i;

    qhandle_t	background = trap_R_RegisterShader( "gfx/2d/hud/default/background.tga" ),
                           staminabar = trap_R_RegisterShader( "gfx/2d/hud/default/staminabar.tga" ),
                                        bullet = trap_R_RegisterShader( "gfx/2d/hud/default/bullet.tga" ),
                                                 frame = trap_R_RegisterShader( "gfx/2d/hud/default/frame.tga" ),
                                                         outline = trap_R_RegisterShader( "gfx/2d/hud/default/outline.tga" );

    CG_DrawHudStatusBar( );

    if ( cg_hudStyle.integer == 2 ) {
        CG_DrawDefconHud();
        return;
    }

    if ( cg_hudStyle.integer != 1 )
        return;

    //if ( cgs.glconfig.vidWidth < 1024 )
    {
        //	base_x = 150;// * ( (float)cgs.glconfig.vidWidth/1024.0f );
        //	base_y = 525;// * ( (float)cgs.glconfig.vidHeight/768.0f ) ;
    }

    {
        vec4_t hcolor_alpha = { 1.0,1.0,1.0, 1.0 };
        hcolor_alpha[3] = cg_hudAlpha1.value;

        if ( hcolor_alpha[3] < 0.0f ||
                hcolor_alpha[3] > 1.0f )
            hcolor_alpha[3] = 0.5f;

        trap_R_SetColor( hcolor_alpha );
        CG_DrawPic2( base_x + 0, base_y + 0, 130,220, background );
        trap_R_SetColor( NULL );
    }

    //
    // stamina bar
    //
    {
        float a = (float)cg.snap->ps.stats[STAT_STAMINA]/(float)HUD_STAMINA_MAX;

        CG_DrawPic2( base_x + 11, base_y + 35 + (float)( (1.0f - ( a ) )*174.0f ) , 41, a*174.0f, staminabar );
    }

    //
    // rounds counter
    //
    {
        float rnds = cg.snap->ps.stats[STAT_ROUNDS];
        float max = BG_GetMaxRoundForWeapon(cg.snap->ps.weapon);
        float x = 15.0f;
        int x2 = 90;

        if ( rnds > max )
            rnds = max;
        if ( max < x )
            x = max;

        if ( cg.snap->ps.stats[STAT_ROUNDS] >= 10 )
            x2 -= 5;

        if ( !BG_IsMelee( cg.snap->ps.weapon ) && !BG_IsGrenade( cg.snap->ps.weapon ) &&
                ( BG_IsPrimary( cg.snap->ps.weapon ) || BG_IsSecondary( cg.snap->ps.weapon ) ) )
            CG_Text_Paint2( base_x + x2, base_y + 51 , 0.35f, colorWhite, va("%i",cg.snap->ps.stats[STAT_ROUNDS] ) , 0,0, ITEM_TEXTSTYLE_SHADOWED );//ITEM_TEXTSTYLE_OUTLINED );

        for ( i = 0; i<x;i++)// i < ( rnds/max )*x; i++ )
        {
            if ( i < ( rnds/max )*x )
                CG_DrawPic2( base_x + 74, base_y + 62 + 112 - i*8, 42,8, bullet );
            else
            {
                vec4_t hcolor = { 1.0,1.0,1.0,0.33f };

                trap_R_SetColor( hcolor );
                CG_DrawPic2( base_x + 74, base_y + 62 + 112 - i*8, 42,8, bullet );
                trap_R_SetColor ( NULL );
            }
        }
    }

    {
        vec4_t hcolor_alpha2 = { 1.0,1.0,1.0,1.0 };
        hcolor_alpha2[3] = cg_hudAlpha2.value;

        if ( hcolor_alpha2[3] < 0.0f ||
                hcolor_alpha2[3] > 1.0f )
            hcolor_alpha2[3] = 0.5f;

        trap_R_SetColor( hcolor_alpha2 );
        CG_DrawPic2( base_x + 0, base_y + 0, 130,220, frame );
        CG_DrawPic2( base_x + 0, base_y + 0, 130,220, outline );
        trap_R_SetColor( NULL );
    }

    CG_DrawHudLocator( base_x + 5, base_y + 25, 96, 192 );

    //
    // clip count
    //
    if ( cg.snap->ps.weapon > WP_SEALKNIFE && !BG_IsMelee(cg.snap->ps.weapon) &&
            cg.snap->ps.weapon != WP_C4 )
    {
        gitem_t *item = BG_FindItemForWeapon( cg.snap->ps.weapon );
        int weapon = cg.predictedPlayerState.weapon;
        int clipGfx = 0;

        if ( BG_IsShotgun( weapon ) )
            clipGfx = 1;
        else if ( weapon == WP_GRENADE )
            clipGfx = 2;
        else if ( weapon == WP_FLASHBANG )
            clipGfx = 3;
        else if ( weapon == WP_SMOKE )
            clipGfx = 3;
        else if ( cg.snap->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) && cg.snap->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ))
            clipGfx = 4;

        if ( item )
        {
            int x2 = 90;

            if ( cg.snap->ps.ammo[item->giAmmoTag] <= 1 /* && ( (cg.time >> 8) & 1*/ &&
                    !BG_IsGrenade( weapon ) && !BG_IsMelee( weapon ) )
                trap_R_SetColor(colorRed);
            else
                trap_R_SetColor(colorWhite);

            if ( cg.snap->ps.ammo[item->giAmmoTag] >= 10 )
                x2 -= 5;

            //	CG_DrawPic2( base_x + 80, base_y + 192, 22, 22, cgs.media.clipIcon[clipGfx] );
            CG_Text_Paint2( base_x + x2 + 4, base_y + 210, 0.35f,( cg.snap->ps.ammo[item->giAmmoTag] <= 1 && ( (cg.time >> 8) & 1 &&
                            !BG_IsGrenade( weapon ) && !BG_IsMelee( weapon ) ) )?colorRed:colorWhite, va("%i",cg.snap->ps.ammo[item->giAmmoTag] ) , 0,0, ITEM_TEXTSTYLE_SHADOWED );//ITEM_TEXTSTYLE_OUTLINED );

            trap_R_SetColor( NULL );
        }
        //	CG_DrawWeaponStatus2( base_x + 20, base_y + 25, 0.35f );
    }
    CG_DrawWeaponStatusIcon( base_x + 20, base_y + 10, 90, 12 );

}

int savedRnds = 0;
int savedClips = 0;
qboolean savedPrimary = qtrue;

void CG_DrawDefconHud( void )
{
    //	int x;
    //	int y;
    int base_x = cg_hud2PosX.integer;
    int base_y = cg_hud2PosY.integer;

    qhandle_t	background = trap_R_RegisterShader( "gfx/2d/hud/defcon/background.tga" ),
                           frame = trap_R_RegisterShader( "gfx/2d/hud/defcon/frame.tga" ),
                                   outline = trap_R_RegisterShader( "gfx/2d/hud/defcon/outline.tga" ),
                                             stamina[3];

    stamina[0]	=	trap_R_RegisterShader( "gfx/2d/hud/defcon/stamina_button1.tga" );
    stamina[1]	=	trap_R_RegisterShader( "gfx/2d/hud/defcon/stamina_button2.tga" );
    stamina[2]	=	trap_R_RegisterShader( "gfx/2d/hud/defcon/stamina_button3.tga" );

    {
        vec4_t hcolor_alpha1 = { 1.0,1.0,1.0, 1.0 };

        hcolor_alpha1[3] = cg_hudAlpha1.value;

        if ( hcolor_alpha1[3] < 0.0f ||
                hcolor_alpha1[3] > 1.0f )
            hcolor_alpha1[3] = 0.5f;

        trap_R_SetColor( hcolor_alpha1 );

        CG_DrawPic2( base_x + 0, base_y + 0, 200,96, background );

        trap_R_SetColor( NULL );
    }

    //
    // stamina buttons
    //
    {
        float staminaV = cg.snap->ps.stats[STAT_STAMINA];
        float temp;
        vec4_t hColor;

        VectorCopy( colorWhite, hColor );

        hColor[3] = 0.0f;

        if ( staminaV > 200 )
        {
            temp = staminaV-200;

            temp = temp/100.0f;

            if ( temp > 1.0f )
                temp = 1.0f;

            hColor[3] = temp;
            staminaV = 200;
        }
        trap_R_SetColor( hColor );
        CG_DrawPic2( base_x + 7, base_y + 77, 41,9, stamina[2] );
        trap_R_SetColor( NULL );

        hColor[3] = 0.0f;

        if ( staminaV > 100 )
        {
            temp = staminaV-100;

            temp = temp/100.0f;

            if ( temp > 1.0f )
                temp = 1.0f;

            hColor[3] = temp;

            staminaV = 100;
        }
        trap_R_SetColor( hColor );
        CG_DrawPic2( base_x + 54, base_y + 77, 41,9, stamina[1] );
        trap_R_SetColor( NULL );

        hColor[3] = 0.0f;

        if ( staminaV > 0 )
        {
            temp = staminaV;

            temp = temp/100.0f;

            if ( temp > 1.0f )
                temp = 1.0f;

            hColor[3] = temp;
        }
        trap_R_SetColor( hColor );
        CG_DrawPic2( base_x + 101, base_y + 77, 41,9, stamina[0] );
        trap_R_SetColor( NULL );
    }

    //
    // rounds counter
    //
    {
        float x = 15.0f;
        int add = 0;
        gitem_t *item ;
        int weapon = cg.snap->ps.weapon;


        if ( BG_IsPrimary( weapon ) ||
                BG_IsSecondary( weapon ) )
        {
            item = BG_FindItemForWeapon( weapon );

            savedRnds = cg.snap->ps.stats[STAT_ROUNDS];
            savedClips = cg.snap->ps.ammo[item->giAmmoTag];

            if ( BG_IsPrimary( weapon ) )
                savedPrimary = qtrue;
            else
                savedPrimary = qfalse;
        }

        if ( savedPrimary && BG_GetPrimary( cg.snap->ps.stats ) == WP_NONE )
        {
            savedRnds = 0;
            savedClips = 0;
        }
        if ( !savedPrimary && BG_GetSecondary( cg.snap->ps.stats ) == WP_NONE )
        {
            savedRnds = 0;
            savedClips = 0;
        }

        if ( savedRnds < 10 )
            add = CG_Text_Width("5",0.45f,0);

        CG_Text_Paint2( base_x + 104 + add,base_y + 40,
                            0.45f, colorWhite, va("%i",savedRnds),0,0, ITEM_TEXTSTYLE_SHADOWED );//ITEM_TEXTSTYLE_OUTLINED );

        if ( cg.snap->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) && cg.snap->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ))
            CG_Text_Paint2( base_x + 130, base_y + 70,
                            0.5, colorWhite, va("%i",cg.snap->ps.ammo[AM_40MMGRENADES]),0,0, ITEM_TEXTSTYLE_SHADOWED );//ITEM_TEXTSTYLE_OUTLINED );
        else
            CG_Text_Paint2( base_x + 130, base_y + 70,
                            0.5, colorWhite, va("%i",savedClips),0,0, ITEM_TEXTSTYLE_SHADOWED );//ITEM_TEXTSTYLE_OUTLINED );
    }

    //
    // grenade counter
    //
    {
        // grenades
        {
            CG_Text_Paint2( base_x + 60, base_y + 60,
                            0.7f, colorWhite, va("%i",cg.snap->ps.ammo[AM_GRENADES] ),0,0, ITEM_TEXTSTYLE_SHADOWED );//ITEM_TEXTSTYLE_OUTLINED );
        }
        // flashbangs
        {
            CG_Text_Paint2( base_x + 10, base_y + 60,
                            0.7f, colorWhite, va("%i",cg.snap->ps.ammo[AM_FLASHBANGS] ),0,0, ITEM_TEXTSTYLE_SHADOWED );//ITEM_TEXTSTYLE_OUTLINED );
        }
    }

    CG_DrawWeaponStatusIcon( base_x + 29, base_y + 4, 109, 14 );
    CG_DrawHudLocatorSmall( base_x + 149, base_y + 7, 40, 80 );

    {
        vec4_t hcolor_alpha2 = { 1.0,1.0,1.0, 1.0 };
        hcolor_alpha2[3] = cg_hudAlpha2.value;

        if ( hcolor_alpha2[3] < 0.0f ||
                hcolor_alpha2[3] > 1.0f )
            hcolor_alpha2[3] = 0.5f;

        trap_R_SetColor( hcolor_alpha2 );

        CG_DrawPic2( base_x + 0, base_y + 0, 200, 96, frame );

        trap_R_SetColor( NULL );
    }


    //
    // clip count
    //
    /*
    if ( cg.snap->ps.weapon > WP_SEALKNIFE && !BG_IsMelee(cg.snap->ps.weapon) &&
    cg.snap->ps.weapon != WP_C4 )
    {
    gitem_t *item = BG_FindItemForWeapon( cg.snap->ps.weapon );
    int weapon = cg.predictedPlayerState.weapon;
    int clipGfx = 0;

    if ( BG_IsShotgun( weapon ) )
    clipGfx = 1;
    else if ( weapon == WP_GRENADE )
    clipGfx = 2;
    else if ( weapon == WP_FLASHBANG )
    clipGfx = 3;
    else if ( weapon == WP_SMOKE )
    clipGfx = 3;
    else if ( cg.snap->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) && cg.snap->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ))
    clipGfx = 4;

    if ( item )
    {
    if ( cg.snap->ps.ammo[item->giAmmoTag] <= 1 && ( (cg.time >> 8) & 1 &&
    !BG_IsGrenade( weapon ) && !BG_IsMelee( weapon ) ) )
    trap_R_SetColor(colorRed);
    else
    trap_R_SetColor(colorWh ite);

    CG_DrawPic2( base_x + 80, base_y + 192, 22, 22, cgs.media.clipIcon[clipGfx] );
    CG_Text_Paint2( base_x + 106, base_y + 210, 0.35f,( cg.snap->ps.ammo[item->giAmmoTag] <= 1 && ( (cg.time >> 8) & 1 &&
    !BG_IsGrenade( weapon ) && !BG_IsMelee( weapon ) ) )?colorRed:colorWhite, va("%i",cg.snap->ps.ammo[item->giAmmoTag] ) , 0,0, 0 );//ITEM_TEXTSTYLE_OUTLINED );

    trap_R_SetColor( NULL );
    }
    CG_DrawWeaponStatus2( base_x + 20, base_y + 25, 0.35f );
    }
    */
}

