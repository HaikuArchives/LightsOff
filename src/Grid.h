#ifndef GRID_H
#define GRID_H

#include <vector>

#include <SupportDefs.h>

typedef std::vector<uint8> grid;

// The Grid class performs data handling and translation for the lights
// themselves and also makes it easy to write a level to disk. :)

class Grid
{
public:
	Grid(int8 dimension);
	void FlipValueAt(uint8 x, uint8 y);
	void FlipValueAt(uint8 offset);
	bool ValueAt(uint8 x, uint8 y);
	bool ValueAt(uint8 offset);
	void SetValue(uint8 offset, bool isOn);
	void SetValue(uint8 x, uint8 y, bool isOn);
	void SetGridValues(uint32 value);
	uint32 GetGridValues();

private:
	int8 fDimension;
	grid fData;
};

#endif
