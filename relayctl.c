/* relayctl - parallel port relay control.
 *
 * Controls a kit I purchased from Jaycar that connects relays to a parallel
 * port on a PC; the data pins connect a D-type flip-flop, then onto an LED and
 * a relay. The kit was labelled "KV3590 PC Link Automatic Control", and came
 * with Win95/98 drivers.
 *
 * Future additions include:
 * - Only flip the given bits (read the current status from the device)
 * - Better argument parsing
 *
 * Author: Alastair Hughes
 * Contact: <hobbitalastair at yandex dot com>
 */

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ppdev.h>

int open_parport(char *name) {
    /* Open the given parallel port */
    int fd = open(name, O_RDWR);
    if (fd == -1) {
        perror("Failed to open the given port");
        return -errno;
    }

    /* Claim the parallel port */
    if (ioctl(fd, PPCLAIM)) {
        perror("Failed to claim the port");
        close(fd);
        return -errno;
    }

    return fd;
}

int write_parport(int fd, char data) {
    /* Write the given data to the parallel port */

    int err = 0;
    if (ioctl(fd, PPWDATA, &data)) {
        perror("Failed to write the given data");
        err = -errno;
    }

    return err;
}

void close_parport(int fd) {
    /* Close the given parallel port */

    if (ioctl(fd, PPRELEASE)) {
        perror("Warning: failed to release to port");
    }
    close(fd);
}

int write_data(char *name, char data) {
    /* Open the given parallel port, write the data, close, and return */

    /* Open and prepare the parallel port */
    int fd = open_parport(name);
    if (fd < 0) {
        return -fd;
    }

    /* Write some data */
    int err = write_parport(fd, data);

    /* Clean up and return */
    close_parport(fd);
    return err;
}

void help() {
    /* Print the usage message */
    printf("Usage: relayctl [(-f|--file) <file>] [-h|--help] [number ...]\n");
}

void version() {
    /* Print the version */
    printf("relayctl version 0.1.0\n");
}

int parse_number(char *arg, char *data) {
    /* Parse the given argument (hopefully a number) */
    long int offset = 0;
    offset = strtol(arg, NULL, 10);
    if (offset == 0) {
        printf("Unknown argument '%s'!\n", arg);
        return EINVAL;
    } else if (offset > 8 || offset <= 0) {
        printf("Out-of-bounds argument '%ld' (expected between 1 and 8)!\n",
               offset);
        return EINVAL;
    }
    *data = *data | (1 << (offset - 1));

    return 0;
}

int main(int argc, char **argv) {

    char* name = "/dev/parport0";
    char data = 0;

    /* Parse the command line arguments */
    unsigned int i;
    unsigned int state = 0;
    for (i = 1; i < argc; i ++) {
        if (state == 0) {
            /* state == 0; default */
            if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v")) {
                version();
                return 0;
            } else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
                help();
                return 0;
            } else if (!strcmp(argv[i], "--")) {
                /* No more arguments now */
                state = 1;
            } else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file")) {
                /* Get the filename to open */
                state = 2;
            } else {
                if (parse_number(argv[i], &data)) {
                    return EINVAL;
                }
            }
        } else if (state == 1) {
            /* state == 1; no more arguments */
            if (parse_number(argv[i], &data)) {
                return EINVAL;
            }
        } else if (state == 2) {
            /* state == 2; expect a file name */
            name = argv[i];
            state = 1;
        }
    }

    printf("Writing data %u to %s...\n", (unsigned int)data, name);

    return write_data(name, data);
}
