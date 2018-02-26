#include <Roster.h>
#include <Alert.h>
#include <String.h>
#include <TranslationUtils.h>
#include <TranslatorFormats.h>
#include <MenuItem.h>
#include <MenuBar.h>
#include <stdio.h>
#include <stdlib.h>
#include <Entry.h>
#include "AboutWindow.h"
#include "GridView.h"
#include "Preferences.h"
#include "PuzzlePack.h"

enum
{
	M_SHOW_MENU='msmn',
	M_RESET_LEVEL,
	M_CHOOSE_RANDOM,
	M_CHOOSE_PACK,
	M_CHOOSE_LEVEL,
	M_SOUND_ON,
	M_SOUND_OFF,
	M_SHOW_MANUAL
};

PuzzlePackSet gPuzzles;

static const float gridMargin = 50;
static const int8 minDimension = 3;
static const int8 maxDimension = 8;
static const int8 defaultDimension = 5;
static const int8 maxNumButtons = maxDimension * maxDimension;

/*
 * Maximum levels (number of moves required) for dimensions 3x3 through 8x8
 *
 * Note: although the nullity of 3x3 and 6x6 through 8x8 dimensions is 0, which
 * means the maximum level is n^2 where n = 3, 6, 7, or 8, I set them to 1 less
 * because otherwise, the solutions would be simply pressing all buttons. --Owen
 */
static const int8 maxLevels[] = { 8, 7, 15, 35, 48, 63 };

static int8 lastLevels[maxDimension - minDimension + 1];

static void
SuccessAlert()
{
	BAlert* alert = new BAlert("Lights Off", "Congratulations!", "OK");
	alert->Go();
}

GridView::GridView()
	:
	BView(BRect(0, 0, 260, 280), "gridview", B_FOLLOW_ALL, B_WILL_DRAW),
	fPuzzle(NULL)
{
	entry_ref ref;
	
	BEntry entry("click.wav");
	entry.GetRef(&ref);
	fClickSound = new BFileGameSound(&ref,false);
	
	entry.SetTo("fanfare.wav");
	entry.GetRef(&ref);
	fWinSound = new BFileGameSound(&ref,false);
	
	entry.SetTo("altwin.wav");
	entry.GetRef(&ref);
	fNoWinSound = new BFileGameSound(&ref,false);

	SetViewColor(0,0,50);
	
	BRect r(0,0,Bounds().Width(),20);
	BMenuBar *bar = new BMenuBar(r,"menubar");
	AddChild(bar);
	
	fMenu = new BMenu("Game");
	bar->AddItem(fMenu);
	
	fMenu->AddItem(new BMenuItem("Restart level",new BMessage(M_RESET_LEVEL),'R'));
	fMenu->AddSeparatorItem();
	
	fSoundMenu = new BMenu("Sound");
	fMenu->AddItem(fSoundMenu);
	fSoundMenu->AddItem(new BMenuItem("On",new BMessage(M_SOUND_ON)));
	fSoundMenu->AddItem(new BMenuItem("Off",new BMessage(M_SOUND_OFF)));
	fSoundMenu->SetRadioMode(true);
	if(fUseSound)
		fSoundMenu->ItemAt(0)->SetMarked(true);
	else
		fSoundMenu->ItemAt(1)->SetMarked(true);
	
	fMenu->AddSeparatorItem();
	fMenu->AddItem(new BMenuItem("How to play",new BMessage(M_SHOW_MANUAL)));
	
	fMenu->AddSeparatorItem();
	fMenu->AddItem(new BMenuItem("About",new BMessage(B_ABOUT_REQUESTED)));

	fPackMenu = new BMenu("Puzzle pack");
	fPackMenu->SetRadioMode(true);
	bar->AddItem(fPackMenu);

	RandomMenu();

	for (int8 i = 0; i < gPuzzles.CountPacks(); i++) {
		PuzzlePack *pack = gPuzzles.PackAt(i);
		
		BMessage *msg = new BMessage(M_CHOOSE_PACK);
		msg->AddInt8("index", i);
		fPackMenu->AddItem(new BMenuItem(pack->Name(),msg));
	}

	fLevelMenu = new BMenu("Level");
	fLevelMenu->SetRadioMode(true);
	bar->AddItem(fLevelMenu);

	StartupPreferences();
	fGrid = new Grid(fDimension);
	srandom(system_time());

	fButtons = new TwoStateDrawButton*[maxNumButtons];

	for (int8 index = 0; index < maxNumButtons; index++) {
		BBitmap* off_up = BTranslationUtils::GetBitmap(B_PNG_FORMAT, 1);
		BBitmap* off_down = BTranslationUtils::GetBitmap(B_PNG_FORMAT, 2);
		BBitmap* on_up = BTranslationUtils::GetBitmap(B_PNG_FORMAT, 3);
		BBitmap* on_down = BTranslationUtils::GetBitmap(B_PNG_FORMAT, 4);

		BString name = "button ";
		name << index;
		fButtons[index] = new TwoStateDrawButton(r,name.String(), off_up,
			off_down, on_up, on_down, new BMessage(1000 + index));
		fButtons[index]->ResizeToPreferred();
		AddChild(fButtons[index]);
	}

	const float buttonSize = fButtons[0]->Bounds().Width();
	const float gridTop = bar->Frame().bottom + gridMargin;
	r.Set(gridMargin, gridTop, gridMargin + buttonSize, gridTop + buttonSize);
	UpdateGrid(r, defaultDimension);

	r.left = 10;
	r.top = bar->Frame().bottom + 10;
	r.right = 11;
	r.bottom = r.top + 1;
	fLevelLabel = new BStringView(r,"levellabel","Level: 1");
	fLevelLabel->ResizeToPreferred();
	AddChild(fLevelLabel);
	fLevelLabel->SetHighColor(255,255,255);

	fMovesLabel = new BStringView(BRect(0, 0, 1, 1), "moveslabel", "Moves: 10",
		B_FOLLOW_RIGHT);
	fMovesLabel->ResizeToPreferred();
	fMovesLabel->MoveTo(Bounds().right - 10 - fMovesLabel->Frame().Width(),
			bar->Frame().bottom + 10);
	fMovesLabel->SetText("Moves: 0");
	AddChild(fMovesLabel);
	fMovesLabel->SetHighColor(255,255,255);

	if (fPuzzle)
		SetPack(fPuzzle);
	else
		SetRandom(fDimension);
}

GridView::~GridView()
{
	ShutdownPreferences();

	delete fClickSound;
	delete fWinSound;
	delete fNoWinSound;
	delete fGrid;

	delete[] fButtons;
}

void GridView::AttachedToWindow()
{
	const float delta
		= fButtons[0]->Bounds().Width() * (fDimension - defaultDimension);
	Window()->ResizeBy(delta, delta);

	for (int8 index = 0; index < maxNumButtons; index++)
		fButtons[index]->SetTarget(this);

	fMenu->SetTargetForItems(this);
	fSoundMenu->SetTargetForItems(this);
	fPackMenu->SetTargetForItems(this);
	fRandomMenu->SetTargetForItems(this);
	fLevelMenu->SetTargetForItems(this);
}

void GridView::MessageReceived(BMessage *msg)
{
	const int8 index = msg->what - 1000;
	const int8 n = fDimension;	// n by n grid

	if (index >= 0 && index < n * n) {
		if (index % n)	// not leftmost column
			FlipButton(index - 1);	// left neighbor

		if ((index + 1) % n)	// not rightmost column
			FlipButton(index + 1);	// right neighbor

		if (index >= n)	// not top row
			FlipButton(index - n);	// neighbor above

		if (index < n * (n - 1))	// not bottom row
			FlipButton(index + n);	// neighbor below

		FlipButton(index);
		fMoveCount++;
		SetMovesLabel(fMoveCount);

		if (fUseSound)
			fClickSound->StartPlaying();
	}

	if (fGrid->GetGridValues() == 0)
		HandleFinish();

	switch(msg->what)
	{
		case M_SHOW_MANUAL:
		{
			entry_ref ref;
			BEntry entry("GettingStarted.htm");
			entry.GetRef(&ref);
			
			be_roster->Launch(&ref);
			break;
		}
		case M_SOUND_ON:
		{
			fUseSound = true;
			break;
		}
		case M_SOUND_OFF:
		{
			fUseSound = false;
			break;
		}
		case M_RESET_LEVEL:
		{
			SetLevel(fLevel);
			break;
		}
		case B_ABOUT_REQUESTED:
		{
			AboutWindow *abwin = new AboutWindow();
			abwin->Show();
			break;
		}
		case M_CHOOSE_RANDOM:
		{
			int8 dimension;
			if (msg->FindInt8("dimension", &dimension) != B_OK) {
				int8 index = fRandomMenu->FindMarkedIndex();
				dimension = index < 0 ? defaultDimension : minDimension + index;
			}
			UpdateDimension(dimension);
			SetRandom(dimension);
			break;
		}
		case M_CHOOSE_PACK:
		{
			int8 index;
			if (msg->FindInt8("index", &index) == B_OK) {
				UpdateDimension(defaultDimension);
				SetPack(gPuzzles.PackAt(index));
			}
			break;
		}
		case M_CHOOSE_LEVEL:
		{
			int8 level;
			if (msg->FindInt8("level", &level) == B_OK)
				SetLevel(level);
			break;
		}
		default:
			BView::MessageReceived(msg);
	}
}

void GridView::RandomMenu()
{
	fRandomMenu = new BMenu("Random");
	fRandomMenu->SetRadioMode(true);
	fPackMenu->AddItem(fRandomMenu);

	BMenuItem* superitem = fRandomMenu->Superitem();
//	superitem->SetShortcut('R', B_COMMAND_KEY);
	superitem->SetMessage(new BMessage(M_CHOOSE_RANDOM));

	for (int8 dimension = minDimension;
		dimension <= maxDimension; dimension++) {
		char label[4];
		BMessage* msg = new BMessage(M_CHOOSE_RANDOM);

		sprintf(label, "%dx%d", dimension, dimension);
		msg->AddInt8("dimension", dimension);
		fRandomMenu->AddItem(new BMenuItem(label, msg, '0' + dimension));
	}
}

void GridView::FlipButton(int8 offset)
{
	if(fButtons[offset]->GetState())
	{
		fButtons[offset]->SetState(0);
		fGrid->SetValue(offset, false);
	}
	else
	{
		fButtons[offset]->SetState(1);
		fGrid->SetValue(offset, true);
	}
}

void GridView::SetRandom(int8 dimension)
{
	fPuzzle = NULL;

	for (int8 index = fLevelMenu->CountItems() - 1; index >= 0; index--)
		delete fLevelMenu->RemoveItem(index);

	for (int8 index = 0; index < maxLevels[dimension - minDimension]; index++) {
		BMessage* msg = new BMessage(M_CHOOSE_LEVEL);
		msg->AddInt8("level", index);

		char levelname[16];
		sprintf(levelname, "Level %d", index + 1);

		BMenuItem* item = new BMenuItem(levelname, msg);
		fLevelMenu->AddItem(item);
	}

	fLevelMenu->SetTargetForItems(this);
	fPackMenu->ItemAt(0)->SetMarked(true);
	SetLevel(lastLevels[dimension - minDimension]);
}

void GridView::UpdateDimension(int8 dimension)
{
	if (fDimension == dimension)
		return;

	const int8 oldDimension = fDimension;
	fDimension = dimension;
	UpdateGrid(fButtons[0]->Frame(), oldDimension);
	fGrid->SetDimension(fDimension);
}

void GridView::UpdateGrid(BRect rect, int8 oldDimension)
{
	int8 index = 0;

	for (int8 row = 0; row < fDimension; row++) {
		for (int8 column = 0; column < fDimension; column++) {
			BButton* button = fButtons[index++];

			button->MoveTo(rect.LeftTop());
			rect.OffsetBy(rect.Width() + 1, 0);

			if (button->IsHidden())
				button->Show();
		}

		rect.OffsetTo(gridMargin, rect.bottom + 1);
	}

	while (index < maxNumButtons) {
		BButton* button = fButtons[index++];

		if (!button->IsHidden())
			button->Hide();
	}

	const float delta = rect.Width() * (fDimension - oldDimension);
	Window()->ResizeBy(delta, delta);
}

void GridView::SetPack(PuzzlePack *pack)
{
	fPuzzle = pack;
	
	for(int8 i = fLevelMenu->CountItems() - 1; i >= 0; i--) {
		BMenuItem *old = fLevelMenu->RemoveItem(i);
		delete old;
	}
	
	for (int8 index = 0; index < fPuzzle->Size(); index++) {
		BMessage *msg = new BMessage(M_CHOOSE_LEVEL);
		msg->AddInt8("level", index);
		
		char levelname[30];
		sprintf(levelname, "Level %d", index + 1);
		
		BMenuItem *item = new BMenuItem(levelname,msg);
		fLevelMenu->AddItem(item);
		if(index>fPuzzle->Highest())
			item->SetEnabled(false);
	}

	fLevelMenu->SetTargetForItems(this);

	for(int8 i = 0; i < fPackMenu->CountItems(); i++) {
		BMenuItem *packitem = fPackMenu->ItemAt(i);
		if(strcmp(packitem->Label(),pack->Name())==0)
		{
			packitem->SetMarked(true);
			break;
		}
	}
	
	SetLevel(fPuzzle->Highest());
}

void GridView::SetLevel(int8 level)
{
	fLevel = level;
	fMoveCount = 0;
	SetMovesLabel(fMoveCount);
	
	char label[16];
	sprintf(label, "Level: %d", fLevel + 1);
	fLevelLabel->SetText(label);
	fLevelLabel->ResizeToPreferred();

	if (fPuzzle)
		fGrid->SetGridValues(fPuzzle->ValueAt(level));
	else {
		lastLevels[fDimension - minDimension] = level;
		fGrid->Random(level + 1);
	}

	UpdateButtons();

	BMenuItem *current = fLevelMenu->ItemAt(level);
	current->SetMarked(true);
}

void GridView::UpdateButtons()
{
	for (int8 index = 0; index < fDimension * fDimension; index++)
		fButtons[index]->SetState(fGrid->ValueAt(index));
}

void GridView::SetMovesLabel(int8 count)
{
	BString string("Moves: ");
	string << count;
	fMovesLabel->SetText(string.String());
}

void GridView::HandleFinish()
{
	if (fPuzzle == NULL) {
		SuccessAlert();
		SetLevel(fLevel);
		return;
	}

	// Determine whether or not the user finished in the required number of
	// moves
	int8 movesreq = fPuzzle->MovesRequired(fLevel);
	if(fMoveCount > (movesreq+10))
	{
		if(fUseSound)
			fNoWinSound->StartPlaying();
		
		BString msg("Great! You solved the puzzle but not within the maximum number ");
		msg << "of moves. The puzzle requires " << movesreq
			<< " moves, but you can make up to " << (movesreq+10)
			<< " to win.";
		BAlert *bummer = new BAlert("Lights Off",msg.String(),"OK");
		bummer->Go();
		SetLevel(fLevel);
		return;
	}
	
	if(fUseSound)
		fWinSound->StartPlaying();
	
	// we got this far, so the user must have won. Congratulate him/her and
	// advance the level if there are any more in the current puzzle pack
	SuccessAlert();
	SetLevel(fLevel+1);
	
	// SetLevel() increments fLevel for us
	if(fPuzzle->Highest()==fLevel-1)
		fPuzzle->SetHighest(fLevel);
	
	BMenuItem *item = fLevelMenu->ItemAt(fLevel);
	if(item && !item->IsEnabled())
		item->SetEnabled(true);
}

void GridView::StartupPreferences()
{
	if (LoadPreferences(PREFERENCES_PATH) == B_OK) {
		for (int8 index = 0; index <= maxDimension - minDimension; index++)
			lastLevels[index] = preferences.GetInt8("levels", index, index + 1);

		for(int8 index = 0; index < gPuzzles.CountPacks(); index++) {
			PuzzlePack* pack = gPuzzles.PackAt(index);
			pack->SetHighest(preferences.GetInt8(pack->Name(), 0));
		}

		BString lastpack;

		if (preferences.FindString("lastpack", &lastpack) == B_OK)
			for(int8 index = 0; index < gPuzzles.CountPacks(); index++) {
				PuzzlePack* pack = gPuzzles.PackAt(index);

				if (strcmp(lastpack.String(), pack->Name()) == 0) {
					fPuzzle = pack;
					fDimension = defaultDimension;
					break;
				}
			}

		int8 dimension = preferences.GetInt8("dimension", defaultDimension);
		fRandomMenu->ItemAt(dimension - minDimension)->SetMarked(true);

		if (fPuzzle == NULL)
			fDimension = dimension;

		fUseSound = preferences.GetBool("usesound", true);
	} else {
		for (int8 index = 0; index <= maxDimension - minDimension; index++)
			lastLevels[index] = index + 1;

		fPuzzle = gPuzzles.PackAt(0);
		fDimension = defaultDimension;
		fRandomMenu->ItemAt(defaultDimension - minDimension)->SetMarked(true);
		fUseSound = true;
	}

	if (fUseSound) {
		fClickSound->Preload();
		fWinSound->Preload();
		fNoWinSound->Preload();
	}
}

void GridView::ShutdownPreferences()
{
	preferences.MakeEmpty();
	
	for (int8 index = 0; index <= maxDimension - minDimension; index++)
		preferences.AddInt8("levels", lastLevels[index]);

	// Save the progress in each of the puzzle packs
	for (int8 i = 0; i < gPuzzles.CountPacks(); i++) {
		PuzzlePack *pack = (PuzzlePack*)gPuzzles.PackAt(i);

		if(pack) {
			preferences.AddString("name",pack->Name());
			preferences.AddInt8(pack->Name(), pack->Highest());
		}
	}

	int8 index = fRandomMenu->FindMarkedIndex();
	preferences.AddInt8("dimension",
		index < 0 ? defaultDimension : minDimension + index);

	if (fPuzzle)
		preferences.AddString("lastpack", fPuzzle->Name());

	preferences.AddBool("usesound",fUseSound);
	SavePreferences(PREFERENCES_PATH);
}
