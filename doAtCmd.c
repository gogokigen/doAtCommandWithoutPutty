/*
 * Embedded At Command
 * cat /dev/smd7 & echo -e "AT+CFUN?\r\n" > /dev/smd7
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
    {"version", no_argument, NULL, 'V'},
    {NULL, 0, NULL, 0}
};
static const char *short_options = "h";

static void help()
{
    printf("Easy AT commands to the modem\n");
    printf("\n");
    printf("e.g.\n");
    printf("Step 1:./doAtCmd\n");
    printf("Step 2:at+cfun?\n");
    printf("\n");
    printf("+CFUN: 1\n");
    printf("\n");
    printf("OK\n");
    printf("\n");
    printf("Exit: :q\n");
}

/**
 * Replace '\n' or "\r\n" with '\r', aka `tr '\012' '\015'`
 */
static bool replace_ending(const char *s)
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

static void strip_newline(char *s)
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
                help();
                return EXIT_SUCCESS;
            case 0:
                if (strcmp("help", long_options[option_index].name) == 0) {
                    help();
                    return EXIT_SUCCESS;
                }
                break;
            case '?':
                break;
            default:
                fprintf(stderr, "error code 0x%02x\n", (unsigned int)c);
        }
    }

    if ((argc - optind) > 1) {
        help();
        return EXIT_FAILURE;
    }

    atcmds = stdin;
    output = stdout;

/**
* search the right modem devices
* cat /dev/smd7 & echo -e "at\r\n" > /dev/smd7
**/
    char tmp_dev_name[16];
    char MODEM_DEVICE[16];
    for(int i = 1; i < 15; i++){
        char s[4];
        sprintf(s,"%d",i);
        // reset
        memset(tmp_dev_name, '\0', sizeof(tmp_dev_name));
        strncpy(tmp_dev_name, "/dev/smd", sizeof(tmp_dev_name)-1);
        tmp_dev_name[sizeof(tmp_dev_name) - 1] = '\0';
        strncat(tmp_dev_name, s, 4);
        modem = fopen(tmp_dev_name, "r+b");
        if (modem == NULL) {
            continue;
        }
        printf("test AT ... modem: %s\n", tmp_dev_name);
        res = fputs("AT\r\n", modem);
        if (res < 0) {
            fprintf(stderr, "failed to send '%s' to modem (res = %d)\n", "at", res);
            continue;
        }

        do {
            line = fgets(buf, (int)sizeof(buf), modem);
            if (line == NULL) {
                fprintf(stderr, "EOF from modem\n");
                return EXIT_FAILURE;
            }
            strip_newline(line);
            printf("strip_newline at ... %s\n", line);
            if ( 0 == strcmp(line,"OK\n")) {
                memset(MODEM_DEVICE, '\0', sizeof(MODEM_DEVICE));
                strncpy(MODEM_DEVICE, tmp_dev_name, sizeof(MODEM_DEVICE)-1);
                MODEM_DEVICE[sizeof(MODEM_DEVICE) - 1] = '\0';
                printf("test at ... MODEM_DEVICE\n");
                break;
            }
        } while (! is_final_result(line));

        if(strlen(MODEM_DEVICE) > 1) break;
    }

    printf("Found MODEM_DEVICE：%s\n", MODEM_DEVICE);
    printf("Wait your at cmd ...\n");

    while ((line = fgets(buf, (int)sizeof(buf), atcmds)) != NULL) {
        if( 0 == strcmp(line, ":q\n") ){
            printf("leave ... huh?\n");
            return EXIT_FAILURE;
        }
        success = replace_ending(line);
        if (! success) {
            fprintf(stderr, "invalid string: '%s'\n", line);
            return EXIT_FAILURE;
        }
        printf("got your cmd ...\n");
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
            strip_newline(line);
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

