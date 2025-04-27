// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleExplosionFX.cpp
//
// PURPOSE : Particle Explosion - Implementation
//
// CREATED : 5/22/98
//
// ----------------------------------------------------------------------- //

#include "ParticleExplosionFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ContainerCodes.h"
#include "ClientServerShared.h"
#include "WeaponFXTypes.h"
#include "DebrisTypes.h"

extern DVector g_vWorldWindVel;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleExplosionFX::Init
//
//	PURPOSE:	Init the Particle trail segment
//
// ----------------------------------------------------------------------- //

DBOOL CParticleExplosionFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return DFALSE;

	PESCREATESTRUCT* pPE = (PESCREATESTRUCT*)psfxCreateStruct;
	ROT_COPY(m_rSurfaceRot, pPE->rSurfaceRot);
	VEC_COPY(m_vPos, pPE->vPos);
	VEC_COPY(m_vColor1, pPE->vColor1);
	VEC_COPY(m_vColor2, pPE->vColor2);
	VEC_COPY(m_vMinVel, pPE->vMinVel);
	VEC_COPY(m_vMaxVel, pPE->vMaxVel);
	VEC_COPY(m_vMinDriftOffset, pPE->vMinDriftOffset);
	VEC_COPY(m_vMaxDriftOffset, pPE->vMaxDriftOffset);
	m_bSmall			= pPE->bSmall;
	m_fLifeTime			= pPE->fLifeTime;
	m_fFadeTime			= pPE->fFadeTime;
	m_fOffsetTime		= pPE->fOffsetTime;
	m_fRadius			= pPE->fRadius;
	m_fGravity			= pPE->fGravity;
	m_nNumPerPuff		= pPE->nNumPerPuff;
	m_nNumEmmitters		= (pPE->nNumEmmitters > MAX_EMMITTERS ? MAX_EMMITTERS : pPE->nNumEmmitters);
	m_nEmmitterFlags	= pPE->nEmmitterFlags;
	m_pTextureName		= pPE->pFilename;
	m_bCreateDebris		= pPE->bCreateDebris;
	m_bRotateDebris		= pPE->bRotateDebris;
	m_nSurfaceType		= pPE->nSurfaceType;
	m_bIgnoreWind		= pPE->bIgnoreWind;
	m_nNumSteps			= pPE->nNumSteps;

	VEC_SET(m_vColorRange, m_vColor2.x - m_vColor1.x, 
						   m_vColor2.y - m_vColor1.y,
						   m_vColor2.z - m_vColor1.z);

	if (m_vColorRange.x < 0.0f) m_vColorRange.x = 0.0f;
	if (m_vColorRange.y < 0.0f) m_vColorRange.y = 0.0f;
	if (m_vColorRange.z < 0.0f) m_vColorRange.z = 0.0f;


	if (m_bRotateDebris)
	{
		m_fPitchVel = GetRandom(-MATH_CIRCLE, MATH_CIRCLE);
		m_fYawVel	= GetRandom(-MATH_CIRCLE, MATH_CIRCLE);
	}

	if (m_bSmall)
	{
		m_fRadius /= 2.0f;
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CParticleExplosionFX::CreateObject(CClientDE *pClientDE)
{
	DBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	// Initialize the emmitters velocity ranges based on our rotation...

	DVector vVelMin, vVelMax, vTemp, vU, vR, vF;
	VEC_SET(vVelMin, 1.0f, 1.0f, 1.0f);
	VEC_SET(vVelMax, 1.0f, 1.0f, 1.0f);

	m_pClientDE->GetRotationVectors(&m_rSurfaceRot, &vU, &vR, &vF);

	if (vF.y <= -0.95f || vF.y >= 0.95f)
	{
		vF.y = vF.y > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 1.0f, 0.0f, 0.0f);
		VEC_SET(vU, 0.0f, 0.0f, 1.0f);
	}
	else if (vF.x <= -0.95f || vF.x >= 0.95f)
	{
		vF.x = vF.x > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 0.0f, 1.0f, 0.0f);
		VEC_SET(vU, 0.0f, 0.0f, 1.0f);
	}
	else if (vF.z <= -0.95f || vF.z >= 0.95f)
	{
		vF.z = vF.z > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 1.0f, 0.0f, 0.0f);
		VEC_SET(vU, 0.0f, 1.0f, 0.0f);
	}

	VEC_MULSCALAR(vVelMin, vF, m_vMinVel.y); 
	VEC_MULSCALAR(vVelMax, vF, m_vMaxVel.y); 

	VEC_MULSCALAR(vTemp, vR, m_vMinVel.x);
	VEC_ADD(vVelMin, vVelMin, vTemp);

	VEC_MULSCALAR(vTemp, vR, m_vMaxVel.x);
	VEC_ADD(vVelMax, vVelMax, vTemp);

	VEC_MULSCALAR(vTemp, vU, m_vMinVel.z);
	VEC_ADD(vVelMin, vVelMin, vTemp);

	VEC_MULSCALAR(vTemp, vU, m_vMaxVel.z);
	VEC_ADD(vVelMax, vVelMax, vTemp);


	// Initialize our emmitters...

	DVector vStartVel;
	for (int i=0; i < m_nNumEmmitters; i++)
	{
		if (m_bCreateDebris) 
		{
			m_hDebris[i] = CreateDebris();
		}

		m_ActiveEmmitters[i] = DTRUE;
		m_BounceCount[i] = 2;

		VEC_SET(vStartVel, GetRandom(vVelMin.x, vVelMax.x), 
						   GetRandom(vVelMin.y, vVelMax.y), 
						   GetRandom(vVelMin.z, vVelMax.z));

		InitMovingObject(&(m_Emmitters[i]), &m_vPos, &vStartVel);
		m_Emmitters[i].m_PhysicsFlags |= m_nEmmitterFlags;
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleExplosionFX::Update
//
//	PURPOSE:	Update the Particle trail (add Particle)
//
// ----------------------------------------------------------------------- //

DBOOL CParticleExplosionFX::Update()
{
	if (!m_hObject || !m_pClientDE) return DFALSE;

	if (!CBaseParticleSystemFX::Update()) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
		m_bFirstUpdate = DFALSE;
		m_fStartTime   = fTime;
		m_fLastTime	   = fTime;
	}


	// Check to see if we should start fading the system...

	if (fTime > m_fStartTime + m_fFadeTime)
	{
		DFLOAT fEndTime = m_fStartTime + m_fLifeTime;
		if (fTime > fEndTime)
		{
			return DFALSE;
		}

		DFLOAT fScale = (fEndTime - fTime) / (m_fLifeTime - m_fFadeTime);

		DFLOAT r, g, b, a;
		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
		m_pClientDE->SetObjectColor(m_hObject, r, g, b, fScale);
	}


	// See if it is time to create a new Particle puff...

	if (fTime >= m_fLastTime + m_fOffsetTime)
	{
		// Loop over our list of emmitters, creating new particles...

		for (int i=0; i < m_nNumEmmitters; i++)
		{
			if (m_ActiveEmmitters[i])
			{
				AddParticles(&m_Emmitters[i]);
			}
		}

		m_fLastTime = fTime;
	}


	// Loop over our list of emmitters, updating the position of each

	for (int i=0; i < m_nNumEmmitters; i++)
	{
		if (m_ActiveEmmitters[i])
		{
			DBOOL bBounced = DFALSE;
			if (bBounced = UpdateEmmitter(&m_Emmitters[i]))
			{
				if (!(m_Emmitters[i].m_PhysicsFlags & MO_LIQUID) && (m_hDebris[i]))
				{
					char* pSound = GetDebrisBounceSound(DBT_STONE_BIG);

					// Play appropriate sound...

					PlaySoundFromPos(&m_Emmitters[i].m_Pos, pSound, 300.0f,
									 SOUNDPRIORITY_MISC_LOW);
				}

				m_BounceCount[i]--;

				if (m_BounceCount[i] <= 0)
				{
					m_Emmitters[i].m_PhysicsFlags |= MO_RESTING;
				}
			}

			if (m_Emmitters[i].m_PhysicsFlags & MO_RESTING)
			{
				m_ActiveEmmitters[i] = DFALSE;
				if (m_hDebris[i])
				{
					m_pClientDE->DeleteObject(m_hDebris[i]);
					m_hDebris[i] = DNULL;
				}
			}
			else if (m_hDebris[i])
			{
				m_pClientDE->SetObjectPos(m_hDebris[i], &(m_Emmitters[i].m_Pos));

				if (m_bRotateDebris)
				{
					if (bBounced)
					{
						// Adjust due to the bounce...

						m_fPitchVel = GetRandom(-MATH_CIRCLE, MATH_CIRCLE);
						m_fYawVel	= GetRandom(-MATH_CIRCLE, MATH_CIRCLE);
					}

					if (m_fPitchVel != 0 || m_fYawVel != 0)
					{
						DFLOAT fDeltaTime = m_pClientDE->GetFrameTime();

						m_fPitch += m_fPitchVel * fDeltaTime;
						m_fYaw   += m_fYawVel * fDeltaTime;

						DRotation rRot;
						m_pClientDE->SetupEuler(&rRot, m_fPitch, m_fYaw, 0.0f);
						m_pClientDE->SetObjectRotation(m_hDebris[i], &rRot);	
					}
				}
			}
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleExplosionFX::UpdateEmmitter
//
//	PURPOSE:	Update emmitter position
//
// ----------------------------------------------------------------------- //

DBOOL CParticleExplosionFX::UpdateEmmitter(MovingObject* pObject)
{	
	if (!m_pClientDE || !pObject || pObject->m_PhysicsFlags & MO_RESTING) return DFALSE;

	DBOOL bRet = DFALSE;

	DVector vNewPos;
	if (UpdateMovingObject(DNULL, pObject, &vNewPos))
	{
		ClientIntersectInfo info;
		bRet = BounceMovingObject(DNULL, pObject, &vNewPos, &info);

		VEC_COPY(pObject->m_LastPos, pObject->m_Pos);
		VEC_COPY(pObject->m_Pos, vNewPos);

		if (m_pClientDE->GetPointStatus(&vNewPos) == DE_OUTSIDE)
		{
			pObject->m_PhysicsFlags |= MO_RESTING;
			VEC_COPY(pObject->m_Pos, pObject->m_LastPos);
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleExplosionFX::AddParticles
//
//	PURPOSE:	Add particles to system
//
// ----------------------------------------------------------------------- //

void CParticleExplosionFX::AddParticles(MovingObject* pObject)
{	
	if (!m_hObject || !m_pClientDE || !pObject || pObject->m_PhysicsFlags & MO_RESTING) return;

	DFLOAT fTime = m_pClientDE->GetTime();

	DVector vCurPos, vLastPos, vPos, vDelta, vTemp, vDriftVel, vColor;

	VEC_COPY(vCurPos, pObject->m_Pos);
	VEC_COPY(vLastPos, pObject->m_LastPos);

	// Calculate Particle puff positions...

	// Current position is relative to the particle system's postion (i.e., 
	// each puff of Particle is some distance away from the particle system's 
	// position)...

	VEC_SUB(vCurPos, vCurPos, m_vPos);
	VEC_SUB(vLastPos, vLastPos, m_vPos);


	// How long has it been since the last Particle puff?

	DFLOAT fTimeOffset = fTime - m_fLastTime;

	
	// Fill the distance between the last projectile position, and it's 
	// current position with Particle puffs...

	VEC_SUB(vTemp, vCurPos, vLastPos);
	VEC_MULSCALAR(vDelta, vTemp, 1.0f/float(m_nNumSteps));

	VEC_COPY(vPos, vLastPos);

	DFLOAT fCurLifeTime    = 10.0f;
	DFLOAT fLifeTimeOffset = fTimeOffset / float(m_nNumSteps);

	DFLOAT fOffset = 0.5f;

	if (m_bSmall)
	{
		fOffset /= 2.0f;
	}

	for (int i=0; i < m_nNumSteps; i++)
	{
		// Build the individual Particle puffs...

		for (int j=0; j < m_nNumPerPuff; j++)
		{
			VEC_COPY(vTemp, vPos);

			VEC_SET(vDriftVel, GetRandom(m_vMinDriftOffset.x, m_vMaxDriftOffset.x), 
							   GetRandom(m_vMinDriftOffset.y, m_vMaxDriftOffset.y), 
							   GetRandom(m_vMinDriftOffset.z, m_vMaxDriftOffset.z));

			if (!m_bIgnoreWind)
			{
				vDriftVel.x += g_vWorldWindVel.x;
				vDriftVel.y += g_vWorldWindVel.y;
				vDriftVel.z += g_vWorldWindVel.z;
			}

			vTemp.x += GetRandom(-fOffset, fOffset);
			vTemp.y += GetRandom(-fOffset, fOffset);
			vTemp.z += GetRandom(-fOffset, fOffset);

			GetRandomColorInRange(vColor);

			m_pClientDE->AddParticle(m_hObject, &vTemp, &vDriftVel, &vColor, fCurLifeTime);
		}

		VEC_ADD(vPos, vPos, vDelta);
		fCurLifeTime += fLifeTimeOffset;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleExplosionFX::CreateDebris
//
//	PURPOSE:	Create a debris model
//
// ----------------------------------------------------------------------- //

HLOCALOBJ CParticleExplosionFX::CreateDebris()
{
	DVector vScale;
	VEC_SET(vScale, 1.0f, 1.0f, 1.0f);
	VEC_MULSCALAR(vScale, vScale, GetRandom(1.0f, 5.0f));

	char* pFilename = GetDebrisModel(DBT_STONE_BIG, vScale);
	char* pSkin     = GetDebrisSkin(DBT_STONE_BIG);

	if (!pFilename) return DNULL;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_MODEL;
	SAFE_STRCPY(createStruct.m_Filename, pFilename);
	SAFE_STRCPY(createStruct.m_SkinName, pSkin);
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT; 
	VEC_COPY(createStruct.m_Pos, m_vPos);

	HLOCALOBJ hObj = m_pClientDE->CreateObject(&createStruct);

	m_pClientDE->SetObjectScale(hObj, &vScale);

	return hObj;
}