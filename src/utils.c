#include "utils.h"

void utlClearArray(uint8_t *arr, uint32_t size) {
	for(uint32_t i = 0; i < size; i++) {
		if(arr[i] != 0) {
			arr[i] = 0;
		}
	}
}
