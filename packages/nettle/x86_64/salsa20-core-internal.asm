C nettle, low-level cryptographics library
C 
C Copyright (C) 2012 Niels Möller
C  
C The nettle library is free software; you can redistribute it and/or modify
C it under the terms of the GNU Lesser General Public License as published by
C the Free Software Foundation; either version 2.1 of the License, or (at your
C option) any later version.
C 
C The nettle library is distributed in the hope that it will be useful, but
C WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
C or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
C License for more details.
C 
C You should have received a copy of the GNU Lesser General Public License
C along with the nettle library; see the file COPYING.LIB.  If not, write to
C the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
C MA 02111-1301, USA.

define(<DST>, <%rdi>)
define(<SRC>, <%rsi>)
define(<COUNT>, <%rdx>)
define(<X0>, <%xmm0>)
define(<X1>, <%xmm1>)
define(<X2>, <%xmm2>)
define(<X3>, <%xmm3>)
define(<T0>, <%xmm4>)
define(<T1>, <%xmm5>)
define(<M0101>, <%xmm6>)
define(<M0110>, <%xmm7>)
define(<M0011>, <%xmm8>)

include_src(<x86_64/salsa20.m4>)

	C _salsa20_core(uint32_t *dst, const uint32_t *src, unsigned rounds)
	.text
	ALIGN(4)
PROLOGUE(_nettle_salsa20_core)
	W64_ENTRY(3, 9)	

	C Load mask registers
	mov	$-1, %eax
	movd	%eax, M0101
	pshufd	$0x09, M0101, M0011	C 01 01 00 00
	pshufd	$0x41, M0101, M0110	C 01 00 00 01
	pshufd	$0x22, M0101, M0101	C 01 00 01 00

	movups	(SRC), X0
	movups	16(SRC), X1
	movups	32(SRC), X2
	movups	48(SRC), X3

	C The original rows are now diagonals.
	SWAP(X0, X1, M0101)
	SWAP(X2, X3, M0101)
	SWAP(X1, X3, M0110)
	SWAP(X0, X2, M0011)	

	shrl	$1, XREG(COUNT)

	ALIGN(4)
.Loop:
	QROUND(X0, X1, X2, X3)
	pshufd	$0x93, X1, X1	C	11 00 01 10 (least sign. left)
	pshufd	$0x4e, X2, X2	C	10 11 00 01
	pshufd	$0x39, X3, X3	C	01 10 11 00

	QROUND(X0, X3, X2, X1)

	C Inverse rotation of the rows
	pshufd	$0x39, X1, X1	C	01 10 11 00
	pshufd	$0x4e, X2, X2	C	10 11 00 01
	pshufd	$0x93, X3, X3	C	11 00 01 10

	decl	XREG(COUNT)
	jnz	.Loop

	SWAP(X0, X2, M0011)	
	SWAP(X1, X3, M0110)
	SWAP(X0, X1, M0101)
	SWAP(X2, X3, M0101)

	movups	(SRC), T0
	movups	16(SRC), T1
	paddd	T0, X0
	paddd	T1, X1
	movups	X0,(DST)
	movups	X1,16(DST)
	movups	32(SRC), T0
	movups	48(SRC), T1
	paddd	T0, X2
	paddd	T1, X3
	movups	X2,32(DST)
	movups	X3,48(DST)
	
	W64_EXIT(3, 9)
	ret
EPILOGUE(_nettle_salsa20_core)
