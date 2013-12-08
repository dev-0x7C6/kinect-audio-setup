/*
 * Copyright 2011 Drew Fisher <drew.m.fisher@gmail.com>. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS  ''AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL DREW FISHER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Drew Fisher.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libusb.h>

#include "endian.h"

static libusb_device_handle *dev;
static unsigned int seq;

typedef struct {
	uint32_t magic;
	uint32_t seq;
	uint32_t bytes;
	uint32_t cmd;
	uint32_t write_addr;
	uint32_t unk;
} bootloader_command;

typedef struct {
	uint32_t magic;
	uint32_t seq;
	uint32_t status;
} status_code;

#define LOG(...) printf(__VA_ARGS__)

#if __BYTE_ORDER == __BIG_ENDIAN
static inline uint32_t fn_le32(uint32_t d)
{
	return (d<<24) | ((d<<8)&0xFF0000) | ((d>>8)&0xFF00) | (d>>24);
}
#else
#define fn_le32(x) (x)
#endif

static void dump_bl_cmd(bootloader_command cmd) {
	int i;
	for (i = 0; i < 24; i++)
		LOG("%02X ", ((unsigned char*)(&cmd))[i]);
	LOG("\n");
}

static int get_first_reply(void) {
	unsigned char buffer[512];
	int res;
	int transferred = 0;
	res = libusb_bulk_transfer(dev, 0x81, buffer, 512, &transferred, 0);
	if (res != 0 ) {
		LOG("Error reading first reply: %d\ttransferred: %d (expected %d)\n", res, transferred, 0x60);
		return res;
	}
	LOG("Reading first reply: ");
	int i;
	for (i = 0; i < transferred; ++i) {
		LOG("%02X ", buffer[i]);
	}
	LOG("\n");
	return res;
}

static int get_reply(void) {
	union {
		status_code buffer;
		/* The following is needed because libusb_bulk_transfer might
		 * fail when working on a buffer smaller than 512 bytes.
		 */
		unsigned char dump[512];
	} reply;
	int res;
	int transferred = 0;

	res = libusb_bulk_transfer(dev, 0x81, reply.dump, 512, &transferred, 0);
	if (res != 0 || transferred != sizeof(status_code)) {
		LOG("Error reading reply: %d\ttransferred: %d (expected %zu)\n", res, transferred, sizeof(status_code));
		return res;
	}
	if (fn_le32(reply.buffer.magic) != 0x0a6fe000) {
		LOG("Error reading reply: invalid magic %08X\n", reply.buffer.magic);
		return -1;
	}
	if (fn_le32(reply.buffer.seq) != seq) {
		LOG("Error reading reply: non-matching sequence number %08X (expected %08X)\n", reply.buffer.seq, seq);
		return -1;
	}
	if (fn_le32(reply.buffer.status) != 0) {
		LOG("Notice reading reply: last uint32_t was nonzero: %d\n", reply.buffer.status);
	}

	LOG("Reading reply: ");
	int i;
	for (i = 0; i < transferred; ++i) {
		LOG("%02X ", reply.dump[i]);
	}
	LOG("\n");

	return res;
}

int main(int argc, char** argv) {
	char default_filename[] = "firmware.bin";
	char* filename = default_filename;
	int res = 0;

	if (argc == 2) {
		filename = argv[1];
	}

	FILE* fw = fopen(filename, "rb");
	if (fw == NULL) {
		fprintf(stderr, "Failed to open %s: %s\n", filename, strerror(errno));
		return errno;
	}

	libusb_init(NULL);
	libusb_set_debug(NULL, 3);

	dev = libusb_open_device_with_vid_pid(NULL, 0x045e, 0x02ad);
	if (dev == NULL) {
		fprintf(stderr, "Couldn't open device.\n");
		res = -ENODEV;
		goto fail_libusb_open;
	}

	int current_configuration = 0;
	libusb_get_configuration(dev, &current_configuration);
	if (current_configuration != 1)
		libusb_set_configuration(dev, 1);

	libusb_claim_interface(dev, 0);

	libusb_get_configuration(dev, &current_configuration);
	if (current_configuration != 1) {
		res = -ENODEV;
		goto cleanup;
	}

	seq = 1;

	bootloader_command cmd;
	cmd.magic = fn_le32(0x06022009);
	cmd.seq = fn_le32(seq);
	cmd.bytes = fn_le32(0x60);
	cmd.cmd = fn_le32(0);
	cmd.write_addr = fn_le32(0x15);
	cmd.unk = fn_le32(0);

	LOG("About to send: ");
	dump_bl_cmd(cmd);

	int transferred = 0;

	res = libusb_bulk_transfer(dev, 1, (unsigned char*)&cmd, sizeof(cmd), &transferred, 0);
	if (res != 0 || transferred != sizeof(cmd)) {
		LOG("Error: res: %d\ttransferred: %d (expected %zu)\n", res, transferred, sizeof(cmd));
		goto cleanup;
	}
	res = get_first_reply(); // This first one doesn't have the usual magic bytes at the beginning, and is 96 bytes long - much longer than the usual 12-byte replies.
	res = get_reply(); // I'm not sure why we do this twice here, but maybe it'll make sense later.
	seq++;

	// Split addr declaration and assignment in order to compile as C++,
	// otherwise this would give "jump to label '...' crosses initialization"
	// errors.
	uint32_t addr;
	addr = 0x00080000;
	unsigned char page[0x4000];
	int read;
	do {
		read = fread(page, 1, 0x4000, fw);
		if (read <= 0) {
			break;
		}
		//LOG("");
		cmd.seq = fn_le32(seq);
		cmd.bytes = fn_le32(read);
		cmd.cmd = fn_le32(0x03);
		cmd.write_addr = fn_le32(addr);
		LOG("About to send: ");
		dump_bl_cmd(cmd);
		// Send it off!
		transferred = 0;
		res = libusb_bulk_transfer(dev, 1, (unsigned char*)&cmd, sizeof(cmd), &transferred, 0);
		if (res != 0 || transferred != sizeof(cmd)) {
			LOG("Error: res: %d\ttransferred: %d (expected %zu)\n", res, transferred, sizeof(cmd));
			goto cleanup;
		}
		int bytes_sent = 0;
		while (bytes_sent < read) {
			int to_send = (read - bytes_sent > 512 ? 512 : read - bytes_sent);
			transferred = 0;
			res = libusb_bulk_transfer(dev, 1, &page[bytes_sent], to_send, &transferred, 0);
			if (res != 0 || transferred != to_send) {
				LOG("Error: res: %d\ttransferred: %d (expected %d)\n", res, transferred, to_send);
				goto cleanup;
			}
			bytes_sent += to_send;
		}
		res = get_reply();

		addr += (uint32_t)read;
		seq++;
	} while (read > 0);

	cmd.seq = fn_le32(seq);
	cmd.bytes = fn_le32(0);
	cmd.cmd = fn_le32(0x04);
	cmd.write_addr = fn_le32(0x00080030);
	dump_bl_cmd(cmd);
	transferred = 0;
	res = libusb_bulk_transfer(dev, 1, (unsigned char*)&cmd, sizeof(cmd), &transferred, 0);
	if (res != 0 || transferred != sizeof(cmd)) {
		LOG("Error: res: %d\ttransferred: %d (expected %zu)\n", res, transferred, sizeof(cmd));
		goto cleanup;
	}
	res = get_reply();
	seq++;
	// Now the device reenumerates.

cleanup:
	libusb_close(dev);
fail_libusb_open:
	libusb_exit(NULL);
	fclose(fw);
	return res;
}
