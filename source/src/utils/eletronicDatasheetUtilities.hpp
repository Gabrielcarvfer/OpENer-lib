//
//	Gabriel Ferreira (gabrielcarvfer)
//

#ifndef UTILS_ELETRONICDATASHEET_H
#define UTILS_ELETRONICDATASHEET_H

#include <string>
typedef struct
{
	void * address;
	int * size;
	bool read;
	bool write;
}memoryMapEntry;

class UTILS_EletronicDatasheet
{
	public:
		//Read eletronic datasheet and generate memory map for registers
		UTILS_EletronicDatasheet(std::string filePath);

		//Free everything
		~UTILS_EletronicDatasheet();

		//List of memory map entries, pointing each entry and their size
		memoryMapEntry* memoryMap;
	private:
		//Memory block
		void * memoryBlock;
};

#endif //UTILS_ELETRONICDATASHEET_H