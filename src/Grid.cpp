#include "Grid.h"
#include <string.h>
#include <stdio.h>

Grid::Grid(uint32 value)
{
	SetGridValues(value);
}

bool Grid::ValueAt(uint8 x, uint8 y)
{
	uint8 offset = (y * 5) + x;
	
	return fData[offset] != 0;
}

bool Grid::ValueAt(uint8 offset)
{
	return fData[offset] != 0;
}

void Grid::SetValue(uint8 x, uint8 y, bool ison)
{
	uint8 offset = (y * 5) + x;
	
	fData[offset] = ison ? 1 : 0;
}

void Grid::SetValue(uint8 offset, bool ison)
{
	fData[offset] = ison ? 1 : 0;
}

void Grid::SetGridValues(uint32 value)
{
	memset(fData,0,sizeof(uint8)*25);
	for(uint32 i=0; i<25; i++)
	{	
		uint32 mask = 1 << i;
		if(value & mask)
			fData[i] = 1;
	}
}

uint32 Grid::GetGridValues(void)
{
	uint32 values=0;
	for(uint32 i=0; i<25; i++)
	{
		if(fData[i])
			values |= 1 << i;
	}
	return values;
}

void Grid::FlipValueAt(uint8 x, uint8 y)
{
	uint8 offset = (y * 5) + x;
	
	fData[offset] = fData[offset] ? 0 : 1;
}

void Grid::FlipValueAt(uint8 offset)
{
	fData[offset] = fData[offset] ? 0 : 1;
}
