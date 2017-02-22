/** 
 * intgrid.h - published as part of SHPTRANS
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





/*
 * THE ORIGINAL SOURCE FILE (NADCONV.C) CONTAINED THE FOLLOWING CREDITS:
 *
 * NAD to NAD conversion program.
 *
 * Tom Moore, March 1997
 * William Lau, January 1997
 *
 * Based on the Fortran program INTGRID.
 *
 * See also the permission notice from the original source code, 
 * found in the comment block at the bottom of this file.
 */


#define GRID_OK 0
#define GRID_ERROR -1

#ifndef INTGRID_H
#define INTGRID_H


/*
 ************************************************************
 *
 * Prototypes
 *
 ************************************************************
 */

struct gridFileType;


gridFileType *grid_open(char *filename, char *fdatum, char *tdatum);

void grid_close(gridFileType *gridPtr);

int grid_find(gridFileType *gridPtr, double const &x_lon, double const &y_lat, int filen_hint = -1);
int grid_eval(gridFileType *gridPtr, double const &x_lon, double const & y_lat, int filen_hint = -1);


/*
 ************************************************************
 *
 * Structures
 *
 ************************************************************
 */

struct subGridType {
    double alimit[6];
    int agscount, astart;
    char anameg[8], apgrid[8];
    int nrows, ncols;
    int *children;
};

struct gridDataType {
    char title[8];
    union {
        double d;
        int i;
        char c[8];
    } value;
};

struct gridFileType {
    int fd2;
    int fd;
    int norecs;				/* # overview header records */
    int nsrecs;				/* # subfile header records */
    int nfiles;				/* # of subfiles */
    int offset;				/* total offset for the file */
    subGridType *subGrid;
    int *topGrids;
  
    int limflag;			/* value on grid limit */
    char typout[10];			/* grid shift units */
    char version[10];			/* version id */
    char fdatum[10];			/* from datum name */
    char tdatum[10];			/* to datum name */
    double tellips[2];			/* major/minor to axis */
    double fellips[2];			/* major/minor from axis */
    
    double shift[4];
    double diflat;			/* interpolated lat shifts */
    double diflon;			/* interpolated lon shifts */
  
    //double varx;			/* interpolated lat accuracy */
    //double vary;			/* interpolated lon accuracy */
  
    gridDataType *pGrid;
    void *hMap;
    void *hFile;
    int nRecs;
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
