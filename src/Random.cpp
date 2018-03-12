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

// partition the 5x5 grid into 4 subsets
static const int c[] = { 0, 4, 20, 24 };					// corners
static const int d[] = { 6, 8, 12, 16, 18 };				// diagonals
static const int h[] = { 1, 3, 10, 11, 13, 14, 21, 23 };	// horizontal
static const int v[] = { 2, 5, 7, 9, 15, 17, 19, 22 };		// vertical

static const int cSize = 4;
static const int dSize = 5;
static const int hvSize = 8;

static const int threshold = (cSize + hvSize) / 2;


static void
Swap(int& a, int& b)
{
	if (a == b)
		return;

	const int tmp = a;
	a = b;
	b = tmp;
}


static void
CopyArray(int dest[], const int src[], int size)
{
	for (int index = 0; index < size; index++)
		dest[index] = src[index];
}


static bool
find(int m, const int array[], int size)	// array size >= size
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


static void
CopyRandom(int*& dest, const int src[], int size)
{
	const int k = size / 2;

	CopyArray(dest, src, size);
	ChooseRandom(dest, size, k);
	dest += k;
}


void
ChooseRandom5x5(int array[], int k)	// array size >= 25
{
	assert(k > 0 && k <= 15);

	int n = 25;	// 5x5

	if (k <= threshold) {
		ChooseRandom(array, n, k);
		return;
	}

	if (k == 15) {
		CopyRandom(array, c, cSize);
		CopyRandom(array, h, hvSize);
		CopyRandom(array, v, hvSize);
		CopyArray(array, d, dSize);
		return;
	}

	// counters
	int C = 0;
	int H = 0;
	int V = 0;

	// upper limits of the counters
	int CMax, HMax, VMax;
	if (k == 14) {
		CMax = 3;
		HMax = VMax = 5;
	} else {
		CMax = cSize;
		HMax = VMax = hvSize;
	}

	for (int index = 0; index < k;) {
		assert(n - index > 0);

		int nextIndex = index;
		if (n - index > 1)
			nextIndex += random() % (n - index);

		const int m = array[nextIndex];
		bool skip = false;

		if (find(m, c, cSize))
			if (C == CMax || C + H == threshold || C + V == threshold)
				skip = true;
			else
				C++;
		else if (find(m, h, hvSize))
			if (H == HMax || C + H == threshold || H + V == hvSize)
				skip = true;
			else
				H++;
		else if (find(m, v, hvSize))
			if (V == VMax || C + V == threshold || H + V == hvSize)
				skip = true;
			else
				V++;

		if (skip)
			array[nextIndex] = array[--n];
		else
			Swap(array[index++], array[nextIndex]);
	}
}
