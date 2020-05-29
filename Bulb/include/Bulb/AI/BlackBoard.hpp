#pragma once

namespace blb::ai {
	using BlackBoardKey = size_t;
	static constexpr BlackBoardKey INVALID_KEY = 0;

	struct BlackBoardDataHeader {
		char* prevItem = nullptr; //TODO: Needed?
		char* nextItem = nullptr;

		BlackBoardKey key = INVALID_KEY;
		size_t itemSize = 0;
	};

	class BlackBoard {
		//VARIABLES
	private:
		//Idea is that we store data in here with a header value so we know the memory layout
		char* data = nullptr;
		size_t dataSize = 0;
		char* pointer = nullptr; //Points to current free space

		size_t nextKey = 1;

		//FUNCTIONS
	public:
		//TODO: Ctors
		BlackBoard() {
			dataSize = 1024 * 1024; //Lets just give ourselves a meg at the moment
			data = reinterpret_cast<char*>(malloc(dataSize));
			pointer = data;
		}

		~BlackBoard() {
			free(data);
		}

		template<typename DataType>
		BlackBoardKey addItem(DataType item) {
			//TODO: Search the current array to see if there are any free spots of the correct size

			const size_t headerSize = sizeof(BlackBoardDataHeader);
			const size_t itemSize = sizeof(DataType);
			const size_t totalSize = headerSize + itemSize;

			//TODO: resize and copy elements to new area if not enough space
			CLV_ASSERT(pointer + totalSize <= data + dataSize, "Not enough space!");

			BlackBoardDataHeader header{};
			if(pointer != data) {
				header.prevItem = pointer;
				//TODO: Update nextItem of previous
			}
			header.key = nextKey++;
			header.itemSize = itemSize;

			//TODO: Doing memcpy for simplicity, try placement new
			memcpy(pointer, &header, headerSize);
			pointer += headerSize;
			memcpy(pointer, &item, itemSize);
			pointer += itemSize;

			return header.key;
		}

		template<typename DataType>
		DataType* getItem(BlackBoardKey key) {
			//TODO: Return nullptr if key is invalid;

			if(pointer == data) {
				return nullptr;
			}

			char* iter = data;
			while(iter != nullptr) {
				auto* header = reinterpret_cast<BlackBoardDataHeader*>(iter);
				if(header->key == key) {
					return reinterpret_cast<DataType*>(iter + sizeof(BlackBoardDataHeader));
				} else {
					iter = header->nextItem;
				}
			}

			return nullptr;
		}

		void removeItem(BlackBoardKey key) {
			//TODO: Return nullptr if key is invalid;

			if(pointer == data) {
				return;
			}

			char* iter = data;
			while(iter != nullptr) {
				auto* header = reinterpret_cast<BlackBoardDataHeader*>(iter);
				if(header->key == key) {
					header->key = INVALID_KEY;
					break;
				} else {
					iter = header->nextItem;
				}
			}
		}

	private:
	};
}