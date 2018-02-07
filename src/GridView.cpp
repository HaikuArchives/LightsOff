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
	M_CHOOSE_PACK,
	M_CHOOSE_LEVEL,
	M_SOUND_ON,
	M_SOUND_OFF,
	M_SHOW_MANUAL
};

PuzzlePackSet gPuzzles;

GridView::GridView(void)
 : BView(BRect(0,0,260,280), "gridview", B_FOLLOW_NONE, B_WILL_DRAW)
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
	
	fLevelMenu = new BMenu("Level");
	fLevelMenu->SetRadioMode(true);
	bar->AddItem(fLevelMenu);
	
	fPackMenu = new BMenu("Puzzle pack");
	for(int32 i=0; i<gPuzzles.CountPacks(); i++)
	{
		PuzzlePack *pack = gPuzzles.PackAt(i);
		
		BMessage *msg = new BMessage(M_CHOOSE_PACK);
		msg->AddInt32("index",i);
		fPackMenu->AddItem(new BMenuItem(pack->Name(),msg));
	}
	
	fPackMenu->SetRadioMode(true);
	bar->AddItem(fPackMenu);
	
	fButtons = (TwoStateDrawButton**) malloc(sizeof(TwoStateDrawButton)*25);
	float inset = 50;
	r.Set(inset,inset + bar->Frame().bottom,inset+32,inset + bar->Frame().bottom + 32);
	
	int8 count=0;
	for(int32 j=0; j<5; j++)
	{
		for(int32 i=0; i<5;i++)
		{
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
	
	fMovesLabel = new BStringView(BRect(0,0,1,1),"moveslabel","Moves: 1000");
	fMovesLabel->ResizeToPreferred();
	fMovesLabel->MoveTo(Bounds().right - 10 - fMovesLabel->Frame().Width(),
			bar->Frame().bottom + 10);
	fMovesLabel->SetText("Moves: 0");
	AddChild(fMovesLabel);
	fMovesLabel->SetHighColor(255,255,255);
	
	SetPack(fPuzzle);
}

GridView::~GridView(void)
{
	delete fClickSound;
	delete fWinSound;
	delete fNoWinSound;
	
	ShutdownPreferences();
	free(fButtons);
}

void GridView::AttachedToWindow(void)
{
	for(int32 i=0; i<25; i++)
		fButtons[i]->SetTarget(this);
	
	fMenu->SetTargetForItems(this);
	fLevelMenu->SetTargetForItems(this);
	fPackMenu->SetTargetForItems(this);
	fSoundMenu->SetTargetForItems(this);
}

void GridView::MessageReceived(BMessage *msg)
{
	const int32 index = msg->what - 1000;
	const int32 n = 5;	// n by n grid

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

	if (fGrid.GetGridValues() == 0)
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
		case M_CHOOSE_PACK:
		{
			int32 index;
			if(msg->FindInt32("index",&index)==B_OK)
				SetPack(gPuzzles.PackAt(index));
			break;
		}
		case M_CHOOSE_LEVEL:
		{
			uint8 level;
			if(msg->FindInt8("level",(int8*)&level)==B_OK)
				SetLevel(level);
			break;
		}
		default:
			BView::MessageReceived(msg);
	}
}


void GridView::FlipButton(uint8 offset)
{
	if(fButtons[offset]->GetState())
	{
		fButtons[offset]->SetState(0);
		fGrid.SetValue(offset,false);
	}
	else
	{
		fButtons[offset]->SetState(1);
		fGrid.SetValue(offset,true);
	}
}

void GridView::SetPack(PuzzlePack *pack)
{
	if(!pack)
		return;
	
	fPuzzle = pack;
	
	for(int32 i=fLevelMenu->CountItems()-1; i>=0; i--)
	{
		BMenuItem *old = fLevelMenu->RemoveItem(i);
		delete old;
	}
	
	for(uint32 index=0; index<fPuzzle->Size(); index++)
	{
		BMessage *msg = new BMessage(M_CHOOSE_LEVEL);
		msg->AddInt8("level",index);
		
		char levelname[30];
		sprintf(levelname,"Level %lu",index+1);
		
		BMenuItem *item = new BMenuItem(levelname,msg);
		fLevelMenu->AddItem(item);
		if(index>fPuzzle->Highest())
			item->SetEnabled(false);
	}
	
	for(int32 i=0; i<fPackMenu->CountItems(); i++)
	{
		BMenuItem *packitem = fPackMenu->ItemAt(i);
		if(strcmp(packitem->Label(),pack->Name())==0)
		{
			packitem->SetMarked(true);
			break;
		}
	}
	
	SetLevel(fPuzzle->Highest());
}

void GridView::SetLevel(uint8 level)
{
	if(fPuzzle->ValueAt(level)==0)
		return;
	
	fLevel = level;
	fMoveCount = 0;
	SetMovesLabel(fMoveCount);
	
	char label[30];
	sprintf(label,"Level: %lu",fLevel+1);
	fLevelLabel->SetText(label);
	fLevelLabel->ResizeToPreferred();
	
	fGrid.SetGridValues(fPuzzle->ValueAt(level));
	for(uint8 i=0; i<25; i++)
	{
		if(fGrid.ValueAt(i))
			fButtons[i]->SetState(1);
		else
			fButtons[i]->SetState(0);
	}
	BMenuItem *current = fLevelMenu->ItemAt(level);
	current->SetMarked(true);
}

void GridView::SetMovesLabel(uint32 count)
{
	BString string("Moves: ");
	string << count;
	fMovesLabel->SetText(string.String());
}

void GridView::HandleFinish(void)
{
	// Determine whether or not the user finished in the required number of
	// moves
	uint32 movesreq = fPuzzle->MovesRequired(fLevel);
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
	BAlert *alert = new BAlert("Lights Off","Congratulations!","OK");
	alert->Go();
	SetLevel(fLevel+1);
	
	// SetLevel() increments fLevel for us
	if(fPuzzle->Highest()==fLevel-1)
		fPuzzle->SetHighest(fLevel);
	
	BMenuItem *item = fLevelMenu->ItemAt(fLevel);
	if(item && !item->IsEnabled())
		item->SetEnabled(true);
}

void GridView::StartupPreferences(void)
{
	if(LoadPreferences(PREFERENCES_PATH)==B_OK)
	{
		BString lastpack;
		if(preferences.FindString("lastpack",&lastpack)!=B_OK)
			lastpack = "Classic";
		
		for(int32 i=0; i<gPuzzles.CountPacks(); i++)
		{
			PuzzlePack *pack = (PuzzlePack*)gPuzzles.PackAt(i);
			if(pack)
			{
				int32 highest=0;
				if(preferences.FindInt32(pack->Name(),&highest)==B_OK)
					pack->SetHighest(highest);
				else
					pack->SetHighest(0);
				
				if(strcmp(lastpack.String(),pack->Name())==0)
					fPuzzle=pack;
			}
		}
		
		if(preferences.FindBool("usesound",&fUseSound)!=B_OK)
			fUseSound = true;
	}
	else
	{
		fPuzzle = gPuzzles.PackAt(0);
		fUseSound = true;
	}
	
	if(fUseSound)
	{
		fClickSound->Preload();
		fWinSound->Preload();
		fNoWinSound->Preload();
	}
}

void GridView::ShutdownPreferences(void)
{
	preferences.MakeEmpty();
	
	// Save the progress in each of the puzzle packs
	for(int32 i=0; i<gPuzzles.CountPacks(); i++)
	{
		PuzzlePack *pack = (PuzzlePack*)gPuzzles.PackAt(i);
		if(pack)
		{
			preferences.AddString("name",pack->Name());
			preferences.AddInt32(pack->Name(),pack->Highest());
		}
	}
	preferences.AddString("lastpack",fPuzzle->Name());
	preferences.AddBool("usesound",fUseSound);
	
	SavePreferences(PREFERENCES_PATH);
}
