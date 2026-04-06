#pragma once

#include <windows.h>
#include <vector>
#include <d3dx8.h>

template <typename T>
struct AlignedAllocator {
	using value_type = T;

	AlignedAllocator() = default;

	template <typename U>
	AlignedAllocator(const AlignedAllocator<U>&) {}

	template <typename U>
	struct rebind { using other = AlignedAllocator<U>; };

	auto allocate(size_t n) -> T* {
		void* ptr = _aligned_malloc(n * sizeof(T), 64);
		if (!ptr) throw std::bad_alloc();
		return static_cast<T*>(ptr);
	}

	auto deallocate(T* ptr, size_t) -> void {
		_aligned_free(ptr);
	}

	auto operator!=(const AlignedAllocator&) const -> bool { return false; }
	auto operator==(const AlignedAllocator&) const -> bool { return true; }
};

#pragma pack(push, 1)
struct XYZNormalColor {
	float x, y, z, nx, ny, nz;
	DWORD color;
	char pad[4];

	using ArrayType = std::vector<XYZNormalColor, AlignedAllocator<XYZNormalColor>>;
	static constexpr DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE;
	static constexpr size_t STRIDE = 32;
};
static_assert(XYZNormalColor::STRIDE == sizeof(XYZNormalColor));
#pragma pack(pop)
