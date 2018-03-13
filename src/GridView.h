#ifndef GRID_VIEW_H
#define GRID_VIEW_H

#include <FileGameSound.h>
#include <Menu.h>
#include <StringView.h>

#include "Grid.h"
#include "PuzzlePack.h"
#include "TwoStateDrawButton.h"

class GridView : public BView
{
public:
	GridView();
	~GridView();
	void MessageReceived(BMessage *msg);
	void KeyDown(const char* bytes, int32 numBytes);
	void AttachedToWindow();
	void StartupPreferences();
	void ShutdownPreferences();

private:
	void RandomMenu();
	void PressButton(int8 index);
	void FlipButton(int8 offset);
	void UpdateButtons();
	void UpdateGrid(BRect rect, int8 oldDimension);
	void UpdateDimension(int8 dimension);
	void SetLevel(int8 level);
	void SetRandom(int8 dimension);
	void SetPack(PuzzlePack *pack);
	void SetMovesLabel(int8 count);
	void HandleFinish();
	void Success();
	void LoadSoundFiles();
	void Restart();
	void Undo();
	void Redo();
	void Restore();

	TwoStateDrawButton **fButtons;
	BMenu *fMenu, *fSoundMenu, *fRandomMenu, *fPackMenu, *fLevelMenu;
	BStringView *fLevelLabel, *fMovesLabel;

	Grid *fGrid;
	PuzzlePack *fPuzzle;

	bool fUseSound;
	int8 fDimension, fLevel, fMoveCount, fCurrentCount;
	uint64 fPuzzleValues, fGridValues;
	std::vector<int8> fMoves;

	BFileGameSound *fClickSound, *fWinSound, *fNoWinSound;
};

#endif
