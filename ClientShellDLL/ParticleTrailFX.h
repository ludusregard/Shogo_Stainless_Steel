// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleTrailFX.h
//
// PURPOSE : ParticleTrail special fx class - Definition
//
// CREATED : 4/27/98
//
// ----------------------------------------------------------------------- //

#ifndef __PARTICLE_TRAIL_FX_H__
#define __PARTICLE_TRAIL_FX_H__

#include "SpecialFX.h"


struct PTCREATESTRUCT : public SFXCREATESTRUCT
{
	PTCREATESTRUCT::PTCREATESTRUCT();

	DBYTE	nType;
	DBOOL   bSmall;
};

inline PTCREATESTRUCT::PTCREATESTRUCT()
{
	memset(this, 0, sizeof(PTCREATESTRUCT));
}

class CParticleTrailFX : public CSpecialFX
{
	public :

		CParticleTrailFX() : CSpecialFX() 
		{
			VEC_INIT(m_vLastPos);
			VEC_INIT(m_vColor1);
			VEC_INIT(m_vColor2);
			VEC_INIT(m_vDriftOffset);

			m_nType			= 0;
			m_fStartTime	= -1.0f;
			m_fLifeTime		= 0.0f;
			m_fFadeTime		= 0.0f;
			m_fOffsetTime	= 0.0f;
			m_nNumPerPuff	= 1;
			m_fSegmentTime	= 1.0f;
			m_bSmall		= DFALSE;
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DVector	m_vLastPos;		// Last Particle particle position
		DVector	m_vColor1;		// Color of darkest Particle particles
		DVector	m_vColor2;		// Color of lightest Particle particles
		DVector	m_vDriftOffset;	// Particle drift offset

		DFLOAT	m_fSegmentTime;	// When should we create a new segment
		DFLOAT	m_fFadeTime;	// When should segment start to fade
		DFLOAT	m_fLifeTime;	// How long segment stays around
		DFLOAT	m_fStartTime;	// When did we start this crazy thing
		DFLOAT	m_fOffsetTime;	// Time between particles

		int		m_nNumPerPuff;	// Number of particles per Particle puff
		DBOOL	m_bSmall;		// Relative size of Particle
		DBYTE	m_nType;		// Type of particle trail.
};

#endif // __PARTICLE_TRAIL_FX_H__