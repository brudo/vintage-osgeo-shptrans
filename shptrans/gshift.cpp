/** 
 * gshift.cpp - published as part of SHPTRANS
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



#include "gshift.h"
#include <stdio.h>

/*
 * An optimized object-based replacement for the some of
 * the routines in nadconv.c.  gshift interfaces to the
 * routines in intgrid.cpp.

 * Optimizations:
 * 1) Process many points at once, in an array.
 * 2) Take advantage of structure of GSB to avoid
 *    duplicate calls to grid_find.  Vertices are
 *    likely to be close together, and therefore
 *    are likely (but not guaranteed) to fall in
 *    the same subgrid.  This is now encapsulated 
 *    in grid_eval's "subgrid hint" argument.
 *
 *    So that point shapefiles can also get some
 *    benefit, the last subgrid is now remembered
 *    between calls, as part of the object state.
 */


bool GridShift::highPrecision = false;

int GridShift::open(char *fname, char*fdatum, char*tdatum) {
    subgridHint = -1;
    grid_close(gridData);
    gridData = grid_open(fname, fdatum, tdatum);
    return gridData ? GRID_OK : GRID_ERROR;
}

int GridShift::forward(double *xy, int xycount, double const*bbox) {
    if (!gridData) return GRID_ERROR;
    
    int filen = subgridHint; //-1
    int i;
    int errcode;
    int haserr = 0;
 
    for (; --xycount >= 0; xy+=2) {
        double x = (xy[0]) * -3600.0;
        double y = (xy[1]) *  3600.0;
   
        if ((filen = grid_eval(gridData, x, y, filen)) < 0) {
            haserr = 1; filen = -1; 
            continue;
        }
        
        xy[0] = (x + gridData->diflon) / -3600.0;
        xy[1] = (y + gridData->diflat) /  3600.0;
    }
    
    subgridHint = filen;
 
    return haserr ? GRID_ERROR : GRID_OK;
}








/*
 * Perform a reverse adjustment of the point.
 * This is more complicated; it involves figuring out which
 * point would be shifted to *xy if this were a forward
 * transformation.
 */
int GridShift::reverse(double *xy, int xycount, double const*bbox) {
    if (!gridData) return GRID_ERROR;
 
    int filen = subgridHint; //-1;
    
    const int iterations = highPrecision ? 12 : 4;
    
    for (; --xycount >= 0; xy+=2) {
        double x = (xy[0]) * -3600;
        double y = (xy[1]) *  3600;
   
        if ((filen = grid_eval(gridData, x, y, filen)) < 0) {
            subgridHint = -1;
            return GRID_ERROR;
        }
        
        for (int i=0; i<iterations ; i++) {
            // subtract the forward shift
            double xWork = x - gridData->diflon;
            double yWork = y - gridData->diflat;
         
            // what would be the forward shift _there_?
            // (we are looking for a where the forward shift
            // would take us back to the input point)
     
            // get the grid, which is probably the same as the
            // previus grid so short-circuit for that
     
            if ((filen = grid_eval(gridData, xWork, yWork, filen)) < 0) {
                subgridHint = -1;
                return GRID_ERROR;
            }
            
            // we could short-circuit if the deltas get really small, but
            // those final iterations will be fast anyway, thanks to the
            // subgrid hint optimization.
        }
   
        xy[0] = (x - gridData->diflon) / -3600;
        xy[1] = (y - gridData->diflat) /  3600;
    }
    
    subgridHint = filen;
    return GRID_OK;
}



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
