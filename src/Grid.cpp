#include "Grid.h"

#include "Random.h"

Grid::Grid(int8 dimension)
{
	SetDimension(dimension);
}

void Grid::SetDimension(int8 dimension)
{
	fDimension = dimension;
	fData.clear();
	fData.resize(dimension * dimension, 0);
}


/*
 * Create a puzzle by pressing minMoves-many random buttons on an empty grid.
 * This ensures that the created puzzle can be solved (by pressing the same
 * buttons again although better solutions may exist).
 */

void Grid::Random(int8 minMoves)
{
	const int8 numButtons = fData.size();

	// start with an empty grid
	for (int8 index = 0; index < numButtons; index++)
		fData[index] = 0;

	/*
	 * Some of the eigenvectors
	 *
	 * An eigenvector is a puzzle whose solution is simply pressing all lit
	 * buttons in the puzzle. --Owen
	 */
#if 0
	// 3x3.png
	if (fDimension == 3 && minMoves == 8) {
		for (int8 index = 0; index < numButtons; index++)
			fData[index] = 1;
		fData[4] = 0;
		return;
	}

	// 6x6.png
	if (fDimension == 6 && minMoves == 6) {
		fData[0] = fData[7] = fData[14] = fData[21] = fData[28] = fData[35] = 1;
		return;
	}

	// 7x7.png
	if (fDimension == 7 && minMoves == 16) {
		fData[3] = fData[9] = fData[11] = fData[15] = fData[17] = fData[19]
			= fData[21] = fData[23] = fData[25] = fData[27] = fData[29]
			= fData[31] = fData[33] = fData[37] = fData[39] = fData[45] = 1;
		return;
	}

	// 8x8.png
	if (fDimension == 8 && minMoves == 14) {
		fData[6] = fData[13] = fData[15] = fData[20] = fData[22] = fData[27]
			= fData[29] = fData[34] = fData[36] = fData[41] = fData[43]
			= fData[48] = fData[50] = fData[57] = 1;
		return;
	}
#endif

	if (minMoves > numButtons)
		minMoves = numButtons;

	int buttonIndices[numButtons];

	for (int8 index = 0; index < numButtons; index++)
		buttonIndices[index] = index;

	int8 begin;
	const int8 n = fDimension;

	if (n == 5) {
		ChooseRandom5x5(buttonIndices, minMoves);
		begin = 0;
	} else
		begin = ChooseRandom(buttonIndices, numButtons, minMoves);

	for (int8 i = 0; i < minMoves; i++) {
		const int8 index = buttonIndices[begin + i];

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

void Grid::SetGridValues(uint64 value)
{
	for (int8 i = 0; i < fData.size(); i++)
		fData[i] = (value & (uint64) 1 << i) != 0;
}

uint64 Grid::GetGridValues()
{
	uint64 values = 0;

	for (int8 i = 0; i < fData.size(); i++)
		if(fData[i])
			values |= (uint64) 1 << i;

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
