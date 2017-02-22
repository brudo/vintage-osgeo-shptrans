/** 
 * dstereo.h - published as part of SHPTRANS
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


#ifndef _DSTEREO_H
#define _DSTEREO_H

#include "projbase.h"



class DoubleStereographic: public ProjectionBase {
  protected:
    // Precalculate parameters that depend only on the spheroid
    // (or the origin latitude/longitude)
  
    double e_squared;   // first eccentricity, e squared
    double e;           // e, root of first eccentricity
  
    double lon0;        // origin longitude (radians)
    double lat0;        // origin latitude (radians)

    double r;           // ellipsoid's radius of curvature at origin
  
    double c1;          // constant 1 used in calculating lat/long on the sphere
    double c2;          // constant 2 used in calculating latitude on the sphere
  
    double slon0;       // origin longitude, projected onto the sphere
    double slat0;       // origin latitude, projected onto the sphere
    double sin_slat0;   // sin of origin latitude (frequently used)
    double cos_slat0;   // cos of origin latitude (frequently used)
    
    int spheroidChanged();
  
  public:
  
    int fromLatLong( double *xy, int numPoints);
    int toLatLong( double *xy, int numPoints);
  
    int setOrigin( double lon, double lat);
    
    double getOriginLongitude();
    double getOriginLatitude();
  
    int setOriginPEI() { return setOrigin(-63.0 , 47.25); }
    int setOriginNB()  { return setOrigin(-66.5 , 46.5 ); }
  
    DoubleStereographic() { setScaleFactor(0.999912);}
  
  // Plain ol' data so no need for destructor, copy ctor, oper=
};


#endif

