/*
    pixel-perfect upscales bdf bitmap fonts

    ![](http://hnng.moe/f/ZsR)
    ![](http://hnng.moe/f/Zsa)

    rationale
    -----------------------------------------------------------
    I love the pixel art aesthetics of small bitmap
    fonts, but they're way too small for a 1080p screen and
    bigger fonts don't look quite as good, so I figured I'd
    simply take the small fonts and make every pixel a 2x2
    block

    for now I've only implemented 2x scaling, and while it's
    trivial to add 3x, I'm not gonna bother as this was made
    out of necessity. feel free to PR it though!

    usage
    -----------------------------------------------------------
    ```sh
    curl -O https://raw.githubusercontent.com/Francesco149/bdf2x/master/bdf2x.c
    sudo gcc bdf2x.c -o /usr/bin/bdf2x

    # copy your desired font and convert it to bdf
    cat /usr/share/fonts/X11/misc/ProFont_r400-11.pcf \
    | pcf2bdf -o ProFont_r400-11.bdf

    bdf2x ProFont_r400-11.bdf > 2xProFont_r400-11.bdf

    bdftopcf 2xProFont_r400-11.bdf \
    | gzip \
    | sudo tee /usr/share/fonts/X11/misc/2xProFont_r400-11.pcf.gz \
    > /dev/null

    sudo mkfontdir /usr/share/fonts/X11/misc
    xset fp rehash

    # check your font in xfontsel
    ```

    license
    -----------------------------------------------------------
    this is free and unencumbered software released into the
    public domain.

    refer to the attached UNLICENSE or http://unlicense.org/
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define WHOAMI "bdf2x"
#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

#define WHITESPACE " \r\n\t\f"

static inline
void printver(FILE* f)
{
    fprintf(f, WHOAMI "-%d.%d.%d\n",
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}

static inline
void bdf2x_font_name(char* start)
{
    int i;
    char* end;

    printf("FONT ");
    start += strspn(start, WHITESPACE);

    for (i = 0; i < 15; ++i)
    {
        end = start + strcspn(start, "-");

        switch (i)
        {
        case 7:
        case 8:
        case 12:
            printf("%d-", atoi(start) * 2);
            break;

        case 2:
            printf("2x"); /* fallthrough to print rest of name */
        default:
            fwrite(start, 1, end - start + 1, stdout);
            break;
        case 14:
            fwrite(start, 1, end - start, stdout);
        }

        start = end + 1;
    }

    puts("");
}

static inline
void bdf2x_double_bits(char* start)
{
    char buf[1024];
    char* p = buf;

    for (;isxdigit(*start);)
    {
        uint8_t byte;
        uint16_t doubled;
        char bytestr[3];

        bytestr[0] = *start++;
        bytestr[1] = *start++;
        bytestr[2] = 0;

        byte = (uint8_t)strtol(bytestr, 0, 16);

        doubled = (uint16_t)(
            ((byte & 0x01)<<0) |
            ((byte & 0x01)<<1) |
            ((byte & 0x02)<<1) |
            ((byte & 0x02)<<2) |
            ((byte & 0x04)<<2) |
            ((byte & 0x04)<<3) |
            ((byte & 0x08)<<3) |
            ((byte & 0x08)<<4) |
            ((byte & 0x10)<<4) |
            ((byte & 0x10)<<5) |
            ((byte & 0x20)<<5) |
            ((byte & 0x20)<<6) |
            ((byte & 0x40)<<6) |
            ((byte & 0x40)<<7) |
            ((byte & 0x80)<<7) |
            ((byte & 0x80)<<8)
        );

        p += snprintf(p, buf + 1024 - p - 1, "%04X", doubled);
    }

    puts(buf);
    puts(buf);
}

int main(int argc, char* argv[])
{
    FILE* f;
    char* line = 0;
    size_t line_size = 0;
    int in_bitmap = 0;

    if (argc != 2) {
        printver(stderr);
        fprintf(stderr, "usage: %s file.bdf > file2x.bdf\n",
            argv[0]);
        return 1;
    }

    if (!strcmp(argv[1], "-v")) {
        printver(stdout);
        return 0;
    }

    f = fopen(argv[1], "r");
    if (!f) {
        perror("fopen");
        return 1;
    }

    while (getline(&line, &line_size, f) >= 0)
    {
        char* start;
        char* end;
        size_t n;

        start = line + strspn(line, WHITESPACE);
        end = start + strcspn(line, WHITESPACE);
        n = end - start;

        if (start >= end) {
            goto continueloop;
        }

        if (!strncmp(start, "FONT", n)) {
            bdf2x_font_name(end);
            continue;
        }

        if (!strncmp(start, "CAP_HEIGHT", n) ||
            !strncmp(start, "POINT_SIZE", n) ||
            !strncmp(start, "X_HEIGHT", n) ||
            !strncmp(start, "QUAD_WIDTH", n) ||
            !strncmp(start, "FONT_DESCENT", n) ||
            !strncmp(start, "FONT_ASCENT", n) ||
            !strncmp(start, "SWIDTH", n) ||
            !strncmp(start, "DWIDTH", n) ||
            !strncmp(start, "BBX", n) ||
            !strncmp(start, "SIZE", n) ||
            !strncmp(start, "FONTBOUNDINGBOX", n))
        {
            int only_first;
            int multiplier = 2;

            only_first = !strncmp(start, "SIZE", n);

            fwrite(start, n, 1, stdout);
            start += strcspn(start, WHITESPACE);
            start += strspn(start, WHITESPACE);

            while (start < line + strlen(line))
            {
                printf(" %d", atoi(start) * multiplier);
                start += strcspn(start, WHITESPACE);
                start += strspn(start, WHITESPACE);

                if (only_first) {
                    multiplier = 1;
                }
            }

            puts("");
            continue;
        }

        if (!strncmp(start, "BITMAP", n)) {
            in_bitmap = 1;
        }

        else if (!strncmp(start, "ENDCHAR", n)) {
            in_bitmap = 0;
        }

        else if (in_bitmap) {
            bdf2x_double_bits(start);
            continue;
        }

continueloop:
        printf("%s", line);
    }

    return 0;
}
