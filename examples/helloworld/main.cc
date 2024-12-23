#include <eal.hh>
#include <lcore.hh>
#include <iostream>

static int
lcore_hello([[maybe_unused]] void *arg)
{
	std::clog << "hello from core " << dpdk::lcore::id() << std::endl;
	return 0;
}

/* Initialization of Environment Abstraction Layer (EAL). 8< */
int
main(int argc, char **argv)
{
	dpdk::EAL eal{argc, argv};

	for_each(eal.workers(), [] (const auto &lcore) {
		launch(lcore, lcore_hello, nullptr);
	});

	lcore_hello(NULL);

	return 0;
}
