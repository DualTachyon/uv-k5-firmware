#include <ctype.h>
#include "dtmf.h"

bool DTMF_ValidateCodes(char *pCode, uint8_t Size)
{
	uint8_t i;

	if (pCode[0] == 0xFF || pCode[0] == 0) {
		return false;
	}

	for (i = 0; i < Size; i++) {
		if (pCode[i] == 0xFF || pCode[i] == 0) {
			pCode[i] = 0;
			break;
		}
		if (!isdigit((int)pCode[i]) && (pCode[i] < 'A' || pCode[i] > 'D') && pCode[i] != '*' && pCode[i] != '#') {
			return false;
		}
	}

	return true;
}

