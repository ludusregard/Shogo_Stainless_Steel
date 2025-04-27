#ifndef __KEYBOARDMENU_H
#define __KEYBOARDMENU_H

#include "BaseMenu.h"

#define TRACK_BUFFER_SIZE	8

struct MenuEntry
{
	MenuEntry()	{ nStringID = 0; nAction = 0; memset(strControlName, 0, 64); 
				  hSurface = NULL; hSurfaceSelected = DNULL; szSurface.cx = 0; szSurface.cy = 0; 
				  hSetting = NULL; hSettingSelected = DNULL; }

	DDWORD		nStringID;
	int			nAction;
	char		strControlName[64];

	HSURFACE	hSurface;
	HSURFACE	hSurfaceSelected;
	CSize		szSurface;

	HSURFACE	hSetting;
	HSURFACE	hSettingSelected;
};

class CKeyboardMenu : public CBaseMenu
{
public:

	CKeyboardMenu()		{ m_bWaitingForKeypress = DFALSE; m_fInputPauseTimeLeft = 0.0f; memset (m_pInputArray, 0, sizeof(DeviceInput) * TRACK_BUFFER_SIZE);
						  m_nSecondColumn = 0; m_nSpacing = 0; m_nTopItem = 0; m_nEntries = 0;	}
	~CKeyboardMenu();

	virtual DBOOL		Init (CClientDE* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight);
	virtual void		ScreenDimsChanged (int nScreenWidth, int nScreenHeight);
	virtual void		Reset()		{ CBaseMenu::Reset(); m_nTopItem = 0; }
	
	virtual DBOOL		LoadAllSurfaces()		{ return LoadSurfaces(); }
	virtual void		UnloadAllSurfaces()		{ UnloadSurfaces(); }
	
	virtual void		HandleInput (int vKey);
	virtual void		Up();
	virtual void		Down();
	virtual void		Left()			{}
	virtual void		Right()			{}
	virtual void		PageUp();
	virtual void		PageDown();
	virtual void		Home();
	virtual void		End();
	virtual void		Return();
	virtual void		Esc()			{ if (m_bWaitingForKeypress || m_fInputPauseTimeLeft) return; CBaseMenu::Esc(); }

	virtual void		Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset = 0);

protected:

	virtual DBOOL		LoadSurfaces();
	virtual void		UnloadSurfaces();

	virtual void		PostCalculateMenuDims();

	virtual void		CheckSelectionOffMenuTop();
	virtual void		CheckSelectionOffMenuBottom();
	
	DBOOL				SetCurrentSelection (DeviceInput* pInput);
	void				ClearKBBindings();
	DBOOL				KeyRemappable (DeviceInput* pInput);

protected:

	DBOOL				m_bWaitingForKeypress;
	DFLOAT				m_fInputPauseTimeLeft;
	DeviceInput			m_pInputArray[TRACK_BUFFER_SIZE];

	int					m_nSecondColumn;
	int					m_nSpacing;

	int					m_nEntries;
	MenuEntry			m_pEntries[NUM_COMMANDS + 1];

	GENERIC_ITEM		m_RestoreDefaults;
	GENERIC_ITEM		m_Back;
};

#endif
