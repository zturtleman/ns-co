/*
//
// all code in this file is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net
//
// very old code based on q2:nf code
// this sucks and should be rewritten anyway (or did i do already?)
*/

#include "g_local.h"

void Cmd_Say_f ( gentity_t *ent, int mode, qboolean arg0 );
void NS_GestureForNumber ( gentity_t *ent, int i );
void NS_SendStatusMessageToTeam( gentity_t *affected, int status, int team );

#define MAX_RADIO_MESSAGES	150
// Each of the possible radio messages and their length
radio_msg_t radio_seal_msgs[MAX_RADIO_MESSAGES];
radio_msg_t	radio_tango_msgs[MAX_RADIO_MESSAGES];
int			num_seal_msgs = 0;
int			num_tango_msgs = 0;
int			sealdeathSoundIndex = 0;
int			tangodeathSoundIndex = 0;

void PrecacheRadioSounds()
{
    G_SoundIndex(RADIO_CLICK);
}

void DeleteFirstRadioQueueEntry(gentity_t *ent)
{
    int i;

    if (ent->client->ns.radio_queue_size <= 0)
    {
        //G_Printf("DeleteFirstRadioQueueEntry: attempt to delete without any entries\n");
        return;
    }

    for (i = 1; i < ent->client->ns.radio_queue_size; i++)
    {
        memcpy(&(ent->client->ns.radio_queue[i - 1]),
               &(ent->client->ns.radio_queue[i]),
               sizeof(radio_queue_entry_t));
    }

    ent->client->ns.radio_queue_size--;
}

void DeleteRadioQueueEntry(gentity_t *ent, int entry_num)
{
    int i;

    if (ent->client->ns.radio_queue_size <= entry_num)
    {
        //G_Printf("DeleteRadioQueueEntry: attempt to delete out of range queue entry\n");
        return;
    }

    for (i = entry_num + 1; i < ent->client->ns.radio_queue_size; i++)
    {
        memcpy(&(ent->client->ns.radio_queue[i - 1]),
               &(ent->client->ns.radio_queue[i]),
               sizeof(radio_queue_entry_t));
    }

    ent->client->ns.radio_queue_size--;
}

// RadioThink should be called once on each player per server frame.
void RadioThink(gentity_t *ent)
{
    // Try to clean things up, a bit....
    if (ent->client->ns.radio_power_off)
    {
        ent->client->ns.radio_queue_size = 0;
        return;
    }

    if (ent->client->ns.radio_delay > 0 && (ent->client->ns.radio_queue[0].from_player == ent ) )
        ent->client->ps.eFlags |= EF_RADIO_TALK;
    else
        ent->client->ps.eFlags &= ~EF_RADIO_TALK;

    if (ent->client->ns.radio_delay > 1)
    {
        ent->client->ns.radio_delay--;
        return;
    }
    else if (ent->client->ns.radio_delay == 1)
    {
        DeleteFirstRadioQueueEntry(ent);
        ent->client->ns.radio_delay = 0;
    }

    if (ent->client->ns.radio_queue_size)
    {
        gentity_t *from;
        int check;

        from = ent->client->ns.radio_queue[0].from_player;

        // suddently our caller died! insert dying message.
        if (!ent->client->ns.radio_queue[0].click &&
                (!from->inuse || from->r.contents == CONTENTS_CORPSE ))
        {
            //	G_Printf("insert deathsound: %i %i\n", from->inuse , ( from->r.contents == CONTENTS_CORPSE ) );

            if ( ent->client->sess.sessionTeam == TEAM_RED ) {
                ent->client->ns.radio_queue[0].soundIndex = radio_seal_msgs[sealdeathSoundIndex].soundIndex;
                ent->client->ns.radio_queue[0].length = radio_seal_msgs[sealdeathSoundIndex].soundLength;
                strcpy( ent->client->ns.radio_queue[0].chatString , radio_seal_msgs[sealdeathSoundIndex].chatString );
                ent->client->ns.radio_queue[0].gesture = radio_seal_msgs[sealdeathSoundIndex].signalType;
            }
            else
            {
                ent->client->ns.radio_queue[0].soundIndex = radio_tango_msgs[tangodeathSoundIndex].soundIndex;
                ent->client->ns.radio_queue[0].length = radio_tango_msgs[tangodeathSoundIndex].soundLength;
                strcpy( ent->client->ns.radio_queue[0].chatString , radio_tango_msgs[tangodeathSoundIndex].chatString );
                ent->client->ns.radio_queue[0].gesture = radio_tango_msgs[tangodeathSoundIndex].signalType;
            }


            for (check = 1; check < ent->client->ns.radio_queue_size; check++)
            {
                if (ent->client->ns.radio_queue[check].from_player == from)
                {
                    DeleteRadioQueueEntry(ent, check);
                    check--;
                }
            }
        }

        //trap_R_SendClientCommand
        //stuffcmd(ent, snd_play_cmd);
        G_LocalSound(ent, CHAN_ANNOUNCER, ent->client->ns.radio_queue[0].soundIndex );
        ent->client->ns.radio_queue[0].now_playing = 1;
        ent->client->ns.radio_delay = ent->client->ns.radio_queue[0].length;
        if ( ent->client->ns.radio_queue[0].from_player == ent )
        {
            G_Say( ent, NULL, SAY_RADIO, ent->client->ns.radio_queue[0].chatString );
            if ( ent->client->ns.radio_queue[0].gesture > 0 )
            {
                NS_GestureForNumber( ent, ent->client->ns.radio_queue[0].gesture );
            }
        }
    }
}

int TotalNonClickMessagesInQueue(gentity_t *ent)
{
    int i, count = 0;

    for (i = 0; i < ent->client->ns.radio_queue_size; i++)
    {
        if (!ent->client->ns.radio_queue[i].click)
            count++;
    }

    return count;
}

void AppendRadioMsgToQueue(gentity_t *ent, int msg_num, int click, gentity_t *from_player)
{
    radio_queue_entry_t *newentry;

    //	G_Printf("Appending: %s\n",msg);


    if (ent->client->ns.radio_queue_size >= MAX_RADIO_QUEUE_SIZE)
    {
        //		G_Printf("AppendRadioMsgToQueue: Maximum radio queue size exceeded\n");
        return;
    }

    newentry = &(ent->client->ns.radio_queue[ent->client->ns.radio_queue_size]);

    newentry->from_player = from_player;
    newentry->now_playing = 0;

    if ( ent->client->sess.sessionTeam == TEAM_RED ) {
        newentry->length = radio_seal_msgs[msg_num].soundLength;
    }
    else
        newentry->length = radio_tango_msgs[msg_num].soundLength;

    newentry->click = click;

    if ( ent->client->sess.sessionTeam == TEAM_RED ) {
        strcpy( newentry->chatString , radio_seal_msgs[msg_num].chatString );
        newentry->soundIndex = radio_seal_msgs[msg_num].soundIndex;
        newentry->gesture = radio_seal_msgs[msg_num].signalType;
    }
    else {
        strcpy( newentry->chatString , radio_tango_msgs[msg_num].chatString );
        newentry->soundIndex = radio_tango_msgs[msg_num].soundIndex;
        newentry->gesture = radio_tango_msgs[msg_num].signalType;
    }



    ent->client->ns.radio_queue_size++;
}

void InsertRadioMsgInQueueBeforeClick(gentity_t *ent, int msg_num, gentity_t *from_player)
{
    radio_queue_entry_t *newentry;

    if (ent->client->ns.radio_queue_size >= MAX_RADIO_QUEUE_SIZE)
    {
        //G_Printf("InsertRadioMsgInQueueBeforeClick: Maximum radio queue size exceeded\n");
        return;
    }

    memcpy(&(ent->client->ns.radio_queue[ent->client->ns.radio_queue_size]),
           &(ent->client->ns.radio_queue[ent->client->ns.radio_queue_size - 1]),
           sizeof(radio_queue_entry_t));

    newentry = &(ent->client->ns.radio_queue[ent->client->ns.radio_queue_size - 1]);

    newentry->from_player = from_player;
    newentry->now_playing = 0;
    if ( ent->client->sess.sessionTeam == TEAM_RED )
        newentry->length = radio_seal_msgs[msg_num].soundLength;
    else
        newentry->length = radio_tango_msgs[msg_num].soundLength;

    newentry->click = 0;
    if ( ent->client->sess.sessionTeam == TEAM_RED ) {
        newentry->gesture = radio_seal_msgs[msg_num].signalType;
        strcpy( newentry->chatString , radio_seal_msgs[msg_num].chatString );
        newentry->soundIndex = radio_seal_msgs[msg_num].soundIndex;
    }
    else {
        newentry->gesture = radio_tango_msgs[msg_num].signalType;
        strcpy( newentry->chatString , radio_tango_msgs[msg_num].chatString );
        newentry->soundIndex = radio_tango_msgs[msg_num].soundIndex;
    }

    ent->client->ns.radio_queue_size++;
}

void AddRadioMsg(gentity_t *ent, int msg_num, gentity_t *from_player)
{
    AppendRadioMsgToQueue(ent, msg_num, 0, from_player);
}

char *GetReportMsg( gentity_t *ent )
{
    char *msg;

    if ( !ent )
        return "not_valid_not_valid";

    if ( ent->health <= 20 )
    {
        msg = "report1";
        NS_SendStatusMessageToTeam( ent, MS_HEALTH1, ent->client->sess.sessionTeam );
    }
    else if ( ent->health <= 40 )
    {
        msg = "report2";
        NS_SendStatusMessageToTeam( ent, MS_HEALTH2, ent->client->sess.sessionTeam );
    }
    else if ( ent->health <= 60 )
    {
        msg = "report3";
        NS_SendStatusMessageToTeam( ent, MS_HEALTH3, ent->client->sess.sessionTeam );
    }
    else if ( ent->health <= 80 )
    {
        msg = "report4";
        NS_SendStatusMessageToTeam( ent, MS_HEALTH4, ent->client->sess.sessionTeam );
    }
    else if ( ent->health > 80 )
    {
        msg = "report5";
        NS_SendStatusMessageToTeam( ent, MS_HEALTH5, ent->client->sess.sessionTeam );
    }
    else
    {
        msg = "report5";
        NS_SendStatusMessageToTeam( ent, MS_HEALTH5, ent->client->sess.sessionTeam );
    }

    return msg;
}

char *filterwords[] = {
                          "aaah",
                          "edown",
                          "edown2",
                          "mdown",
                          "hdown",
                          "vdown",
                          "outgrenade",
                          "report5",
                          "report4",
                          "report3",
                          "report2",
                          "report1",
                          "-delimiter-"
                      };
// are we allowed to use that msg?
qboolean FilterRadioMsg( gentity_t *caller, char *msg )
{
    int i;

    if ( !Q_stricmp( msg, "reportin" ) )
    {
        GetReportMsg( caller );
        return qfalse;
    }

    while ( 0 )
    {
        // stop list
        if ( !Q_stricmp( filterwords[i], "-delimiter-" ) )
            break;

        // found word, it is marked as not valid
        if ( !Q_stricmp( msg, filterwords[i] ) )
            return qtrue;

        i++;
    }
    return qfalse;

}
void RadioBroadcast(gentity_t *ent, char *msg, qboolean userinput )
{
    qboolean found = qfalse;
    int	clientNum = ent->client->ps.clientNum;
    int i = 0;

    if ( ent->r.contents == CONTENTS_CORPSE )
        return;

    if ( g_gametype.integer < GT_TEAM )
        return;  // don't allow in a non-team setup...

    if ( ent->client->sess.waiting )
    {
        PrintMsg( ent, "You're dead. You can't use your radio.\n");
        return;
    }
    if ( ent->client->ns.radio_power_off )
    {
        PrintMsg( ent, va("print \"Your radio is currently turned off!\n\"" ) );
        return;
    }

    if ( FilterRadioMsg ( ent, msg ) && userinput )
        return;

    if ( !Q_stricmp( msg , "reportingin") )
        msg = GetReportMsg( ent );

    if ( ent->client->sess.sessionTeam == TEAM_RED ) {
        while ( i < num_seal_msgs )
        {
            if ( !Q_stricmp(radio_seal_msgs[i].soundAlias, msg) )
            {
                found = qtrue;
                break;
            }
            i++;
        }
    }
    else
    {
        while ( i < num_tango_msgs )
        {
            if ( !Q_stricmp(radio_tango_msgs[i].soundAlias, msg) )
            {
                found = qtrue;
                break;
            }
            i++;
        }
    }

    if ( !found )
    {
        trap_SendServerCommand( clientNum, va("print \"'%s' is not a valid radio message.\n\"", msg) );
        return;
    }

    {
        gentity_t *e;

        e = g_entities;

        for (; e < &g_entities[level.num_entities]; e++)
        {
            if (!e->inuse)
                continue;
            // not a client
            if (!e->client)
                continue;
            // not on same team
            if (!OnSameTeam(ent,e) )
                continue;
            // abviously dead.
            if ( e->client->sess.waiting )
                continue;
            // not on same team
            if ( ent->client->ns.radio_channel != e->client->ns.radio_channel )
                continue;
            // stop sending msg twice to once player
            if ( !Q_stricmp( e->classname, "player_bbox_head") )
                continue;

            //			PrintMsg(ent, "sending msg to %s %i\n", e->classname,e->s.number );

            // if it's a bot react to report-in in requests
            if ( !Q_stricmp( msg , "reportin" ) && NS_IsBot( e ) )
                RadioBroadcast( e, "reportingin", qfalse );

            AddRadioMsg(e,i,ent);
        }
    }
}


void Cmd_RadioChannel_f ( gentity_t *ent )
{
    char *chan = "Unknown";

    if ( ent->client->ps.pm_flags & PMF_FOLLOW )
        return;

    if ( ent->client->ns.radio_channel == 0 )
        chan = "Bravo";
    else if ( ent->client->ns.radio_channel == 1 )
        chan = "Charlie";
    else if ( ent->client->ns.radio_channel == 2 )
        chan = "Delta";
    else if ( ent->client->ns.radio_channel == 3 )
        chan = "Echo";
    else if ( ent->client->ns.radio_channel == 4 )
        chan = "Alpha ( Open Channel )";

    ent->client->ns.radio_channel++;

    if ( ent->client->ns.radio_channel > 4 )
        ent->client->ns.radio_channel = 0;
    PrintMsg( ent, "Changed radio channel to %s.\n", chan );
}

char *ConcatArgs(int start);
void Cmd_Radioteam_f(gentity_t *ent)
{
    if ( ent->client->sess.waiting ) {
        PrintMsg( ent, "You can't send radiocommands when you're on the waiting line.\n");
        return;
    }
    if ( ent->client->ps.pm_flags & PMF_FOLLOW )
        return;

    RadioBroadcast(ent, ConcatArgs( 1 ), qtrue );
}

void Cmd_Radio_power_f(gentity_t *ent)
{
    if (g_gametype.integer < GT_TEAM)
        return;  // don't allow in a non-team setup...
    if ( ent->client->ps.pm_flags & PMF_FOLLOW )
        return;

    ent->client->ns.radio_power_off = !ent->client->ns.radio_power_off;

    if (ent->client->ns.radio_power_off)
    {
        trap_SendServerCommand( ent->client->ps.clientNum, va("print \"Radio switched off\n\"") );
        G_LocalSound(ent, CHAN_ANNOUNCER, G_SoundIndex( RADIO_CLICK ) );
    }
    else
    {
        trap_SendServerCommand( ent->client->ps.clientNum, va("print \"Radio switched on\n\"") );
        G_LocalSound(ent, CHAN_ANNOUNCER, G_SoundIndex( RADIO_CLICK ) );
    }
}


typedef enum {
    SIGNAL_NONE,
    SIGNAL_POINT,
    SIGNAL_FIST,
    SIGNAL_WAVE
} singalType_t;

int CG_GetSingalTypeForString( char *string ) {
    // return default
    if (!string)
        return 0;

    if ( !Q_stricmp( string , "point" ) )
        return SIGNAL_POINT;
    else if ( !Q_stricmp( string , "fist" ) )
        return SIGNAL_FIST;
    else if ( !Q_stricmp( string , "wave" ) )
        return SIGNAL_WAVE;
    else if ( !Q_stricmp( string , "none" ) )
        return SIGNAL_NONE;
    else
        return SIGNAL_NONE;
}

void	G_ParseRadioConfigFileForTeam( int team ) {
    char		*text_p;
    int			len;
    //	int			lines = 0;
    char		*token;
    //	float		fps;
    int			skip;
    char		text[20000];
    char		filename[128];
    fileHandle_t	f;

    if ( team == TEAM_BLUE )
        Com_sprintf(filename, sizeof( filename), "scripts/radio_tango.cfg" );
    else
        Com_sprintf(filename, sizeof( filename), "scripts/radio_seals.cfg" );

    // load the file
    len = trap_FS_FOpenFile( filename, &f, FS_READ );
    if ( len <= 0 ) {
        return ;
    }
    if ( len >= sizeof( text ) - 1 ) {
        G_Printf( "File %s (%i>%i)too long\n", text, len, sizeof( text) );
        return ;
    }
    trap_FS_Read( text, len, f );
    text[len] = 0;
    trap_FS_FCloseFile( f );

    // parse the text
    text_p = text;
    skip = 0;	// quite the compiler warning

    //
    // set defaults
    //
    if ( team == TEAM_RED )
        num_seal_msgs = 0;
    else
        num_tango_msgs = 0;

    // parse
    while ( 1 ) {

        token = COM_Parse( &text_p );

        // no token? stop parsing...
        if ( !token || !Q_stricmp( token, "-1") ) {
            break;
        }

        if ( team == TEAM_RED )
            radio_seal_msgs[num_seal_msgs].soundLength = atoi( token );
        else
            radio_tango_msgs[num_tango_msgs].soundLength = atoi( token );

        token = COM_Parse( &text_p );

        // no token? stop parsing...
        if ( !token || !Q_stricmp( token, "-1") ) {
            break;
        }

        if ( team == TEAM_RED )
            radio_seal_msgs[num_seal_msgs].soundIndex = G_SoundIndex( token );
        else
            radio_tango_msgs[num_tango_msgs].soundIndex = G_SoundIndex( token );

        token = COM_Parse( &text_p );

        // no token? stop parsing...
        if ( !token || !Q_stricmp( token, "-1") ) {
            break;
        }

        // start parsing seals part
        if ( !Q_stricmp( token, "{" ) )
        {
            while ( 1 )
            {
                // get new token
                token = COM_Parse( &text_p );

                if ( !token || !Q_stricmp( token, "$EOF") )
                {
                    G_Error("Unexpected end of file (file:%s)\n", filename );
                    break;
                }

                // stop parsing the string
                if ( !Q_stricmp( token , "}" ) )
                    break;

                {
                    char temp[256];

                    if ( team == TEAM_RED )
                    {
                        if ( strlen( radio_seal_msgs[num_seal_msgs].chatString ) <= 0 )
                            Com_sprintf( temp, sizeof( temp ), "%s", token );
                        else
                            Com_sprintf( temp, sizeof( temp ), "%s %s", radio_seal_msgs[num_seal_msgs].chatString , token );

                        strcpy( radio_seal_msgs[num_seal_msgs].chatString , temp );
                    }
                    else
                    {
                        if ( strlen( radio_tango_msgs[num_tango_msgs].chatString ) <= 0 )
                            Com_sprintf( temp, sizeof( temp ), "%s", token );
                        else
                            Com_sprintf( temp, sizeof( temp ), "%s %s", radio_tango_msgs[num_tango_msgs].chatString , token );

                        strcpy( radio_tango_msgs[num_tango_msgs].chatString , temp );
                    }
                }

            }
        }


        if ( team == TEAM_RED )
        {
            if (!Q_stricmp( radio_seal_msgs[num_seal_msgs].soundAlias , "aaah") )
                sealdeathSoundIndex = num_seal_msgs;

            radio_seal_msgs[num_seal_msgs].signalType = CG_GetSingalTypeForString( COM_Parse( &text_p ) );

            Com_sprintf( radio_seal_msgs[num_seal_msgs].soundAlias,sizeof(radio_seal_msgs[num_seal_msgs].soundAlias), "%s",  COM_Parse( &text_p  ) );

            num_seal_msgs++;

            if ( num_seal_msgs >= MAX_RADIO_MESSAGES )
                G_Error("num_seal_msgs >= MAX_RADIO_MESSAGES");
        }
        else
        {
            if (!Q_stricmp( radio_tango_msgs[num_tango_msgs].soundAlias , "aaah") )
                tangodeathSoundIndex = num_tango_msgs;

            radio_tango_msgs[num_tango_msgs].signalType = CG_GetSingalTypeForString( COM_Parse( &text_p ) );

            Com_sprintf( radio_tango_msgs[num_tango_msgs].soundAlias,sizeof(radio_tango_msgs[num_tango_msgs].soundAlias), "%s",  COM_Parse( &text_p  ) );

            num_tango_msgs++;

            if ( num_tango_msgs >= MAX_RADIO_MESSAGES )
                G_Error("num_tango_msgs >= MAX_RADIO_MESSAGES");
        }
    }

}
