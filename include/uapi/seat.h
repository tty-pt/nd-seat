/* SOFT REQUIRES: fight, mortal */

#ifndef SEAT_H
#define SEAT_H

#define STANDING (unsigned)(NOTHING - 1)

typedef struct {
	unsigned quantity;
	unsigned capacity;
} seat_t;

typedef unsigned sitter_t;

#endif
