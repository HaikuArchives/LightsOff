#ifndef PUZZLEPACK_H
#define PUZZLEPACK_H

#include <String.h>
#include <List.h>

class PuzzlePack
{
public:
	PuzzlePack(const char *name, uint32 *data, const uint32 size,const uint8 &moves);
	virtual ~PuzzlePack(void) { }
	const char *Name(void) const { return fName.String(); }
	uint32	Size(void) const { return fSize; }
	uint32	ValueAt(const uint32 &index);
	virtual uint8 MovesRequired(const uint32 &index);
	void SetHighest(const uint32 &highest) { fHighest = highest; }
	uint32 Highest(void) const { return fHighest; }
	
private:
	BString	fName;
	uint32	fSize;
	uint32	*fData;
	uint8	fMoves;
	uint32	fHighest;
};

class PuzzlePackSet
{
public:
	PuzzlePackSet(void);
	~PuzzlePackSet(void);
	
	PuzzlePack *PackAt(const int32 &index) const { return (PuzzlePack*)fList.ItemAt(index); }
	int32 CountPacks(void) const { return fList.CountItems(); }
	
private:
	BList fList;
};

#endif
