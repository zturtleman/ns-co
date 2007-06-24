// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_scoreboard -- draw the scoreboard on top of the game screen

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"


#define	SCOREBOARD_X		(0)

#define SB_HEADER			86
#define SB_TOP				(SB_HEADER+32)

// Where the status bar starts, so we don't overwrite it
#define SB_STATUSBAR		420

#define SB_NORMAL_HEIGHT	40
#define SB_INTER_HEIGHT		16 // interleaved height

#define SB_MAXCLIENTS_NORMAL  ((SB_STATUSBAR - SB_TOP) / SB_NORMAL_HEIGHT)
#define SB_MAXCLIENTS_INTER   ((SB_STATUSBAR - SB_TOP) / SB_INTER_HEIGHT - 1)

// Used when interleaved



#define SB_LEFT_BOTICON_X	(SCOREBOARD_X+0)
#define SB_LEFT_HEAD_X		(SCOREBOARD_X+32)
#define SB_RIGHT_BOTICON_X	(SCOREBOARD_X+64)
#define SB_RIGHT_HEAD_X		(SCOREBOARD_X+96)
// Normal
#define SB_BOTICON_X		(SCOREBOARD_X+32)
#define SB_HEAD_X			(SCOREBOARD_X+64)

#define SB_SCORELINE_X		112

#define SB_RATING_WIDTH	    (6 * BIGCHAR_WIDTH) // width 6
#define SB_SCORE_X			(SB_SCORELINE_X + BIGCHAR_WIDTH) // width 6
#define SB_RATING_X			(SB_SCORELINE_X + 6 * BIGCHAR_WIDTH) // width 6
#define SB_PING_X			(SB_SCORELINE_X + 12 * BIGCHAR_WIDTH + 8) // width 5
#define SB_TIME_X			(SB_SCORELINE_X + 17 * BIGCHAR_WIDTH + 8) // width 5
#define SB_NAME_X			(SB_SCORELINE_X + 22 * BIGCHAR_WIDTH) // width 15

// The new and improved score board
//
// In cases where the number of clients is high, the score board heads are interleaved
// here's the layout

//
//	0   32   80  112  144   240  320  400   <-- pixel position
//  bot head bot head score ping time name
//
//  wins/losses are drawn on bot icon now

static qboolean localClient; // true if local client has been displayed


//================================================================================

/*
================
CG_CenterGiantLine
================
*/
static void CG_CenterGiantLine( float y, const char *string ) {
    float		x;
    vec4_t		color;

    color[0] = 1;
    color[1] = 1;
    color[2] = 1;
    color[3] = 1;

    x = 0.5 * ( 640 - GIANT_WIDTH * CG_DrawStrlen( string ) );

    CG_DrawStringExt( x, y, string, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
}

/*
=================
CG_DrawTourneyScoreboard

Draw the oversize scoreboard for tournements
=================
*/
void CG_DrawOldTourneyScoreboard( void ) {
    const char		*s;
    vec4_t			color;
    int				min, tens, ones;
    clientInfo_t	*ci;
    int				y;
    int				i;

    // request more scores regularly
    if ( cg.scoresRequestTime + 2000 < cg.time ) {
        cg.scoresRequestTime = cg.time;
        trap_SendClientCommand( "score" );
    }

    color[0] = 1;
    color[1] = 1;
    color[2] = 1;
    color[3] = 1;

    // draw the dialog background
    color[0] = color[1] = color[2] = 0;
    color[3] = 1;
    CG_FillRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color );

    // print the mesage of the day
    s = CG_ConfigString( CS_MOTD );
    if ( !s[0] ) {
        s = "Scoreboard";
    }

    // print optional title
    CG_CenterGiantLine( 8, s );

    // print server time
    ones = cg.time / 1000;
    min = ones / 60;
    ones %= 60;
    tens = ones / 10;
    ones %= 10;
    s = va("%i:%i%i", min, tens, ones );

    CG_CenterGiantLine( 64, s );


    // print the two scores

    y = 160;
    if ( cgs.gametype >= GT_TEAM ) {
        //
        // teamplay scoreboard
        //
        CG_DrawStringExt( 8, y, "Red Team", color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
        s = va("%i", cg.teamScores[0] );
        CG_DrawStringExt( 632 - GIANT_WIDTH * strlen(s), y, s, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );

        y += 64;

        CG_DrawStringExt( 8, y, "Blue Team", color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
        s = va("%i", cg.teamScores[1] );
        CG_DrawStringExt( 632 - GIANT_WIDTH * strlen(s), y, s, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
    } else {
        //
        // free for all scoreboard
        //
        for ( i = 0 ; i < MAX_CLIENTS ; i++ ) {
            ci = &cgs.clientinfo[i];
            if ( !ci->infoValid ) {
                continue;
            }
            if ( ci->team != TEAM_FREE ) {
                continue;
            }

            CG_DrawStringExt( 8, y, ci->name, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
            s = va("%i", ci->score );
            CG_DrawStringExt( 632 - GIANT_WIDTH * strlen(s), y, s, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
            y += 64;
        }
    }


}

