// ----------------------------------------------------------------------- //
//
// MODULE  : ClientUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#include <stdlib.h>
#include "ClientUtilities.h"
#include "RiotClientShell.h"
#include "ClientRes.h"

extern CRiotClientShell* g_pRiotClientShell;

CommandID g_CommandArray[NUM_COMMANDS] = 
{
	{ IDS_CONTROL_FORWARD,				COMMAND_ID_FORWARD },
	{ IDS_CONTROL_BACKWARD,				COMMAND_ID_REVERSE },
	{ IDS_CONTROL_TURNLEFT,				COMMAND_ID_LEFT },
	{ IDS_CONTROL_TURNRIGHT,			COMMAND_ID_RIGHT },
	{ IDS_CONTROL_STRAFE,				COMMAND_ID_STRAFE },
	{ IDS_CONTROL_STRAFELEFT,			COMMAND_ID_STRAFE_LEFT },
	{ IDS_CONTROL_STRAFERIGHT,			COMMAND_ID_STRAFE_RIGHT },
	{ IDS_CONTROL_TURNAROUND,			COMMAND_ID_TURNAROUND },
	{ IDS_CONTROL_RUN,					COMMAND_ID_RUN },
	{ IDS_CONTROL_FIRE,					COMMAND_ID_FIRING },
//	{ IDS_CONTROL_ACTIVATE,				COMMAND_ID_ACTIVATE },
	{ IDS_CONTROL_JUMP,					COMMAND_ID_JUMP },
	{ IDS_CONTROL_DOUBLEJUMP,			COMMAND_ID_DOUBLEJUMP },
	{ IDS_CONTROL_DUCK,					COMMAND_ID_DUCK },
	{ IDS_CONTROL_NEXTWEAPON,			COMMAND_ID_NEXT_WEAPON },
	{ IDS_CONTROL_PREVIOUSWEAPON,		COMMAND_ID_PREV_WEAPON },
	{ IDS_CONTROL_LOOKUP,				COMMAND_ID_LOOKUP },
	{ IDS_CONTROL_LOOKDOWN,				COMMAND_ID_LOOKDOWN },
	{ IDS_CONTROL_CENTERVIEW,			COMMAND_ID_CENTERVIEW },
	{ IDS_CONTROL_CHASEVIEW,			COMMAND_ID_CHASEVIEWTOGGLE },
//	{ IDS_CONTROL_DROPUPGRADE,			COMMAND_ID_DROPUPGRADE },
	{ IDS_CONTROL_SHOWORDINANCE,		COMMAND_ID_SHOWORDINANCE },
	{ IDS_CONTROL_FRAGCOUNT,			COMMAND_ID_FRAGCOUNT },
	{ IDS_CONTROL_SAY,					COMMAND_ID_MESSAGE },
	{ IDS_CONTROL_TRACTORBEAM,			COMMAND_ID_SPECIAL_MOVE },
	{ IDS_CONTROL_VEHICLEMODETOGGLE,	COMMAND_ID_VEHICLETOGGLE },
	{ IDS_CONTROL_RUNLOCKTOGGLE,		COMMAND_ID_RUNLOCK },
	{ IDS_CONTROL_MOUSELOOKTOGGLE,		COMMAND_ID_MOUSEAIMTOGGLE },
	{ IDS_CONTROL_CROSSHAIRTOGGLE,		COMMAND_ID_CROSSHAIRTOGGLE },
	{ IDS_CONTROL_UNASSIGNED,			COMMAND_ID_UNASSIGNED },		// this control must always remain as the last one in the array
};

//-------------------------------------------------------------------------------------------
// CommandToArrayPos
//
// Returns the array position of the specified command
// Arguments:
//		nCommand - command number
// Return:
//		Array position of command in global command array
//-------------------------------------------------------------------------------------------
int CommandToArrayPos(int nCommand)
{
	switch (nCommand)
	{
		case COMMAND_ID_FORWARD:			return 0;
		case COMMAND_ID_REVERSE:			return 1;
		case COMMAND_ID_LEFT:				return 2;
		case COMMAND_ID_RIGHT:				return 3;
		case COMMAND_ID_STRAFE:				return 4;
		case COMMAND_ID_STRAFE_LEFT:		return 5;
		case COMMAND_ID_STRAFE_RIGHT:		return 6;
		case COMMAND_ID_TURNAROUND: 		return 7;
		case COMMAND_ID_RUN: 				return 8;
		case COMMAND_ID_FIRING: 			return 9;
//		case COMMAND_ID_ACTIVATE:			return 10;
		case COMMAND_ID_JUMP: 				return 10;
		case COMMAND_ID_DOUBLEJUMP:			return 11;
		case COMMAND_ID_DUCK: 				return 12;
		case COMMAND_ID_NEXT_WEAPON:		return 13;
		case COMMAND_ID_PREV_WEAPON:		return 14;
		case COMMAND_ID_LOOKUP:				return 15;
		case COMMAND_ID_LOOKDOWN:			return 16;
		case COMMAND_ID_CENTERVIEW:			return 17;
		case COMMAND_ID_CHASEVIEWTOGGLE:	return 18;
//		case COMMAND_ID_DROPUPGRADE:		return 19;
		case COMMAND_ID_SHOWORDINANCE:		return 19;
		case COMMAND_ID_FRAGCOUNT:			return 20;
		case COMMAND_ID_MESSAGE:			return 21;
		case COMMAND_ID_SPECIAL_MOVE:		return 22;
		case COMMAND_ID_VEHICLETOGGLE:		return 23;
		case COMMAND_ID_RUNLOCK:			return 24;
		case COMMAND_ID_MOUSEAIMTOGGLE:		return 25;
		case COMMAND_ID_CROSSHAIRTOGGLE:	return 26;
		case COMMAND_ID_UNASSIGNED:			return 27;
	}

	return 27;
}

//-------------------------------------------------------------------------------------------
// CommandName
//
// Retrieves the command name from a command number
// Arguments:
//		nCommand - command number
// Return:
//		String containing the name of the action
//-------------------------------------------------------------------------------------------
char* CommandName(int nCommand)
{
	static char buffer[128];

	SAFE_STRCPY(buffer, "error");
	DDWORD nStringID = 0;

	if (!g_pRiotClientShell || !g_pRiotClientShell->GetClientDE()) return buffer;
	
	switch (nCommand)
	{
		case COMMAND_ID_FORWARD:				nStringID = IDS_ACTIONSTRING_FORWARD;			 break;
		case COMMAND_ID_REVERSE:				nStringID = IDS_ACTIONSTRING_BACKWARD;           break;
		case COMMAND_ID_LEFT:					nStringID = IDS_ACTIONSTRING_TURNLEFT;           break;
		case COMMAND_ID_RIGHT:					nStringID = IDS_ACTIONSTRING_TURNRIGHT;          break;
		case COMMAND_ID_STRAFE:					nStringID = IDS_ACTIONSTRING_STRAFE;             break;
		case COMMAND_ID_STRAFE_LEFT:			nStringID = IDS_ACTIONSTRING_STRAFELEFT;         break;
		case COMMAND_ID_STRAFE_RIGHT:			nStringID = IDS_ACTIONSTRING_STRAFERIGHT;        break;
		case COMMAND_ID_TURNAROUND: 			nStringID = IDS_ACTIONSTRING_TURNAROUND;         break;
		case COMMAND_ID_RUN: 					nStringID = IDS_ACTIONSTRING_RUN;                break;
		case COMMAND_ID_FIRING: 				nStringID = IDS_ACTIONSTRING_FIRE;               break;
//		case COMMAND_ID_ACTIVATE:				nStringID = IDS_ACTIONSTRING_ACTIVATE;           break;
		case COMMAND_ID_JUMP: 					nStringID = IDS_ACTIONSTRING_JUMP;               break;
		case COMMAND_ID_DOUBLEJUMP:				nStringID = IDS_ACTIONSTRING_DOUBLEJUMP;		 break;
		case COMMAND_ID_DUCK: 					nStringID = IDS_ACTIONSTRING_DUCK;               break;
		case COMMAND_ID_NEXT_WEAPON:			nStringID = IDS_ACTIONSTRING_NEXTWEAPON;         break;
		case COMMAND_ID_PREV_WEAPON:			nStringID = IDS_ACTIONSTRING_PREVIOUSWEAPON;     break;
		case COMMAND_ID_LOOKUP:					nStringID = IDS_ACTIONSTRING_LOOKUP;             break;
		case COMMAND_ID_LOOKDOWN:				nStringID = IDS_ACTIONSTRING_LOOKDOWN;           break;
		case COMMAND_ID_CENTERVIEW:				nStringID = IDS_ACTIONSTRING_CENTERVIEW;         break;
		case COMMAND_ID_CHASEVIEWTOGGLE:		nStringID = IDS_ACTIONSTRING_CHASEVIEW;          break;
//		case COMMAND_ID_DROPUPGRADE:			nStringID = IDS_ACTIONSTRING_DROPUPGRADE;        break;
		case COMMAND_ID_SHOWORDINANCE:			nStringID = IDS_ACTIONSTRING_SHOWORDINANCE;		 break;
		case COMMAND_ID_FRAGCOUNT:				nStringID = IDS_ACTIONSTRING_FRAGCOUNT;			 break;
		case COMMAND_ID_MESSAGE:				nStringID = IDS_ACTIONSTRING_SAY;                break;
		case COMMAND_ID_SPECIAL_MOVE:			nStringID = IDS_ACTIONSTRING_TRACTORBEAM;		 break;
		case COMMAND_ID_VEHICLETOGGLE:			nStringID = IDS_ACTIONSTRING_VEHICLEMODETOGGLE;	 break;
		case COMMAND_ID_RUNLOCK:				nStringID = IDS_ACTIONSTRING_RUNLOCKTOGGLE;      break;
		case COMMAND_ID_MOUSEAIMTOGGLE:			nStringID = IDS_ACTIONSTRING_MOUSELOOKTOGGLE;    break;
		case COMMAND_ID_CROSSHAIRTOGGLE:		nStringID = IDS_ACTIONSTRING_CROSSHAIRTOGGLE;    break;
		case COMMAND_ID_UNASSIGNED:				nStringID = IDS_ACTIONSTRING_UNASSIGNED;		 break;
	}

	if (nStringID)
	{
		CClientDE* pClientDE = g_pRiotClientShell->GetClientDE();
		HSTRING hStr = pClientDE->FormatString (nStringID);
		SAFE_STRCPY(buffer, pClientDE->GetStringData (hStr));
		pClientDE->FreeString (hStr);
	}

	return buffer;
}

//-------------------------------------------------------------------------------------------
// PlaySoundFromObject
//
// Plays sound attached to object.
// Arguments:
//		hObject - Handle to object
//		pSoundName - path of sound file.
//		fRadius - max radius of sound.
//		nSoundPriority - sound priority
//		bLoop - Loop the sound (default: false)
//		bHandle - Return handle to sound (default: false)
//		bTime - Have server keep track of time (default: false)
//		nVolume - 0 - 100
// Return:
//		Handle to sound, if bHandle was set to TRUE.
//-------------------------------------------------------------------------------------------
HSOUNDDE PlaySoundFromObject(HOBJECT hObject, char *pSoundName, DFLOAT fRadius, DBYTE nSoundPriority, 
							 DBOOL bLoop, DBOOL bHandle, DBOOL bTime, DBYTE nVolume )
{
	if (!g_pRiotClientShell || !g_pRiotClientShell->GetClientDE() || !hObject) return DNULL;

	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_REVERB | PLAYSOUND_ATTACHED;

	if ( bLoop )
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	if ( bHandle )
		playSoundInfo.m_dwFlags |= PLAYSOUND_GETHANDLE;
	if ( bTime )
		playSoundInfo.m_dwFlags |= PLAYSOUND_TIME;
	if ( nVolume < 100 )
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;

	strncpy( playSoundInfo.m_szSoundName, pSoundName, _MAX_PATH );
	
	// TEMP - CAN'T SET OBJECT, so use object pos...
	// playSoundInfo.m_hObject = hObject;
	DVector vPos;
	g_pRiotClientShell->GetClientDE()->GetObjectPos(hObject, &vPos);
	VEC_COPY( playSoundInfo.m_vPosition, vPos );

	playSoundInfo.m_nPriority = nSoundPriority;
	playSoundInfo.m_fOuterRadius = fRadius;
	playSoundInfo.m_fInnerRadius = fRadius * 0.25f;
	playSoundInfo.m_nVolume = nVolume;
	g_pRiotClientShell->GetClientDE()->PlaySound( &playSoundInfo );

	return playSoundInfo.m_hSound;
}

//-------------------------------------------------------------------------------------------
// PlaySoundFromPos
//
// Plays sound at a position
// Arguments:
//		vPos - position of sound
//		pSoundName - path of sound file.
//		fRadius - max radius of sound.
//		nSoundPriority - sound priority
//		bLoop - Loop the sound (default: false)
//		bHandle - Return handle to sound (default: false)
//		bTime - Have server keep track of time (default: false)
//		nVolume - 0 - 100
// Return:
//		Handle to sound, if bHandle was set to TRUE.
//-------------------------------------------------------------------------------------------
HSOUNDDE PlaySoundFromPos(DVector *vPos, char *pSoundName, DFLOAT fRadius, DBYTE nSoundPriority, 
						  DBOOL bLoop, DBOOL bHandle, DBOOL bTime, DBYTE nVolume )
{
	if (!g_pRiotClientShell || !g_pRiotClientShell->GetClientDE()) return DNULL;

	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_REVERB;

	if ( bLoop )
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	if ( bHandle )
		playSoundInfo.m_dwFlags |= PLAYSOUND_GETHANDLE;
	if ( bTime )
		playSoundInfo.m_dwFlags |= PLAYSOUND_TIME;
	if ( nVolume < 100 )
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;

	strncpy( playSoundInfo.m_szSoundName, pSoundName, _MAX_PATH );
	VEC_COPY( playSoundInfo.m_vPosition, *vPos );
	playSoundInfo.m_nPriority = nSoundPriority;
	playSoundInfo.m_fOuterRadius = fRadius;
	playSoundInfo.m_fInnerRadius = fRadius * 0.25f;
	playSoundInfo.m_nVolume = nVolume;
	g_pRiotClientShell->GetClientDE()->PlaySound( &playSoundInfo );

	return playSoundInfo.m_hSound;
}

//-------------------------------------------------------------------------------------------
// PlaySoundLocal
//
// Plays sound inside player's head
// Arguments:
//		pSoundName - path of sound file.
//		nSoundPriority - sound priority
//		bLoop - Loop the sound (default: false)
//		bHandle - Return handle to sound (default: false)
//		nVolume - 0 - 100
//		bReverb - Add reverb
// Return:
//		Handle to sound, if bHandle was set to TRUE.
//-------------------------------------------------------------------------------------------
HSOUNDDE PlaySoundLocal( char *pSoundName, DBYTE nSoundPriority, DBOOL bLoop, DBOOL bHandle, DBYTE nVolume, DBOOL bReverb )
{
	if (!g_pRiotClientShell || !g_pRiotClientShell->GetClientDE()) return DNULL;

	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_LOCAL;
	
	if ( bLoop )
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	if ( bHandle )
		playSoundInfo.m_dwFlags |= PLAYSOUND_GETHANDLE;
	if ( nVolume < 100 )
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	if( bReverb )
		playSoundInfo.m_dwFlags |= PLAYSOUND_REVERB;

	strncpy( playSoundInfo.m_szSoundName, pSoundName, _MAX_PATH );
	playSoundInfo.m_nPriority = nSoundPriority;
	playSoundInfo.m_nVolume = nVolume;
	g_pRiotClientShell->GetClientDE()->PlaySound( &playSoundInfo );

	return playSoundInfo.m_hSound;
}

//-------------------------------------------------------------------------------------------
// CropSurface
//
// Crops the given surface to smallest possible area
// Arguments:
//		hSurf - surface to be cropped
//		hBorderColor - color of area to be cropped
// Return:
//		cropped surface if successful, original surface otherwise
//-------------------------------------------------------------------------------------------
HSURFACE CropSurface ( HSURFACE hSurf, HDECOLOR hBorderColor )
{
	if (!g_pRiotClientShell) return hSurf;

	if (!hSurf) return DNULL;
	
	CClientDE* pClientDE = g_pRiotClientShell->GetClientDE();
	if (!pClientDE) return hSurf;

	DDWORD nWidth, nHeight;
	pClientDE->GetSurfaceDims (hSurf, &nWidth, &nHeight);

	DRect rcBorders;
	memset (&rcBorders, 0, sizeof (DRect));
	pClientDE->GetBorderSize (hSurf, hBorderColor, &rcBorders);

	if (rcBorders.left == 0 && rcBorders.top == 0 && rcBorders.right == 0 && rcBorders.bottom == 0) return hSurf;

	HSURFACE hCropped = pClientDE->CreateSurface (nWidth - rcBorders.left - rcBorders.right, nHeight - rcBorders.top - rcBorders.bottom);
	if (!hCropped) return hSurf;

	DRect rcSrc;
	rcSrc.left = rcBorders.left;
	rcSrc.top = rcBorders.top;
	rcSrc.right = nWidth - rcBorders.right;
	rcSrc.bottom = nHeight - rcBorders.bottom;

	pClientDE->DrawSurfaceToSurface (hCropped, hSurf, &rcSrc, 0, 0);

	pClientDE->DeleteSurface (hSurf);

	return hCropped;
}

