#ifndef GRID_H
#define GRID_H

#include <vector>

#include <SupportDefs.h>

typedef std::vector<int8> grid;

// The Grid class performs data handling and translation for the lights
// themselves and also makes it easy to write a level to disk. :)

class Grid
{
public:
	Grid(int8 dimension);
	void SetDimension(int8 dimension);
	void Random(int8 minMoves);
	void FlipValueAt(int8 x, int8 y);
	void FlipValueAt(int8 offset);
	bool ValueAt(int8 x, int8 y);
	bool ValueAt(int8 offset);
	void SetValue(int8 offset, bool isOn);
	void SetValue(int8 x, int8 y, bool isOn);
	void SetGridValues(uint64 value);
	uint64 GetGridValues();

private:
	int8 fDimension;
	grid fData;
};

#endif
