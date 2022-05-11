#include "test.h"

void test_score_update(struct test_machine *tm, const struct test *ts, int t)
{
	tm->scores[t] = ts[t].points;
	tm->total_score += tm->scores[t];
}

void test_print_failure(const char *name, int total_score)
{
	const void *args[] = { name, &total_score };
	printf("%s: fail\n\ttotal: %d\n", args);
}

void test_print_success(const char *name, int score, int total_score)
{
	const void *args[] = { name, &score, &total_score };
	printf("%s: pass\n\tpoints earned: %d\n\tnew total: %d\n\n", args);
}
