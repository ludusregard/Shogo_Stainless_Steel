// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileFX.cpp
//
// PURPOSE : Weapon special FX - Implementation
//
// CREATED : 7/6/98
//
// ----------------------------------------------------------------------- //

#include "ProjectileFX.h"
#include "RiotClientShell.h"
#include "ParticleTrailFX.h"
#include "SFXMsgIds.h"
#include "ClientUtilities.h"
#include "physics_lt.h"
#include "ClientWeaponUtils.h"
#include "ParticleTrailFX.h"
#include "CMoveMgr.h"

extern CRiotClientShell* g_pRiotClientShell;
extern DBYTE g_nRandomWeaponSeed;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::Init
//
//	PURPOSE:	Init the weapon fx
//
// ----------------------------------------------------------------------- //

DBOOL CProjectileFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	PROJECTILECREATESTRUCT* pCS = (PROJECTILECREATESTRUCT*)psfxCreateStruct;

	DBYTE nWeapon	= (pCS->nWeaponId & ~MODEL_FLAG_MASK);
	DBYTE nFlag		= (pCS->nWeaponId & MODEL_FLAG_MASK);

	m_nWeaponId		= (RiotWeaponId)nWeapon;
	m_eSize			= MS_NORMAL;

	m_nShooterId	= pCS->nShooterId;
	m_bLocal		= pCS->bLocal;

	if (nFlag & MODEL_SMALL_FLAG)
	{
		m_eSize = MS_SMALL;
	}
	else if (nFlag & MODEL_LARGE_FLAG)
	{
		m_eSize = MS_LARGE;
	}

	m_nFX = GetProjectileFX(m_nWeaponId);

	return (m_nFX > 0);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

DBOOL CProjectileFX::CreateObject(CClientDE* pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return DFALSE;

	CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
	if (!pSettings) return DFALSE;

	DBYTE nDetailLevel = pSettings->SpecialFXSetting();

#define HIDE_SERVER_PROJECTILE_FX
#ifdef HIDE_SERVER_PROJECTILE_FX

	// Only do these checks if this is a multiplayer game...

	if (g_pRiotClientShell->IsMultiplayerGame())
	{
		// Don't do the projectile effect twice (for the same client)...

		DDWORD dwId;
		m_pClientDE->GetLocalClientID(&dwId);
		if ((DBYTE)dwId == m_nShooterId && !m_bLocal) 
		{
			// Of course there are always special cases...

			if (m_nWeaponId != GUN_KATOGRENADE_ID && m_nWeaponId != GUN_SPIDER_ID) 
			{
				return DFALSE;
			}
		}
	}
#endif

	DVector vPos;
	DRotation rRot;
	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
	m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);

	if (nDetailLevel != RS_LOW)
	{
		if (m_nFX & PFX_SMOKETRAIL)
		{
			CreateSmokeTrail(vPos, rRot);
		}
		
		if (m_nFX & PFX_LIGHT)
		{
			CreateLight(vPos, rRot);
		}
	}

	if (m_nFX & PFX_FLARE)
	{
		CreateFlare(vPos, rRot);
	}
	
	if (m_nFX & PFX_FLYSOUND)
	{
		CreateFlyingSound(vPos, rRot);	
	}

	CreateProjectile(vPos, rRot);


	// Do client-side projectiles in multiplayer games...

	if (g_pRiotClientShell->IsMultiplayerGame())
	{
		// Set the velocity of the "server" object if it is really just a local
		// object...

		if (m_bLocal)
		{
			VEC_COPY(m_vFirePos, vPos);

			m_fStartTime = m_pClientDE->GetTime();

			DVector vVel, vU, vR, vF;
			m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

			VEC_COPY(m_vPath, vF);

			// Special case of adjusting TOW rocket speed...

			DFLOAT fVel = GetWeaponVelocity(m_nWeaponId);
			if (m_nWeaponId == GUN_TOW_ID)
			{
				DFLOAT fMultiplier = 1.0f;
				if (m_pClientDE->GetSConValueFloat("MissileSpeed", fMultiplier) != DE_NOTFOUND)
				{
					fVel *= fMultiplier;
				}
			}

			VEC_MULSCALAR(vVel, vF, fVel);
			m_pClientDE->Physics()->SetVelocity(m_hServerObject, &vVel);
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::Update
//
//	PURPOSE:	Update the weapon fx
//
// ----------------------------------------------------------------------- //

DBOOL CProjectileFX::Update()
{
	if (!m_pClientDE || !m_hServerObject) return DFALSE;


	if (g_pRiotClientShell->IsMultiplayerGame())
	{
		// If this is a local fx, we control the position of the "server object"...

		if (m_bLocal)
		{
			if (!MoveServerObj())
			{
				Detonate(DNULL);
			
				// Remove the "server" object...

				m_pClientDE->DeleteObject(m_hServerObject);
				m_hServerObject = DNULL;
				m_bWantRemove = DTRUE;
			}
		}
	}


	if (m_bWantRemove)
	{
		RemoveFX();
		return DFALSE;
	}


	// Update fx positions...

	DRotation rRot;
	DVector vPos;
	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
	m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);

	if (m_pSmokeTrail)
	{
		m_pSmokeTrail->Update();
	}

	if (m_hFlare)
	{
		m_pClientDE->SetObjectPos(m_hFlare, &vPos);
		m_pClientDE->SetObjectRotation(m_hFlare, &rRot);
	}

	if (m_hLight)
	{
		m_pClientDE->SetObjectPos(m_hLight, &vPos);
		m_pClientDE->SetObjectRotation(m_hLight, &rRot);
	}

	if (m_hProjectile)
	{
		m_pClientDE->SetObjectPos(m_hProjectile, &vPos);
		m_pClientDE->SetObjectRotation(m_hProjectile, &rRot);
	}

	if (m_hFlyingSound)
	{
		m_pClientDE->SetSoundPosition(m_hFlyingSound, &vPos);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::MoveServerObj
//
//	PURPOSE:	Update mover
//
// ----------------------------------------------------------------------- //

DBOOL CProjectileFX::MoveServerObj()
{	
	if (!m_pClientDE || !m_bLocal || !m_hServerObject) return DFALSE;

	CPhysicsLT* pPhysicsLT = m_pClientDE->Physics();
	if (!pPhysicsLT) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	// If we didn't hit anything we're done...

	if (fTime >= (m_fStartTime + GetWeaponLifeTime(m_nWeaponId))) 
	{
		return DFALSE;
	} 
	
	
	DFLOAT fFrameTime = m_pClientDE->GetFrameTime();

	// Zero out the acceleration to start with.
	
	DVector zeroVec;
	zeroVec.Init();
	pPhysicsLT->SetAcceleration(m_hServerObject, &zeroVec);


	// Do special bullgut movement...

	if (m_nWeaponId == GUN_BULLGUT_ID)
	{
		DVector vTemp, vU, vR, vF;
		
		DVector vVel;
		pPhysicsLT->GetVelocity(m_hServerObject, &vVel);

		DRotation rRot;
		m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		VEC_NORM(vU);
		VEC_NORM(vF);
		
		if (m_bFirstSnake)
		{
			m_bFirstSnake = DFALSE;
			m_fSnakeDir   = (g_nRandomWeaponSeed & 1) ? 1.0f : -1.0f;
		}
		else
		{
			// Subtract velocity off current up vector...

			VEC_MULSCALAR(vTemp, vU, m_fSnakeUpVel);
			VEC_SUB(vVel, vVel, vTemp);
		}

		m_pClientDE->RotateAroundAxis(&rRot, &vF, m_fSnakeDir * 10.0f * fFrameTime);
		m_pClientDE->SetObjectRotation(m_hServerObject, &rRot);
		m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		VEC_NORM(vU);

		// Add velocity to new up vector...

		m_fSnakeUpVel = GetWeaponVelocity(m_nWeaponId) * 0.67f * fFrameTime;

		VEC_MULSCALAR(vTemp, vU, m_fSnakeUpVel);
		VEC_ADD(vVel, vVel, vU);

		pPhysicsLT->SetVelocity(m_hServerObject, &vVel);
	}


	DBOOL bRet = DTRUE;

	MoveInfo info;

	info.m_hObject  = m_hServerObject;
	info.m_dt		= fFrameTime;
	pPhysicsLT->UpdateMovement(&info);

	if (info.m_Offset.MagSqr() > 0.01f)
	{
		DVector vDiff, vNewPos, vCurPos;
		m_pClientDE->GetObjectPos(m_hServerObject, &vCurPos);
		vNewPos = vCurPos + info.m_Offset;
		pPhysicsLT->MoveObject(m_hServerObject, &vNewPos, 0);

		vDiff = vCurPos - vNewPos;
		if (vDiff.MagSqr() < 5.0f)
		{
			bRet = DFALSE;
		}
	}
	else
	{
		bRet = DFALSE;
	}
	
	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateProjectile
//
//	PURPOSE:	Create the projectile
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateProjectile(DVector & vPos, DRotation & rRot)
{
	if (!g_pRiotClientShell || !m_hServerObject) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	DVector vScale, vDims;
	VEC_SET(vScale, 1.0f, 1.0f, 1.0f);
	VEC_SET(vDims, 1.0f, 1.0f, 1.0f);
	char* pSpriteFilename = DNULL;
	char* pModelFilename = DNULL;	
	char* pModelSkin = DNULL;	

	// Create a model or sprite depending on the projectile...

	switch (m_nWeaponId)
	{
		case GUN_TOW_ID: 
		case GUN_BULLGUT_ID: 
		{
			VEC_SET(vScale, 15.0f, 15.0f, 15.0f);
			pModelFilename = "Models\\PV_Weapons\\Bullgut_Projectile.abc";
			pModelSkin = "Skins\\Weapons\\Bullgut_Projectile_a.dtx";
		}
		break;

		case GUN_ENERGYGRENADE_ID: 
		{
			pSpriteFilename = "Sprites\\grenade1.spr";
		}
		break;

		case GUN_PULSERIFLE_ID: 
		{
			VEC_SET(vScale, 0.15f, 0.15f, 1.0f);
			pSpriteFilename = "Sprites\\PulseRifle.spr";
		}
		break;

		default : 
			return;
		break;
	}


	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_Flags = FLAG_VISIBLE;  
	VEC_COPY(createStruct.m_Pos, vPos);
	ROT_COPY(createStruct.m_Rotation, rRot);

	// Create a sprite if appropriate...

	if (pSpriteFilename)
	{
		createStruct.m_ObjectType = OT_SPRITE;
		SAFE_STRCPY(createStruct.m_Filename, pSpriteFilename);

		m_hProjectile = m_pClientDE->CreateObject(&createStruct);
		if (!m_hProjectile) return;

		m_pClientDE->SetObjectColor(m_hProjectile, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		createStruct.m_ObjectType = OT_MODEL;
		SAFE_STRCPY(createStruct.m_Filename, pModelFilename);
		SAFE_STRCPY(createStruct.m_SkinName, pModelSkin);

		m_hProjectile = m_pClientDE->CreateObject(&createStruct);
		if (!m_hProjectile) return;

		if (m_eSize == MS_SMALL)
		{
			VEC_MULSCALAR(vDims, vDims, 0.2f);
			VEC_SET(vScale, .333f, .333f, .333f);
		}
	}


	if (m_hProjectile)
	{
		m_pClientDE->Physics()->SetObjectDims(m_hProjectile, &vDims, 0);
		m_pClientDE->SetObjectScale(m_hProjectile, &vScale);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateSmokeTrail
//
//	PURPOSE:	Create a smoke trail special fx
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateSmokeTrail(DVector & vPos, DRotation & rRot)
{
	if (!g_pRiotClientShell || !m_hServerObject) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	PTCREATESTRUCT pt;
	
	pt.hServerObj = m_hServerObject;
	pt.nType	  = PT_SMOKE;
	pt.bSmall	  = (m_eSize == MS_SMALL ? DTRUE : DFALSE);

	// CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_PARTICLETRAIL_ID, &pt);

	m_pSmokeTrail = new CParticleTrailFX();
	if (!m_pSmokeTrail) return;

	m_pSmokeTrail->Init(&pt);
	m_pSmokeTrail->CreateObject(m_pClientDE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateFlare
//
//	PURPOSE:	Create a flare special fx
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateFlare(DVector & vPos, DRotation & rRot)
{
	if (!m_pClientDE || !m_hServerObject) return;

	DVector vScale;
	VEC_SET(vScale, 1.0f, 1.0f, 1.0f);
	char* pFlareFilename = "Sprites\\glow.spr";

	switch (m_nWeaponId)
	{
		case GUN_TOW_ID: 
		case GUN_BULLGUT_ID: 
		{
			VEC_SET(vScale, 0.25f, 0.25f, 1.0f);
			pFlareFilename = "Sprites\\BullgutFlare.spr";
		}
		break;

		default : break;
	}

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_SPRITE;
	SAFE_STRCPY(createStruct.m_Filename, pFlareFilename);
	createStruct.m_Flags = FLAG_VISIBLE;  
	VEC_COPY(createStruct.m_Pos, vPos);
	ROT_COPY(createStruct.m_Rotation, rRot);

	m_hFlare = m_pClientDE->CreateObject(&createStruct);
	if (!m_hFlare) return;

	m_pClientDE->SetObjectScale(m_hFlare, &vScale);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateLight
//
//	PURPOSE:	Create a light special fx
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateLight(DVector & vPos, DRotation & rRot)
{
	if (!m_pClientDE || !m_hServerObject) return;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_LIGHT;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_DONTLIGHTBACKFACING;
	VEC_COPY(createStruct.m_Pos, vPos);

	m_hLight = m_pClientDE->CreateObject(&createStruct);
	if (!m_hLight) return; 

	DVector vColor;
	VEC_SET(vColor, 0.98f, 0.98f, 0.75f);

	switch (m_nWeaponId)
	{
		case GUN_PULSERIFLE_ID :
			VEC_SET(vColor, 0.65f, 0.98f, 0.98f);
		break;
		case GUN_ENERGYGRENADE_ID :
			VEC_SET(vColor, 0.65f, 0.98f, 0.98f);
		break;
		case GUN_KATOGRENADE_ID :
			VEC_SET(vColor, 0.98f, 0.98f, 0.1f);
		break;
		default : break;
	}

	m_pClientDE->SetLightColor(m_hLight, vColor.x, vColor.y, vColor.z);
	m_pClientDE->SetLightRadius(m_hLight, 100.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateFlyingSound
//
//	PURPOSE:	Create the flying sound
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateFlyingSound(DVector & vPos, DRotation & rRot)
{
	if (!m_pClientDE || m_hFlyingSound) return;

	char* pFlySound = GetWeaponFlyingSound(m_nWeaponId);
	if (pFlySound)
	{
		DFLOAT fRadius = 800.0f;
		fRadius *= ((m_eSize == MS_SMALL) ? 0.2f : (m_eSize == MS_LARGE ? 5.0f : 1.0f));

		m_hFlyingSound = PlaySoundFromPos(&vPos, pFlySound, fRadius, SOUNDPRIORITY_MISC_LOW, DTRUE, DTRUE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::RemoveFX
//
//	PURPOSE:	Remove all fx
//
// ----------------------------------------------------------------------- //

void CProjectileFX::RemoveFX()
{
	if (!g_pRiotClientShell || !m_pClientDE) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	if (m_pSmokeTrail)
	{
		delete m_pSmokeTrail;
		m_pSmokeTrail = DNULL;
	}
		
	if (m_hFlare)
	{
		m_pClientDE->DeleteObject(m_hFlare);
		m_hFlare = DNULL;
	}

	if (m_hLight)
	{
		m_pClientDE->DeleteObject(m_hLight);
		m_hLight = DNULL;
	}

	if (m_hProjectile)
	{
		m_pClientDE->DeleteObject(m_hProjectile);
		m_hProjectile = DNULL;
	}

	if (m_hFlyingSound)
	{
		m_pClientDE->KillSound(m_hFlyingSound);
		m_hFlyingSound = DNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CProjectileFX::HandleTouch(CollisionInfo *pInfo, float forceMag)
{
	if (!m_pClientDE || !pInfo || !pInfo->m_hObject || !g_pRiotClientShell) return;

	 // Let it get out of our bounding box...

	CMoveMgr* pMoveMgr = g_pRiotClientShell->GetMoveMgr();
	if (pMoveMgr)
	{
		// Don't colide with the move mgr object...

		HLOCALOBJ hMoveObj = pMoveMgr->GetObject();
		if (pInfo->m_hObject == hMoveObj) return;

		// Don't colide with the player object...

		HLOCALOBJ hPlayerObj = m_pClientDE->GetClientObject();
		if (pInfo->m_hObject == hPlayerObj) return;
	}


	// See if we want to impact on this object...

	DDWORD dwUsrFlags;
	m_pClientDE->GetObjectUserFlags(pInfo->m_hObject, &dwUsrFlags);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) return;

	HLOCALOBJ hWorld;
	m_pClientDE->Physics()->GetWorldObject(&hWorld);

	// Don't impact on non-solid objects...

	DDWORD dwFlags = m_pClientDE->GetObjectFlags(pInfo->m_hObject);
	if (pInfo->m_hObject != hWorld && !(dwFlags & FLAG_SOLID)) return;


	// Don't hit projectiles we (i.e., this client) fired...

	//if (dwUsrFlags & USRFLG_CLIENT_PROJECTILE)
	//{
	//		return;
	//}


	// See if we hit the sky...

	if (pInfo->m_hObject == hWorld)
	{
		SurfaceType eType = GetSurfaceType(pInfo->m_hPoly);

		if (eType == ST_SKY)
		{
			m_bWantRemove = DTRUE;
			return;
		}
	}


	Detonate(pInfo);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::Detonate()
//
//	PURPOSE:	Handle blowing up the projectile
//
// ----------------------------------------------------------------------- //

void CProjectileFX::Detonate(CollisionInfo* pInfo)
{
	if (!m_pClientDE || m_bDetonated) return;

	m_bDetonated = DTRUE;

	SurfaceType eType = ST_UNKNOWN;

	DVector vPos;
	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);

	// Determine the normal of the surface we are impacting on...

	DVector vNormal;
	VEC_SET(vNormal, 0.0f, 1.0f, 0.0f);

	if (pInfo)
	{
		if (pInfo->m_hObject)
		{
			eType = GetSurfaceType(pInfo->m_hObject);
		}
		else if (pInfo->m_hPoly)
		{
			eType = GetSurfaceType(pInfo->m_hPoly);

			VEC_COPY(vNormal, pInfo->m_Plane.m_Normal);

			DRotation rRot;
			m_pClientDE->AlignRotation(&rRot, &vNormal, DNULL);
			m_pClientDE->SetObjectRotation(m_hServerObject, &rRot);

			// Calculate where we really hit the plane...

			DVector vVel, vP0, vP1;
			m_pClientDE->Physics()->GetVelocity(m_hServerObject, &vVel);

			VEC_COPY(vP1, vPos);
			VEC_MULSCALAR(vVel, vVel, m_pClientDE->GetFrameTime());
			VEC_SUB(vP0, vP1, vVel);

			DFLOAT fDot1 = DIST_TO_PLANE(vP0, pInfo->m_Plane);
			DFLOAT fDot2 = DIST_TO_PLANE(vP1, pInfo->m_Plane);

			if (fDot1 < 0.0f && fDot2 < 0.0f || fDot1 > 0.0f && fDot2 > 0.0f)
			{
				VEC_COPY(vPos, vP1);
			}
			else
			{
				DFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
				VEC_LERP(vPos, vP0, vP1, fPercent);
			}
		}
	}
	else
	{
		// Since pInfo was null, this means the projectile's lifetime was up,
		// so we just blow-up in the air.

		eType = ST_AIR; 
	}


	HOBJECT hObj = !!pInfo ? pInfo->m_hObject : DNULL;
	::AddLocalImpactFX(hObj, m_vFirePos, vPos, vNormal, eType, m_vPath, m_nWeaponId, 0);

	m_bWantRemove = DTRUE;
}

