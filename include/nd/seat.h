/* seat.h — nd-seat's cross-module API: sitting state.
 *
 * Include this from a module TU that wants to test whether someone is sitting
 * (nd-spell does, to suppress mana regen), and NOT from nd-seat's own
 * src/libnd-seat.c without SEAT_IMPL: an XY_IMPL and an XY_DECL of the same
 * name in one TU collide, which is the direct replacement for the old
 * `SIC_DECL` + `SIC_DEF` pairing in a single file. The provider needs STANDING
 * from here, so it includes under the guard rather than skipping the header.
 *
 * Usage:
 *
 *     #include <ttypt/xy-mod.h>     // must come first: injects the xy context
 *     #include <nd/xy.h>            // engine service hooks (nd_get, ...)
 *     #include <nd/seat.h>          // this file
 *
 * The consumer does not need to load nd-seat itself -- the engine loads every
 * module in mods.load into one region and XY dispatches by name -- but the
 * engine's mods.load must list seat, or these forward to a provider that is
 * not there.
 *
 * NOTE: this is a MODULE-OWNED header, not an engine one. The old location was
 * `include/uapi/seat.h`; the old `~/nd/module.mk` installed it as
 * `$(PREFIX)/include/nd/seat.h`, so `nd/` is this header's home and it is
 * installed here with `FOLDER := nd`.
 *
 * The old header included `<nd/type.h>`, a file that no longer exists. Its only
 * live content for this module was SIC_DECL/SIC_DEF/SIC_CALL (all now XY_*);
 * NOTHING comes from `<nd/xy-types.h>` via `<nd/xy.h>`.
 */

#ifndef ND_SEAT_H
#define ND_SEAT_H

#include <ttypt/xy.h>

#define STANDING (unsigned)(NOTHING - 1)

#ifndef SEAT_IMPL

XY_DECL(unsigned, sitting, unsigned, ref);

#endif /* !SEAT_IMPL */

#endif /* !ND_SEAT_H */