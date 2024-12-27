#include <algorithm>
#include <iostream>
#include <eal.hh>

static int lcore_hello([[maybe_unused]] void *)
{
	std::clog << "hello from core " << rte_lcore_id() << std::endl;
	return 0;
}

int
main(int argc, char **argv)
{
	dpdk::EAL eal(argc, argv);
	std::ranges::for_each(eal.workers(), [] (auto &&w) {
			w.launch(lcore_hello, nullptr);
	});
	lcore_hello(NULL);

	return 0;
}
