/* seat.h — nd-seat's cross-module API: sitting state.
 *
 * Caller-facing header. Implementers include <nd/seat-types.h>, not this header.
 */

#ifndef ND_SEAT_H
#define ND_SEAT_H

#include <ttypt/xy.h>
#include <nd/seat-types.h>

XY_DECL(unsigned, sitting, unsigned, ref);

#endif /* !ND_SEAT_H */
