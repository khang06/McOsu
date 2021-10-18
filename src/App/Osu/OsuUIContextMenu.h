//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		context menu, dropdown style
//
// $NoKeywords: $
//===============================================================================//

#ifndef OSUUICONTEXTMENU_H
#define OSUUICONTEXTMENU_H

#include "CBaseUIButton.h"

class CBaseUIContainer;
class CBaseUIScrollView;

class Osu;

class OsuUIContextMenuButton;

class OsuUIContextMenu : public CBaseUIElement
{
public:
	static void clampToBottomScreenEdge(OsuUIContextMenu *menu);
	static void clampToRightScreenEdge(OsuUIContextMenu *menu);

public:
	OsuUIContextMenu(Osu *osu, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0, UString name = "", CBaseUIScrollView *parent = NULL);
	virtual ~OsuUIContextMenu();

	virtual void draw(Graphics *g);
	virtual void update();

	typedef fastdelegate::FastDelegate2<UString, int> ButtonClickCallback;
	void setClickCallback(ButtonClickCallback clickCallback) {m_clickCallback = clickCallback;}

	void begin(int minWidth = 0, bool bigStyle = false);
	OsuUIContextMenuButton *addButton(UString text, int id = -1);
	void end(bool invertAnimation, bool clampUnderflowAndOverflowAndEnableScrollingIfNecessary);

	void setVisible2(bool visible2);

	virtual bool isVisible() {return m_bVisible && m_bVisible2;}

private:
	virtual void onResized();
	virtual void onMoved();
	virtual void onMouseDownOutside();
	virtual void onFocusStolen();

	void onClick(CBaseUIButton *button);

	Osu *m_osu;

	CBaseUIScrollView *m_container;
	CBaseUIScrollView *m_parent;

	ButtonClickCallback m_clickCallback;

	int m_iYCounter;
	int m_iWidthCounter;

	bool m_bVisible2;
	float m_fAnimation;
	bool m_bInvertAnimation;

	bool m_bBigStyle;
	bool m_bClampUnderflowAndOverflowAndEnableScrollingIfNecessary;
};

class OsuUIContextMenuButton : public CBaseUIButton
{
public:
	OsuUIContextMenuButton(Osu *osu, float xPos, float yPos, float xSize, float ySize, UString name, UString text, int id);
	virtual ~OsuUIContextMenuButton() {;}

	virtual void update();

	inline int getID() const {return m_iID;}

	void setTooltipText(UString text);

private:
	Osu *m_osu;

	int m_iID;

	std::vector<UString> m_tooltipTextLines;
};

#endif
