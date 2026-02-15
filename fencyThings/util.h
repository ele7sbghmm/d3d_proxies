#pragma once

template <typename T>
struct SwapArray {
	int mSize;
	int mUseSize;
	T* mpData;
	void* mSwapT;
};

template <typename T>
using NodeSwapArray = SwapArray<T>;
