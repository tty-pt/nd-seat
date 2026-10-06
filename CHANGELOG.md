## 1.0.2

- **The sibling `-I` lines are gone.** `CFLAGS += -I$(shell cd .. && pwd)/…`
  pointed at the axil-nd/nd sibling checkouts and only existed for a dev
  build: in CI those directories do not exist and every header comes from the
  installed packages named in `.github/workflows/ci.yml`. The build now
  resolves `<nd/…>` the way a packager sees it.
- **macOS: link with `-undefined dynamic_lookup`.** macOS `ld` rejects
  undefined symbols in a shared library, but `WARN` needs `qsyslog` — an
  engine-provided function pointer resolved at `dlopen` time (Linux allows
  this by default). `-undefined dynamic_lookup` is the Darwin equivalent, set
  as `LDFLAGS-libnd-seat-Darwin` so no other platform is affected.

## [1.0.0]

- **nd-seat is now an installable library rather than a build artifact of
  the engine.** It builds and installs exactly two files,
  `lib/libnd-seat.so` and `include/nd/seat.h`, following the same layout as
  `axil-tty` and `axil-auth`, and the same layout `nd-core` was converted to
  first. Previously `make` produced a `seat.so` named by the engine's
  `mods.load` and installed nothing. There is no `lib/nd-seat.so` symlink:
  `mods.load` names this module `libnd-seat`, the installed filename, and
  `module_load_path()` only appends `.so`.

- **The link line is libxylem alone.** `LDLIBS := -lxylem`; the engine is not
  linked. `NEEDED` is `libxylem.so` and `libc.so.6`.

- **The seat query is declared in `<nd/seat.h>`.** `sitting` is `XY_DECL`'d
  there; this TU defines `SEAT_IMPL` because it `XY_IMPL`s the same name,
  and defines `FIGHT_IMPL` because it co-implements `on_will_attack`
  (stand up, pass the hit through) from `<nd/fight.h>`.

- **`call_verb`/`call_verb_to` never existed** and are gone from the port;
  sit/stand messages go out as `nd_printf` + `nd_rwrite` via `OBJ.location`.

- **Dropped the `nd-mod.mk` dependency.** `nd-mod.mk` has now been deleted.
