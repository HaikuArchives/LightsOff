#include "Grid.h"

Grid::Grid(int8 dimension)
{
	SetDimension(dimension);
}

void Grid::SetDimension(int8 dimension)
{
	fDimension = dimension;
	fData.clear();
	fData.resize(dimension * dimension, 0);
	fGrid = fData;
}


/*
 * Create a puzzle by pressing minMoves-many random buttons on an empty grid.
 * This ensures that the created puzzle can be solved (by pressing the same
 * buttons again although better solutions may exist).
 */

void Grid::Random(int8 minMoves)
{
	// start with an empty grid
	for (int8 index = 0; index < fGrid.size(); index++)
		fData[index] = fGrid[index] = 0;

	if (minMoves > fDimension)
		minMoves = fDimension;

	for (int8 count = 0; count < minMoves; count++) {
		int8 index;

		// next random button not already marked
		do index = random() % fGrid.size();
		while (fGrid[index]);

		fGrid[index] = 1;	// mark the button to be pressed
	}

	const int8 n = fDimension;

	for (int8 index = 0; index < fGrid.size(); index++)
		if (fGrid[index]) {
			if (index % n)	// not leftmost column
				fData[index - 1] = !fData[index - 1];	// left neighbor

			if ((index + 1) % n)	// not rightmost column
				fData[index + 1] = !fData[index + 1];	// right neighbor

			if (index >= n)	// not top row
				fData[index - n] = !fData[index - n];	// neighbor above

			if (index < n * (n - 1))	// not bottom row
				fData[index + n] = !fData[index + n];	// neighbor below

			fData[index] = !fData[index];
		}
}


bool Grid::ValueAt(int8 x, int8 y)
{
	return fData[x + y * fDimension];
}

bool Grid::ValueAt(int8 offset)
{
	return fData[offset];
}

void Grid::SetValue(int8 x, int8 y, bool isOn)
{
	fData[x + y * fDimension] = isOn;
}

void Grid::SetValue(int8 offset, bool isOn)
{
	fData[offset] = isOn;
}

void Grid::SetGridValues(uint32 value)
{
	for (int8 i = 0; i < fData.size(); i++)
		fData[i] = (value & 1 << i) != 0;
}

uint32 Grid::GetGridValues()
{
	uint32 values = 0;

	for (int8 i = 0; i < fData.size(); i++)
		if(fData[i])
			values |= 1 << i;

	return values;
}

void Grid::FlipValueAt(int8 x, int8 y)
{
	const int8 offset = x + y * fDimension;
	fData[offset] = !fData[offset];
}

void Grid::FlipValueAt(int8 offset)
{
	fData[offset] = !fData[offset];
}
