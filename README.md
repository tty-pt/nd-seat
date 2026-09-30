# axil-nd-seat

`nd-seat` for [axil-nd](../axil-nd), ported from SIC to libxylem.

Owns sitting: the `sitter` and `seat` tables, the `sitting` query, and the
stand-up path. Its one combat role is co-implementing nd-fight's
`on_will_attack` chain — stand the target up silently, then pass the hit
through unchanged — reading fight's hit with `nd_last()`.

## Install

```sh
make install
```

Installs:

```
lib/libnd-seat.so
include/nd/seat.h
```

There is deliberately no `lib/nd-seat.so` symlink (see `axil-nd-wts` for
why: `mods.load` names the installed filename, and the OpenBSD packing list
never lists a symlink).

Also packaged for deb, apk, rpm, brew and openbsd from a `v*` tag.

## Build from source

```sh
make
```

Needs [libxylem](https://github.com/tty-pt/libxylem) and the engine's game
API, `<nd/xy.h>`, plus `<nd/fight.h>` for the `hit_t` this module passes
through — from a checkout beside this repo or from the installed package:

```sh
git clone https://github.com/tty-pt/nd-seat && cd nd-seat
git clone https://github.com/tty-pt/axil-nd ../axil-nd
git clone https://github.com/tty-pt/nd-fight ../axil-nd-fight
make
```

Both the checkout `-I` flags and the installed-package paths are on the
command line at once (see `Makefile`), and a missing `-I` is ignored, so the
same command works either way. CI names the deps explicitly
(`axil-nd,libxylem,nd-fight`).

## What it does

* `xy_install()` registers the `sitter` table (a `u32` map, player → seat)
  and the `seat` table (one `seat_t` per seat).
* `sitting` answers whether a player is seated; `on_before_leave` stands them
  up; `on_will_attack` stands the target up silently and returns the hit
  unchanged; `on_examine` reports the seat; `on_add` initializes rows.

## Testing

There is no `test.sh` here. Behaviour is asserted by the engine's own suite:

```sh
cd ../axil-nd
make && ./test.sh
```

## Notes from the port

* `SIC_DEF` → `XY_IMPL`, `mod_install` → `xy_install`, `call_f(...)` →
  `f(...)`. `call_verb`/`call_verb_to` never existed; sit/stand messages go
  out as `nd_printf` + `nd_rwrite` via `OBJ.location`.
* Because this TU `XY_IMPL`s `on_will_attack` from `<nd/fight.h>`, it defines
  `FIGHT_IMPL` before including it, and defines `SEAT_IMPL` for its own
  `<nd/seat.h>`.
* The link line is libxylem alone. `NEEDED` is `libxylem.so` and `libc.so.6`.

## License

BSD 2-Clause, carried over from `tty-pt/nd-seat`. See `LICENSE`.
