#define NS_VERSION	"v0.1"
#define MAX_MENU_LINES	19 

// same as in g_seals.h
#define ARMOR_NONE		0
#define ARMOR_LIGHT		1
#define ARMOR_MEDIUM	2
#define ARMOR_HEAVY		3

typedef struct menuState_s {
    char	Line[65];
    float	*l_color;
} menuState_t;

extern menuState_t i_Menu[MAX_MENU_LINES]; 

// seals
void CG_LoadBarInit( void );
void CG_LoadingBarUpdate( int amount );
void CG_LoadingBarSetMax( int maximum );
void CG_CacheAllModels( );
void CG_RemoveAllFragments( void );

void CG_CreateBleeder( vec3_t origin , int damage, int playerNum );
void CG_BloodOnWallMark(vec3_t end, vec3_t normal, int damage, qboolean brain);
void CG_ReloadClipWarning();
void CG_LaunchParticle( vec3_t origin, vec3_t baseangle, vec3_t dir, qhandle_t hModel, int sound, int bouncefactor );
void CG_BleederTrail(localEntity_t *le);
qboolean	CG_ParseWeaponAnimationFile( const char *filename, int weapon_num );
void CG_AddPlayerWeapon2(refEntity_t *parent, playerState_t *ps, centity_t *cent);
void CG_WeaponAnimation(playerState_t *ps);
void NS_CG_LaunchFuncExplosive ( centity_t *cent );
void CG_Explosion( vec3_t origin, int c4explo );
void CG_SparkTrail( localEntity_t *le );
void CG_ResetMonsterEntity ( centity_t *cent );
void CG_ExplosionSparks ( vec3_t playerOrigin );
qboolean CG_ParseBriefingFile( char * mapstring );
void CG_BloodPool( vec3_t origin );
void CG_EnviromentParse ( char * text_p );
char *CG_GetCamoStringForType( int camoType );
void CG_InvenSelect( );
void CG_AddAtmosphericEffects( );
void CG_GetOriginFromTag( const refEntity_t *parent, qhandle_t parentModel, char *tagName, vec3_t out );
char *vtos( const vec3_t v );
// id
void CG_CalculateWeaponPosition(vec3_t origin, vec3_t angles);
void CG_Tracer( vec3_t source, vec3_t dest, float width, qhandle_t shader, vec4_t rgba );
void CG_LightParticles( vec3_t origin, vec4_t hcolor,  float limit );
// scripting
void ClientScript_Init( void );
void ClientScript_Update( void );

