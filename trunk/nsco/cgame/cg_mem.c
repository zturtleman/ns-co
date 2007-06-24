// Copyright (C) 2003-2003 team-mirage
//
//
// cg_mem.c - dynamic memory pool written for regexp
//

// every line of code is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"


#define POOLSIZE	(256 * 1024)

static char		cg_memoryPool[POOLSIZE];
static int		cg_allocPoint;

vmCvar_t	cg_debugAlloc;

void *CG_Alloc( int size ) {
    char	*p;

    if ( cg_debugAlloc.integer ) {
        CG_Printf( "CG_Alloc of 7%i bytes (%i left)\n", size, POOLSIZE - cg_allocPoint - ( ( size + 31 ) & ~31 ) );
    }

    if ( cg_allocPoint + size > POOLSIZE ) {
        CG_Error( "CG_Alloc: failed on allocation of %u bytes\n", size );
        return NULL;
    }

    p = &cg_memoryPool[cg_allocPoint];

    cg_allocPoint += ( size + 31 ) & ~31;

    return p;
}

void CG_InitMemory( void ) {
    cg_allocPoint = 0;
}

void CG_GameMem_f( void ) {
    CG_Printf( "CGame memory status: %i out of %i bytes allocated\n", cg_allocPoint, POOLSIZE );
}
