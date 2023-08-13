#ifndef AIRCOPY_H
#define AIRCOPY_H

enum AIRCOPY_State_t {
	AIRCOPY_READY		= 0U,
	AIRCOPY_TRANSFER	= 1U,
	AIRCOPY_CMP		= 2U,
};

typedef enum AIRCOPY_State_t AIRCOPY_State_t;

extern AIRCOPY_State_t gAircopyState;

#endif

