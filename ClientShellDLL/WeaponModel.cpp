// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponModel.cpp
//
// PURPOSE : Generic client-side WeaponModel wrapper class - Implementation
//
// CREATED : 9/27/97
//
// ----------------------------------------------------------------------- //

#include "WeaponModel.h"
#include "ClientUtilities.h"
#include "RiotClientShell.h"
#include "ShellCasingFX.h"
#include "SFXMsgIds.h"
#include "RiotSettings.h"
#include "RiotMsgIds.h"
#include "WeaponFX.h"
#include "ProjectileFX.h"
#include "ClientServerShared.h"
#include "ClientWeaponUtils.h"
#include "physics_lt.h"
#include "PlayerInventory.h"
#include "PlayerStats.h"
#include "CMoveMgr.h"

extern CRiotClientShell* g_pRiotClientShell;

DBYTE g_nRandomWeaponSeed;

#define INVALID_ANI				((HMODELANIM)-1)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CWeaponModel()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CWeaponModel::CWeaponModel()
{
	m_nWeaponId			= GUN_NONE;
	m_hObject			= DNULL;
	m_pClientDE			= DNULL;
	m_hFlashObject		= DNULL;

	m_fBobHeight		= 0.0f;
	m_fBobWidth			= 0.0f;
	m_fFlashStartTime	= 0.0f;

	VEC_INIT(m_vFlashPos);
	VEC_INIT(m_vOffset);
	VEC_INIT(m_vMuzzleOffset);

	m_fLastIdleTime			= 0.0f;
	m_fTimeBetweenIdles		= WEAPON_MIN_IDLE_TIME;
	m_nAmmoInClip			= 1;
	m_bFire					= DFALSE;
	m_eState				= W_IDLE;
	m_eLastWeaponState		= W_IDLE;

	m_nSelectAni			= INVALID_ANI;
	m_nDeselectAni			= INVALID_ANI;
	m_nIdleAni1				= INVALID_ANI;
	m_nIdleAni2				= INVALID_ANI;
	m_nFireAni				= INVALID_ANI;
	m_nFireAni2				= INVALID_ANI;
	m_nFireZoomAni			= INVALID_ANI;
	m_nLastFireAni			= INVALID_ANI;
	m_nStartFireAni			= INVALID_ANI;
	m_nStopFireAni			= INVALID_ANI;
	m_nReloadAni			= INVALID_ANI;

	m_vPath.Init();
	m_vFirePos.Init();
	m_vEndPos.Init();

	m_nIgnoreFX				= 0;
	m_eType					= PROJECTILE;
	m_bZoomView				= DFALSE;
	m_nRequestedWeaponId	= 0;
	m_bWeaponDeselected		= DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CWeaponModel()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CWeaponModel::~CWeaponModel()
{
	if (!m_pClientDE) return;

	if (m_hObject) 
	{
		m_pClientDE->DeleteObject(m_hObject);
	}

	if (m_hFlashObject) 
	{
		m_pClientDE->DeleteObject(m_hFlashObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Create()
//
//	PURPOSE:	Create the WeaponModel model
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponModel::Create(CClientDE* pClientDE, DBYTE nWeaponId)
{
	if (!pClientDE || !g_pRiotClientShell) return DFALSE;

	m_pClientDE   = pClientDE;
	m_nWeaponId	  = (RiotWeaponId)nWeaponId;

	CPlayerStats* pStats = g_pRiotClientShell->GetPlayerStats();
	if (!pStats) return DFALSE;

	m_nAmmoInClip = GetShotsPerClip(m_nWeaponId);
	//int nAmmo = pStats->GetAmmoCount(m_nWeaponId);
	//int nShotsPerClip = GetShotsPerClip(m_nWeaponId);
	//if (nShotsPerClip > 0)
	//{
	//	m_nAmmoInClip = nAmmo < nShotsPerClip ? nAmmo : nShotsPerClip;
	//}

	CreateModel();
	CreateFlash();

	// Allows for dynamic adjustment...

	VEC_COPY(m_vOffset, GetWeaponOffset2(m_nWeaponId));
	VEC_COPY(m_vMuzzleOffset, GetWeaponOffset2(m_nWeaponId));

	InitAnimations();

	Select();	// Select the weapon

	m_fLastIdleTime = m_pClientDE->GetTime();

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::InitAnimations
//
//	PURPOSE:	Set the animations
//
// ----------------------------------------------------------------------- //

void CWeaponModel::InitAnimations()
{
	if (!m_pClientDE || !m_hObject) return;

	m_nSelectAni	= m_pClientDE->GetAnimIndex(m_hObject, "Select");
	m_nDeselectAni	= m_pClientDE->GetAnimIndex(m_hObject, "Deselect");
	m_nStartFireAni	= m_pClientDE->GetAnimIndex(m_hObject, "Start_fire");
	m_nFireAni		= m_pClientDE->GetAnimIndex(m_hObject, "Fire");
	m_nFireAni2		= m_pClientDE->GetAnimIndex(m_hObject, "Fire2");
	m_nFireZoomAni	= m_pClientDE->GetAnimIndex(m_hObject, "Fire_zoom");
	m_nStopFireAni	= m_pClientDE->GetAnimIndex(m_hObject, "End_fire");
	m_nIdleAni1		= m_pClientDE->GetAnimIndex(m_hObject, "Idle_1");
	m_nIdleAni2		= m_pClientDE->GetAnimIndex(m_hObject, "Idle_2");
	m_nReloadAni	= m_pClientDE->GetAnimIndex(m_hObject, "Reload");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Reset()
//
//	PURPOSE:	Reset the model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::Reset()
{
	if (!m_pClientDE || !m_hObject) return;

	RemoveModel();

	m_nWeaponId				= GUN_NONE;
	m_nAmmoInClip			= 1;
	m_bFire					= DFALSE;
	m_bZoomView				= DFALSE;
	m_nRequestedWeaponId	= 0;
	m_bWeaponDeselected		= DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::RemoveModel()
//
//	PURPOSE:	Remove our model data member
//
// ----------------------------------------------------------------------- //

void CWeaponModel::RemoveModel()
{
	if (!m_pClientDE || !m_hObject) return;

	m_pClientDE->DeleteObject(m_hObject);
	m_hObject	= DNULL;
		
	if (m_hFlashObject) // Turn flash off...
	{
		m_pClientDE->SetObjectFlags(m_hFlashObject, 0);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateWeaponModel()
//
//	PURPOSE:	Update the WeaponModel state
//
// @parm the rotation of the WeaponModel
// @parm the original position
// @parm is the WeaponModel being fired?
//
// ----------------------------------------------------------------------- //

WeaponState CWeaponModel::UpdateWeaponModel(DRotation rRot, DVector vPos, DBOOL bFire)
{
	if (!m_pClientDE || !m_hObject || !g_pRiotClientShell) return W_IDLE;

	CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
	if (!pSettings) return W_IDLE;

	m_eLastWeaponState = m_eState;

	// Update the state of the model...

	WeaponState eState = UpdateModelState(bFire);
								  
	
		
	DVector vU, vR, vF, vNewPos;
	VEC_COPY(vNewPos, vPos);

	// Compute offset for WeaponModel and move the model to the correct position

	m_pClientDE->SetObjectPosAndRotation(m_hObject, &vPos, &rRot);
	m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	DVector vOffset, vMuzzleOffset, vRecoil;

	DBYTE nVal = pSettings->PlayerViewWeaponSetting();

	if (nVal == 1)
	{
		// This is left in to adjust muzzle position...
		// VEC_COPY(vOffset, m_vOffset);
		// VEC_COPY(vMuzzleOffset, m_vMuzzleOffset);
		VEC_COPY(vOffset, GetWeaponOffset2(m_nWeaponId));
		VEC_COPY(vMuzzleOffset, GetWeaponMuzzleOffset2(m_nWeaponId, eState));
	}
	else
	{
		VEC_COPY(vOffset, GetWeaponOffset(m_nWeaponId));
		VEC_COPY(vMuzzleOffset, GetWeaponMuzzleOffset(m_nWeaponId, eState));
	}

	VEC_COPY(vRecoil, GetWeaponRecoil(m_nWeaponId));

	DVector vTemp;
	
	// m_Pos += vR * vOffset.x;
	VEC_MULSCALAR(vTemp, vR, (vOffset.x + m_fBobWidth));
	VEC_ADD(vNewPos, vNewPos, vTemp);

	//m_Pos += vU * (vOffset.y + m_fBob);
	VEC_MULSCALAR(vTemp, vU, (vOffset.y + m_fBobHeight));
	VEC_ADD(vNewPos, vNewPos, vTemp);
	
	//m_Pos += vF * vOffset.z;
	VEC_MULSCALAR(vTemp, vF, (vOffset.z));
	VEC_ADD(vNewPos, vNewPos, vTemp);

	// m_Flash.m_Pos.Copy(pos);
	VEC_COPY(m_vFlashPos, vPos);

	//m_Flash.m_Pos += vR * (vOffset.x + vMuzzleOffset.x);
	VEC_MULSCALAR(vTemp, vR, (vOffset.x + vMuzzleOffset.x + m_fBobWidth));
	VEC_ADD(m_vFlashPos, m_vFlashPos, vTemp);

	//m_Flash.m_Pos += vU * (vOffset.y + vMuzzleOffset.y + m_fBob);
	VEC_MULSCALAR(vTemp, vU, (vOffset.y + vMuzzleOffset.y + m_fBobHeight));
	VEC_ADD(m_vFlashPos, m_vFlashPos, vTemp);

	//m_Flash.m_Pos += vF * (vOffset.z + vMuzzleOffset.z);
	VEC_MULSCALAR(vTemp, vF, (vOffset.z + vMuzzleOffset.z));
	VEC_ADD(m_vFlashPos, m_vFlashPos, vTemp);


	if (FiredWeapon(eState))
	{
		DFLOAT xRand = GetRandom(-vRecoil.x, vRecoil.x);
		DFLOAT yRand = GetRandom(-vRecoil.y, vRecoil.y);
		DFLOAT zRand = GetRandom(-vRecoil.z, vRecoil.z);

		// m_Pos += vU * yRand;
		VEC_MULSCALAR(vTemp, vU, yRand);
		VEC_ADD(vNewPos, vNewPos, vTemp);

		// m_Pos += vR * xRand;
		VEC_MULSCALAR(vTemp, vR, xRand);
		VEC_ADD(vNewPos, vNewPos, vTemp);

		// m_Pos += vF * zRand;
		VEC_MULSCALAR(vTemp, vF, zRand);
		VEC_ADD(vNewPos, vNewPos, vTemp);

		//m_Flash.m_Pos += vU * yRand;
		VEC_MULSCALAR(vTemp, vU, yRand);
		VEC_ADD(m_vFlashPos, m_vFlashPos, vTemp);

		//m_Flash.m_Pos += vR * xRand;
		VEC_MULSCALAR(vTemp, vR, xRand);
		VEC_ADD(m_vFlashPos, m_vFlashPos, vTemp);

		//m_Flash.m_Pos += vF * zRand;
		VEC_MULSCALAR(vTemp, vF, zRand);
		VEC_ADD(m_vFlashPos, m_vFlashPos, vTemp);

		if (!g_pRiotClientShell->HaveSilencer()) 
		{
			StartFlash();
		}
		
		
		// Send message to server telling player to fire...
			
		SendFireMsg();
	} 


	m_pClientDE->SetObjectPos(m_hObject, &vNewPos);


	// Update the muzzle flash...

	if (!g_pRiotClientShell->HaveSilencer())
	{
		UpdateFlash(eState);
	}

	
	// Inform the player if the weapon state changes...
	// (Don't tell the server about the deselect state - it told us to
	// deselect the weapon)...

	//if (m_eState != m_eLastWeaponState && m_eState != W_DESELECT)
	//{
	//	HMESSAGEWRITE hMsg = m_pClientDE->StartMessage(MID_WEAPON_STATE);
	//	m_pClientDE->WriteToMessageByte(hMsg, m_nWeaponId);
	//	m_pClientDE->WriteToMessageByte(hMsg, m_eState);
	//	m_pClientDE->EndMessage(hMsg);
	//}

	return eState;
}


/// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateBob()
//
//	PURPOSE:	Update WeaponModel bob
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateBob(DFLOAT fWidth, DFLOAT fHeight)
{
	m_fBobWidth  = fWidth;
	m_fBobHeight = fHeight;
}


/// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SetVisible()
//
//	PURPOSE:	Hide/Show the weapon model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::SetVisible(DBOOL bVis)
{
	if (!m_pClientDE || !m_hObject) return;
	
	DDWORD dwFlags = m_pClientDE->GetObjectFlags(m_hObject);

	if (bVis) dwFlags |= FLAG_VISIBLE;
	else dwFlags &= ~FLAG_VISIBLE;

	m_pClientDE->SetObjectFlags(m_hObject, dwFlags);
	
	if (m_hFlashObject) 
	{
		m_pClientDE->SetObjectFlags(m_hFlashObject, 0);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateFlash
//
//	PURPOSE:	Create the muzzle flash
//
// ----------------------------------------------------------------------- //

void CWeaponModel::CreateFlash()
{
	if (!m_pClientDE) return;

	char* pFlashName = GetFlashFilename(m_nWeaponId);
	if (!pFlashName) return;

	if (m_hFlashObject) 
	{
		m_pClientDE->DeleteObject(m_hFlashObject);
		m_hFlashObject = DNULL;
	}

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);
	
	createStruct.m_ObjectType = OT_SPRITE;
	SAFE_STRCPY(createStruct.m_Filename, pFlashName);
	createStruct.m_Flags	  = 0;

	if (createStruct.m_Filename[0] != ' ')
	{
		DVector vFlashScale = GetFlashScale(m_nWeaponId);
		m_hFlashObject = m_pClientDE->CreateObject(&createStruct);
		m_pClientDE->SetObjectScale(m_hFlashObject, &vFlashScale);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateModel
//
//	PURPOSE:	Create the weapon model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::CreateModel()
{
	if (!m_pClientDE) return;

	char* pModelName = GetPVModelName(m_nWeaponId);
	char* pModelSkin = GetPVModelSkin(m_nWeaponId);
	if (!pModelName || !pModelSkin) return;

	RemoveModel();

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);
	
	SAFE_STRCPY(createStruct.m_Filename, pModelName);
	SAFE_STRCPY(createStruct.m_SkinName, pModelSkin);
	createStruct.m_ObjectType = OT_MODEL;
	createStruct.m_Flags	  = FLAG_VISIBLE | FLAG_REALLYCLOSE | GetExtraWeaponFlags(m_nWeaponId);

	m_hObject = m_pClientDE->CreateObject(&createStruct);
	if (!m_hObject) return;

	m_pClientDE->SetObjectClientFlags(m_hObject, CF_NOTIFYMODELKEYS);

	m_pClientDE->SetModelLooping(m_hObject, DFALSE);
	m_pClientDE->SetModelAnimation(m_hObject, INVALID_ANI);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateFlash()
//
//	PURPOSE:	Update muzzle flash state
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateFlash(WeaponState eState)
{
	if (!m_pClientDE || !m_hFlashObject) return;

	DDWORD dwFlags = m_pClientDE->GetObjectFlags(m_hObject);
	if (!(dwFlags & FLAG_VISIBLE))
	{
		m_pClientDE->SetObjectFlags(m_hFlashObject, 0);
		return;
	}

	DFLOAT fCurTime		  = m_pClientDE->GetTime();
	DFLOAT fFlashDuration = GetFlashDuration(m_nWeaponId);

	if ( fCurTime >= m_fFlashStartTime + fFlashDuration ||
		 g_pRiotClientShell->GetPlayerState() != PS_ALIVE ||
		 IsLiquid(g_pRiotClientShell->GetCurContainerCode()) )
	{
		m_pClientDE->SetObjectFlags(m_hFlashObject, 0);
	}	
	else
	{
		m_pClientDE->SetObjectFlags(m_hFlashObject, FLAG_VISIBLE);
		m_pClientDE->SetObjectPos(m_hFlashObject, &m_vFlashPos);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::StartFlash()
//
//	PURPOSE:	Start the muzzle flash
//
// ----------------------------------------------------------------------- //

void CWeaponModel::StartFlash()
{
	if (!m_pClientDE || !m_hFlashObject) return;

	DFLOAT fCurTime		  = m_pClientDE->GetTime();
	DFLOAT fFlashDuration = GetFlashDuration(m_nWeaponId);

	// Scale the flash based on the weapon position setting...

	CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
	if (!pSettings) return;

	DBYTE nVal = pSettings->PlayerViewWeaponSetting();
	DVector vFlashScale;

	if (nVal == 1)
	{
		vFlashScale = GetFlashScale2(m_nWeaponId);
	}
	else
	{
		vFlashScale = GetFlashScale(m_nWeaponId);
	}

	m_pClientDE->SetObjectScale(m_hFlashObject, &vFlashScale);


	if (fCurTime >= m_fFlashStartTime + fFlashDuration)
	{
		m_fFlashStartTime = m_pClientDE->GetTime();
	}	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetModelPos()
//
//	PURPOSE:	Get the position of the weapon model
//
// ----------------------------------------------------------------------- //

DVector CWeaponModel::GetModelPos() const
{
	DVector vPos;
	VEC_INIT(vPos);

	if (m_pClientDE && m_hObject)
	{
		m_pClientDE->GetObjectPos(m_hObject, &vPos);
	}
			
	return vPos;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::StringKey()
//
//	PURPOSE:	Handle animation command
//
// ----------------------------------------------------------------------- //

void CWeaponModel::OnModelKey(HLOCALOBJ hObj, ArgList* pArgList)
{
	if (!m_pClientDE || !hObj || (hObj != m_hObject) || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	if (stricmp(pKey, WEAPON_KEY_FIRE) == 0)
	{
		m_bFire = DTRUE;
	}
	else if (stricmp(pKey, WEAPON_KEY_SOUND) == 0)
	{
		if (pArgList->argc > 1)
		{
			char* pSound = pArgList->argv[1];
			if (pSound)
			{
				char buf[100];
				sprintf(buf,"Sounds\\Weapons\\%s.wav", pSound);

				PlaySoundLocal(buf, SOUNDPRIORITY_PLAYER_HIGH, DFALSE, DFALSE, 100, DTRUE );

				// Send message to Server so that other client's can hear this sound...

				HSTRING hSound = m_pClientDE->CreateString(buf);

				HMESSAGEWRITE hWrite = m_pClientDE->StartMessage(MID_WEAPON_SOUND);
				m_pClientDE->WriteToMessageByte(hWrite, WEAPON_SOUND_KEY);
				m_pClientDE->WriteToMessageByte(hWrite, m_nWeaponId);
				m_pClientDE->WriteToMessageVector(hWrite, &m_vFlashPos);
				m_pClientDE->WriteToMessageHString(hWrite, hSound);
				m_pClientDE->EndMessage2(hWrite, MESSAGE_NAGGLEFAST|MESSAGE_GUARANTEED);

				m_pClientDE->FreeString(hSound);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateModelState
//
//	PURPOSE:	Update the model's state (fire if bFire == DTRUE)
//
// ----------------------------------------------------------------------- //

WeaponState CWeaponModel::UpdateModelState(DBOOL bFire)
{
	if (!m_pClientDE) return W_IDLE;

	WeaponState eRet = W_IDLE;


	// Determine what we should be doing...

	if (bFire) UpdateFiring();
	else UpdateNonFiring();

	if (m_bFire) 
	{
		eRet = Fire();
	}


	// See if we just finished deselecting the weapon...

	if (m_bWeaponDeselected)
	{
		m_bWeaponDeselected = DFALSE;
		HandleInternalWeaponChange(m_nRequestedWeaponId);
	}


	// See if we should force a weapons change...

	if (m_eState == W_FIRING_NOAMMO)
	{
		AutoSelectWeapon();
	}

	return eRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Fire
//
//	PURPOSE:	Handle firing the weapon
//
// ----------------------------------------------------------------------- //

WeaponState CWeaponModel::Fire()
{
	if (!m_pClientDE || !g_pRiotClientShell) return W_IDLE;

	WeaponState eRet = W_IDLE;

	CPlayerStats* pStats = g_pRiotClientShell->GetPlayerStats();
	if (!pStats) return W_IDLE;

	int nAmmo = pStats->GetAmmoCount(m_nWeaponId);

	// If this weapon uses ammo, make sure we have ammo...

	if (!UsesAmmo((RiotWeaponId)m_nWeaponId))
	{
		eRet = W_FIRED;
	}
	else if (nAmmo > 0)
	{
		int nShotsPerClip = GetShotsPerClip(m_nWeaponId);

		if (m_nAmmoInClip > 0)
		{
			if (nShotsPerClip > 0)
			{
				m_nAmmoInClip--;
			}

			nAmmo--;
		}


		eRet = W_FIRED;


		// See if we're playing the 2nd fire animation...

		if (m_hObject)
		{
			DDWORD dwAni = m_pClientDE->GetModelAnimation(m_hObject);
			if (dwAni == m_nFireAni2 && dwAni != INVALID_ANI)
			{
				eRet = W_FIRED2;
			}
		}

		
		// Check to see if we need to reload...

		if (nShotsPerClip > 0)
		{
			if ((m_nAmmoInClip <= 0) && (nAmmo > 0)) 
			{
				m_eState = W_RELOADING;
				m_nAmmoInClip = nAmmo < nShotsPerClip ? nAmmo : nShotsPerClip;
			}
		}
	} 
	else  // NO AMMO
	{
		m_eState = W_FIRING_NOAMMO;

		// Play dry-fire sound...

		char* pDryFireSound = GetWeaponDryFireSound(m_nWeaponId);
		if (pDryFireSound)
		{
			PlaySoundLocal(pDryFireSound, SOUNDPRIORITY_PLAYER_HIGH, DFALSE, DFALSE, 100, DTRUE );
		}

		
		// Send message to Server so that other client's can hear this sound...

		HMESSAGEWRITE hWrite = m_pClientDE->StartMessage(MID_WEAPON_SOUND);
		m_pClientDE->WriteToMessageByte(hWrite, WEAPON_SOUND_DRYFIRE);
		m_pClientDE->WriteToMessageByte(hWrite, m_nWeaponId);
		m_pClientDE->WriteToMessageVector(hWrite, &m_vFlashPos);
		m_pClientDE->WriteToMessageHString(hWrite, DNULL);
		m_pClientDE->EndMessage2(hWrite, MESSAGE_NAGGLEFAST|MESSAGE_GUARANTEED);
	}

	m_bFire = DFALSE;
	
	return eRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateFiring
//
//	PURPOSE:	Update the animation state of the model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateFiring()
{
	if (!m_pClientDE) return;

	switch (m_eState)
	{
		case W_IDLE :
		case W_END_FIRING :
		case W_BEGIN_FIRING :
		{
			m_eState = W_BEGIN_FIRING;

			if (!PlayStartFireAnimation())
			{
				PlayFireAnimation();
				m_eState = W_FIRING;
			}
		}
		break;

		case W_FIRING_NOAMMO :
		case W_FIRING :
		{
			PlayFireAnimation();
		}
		break;

		case W_RELOADING :
		{
			if (!PlayReloadAnimation())
			{
				PlayFireAnimation();
				m_eState = W_FIRING;
			}
		}
		break;

		case W_SELECT:
		{
			if (!PlaySelectAnimation())
			{
				PlayStartFireAnimation();
				m_eState = W_BEGIN_FIRING;
			}
		}
		break;

		case W_DESELECT:
		{
			if (!PlayDeselectAnimation())
			{
				m_bWeaponDeselected = DTRUE;
				PlayIdleAnimation();
				m_eState = W_IDLE;
			}
		}
		break;

		default : break;
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateNonFiring
//
//	PURPOSE:	Update the non-firing animation state of the model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateNonFiring()
{
	if (!m_pClientDE) return;

	switch (m_eState)
	{
		case W_IDLE :
		{
			PlayIdleAnimation();
		}
		break;

		case W_FIRING :
		case W_FIRING_NOAMMO :
		case W_END_FIRING :
		case W_BEGIN_FIRING :
		{
			m_eState = W_END_FIRING;

			if (!PlayStopFireAnimation())
			{
				// Don't idle right away after firing...

				m_fLastIdleTime = m_pClientDE->GetTime();

				PlayIdleAnimation();			
				m_eState = W_IDLE;
			}
		}
		break;

		case W_RELOADING :
		{
			if (!PlayReloadAnimation())
			{
				PlayIdleAnimation();
				m_eState = W_IDLE;
			}
		}
		break;

		case W_SELECT:
		{
			if (!PlaySelectAnimation())
			{
				PlayIdleAnimation();			
				m_eState = W_IDLE;
			}
		}
		break;

		case W_DESELECT:
		{
			if (!PlayDeselectAnimation())
			{
				m_bWeaponDeselected = DTRUE;
				PlayIdleAnimation();
				m_eState = W_IDLE;
			}
		}
		break;

		default : break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlaySelectAnimation()
//
//	PURPOSE:	Set model to select animation
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponModel::PlaySelectAnimation()
{
	if (!m_pClientDE || !m_hObject || m_nSelectAni == INVALID_ANI) return DFALSE;

	DDWORD dwAni	= m_pClientDE->GetModelAnimation(m_hObject);
	DDWORD dwState	= m_pClientDE->GetModelPlaybackState(m_hObject);

	if (dwAni == m_nSelectAni && (dwState & MS_PLAYDONE))
	{
		return DFALSE;  
	}
	else if (dwAni != m_nSelectAni)
	{
		m_pClientDE->SetModelAnimation(m_hObject, m_nSelectAni);
	}

	return DTRUE;  // Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayDeselectAnimation()
//
//	PURPOSE:	Set model to select animation
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponModel::PlayDeselectAnimation()
{
	if (!m_pClientDE || !m_hObject || m_nDeselectAni == INVALID_ANI) return DFALSE;

	DDWORD dwAni	= m_pClientDE->GetModelAnimation(m_hObject);
	DDWORD dwState	= m_pClientDE->GetModelPlaybackState(m_hObject);

	if (dwAni == m_nDeselectAni && (dwState & MS_PLAYDONE))
	{
		return DFALSE;  
	}
	else if (dwAni != m_nDeselectAni)
	{
		m_pClientDE->SetModelAnimation(m_hObject, m_nDeselectAni);
	}

	return DTRUE;  // Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayStartFireAnimation()
//
//	PURPOSE:	Set model to starting firing animation
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponModel::PlayStartFireAnimation()
{
	if (!m_pClientDE || !m_hObject || m_nStartFireAni == INVALID_ANI) return DFALSE;

	DDWORD dwAni	= m_pClientDE->GetModelAnimation(m_hObject);
	DDWORD dwState	= m_pClientDE->GetModelPlaybackState(m_hObject);

	if (dwAni == m_nStartFireAni && (dwState & MS_PLAYDONE))
	{
		return DFALSE;  
	}
	else if (dwAni != m_nStartFireAni)
	{
		m_pClientDE->SetModelAnimation(m_hObject, m_nStartFireAni);
	}

	return DTRUE;  // Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayStopFireAnimation()
//
//	PURPOSE:	Set model to the stop firing animation
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponModel::PlayStopFireAnimation()
{
	if (!m_pClientDE || !m_hObject || m_nStopFireAni == INVALID_ANI) return DFALSE;

	DDWORD dwAni	= m_pClientDE->GetModelAnimation(m_hObject);
	DDWORD dwState	= m_pClientDE->GetModelPlaybackState(m_hObject);

	if (dwAni == m_nStopFireAni && (dwState & MS_PLAYDONE))
	{
		return DFALSE;
	}
	else if (dwAni != m_nStopFireAni)
	{
		m_pClientDE->SetModelAnimation(m_hObject, m_nStopFireAni);
	}

	return DTRUE;  // Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayFireAnimation()
//
//	PURPOSE:	Set model to firing animation
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponModel::PlayFireAnimation()
{
	if (!m_pClientDE || !m_hObject) return DFALSE;

	DDWORD dwAni	= m_pClientDE->GetModelAnimation(m_hObject);
	DDWORD dwState	= m_pClientDE->GetModelPlaybackState(m_hObject);

	if ((dwAni != m_nFireAni && dwAni != m_nFireAni2 && dwAni != m_nFireZoomAni) || (dwState & MS_PLAYDONE))
	{
		if (g_pRiotClientShell->IsZoomed())
		{
			m_nLastFireAni = m_nFireZoomAni;
		}
		else  // Normal firing
		{
			if (m_nFireAni2 != INVALID_ANI)
			{
				m_nLastFireAni = (m_nLastFireAni == m_nFireAni ? m_nFireAni2 : m_nFireAni);
			}
			else
			{
				m_nLastFireAni = m_nFireAni;
			}
		}

		m_pClientDE->SetModelAnimation(m_hObject, m_nLastFireAni);
		m_pClientDE->ResetModelAnimation(m_hObject);  // Start from beginning
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayReloadAnimation()
//
//	PURPOSE:	Set model to reloading animation
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponModel::PlayReloadAnimation()
{
	if (!m_pClientDE || !m_hObject || m_nReloadAni == INVALID_ANI) return DFALSE;

	DDWORD dwAni	= m_pClientDE->GetModelAnimation(m_hObject);
	DDWORD dwState	= m_pClientDE->GetModelPlaybackState(m_hObject);

	if (dwAni == m_nReloadAni && (dwState & MS_PLAYDONE))
	{
		return DFALSE;
	}
	else if (dwAni != m_nReloadAni && (dwState & MS_PLAYDONE))
	{
		m_pClientDE->SetModelAnimation(m_hObject, m_nReloadAni);
	}

	return DTRUE;  // Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayIdleAnimation()
//
//	PURPOSE:	Set model to Idle animation
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponModel::PlayIdleAnimation()
{
	if (!m_pClientDE || !m_hObject || g_pRiotClientShell->IsZoomed()) return DFALSE;

	// Make sure idle animation is done if one is currently playing...

	DDWORD dwAni = m_pClientDE->GetModelAnimation(m_hObject);
	if (dwAni == m_nIdleAni1 || dwAni == m_nIdleAni2)
	{ 
		if (!(m_pClientDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE))
		{
			return DTRUE;
		}
	}


	DFLOAT fTime = m_pClientDE->GetTime();

	// Play idle if it is time...

	if ((fTime > m_fLastIdleTime + m_fTimeBetweenIdles))
	{
		m_fLastIdleTime		= fTime;
		m_fTimeBetweenIdles	= GetRandom(WEAPON_MIN_IDLE_TIME, WEAPON_MAX_IDLE_TIME);

		DDWORD nAni = m_nIdleAni1;
		if (m_nIdleAni2 != INVALID_ANI)
		{
			nAni = (GetRandom(0,1)==0 ? m_nIdleAni1 : m_nIdleAni2);
		}

		if (nAni == INVALID_ANI) nAni = 0;

		m_pClientDE->SetModelAnimation(m_hObject, nAni);

		return DTRUE;
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Select()
//
//	PURPOSE:	Select the weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::Select()
{
	if (!m_pClientDE) return;

	m_eState = W_SELECT;

	if (m_hObject && m_nSelectAni != INVALID_ANI)
	{
		m_pClientDE->SetModelAnimation(m_hObject, m_nSelectAni);
		m_pClientDE->ResetModelAnimation(m_hObject);
	}

	m_fLastIdleTime	= m_pClientDE->GetTime();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Deselect()
//
//	PURPOSE:	Deselect the weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::Deselect()
{
	if (!m_pClientDE) return;

	if (m_hObject && m_nDeselectAni != INVALID_ANI)
	{
		DDWORD dwAni = m_pClientDE->GetModelAnimation(m_hObject);

		if (dwAni != m_nDeselectAni)
		{
			m_eState = W_DESELECT;
			m_pClientDE->SetModelAnimation(m_hObject, m_nDeselectAni);
			m_pClientDE->ResetModelAnimation(m_hObject);
		}
	}
	else
	{
		m_bWeaponDeselected = DTRUE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::HandleStateChange()
//
//	PURPOSE:	Handle the weapon state changing
//
// ----------------------------------------------------------------------- //

void CWeaponModel::HandleStateChange(HMESSAGEREAD hMessage)
{
	if (!m_pClientDE) return;

	m_eState = (WeaponState) m_pClientDE->ReadFromMessageByte(hMessage);

	if (m_eState == W_DESELECT)
	{
		Deselect();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SendFireMsg
//
//	PURPOSE:	Send fire message to server
//
// ----------------------------------------------------------------------- //

void CWeaponModel::SendFireMsg()
{
	if (!m_pClientDE || !m_hObject) return;

	DRotation rRot;
	DVector vU, vR, vF;
	m_pClientDE->GetObjectRotation(m_hObject, &rRot);
	m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);


	// Make sure we always ignore the fire sound...

	m_nIgnoreFX = WFX_FIRESOUND;


	// Calculate a random seed...(srand uses this value so it can't be 1, since
	// that has a special meaning for srand)
	
	DBYTE nRandomSeed = GetRandom(2, 255);

	g_nRandomWeaponSeed = nRandomSeed;


	// Create a client-side projectile for every vector...

	int nVectorsPerShot = GetVectorsPerShot(m_nWeaponId);

	for (int i=0; i < nVectorsPerShot; i++)
	{
		DVector vPath, vFirePos;
		VEC_COPY(vPath, vF);
		VEC_COPY(vFirePos, m_vFlashPos);

		srand(g_nRandomWeaponSeed);
		g_nRandomWeaponSeed = GetRandom(2, 255);
		
		CalculateWeaponPathAndFirePos(m_nWeaponId, vPath, vFirePos, vU, vR);
		
		// m_pClientDE->CPrint("Client Fire Path (%d): %.2f, %.2f, %.2f", g_nRandomWeaponSeed, vPath.x, vPath.y, vPath.z);

		// Do client-side firing...

		ClientFire(vPath, vFirePos);
	}

	// Play Fire sound...

	char* pFireSound = GetWeaponFireSound(m_nWeaponId);
	if (pFireSound && !g_pRiotClientShell->HaveSilencer())
	{
		PlaySoundLocal(pFireSound, SOUNDPRIORITY_PLAYER_HIGH, DFALSE, DFALSE, 100, DTRUE );
	}

	// Calculate the range of this fire instance...

	DVector vTemp;
	VEC_SUB(vTemp, m_vFlashPos, m_vEndPos);
	DFLOAT fNewRange = VEC_MAG(vTemp);

	// Put zoom value in top bit...

	DBYTE nWeaponId = (m_nWeaponId | (m_bZoomView ? 0x80 : 0));

	// Send Fire message to server...

	HMESSAGEWRITE hWrite = m_pClientDE->StartMessage(MID_WEAPON_FIRE);
	m_pClientDE->WriteToMessageVector(hWrite, &m_vFlashPos);
	m_pClientDE->WriteToMessageVector(hWrite, &vF);
	m_pClientDE->WriteToMessageFloat(hWrite, fNewRange);
	m_pClientDE->WriteToMessageByte(hWrite, nRandomSeed);
	m_pClientDE->WriteToMessageByte(hWrite, nWeaponId);
	m_pClientDE->EndMessage2(hWrite, MESSAGE_NAGGLEFAST|MESSAGE_GUARANTEED);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ClientFire
//
//	PURPOSE:	Do client-side weapon firing
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ClientFire(DVector & vPath, DVector & vFirePos)
{
	if (!g_pRiotClientShell || !g_pRiotClientShell->IsMultiplayerGame()) return;

	VEC_COPY(m_vPath, vPath);
	VEC_COPY(m_vFirePos, vFirePos);

	m_eType	= GetWeaponType(m_nWeaponId);

	if (m_eType == PROJECTILE)
	{
		DoProjectile();
	}
	else
	{
		DoVector();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DoProjectile
//
//	PURPOSE:	Do client-side projectile
//
// ----------------------------------------------------------------------- //

void CWeaponModel::DoProjectile()
{
	if (!m_pClientDE || !m_hObject) return;

	// Let the server handle doing the kato grenade (since its movement is
	// so erratic)...Also don't do spider since it can stick to people...

	if (m_nWeaponId == GUN_KATOGRENADE_ID || m_nWeaponId == GUN_SPIDER_ID) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	PROJECTILECREATESTRUCT projectile;

	DDWORD dwId;
	m_pClientDE->GetLocalClientID(&dwId);

	projectile.hServerObj = CreateServerObj();
	projectile.nWeaponId  = m_nWeaponId;
	projectile.nShooterId = (DBYTE)dwId;
	projectile.bLocal	  = DTRUE;

	psfxMgr->CreateSFX(SFX_PROJECTILE_ID, &projectile);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateServerObj
//
//	PURPOSE:	Create a "server" object used by the projectile sfx
//
// ----------------------------------------------------------------------- //

HLOCALOBJ CWeaponModel::CreateServerObj()
{
	if (!m_pClientDE || !m_hObject) return DNULL;

	DRotation rRot;
	m_pClientDE->AlignRotation(&rRot, &m_vPath, DNULL);

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	DDWORD dwFlags = FLAG_POINTCOLLIDE | FLAG_NOSLIDING | FLAG_TOUCH_NOTIFY;
	dwFlags |= (m_nWeaponId == GUN_ENERGYGRENADE_ID) ? FLAG_GRAVITY : 0;

	createStruct.m_ObjectType = OT_NORMAL;
	createStruct.m_Flags = dwFlags;
	VEC_COPY(createStruct.m_Pos, m_vFirePos);
	ROT_COPY(createStruct.m_Rotation, rRot);

	HLOCALOBJ hObj = m_pClientDE->CreateObject(&createStruct);

	m_pClientDE->Physics()->SetForceIgnoreLimit(hObj, 0.0f);

	return hObj;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DoVector
//
//	PURPOSE:	Do client-side vector
//
// ----------------------------------------------------------------------- //

void CWeaponModel::DoVector()
{
	if (!m_pClientDE || !m_hObject || !g_pRiotClientShell) return;

	IntersectInfo iInfo;
	IntersectQuery qInfo;
	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	DVector vTemp;
	VEC_MULSCALAR(vTemp, m_vPath, GetWeaponRange(m_nWeaponId));
	VEC_ADD(m_vEndPos, m_vFirePos, vTemp);

	HOBJECT hFilterList[] = { m_pClientDE->GetClientObject(), 
						      g_pRiotClientShell->GetMoveMgr()->GetObject(), 
							  DNULL };

	qInfo.m_FilterFn  = ObjListFilterFn;
	qInfo.m_pUserData = hFilterList;

	VEC_COPY(qInfo.m_From, m_vFirePos);
	VEC_COPY(qInfo.m_To, m_vEndPos);

	if (m_pClientDE->IntersectSegment(&qInfo, &iInfo))
	{
		HandleVectorImpact(qInfo, iInfo);

		// If we hit something and we're firing a cannon type weapon figure out
		// where the impact was...

		if (m_eType == CANNON)
		{
			VEC_COPY(m_vEndPos, iInfo.m_Point);
		}
	}
	else
	{
		if (m_eType != MELEE) 
		{
			DVector vUp;
			VEC_SET(vUp, 0.0f, 1.0f, 0.0f);
			AddImpact(DNULL, m_vEndPos, vUp, ST_SKY);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::AddImpact
//
//	PURPOSE:	Add the weapon impact
//
// ----------------------------------------------------------------------- //

void CWeaponModel::AddImpact(HLOCALOBJ hObj, DVector & vImpactPoint, 
							 DVector & vNormal, SurfaceType eType)
{
	if (!m_pClientDE || !m_hObject || !g_pRiotClientShell) return;

	::AddLocalImpactFX(hObj, m_vFirePos, vImpactPoint, vNormal, eType, 
					   m_vPath, m_nWeaponId, m_nIgnoreFX);

	// If we do multiple calls to AddLocalImpact, make sure we only do some
	// effects once :)

	m_nIgnoreFX |= WFX_SHELL | WFX_LIGHT | WFX_MUZZLE | WFX_TRACER;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleVectorImpact
//
//	PURPOSE:	Handle a vector hitting something
//
// ----------------------------------------------------------------------- //

void CWeaponModel::HandleVectorImpact(IntersectQuery & qInfo, IntersectInfo & iInfo)
{
	if (!m_pClientDE) return;

	// Get the surface type (check the poly first)...

	SurfaceType eType = GetSurfaceType(iInfo.m_hPoly);
	
	if (eType == ST_UNKNOWN)
	{
		eType = GetSurfaceType(iInfo.m_hObject);
	}


	if (!(eType == ST_LIQUID && m_eType == CANNON))
	{
		AddImpact(iInfo.m_hObject, iInfo.m_Point, iInfo.m_Plane.m_Normal, eType);
	}

	
	// If we hit liquid, cast another ray that will go through the water...
	
	if (eType == ST_LIQUID)
	{
		qInfo.m_FilterFn = AttackerLiquidFilterFn;

		if (m_pClientDE->IntersectSegment(&qInfo, &iInfo))
		{
			// Get the surface type (check the poly first)...

			SurfaceType eType = GetSurfaceType(iInfo.m_hPoly);
			
			if (eType == ST_UNKNOWN)
			{
				eType = GetSurfaceType(iInfo.m_hObject);
			}

			AddImpact(iInfo.m_hObject, iInfo.m_Point, iInfo.m_Plane.m_Normal, eType);
		}
	}
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CanChangeToWeapon()
//
//	PURPOSE:	See if we can change to this weapon
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponModel::CanChangeToWeapon(DBYTE nCommandId)
{
	if (!g_pRiotClientShell) return DFALSE;
	if (g_pRiotClientShell->IsPlayerDead() || g_pRiotClientShell->IsSpectatorMode()) return DFALSE;

	CPlayerInventory* pInventory = g_pRiotClientShell->GetInventory();
	if (!pInventory) return DFALSE;

	CPlayerStats* pStats = g_pRiotClientShell->GetPlayerStats();
	if (!pStats) return DFALSE;

	DBYTE nPlayerMode = g_pRiotClientShell->GetPlayerMode();
	DBYTE nWeaponId = GetWeaponId(nCommandId, nPlayerMode);


#ifdef _DEMO
	if (!IsDemoWeapon(nCommandId))
	{
		// Tell the user that this weapon is not available...

		g_pRiotClientShell->UpdatePlayerStats(IC_OUTOFAMMO_ID, nWeaponId, 1.0f);		
		return DFALSE;
	}
#endif


	// Kid doesn't have any weapons...

	if (nPlayerMode == PM_MODE_KID) return DFALSE;

	// Can't use weapons in vehicle mode...

	if (g_pRiotClientShell->IsVehicleMode()) return DFALSE;


	// Make sure this is a valid weapon for us to switch to...

	if (!pInventory->CanDrawGun(nWeaponId)) return DFALSE;

	
	// If this weapon has no ammo, let user know...

	if (UsesAmmo((RiotWeaponId)nWeaponId) && pStats->GetAmmoCount(nWeaponId) <= 0)
	{
		g_pRiotClientShell->UpdatePlayerStats(IC_OUTOFAMMO_ID, nWeaponId, 0.0f);		
		return DFALSE;
	}

	
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ChangeWeapon()
//
//	PURPOSE:	Change to a different weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ChangeWeapon(DBYTE nCommandId)
{
	if (!g_pRiotClientShell || !CanChangeToWeapon(nCommandId)) return;

	DBYTE nPlayerMode = g_pRiotClientShell->GetPlayerMode();

	// Don't do anything if we are trying to change to the same weapon...

	DBOOL bDeselectWeapon = (m_nWeaponId != GUN_NONE);
	DBYTE nWeaponId = GetWeaponId(nCommandId, nPlayerMode);

	if (nWeaponId == m_nWeaponId)
	{
		// Well, okay, some weapons have toggles if selected again...

		if (CanWeaponZoom(nWeaponId))
		{
			bDeselectWeapon = DFALSE;
		}
		else
		{
			return;
		}
	}


	// Handle deselection of current weapon...

	if (bDeselectWeapon)
	{
		Deselect();

		// Need to wait for deselection animation to finish...Save the
		// new weapon id...

		m_nRequestedWeaponId = nWeaponId;
	}
	else
	{
		HandleInternalWeaponChange(nWeaponId);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::HandleInternalWeaponChange()
//
//	PURPOSE:	Change to a different weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::HandleInternalWeaponChange(DBYTE nWeaponId)
{
	if (!g_pRiotClientShell) return;
	if (g_pRiotClientShell->IsPlayerDead() || g_pRiotClientShell->IsSpectatorMode()) return;

	// Check to see if we are already on this weapon...

	if (nWeaponId == m_nWeaponId)
	{
		// If the weapon can be zoomed, toggle the zoom modes...

		if (CanWeaponZoom(nWeaponId))
		{
			if (g_pRiotClientShell->IsChaseView())
			{ 
				m_bZoomView = DFALSE;
			}
			else
			{
				m_bZoomView = !m_bZoomView;
			}
		}
		else
		{
			return;
		}
	}
	else
	{
		m_bZoomView = DFALSE;
	}


	// Change to the weapon...

	DoWeaponChange(nWeaponId);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DoWeaponChange()
//
//	PURPOSE:	Do the actual weapon change.  This isn't part of 
//				HandleInternalWeaponChange() so that it can be called when
//				loading the player.  
//
// ----------------------------------------------------------------------- //

void CWeaponModel::DoWeaponChange(DBYTE nWeaponId)
{
	if (!g_pRiotClientShell) return;

	CPlayerInventory* pInventory = g_pRiotClientShell->GetInventory();
	if (!pInventory) return;

	CPlayerStats* pStats = g_pRiotClientShell->GetPlayerStats();
	if (!pStats) return;

	if (pInventory->CanDrawGun(nWeaponId))
	{
		g_pRiotClientShell->ChangeWeapon(nWeaponId, m_bZoomView, pStats->GetAmmoCount(nWeaponId));
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::AutoSelectWeapon()
//
//	PURPOSE:	Determine what weapon to switch to, and switch
//
// ----------------------------------------------------------------------- //

void CWeaponModel::AutoSelectWeapon()
{
	if (!g_pRiotClientShell) return;

	DBYTE nPlayerMode = g_pRiotClientShell->GetPlayerMode();
	DBYTE nWeaponId = NextWeapon(nPlayerMode);

	// Don't switch to the melee weapon unless it is our only weapon...

	if (GetWeaponType((RiotWeaponId)nWeaponId) == MELEE)
	{
		nWeaponId = PrevWeapon(nPlayerMode);
	}

	ChangeWeapon(GetCommandId(nWeaponId)); 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ChangeToPrevWeapon()
//
//	PURPOSE:	Change to the previous weapon is
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ChangeToPrevWeapon()	
{ 
	if (!g_pRiotClientShell) return;

	DBYTE nPlayerMode = g_pRiotClientShell->GetPlayerMode();
	DBYTE nWeaponId = PrevWeapon(nPlayerMode);

	ChangeWeapon(GetCommandId(nWeaponId));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ChangeToNextWeapon()
//
//	PURPOSE:	Change to the next weapon is
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ChangeToNextWeapon()	
{ 
	if (!g_pRiotClientShell) return;

	DBYTE nPlayerMode = g_pRiotClientShell->GetPlayerMode();
	DBYTE nWeaponId = NextWeapon(nPlayerMode);

	ChangeWeapon(GetCommandId(nWeaponId));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PrevWeapon()
//
//	PURPOSE:	Determine what the previous weapon is
//
// ----------------------------------------------------------------------- //

DBYTE CWeaponModel::PrevWeapon(DBYTE nPlayerMode)
{
	if (!g_pRiotClientShell) return -1;

	CPlayerInventory* pInventory = g_pRiotClientShell->GetInventory();
	if (!pInventory) return -1;

	CPlayerStats* pStats = g_pRiotClientShell->GetPlayerStats();
	if (!pStats) return -1;

	if (!pInventory->CanDrawGun(m_nWeaponId)) return -1;

#ifdef _DEMO
	return DemoPrevWeapon(nPlayerMode);
#endif

	int nMinWeapon = COMMAND_ID_WEAPON_1;
	int nMaxWeapon = COMMAND_ID_WEAPON_10;
	int nOriginalWeapon = GetCommandId(m_nWeaponId);
	
	int nWeapon = nOriginalWeapon - 1;
	if (nWeapon < nMinWeapon) nWeapon = nMaxWeapon;
	int nWeaponIndex = GetWeaponId(nWeapon, nPlayerMode);
	
	while (!pInventory->CanDrawGun(nWeaponIndex) || 
		   (UsesAmmo((RiotWeaponId)nWeaponIndex) && 
		    pStats->GetAmmoCount(nWeaponIndex) <= 0))
	{
		nWeapon--;

		if (nWeapon < nMinWeapon) nWeapon = nMaxWeapon;

		if (nWeapon == nOriginalWeapon) break;

		nWeaponIndex = GetWeaponId (nWeapon, nPlayerMode);
	}

	return (DBYTE)GetWeaponId(nWeapon, nPlayerMode);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::NextWeapon()
//
//	PURPOSE:	Determine what the next weapon is
//
// ----------------------------------------------------------------------- //

DBYTE CWeaponModel::NextWeapon(DBYTE nPlayerMode)
{
	if (!g_pRiotClientShell) return -1;

	CPlayerInventory* pInventory = g_pRiotClientShell->GetInventory();
	if (!pInventory) return -1;

	CPlayerStats* pStats = g_pRiotClientShell->GetPlayerStats();
	if (!pStats) return -1;

	if (!pInventory->CanDrawGun(m_nWeaponId)) return -1;

#ifdef _DEMO
	return DemoNextWeapon(nPlayerMode);
#endif

	int nMinWeapon = COMMAND_ID_WEAPON_1;
	int nMaxWeapon = COMMAND_ID_WEAPON_10;
	int nOriginalWeapon = GetCommandId(m_nWeaponId);
	
	int nWeapon = nOriginalWeapon + 1;
	if (nWeapon > nMaxWeapon) nWeapon = nMinWeapon;
	int nWeaponIndex = GetWeaponId(nWeapon, nPlayerMode);

	while (!pInventory->CanDrawGun(nWeaponIndex) || 
		   (UsesAmmo((RiotWeaponId)nWeaponIndex) && 
		    pStats->GetAmmoCount(nWeaponIndex) <= 0))
	{
		nWeapon++;

		if (nWeapon > nMaxWeapon) nWeapon = nMinWeapon;

		if (nWeapon == nOriginalWeapon) break;

		nWeaponIndex = GetWeaponId(nWeapon, nPlayerMode);
	}

	return (DBYTE)GetWeaponId(nWeapon, nPlayerMode);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DemoPrevWeapon()
//
//	PURPOSE:	Return the previous weapon (if valid) in Demo mode
//
// ----------------------------------------------------------------------- //

DBYTE CWeaponModel::DemoPrevWeapon(DBYTE nPlayerMode)
{
	if (!g_pRiotClientShell) return -1;

	CPlayerInventory* pInventory = g_pRiotClientShell->GetInventory();
	if (!pInventory) return -1;

	CPlayerStats* pStats = g_pRiotClientShell->GetPlayerStats();
	if (!pStats) return -1;

	DBYTE OnFootDemoWeapons[] = 
	{
		COMMAND_ID_WEAPON_10,	// Melee
		COMMAND_ID_WEAPON_1,	// Colts
		COMMAND_ID_WEAPON_3,	// Mac10
		COMMAND_ID_WEAPON_7		// Tow
	};

	DBYTE MCADemoWeapons[] = 
	{
		COMMAND_ID_WEAPON_10,	// Melee
		COMMAND_ID_WEAPON_1,	// Pulse rifle
		COMMAND_ID_WEAPON_4,	// Bullgut
		COMMAND_ID_WEAPON_5		// Sniper rifle
	};

	DBYTE* pValidWeapons = MCADemoWeapons;

	if (nPlayerMode == PM_MODE_FOOT)
	{
		pValidWeapons = OnFootDemoWeapons;
	}

	int nMinWeapon		= COMMAND_ID_WEAPON_1;
	int nMaxWeapon		= COMMAND_ID_WEAPON_10;
	int nOriginalWeapon = GetCommandId(m_nWeaponId);
	int nOriginalIndex  = 0;

	for (nOriginalIndex=0; nOriginalIndex < 4; nOriginalIndex++)
	{
		if (pValidWeapons[nOriginalIndex] == nOriginalWeapon) break;
	}
	if (nOriginalIndex == 4) return -1;


	int nIndex = nOriginalIndex - 1;
	if (nIndex < 0) nIndex = 3;
	int nWeaponIndex = GetWeaponId(pValidWeapons[nIndex], nPlayerMode);
	
	while (!pInventory->CanDrawGun(nWeaponIndex) || 
		   (UsesAmmo((RiotWeaponId)nWeaponIndex) && 
		    pStats->GetAmmoCount(nWeaponIndex) <= 0))
	{
		nIndex--;

		if (nIndex < 0) nIndex = 3;

		if (nIndex == nOriginalIndex) break;

		nWeaponIndex = GetWeaponId(pValidWeapons[nIndex], nPlayerMode);
	}
	
	return (DBYTE)GetWeaponId(pValidWeapons[nIndex], nPlayerMode);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DemoNextWeapon()
//
//	PURPOSE:	Return the next weapon (if valid) in Demo mode
//
// ----------------------------------------------------------------------- //

DBYTE CWeaponModel::DemoNextWeapon(DBYTE nPlayerMode)
{
	if (!g_pRiotClientShell) return -1;

	CPlayerInventory* pInventory = g_pRiotClientShell->GetInventory();
	if (!pInventory) return -1;

	CPlayerStats* pStats = g_pRiotClientShell->GetPlayerStats();
	if (!pStats) return -1;

	DBYTE OnFootDemoWeapons[] = 
	{
		COMMAND_ID_WEAPON_10,	// Melee
		COMMAND_ID_WEAPON_1,	// Colts
		COMMAND_ID_WEAPON_3,	// Mac10
		COMMAND_ID_WEAPON_7		// Tow
	};

	DBYTE MCADemoWeapons[] = 
	{
		COMMAND_ID_WEAPON_10,	// Melee
		COMMAND_ID_WEAPON_1,	// Pulse rifle
		COMMAND_ID_WEAPON_4,	// Bullgut
		COMMAND_ID_WEAPON_5		// Sniper rifle
	};

	DBYTE* pValidWeapons = MCADemoWeapons;

	if (nPlayerMode == PM_MODE_FOOT)
	{
		pValidWeapons = OnFootDemoWeapons;
	}

	int nMinWeapon		= COMMAND_ID_WEAPON_1;
	int nMaxWeapon		= COMMAND_ID_WEAPON_10;
	int nOriginalWeapon = GetCommandId(m_nWeaponId);
	int nOriginalIndex  = 0;

	for (nOriginalIndex=0; nOriginalIndex < 4; nOriginalIndex++)
	{
		if (pValidWeapons[nOriginalIndex] == nOriginalWeapon) break;
	}
	if (nOriginalIndex == 4) return -1;


	int nIndex = nOriginalIndex + 1;
	if (nIndex > 3) nIndex = 0;
	int nWeaponIndex = GetWeaponId(pValidWeapons[nIndex], nPlayerMode);
	
	while (!pInventory->CanDrawGun(nWeaponIndex) || 
		   (UsesAmmo((RiotWeaponId)nWeaponIndex) && 
		    pStats->GetAmmoCount(nWeaponIndex) <= 0))
	{
		nIndex++;

		if (nIndex > 3) nIndex = 0;

		if (nIndex == nOriginalIndex) break;

		nWeaponIndex = GetWeaponId(pValidWeapons[nIndex], nPlayerMode);
	}
	
	return (DBYTE)GetWeaponId(pValidWeapons[nIndex], nPlayerMode);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsDemoWeapon()
//
//	PURPOSE:	See if this is a weapon available in the demo...
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponModel::IsDemoWeapon(DBYTE nCommandId)
{
	if (!g_pRiotClientShell) return DFALSE;

	DBOOL bRet = DTRUE;

	if (g_pRiotClientShell->IsOnFoot())
	{
		// We can only use the Melee, Colts, Mac10, and TOW...

		if (nCommandId != COMMAND_ID_WEAPON_10 && nCommandId != COMMAND_ID_WEAPON_1 &&
			nCommandId != COMMAND_ID_WEAPON_3 && nCommandId != COMMAND_ID_WEAPON_7)
		{
			bRet = DFALSE;
		}
	}
	else  // We're in Mecha mode
	{
		// We can only use the Melee, Pulse rilfe, Bullgut, and sniper rifle...

		if (nCommandId != COMMAND_ID_WEAPON_10 && nCommandId != COMMAND_ID_WEAPON_1 &&
			nCommandId != COMMAND_ID_WEAPON_4 && nCommandId != COMMAND_ID_WEAPON_5)
		{
			bRet = DFALSE;
		}
	}

	return bRet;
}

