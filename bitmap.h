/*
 * ksm - a really simple and fast x64 hypervisor
 * Copyright (C) 2016, 2017 Ahmed Samy <asamy@protonmail.com>
 *
 * A simple bitmap to easily manage large bitmaps.  Bitmaps can be
 * 32-bit or 64-bit, 64-bit if on GCC or so, because unsigned long is
 * 64-bit there, however, MSVC treats unsigned long as 32-bit, so each
 * entry can handle up to that and is determined via BITMAP_NBITS.
 *
 * For usage examples, see ksm.c: init_msr_bitmaps() / init_io_bitmaps().
 * Those initialize the MSR/IO bitmaps required for the VMM to run (e.g.
 * nested VMMs, etc.)
 * 
 * Some functions from the Linux kernel bitmap implementation:
 *	lib/find_bit.c
 *
 *	Copyright (C) 2004 Red Hat, Inc. All Rights Reserved.
 *	Written by David Howells (dhowells@redhat.com)
 *
 *	Copyright (C) 2008 IBM Corporation
 *	'find_last_bit' is written by Rusty Russell <rusty@rustcorp.com.au>
 *	(Inspired by David Howell's find_next_bit implementation)
 *
 *	Rewritten by Yury Norov <yury.norov@gmail.com> to decrease
 *	size and improve performance, 2015.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#ifndef __BITMAP_H
#define __BITMAP_H

#ifndef __linux__
#ifndef CHAR_BIT
#define CHAR_BIT	8
#endif

#define BITMAP_BITS			(sizeof(unsigned long) * CHAR_BIT)
#define DECLARE_BITMAP(name, bits)	\
	unsigned long name[BITMAP_BITS * bits]

static inline unsigned long pos_bit(unsigned long pos)
{
	return 1 << (pos % BITMAP_BITS);
}

static inline unsigned long bit_at(unsigned long pos)
{
	return pos / BITMAP_BITS;
}

static inline void set_bit(unsigned long pos, unsigned long *bmp)
{
	bmp[bit_at(pos)] |= pos_bit(pos);
}

static inline void clear_bit(unsigned long pos, unsigned long *bmp)
{
	bmp[bit_at(pos)] &= ~pos_bit(pos);
}

static inline bool test_bit(unsigned long pos, volatile const unsigned long *bmp)
{
	return !!(bmp[bit_at(pos)] & pos_bit(pos));
}

static inline unsigned long count_bits(unsigned long count)
{
	return ((count + BITMAP_BITS - 1) / BITMAP_BITS) * sizeof(unsigned long);
}

static inline void clear_bits(unsigned long *bmp, unsigned long count)
{
	memset(bmp, 0x00, count_bits(count));
}

static inline void fill_bits(unsigned long *bmp, unsigned long count, unsigned char bits)
{
	memset(bmp, bits, count_bits(count));
}

static inline unsigned long __ffs(unsigned long x)
{
#ifdef _MSC_VER
	unsigned long i;
	_BitScanForward64(&i, x);
	return i;
#else
	return __builtin_ffs(x);
#endif
}

static inline unsigned long __ffz(unsigned long x)
{
#ifdef _MSC_VER
	unsigned long i;
	_BitScanForward64(&i, ~x);
	return i;
#else
	return __builtin_ffs(~x);
#endif
}

static inline unsigned long find_first_bit(unsigned long *bmp, unsigned long size)
{
	unsigned long i;

	for (i = 0; i *  BITMAP_BITS < size; ++i)
		if (bmp[i])
			return min(i * BITMAP_BITS + __ffs(bmp[i]), size);

	return size;
}

static inline unsigned long find_first_zero_bit(unsigned long *bmp, unsigned long size)
{
	unsigned long i;

	for (i = 0; i *  BITMAP_BITS < size; ++i)
		if (bmp[i] != ~0UL)
			return min(i * BITMAP_BITS + __ffz(bmp[i]), size);

	return size;
}

#endif
#endif
