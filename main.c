#include "./include/uapi/seat.h"

#include <nd/nd.h>
#include <nd/fight.h>

typedef struct {
	unsigned quantity;
	unsigned capacity;
} seat_t;

typedef unsigned sitter_t;

unsigned type_seat, sitter_hd;

SIC_DEF(unsigned, sitting, unsigned, ref);

unsigned sitting(unsigned ref) {
	sitter_t sitter;
	nd_get(sitter_hd, &sitter, &ref);
	return sitter;
}

static inline void
sit(unsigned player_ref, sitter_t *sitter, char *name)
{
	if ((*sitter) != STANDING) {
		nd_writef(player_ref, "You are already sitting.\n");
		return;
	}

	if (!*name) {
		notify_wts(player_ref, "sit", "sits", " on the ground");
		*sitter = NOTHING;
		return;
	}

	unsigned seat_ref = ematch_near(player_ref, name);
	if (seat_ref == NOTHING) {
		nd_writef(player_ref, "Invalid target.\n");
		return;
	}

	OBJ seat;
	nd_get(HD_OBJ, &seat, &seat_ref);

	if (seat.type != type_seat) {
		nd_writef(player_ref, "You can't sit on that.\n");
		return;
	}

	seat_t *sseat = (seat_t *) &seat.data;

	if (sseat->quantity >= sseat->capacity) {
		nd_writef(player_ref, "No seats available.\n");
		return;
	}

	sseat->quantity += 1;
	*sitter = seat_ref;
	notify_wts(player_ref, "sit", "sits", " on %s", seat.name);
}

int
stand_silent(unsigned player_ref, sitter_t *sitter)
{
	if (*sitter == STANDING)
		return 1;

	if (*sitter != NOTHING) {
		OBJ chair;
		nd_get(HD_OBJ, &chair, sitter);
		seat_t *schair = (seat_t *) &chair.data;
		schair->quantity--;
		nd_put(HD_OBJ, sitter, &chair);
		*sitter = NOTHING;
	}

	notify_wts(player_ref, "stand", "stands", " up");
	return 0;
}

void
stand(unsigned player_ref, sitter_t *sitter) {
	if (stand_silent(player_ref, sitter))
		nd_writef(player_ref, "You are already standing.\n");
}

void
do_sit(int fd, int argc __attribute__((unused)), char *argv[])
{
	unsigned player_ref = fd_player(fd);
	sitter_t sitter;

	nd_get(sitter_hd, &sitter, &player_ref);
        sit(player_ref, &sitter, argv[1]);
	nd_get(sitter_hd, &player_ref, &sitter);
}

void
do_stand(int fd, int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
	unsigned player_ref = fd_player(fd);
	sitter_t sitter;
	nd_get(sitter_hd, &sitter, &player_ref);

        stand(player_ref, &sitter);

	nd_put(sitter_hd, &player_ref, &sitter);
}

int on_before_leave(unsigned player_ref) {
	sitter_t sitter;
	nd_get(sitter_hd, &sitter, &player_ref);

	if (sitter == STANDING)
		return 1;

	stand_silent(player_ref, &sitter);

	nd_put(sitter_hd, &player_ref, &sitter);
	return 0;
}

hit_t on_will_attack(unsigned player_ref, double dt __attribute__((unused))) {
	sitter_t sitter;
	hit_t hit;

	nd_get(sitter_hd, &sitter, &player_ref);
	stand_silent(player_ref, &sitter);
	nd_put(sitter_hd, &player_ref, &sitter);

	sic_last(&hit);
	return hit; // don't hit on first round if you were sitting
}

int on_examine(unsigned player_ref, unsigned thing_ref, unsigned type) {
	OBJ obj;
	seat_t *seat = (seat_t *) &obj.data;

	if (type != type_seat)
		return 1;

	nd_get(HD_OBJ, &obj, &thing_ref);
	nd_writef(player_ref, "seat quantity %u capacity %u.\n", seat->quantity, seat->capacity);
	return 0;
}

int on_add(unsigned ref, unsigned type, uint64_t v __attribute__((unused))) {
	sitter_t sitter;

	if (type != TYPE_ENTITY)
		return 0;

	sitter = STANDING;
	nd_put(sitter_hd, &ref, &sitter);
	return 0;
}

void mod_open(void) {
	sitter_hd = nd_open("sitter", "u", "u", 0);

	type_seat = nd_put(HD_TYPE, NULL, "seat");

	nd_register("sit", do_sit, 0);
	nd_register("stand", do_stand, 0);
}

void mod_install(void) {
	mod_open();
}
