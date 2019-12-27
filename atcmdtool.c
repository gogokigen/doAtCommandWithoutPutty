/*
 *  AT command tool
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <getopt.h>

#define MAX_LINE_LENGTH (4 * 1024)
static char buf[MAX_LINE_LENGTH];

static struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
};
static const char *short_options = "h";

static void help(const char * const argv0)
{
    printf("e.g.\n");
    printf("\n");
    printf("$ echo 'at+cfun?' | %s \n", argv0);
    printf("at+cfun?\n");
    printf("\n");
    printf("+CFUN: 1\n");
    printf("\n");
    printf("OK\n");
    printf("\n");
    printf("\n");
    printf("See also the --help command line option\n");

}

/**
 * Replace '\n' with '\r', aka `tr '\012' '\015'` for a single line.
 * Supports both "\n" and "\r\n" line endings.
 */
static bool tr_lf_cr(const char *s)
{
    char *p;
    p = strchr(s, '\n');
    if (p == NULL || p[1] != '\0') {
        return false;
    }
    if (p > s && p[-1] == '\r') {
        *p = '\0';
    } else {
        *p = '\r';
    }
    return true;
}

static void strip_cr(char *s)
{
    char *from, *to;
    from = to = s;
    while (*from != '\0') {
        if (*from == '\r') {
            from++;
            continue;
        }
        *to++ = *from++;
    }
    *to = '\0';
}

static bool is_final_result(const char * const response)
{
#define STARTS_WITH(a, b) ( strncmp((a), (b), strlen(b)) == 0)
    switch (response[0]) {
        case '+':
            if (STARTS_WITH(&response[1], "CME ERROR:")) {
                return true;
            }
            if (STARTS_WITH(&response[1], "CMS ERROR:")) {
                return true;
            }
            return false;
        case 'B':
            if (strcmp(&response[1], "USY\n") == 0) {
                return true;
            }

            return false;

        case 'E':
            if (strcmp(&response[1], "RROR\n") == 0) {
                return true;
            }
            return false;
        case 'N':
            if (strcmp(&response[1], "O ANSWER\n") == 0) {
                return true;
            }
            if (strcmp(&response[1], "O CARRIER\n") == 0) {
                return true;
            }
            if (strcmp(&response[1], "O DIALTONE\n") == 0) {
                return true;
            }
            return false;
        case 'O':
            if (strcmp(&response[1], "K\n") == 0) {
                return true;
            }
            // fallthrough
        default:
            return false;
    }
}

int main(int argc, char *argv[])
{
    FILE *atcmds;
    FILE *modem;
    FILE *output;
    char *line;
    bool success;
    int res;
    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        switch (c) {
            case 'h':
                help(argv[0]);
                return EXIT_SUCCESS;
            case 0:
                if (strcmp("help", long_options[option_index].name) == 0) {
                    help(argv[0]);
                    return EXIT_SUCCESS;
                }
                break;
            case '?':
                break;
            default:
                fprintf(stderr, "getopt returned character code 0x%02x\n", (unsigned int)c);
        }
    }

//#define INPUT_FILE  ""
//#define OUTPUT_FILE "/dev/console"
#define MODEM_DEVICE "/dev/smd11"

    atcmds = stdin;

    //printf("[DBG] MODEM_DEVICE %s \n", MODEM_DEVICE);
    modem = fopen(MODEM_DEVICE, "r+b");

    if (modem == NULL) {
        fprintf(stderr, "fopen(%s) failed: %s\n", MODEM_DEVICE, strerror(errno));
        return EXIT_FAILURE;
    }

    output = stdout;

    while ((line = fgets(buf, (int)sizeof(buf), atcmds)) != NULL) {
        success = tr_lf_cr(line);
        if (! success) {
            fprintf(stderr, "invalid string: '%s'\n", line);
            return EXIT_FAILURE;
        }
        res = fputs(line, modem);
        if (res < 0) {
            fprintf(stderr, "failed to send '%s' to modem (res = %d)\n", line, res);
            return EXIT_FAILURE;
        }
        do {
            line = fgets(buf, (int)sizeof(buf), modem);
            if (line == NULL) {
                fprintf(stderr, "EOF from modem\n");
                return EXIT_FAILURE;
            }
            strip_cr(line);
            res = fputs(line, output);
            if (res < 0) {
                fprintf(stderr, "failed to write '%s' to output file (res = %d)\n", line, res);
                return EXIT_FAILURE;
            }
        } while (! is_final_result(line));
    }

    res = fclose(modem);
    if (res != 0) {
        fprintf(stderr, "closing modem failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
