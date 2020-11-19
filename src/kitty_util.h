#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#include <alloca.h>
#include <unistd.h>
#include <termios.h>
#include <poll.h>

/*
 * base64.c : base-64 / MIME encode/decode
 * PUBLIC DOMAIN - Jon Mayo - November 13, 2003
 * $Id: base64.c 156 2007-07-12 23:29:10Z orange $
 */

static const uint8_t base64enc_tab[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int base64_encode(size_t in_len, const uint8_t *in, size_t out_len, char *out)
{
    uint ii, io;
    uint_least32_t v;
    uint rem;

    for(io=0,ii=0,v=0,rem=0;ii<in_len;ii++) {
        uint8_t ch;
        ch=in[ii];
        v=(v<<8)|ch;
        rem+=8;
        while(rem>=6) {
            rem-=6;
            if(io>=out_len) return -1; /* truncation is failure */
            out[io++]=base64enc_tab[(v>>rem)&63];
        }
    }
    if(rem) {
        v<<=(6-rem);
        if(io>=out_len) return -1; /* truncation is failure */
        out[io++]=base64enc_tab[v&63];
    }
    while(io&3) {
        if(io>=out_len) return -1; /* truncation is failure */
        out[io++]='=';
    }
    if(io>=out_len) return -1; /* no room for null terminator */
    out[io]=0;
    return io;
}

/*
 * kitty image protocol
 *
 * outputs base64 encoding of image data in a span_vector
 */

static void kitty_rgba_base64(char cmd, uint32_t id,
    const uint8_t *color_pixels, uint32_t width, uint32_t height)
{
    const size_t chunk_limit = 4096;

    size_t pixel_count = width * height;
    size_t total_size = pixel_count << 2;
    size_t base64_size = ((total_size + 2) / 3) * 4;
    uint8_t *base64_pixels = (uint8_t*)alloca(base64_size+1);


    /* base64 encode the data */
    int ret = base64_encode(total_size, color_pixels, base64_size+1, (char*)base64_pixels);
    if (ret < 0) {
        fprintf(stderr, "error: base64_encode failed: ret=%d\n", ret);
        exit(1);
    }

    /*
     * write kitty protocol RGBA image in chunks no greater than 4096 bytes
     *
     * <ESC>_Gf=32,s=<w>,v=<h>,m=1;<encoded pixel data first chunk><ESC>\
     * <ESC>_Gm=1;<encoded pixel data second chunk><ESC>\
     * <ESC>_Gm=0;<encoded pixel data last chunk><ESC>\
     */

    size_t sent_bytes = 0;
    while (sent_bytes < base64_size) {
        size_t chunk_size = base64_size - sent_bytes < chunk_limit
            ? base64_size - sent_bytes : chunk_limit;
        int cont = !!(sent_bytes + chunk_size < base64_size);
        if (sent_bytes == 0) {
            fprintf(stdout,"\x1B_Gf=32,a=%c,i=%u,s=%d,v=%d,m=%d;",
                cmd, id, width, height, cont);
        } else {
            fprintf(stdout,"\x1B_Gm=%d;", cont);
        }
        fwrite(base64_pixels + sent_bytes, chunk_size, 1, stdout);
        fprintf(stdout, "\x1B\\");
        sent_bytes += chunk_size;
    }
    fflush(stdout);
}

/*
 * kitty terminal helpers
 */

struct line { size_t r; char buf[256]; };
struct kdata { int iid; size_t offset; struct line data; };
struct pos  { int x, y; };

typedef struct line line;
typedef struct kdata kdata;
typedef struct pos pos;

static line recv_term(int timeout)
{
    line l = { 0, { 0 }};
    int r;
    struct pollfd fds[1];

    memset(fds, 0, sizeof(fds));
    fds[0].fd = fileno(stdin);
    fds[0].events = POLLIN;

    if ((r = poll(fds, 1, timeout)) < 0) {
        return l;
    }
    if ((fds[0].revents & POLLIN) == 0) {
        return l;
    }

    l.r = read(0, l.buf, sizeof(l.buf)-1);
    if (l.r > 0) {
        l.buf[l.r] = '\0';
    }

    return l;
}

static line send_term(const char* s)
{
    fputs(s, stdout);
    fflush(stdout);
    return recv_term(-1);
}

static void set_position(int x, int y)
{
    printf("\x1B[%d;%dH", y, x);
    fflush(stdout);
}

static pos get_position()
{
    pos p;
    line l = send_term("\x1B[6n");
    int r = sscanf(l.buf+1, "[%d;%dR", &p.y, &p.x);
    return p;
}

static void hide_cursor()
{
    puts("\x1B[?25l");
}

static void show_cursor()
{
    puts("\x1B[?25h");
}

static kdata parse_kitty(line l)
{
    /*
     * parse kitty response of the form: "\x1B_Gi=<image_id>;OK\x1B\\"
     *
     * NOTE: a keypress can be present before or after the data
     */
    if (l.r < 1) {
        return (kdata) { -1, 0, l };
    }
    char *esc = strchr(l.buf, '\x1B');
    if (!esc) {
        return (kdata) { -1, 0, l };
    }
    ptrdiff_t offset = (esc - l.buf) + 1;
    int iid = 0;
    int r = sscanf(l.buf+offset, "_Gi=%d;OK", &iid);
    if (r != 1) {
        return (kdata) { -1, 0, l };
    }
    return (kdata) { iid, offset, l };
}

/*
 * flip image buffer y-axis
 */
static void flip_buffer_y(uint32_t* buffer, uint32_t width, uint32_t height)
{
    /* Iterate only half the buffer to get a full flip */
    GLint rows = height >> 1;
    uint32_t* scan_line = (uint32_t*)alloca(width * sizeof(uint32_t));

    for (uint32_t rowIndex = 0; rowIndex < rows; rowIndex++)
    {
        memcpy(scan_line, buffer + rowIndex * width, width * sizeof(uint32_t));
        memcpy(buffer + rowIndex * width, buffer + (height - rowIndex - 1) * width, width * sizeof(uint32_t));
        memcpy(buffer + (height - rowIndex - 1) * width, scan_line, width * sizeof(uint32_t));
    }
}
