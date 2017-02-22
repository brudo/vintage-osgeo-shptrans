/** 
 * byteswap.h - published as part of SHPTRANS
 *
 *
 * SHPTRANS is Copyright (c) 1999-2004 Bruce Dodson and others.
 * All rights Reserved.
 * 
 * Permission to use, copy, modify, merge, publish, perform,
 * distribute, sublicense, and/or sell copies of this original work
 * of authorship (the "Software") and derivative works thereof, is 
 * hereby granted free of charge to any person obtaining a copy of 
 * the Software, subject to the following conditions:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimers.
 *
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimers in 
 *    the documentation and/or other materials provided with the 
 *    distribution.
 *
 * 3. Neither the names of the copyright holders, nor the names of any
 *    contributing authors, may be used to endorse or promote products
 *    derived from the Software without specific prior written
 *    permission.
 *
 * 4. If you modify a copy of the Software, or any portion thereof,
 *    you must cause the modified files to carry prominent notices 
 *    stating that you changed the files.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND.
 * THE COPYRIGHT HOLDERS AND CONTRIBUTING AUTHORS DISCLAIM ANY AND
 * ALL WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 * A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTING AUTHORS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING IN ANY WAY OUT OF THE USE
 * OR DISTRIBUTION OF THE SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
**/


#ifndef __BYTESWAP_H
#define __BYTESWAP_H

// Define a few inlines for swapping bytes.
// These functions pass by reference, so the variable passed in is
// changed.  They also return that variable by reference for use in
// nested expressions.

// This would also work, maybe for both signed, unsigned long.
// (( (unsigned long)val >> 24) | (val >> 8 & 0xFF00) | (val & 0xFF00 << 8) | (val<<24 ))

inline
unsigned long &
ByteSwap(unsigned long & val) {
    val = ( (val>>8) & 0x00FF00FFUL ) | ( (val<<8) & 0xFF00FF00UL );
    return val;
}


inline
unsigned long &
ShortSwap(unsigned long & val) {
    val = ( (val>>16) & 0x0000FFFFUL ) | ( (val<<16) & 0xFFFF0000UL );
    return val;
}


inline
unsigned long &
AllSwap(unsigned long & val) {
    return ShortSwap(ByteSwap(val));
}



inline
long &
ByteSwap(long & val) {
    val = ( (val>>8) & 0x00FF00FFL ) | ( (val<<8) & 0xFF00FF00L );
    return val;
}


inline
long &
ShortSwap(long & val) {
    val = ( (val>>16) & 0x0000FFFFL ) | ( (val<<16) & 0xFFFF0000L );
    return val;
}


inline
long &
AllSwap(long & val) {
    return ShortSwap(ByteSwap(val));
}

inline double &
DoubleSwap(double &val) {
    long tmp = AllSwap(((long*)&val)[0]);
    ((long*)&val)[0] = AllSwap(((long*)&val)[1]);
    ((long*)&val)[1] = tmp;
    return val;
}

inline long ReturnSwapped(long val) {
    return ShortSwap(ByteSwap(val));
}

inline long ReturnSwapped(unsigned long val) {
    return ShortSwap(ByteSwap(val));
}

#endif
