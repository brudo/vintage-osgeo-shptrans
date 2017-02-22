/** 
 * projbase.h - published as part of SHPTRANS
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


#ifndef _PROJBASE_H
#define _PROJBASE_H

enum {
 PROJ_SUCCESS=0,
 PROJ_E_COORD,     // coord out of range
 PROJ_E_SPHEROID,  // invalid or unsupported spheroid
 PROJ_E_PARAM,     // invalid parameters, e.g. zone
 PROJ_E_CALC,      // calculation error
 PROJ_E_SEQUENCE,  // called out of sequence
 PROJ_E_OTHER
};

class ProjectionBase {
protected:
   virtual int spheroidChanged() { return PROJ_SUCCESS; }
public:
   virtual int fromLatLong(double *xy, int count) = 0;
   virtual int toLatLong(double *xy, int count) = 0;

   int setSpheroid(double axis, double flattening);
   double getAxis() { return a; }
   double getFlattening() { return f; }
   
   int setScaleFactor(double scaleFactor);
   double getScaleFactor() { return k0; }
   
   void setFalseOffsets(double falseEasting, double falseNorthing) {
      x0 = falseEasting; y0 = falseNorthing;
   }
   double getFalseEasting() { return x0; }
   double getFalseNorthing() { return y0; }


   ProjectionBase() : a(0), f(0), x0(0), y0(0), k0(1) {}
   virtual ~ProjectionBase() {} // just in case any subclass
                                // might have dynamic resources

   static bool highPrecision;

protected:
   double a;                    // semi-major axis
   double f;                    // flattening
   double x0;                   // false easting
   double y0;                   // false northing
   double k0;                   // scale factor

   static const double epsilon;
};

class NullProjection : public ProjectionBase {
public:
   int fromLatLong(double *,int) { return 0; }
   int toLatLong(double *, int) { return 0; }
protected:
   int spheroidChanged();
};

#endif
