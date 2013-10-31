#ifndef GRIDVIEW_H
#define GRIDVIEW_H

#include <View.h>
#include <Message.h>
#include <Menu.h>
#include <StringView.h>
#include <List.h>
#include <FileGameSound.h>

#include "TwoStateDrawButton.h"
#include "DrawButton.h"
#include "Grid.h"
#include "PuzzlePack.h"

class GridView : public BView
{
public:
	GridView(void);
	~GridView(void);
	void MessageReceived(BMessage *msg);
	void AttachedToWindow(void);
	void StartupPreferences(void);
	void ShutdownPreferences(void);
private:
	void FlipButton(uint8 offset);
	void SetLevel(uint8 level);
	void SetPack(PuzzlePack *pack);
	void SetMovesLabel(uint32 count);
	void HandleFinish(void);
	
	TwoStateDrawButton **fButtons;
	BMenu *fMenu,*fLevelMenu, *fPackMenu, *fSoundMenu;
	BStringView *fLevelLabel;
	BStringView *fMovesLabel;
	
	Grid fGrid;
	PuzzlePack *fPuzzle;
	
	uint32 fLevel;
	bool fUseSound;
	uint32 fMoveCount;
	
	BFileGameSound *fClickSound;
	BFileGameSound *fWinSound;
	BFileGameSound *fNoWinSound;
};

#endif
