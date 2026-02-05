
#include "pch.h"
#include <iostream>
#include "worldsim/coins/coinmanager.h"

void coinPos(CoinManager* cm, char x0[], char x1[], char y0[], char y1[], char z0[], char z1[]) {
	CoinManager::ActiveCoin* c = cm->mActiveCoins;

	int bufferSize = 5000;

	int offset = 0;
	for (int i = 0; i < 100; i++) {
		int written = sprintf_s(x0 + offset, bufferSize - offset, "%3d: %d %+.2f\n", i, c[i].State, c[i].Position.x);
		if (written > 0) offset += written;
		if (offset > bufferSize - 50) break;
	}
	offset = 0;
	for (int i = 100; i < 200; i++) {
		int written = sprintf_s(x1 + offset, bufferSize - offset, "%3d: %d %+.2f\n", i, c[i].State, c[i].Position.x);
		if (written > 0) offset += written;
		if (offset > bufferSize - 50) break;
	}

	offset = 0;
	for (int i = 0; i < 100; i++) {
		int written = sprintf_s(y0 + offset, bufferSize - offset, "%+.2f\n", c[i].Position.y);
		if (written > 0) offset += written;
		if (offset > bufferSize - 50) break;
	}
	offset = 0;
	for (int i = 100; i < 200; i++) {
		int written = sprintf_s(y1 + offset, bufferSize - offset, "%+.2f\n", c[i].Position.y);
		if (written > 0) offset += written;
		if (offset > bufferSize - 50) break;
	}

	offset = 0;
	for (int i = 0; i < 100; i++) {
		int written = sprintf_s(z0 + offset, bufferSize - offset, "%+.2f\n", c[i].Position.z);
		if (written > 0) offset += written;
		if (offset > bufferSize - 50) break;
	}
	offset = 0;
	for (int i = 100; i < 200; i++) {
		int written = sprintf_s(z1 + offset, bufferSize - offset, "%+.2f\n", c[i].Position.z);
		if (written > 0) offset += written;
		if (offset > bufferSize - 50) break;
	}
}
