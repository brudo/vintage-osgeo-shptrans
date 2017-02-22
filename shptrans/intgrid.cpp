/** 
 * intgrid.cpp - published as part of SHPTRANS
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

/*
 * There seemed to be some error with gridint; it would fail
 * a significant fraction of the time. -Bruce
 *
 * 1: "static prev".  Removed since >1 grid-shift file
 *    may be used.  This did not fix the problem.
 * 2: problem was that the original implementation was 
 *    corrupting the in-memory structure of the tree.
 *
 * Unfortunately the way the tree was getting corrupted also
 * made the routine faster than it should have been; fixing
 * the corruption slowed the routine drastically.  Therefore
 * I added some code to traverse the tree more efficiently.
 * The result is faster than the original (corrupted) version
 * on my tests.
 *
 * Another optimization involves a "subgrid hint" argument which
 * allows the caller to suggest a candidate sub-grid for the point.
 * Then grid_find will start there (and if it's not right, will go
 * back to the top of the tree).  This extra hint is pretty simple
 * to check, so it doesn't add much time if it's wrong, but it saves
 * a lot if it's right.   See also, gshift.cpp/gshift.h
 */


/*
 ************************************************************
 *
 * Include files
 *
 ************************************************************
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <io.h>
#include <unistd.h>
#include <fcntl.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


/*
 ************************************************************
 *
 * Defines and macros
 *
 ************************************************************
 */
#define INTGRID_INT

#ifdef sun
#define NEED_TO_SWAP
#endif

#ifdef NEED_TO_SWAP
#define BYTESWAP(BUFFER, SIZE, COUNT) byteswap(BUFFER, SIZE, COUNT)
#else
#define BYTESWAP(BUFFER, SIZE, COUNT)
#endif





/*
 ************************************************************
 *
 * Structures
 *
 ************************************************************
 */



/*
 ************************************************************
 *
 * Prototypes
 *
 ************************************************************
 */
#include "intgrid.h"

typedef gridFileType NAD_DATA;



/*
 ************************************************************
 *
 * Functions and procedures
 *
 ************************************************************
 */





#ifdef NEED_TO_SWAP
/*
 * Byte swapping routines required for binary compatibility
 * between platforms.  The ntv2_0.gsb file is in intel byte order.
 */

static void
byteswap( void *buf, short int size, short int num) {
    /*
     * History: 11/92  Todd Stellhorn  - original coding
     */
    short int      i, j;
    unsigned char  *p, tmp;
  
    p = ( unsigned char * ) buf;
    for (i = 0; i < num; i++) {
        for (j = 0; j < ( size >> 1 ); j++) {
            tmp = p[j];
            p[j] = p[size - j - 1];
            p[size - j - 1] = tmp;
        }
        p += size;
    }
}
#endif






#ifdef USE_OLD_FGRID
// Use old string-based fgrid routine (SLOW!).  The only real change
// from the original routine in nadconv.c is a bug-fix.

/*
 * Search the subgrid hierarchy to determine the subgrid that
 * the point falls into.
 */
int grid_find(gridFileType *nadPtr, double const &lon,double const &lat, int) {
    int i;
    int filen;
    int onlimit;
    int ifnd[4];
    char oldnam[12];
    char name[12];
  
    strcpy(name,"NONE");
    filen = 0;
  
    /*
     * Search down through the hierarchical list of
     * sub-grids until a final match is found.
     */
    do {
        strcpy(oldnam,name);
        onlimit = 0;
        for(i=0; i<4; i++) {
            ifnd[i] = -1;
        }
    
        /*
         * At each iteration, search the entire list.
         */
        for (i=0;i<nadPtr->nfiles;i++) {
    
            /*
             * Match on an indentical sub-grid name, and always
             * match on the first record (which contains
             * the boundary of the entire area).
             */
            if ((!strncmp(name,nadPtr->subGrid[i].apgrid,6)) || (nadPtr->nfiles == 1)) {
      
                /*
                 * A simple box test to see if the point is within this
                 * sub-grid.
                 */
                if (
                    (lat >= nadPtr->subGrid[i].alimit[0]) &&
                    (lat <= nadPtr->subGrid[i].alimit[1]) &&
                    (lon >= nadPtr->subGrid[i].alimit[2]) &&
                    (lon <= nadPtr->subGrid[i].alimit[3])
                ) {
    
                    /*
                     * Determine if the point falls on a boundary, give
                     * priority to sub-grids where the point is not
                     * on a boundary.
                     */
                    if ((lat < nadPtr->subGrid[i].alimit[1]) &&
                        (lon < nadPtr->subGrid[i].alimit[3])) {
                        
                        onlimit = 0;
                    } else if ((lat == nadPtr->subGrid[i].alimit[1]) &&
                        (lon < nadPtr->subGrid[i].alimit[3])) {
                          
                        onlimit = 1;
                    } else if ((lat < nadPtr->subGrid[i].alimit[1]) &&
                        (lon == nadPtr->subGrid[i].alimit[3])) {
                          
                        onlimit = 2;
                    } else {
                        onlimit = 3;
                    }
  
                    if (ifnd[onlimit] == -1) {
                        ifnd[onlimit] = i;
                    } else {
                        return -1;
                    }
                }
            }
        }
    
        /*
         * Now choose the grid that has the best fit for this point.
         */
        for (i=0; i<4; i++) {
            if(ifnd[i] > -1) {
                filen = ifnd[i];
                nadPtr->limflag = i;
                strncpy(name, nadPtr->subGrid[filen].anameg, 8);
                break;
            }
        }
    
    } while (strncmp(oldnam,name,8) != 0);
  
    return filen;
}



#else
// NOT USE_OLD_FGRID

/*
 * Search the subgrid hierarchy to determine the subgrid that
 * the point falls into.
 */
int grid_find(gridFileType *nadPtr, double const &lon,double const &lat, int ihint) {
    // special case: just one subgrid.
    // does it need to be a special case?
    if (nadPtr->nfiles==1) {
      subGridType *subGrid = nadPtr->subGrid;
      if ((lat >= subGrid->alimit[0]) &&
        (lat <= subGrid->alimit[1]) &&
        (lon >= subGrid->alimit[2]) &&
        (lon <= subGrid->alimit[3])) {
        
        // which sides does it just touch?
        nadPtr->limflag = (lat == subGrid->alimit[1]);
        if (lon == subGrid->alimit[3]) nadPtr->limflag += 2;
        return 0;
      }
      return -1;
    }
  
  
  
  
    int filen = -1;
    int hint_inject[2] = { ihint, -1};
    
    subGridType *pTest;
    int *piParent = nadPtr->topGrids;
  
  
    // use the hint if it fits completely.
    // this method does one unnecessary box-test
    // but oh well.
    if ( (ihint>=0) && (ihint < nadPtr->nfiles) ) {
      pTest = nadPtr->subGrid + (ihint);
      if ((lat >= pTest->alimit[0]) &&
         (lat < pTest->alimit[1]) &&
         (lon >= pTest->alimit[2]) &&
              (lon < pTest->alimit[3])) {
        piParent = hint_inject;
      }
    }
  
    // find the best "wholly contained" subgrid.
  
    while (*piParent >= 0) {
      pTest = nadPtr->subGrid + (*piParent);
      
      if((lat >= pTest->alimit[0]) &&
         (lat <  pTest->alimit[1]) &&
         (lon >= pTest->alimit[2]) &&
         (lon <  pTest->alimit[3]))
      {
        filen = *piParent;
        if (pTest->children) {
          piParent = pTest->children;
          continue;
        } else {
          break;
        }
      }
      ++piParent;
    }
  
  
  
    if (filen >= 0) {
      nadPtr->limflag = 0;
      return filen;
      
      // according to the file description, I think we are supposed to stop 
      // now if we found anything, at any level, that completely contains 
      // the point.  That is not what fgrid does; it continues looking for 
      // partial matches.  But fgrid has been wrong in a few other places, 
      // so I'll follow my understanding of the spec.
    }
  
    int bestLimit = 4; // the best limflag among the current siblings 
    int parentLimit = 1; // the parent's limflag.  Since we got this far,
                         // it won't be better than 1.
  
    piParent = nadPtr->topGrids;
  
    while (1) {
        pTest = nadPtr->subGrid + (*piParent);
    
        // is it contained at all?
        if (
            (lat >= pTest->alimit[0]) &&
            (lat <= pTest->alimit[1]) &&
            (lon >= pTest->alimit[2]) &&
            (lon <= pTest->alimit[3])
        ) {
          
            // how many sides does it just touch?
            int limit = (lat == pTest->alimit[1]); // 0 or 1.
            if (lon == pTest->alimit[3]) {
                limit += 2;
            }
      
            // is this better than any previous sibling?
            if (limit < bestLimit) {
                filen = *piParent;
                bestLimit = limit;
        
                // is it as good as the parent (best we can hope for)
                if (limit == parentLimit) {
                    // short circuit to children, or to return answer
                    piParent = pTest->children;
                    // parentLimit = limit; //which it already is.
                    if (piParent) {
                        bestLimit = 4;
                        continue;
                    } else {
                        break;
                    }
                }
            }
        }
    
        if (*(++piParent) < 0) {
            // what was the best match, if any?
            if (bestLimit < 4) {
                piParent = nadPtr->subGrid[filen].children;
                parentLimit = bestLimit;
                // move on to the best match's children, if any
                if (piParent) {
                    bestLimit = 4;
                    continue;
                }
            }
            // no match or no children to process.
            break;
        }
    }
  
    if (filen >= 0) nadPtr->limflag = bestLimit;
  
    return filen;
}



#endif


/* grid_eval
 * Interpolate based on best-match subgrid for a given 
 * location.  A subgrid number may be passed; this is a hint 
 * for where to start; it is necessarily the grid that will
 * be used.
 * Return value is the matching subgrid, or less than 0 on
 * error.
 */

int grid_eval(
    gridFileType *nadPtr, 
    double const & lon, double const & lat,
    int filen_hint
) {
    int filen = grid_find(nadPtr, lon, lat, filen_hint);
    if (filen < 0) return GRID_ERROR;
  
    float *se, *sw, *ne, *nw;
  
    subGridType *subgrid = (nadPtr->subGrid + filen);
  
    int row_idx = subgrid->nrows;
    int col_idx = subgrid->ncols;
    double dbl_idx;
  
    int rec_offset;
  
    double ns_frac = 0, ew_frac = 0;
  
    int subgrid_offset = subgrid->astart - 1;
  
    if (nadPtr->limflag < 2) { //if it's not along the top (lat) border
        ns_frac = modf((lat - subgrid->alimit[0]) / subgrid->alimit[4], &dbl_idx);
        row_idx = int(dbl_idx + 1E-12);
    }
  
    if (!(nadPtr->limflag & 1)) { // if it's not along the right (lon) border
        ew_frac = modf((lon - subgrid->alimit[2]) / subgrid->alimit[5], &dbl_idx);
        col_idx = int(dbl_idx + 1E-12);
    }
  
    rec_offset = row_idx * subgrid->ncols + col_idx;
  
    gridDataType *rec = nadPtr->pGrid + (subgrid_offset +  rec_offset);
  
    se = ne = (float*)rec;
    if (ns_frac > 1E-12) {
        ne = (float*)(rec + subgrid->ncols);
    }
    
    sw = se; nw = ne;
    if (ew_frac > 1E-12) { nw += 4; sw += 4; }


#ifndef _WIN32
#define COPY_GRID_RECORD(corner, buf_idx) \
  _lseek(nadPtr->fd, (long)(corner), SEEK_SET); \
  _read(nadPtr->fd, gridBuf + buf_idx, sizeof(gridDataType)); \
  corner = (float*)(gridBuf + buf_idx);
  	 
    // this is not very efficient... but mmap is so problematic
  
    gridDataType gridBuf[4];

    COPY_GRID_RECORD(ne, 0);
    COPY_GRID_RECORD(nw, 1);
    COPY_GRID_RECORD(se, 2);
    COPY_GRID_RECORD(sw, 3);
  
    BYTESWAP(gridBuf, sizeof(float), sizeof(gridBuf) / sizeof(float));
  
#undef COPY_GRID_RECORD  
#endif



#ifndef ACCURACIES
    double sval = se[0] + (sw[0]-se[0])*ew_frac;
    double nval = ne[0] + (nw[0]-ne[0])*ew_frac;
    nadPtr->diflat = sval + (nval-sval)*ns_frac;
  
    sval = se[1] + (sw[1]-se[1])*ew_frac;
    nval = ne[1] + (nw[1]-ne[1])*ew_frac;
    nadPtr->diflon = sval + (nval-sval)*ns_frac;

#else
  
    for (int i=0; i<4; ++i) { 
        double sval = se[i] + (sw[i]-se[i])*ew_frac;
        double nval = ne[i] + (nw[i]-ne[i])*ew_frac;
        nadPtr->shift[i ^ 1] = sval + (nval-sval)*ns_frac;
    }
  
    nadPtr->diflon = nadPtr->shift[0];
    nadPtr->diflat = nadPtr->shift[1];

#endif

    return filen;
}



#define GET_INT(REC, VAR) \
    lseek(nadPtr->fd, ((REC)-1)*16, SEEK_SET); \
    read(nadPtr->fd, &buff, 16); \
    VAR = buff.value.i; \
    BYTESWAP(&VAR, sizeof(VAR), 1);

#define GET_CHAR(REC, VAR) \
    lseek(nadPtr->fd, ((REC)-1)*16, SEEK_SET); \
    read(nadPtr->fd, &buff, 16); \
    strncpy(VAR, buff.value.c, 8); \
    {char *s; for(s=(VAR)+7; s>=(VAR) && (*s==0 || *s == ' '); *s--=0) {}}

#define GET_DBL(REC, VAR) \
    lseek(nadPtr->fd, ((REC)-1)*16, SEEK_SET); \
    read(nadPtr->fd, &buff, 16); \
    VAR = buff.value.d; \
    BYTESWAP(&VAR,sizeof(VAR),1);

/*
 * Initialize the conversion by reading in the subgrid headers.
 */
gridFileType *grid_open(char *filename, char *fdatum, char *tdatum) {
    gridFileType *nadPtr;
    int i, j, count;
    gridDataType buff;
    subGridType *subGrid;
    nadPtr = (NAD_DATA*)calloc(1,sizeof(NAD_DATA));
    if (!nadPtr) {
        return NULL;
    }
    nadPtr->subGrid = NULL;
    nadPtr->hFile = INVALID_HANDLE_VALUE;
  
#if defined(_WINDOWS) || defined(_WIN32)
    nadPtr->fd = open(filename, O_RDONLY | O_BINARY);
#else
    nadPtr->fd = open(filename, O_RDONLY );
#endif
    if (nadPtr->fd < 0) {
        free(nadPtr);
        return NULL;
    }
  
    nadPtr->offset = 0;
  
    GET_INT(1,nadPtr->norecs);
    GET_INT(2,nadPtr->nsrecs);
    GET_INT(3,nadPtr->nfiles);
    GET_CHAR(4,nadPtr->typout);
    GET_CHAR(5,nadPtr->version);
    GET_CHAR(6,nadPtr->fdatum);
    GET_CHAR(7,nadPtr->tdatum);
    GET_DBL(8,nadPtr->fellips[0]);
    GET_DBL(9,nadPtr->fellips[1]);
    GET_DBL(10,nadPtr->tellips[0]);
    GET_DBL(11,nadPtr->tellips[1]);
  
    /*
     * Confirm that the source and target datums are correct.
     * (WBD: Skip this test if inputs are NULL)
     */
    if (
        ((fdatum && strncmp(fdatum, nadPtr->fdatum, 8)) != 0) ||
        ((tdatum && strncmp(tdatum, nadPtr->tdatum, 8)) != 0)
    ) {
      grid_close(nadPtr);
      return NULL;
    }
  
    /*
     * Allocate space to hold the subgrid records.
     */
    nadPtr->subGrid = (subGridType*)calloc(nadPtr->nfiles, sizeof(subGridType));
    if (!nadPtr->subGrid) {
        grid_close(nadPtr);
        return NULL;
    }
  
    /*
     * Loop through all of the subgrid header records.  Skip
     * over the actual adjustment data (don't read the detail until
     * later).
     */
    count = nadPtr->norecs;
    for (i=0;i<nadPtr->nfiles;i++) {
        subGrid = nadPtr->subGrid + i;
        count++;
        /*
         * Read the name of the sub grid, and validate that this
         * is a correct record.
         */
        GET_CHAR(count, subGrid->anameg);
        if (strncmp(buff.title, "SUB_NAME", 8) != 0) {
            grid_close(nadPtr);
            return NULL;
        }
        count++;
        GET_CHAR(count, subGrid->apgrid);
        count += 3;
        /*
         * Read the limits of this subgrid.
         */
        for (j=0; j<6; j++) {
            GET_DBL(count, subGrid->alimit[j]);
            count++;
        }
        GET_INT(count,  subGrid->agscount);
        
        subGrid->ncols = int( (subGrid->alimit[3] - subGrid->alimit[2]) 
                              / subGrid->alimit[5] + 1E-10) + 1;
        subGrid->nrows = int( (subGrid->alimit[1] - subGrid->alimit[0]) 
                              / subGrid->alimit[4] + 1E-10) + 1;
    
        if (subGrid->agscount != (subGrid->nrows*subGrid->ncols)) {
            grid_close(nadPtr);
            return NULL;
        }
    
        nadPtr->subGrid[i].astart = count + 1;
        count += nadPtr->subGrid[i].agscount;
    }
  
  
    if (nadPtr->nfiles != 1) {
        subGrid = nadPtr->subGrid;
        int ichild, nchildren, ntop = 0;
        for (i = 0; i < nadPtr->nfiles; ++i) {
            if (0==strncmp(subGrid[i].apgrid, "NONE",6)) {
                ntop++;
            }
        }
        
        if (ntop==0) { grid_close(nadPtr); return NULL; }
    
        nadPtr->topGrids = (int*)calloc(ntop+1, sizeof(int));
        nadPtr->topGrids[ntop] = -1;
        ntop = 0;
        for (i = 0; i < nadPtr->nfiles; ++i) {
            if (0==strncmp(subGrid[i].apgrid, "NONE",6)) {
                nadPtr->topGrids[ntop++] = i;
            }
        }
        
    
        for (i = 0; i < nadPtr->nfiles; ++i) {
            nchildren = 0;
            for (j = 0; j < nadPtr->nfiles; ++j) {
                if (0==strncmp(subGrid[i].anameg, subGrid[j].apgrid,6)) {
                    nchildren++;
                }
            }
            if (nchildren) {
                subGrid[i].children = (int*)
                                      calloc(nchildren + 1, sizeof(int));
                subGrid[i].children[nchildren] = -1;
                for (j = 0, ichild = 0; j < nadPtr->nfiles; ++j) {
                    if (0 == strncmp(subGrid[i].anameg, subGrid[j].apgrid,6)) {
                        subGrid[i].children[ichild++] = j;
                    }
                }
            }
        }
    }
  
  
  
  
    // OK, switch to Memory mapped I/O to simplify grid_interp.
    // fd is no longer needed.  (Depending on the compiler, fd
    // might work as an hFile but it's safest to just do this.)
  
# ifdef _WIN32
    close(nadPtr->fd);
    nadPtr->fd = -1;
    
    nadPtr->hFile = CreateFile(
        filename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
        0 , OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
    );
  
    if (nadPtr->hFile != INVALID_HANDLE_VALUE) {
        DWORD loSize, hiSize;
        loSize = GetFileSize(nadPtr->hFile, &hiSize);
        nadPtr->nRecs = loSize / 16;
        nadPtr->hMap = CreateFileMapping(
            nadPtr->hFile,0,PAGE_READONLY, 0, loSize, NULL
        );
      
        if (nadPtr->hMap) {
            nadPtr->pGrid = (gridDataType*) MapViewOfFile(nadPtr->hMap, FILE_MAP_READ, 0,0,0);
        }
    }
  
    if (!(nadPtr->pGrid)) {
        if (nadPtr->hMap) CloseHandle(nadPtr->hMap);
        if (nadPtr->hFile != INVALID_HANDLE_VALUE) CloseHandle(nadPtr->hFile);
        free(nadPtr);
        return NULL;
    }
# endif
  
    return nadPtr;
}


/*
 * Close the grid file and release memory.
 */
void grid_close(gridFileType *nadPtr) {
    if (!nadPtr) {
        return;
    }

# ifdef _WIN32
    if (nadPtr->pGrid) UnmapViewOfFile(nadPtr->pGrid);
    if (nadPtr->hMap) CloseHandle(nadPtr->hMap);
    if (nadPtr->hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(nadPtr->hFile);
    }
# endif

    if ((nadPtr->fd != -1) && (nadPtr->fd != 0)) {
        close(nadPtr->fd);
    }

    if (nadPtr->subGrid) {
        for (int i = 0; i < nadPtr->nfiles; i++) {
            free(nadPtr->subGrid[i].children);
        }
        free(nadPtr->subGrid);
    }

    free(nadPtr);
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
