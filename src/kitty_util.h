/*
 * PLEASE LICENSE 11/2020, Michael Clark <michaeljclark@mac.com>
 *
 * All rights to this work are granted for all purposes, with exception of
 * author's implied right of copyright to defend the free use of this work.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

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

static int base64_encode
    (size_t in_len, const uint8_t *in, size_t out_len, char *out)
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
 * zlib compression
 */
#ifdef HAVE_ZLIB

typedef struct zlib_span { const uint8_t *data; size_t len; } zlib_span;

static zlib_span kitty_zlib_compress
    (const uint8_t *data, size_t len, uint32_t compression)
{
    zlib_span result = { NULL, 0 };
    z_stream s = { 0 };
    uint8_t *xdata;
    size_t xlen;
    int ret;

    deflateInit(&s, compression > 1 ? Z_BEST_COMPRESSION : Z_BEST_SPEED);
    xlen = deflateBound(&s, len);
    if (!(xdata = malloc(xlen))) {
        return result;
    }
    s.avail_in = len;
    s.next_in = (uint8_t*)data;
    s.avail_out = xlen;
    s.next_out = xdata;
    do {
        if (Z_STREAM_ERROR == (ret = deflate(&s, Z_FINISH))) {
            free(xdata);
            return result;
        }
    } while (s.avail_out == 0);
    assert(s.avail_in == 0);
    result.data = xdata;
    result.len = s.total_out;
    deflateEnd(&s);

    return result;
}

#endif

/*
 * kitty image protocol
 *
 * outputs base64 encoding of image data
 */

static size_t kitty_send_rgba
    (char cmd, uint32_t id, uint32_t compression,
    const uint8_t *color_pixels, uint32_t width, uint32_t height)
{
    const size_t chunk_limit = 4096;

    size_t pixel_count = width * height;
    size_t total_size = pixel_count << 2;
    const uint8_t *encode_data;
    size_t encode_size;
    zlib_span z;

#ifdef HAVE_ZLIB
#define COMPRESSION_STRING (compression ? ",o=z" : "")
#else
#define COMPRESSION_STRING ""
#endif

#ifdef HAVE_ZLIB
    /*
     * if compression is enabled, compress data before base64 encoding.
     */
    if (compression) {
        z = kitty_zlib_compress(color_pixels, total_size, compression);
        if (!z.data) return 0;
        encode_data = z.data;
        encode_size = z.len;
    } else {
        encode_data = color_pixels;
        encode_size = total_size;
    }
#else
    encode_data = color_pixels;
    encode_size = total_size;
#endif

    size_t base64_size = ((encode_size + 2) / 3) * 4;
    uint8_t *base64_pixels = (uint8_t*)alloca(base64_size+1);

    /* base64 encode the data */
    int ret = base64_encode(encode_size, encode_data, base64_size+1,
        (char*)base64_pixels);
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
            fprintf(stdout,"\x1B_Gf=32,a=%c,i=%u,s=%d,v=%d,m=%d%s;",
                cmd, id, width, height, cont, COMPRESSION_STRING);
        } else {
            fprintf(stdout,"\x1B_Gm=%d;", cont);
        }
        fwrite(base64_pixels + sent_bytes, chunk_size, 1, stdout);
        fprintf(stdout, "\x1B\\");
        sent_bytes += chunk_size;
    }
    fflush(stdout);

#ifdef HAVE_ZLIB
    /* 
     * carefully only free encoded data if compression is enabled, because
     * if compression is not enabled, encoded_data is stack from alloca.
     */
    if (compression) {
        free((void*)encode_data);
    }
#endif
    return encode_size;
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

static line kitty_recv_term(int timeout)
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

static line kitty_send_term(const char* s)
{
    fputs(s, stdout);
    fflush(stdout);
    return kitty_recv_term(-1);
}

static void kitty_set_position(int x, int y)
{
    printf("\x1B[%d;%dH", y, x);
    fflush(stdout);
}

static pos kitty_get_position()
{
    pos p;
    line l = kitty_send_term("\x1B[6n");
    int r = sscanf(l.buf+1, "[%d;%dR", &p.y, &p.x);
    return p;
}

static void kitty_hide_cursor()
{
    puts("\x1B[?25l");
}

static void kitty_show_cursor()
{
    puts("\x1B[?25h");
}

static kdata kitty_parse_response(line l)
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
static void kitty_flip_buffer_y
    (uint32_t* buffer, uint32_t width, uint32_t height)
{
    /* Iterate only half the buffer to get a full flip */
    size_t rows = height >> 1;
    size_t line_size = width * sizeof(uint32_t);
    uint32_t* scan_line = (uint32_t*)alloca(line_size);

    for (uint32_t rowIndex = 0; rowIndex < rows; rowIndex++)
    {
        size_t l1 = rowIndex * width;
        size_t l2 = (height - rowIndex - 1) * width;
        memcpy(scan_line, buffer + l1, line_size);
        memcpy(buffer + l1, buffer + l2, line_size);
        memcpy(buffer + l2, scan_line, line_size);
    }
}

typedef void (*key_cb)(int k);

static key_cb* _get_key_callback()
{
    static key_cb cb;
    return &cb;
}

static void kitty_key_callback(key_cb cb)
{
    *_get_key_callback() = cb;
}

static void kitty_poll_events(int millis)
{
    kdata k;

    /*
     * loop until we see the image id from kitty acknowledging
     * the image upload. keypresses can arrive on their own,
     * or before or after the image id response from kitty.
     */
    do {
        k = kitty_parse_response(kitty_recv_term(millis));
        /* handle keypress present at the beginning of the response */
        if (k.offset == 2) {
            (*_get_key_callback())(k.data.buf[0]);
        }
        /* handle keypress present at the end of the response */
        if (k.offset == 1) {
            char *ok = strstr(k.data.buf + k.offset, ";OK\x1B\\");
            ptrdiff_t ko = (ok != NULL) ? ((ok - k.data.buf) + 5) : 0;
            if (k.data.r > ko) {
                (*_get_key_callback())(k.data.buf[ko]);
            }
        }
        /* handle keypress on its own */
        if (k.offset == 0 && k.data.r == 1) {
            (*_get_key_callback())(k.data.buf[0]);
        }
        /* loop once more for a keypress, if we got our image id */
    } while (k.iid > 0);

}

static struct termios* _get_termios_backup()
{
    static struct termios backup;
    return &backup;
}

static struct termios* _get_termios_raw()
{
    static struct termios raw;
    cfmakeraw(&raw);
    return &raw;
}

static void kitty_setup_termios()
{
    /* save termios and set to raw */
    tcgetattr(0, _get_termios_backup());
    tcsetattr(0, TCSANOW, _get_termios_raw());
}

static void kitty_restore_termios()
{
    /* restore termios */
    tcsetattr(0, TCSANOW, _get_termios_backup());
}