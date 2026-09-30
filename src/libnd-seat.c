/* src/libnd-seat.c — nd-seat, ported to libxylem.
 *
 * Owns sitting: who is seated where, the sit/stand commands, and standing up
 * automatically on attack, examine-leave, or the will-attack chain (sitting
 * targets don't hit back on the first round).
 *
 * Original: tty-pt/nd-seat @ 182 lines main.c, from the nd-basics
 * superproject.
 *
 * This TU XY_IMPLs sitting, on_will_attack, on_examine, on_add and
 * on_before_leave, and so defines SEAT_IMPL before including its own header:
 * an XY_IMPL and an XY_DECL of the same name in one TU is the XY equivalent
 * of the old SIC_DEF + SIC_DECL collision. The header's STANDING constant is
 * still needed, which is why the header is included under the guard rather
 * than skipped.
 *
 * The old call_verb() named a service that never existed (MODS.md: "no such
 * symbols exist anywhere in nd-basics"), so sit/stand announcements are room
 * broadcasts written directly.
 */

#include <ttypt/xy-mod.h>

#include <nd/xy.h>

#include <stdio.h>
#include <string.h>

#define SEAT_IMPL
#include <nd/seat.h>

/* hit_t comes from nd-fight's header. Seat co-implements on_will_attack, so
 * the FIGHT_IMPL guard suppresses that XY_DECL; nothing else from fight.h is
 * called here. */
#define FIGHT_IMPL
#include <nd/fight.h>

typedef struct {
	unsigned quantity;
	unsigned capacity;
} seat_t;

typedef unsigned sitter_t;

static unsigned type_seat, sitter_hd, seat_hd;
static unsigned wt_sit, wt_stand;

/* API. XY_IMPL both defines the function and emits the dispatch adapter, so
 * each name gets exactly one, with its body -- no forward declarations.
 *
 * Order matters below: XY_IMPL emits a definition, so a caller has to come
 * after its callee. sitting leads because sit/stand read it. */

XY_IMPL(unsigned, sitting, unsigned, ref)
{
	sitter_t sitter;
	nd_get(sitter_hd, &sitter, &ref);
	return sitter;
}

/* A sit/stand announcement. The old code called call_verb(player, wt, msg),
 * but verb never existed. "<Name><msg>" goes to the room, "You<msg>" to the
 * sitter. */
static void
say_seat(unsigned player_ref, char *msg)
{
	OBJ player;
	char buf[BUFSIZ];
	int len;

	nd_get(HD_OBJ, &player, &player_ref);
	nd_printf(player_ref, "You%s\n", msg);
	len = snprintf(buf, sizeof(buf), "%s%s\n", player.name, msg);
	nd_rwrite(player.location, player_ref, buf, (size_t)len);
}

static inline void
sit(unsigned player_ref, sitter_t *sitter, char *name)
{
	if ((*sitter) != STANDING) {
		nd_printf(player_ref, "You are already sitting.\n");
		return;
	}

	if (!*name) {
		say_seat(player_ref, " sits on the ground.");
		*sitter = NOTHING;
		return;
	}

	unsigned seat_ref = ematch_near(player_ref, name);
	if (seat_ref == NOTHING) {
		nd_printf(player_ref, "Invalid target.\n");
		return;
	}

	OBJ seat;
	nd_get(HD_OBJ, &seat, &seat_ref);

	if (seat.type != type_seat) {
		nd_printf(player_ref, "You can't sit on that.\n");
		return;
	}

	seat_t *sseat = (seat_t *) &seat.data;

	if (sseat->quantity >= sseat->capacity) {
		nd_printf(player_ref, "No seats available.\n");
		return;
	}

	sseat->quantity += 1;
	*sitter = seat_ref;

	char buf[BUFSIZ];
	snprintf(buf, sizeof(buf), " sits on %s", seat.name);
	say_seat(player_ref, buf);
}

static int
stand_silent(unsigned player_ref, sitter_t *sitter)
{
	if (*sitter == STANDING)
		return 1;

	if (*sitter != NOTHING) {
		seat_t seat;
		nd_get(seat_hd, &seat, sitter);
		seat.quantity--;
		nd_put(seat_hd, sitter, &seat);
		*sitter = NOTHING;
	}

	say_seat(player_ref, " stands up");
	return 0;
}

static void
stand(unsigned player_ref, sitter_t *sitter) {
	if (stand_silent(player_ref, sitter))
		nd_printf(player_ref, "You are already standing.\n");
}

static void
do_sit(int fd, int argc __attribute__((unused)), char *argv[])
{
	unsigned player_ref = fd_player(fd);
	sitter_t sitter;

	nd_get(sitter_hd, &sitter, &player_ref);
        sit(player_ref, &sitter, argv[1]);
	nd_get(sitter_hd, &player_ref, &sitter);
}

static void
do_stand(int fd, int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
	unsigned player_ref = fd_player(fd);
	sitter_t sitter;
	nd_get(sitter_hd, &sitter, &player_ref);

        stand(player_ref, &sitter);

	nd_put(sitter_hd, &player_ref, &sitter);
}

XY_IMPL(int, on_before_leave, unsigned, player_ref)
{
	sitter_t sitter;
	nd_get(sitter_hd, &sitter, &player_ref);

	if (sitter == STANDING)
		return 1;

	stand_silent(player_ref, &sitter);

	nd_put(sitter_hd, &player_ref, &sitter);
	return 0;
}

/* Co-implementor of nd-fight's on_will_attack chain: stand up, then pass the
 * hit through unchanged. nd_last() gives us fight's (or spell's) hit. */
XY_IMPL(hit_t, on_will_attack, unsigned, player_ref, double, dt)
{
	sitter_t sitter;
	hit_t hit;

	(void) dt;
	nd_get(sitter_hd, &sitter, &player_ref);
	stand_silent(player_ref, &sitter);
	nd_put(sitter_hd, &player_ref, &sitter);

	nd_last(&hit);
	return hit; // don't hit on first round if you were sitting
}

XY_IMPL(int, on_examine, unsigned, player_ref, unsigned, thing_ref, unsigned, type)
{
	OBJ obj;
	seat_t *seat = (seat_t *) &obj.data;

	if (type != type_seat)
		return 1;

	nd_get(HD_OBJ, &obj, &thing_ref);
	nd_printf(player_ref, "seat quantity %u capacity %u.\n", seat->quantity, seat->capacity);
	return 0;
}

XY_IMPL(int, on_add, unsigned, ref, unsigned, type, uint64_t, v)
{
	sitter_t sitter;

	(void) v;
	if (type != TYPE_ENTITY)
		return 0;

	sitter = STANDING;
	nd_put(sitter_hd, &ref, &sitter);
	return 0;
}

XY_MODULE_API void
xy_install(void)
{
	/* Order matches the original mod_install: WTS words (including its
	 * asymmetric nd_get for "stand"), then the opens, then the type. */
	nd_put(HD_WTS, NULL, "sit");
	nd_get(HD_WTS, NULL, "stand");

	nd_get(HD_RWTS, &wt_sit, "sit");
	nd_get(HD_RWTS, &wt_stand, "stand");

	nd_register("sit", do_sit, 0);
	nd_register("stand", do_stand, 0);

	nd_len_reg("seat", sizeof(seat_t));
	sitter_hd = (unsigned)nd_open("sitter", "u", "u", 0);
	seat_hd = (unsigned)nd_open("seat", "u", "seat", 0);

	type_seat = (unsigned)nd_put(HD_TYPE, NULL, "seat");
}