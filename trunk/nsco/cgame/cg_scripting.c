#include "cg_local.h"
#include "../ui/ui_shared.h"
#include "regexp.h"
//
// all code in this file is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#define MAX_SCRIPT_PARAMETER	512

typedef enum {
    OP_SAME,			// ==
    OP_NOTSAME,		// !=
    OP_SMALLERTHAN,		// <
    OP_BIGGERTHAN,		// >
    OP_SMALLERSAME,		// <=
    OP_BIGGERSAME,		// >=

    OP_REGEXP,			// /~

    OP_BITAND 			// &
} nsslOperator_t;

typedef enum {
    CT_NONE,

    CT_FLOAT,
    CT_INT,
    CT_STRING,

    CT_TYPES,
} nsslCastType_t;

typedef enum {
    MATH_NONE,

    MATH_ADD,
    MATH_SUBTRACT,
    MATH_MULTIPLY,
    MATH_DIVIDE
} nssl_mathop_t;

typedef struct {
    char	math_char;
    int		math_op;
} nssl_math_ops_t;

static nssl_math_ops_t	math_ops[] = {
                                        { '+', MATH_ADD },
                                        { '-', MATH_SUBTRACT },
                                        { '*', MATH_MULTIPLY },
                                        { '/', MATH_DIVIDE }
                                    };
// casttype charlength
// 0, (float),(int),(string)
int ctLength[] = { 0, 7,5,8, 0 };

typedef struct {
    int				castType;
    char			value[256];
} nsslCurrentToken_t;

#define Q3_LINEBREAK	";"
#define NSSL_VERSION	"1.3"

int cur_token;

vmCvar_t	nssl_debug;
vmCvar_t	nssl_init;

void ClientScript_ProcessWhileLoop( int num );
void ClientScript_Help_f ( void );
void ClientScript_GetCommand( char cmd[512] );
void ClientScript_GetValue( char	value[256], const char *token, qboolean cvarScan );
float ClientScript_GetValueFloat( const char *token, qboolean cvarScan );

//
//===============================================================================

void QDECL NSSL_Printf( const char *msg, ... ) {
    va_list		argptr;
    char		text[1024];

    va_start (argptr, msg);
    vsprintf (text, msg, argptr);
    va_end (argptr);

    Com_sprintf( text, 1024, S_COLOR_YELLOW "NSSL: %s", text );

    CG_Printf(text);
}

void ClientScript_Init( void )
{
    trap_Cvar_Register( &nssl_debug, "nssl_debug", "0", 0 );
    trap_Cvar_Register( &nssl_init, "nssl_init", "1", CVAR_LATCH );
}

const char *ClientScript_NextToken( void )
{
    const char *token;

    cur_token++;
    token = CG_Argv( cur_token );

    if ( strlen( token ) <= 0 )
        return NULL;

    if ( nssl_debug.value == 2 )
        NSSL_Printf( "token %i: %s\n", cur_token, token );

    return token;
}

const char *ClientScript_CurrentToken ( void )
{
    const char *token;

    token = CG_Argv( cur_token );

    if ( strlen( token ) <= 0 )
        return NULL;

    if ( nssl_debug.value == 2 )
        NSSL_Printf( "token %i: %s\n", cur_token, token );
    return token;
}

const char *ClientScript_PrevToken ( void )
{
    const char *token;

    cur_token--;
    token = CG_Argv( cur_token );

    if ( strlen( token ) <= 0 )
        return NULL;

    if ( nssl_debug.value == 2 )
        NSSL_Printf( "token %i: %s\n", cur_token, token );
    return token;
}

void ClientScript_ExecText( char *text )
{
    strcat( text, ";" );

    if ( nssl_debug.value == 1 )
        NSSL_Printf( "Execing Text: %s\n", text );

    trap_SendConsoleCommand( text );
}


qboolean ClientScript_IsNumber( const char *token )
{
    char c;

    if ( !token )
        return qfalse;

    c = token[0];

    if ( c < '0' || c > '9' ) {
        return qfalse;
    }
    return qtrue;
}

qboolean ClientScript_Operate( char value1[256], int function, char value2[256] )
{
    if ( function <= OP_NOTSAME )
    {
        // compare it
        if ( !Q_stricmp( Q_CleanStr( value1 ), Q_CleanStr( value2 ) ) ) // it IS the same
        {
            if ( function == OP_NOTSAME ) // but it shouldn't be the same
                return 0;

            return 1; // it's the same
        }
        else if ( ClientScript_IsNumber( value1 ) && atof ( value1 ) == atof ( value2 ) ) // the same
        {
            if ( function == OP_NOTSAME )
                return 0;

            // value is NOT the same so execute the text
            return 1;
        }
        else
        {
            // we actually didn't want the same
            if ( function == OP_NOTSAME )
                return 1;
        }
    }
    else
    {
        float	i_value1 = atof( value1 );
        float	i_value2 = atof( value2 );

        switch ( function )
        {
        case OP_SMALLERTHAN:
            if ( i_value1 < i_value2 )
                return 1;
            break;
        case OP_BIGGERTHAN:
            if ( i_value1 > i_value2 )
                return 1;
            break;
        case OP_SMALLERSAME:
            if ( i_value1 <= i_value2 )
                return 1;
            break;
        case OP_BIGGERSAME:
            if ( i_value1 >= i_value2 )
                return 1;
            break;
        case OP_BITAND:
            if ( (int)i_value1 & (int)i_value2 )
                return 1;
            break;
        case OP_REGEXP:
            {
                regexp *test;
                // value2 is a expression: \.*\
                // value1 is a string: bla
                // term is: if bla =~ "\.*\"
                test = regcomp( Q_CleanStr( value2 ) );
                return regexec( test, Q_CleanStr( value1 ) );
            }
            break;
        default:
            break;
        }
    }
    return 0;
}

void ClientScript_GetScriptCommand( char	value[256], const char *token )
{
    int size = 256;

    // remove *
    token++;

    if ( token[0] == 'c' &&	token[1] == 'h' && token[2] == 'a' && token[3] == 't' )
    {
        int height = cg_chatHeight.integer;
        int num;

        token +=4;

        num = atoi(token);
        if ( num>height )
            num = height;

        if ( strlen(cgs.ChatMsgs[(cgs.ChatPos - 1 - num) % height]) > 0 )
            strcpy( value, cgs.ChatMsgs[(cgs.ChatPos - 1 - num) % height] );
        else
            strcpy( value, "<empty>" );
    }
    else if ( token[0] == 'r' && token[1] == 'a' && token[2] == 'n' && token[3] == 'd' )
    {
        float mod;

        token +=4;

        mod = ClientScript_GetValueFloat(token, qfalse);

        if ( mod == 0.0f )	mod = 1;

        strcpy( value, va("%f", random() * mod ) );
    }
    else if ( !Q_stricmp( token, "getkey") )
    {
        int key;
        char cha;
        ClientScript_GetValue( value, ClientScript_NextToken(), qtrue );
        key = trap_Key_GetKey( value );
        cha = key;
        if ( key == -1 ) {
            strcpy( value, "<???>" );
            return;
        }
        Com_sprintf(value,size,"%c",key);
    }
    else if ( !Q_stricmp( token, "chasing") )
    {
        if ( cg.predictedPlayerState.pm_flags & PMF_FOLLOW )
            strcpy( value, "1" );
        else
            strcpy( value, "0" );
    }
    else if ( !Q_stricmp( token, "spectator") )
    {
        if ( cg.predictedPlayerState.persistant[PERS_TEAM] == TEAM_SPECTATOR || cg.predictedPlayerState.pm_type == PM_SPECTATOR || cg.predictedPlayerState.pm_type == PM_NOCLIP )
            strcpy( value, "1" );
        else
            strcpy( value, "0" );
    }
    else if (!Q_stricmp( token, "clips") )
    {
        gitem_t *item = BG_FindItemForWeapon( cg.predictedPlayerState.weapon );

        if ( !item ) {
            strcat( value, "0" );
            return;
        }
        strcpy( value, va( "%i", cg.predictedPlayerState.ammo[ BG_FindItemForWeapon( cg.predictedPlayerState.weapon )->giAmmoTag ] ) );
    }
    else if (!Q_stricmp( token, "weapon") )
    {
        gitem_t *item = BG_FindItemForWeapon( cg.predictedPlayerState.weapon );

        if ( !item ) {
            strcat( value, "nothing" );
            return;
        }
        strcpy( value, BG_FindItemForWeapon( cg.predictedPlayerState.weapon )->pickup_name );
    }
    else if (!Q_stricmp( token, "health") )
        strcpy( value, va( "%i", cg.predictedPlayerState.stats[STAT_HEALTH] ) );
    else if (!Q_stricmp( token, "rounds") )
        strcpy( value, va( "%i", cg.predictedPlayerState.stats[STAT_ROUNDS] ) );
    else if (!Q_stricmp( token, "stamina") )
        strcpy( value, va( "%i", cg.predictedPlayerState.stats[STAT_STAMINA] ) );
    else if (!Q_stricmp( token, "ping") )
        strcpy( value, va( "%i", cg.predictedPlayerState.ping ) );
    else if ( token[0] == 's' && token[1] == 't' && token[2] == 'a' && token[3] == 't' )
    {
        token+=4;

        if ( atoi( token ) >= MAX_STATS )
        {
            NSSL_Printf( "*stat%s > MAX_STATS\n", token );
            strcpy( value, "<???>" );
            return;
        }
        strcpy( value, va( "%i", cg.predictedPlayerState.stats[atoi(token)] ) );
    }
    else if ( token[0] == 'p' && token[1] == 'e' && token[2] == 'r' && token[3] == 's' )
    {
        token+=4;

        if ( atoi( token ) >= MAX_PERSISTANT )
        {
            NSSL_Printf( "*pers%s > MAX_PERSISTANT\n", token );
            strcpy( value, "<???>" );
            return;
        }
        strcpy( value, va( "%i", cg.predictedPlayerState.persistant[atoi(token)] ) );
    }
    else if (!Q_stricmp( token, "pm_type") )
        strcpy( value, va("%i", cg.predictedPlayerState.pm_type ) );
    else if (!Q_stricmp( token, "time") )
        strcpy( value, va("%i", cg.time ) );
    else if (!Q_stricmp( token, "qcmd_active") )
        strcpy( value, va("%i", cg.viewCmd ) );
    else
        strcpy( value, "<???>" );
}

int ClientScript_CastType( const char *token )
{

    if ( !token )
        return CT_NONE;

    if (token[0] == '(' &&
            token[1] == 'i' &&
            token[2] == 'n' &&
            token[3] == 't' &&
            token[4] == ')' )
    {
        return CT_INT;
    }
    else if (token[0] == '(' &&
             token[1] == 'f' &&
             token[2] == 'l' &&
             token[3] == 'o' &&
             token[4] == 'a' &&
             token[5] == 't' &&
             token[6] == ')' )
    {
        return CT_FLOAT;
    }
    else if (token[0] == '(' &&
             token[1] == 's' &&
             token[2] == 't' &&
             token[3] == 'r' &&
             token[4] == 'i' &&
             token[5] == 'n' &&
             token[6] == 'g' &&
             token[7] == ')' )
    {
        return CT_STRING;
    }

    return CT_NONE;
}

/*
===================
Client Script
Do Calc

Do NSSL calculations with variables,gamepointers and values
usage:

calc:$variable+\*gamepointer-value
calc:$test+\*health-5
===================
*/
int ClientScript_GetMathOpForChar( char math_char )
{
    int		i;

    for ( i = 0 ; i < sizeof( math_ops ) / sizeof( math_ops[0] ) ; i++ ) {
        if ( math_ops[i].math_char == math_char )
            return math_ops[i].math_op;
    }
    return MATH_NONE;
}
qboolean ClientScript_IsMathOpForChar( char math_char )
{
    int		i;

    for ( i = 0 ; i < sizeof( math_ops ) / sizeof( math_ops[0] ) ; i++ ) {
        if ( math_ops[i].math_char == math_char )
            return qtrue;
    }
    return qfalse;
}
float ClientScript_Calc( float value1, float value2, int math_operator )
{
    switch ( math_operator )
    {
    case MATH_ADD:
        return value1+value2;
        break;
    case MATH_SUBTRACT:
        return value1-value2;
        break;
    case MATH_MULTIPLY:
        return value1*value2;
        break;
    case MATH_DIVIDE:
        if ( value2 == 0.0f )
        {
            NSSL_Printf("^1Error: Moron divide by ZERO.\n");
            return value1;
        }
        return value1/value2;
        break;
    default:
        NSSL_Printf("Unkown math operator %i\n", math_operator);
        return (value1)?value1:value2;
    }
}
void ClientScript_DoCalc( char	value[256], const char *token )
{
    int lastoperator = 0;
    float previousvalue;
    float currentvalue;
    char currentstring[256];

    token++;
    token++;
    token++;
    token++;
    token++;

    strcpy( currentstring, "" );

    while ( token && token != NULL && token[0] != ' ' && strlen(token) > 0 )
    {
        // escape character? then the next operation will never be a math op
        if ( token[0] == '\\' )
            token++;
        else if ( ClientScript_IsMathOpForChar( token[0] ) )
        {
            if ( nssl_debug.integer )
                NSSL_Printf("Found Math Operator: %c\n", token[0] );
            // prepare for first operation
            if ( lastoperator == MATH_NONE )
            {
                // set value
                previousvalue = ClientScript_GetValueFloat(currentstring, qfalse);
                // clear current value
                strcpy(currentstring,"");

                lastoperator = ClientScript_GetMathOpForChar( token[0] );
            }
            else
            {
                // set 2nd value
                currentvalue = ClientScript_GetValueFloat(currentstring, qfalse);
                // clear current value
                strcpy(currentstring, "" );

                // calc
                previousvalue = ClientScript_Calc( previousvalue, currentvalue, lastoperator );

                // set new operator for next calc
                lastoperator = ClientScript_GetMathOpForChar( token[0] );
            }
            token++;
            continue;
        }
        Com_sprintf( currentstring, sizeof(currentstring), "%s%c", currentstring, token[0] );
        token++;
    }
    // there is something left in our buffer so we have to calc that
    if ( strlen(currentstring) > 0 )
    {
        // set value
        currentvalue = ClientScript_GetValueFloat(currentstring, qfalse);

        currentvalue = ClientScript_Calc( previousvalue, currentvalue, lastoperator );
    }

    Com_sprintf( value, 256, "%f", currentvalue );
}

/*
===================
Client Script
RegExp

RegExp for NSSL
usage: %string,string match
===================
*/
void ClientScript_RegExp( char value[256], const char *token )
{
    char *_token;
    char string[2][256];
    char temp[256];
    regexp *reg_exp;
    int result;
    int len;

    token++;
    _token = strchr( token, ',' );

    if (!_token)
    {
        NSSL_Printf("Syntax Warning: Missing TYPE after '%s'\n", ClientScript_PrevToken() );
        strcpy( value, "<error>");
        return;
    }
    len = strlen(token)-strlen(_token);
    _token++;

    Q_strncpyz( temp, token, len+1 );

    ClientScript_GetValue( string[0], temp, qfalse );
    ClientScript_GetValue( string[1], _token, qfalse );

    // string[1] is a expression: \.*\
    // string[0] is a string: bla
    reg_exp = regcomp( string[1] );
    result = regexec( reg_exp, string[0] );

    // variable parsed?
    if ( result && reg_exp->startp[1] != NULL && reg_exp->endp[1] != NULL )
    {
        len = strlen(reg_exp->startp[1]) - strlen(reg_exp->endp[1]);

        Q_strncpyz( value, reg_exp->startp[1], len+1);
        if ( nssl_debug.integer )
            NSSL_Printf("Variable Parsed: [%s] '%s','%s'\n", value, reg_exp->startp[1], reg_exp->endp[1] );
    }
    else {
        NSSL_Printf("Syntax Warning: %s doesn't match %s\n", string[0],string[1] );
        strcpy( value, "<error>");
        return;
    }
}

/*
===================
Client Script
Get Value

Get a special value or just copy a token
===================
*/
float ClientScript_GetValueFloat( const char *token, qboolean cvarScan )
{
    char value[256];
    ClientScript_GetValue( value, token, cvarScan );
    return atof( value );
}

void ClientScript_GetValue( char	value[256], const char *token, qboolean cvarScan )
{
    char temp[256];
    int op = 0;
    int casttype = ClientScript_CastType( token );
    qboolean cleanstring = qfalse;

    if ( casttype != CT_NONE )
    {
        token += ctLength[casttype];

        if ( nssl_debug.integer )
            NSSL_Printf("Casttype: %i token: %s\n", casttype, token );
    }

    if ( token[0] == '+' ||	token[0] == '-' )
    {
        if ( token[0] == '-' )
            op = 2;
        else
            op = 1;

        token++;
    }

    if ( token[0] == '~' )
    {
        token++;
        cleanstring = qtrue;
    }

    if ( token[0] == '*' )
        ClientScript_GetScriptCommand( value, token );
    else if ( token[0] == 'c' && token[1] == 'a' &&	token[2] == 'l' && token[3] == 'c' &&
              token[4] == ':')
        ClientScript_DoCalc( value, token );
    else if ( token[0] == '%' ) // parse a string, ~$string,$parse
        ClientScript_RegExp( value, token );
    // it's a cvar
    else if ( token[0] == '$' )
    {
        token++;
        if ( token[0] == '$' ) // double evaluation
        {
            token++;
            trap_Cvar_VariableStringBuffer( token, value, 256 );
            token = value;
        }
        trap_Cvar_VariableStringBuffer( token, value, 256 );
    }
    else
    {
        // not a number see if it's a cvar sometimes we need to scan cvars without
        // the evaluation symbol
        if ( !ClientScript_IsNumber( token ) && cvarScan )
        {
            trap_Cvar_VariableStringBuffer( token, temp, 256 );

            if ( strlen ( temp ) > 0 )
                strcpy( value, temp );
            else
                strcpy( value, token );
        }
        else
            strcpy( value, token );
    }

    if ( op )
    {
        if ( op == 2 )
            strcpy( temp, "-" );
        else
            strcpy( temp, "+" );
        strcat( temp, value );
        strcpy( value, temp );
    }

    switch ( casttype ) {
    case CT_INT:
        strcpy( value, va( "%i", atoi( value ) ) );
        break;
    case CT_FLOAT:
        strcpy( value, va( "%f", atof( value ) ) );
        break;
    case CT_STRING:
    default:
        break;
    }

    if ( cleanstring )
        strcpy( value, Q_CleanStr( value ) );

    if ( nssl_debug.value == 2 )
        NSSL_Printf( "NSSL: Evaulted %s to %s\n", token, value );
}

/*
===================
Client Script
Request Linebreak

Is there a linebrake char at the beginning or at the end?
===================
*/
qboolean ClientScript_Linebreak( const char *token )
{
    if ( token[0] == '#' )
        return qtrue;
    else if ( token[strlen(token)-1] == '#' )
        return qtrue;

    return qfalse;
}

/*
===================
Client Script
Get Command

This reads the contents of a { } directive
===================
*/
void ClientScript_GetCommand( char	cmd[512] )
{
    int openSubs;
    const char *token;

    openSubs = 0;

    token = ClientScript_NextToken();

    strcpy( cmd, "" );

    if ( !token || strlen(token) <= 0 )
    {
        NSSL_Printf( "Wrong syntax: '%s' expected '{'\n", token );
        return;
    } // if it's more than just a single { then we should remove it
    else if ( strlen( token ) > 1 && token[0] == '{' )
    {
        // remove the first char {
        token++;

        if ( token[0] == '}' )
        {
            NSSL_Printf( "Warning empty command\n" );
            return;
        }
        goto copy;
    }
    else if ( Q_stricmp( token, "{" ) != 0 )
    {
        NSSL_Printf( "Wrong syntax: '%s' expected '{'\n", token );
        return;
    }

    while ( token && strlen(token) > 0  )
    {
        char _token[256];

        token = ClientScript_NextToken();

        if ( !token )
            break;

        if ( !Q_stricmp( token , "{" ) )
            openSubs++;
        if ( !Q_stricmp( token , "}" ) )
        {
            if ( openSubs > 0 )
                openSubs--;
            else
                break;
        }
copy:
        // a rare case, if quit==1 then {quit}, remove the } we removed the { previously
        if ( strlen(token) > 1 && token[strlen(token)-1] == '}' && openSubs == 0 )
        {
            strcpy( _token, token );
            _token[strlen(token)-1] = '\0';
            token = _token;
        }

        ClientScript_GetValue( _token, token, qfalse );
        token = _token;

        if ( openSubs == 0 && ClientScript_Linebreak(token) )
        {
            if ( strlen( token ) > 1 )
            {
                strcpy(_token,token);
                if ( _token[0] == '#' )
                    _token[0] = ' ';
                if ( _token[strlen(_token)-1] == '#' )
                    _token[strlen(_token)-1] = '\0';

                // insert the modified token
                strcat( cmd, Q3_LINEBREAK );
                strcat( cmd, _token );
                strcat( cmd, " " );
            }
            else
            {
                // just append a Q3_LINEBREAK but not the token, it's only
                // a # char
                strcat( cmd, "; ");
            }
        }
        else
        {
            strcat( cmd, token ); // append token
            strcat( cmd, " " );   // and a space
        }
    }

    if ( nssl_debug.value == 1 )
        NSSL_Printf( "NSSL: Parsed {}: %s\n", cmd );
}

/*
===================
Client Script
Modify

Parses the modify parameter from a string
format:
modify <cvar> +/-<value>

===================
*/
void ClientScript_Modify ( void )
{
    const	char	*token; // a pointer
    char	cvar[256];	// the cvar i parse from the big string
    char	modifier[256];	// the value i parse from the big string
    char	inValue[256];
    float	cvarResult; // the cvar buffer i use to modify
    qboolean	useElse = qtrue;

    // get the cvar
    token = ClientScript_NextToken();

    if (!token)
    {
        NSSL_Printf( "usage: modify <cvar> <modifier>\n");
        return;
    }

    strcpy( cvar, token );
    ClientScript_GetValue( inValue, cvar, qtrue );  // convert raw cvar into result wihtout $

    // get the modifier
    token = ClientScript_NextToken();

    if (!token)
    {
        NSSL_Printf( "usage: modify <cvar> <modifier>\n");
        return;
    }

    // get the string we want to modify it by
    ClientScript_GetValue( modifier, token, qtrue );

    if ( nssl_debug.value == 1 )
        NSSL_Printf( "NSSL: Modified %s from %s by %s\n",cvar, inValue, modifier );

    if ( ClientScript_IsNumber( inValue ) ||
            modifier[0] == '+' ||
            modifier[0] == '-' )
    {
        cvarResult = atof( inValue );
        cvarResult += atof( modifier );

        trap_Cvar_Set( cvar, va("%f",cvarResult ) );
    }
    else
    {
        strcat( inValue, modifier );

        trap_Cvar_Set( cvar, va("%s", inValue ) );
    }

}

int ClientScript_GetOperator( const char *token )
{
    if ( !Q_stricmp( token, "==" ) )
        return OP_SAME;
    else if ( !Q_stricmp( token, "!=" ) )
        return OP_NOTSAME;
    else if ( !Q_stricmp( token, "<" ) )
        return OP_SMALLERTHAN;
    else if ( !Q_stricmp( token, ">" ) )
        return OP_BIGGERTHAN;
    else if ( !Q_stricmp( token, "<=" ) )
        return OP_SMALLERSAME;
    else if ( !Q_stricmp( token, ">=" ) )
        return OP_BIGGERSAME;
    else if ( !Q_stricmp( token, "&" ) )
        return OP_BITAND;
    else if ( !Q_stricmp( token, "=|" ) )
        return OP_REGEXP;
    else
    {
        if ( !token || Q_stricmp( token, "==" ) != 0 )
        {
            NSSL_Printf( "Wrong syntax: '%s' expected '==,!=,<,>,<=,>=,=|'\n", token );
            return 0;
        }
    }
    return 0;
}


/*
===================
Client Script
GFX Handle

Parses the while parameter from a string
format:
while <cvar> == <value/string> { ... } <sleep in ms> <timetolive>

== can also be !=,<=,>=,<,>
===================
*/
#define	MAX_SCRIPT_GFX		 128

typedef struct {
    int			x;
    int			y;
    int			w;
    int			h;
    int			flags;

    float		color[4];
    float		scale;

    char		groupname[128];
    char		text[256];

    qhandle_t	shader;
    qboolean	inuse;
} nsslGFXTable_t;

int numScriptGFX = 0;
nsslGFXTable_t		scriptGFXTable[MAX_SCRIPT_GFX];

int GetFreeGFXSlot( void )
{
    int i = 0;

    for (i=0;i<MAX_SCRIPT_GFX;i++)
    {
        if ( !scriptGFXTable[i].inuse )
        {
            memset( &scriptGFXTable[i],0,sizeof(scriptGFXTable[i]) );
            return i;
        }
    }
    return 0;
}

void AddGFXLoop( char groupname[128], int x,int y, int w, int h, float r,float g, float b, float a, qhandle_t shader )
{
    int num = GetFreeGFXSlot( );
    nsslGFXTable_t	*GFX = &scriptGFXTable[num];

    if ( nssl_debug.value == 1 )
        NSSL_Printf( "Added(%i): %s x %i y %i w %i h %i r %f g %f b %f a %f\n",num,groupname, x,y,w,h,r,g,b,a );

    strcpy( GFX->groupname, groupname );
    GFX->x = x;
    GFX->y = y;
    GFX->w = w;
    GFX->h = h;

    GFX->color[0] = r;
    GFX->color[1] = g;
    GFX->color[2] = b;
    GFX->color[3] = a;

    GFX->shader = shader;

    GFX->inuse = qtrue;
}

void ClientScript_ProcessGFX( int num )
{
    nsslGFXTable_t	*GFX = &scriptGFXTable[num];

    if ( !GFX->inuse )
        return;

    if ( strlen(GFX->text) > 0 )
        CG_Text_Paint( GFX->x,GFX->y,GFX->scale,GFX->color,GFX->text,0,0, GFX->flags );
    else if ( GFX->shader ) {
        trap_R_SetColor( GFX->color );
        CG_DrawPic( GFX->x,GFX->y,GFX->w,GFX->h, GFX->shader );
        trap_R_SetColor( NULL );
    } else {
        CG_FillRect( GFX->x,GFX->y,GFX->w,GFX->h, GFX->color );
    }
}

// addgfx <x> <y> <w> <h> <r> <g> <b> <a> <shader loc>
void ClientScript_DeleteGfxGroup( void )
{
    int i = 0;
    const char *token = ClientScript_NextToken();

    if (!token)
    {
        NSSL_Printf("Syntax Warning: Missing Groupname for delgroup.\n");
        return;
    }

    if ( nssl_debug.integer )
        NSSL_Printf("Removing Group %s\n", token );

    for (i = 0; i < MAX_SCRIPT_GFX; i++ )
    {
        if ( !scriptGFXTable[i].inuse )
            continue;

        if ( !Q_stricmp( token, scriptGFXTable[i].groupname ) )
            scriptGFXTable[i].inuse = qfalse;
    }
}


void AddGFXText( char groupname[128], char text[256], int x,int y, float scale, float r,float g, float b, float a, int flags )
{
    int num = GetFreeGFXSlot( );
    nsslGFXTable_t	*GFX = &scriptGFXTable[num];

    if ( nssl_debug.value == 1 )
        NSSL_Printf( "Added TEXT(%i): %s x %i y %i r %f g %f b %f a %f\n",num,groupname, x,y,r,g,b,a );

    strcpy( GFX->groupname, groupname );
    strcpy( GFX->text, text );

    GFX->x = x;
    GFX->y = y;
    GFX->flags = flags;

    GFX->scale = scale;

    GFX->color[0] = r;
    GFX->color[1] = g;
    GFX->color[2] = b;
    GFX->color[3] = a;

    GFX->inuse = qtrue;
}
void ClientScript_AddString( void )
{
    const char *token;
    char groupname[128];
    char text[256];
    char _token[256];
    int		x,y,flags;
    float scale,r,g,b,a;

    x = y = 64;
    r = g = b = a = 1.0f;

    token = ClientScript_NextToken();
    if ( token )
        ClientScript_GetValue( groupname, token, qtrue );
    else {
        NSSL_Printf("Syntax Warning: No groupname! GFX not added.\n");
        return;
    }

    ClientScript_GetCommand( text );

    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        x = atoi( _token );
    }

    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        y = atoi( _token );
    }

    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        scale = atof( _token );
    }

    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        flags = atoi( _token );
    }

    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        r = atof( _token );
    }

    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        g = atof( _token );
    }

    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        b = atof( _token );
    }

    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        a = atof( _token );
    }

    AddGFXText( groupname, text, x, y, scale, r, g, b, a, flags );
}

void ClientScript_AddGfx( void )
{
    const char *token;
    char groupname[128];
    char _token[256];
    int x,y,w,h;
    float r,g,b,a;
    qhandle_t shader = 0;

    x = y = w = h = 64;
    r = g = b = a = 1.0f;

    token = ClientScript_NextToken();
    if ( token )
        ClientScript_GetValue( groupname, token, qtrue );
    else {
        NSSL_Printf("Syntax Warning: No groupname! GFX not added.\n");
        return;
    }

    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        x = atoi( _token );
    }

    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        y = atoi( _token );
    }

    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        w = atoi( _token );
    }

    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        h = atoi( _token );
    }


    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        r = atof( _token );
    }


    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        g = atof( _token );
    }


    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        b = atof( _token );
    }


    token = ClientScript_NextToken();
    if ( token )
    {
        ClientScript_GetValue( _token, token, qtrue );
        a = atof( _token );
    }


    token = ClientScript_NextToken();
    if ( token ) {
        ClientScript_GetValue( _token, token, qtrue );

        shader = trap_R_RegisterShader( _token );
    }

    AddGFXLoop( groupname, x,y,w,h,r,g,b,a,shader );
}


/*
===================
Client Script
While Loop

Parses the while parameter from a string
format:
while <cvar> == <value> { ... }

== can also be !=,<=,>=,<,>
===================
*/
#define MAX_SCRIPT_LOOPS	1024
#define	MAX_SCRIPT_PROCESS	 128


// Navy Seals --
typedef struct {
    char		cvar[256];
    char		cmd[512]; // the command that has to be executed
    int			function;
    float		value;
    int			age;

    int			sleep;
    int			lasttime;

    qboolean	inuse;
} nsslLoopTable_t;

int numScriptProcess = 0;
nsslLoopTable_t		scriptLoopTable[MAX_SCRIPT_PROCESS];

/*
===================
Client Script
While Loop

Parses the while parameter from a string
format:
while <cvar> == <value/string> { ... } <sleep in ms> <timetolive>

== can also be !=,<=,>=,<,>
===================
*/
int GetFreeLoopSlot( void )
{
    int i = 0;

    for (i=0;i<MAX_SCRIPT_PROCESS;i++)
    {
        if ( !scriptLoopTable[i].inuse )
            return i;
    }
    return 0;
}
void AddScriptLoop( char cvar[256], char cmd[512], int function, float value, int sleep, int age )
{
    int num = GetFreeLoopSlot( );

    if ( nssl_debug.value == 1 )
        NSSL_Printf( "Added(%i): cvar %s cmd '%s' func %i value %f sleep %i age %i\n",num,cvar, cmd,function,value,sleep,age);

    strcpy( scriptLoopTable[num].cvar , cvar );
    strcpy( scriptLoopTable[num].cmd , cmd );
    scriptLoopTable[num].value = value;
    scriptLoopTable[num].inuse = qtrue;
    scriptLoopTable[num].function = function;
    scriptLoopTable[num].age = age;
    scriptLoopTable[num].sleep = sleep;

    // process it
    ClientScript_ProcessWhileLoop( num );
}
void ClientScript_ProcessWhileLoop( int num )
{
    int function = scriptLoopTable[num].function;
    char cvarResult[256];
    float i_value = scriptLoopTable[num].value;
    char cmd[512];
    int result = 0;

    ClientScript_GetValue( cvarResult, scriptLoopTable[num].cvar, qtrue );

    strcpy( cmd, scriptLoopTable[num].cmd );

    if ( cg.time < scriptLoopTable[num].lasttime )
        return;

    if ( scriptLoopTable[num].age <= 0 && scriptLoopTable[num].age != -1 )
    {
        if ( nssl_debug.value )
            NSSL_Printf( "Script Warning: Killed thread (endless-loop) age %i\n", scriptLoopTable[num].age);
        scriptLoopTable[num].inuse = qfalse;
        return;
    }

    scriptLoopTable[num].lasttime = cg.time + scriptLoopTable[num].sleep;

    if ( scriptLoopTable[num].age > 0 )
        scriptLoopTable[num].age--;


    // string comparision works only with same/notsame
    if ( function == OP_NOTSAME )
    {
        if ( atof(cvarResult) != scriptLoopTable[num].value )
            ClientScript_ExecText( cmd ); // execute it
        else
            scriptLoopTable[num].inuse = qfalse;
    }
    else if ( function == OP_SAME )
    {
        if ( atof ( cvarResult ) == scriptLoopTable[num].value )
            ClientScript_ExecText( cmd ); // execute it
        else
            scriptLoopTable[num].inuse = qfalse;
    }
    else
    {

        switch ( function )
        {
        case OP_SMALLERTHAN:
            if ( atof(cvarResult) < i_value )
                ClientScript_ExecText( cmd );
            else
                scriptLoopTable[num].inuse = qfalse;
            break;
        case OP_BIGGERTHAN:
            if ( atof(cvarResult) > i_value )
                ClientScript_ExecText( cmd );
            else
                scriptLoopTable[num].inuse = qfalse;
            break;
        case OP_SMALLERSAME:
            if ( atof(cvarResult) <= i_value  )
                ClientScript_ExecText( cmd );
            else
                scriptLoopTable[num].inuse = qfalse;
            break;
        case OP_BIGGERSAME:
            if ( atof(cvarResult) >= i_value )
                ClientScript_ExecText( cmd );
            else
                scriptLoopTable[num].inuse = qfalse;
            break;
        default:
            break;
        }
    }

}
void ClientScript_WhileLoop( void )
{
    const	char	*token; // a pointer
    char	cvar[256];	// the cvar i parse from the big string
    char	value[256];	// the value i parse from the big string
    char	cmd[512];	// the command that has to be executed
    int		function = 0;
    int		openSubs = 0; // amount of open { } there are
    int		age = MAX_SCRIPT_LOOPS;
    int		sleep = 500;

    qboolean	useElse = qtrue;


    /* Get next token: */
    token = ClientScript_NextToken();

    if (!token)
        return;

    strcpy( cvar, token );

    /* Get next token: */
    token = ClientScript_NextToken();

    // get operator
    function = ClientScript_GetOperator( token );

    token = ClientScript_NextToken();

    if (!token)
        return;

    // get the string we want to compare
    ClientScript_GetValue( value, token, qtrue );  // convert cvars without $, too
    ClientScript_GetCommand( cmd );
    strcat( cmd, ";");

    token = ClientScript_NextToken();

    if ( token )
    {
        char	temp[256];

        ClientScript_GetValue( temp, token, qtrue ); // convert cvars without $ too
        sleep = atoi( temp );

        token = ClientScript_NextToken();
        ClientScript_GetValue( temp, token, qtrue );
        age = atoi( temp );
    }

    // get the current CVARs variable into a buffer
    AddScriptLoop( cvar, cmd, function, atof( value ), sleep, age );
}

/*
===================
Client Script
Copy

Parses the copy function from a string.

format:
copy <cvar_in> <cvar_out>
===================
*/
void ClientScript_CopyVariable( void )
{
    const	char	*token; // a pointer
    char	cvarIn[256];	// the cvar i parse from the big string
    char	value[256];	// the value i parse from the big string
    char	cvarOut[256];	// the cvar buffer i use to compare to


    /* Get next token: */
    token = ClientScript_NextToken();
    if (!token)
    {
        NSSL_Printf( "Wrong syntax: missing variable\n", token );
        return;
    }
    // get the valueparameter
    strcpy( cvarIn, token );

    /* Get next token: */
    token = ClientScript_NextToken();
    if (!token)
    {
        NSSL_Printf( "Wrong syntax: missing variable\n", token );
        return;
    }
    // get the cvar
    strcpy( cvarOut, token );

    // get the value from parameter
    ClientScript_GetValue( value, cvarIn, qtrue );

    // and set it on our output cvar
    trap_Cvar_Set( cvarOut, value );

    if ( nssl_debug.value == 1 )
        NSSL_Printf( "Copy Cvar %s to %s\n", cvarIn, cvarOut );
}

/*
===================
Client Script
Set


format:
vset <cvar_in> { <string> }
===================
*/
void ClientScript_Set( void )
{
    const	char	*token; // a pointer
    char	cvar[256];	// the cvar i parse from the big string
    char	value[256];	// the value i parse from the big string


    /* Get next token: */
    token = ClientScript_NextToken();
    if (!token)
    {
        NSSL_Printf( "Wrong syntax: missing variable\n", token );
        return;
    }
    // get the valueparameter
    ClientScript_GetValue( cvar, token, qfalse ); // do not automatically pickup cvars

    // get the cvar
    ClientScript_GetCommand( value );

    // and set it on our output cvar
    trap_Cvar_Set( cvar, value );

    if ( nssl_debug.value == 1 )
        NSSL_Printf( "Set Cvar %s to %s\n", cvar, value );
}

/*
===================
Client Script
Switch

Parses the switch parameter from a string.
can be multiple <value/string>{} combinations

format:
switch <cvar> <value/string> { ... } else { ... }

== can also be !=,<=,>=,<,>
===================
*/
#define MAX_SWITCH_VALUES	32

void ClientScript_SwitchStatement( void )
{
    const	char	*token; // a pointer
    char	cvar[256];	// the cvar i parse from the big string
    char	value[MAX_SWITCH_VALUES][256]; // the value i parse from the big string
    int		numValue = 0;
    char	cmd[MAX_SWITCH_VALUES][256]; // the command that has to be executed
    char	cvarResult[256];	     // the cvar buffer i use to compare to
    int		openSubs = 0;	     // amount of open { } there are
    char	elseCmd[256];
    qboolean	useElse = qfalse;
    int		i;

    /* Get next token: */
    token = ClientScript_NextToken();
    if (!token)
    {
        NSSL_Printf( "Wrong syntax: missing variable\n", token );
        return;
    }

    // get the cvar
    strcpy( cvar, token );

    // parses <value> { <command }
    while ( token )
    {
        // first value
        token = ClientScript_NextToken();

        if ( !token )
            break;

        if ( !Q_stricmp( token, "else") )
        {
            useElse = qtrue;
            break;
        }

        ClientScript_GetValue( value[numValue], token, qtrue );
        ClientScript_GetCommand( cmd[numValue] );

        numValue++;
    }

    if ( useElse )
        ClientScript_GetCommand( elseCmd );

    // get the current CVARs variable into a buffer
    ClientScript_GetValue( cvarResult, cvar, qtrue );

    for ( i = 0; i < numValue; i++ )
    {
        // compare it
        if (Q_stricmp( cvarResult, value[i] ) != 0 &&
                atof( cvarResult ) != atof( value[i] ) )
            continue; // it is not the same

        // cvar is the same so execute the text
        ClientScript_ExecText( cmd[i] );
        return;
    }
    if ( useElse )
        ClientScript_ExecText( elseCmd );
}

qboolean ClientScript_LogicOperation( void )
{
    const char *token;
    char	value1[256];
    char	value2[256];
    int	result = 0;
    int numOperators = 1;
    int function;
    int openSub = 0;

logic_operation:
    // get first value
    token = ClientScript_NextToken();
    if ( !token ) {
        NSSL_Printf( "Wrong Syntax: Missing value/cvar/string\n" );
        return 0;
    }

    if ( !Q_stricmp( token, "(" ) ) {
        token = ClientScript_NextToken();
        if ( !token ) {
            NSSL_Printf( "Wrong Syntax: Missing value/cvar/string\n" );
            return 0;
        }

        // sounds nested...
        if ( !Q_stricmp( token, "(" ) )
        {
            ClientScript_PrevToken(); // do the nested fucntion
            result += ClientScript_LogicOperation();

            goto finish_operation; // drop to finish the operation, because the next symbol should be a && or a ||

            //                          || 0 == 0
            //  (           && 1 == 1 )
            //   ( a == 1 )
        }
    }

    ClientScript_GetValue( value1, token, qtrue );

    // get operator
    token = ClientScript_NextToken();
    if ( !token ) {
        NSSL_Printf( "Wrong Syntax: Missing operator\n" );
        return 0;
    }
    function = ClientScript_GetOperator( token );

    // get second value
    token = ClientScript_NextToken();
    if ( !token ) {
        NSSL_Printf( "Wrong Syntax: Missing value after: %s\n", ClientScript_PrevToken() );
        return 0;
    }

    // remove the ) if there is one at the end of the value
    ClientScript_GetValue( value2, token, qtrue );

    if ( nssl_debug.integer )
        NSSL_Printf("Logic Operation: %s with %i on %s\n", value1,function,value2);

    // operate on it
    result += ClientScript_Operate( value1, function, value2 );
finish_operation:
    token = ClientScript_NextToken();
    if ( !token ) {
        NSSL_Printf( "Wrong Syntax: Missing 'then' after %s\n", ClientScript_PrevToken() );
        return 0;
    }

    if ( !Q_stricmp( token, "&&" ) ||
            !Q_stricmp( token, "||" ) )
    {
        if ( !Q_stricmp( token, "&&" ) )
            numOperators++;
        //
        // check for a nested function
        //
        token = ClientScript_NextToken();

        if ( !token ) {
            NSSL_Printf( "Wrong Syntax: Missing TYPE after %s\n", ClientScript_PrevToken() );
            return 0;
        }

        if ( nssl_debug.integer )
            NSSL_Printf("Logic Operation: && or || token %s\n",  token );

        if ( !Q_stricmp( token, "(" ) )
        {
            ClientScript_PrevToken();
            result += ClientScript_LogicOperation();

            goto finish_operation;
        } else // go back one token so we can properly continue parsing
            ClientScript_PrevToken();

        goto logic_operation;
    }
    else if ( !Q_stricmp(token, ")" ) ) {
        return (result >= numOperators);
    }
    else {
        // go back one token so we can properly continue parsing
        ClientScript_PrevToken();
    }

    return (result >= numOperators);
}
/*
===================
Client Script
Parse If Clause

Parses the ifclause parameter from a string
format:
if <cvar> == <value/string> then { ... } else { ... }

== can also be !=,<=,>=,<,>
===================
*/
void ClientScript_ParseIfClause( void )
{
    const	char	*token; // a pointer
    char	cmd[512];	// the command that has to be executed
    char	elseCmd[512];	// the command that has to be executed when !=
    int result;
    // parse the logic operation
    //   ( $g_gametype == 1 )
if_clause:
    result = ClientScript_LogicOperation( );

    // look for the 'then' and go back if it was not there
    token = ClientScript_NextToken();
    if ( Q_stricmp( token, "then" ) != 0 )
        ClientScript_PrevToken();

    ClientScript_GetCommand( cmd );

    // if it's okay we don't even need to go further
    // just exec the text and leave
    if ( result )
    {
        ClientScript_ExecText( cmd );
        return;
    }

    // see if somebody wants an else if
    if ( result == 0 )
    {
        token = ClientScript_NextToken();
        if ( !token )
            return;

        if ( !Q_stricmp( token, "elseif" )  )
            goto if_clause; // handle if
        else
            ClientScript_PrevToken(); // go back cos somethng is there but no if
    }

    // see if there is an else statement given
    token = ClientScript_NextToken();
    if ( token && !Q_stricmp( token, "else" ) )
    {
        ClientScript_GetCommand( elseCmd );
        ClientScript_ExecText( elseCmd );
    }
}

void ClientScript_Exec( void )
{
    char cmd[512];

    ClientScript_GetCommand( cmd );

    if ( strlen ( cmd ) > 0 )
        ClientScript_ExecText( cmd );
}

/*
===================
Client Script

Execute a clientside scripting command
===================
*/
typedef struct {
    char	*param;
    void	(*function)(void);
    char	*usage;
} clientScriptParam_t;

static clientScriptParam_t	csParams[] = {
                                            { "modify", ClientScript_Modify,			"modify <IN> <$CVAR/VALUE>" },
                                            { "if",		ClientScript_ParseIfClause,		"if <CVAR> ==,!=,<=,>=,=|,& <$CVAR/VALUE/STRING> then { <CMD> } else { <CMD> }" },
                                            { "switch", ClientScript_SwitchStatement,	"switch <CVAR> <$CVAR/STRING/VALUE> { <CMD> } <STRING/VALUE> { <CMD> } else { <CMD> }" },
                                            { "while",	ClientScript_WhileLoop,			"while <CVAR> ==,!=,<>= <$CVAR/VALUE> then { <CMD> } <SLEEP> <AGE> *** AGE -1 = unlimited" },
                                            { "copy",	ClientScript_CopyVariable,		"copy <IN> <OUT>" },
                                            { "exec",	ClientScript_Exec,				"exec { <CMD> }" },
                                            { "addgfx", ClientScript_AddGfx,			"addgfx <GROUP> <X> <Y> <W> <H> <R> <G> <B> <A> (optional: <IMAGE>)// adds an item to a gfx group" },
                                            { "delgroup", ClientScript_DeleteGfxGroup,	"delgroup <GROUP> // deletes a gfx group" },
                                            { "delgfx", ClientScript_DeleteGfxGroup,	"delgfx <GROUP> // deletes a gfx group" },
                                            { "addstring", ClientScript_AddString,		"addstring <GROUP> { <text> } <X> <Y> <SCALE> <TYPE> <R> <G> <B> <A> // adds a sting item to a gfx group" },
                                            { "delstring", ClientScript_DeleteGfxGroup,	"delstring <GROUP> // deletes a gfx group" },
                                            { "vset",	ClientScript_Set ,				"vset <CVAR> { <STRING> } // sets a cvar" },

                                            // q3a hardcoded
                                            { "vstr",	0 ,	"vstr <FUNCTIONNAME/CVAR> *** Execute a function" },
                                            { "set",	0 ,	"set <CVAR> <STRING/VALUE> *** there is also a NSSL command called vset" },
                                            { "seta",	0 ,	"seta <CVAR> <STRING/VALUE> *** Cvar will be saved" },
                                            { "+alias",	0 ,	"+alias <CVAR>  *** useful when binding a key to a command" },
                                            // misc help
                                            { "help",	ClientScript_Help_f, "Type 'nssl help' to get a help, you can also type 'nssl help <cmd>' to get information about <cmd>." },
                                        };

void ClientScript_Help_f (  )
{
    const char *token = ClientScript_NextToken( );
    int i;

    if ( token )
    {
        for ( i = 0 ; i < sizeof( csParams ) / sizeof( csParams[0] ) ; i++ ) {
            if ( !Q_stricmp( token, csParams[i].param ) ) {
                NSSL_Printf( "Help for %s: %s\n", token, csParams[i].usage );
                return;
            }
        }
    }

    CG_Printf( "^2NSSL: Enviroment Help^7 ( "S_COLOR_RED""NSSL_VERSION""S_COLOR_WHITE" )\n" );
    CG_Printf( "-[^3cmd^7]-------[^3usage^7]----------------------------\n" );
    for ( i = 0 ; i < sizeof( csParams ) / sizeof( csParams[0] ) ; i++ ) {
        CG_Printf("[^3%s^7]\n  %s\n", csParams[i].param, csParams[i].usage );
    }
    CG_Printf("Insert a '$' infront of a cvar within the NSSL enviroment to evaluate a cvar into a value.\n" );
    CG_Printf("Type '*<type>' to insert clientinformation as a value.\n" );
    CG_Printf("Type '%<variable>,<match>' to parse a variable out of a string (.*)\n" );

    CG_Printf( "-[^3cmd^7]-------[^3usage^7]----------------------------\n" );
}

// we could also parse any other buffer if we use trap_Args to tokenize it
void CG_ClientScript_f ( void )
{
    const	char	*token;
    int		i;

    if ( nssl_init.integer < 1 ) {
        NSSL_Printf( "NSSL is disabled.\n");
        return;
    }

    cur_token = 0;
    token = ClientScript_NextToken( );

    for ( i = 0 ; i < sizeof( csParams ) / sizeof( csParams[0] ) ; i++ ) {
        if ( !Q_stricmp( token, csParams[i].param ) &&
                csParams[i].function != NULL ) {
            csParams[i].function();
            return;
        }
    }

    NSSL_Printf("Type '/NSSL help' for help\n");
}


// needed to be called once a frame
void ClientScript_Update( void )
{
    int i = 0;


    // update cvars
    trap_Cvar_Update( &nssl_debug );
    trap_Cvar_Update( &nssl_init );

    for (i=0;i<MAX_SCRIPT_PROCESS;i++)
    {
        if ( !scriptLoopTable[i].inuse )
            continue;

        ClientScript_ProcessWhileLoop( i );
    }

    for (i=0;i<MAX_SCRIPT_GFX;i++)
    {
        if ( !scriptGFXTable[i].inuse )
            continue;

        ClientScript_ProcessGFX( i );
    }
}
