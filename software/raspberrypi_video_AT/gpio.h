extern "C" {

	#include <stdio.h>
	#include <stdint.h>
	#include <stdlib.h>
	#include <fcntl.h>
	#include <sys/mman.h>
	#include <unistd.h>

	void gpio_state(int pin, int state);

}
