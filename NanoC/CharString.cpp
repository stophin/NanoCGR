//CharString.cpp
//


#include "CharString.h"

CharStringPool charStringPool;

extern "C" __NANOC_EXPORT__ ICharStringPool  * GetPool() {
	return &charStringPool;
}
