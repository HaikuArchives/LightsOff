#include "Grid.h"

Grid::Grid(int8 dimension)
	:
	fDimension(dimension)
{
	fData.clear();
	fData.resize(dimension * dimension, 0);
}

bool Grid::ValueAt(uint8 x, uint8 y)
{
	return fData[x + y * fDimension];
}

bool Grid::ValueAt(uint8 offset)
{
	return fData[offset];
}

void Grid::SetValue(uint8 x, uint8 y, bool isOn)
{
	fData[x + y * fDimension] = isOn;
}

void Grid::SetValue(uint8 offset, bool isOn)
{
	fData[offset] = isOn;
}

void Grid::SetGridValues(uint32 value)
{
	for (uint32 i = 0; i < fData.size(); i++)
		fData[i] = (value & 1 << i) != 0;
}

uint32 Grid::GetGridValues()
{
	uint32 values = 0;

	for (uint32 i = 0; i < fData.size(); i++)
		if(fData[i])
			values |= 1 << i;

	return values;
}

void Grid::FlipValueAt(uint8 x, uint8 y)
{
	const uint8 offset = x + y * fDimension;
	fData[offset] = !fData[offset];
}

void Grid::FlipValueAt(uint8 offset)
{
	fData[offset] = !fData[offset];
}
