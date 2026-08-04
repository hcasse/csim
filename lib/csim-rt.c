/*
 *	CSIM RT functions
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
 *
 *	GLISS is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	GLISS is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "csim-rt.h"

void _panic(const char *message) {
	fprintf(stderr, "ERROR: %s\n", message);
	exit(1);
}

/**
 * Convert float to 32-bits, bit to bits.
 * @param 	Float to convert.
 * @return	Converted float.
 */
uint32_t _f2bits(float f) {
	union { uint32_t i; float f; } u;
	u.f = f;
	return u.i;
}

/**
 * Convert double to 64-bits, bit to bits.
 * @param 	Double to convert.
 * @return	Converted double.
 */
uint64_t _d2bits(double d) {
	union { uint64_t i; double d; } u;
	u.d = d;
	return u.i;
}


uint32_t _rotate_left32(int l, uint32_t v, int r)
{
	uint32_t mask = _mask32(l);
	if (l == 32)
		r = FMOD(r, 32);
	else {
		r = r % l;	// slow modulo
		v = v & mask;	// discarding useless bits
	}
	return ((v << r) | (v >> (l - r))) & mask;
	/*return (((v ) << r) | ((v ) >> (32 - FMOD(r, 32))));*/
}

uint32_t _rotate_left8(int l, uint32_t v, int r)
{
	uint32_t mask = _mask32(l);
	if (l == 8)
		r = FMOD(r, 8);
	else {
		r = r % l;	// slow modulo
		v = v & mask;	// discarding useless bits
	}
	return ((v << r) | (v >> (l - r))) & mask;

	/*r = FMOD(r, 8);
	int len = 8 - r;
	uint32_t mask = _mask32(len);
	return (((v & mask) << r) | ((v & ~mask) >> len));*/
}

uint32_t _rotate_left16(int l, uint32_t v, int r)
{
	uint32_t mask = _mask32(l);
	if (l == 16)
		r = FMOD(r, 16);
	else {
		r = r % l;	// slow modulo
		v = v & mask;	// discarding useless bits
	}
	return ((v << r) | (v >> (l - r))) & mask;

	/*r = FMOD(r, 16);
	int len = 16 - r;
	uint32_t mask = _mask32(len);
	return (((v & mask) << r) | ((v & ~mask) >> len));*/
}

uint64_t _rotate_left64(int l, uint64_t v, int r)
{
	uint64_t mask = _mask64(l);
	if (l == 64)
		r = FMOD(r, 64);
	else {
		r = r % l;	// slow modulo
		v = v & mask;	// discarding useless bits
	}
	return ((v << r) | (v >> (l - r))) & mask;

	/*r = FMOD(r, 64);
	int len = 64 - r;
	uint64_t mask = _mask64(len);
	return (((v & mask) << r) | ((v & ~mask) >> len));*/
}

uint32_t _rotate_right32(int l, uint32_t v, int r)
{
	uint32_t mask = _mask32(l);
	if (l == 32)
		r = FMOD(r, 32);
	else {
		r = r % l;	// slow modulo
		v = v & mask;	// discarding useless bits
	}
	return ((v >> r) | (v << (l - r))) & mask;

	/*r = FMOD(r, 32);
	uint32_t mask = _mask32(r);
	return (((v & mask) << (32 - r)) | ((v & ~mask) >> r));*/
}

uint32_t _rotate_right8(int l, uint32_t v, int r)
{
	uint32_t mask = _mask32(l);
	if (l == 8)
		r = FMOD(r, 8);
	else {
		r = r % l;	// slow modulo
		v = v & mask;	// discarding useless bits
	}
	return ((v >> r) | (v << (l - r))) & mask;
	/*r = FMOD(r, 8);
	uint32_t mask = _mask32(r);
	return (((v & mask) << (8 - r)) | ((v & ~mask) >> r));*/
}

uint32_t _rotate_right16(int l, uint32_t v, int r)
{
	uint32_t mask = _mask32(l);
	if (l == 16)
		r = FMOD(r, 16);
	else {
		r = r % l;	// slow modulo
		v = v & mask;	// discarding useless bits
	}
	return ((v >> r) | (v << (l - r))) & mask;
	/*r = FMOD(r, 16);
	uint32_t mask = _mask32(r);
	return (((v & mask) << (16 - r)) | ((v & ~mask) >> r));*/
}

uint64_t _rotate_right64(int l, uint64_t v, int r)
{
	uint64_t mask = _mask64(l);
	if (l == 64)
		r = FMOD(r, 64);
	else {
		r = r % l;	// slow modulo
		v = v & mask;	// discarding useless bits
	}
	return ((v >> r) | (v << (l - r))) & mask;
	/*r = FMOD(r, 64);
	uint64_t mask = _mask64(r);
	return (((v & mask) << (64 - r)) | ((v & ~mask) >> r));*/
}


int32_t _exp32(int32_t v1, int32_t v2) {
	if(v1 >= 0)
		return _exp32u(v1, v2);
	else if(v2 & 1)
		return -_exp32u(-v1, v2);
	else
		return _exp32u(-v1, v2);
}


uint32_t _exp32u(uint32_t v1, uint32_t v2) {
	/* !!TODO!! may be improved */
	uint32_t res = 0;

	if(!v2)
		res = 1;
	else
		while(v2) {
			if(v2 & 1)
				res += v1;
			v2 >>= 1;
			v1 <<= 1;
		}
	return res;
}


int64_t _exp64(int64_t v1, int64_t v2) {
	if(v1 >= 0)
		return _exp64u(v1, v2);
	else if(v2 & 1)
		return -_exp64u(-v1, v2);
	else
		return _exp64u(-v1, v2);
}

uint64_t _exp64u(uint64_t v1, uint64_t v2) {
	/* !!TODO!! may be improved */
	uint64_t res = 0;

	if(!v2)
		res = 1;
	else
		while(v2) {
			if(v2 & 1)
				res += v1;
			v2 >>= 1;
			v1 <<= 1;
		}
	return res;
}

/* inversion of the n lowest bit of a given value */
uint32_t _invert32(uint32_t v, uint32_t n)
{
	uint32_t res = 0;
	int i = 0;

	/* !!DEBUG!! */
	//printf("_invert32(0X%08X, 0X%08X) => ", v, n);

	for ( ; i < n ; i++)
	{
		res <<= 1;
		if (v & 0x01)
			res |= 0x01;
		v >>= 1;
	}
	/* !!DEBUG!! */
	//printf("0X%08X\n", res);

	return res;
}

uint64_t _invert64(uint64_t v, uint64_t n)
{
	uint64_t res = 0;
	int i = 0;

	/* !!DEBUG!! */
	//printf("_invert64(0X%016LX, 0X%016LX) => ", v, n);

	for ( ; i < n ; i++)
	{
		res <<= 1;
		if (v & 0x01)
			res |= 0x01;
		v >>= 1;
	}
	/* !!DEBUG!! */
	//printf("0X%016LX\n", res);

	return res;
}

float _invertf(float v, uint32_t n)
{
	*((uint32_t *)&v) = _invert32(*(uint32_t *)&v, n);
	return v;
}

double _invertd(double v, uint32_t n)
{
	*((uint64_t *)&v) = _invert64(*(uint64_t *)&v, n);
	return v;
}

/* for these functions no inversion is done and l <= u */
//#ifdef COMPAT
uint32_t _set_field32u(uint32_t v, uint32_t s, int32_t u, int32_t l) {
    uint32_t mask = _mask32(u - l + 1) << l;
    return (v & ~mask) | ((s << l) & mask);
}
//#endif

uint64_t _set_field64u(uint64_t v, uint64_t s, int32_t u, int32_t l) {
	uint64_t mask = _mask64(u - l + 1) << l;

	/* !!DEBUG!! */
	//printf("_set_field64u(0X%016llX, 0X%016llX, 0X%08X, 0X%08X) => ", v, s, u, l);
	//printf("0X%016llX\n", (v & ~mask) | ((s << l) & mask));

	return (v & ~mask) | ((s << l) & mask);
}

float _set_fieldf(float v, uint32_t s, int32_t u, int32_t l) {
	union {
		uint32_t i;
		float d;
	} x;
	x.i = _set_field32u(*((uint32_t *)&v), s, u, l);
	return x.d;
}

double _set_fieldd(double v, uint64_t s, int32_t u, int32_t l) {
	union {
		uint64_t i;
		double d;
	} x;
	x.i = _set_field64u(*((uint64_t *)&v), s, u, l);
	return x.d;
}


/* the read bits are inversed before being assigned to the result, l <= u */
uint32_t _set_field32u_inverted(uint32_t v, uint32_t s, int32_t u, int32_t l) {
	uint32_t mask = _mask32(u - l + 1);
	assert(u >= l);

	/* !!DEBUG!! */
	//printf("_set_field32u_inverted(0X%08X, 0X%08X, 0X%08X, 0X%08X) => ", v, s, u, l);
	//printf("0X%08X\n", (v & ~mask) | (_invert32(s & mask, u-l+1) << l));

	return (v & ~(mask << l)) | (_invert32(s, u-l+1) << l);
}

uint64_t _set_field64u_inverted(uint64_t v, uint64_t s, int32_t u, int32_t l) {
	uint64_t mask = _mask64(u - l + 1);

	/* !!DEBUG!! */
	//printf("_set_field64u_inverted(0X%016llX, 0X%016llX, 0X%08X, 0X%08X) => ", v, s, u, l);
	//printf("0X%016llX\n", (v & ~mask) | (_invert64(s & mask, u-l+1) << l));

	return (v & ~(mask << l)) | (_invert64(s, u-l+1) << l);
}

float _set_fieldf_inverted(float v, uint32_t s, int32_t u, int32_t l) {
	*((uint32_t *)&v) = _set_field32u_inverted(*((uint32_t *)&v), s, u, l);
	return v;
}

double _set_fieldd_inverted(double v, uint64_t s, int32_t u, int32_t l) {
	*((uint64_t *)&v) = _set_field64u_inverted(*((uint64_t *)&v), s, u, l);
	return v;
}


/* will perform v<a..b> = s, s is reversed if needed depending on the value of bit_order (0 : lowermost (default), !=0 : uppermost) */
uint32_t _set_field32u_generic(uint32_t v, uint32_t s, int32_t a, int32_t b, int bit_order)
{
	/* !!DEBUG!! */
	//printf("_set_field32u_generic(0X%08X, 0X%08X, 0X%08X, 0X%08X, %d) ==> \\\\\\\n++++", v, s, a, b, bit_order);

	/* only 1 bit to copy, do not care about bit_order, just do it the most convenient way */
	if (a == b)
		return _set_field32u(v, s, a, b);

	/* lower bits first and uppermost OR upper bits first and lowermost => inversion */
	if (a < b)
	{
		if (bit_order)
			return _set_field32u_inverted(v, s, b, a);
		else
			return _set_field32u(v, s, b, a);
	}
	else
	{
		if (bit_order)
			return _set_field32u(v, s, a, b);
		else
			return _set_field32_inverted(v, s, a, b);
	}
}

uint64_t _set_field64u_generic(uint64_t v, uint64_t s, int32_t a, int32_t b, int bit_order)
{
	/* !!DEBUG!! */
	//printf("_set_field64u_generic(0X%016llX, 0X%016llX, 0X%08X, 0X%08X, %d) ==> \\\\\\\n++++", v, s, a, b, bit_order);

	/* only 1 bit to copy, do not care about bit_order, just do it the most convenient way */
	if (a == b)
		return _set_field64u(v, s, a, b);

	/* lower bits first and uppermost OR upper bits first and lowermost => inversion */
	if (a < b)
	{
		if (bit_order)
			return _set_field64u_inverted(v, s, b, a);
		else
			return _set_field64u(v, s, b, a);
	}
	else
	{
		if (bit_order)
			return _set_field64u(v, s, a, b);
		else
			return _set_field64_inverted(v, s, a, b);
	}
}

float _set_fieldf_generic(float v, uint32_t s, int32_t a, int32_t b, int bit_order) {
	union {
		float f;
		uint32_t i;
	} x;		/* workaround for bug in GCC 4.4.1 */
	x.i = _set_field32u_generic(*((uint32_t *)&v), s, a, b, bit_order);
	return x.f;
}

double _set_fieldd_generic(double v, uint64_t s, int32_t a, int32_t b, int bit_order) {
	union {
		double d;
		uint64_t i;
	} x;		/* workaround for bug in GCC 4.4.1 */
	x.i = _set_field64u_generic(*(uint64_t *)&v, s, a, b, bit_order);
	return x.d;
}


/* for these functions inversion is done and l <= u */
uint32_t _field32u_inverted(uint32_t v, uint32_t u, uint32_t l)
{
	uint32_t mask = _mask32(u - l + 1) << l;

	/* !!DEBUG!! */
	//printf("_field32u_inverted(0X%08X, 0X%08X, 0X%08X) => ", v, u, l);
	//printf("0X%08X\n", _invert32((v & mask) >> l, u-l+1));

	return _invert32((v & mask) >> l, u-l+1);
}

uint64_t _field64u_inverted(uint64_t v, uint32_t u, uint32_t l)
{
	uint64_t mask = _mask64(u - l + 1) << l;

	/* !!DEBUG!! */
	//printf("_field64u_inverted(0X%016LX, 0X%08X, 0X%08X) => ", v, u, l);
	//printf("0X%016LX\n", _invert64((v & mask) >> l, u-l+1));

	return _invert64((v & mask) >> l, u-l+1);
}


uint32_t _fieldf_inverted(float v, uint32_t u, uint32_t l) {
	return _field32u_inverted(*(uint32_t *)&v, u, l);
}

uint64_t _fieldd_inverted(double v, uint32_t u, uint32_t l) {
	return _field64u_inverted(*(uint64_t *)&v, u, l);
}


/* generic functions, will interpret bitfield expression v<a..b> depending on the value of bit_order (0 : lowermost (default), !=0 : uppermost) */
uint32_t _field32u_generic(uint32_t v, uint32_t a, uint32_t b, int bit_order)
{
	/* !!DEBUG!! */
	//printf("_field32u_generic(0X%08X, 0X%08X, 0X%08X, %d) ==> \\\\\\\n++++", v, a, b, bit_order);

	/* only 1 bit to extract, do not care about bit_order, just do it the most convenient way */
	if (a == b)
		return _field32u(v, a, b);

	/* lower bits first and uppermost OR upper bits first and lowermost => inversion */
	if (a < b)
	{
		if (bit_order)
			return _field32u_inverted(v, b, a);
		else
			return _field32u(v, b, a);
	}
	else
	{
		if (bit_order)
			return _field32u(v, a, b);
		else
			return _field32_inverted(v, a, b);
	}
}

uint64_t _field64u_generic(uint64_t v, uint32_t a, uint32_t b, int bit_order)
{
	/* !!DEBUG!! */
	//printf("_field64u_generic(0X%016LX, 0X%08X, 0X%08X, %d) ==> \\\\\\\n++++", v, a, b, bit_order);

	/* only 1 bit to extract, do not care about bit_order, just do it the most convenient way */
	if (a == b)
		return _field64u(v, a, b);

	/* lower bits first and uppermost OR upper bits first and lowermost => inversion */
	if (a < b)
	{
		if (bit_order)
			return _field64u_inverted(v, b, a);
		else
			return _field64u(v, b, a);
	}
	else
	{
		if (bit_order)
			return _field64u(v, a, b);
		else
			return _field64_inverted(v, a, b);
	}
}

uint32_t _fieldf_generic(float v, uint32_t a, uint32_t b, int bit_order)
{
	return _field32u_generic(*(uint32_t *)&v, a, b, bit_order);
}

uint64_t _fieldd_generic(double v, uint32_t a, uint32_t b, int bit_order)
{
	return _field64u_generic(*(uint64_t *)&v, a, b, bit_order);
}


/**
 * Test if the value in the range [0, max].
 * Call _error() else.
 * @param val	Value to test.
 * @param max	Maximum enumration value.
 * @return		Value.
 */
int _enumerate(int val, int max) {
	if(val >= max)
		_panic("coercition to enumeration out of bounds");
	return val;
}


/**
 * Cast the given float to raw 32-bits.
 * @param f		Float to cast.
 * @return		Casted float.
 */
uint32_t _cast_fto32(float f) {
	union {
		uint32_t i;
		float f;
	} d;
	d.f = f;
	return d.i;
}


/**
 * Cast the given float to raw 64-bits.
 * @param f		Float to cast.
 * @return		Casted float.
 */
uint64_t _cast_dto64(double f) {
	union {
		uint64_t i;
		double f;
	} d;
	d.f = f;
	return d.i;
}


/**
 * Cast 32-bits to float.
 * @param i	32-bits integer.
 * @return	Float value.
 */
float _cast_32tof(uint32_t i) {
	union {
		uint32_t i;
		float f;
	} x;
	x.i = i;
	return x.f;
}


/**
 * Cast 64-bits to double.
 * @param i	64-bits integer.
 * @return	Double value.
 */
double _cast_64tod(uint64_t i) {
	union {
		uint64_t i;
		double d;
	} x;
	x.i = i;
	return x.d;
}
