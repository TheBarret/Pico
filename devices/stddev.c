#include "stddev.h"
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

/*
 * Host Non-Blocking Keyboard IO Helper
 *  */
static int host_getch_nonblocking(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    /* Disable canonical mode (line buffering) and echoing */
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    /* Restore original terminal attributes */
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    return ch;
}

/*
 * BusDevice Callback Implementations
 *  */

static byte stdio_get_type(void) {
    return DEVICE_TYPE_STDIO;
}

static byte stdio_get_status(void) {
    /* Always ready to send or receive characters */
    return BUS_READY;
}

static void stdio_handle_command(byte cmd) {
    switch (cmd) {
        case STDIO_CMD_FLUSH:
            fflush(stdout);
            break;
        case STDIO_CMD_CLEAR:
            /* ANSI escape sequence to clear screen and move cursor home */
            printf("\033[2J\033[H");
            fflush(stdout);
            break;
        default:
            break;
    }
}

static byte stdio_read_data(void) {
    int ch = host_getch_nonblocking();
    if (ch != -1) {
        return (byte)ch;
    }
    return 0x00; /* Return NULL byte if no character is buffered */
}

static void stdio_write_data(byte value) {
    putchar((char)value);
    /* Auto-flush on newline to keep terminal interactive */
    if (value == '\n') {
        fflush(stdout);
    }
}

/*
 * Device Interface Instance
 *  */

static BusDevice stdio_device_instance = {
    .get_type       = stdio_get_type,
    .get_status     = stdio_get_status,
    .handle_command = stdio_handle_command,
    .read_data      = stdio_read_data,
    .write_data     = stdio_write_data
};

BusDevice* stdio_device_get(void) {
    return &stdio_device_instance;
}
