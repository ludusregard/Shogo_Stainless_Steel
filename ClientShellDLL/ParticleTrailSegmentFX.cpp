// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleTrailSegmentFX.cpp
//
// PURPOSE : ParticleTrail segment special FX - Implementation
//
// CREATED : 4/27/98
//
// ----------------------------------------------------------------------- //

#include "ParticleTrailSegmentFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ContainerCodes.h"
#include "ClientServerShared.h"
#include "WeaponFXTypes.h"
#include "RiotSettings.h"
#include "RiotClientShell.h"

extern DVector g_vWorldWindVel;
extern CRiotClientShell* g_pRiotClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleTrailSegmentFX::Init
//
//	PURPOSE:	Init the Particle trail segment
//
// ----------------------------------------------------------------------- //

DBOOL CParticleTrailSegmentFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return DFALSE;

	PTSCREATESTRUCT* pPTS = (PTSCREATESTRUCT*)psfxCreateStruct;
	VEC_COPY(m_vColor1, pPTS->vColor1);
	VEC_COPY(m_vColor2, pPTS->vColor2);
	VEC_COPY(m_vDriftOffset, pPTS->vDriftOffset);
	m_nType			= pPTS->nType;
	m_bSmall		= pPTS->bSmall;
	m_fLifeTime		= pPTS->fLifeTime;
	m_fFadeTime		= pPTS->fFadeTime;
	m_fOffsetTime	= pPTS->fOffsetTime;
	m_fRadius		= pPTS->fRadius;
	m_fGravity		= pPTS->fGravity;
	m_nNumPerPuff	= pPTS->nNumPerPuff;

	m_bIgnoreWind	= DFALSE;

	if (m_bSmall)
	{
		m_fRadius /= 2.0f;
	}

	//m_vRotVel.z = 360000.0f;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CParticleTrailSegmentFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE || !m_hServerObject) return DFALSE;

	if ((m_nType & PT_SMOKE) || (m_nType & PT_GIBSMOKE))
	{
		// m_pTextureName = "SpecialFX\\ParticleTextures\\Smoke.dtx";
		m_pTextureName = "Sprites\\SmokeTest.spr";
	}
	else if (m_nType & PT_BLOOD)
	{
		if (GetRandom(0, 1) == 0)
		{
			m_pTextureName = "SpecialFX\\ParticleTextures\\Blood_1.dtx";
		}
		else
		{
			m_pTextureName = "SpecialFX\\ParticleTextures\\Blood_2.dtx";
		}
	}

	// Determine if we are in a liquid...

	DVector vPos;
	pClientDE->GetObjectPos(m_hServerObject, &vPos);

	HLOCALOBJ objList[1];
	DDWORD dwNum = pClientDE->GetPointContainers(&vPos, objList, 1);

	if (dwNum > 0 && objList[0])
	{
		DDWORD dwUserFlags;
		pClientDE->GetObjectUserFlags(objList[0], &dwUserFlags);

		if (dwUserFlags & USRFLG_VISIBLE)
		{
			D_WORD dwCode;
			if (pClientDE->GetContainerCode(objList[0], &dwCode))
			{
				if (IsLiquid((ContainerCode)dwCode))
				{
					// Don't create blood under water...

					if (m_nType & PT_BLOOD)
					{
						m_bWantRemove = DTRUE;
						m_fLifeTime = 0.0f;
						m_fFadeTime = 0.0f;
						return DFALSE;
					}

					m_fRadius = 500.0f;
					m_fGravity = 5.0f;
					m_nNumPerPuff *= 3;
					m_bIgnoreWind = DTRUE;
					m_pTextureName = "SpecialFX\\ParticleTextures\\GreySphere_1.dtx";
					GetLiquidColorRange((ContainerCode)dwCode, &m_vColor1, &m_vColor2);
				}
			}
		}
	}

	return CBaseParticleSystemFX::CreateObject(pClientDE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleTrailSegmentFX::Update
//
//	PURPOSE:	Update the Particle trail (add Particle)
//
// ----------------------------------------------------------------------- //

DBOOL CParticleTrailSegmentFX::Update()
{
	if (!m_hObject || !m_pClientDE) return DFALSE;

	if (!CBaseParticleSystemFX::Update()) return DFALSE;

	CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
	if (!pSettings) return DFALSE;

	DBYTE nDetailLevel = pSettings->SpecialFXSetting();


	DFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
		if (!m_hServerObject) return DFALSE;

		m_bFirstUpdate = DFALSE;
		m_fStartTime   = fTime;
		m_fLastTime	   = fTime;

		// Where is the server (moving) object...

		DVector vPos, vTemp;
		m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
		
		// Current position is relative to the particle system's postion (i.e., 
		// each puff of Particle is some distance away from the particle system's 
		/// position)...

		m_pClientDE->GetObjectPos(m_hObject, &vTemp);
		VEC_SUB(vPos, vPos, vTemp);

		VEC_COPY(m_vLastPos, vPos);
	}


	// Check to see if we should just wait for last Particle puff to go away...

	if (m_bWantRemove || (fTime > m_fStartTime + m_fFadeTime))
	{
		if (fTime > m_fLastTime + m_fLifeTime)
		{
			return DFALSE;
		}

		DFLOAT fScale = (m_fLifeTime - (fTime - m_fLastTime)) / m_fLifeTime;

		DFLOAT r, g, b, a;
		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
		m_pClientDE->SetObjectColor(m_hObject, r, g, b, fScale);
		
		return DTRUE;
	}


	// See if it is time to create a new Particle puff...

	if ((fTime > m_fLastTime + m_fOffsetTime) && m_hServerObject)
	{
		DVector vCurPos, vPos, vDelta, vTemp, vDriftVel, vColor;

		// Calculate Particle puff position...

		// Where is the server (moving) object...

		m_pClientDE->GetObjectPos(m_hServerObject, &vCurPos);
		

		// Current position is relative to the particle system's postion (i.e., 
		// each puff of Particle is some distance away from the particle system's 
		/// position)...

		m_pClientDE->GetObjectPos(m_hObject, &vTemp);
		VEC_SUB(vCurPos, vCurPos, vTemp);


		// How long has it been since the last Particle puff?

		DFLOAT fTimeOffset = fTime - m_fLastTime;

		
		// What is the range of colors?

		DFLOAT fRange = m_vColor2.x - m_vColor1.x;

		
		// Fill the distance between the last projectile position, and it's 
		// current position with Particle puffs...

		int nNumSteps = (m_fLastTime > 0) ? (((m_nType & PT_BLOOD) || (m_nType & PT_GIBSMOKE)) ? 20 : 5): 1;

		if (nDetailLevel != RS_HIGH)
		{
			nNumSteps /= 2;
		}

		VEC_SUB(vTemp, vCurPos, m_vLastPos);
		VEC_MULSCALAR(vDelta, vTemp, 1.0f/float(nNumSteps));

		VEC_COPY(vPos, m_vLastPos);

		DFLOAT fCurLifeTime = 10.0f;
		if (nDetailLevel == RS_HIGH)
		{
			fCurLifeTime /= 2;
		}

		DFLOAT fLifeTimeOffset = fTimeOffset / float(nNumSteps);

		DFLOAT fOffset = 0.5f;

		if (m_bSmall)
		{
			fOffset /= 2.0f;
		}

		for (int i=0; i < nNumSteps; i++)
		{
			// Build the individual Particle puffs...

			for (int j=0; j < m_nNumPerPuff; j++)
			{
				VEC_COPY(vTemp, vPos);

				if (m_bIgnoreWind)
				{
					VEC_SET(vDriftVel, GetRandom(-m_vDriftOffset.x*2.0f, -m_vDriftOffset.x), 
									   GetRandom(5.0f, 6.0f), 
									   GetRandom(-m_vDriftOffset.z, m_vDriftOffset.z));
				}
				else
				{
					VEC_SET(vDriftVel, g_vWorldWindVel.x + GetRandom(-m_vDriftOffset.x*2.0f, -m_vDriftOffset.x), 
									   g_vWorldWindVel.y + GetRandom(5.0f, 6.0f), 
									   g_vWorldWindVel.z + GetRandom(-m_vDriftOffset.z, m_vDriftOffset.z));
				}

				vTemp.x += GetRandom(-fOffset, fOffset);
				vTemp.y += GetRandom(-fOffset, fOffset);
				vTemp.z += GetRandom(-fOffset, fOffset);

				GetRandomColorInRange(vColor);

				// For software make sure blood is red...

				if ((m_nType & PT_BLOOD))
				{
					m_pClientDE->SetSoftwarePSColor(m_hObject, 1.0f, 0.0f, 0.0f);
				}

				m_pClientDE->AddParticle(m_hObject, &vTemp, &vDriftVel, &vColor, fCurLifeTime);
			}

			VEC_ADD(vPos, vPos, vDelta);
			fCurLifeTime += fLifeTimeOffset;
		}

		m_fLastTime = fTime;

		VEC_COPY(m_vLastPos, vCurPos);
	}

	return DTRUE;
}
