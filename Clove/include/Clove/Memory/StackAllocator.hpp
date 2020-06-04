#pragma once

namespace clv::mem {
	class StackAllocator {
		//TYPES
	public:
		using Marker = size_t;

		//VARIABLES
	private:
		std::byte* stack;
		size_t stackSize = 0;
		std::byte* top	 = nullptr;

		bool freeMemory = true;

		//FUNCTIONS
	public:
		StackAllocator() = delete;
		StackAllocator(size_t sizeBytes);
		StackAllocator(std::byte* start, size_t sizeBytes);
		
		StackAllocator(const StackAllocator& other) = delete;
		StackAllocator(StackAllocator&& other) noexcept;

		StackAllocator& operator=(const StackAllocator& other) = delete;
		StackAllocator& operator=(StackAllocator&& other) noexcept;

		~StackAllocator();

		Marker markPosition();

		void* alloc(size_t bytes);

		void free();
		void free(Marker marker);
	};
}