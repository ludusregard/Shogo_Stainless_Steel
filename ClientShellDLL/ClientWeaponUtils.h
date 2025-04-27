// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponUtils.h
//
// PURPOSE : Client-side firing helper functions
//
// CREATED : 11/2/98
//
// ----------------------------------------------------------------------- //

#include "basedefs_de.h"
#include "SurfaceTypes.h"

#ifndef __CLIENT_WEAPON_UTILS_H__
#define __CLIENT_WEAPON_UTILS_H__

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsMoveable()
//
//	PURPOSE:	Determine if the passed in object is moveable
//
// ----------------------------------------------------------------------- //

DBOOL IsMoveable(HLOCALOBJ hObj);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with a poly
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(HPOLY hPoly);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with an object
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(HLOCALOBJ hObj);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjListFilterFn()
//
//	PURPOSE:	Filter specific objects out of CastRay and/or 
//				IntersectSegment calls.
//
// ----------------------------------------------------------------------- //

DBOOL ObjListFilterFn(HLOCALOBJ hObj, void *pUserData);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecificObjectFilterFn()
//
//	PURPOSE:	Filter a specific object out of CastRay and/or 
//				IntersectSegment calls.
//
// ----------------------------------------------------------------------- //

DBOOL SpecificObjectFilterFn(HLOCALOBJ hObj, void *pUserData);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackerLiquidFilterFn()
//
//	PURPOSE:	Filter the attacker out of CastRay and/or 
//				IntersectSegment calls (so you don't shot yourself).
//				However, we want to ignore liquid as well...
//
// ----------------------------------------------------------------------- //

DBOOL AttackerLiquidFilterFn(HLOCALOBJ hObj, void *pUserData);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AddLocalImpactFX
//
//	PURPOSE:	Add a weapon impact special fx
//
// ----------------------------------------------------------------------- //

void AddLocalImpactFX(HLOCALOBJ hObj, DVector & vFirePos, DVector & vImpactPoint, 
					  DVector & vNormal, SurfaceType eType, DVector & vPath, 
					  DBYTE nWeaponId, DBYTE nIgnoreFX);


#endif // __CLIENT_WEAPON_UTILS_H__