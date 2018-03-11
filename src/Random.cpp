/*
 * Copyright 2018 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *			Owen Pan <owen.pan@yahoo.com>
 */

#include "Random.h"

//#define NDEBUG
#include <assert.h>	// assert
#include <stdlib.h>	// random


static void
Swap(int& a, int& b)
{
	if (a == b)
		return;

	const int tmp = a;
	a = b;
	b = tmp;
}


static bool
find(int m, int array[], int size)
{
	for (int index = 0; index < size; index++)
		if (m == array[index])
			return true;

	return false;
}


int
ChooseRandom(int array[], int n, int k)	// array size >= n
{
	assert(k > 0 && k < n);

	const bool chooseComplement = n / k < 2;

	if (chooseComplement)
		k = n - k;

	for (int index = 0; index < k; index++) {
		assert(n - index > 1);
		Swap(array[index], array[index + random() % (n - index)]);
	}

	return chooseComplement ? k : 0;
}


void
ChooseRandom5x5(int array[], int k)	// array size >= 25
{
	assert(k > 0 && k <= 15);

	// partition the 5x5 grid
	int c[] = { 0, 4, 20, 24 };					// corners
	int h[] = { 1, 3, 10, 11, 13, 14, 21, 23 };	// horizontal
	int v[] = { 2, 5, 7, 9, 15, 17, 19, 22 };	// vertical
//	int d[] = { 6, 8, 12, 16, 18 };				// diagonals

	// initialize the corresponding counters
	int C = 0;
	int H = 0;
	int V = 0;

	// set the upper limits of the counters
	int CMax, HMax, VMax;
	switch (k) {
		case 14:
			CMax = 3;
			HMax = VMax = 5;
			break;
		case 15:
			CMax = 2;
			HMax = VMax = 4;
			break;
		default:
			CMax = 4;
			HMax = VMax = 8;
	}

	int n = 25;	// 5x5

	for (int index = 0; index < k;) {
		assert(n - index > 0);

		int nextIndex = index;
		if (n - index > 1)
			nextIndex += random() % (n - index);

		const int m = array[nextIndex];
		bool skip = false;

		if (find(m, c, 4))
			if (C == CMax || C + H == 6 || C + V == 6)
				skip = true;
			else
				C++;
		else if (find(m, h, 8))
			if (H == HMax || C + H == 6 || H + V == 8)
				skip = true;
			else
				H++;
		else if (find(m, v, 8))
			if (V == VMax || C + V == 6 || H + V == 8)
				skip = true;
			else
				V++;

		if (skip)
			array[nextIndex] = array[--n];
		else
			Swap(array[index++], array[nextIndex]);
	}
}
