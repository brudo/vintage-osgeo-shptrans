/** 
 * math87.h - published as part of SHPTRANS
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


#include <math.h>

#ifndef PI
#define PI (3.1415926535897932384626433832795028842)
#endif

inline double square(double n) {
    return n*n;
}

// a minor improvement, but easy to implement.
#if defined(__GNUC__) && defined(_X86_)
inline void sin_cos(double theta, double*sin_theta, double*cos_theta) {
    asm(
        "fldl (%0)\n"
        "fsincos\n"
        "fstpl (%1)\n"
        "fstpl (%2)"
      : 
      : "r" (&theta), "r" (cos_theta), "r" (sin_theta) 
      : "memory" 
    );
}

#else

inline void sin_cos(double theta, double*sin_theta, double*cos_theta) {
    *sin_theta = sin(theta);
    *cos_theta = cos(theta);
}
#endif

