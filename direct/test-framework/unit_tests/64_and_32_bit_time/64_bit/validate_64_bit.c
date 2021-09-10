#include "shared.h"

int random_seed;
int simulate_power_failure = 0;

int main(int argc, char *argv[] ){
	return shared_validate_file(argc, argv);
}
