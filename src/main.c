#include "perft.h"
#include "uci.h"

#include <stdlib.h>
#include <transposition_table.h>

#define PERFT 0

int main(void)
{
#if PERFT
	perft_run();
#else
	uci_run("the gambit", "mschuurm tridley dsewlia");
#endif
	free_transposition_table();
	return EXIT_SUCCESS;
}
