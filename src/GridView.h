#ifndef GRID_VIEW_H
#define GRID_VIEW_H

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
	GridView();
	~GridView();
	void MessageReceived(BMessage *msg);
	void AttachedToWindow();
	void StartupPreferences();
	void ShutdownPreferences();

private:
	void FlipButton(int8 offset);
	void UpdateButtons();
	void SetDimension(int8 dimension);
	void SetLevel(int8 level);
	void SetPack(PuzzlePack *pack);
	void SetMovesLabel(int8 count);
	void HandleFinish();

	TwoStateDrawButton **fButtons;
	BMenu *fMenu, *fSoundMenu, *fRandomMenu, *fPackMenu, *fLevelMenu;
	BStringView *fLevelLabel, *fMovesLabel;

	Grid *fGrid;
	PuzzlePack *fPuzzle;

	bool fUseSound;
	int8 fDimension, fLevel, fMoveCount;

	BFileGameSound *fClickSound, *fWinSound, *fNoWinSound;
};

#endif
