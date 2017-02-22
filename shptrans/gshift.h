/** 
 * gshift.h - published as part of SHPTRANS
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



#ifndef _GSHIFT_H
#define _GSHIFT_H

#include "intgrid.h"

class GridShift {
  public:
    enum direction {
      apply_forward,apply_reverse
    };
  
    GridShift():gridData(0) {}
    GridShift(char *fname, char*fdatum=0, char*tdatum=0):
        gridData(grid_open(fname, fdatum, tdatum)) {}
  
    ~GridShift() { grid_close(gridData); }
  
    int open(char *fname, char*fdatum=0, char*tdatum=0);
    void close() { grid_close(gridData); gridData=0; }
  
    int forward(double *xy, int count, double const*bbox=0);
    int reverse(double *xy, int count, double const*bbox=0);
  
    int apply(direction d, double *xy, int count, double const*bbox=0) {
        return (d==apply_forward)
          ? forward(xy,count,bbox)
          : reverse(xy,count,bbox);
    }
    
    static bool highPrecision;
  
  protected:
    gridFileType *gridData;
    int subgridHint;
  
  private:
    GridShift(GridShift&);
    void operator=(GridShift&);
};

#endif

/**
 * NTv2 Support:
 * ------------
 *
 * Support for Canadian NTv2 datum conversion as implemented in SHPTRANS
 * was originally based on a program called NADCONV, which was published
 * as part of the Open Geographic Datastore Interface (OGDI) project.
 * The source code for NADCONV was licensed under the following terms:
 *
 *   Copyright (c) 1996 Her Majesty the Queen in Right of Canada.
 *
 *   Permission to use, copy, modify and distribute this software and
 *   its documentation for any purpose and without fee is hereby
 *   granted, provided that the above copyright notice appear in all
 *   copies, that both the copyright notice and this permission notice
 *   appear in supporting documentation, and that the name of Her
 *   Majesty the Queen in Right of Canada not be used in advertising
 *   or publicity pertaining to distribution of the software without
 *   specific, written prior permission. Her Majesty the Queen in Right
 *   of Canada makes no representations about the suitability of this
 *   software for any purpose. It is provided "as is" without express
 *   or implied warranty.
 *
 * The original source code for NADCONV has been enhanced in accordance
 * with those terms.  The enhancements are part of SHPTRANS.
 *
 * In order to exercise rights in SHPTRANS, or in any other work that 
 * incorporates the SHPTRANS implementation of NTv2, you must do so in a 
 * manner that satisfies the conditions of both the NADCONV permission 
 * notice and the SHPTRANS license.
 *
**/
