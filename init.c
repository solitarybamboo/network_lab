#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "gy39.h"
#include "word.h"

void init_all(void) {
    init_serial(COM2, 115200);
}