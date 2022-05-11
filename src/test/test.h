#include "../kernel/util.h"

struct test {
	const char name[64];
	int points;
};

struct test_machine {
	int scores[128];
	int total_score;
	int t;
};

void test_score_update(struct test_machine *tm, const struct test *ts, int t);
void test_print_failure(const char *name, int total_score);
void test_print_success(const char *name, int score, int total_score);
