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
	assert (a != b);

	const int tmp = a;
	a = b;
	b = tmp;
}


// array size >= n
int ChooseRandom(int array[], int n, int k)
{
	assert(k > 0 && k < n);

	const bool chooseComplement = n / k < 2;

	if (chooseComplement)
		k = n - k;

	for (int i = 0; i < k; i++) {
		const int offset = random() % (n - i);

		if (offset > 0)
			Swap(array[i], array[i + offset]);
	}

	return chooseComplement ? k : 0;
}
