#ifndef __SAVELEVELMENU_H
#define __SAVELEVELMENU_H

#include "BaseMenu.h"

#define MAXSAVELEVELS		(__min (MAX_GENERIC_ITEMS, 11))

class CSaveLevelMenu : public CBaseMenu
{
public:

	virtual DBOOL		Init (CClientDE* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight);
	virtual void		ScreenDimsChanged (int nScreenWidth, int nScreenHeight);
	virtual void		Reset()					{ m_bContentChanged = DFALSE; }

	virtual DBOOL		LoadAllSurfaces()		{ return LoadSurfaces(); }
	virtual void		UnloadAllSurfaces()		{ UnloadSurfaces(); }

	virtual void		Return();
	virtual void		Esc();

	virtual void		Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset = 0);

protected:

	virtual DBOOL		LoadSurfaces();
	virtual void		UnloadSurfaces();
	
	virtual void		PostCalculateMenuDims();

protected:

	DBOOL				m_bContentChanged;

	int					m_nSecondColumn;

	GENERIC_ITEM		m_DateTime[MAXSAVELEVELS];
};

#endif
