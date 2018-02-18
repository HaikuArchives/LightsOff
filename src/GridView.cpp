#include <Roster.h>
#include <Alert.h>
#include <String.h>
#include <TranslationUtils.h>
#include <TranslatorFormats.h>
#include <MenuItem.h>
#include <MenuBar.h>
#include <stdio.h>
#include <malloc.h>
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

static const int8 minDimension = 5;	// set to 3 when ready
static const int8 maxDimension = 5;	// set to 6 when ready
static const int8 defaultDimension = 5;
//static const int8 maxLevels[maxDimension - minDimension + 1] = { 4, 7, 5, 18 };
static const int8 maxLevels[1] = { 5 };

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
	
	StartupPreferences();
	fGrid = new Grid(fDimension);
	srandom(system_time());

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

	fRandomMenu = new BMenu("Random");

	for (int8 dimension = minDimension;
		dimension <= maxDimension; dimension++) {
		char label[4];
		BMessage *msg = new BMessage(M_CHOOSE_RANDOM);

		sprintf(label, "%dx%d", dimension, dimension);
		msg->AddInt8("dimension", dimension);
		fRandomMenu->AddItem(new BMenuItem(label, msg));
	}

	fRandomMenu->SetRadioMode(true);
	fRandomMenu->ItemAt(defaultDimension - minDimension)->SetMarked(true);

	fPackMenu = new BMenu("Puzzle pack");
	fPackMenu->AddItem(fRandomMenu);

	BMenuItem* superitem = fRandomMenu->Superitem();
//	superitem->SetShortcut('R', B_COMMAND_KEY);
	superitem->SetMessage(new BMessage(M_CHOOSE_RANDOM));

	for (int8 i = 0; i < gPuzzles.CountPacks(); i++) {
		PuzzlePack *pack = gPuzzles.PackAt(i);
		
		BMessage *msg = new BMessage(M_CHOOSE_PACK);
		msg->AddInt8("index", i);
		fPackMenu->AddItem(new BMenuItem(pack->Name(),msg));
	}
	
	fPackMenu->SetRadioMode(true);
	bar->AddItem(fPackMenu);

	fLevelMenu = new BMenu("Level");
	fLevelMenu->SetRadioMode(true);
	bar->AddItem(fLevelMenu);

	fButtons = (TwoStateDrawButton**)
		malloc(sizeof(TwoStateDrawButton) * fDimension * fDimension);
	float inset = 50;
	r.Set(inset,inset + bar->Frame().bottom,inset+32,inset + bar->Frame().bottom + 32);
	
	int8 count=0;
	for (int8 j = 0; j < fDimension; j++) {
		for (int8 i = 0; i < fDimension; i++) {
			BBitmap *off_up = BTranslationUtils::GetBitmap(B_PNG_FORMAT,1);
			BBitmap *off_down = BTranslationUtils::GetBitmap(B_PNG_FORMAT,2);
			BBitmap *on_up = BTranslationUtils::GetBitmap(B_PNG_FORMAT,3);
			BBitmap *on_down = BTranslationUtils::GetBitmap(B_PNG_FORMAT,4);
			
			BString name = "button ";
			name << count;
			fButtons[count] = new TwoStateDrawButton(r,name.String(),off_up,
							off_down, on_up,on_down,new BMessage(1000+count),
							B_FOLLOW_NONE,B_WILL_DRAW);
			AddChild(fButtons[count]);
			count++;
			r.OffsetBy(r.Width(),0);
		}
		r.OffsetTo(inset,r.bottom+1);
	}

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
		SetDimension(fDimension);
}

GridView::~GridView()
{
	delete fClickSound;
	delete fWinSound;
	delete fNoWinSound;
	delete fGrid;
	
	ShutdownPreferences();
	free(fButtons);
}

void GridView::AttachedToWindow()
{
	const float delta
		= fButtons[0]->Bounds().Width() * (fDimension - defaultDimension);
	Window()->ResizeBy(delta, delta);

	for (int8 i = 0; i < fDimension * fDimension; i++)
		fButtons[i]->SetTarget(this);
	
	fMenu->SetTargetForItems(this);
	fSoundMenu->SetTargetForItems(this);
	fRandomMenu->SetTargetForItems(this);
	fPackMenu->SetTargetForItems(this);
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
			int8 dimension = msg->GetInt8("dimension", defaultDimension);
			if (fDimension != dimension) {
				fDimension = dimension;
				fGrid->SetDimension(dimension);
			}
			SetDimension(dimension);
			break;
		}
		case M_CHOOSE_PACK:
		{
			int8 index;
			if (msg->FindInt8("index", &index) == B_OK)
				SetPack(gPuzzles.PackAt(index));
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

void GridView::SetDimension(int8 dimension)
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
	SetLevel(2);
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
	else
		fGrid->Random(level + 1);

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
					break;
				}
			}

		fDimension = preferences.GetInt8("dimension", defaultDimension);
		fUseSound = preferences.GetBool("usesound", true);
	} else {
		fPuzzle = gPuzzles.PackAt(0);
		fDimension = defaultDimension;
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
	
	// Save the progress in each of the puzzle packs
	for (int8 i = 0; i < gPuzzles.CountPacks(); i++) {
		PuzzlePack *pack = (PuzzlePack*)gPuzzles.PackAt(i);
		if(pack)
		{
			preferences.AddString("name",pack->Name());
			preferences.AddInt8(pack->Name(), pack->Highest());
		}
	}

	if (fPuzzle)
		preferences.AddString("lastpack", fPuzzle->Name());

	preferences.AddInt8("dimension", fDimension);
	preferences.AddBool("usesound",fUseSound);
	
	SavePreferences(PREFERENCES_PATH);
}
