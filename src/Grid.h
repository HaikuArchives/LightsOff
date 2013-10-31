#ifndef GRID_H
#define GRID_H

#include <SupportDefs.h>

// The Grid class performs data handling and translation for the lights
// themselves and also makes it easy to write a level to disk. :)

class Grid
{
public:
	Grid(uint32 value=0);
	void FlipValueAt(uint8 x, uint8 y);
	void FlipValueAt(uint8 offset);
	bool ValueAt(uint8 x, uint8 y);
	bool ValueAt(uint8 offset);
	void SetValue(uint8 offset, bool ison);
	void SetValue(uint8 x, uint8 y, bool ison);
	void SetGridValues(uint32 value);
	uint32 GetGridValues(void);
	
private:
	uint8 fData[25];
};

#endif
