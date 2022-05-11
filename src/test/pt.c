#include "../kernel/pt.c"
#include "test.h"

enum tests {
	map_one_page,
	map_several_pages,
	access_demand_several_pages,
	map_mega_page,
	map_mega_normal_pages,
	access_demand_mega_normal_pages,
	map_populate_mega_normal_pages,
	access_mega_normal_pages,
	map_several_vmas,
	__success
};

static const struct test ts[] = {
	[map_one_page] = { "map_one_page", 1 },
	[map_several_pages] = { "map_several_pages", 1 },
	[access_demand_several_pages] = { "access_demand_several_pages", 1 },
	[map_mega_page] = { "map_mega_page", 1 },
	[map_mega_normal_pages] = { "map_mega_normal_pages", 1 },
	[access_demand_mega_normal_pages] = { "access_demand_mega_normal_pages",
					      1 },
	[map_populate_mega_normal_pages] = { "map_populate_mega_normal_pages",
					     1 },
	[access_mega_normal_pages] = { "access_mega_normal_pages", 1 },
	[map_several_vmas] = { "map_several_vmas", 1 }
};

struct pt_test_machine {
	struct vma *vma;
	uintptr_t vpn;
	size_t pagen;
	char *virt;
	int unmap;
	uintptr_t ptes[PT_NUM_LEVELS];
};

#define update_args()                                                          \
	do {                                                                   \
		args[0] = ts[tm.t].name;                                       \
		args[1] = &ptm.pagen;                                          \
		args[2] = (void *)ptm.virt;                                    \
	} while (0)

int pt_test(void)
{
	struct test_machine tm = { 0 };
	struct pt_test_machine ptm = { 0 };

	ptm.unmap = 1;
	const void *args[3];

	if (!(ptm.vma = pt_alloc_kvma())) goto should_not_happen;

	for (tm.t = 0; tm.t < __success; tm.t++) {
		switch (tm.t) {
		case map_one_page:
			ptm.vpn = 0xabcd;
			ptm.pagen = 1;
			ptm.virt = (char *)vpn2virt(ptm.vpn);

			update_args();
			printf("%s: mapping %d page(s) (with VMA_POPULATE) at %p...\n",
			       args);

			if (pt_vma_new(&ksatp, ptm.vpn, ptm.pagen,
					 VMA_READ | VMA_POPULATE, ptm.vma))
				goto map_failure;

			if (pt_get_pte(&ksatp, ptm.vpn, ptm.ptes, NULL) != 4)
				goto map_failure;

			break;

		case map_several_pages:
			ptm.vpn = 0x1234;
			ptm.pagen = 13;
			ptm.virt = (char *)vpn2virt(ptm.vpn);
			ptm.unmap = 0;

			update_args();
			printf("%s: mapping %d pages (without VMA_POPULATE) at %p...\n", args);

			if (pt_vma_new(&ksatp, ptm.vpn, ptm.pagen, VMA_READ,
					 ptm.vma))
				goto map_failure;

			uintptr_t tmp = ptm.vpn + rand(ptm.pagen) % ptm.pagen;

			if (!vpn2vma(kvmas_head, tmp)) goto map_failure;

			/* There should be no VMA for this address */
			tmp += ptm.pagen + 1;

			if (vpn2vma(kvmas_head, tmp)) goto map_failure;

			break;

		case access_demand_several_pages:
			update_args();
			printf("%s: accessing them (to test demand paging)...\n",
			       args);

			for (size_t i = 0; i < ptm.pagen; i++) {
				ptm.virt = (char *)vpn2virt(ptm.vpn) +
					   i * PAGE_SIZE;
				if (*ptm.virt) goto map_failure;
				if (pt_get_pte(&ksatp,
					       virt2vpn((uintptr_t)ptm.virt),
					       ptm.ptes, NULL) != 4) {
					goto map_failure;
				}
			}

			break;

		case map_mega_page:
			ptm.vpn = 0x567800;
			ptm.pagen = 512;
			ptm.virt = (char *)vpn2virt(ptm.vpn);

			update_args();
			printf("%s: mapping %d pages (huge page) (with VMA_POPULATE) at %p...\n",
			       args);

			if (pt_vma_new(&ksatp, ptm.vpn, ptm.pagen,
					 VMA_READ | VMA_POPULATE, ptm.vma))
				goto map_failure;

			if (pt_get_pte(&ksatp, ptm.vpn, ptm.ptes, NULL) != 3)
				goto map_failure;

			break;

		case map_mega_normal_pages:
			ptm.vpn = 0xaaa000;
			ptm.pagen = 515;
			ptm.virt = (char *)vpn2virt(ptm.vpn);
			ptm.unmap = 0;

			update_args();
			printf("%s: mapping %d pages (huge page and a few normal) (without VMA_POPULATE) at %p...\n",
			       args);

			if (pt_vma_new(&ksatp, ptm.vpn, ptm.pagen,
					 VMA_READ | VMA_WRITE, ptm.vma))
				goto map_failure;

			if (!vpn2vma(kvmas_head, ptm.vpn)) goto map_failure;

			if (!vpn2vma(kvmas_head, ptm.vpn + 512))
				goto map_failure;

			break;

		case access_demand_mega_normal_pages:
			update_args();
			printf("%s: writing to them (involves a demand huge page fault)...\n",
			       args);

			*ptm.virt = 0xa;

			if (pt_get_pte(&ksatp, ptm.vpn, ptm.ptes, NULL) != 3)
				goto map_failure;

			/* This one should not yet be mapped! */
			if (pt_get_pte(&ksatp, ptm.vpn + 512, ptm.ptes, NULL) >
			    0)
				goto map_failure;

			*(ptm.virt + 512 * PAGE_SIZE) = 0xb;

			if (pt_get_pte(&ksatp, ptm.vpn + 512, ptm.ptes, NULL) !=
			    4)
				goto map_failure;

			break;

		case map_populate_mega_normal_pages:
			ptm.vpn = 0xc0ffee;
			ptm.pagen = 634;
			ptm.virt = (char *)vpn2virt(ptm.vpn);
			ptm.unmap = 0;

			update_args();
			printf("%s: testing VMA_POPULATE by mapping %d pages at %p...\n",
			       args);

			if (pt_vma_new(&ksatp, ptm.vpn, ptm.pagen,
					 VMA_READ | VMA_WRITE | VMA_POPULATE,
					 ptm.vma))
				goto map_failure;

			if (pt_get_pte(&ksatp, ptm.vpn, ptm.ptes, NULL) != 4)
				goto map_failure;

			if (pt_get_pte(&ksatp, vpn2nextpageboundary(ptm.vpn, 1),
				       ptm.ptes, NULL) != 3)
				goto map_failure;

			if (pt_get_pte(&ksatp, ptm.vpn + ptm.pagen - 1,
				       ptm.ptes, NULL) != 4)
				goto map_failure;

			break;

		case access_mega_normal_pages:
			ptm.virt += 123 * PAGE_SIZE;
			*ptm.virt = 0x44;

			update_args();
			printf("%s: accessing them (read and write)...\n", args);

			if (*ptm.virt != 0x44) goto map_failure;

			break;

		case map_several_vmas:
			ptm.vpn = 0x664;
			ptm.pagen = 1;

			uintptr_t another_vpn = 0x666;
			size_t another_pagen = 784;
			struct vma *another_vma;

			if (!(another_vma = pt_alloc_kvma()))
				goto should_not_happen;

			const void *extra_args[] = {
				ts[tm.t].name, (void *)ptm.vpn, &ptm.pagen,
				(void *)another_vpn, &another_pagen
			};

			printf("%s: creating two VMAs, one at VPN %p of size %d page(s), and one at VPN %p of size %d page(s)...\n",
			       extra_args);

			if (pt_vma_new(&ksatp, ptm.vpn, ptm.pagen,
					 VMA_READ | VMA_WRITE | VMA_POPULATE,
					 ptm.vma))
				goto map_failure;

			if (pt_vma_new(&ksatp, another_vpn, another_pagen,
					 VMA_READ | VMA_EXEC | VMA_POPULATE,
					 another_vma))
				goto map_failure;

			if (!vpn2vma(kvmas_head, ptm.vpn)) goto map_failure;

			if (pt_get_pte(&ksatp, ptm.vpn, ptm.ptes, NULL) != 4)
				goto map_failure;

			if (!vpn2vma(kvmas_head, another_vpn)) goto map_failure;

			if (pt_get_pte(&ksatp,
				       another_vpn + rand(another_pagen) %
							     another_pagen,
				       ptm.ptes, NULL) < 3)
				goto map_failure;

			break;

		default:
			goto map_failure;
		}

		test_score_update(&tm, ts, tm.t);
		test_print_success(ts[tm.t].name, tm.scores[tm.t],
				   tm.total_score);

		if (ptm.vpn && ptm.unmap) pt_vma_unmap_pages(&ksatp, ptm.vma);

		ptm.unmap = 1;
	}

	return 0;
should_not_happen:
	printstr("\toops, this shouldn't happen?\n");
	return -1;
map_failure:
	test_print_failure(ts[tm.t].name, tm.total_score);
	goto failure;
failure:
	printstr("\tyour page table looks as follows: \n");
	pt_print(ptm.vpn);
	return -1;
}
