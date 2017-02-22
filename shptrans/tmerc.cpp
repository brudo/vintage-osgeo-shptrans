/** 
 * tmerc.cpp - published as part of SHPTRANS
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

#include "tmerc.h"

#include <math.h>
#include "math87.h"


/* 
  For this module, I started with a UTM implementation code 
  written by Chuck Gantz, placed by Chuck in the public domain.
  From that starting point, I generalized it to any Transverse 
  Mercator projection with lat0=0, rearranged to fit my 
  ProjectionBase harness.  Also moved some of the more 
  expensive common subexpressions that depend only on the
  spheroid, so they're cached in the object state.  Also added 
  a few extra terms for added precision, based on LRIS notes.

  The following notes also appeared in Chuck's code, and are
  still relevant:
    Converts lat/long to UTM coords.  Equations from USGS Bulletin 1532
    East lonitudes are positive, West longitudes are negative.
    North latitudes are positive, South latitudes are negative
    lat and lon are in decimal degrees
    Does not take into account the special UTM zones between 0 degrees and
    36 degrees longitude above 72 degrees latitude and a special zone 32
    between 56 degrees and 64 degrees north latitude
    Written by Chuck Gantz- chuck.gantz@globalstar.com
*/

int TransverseMercator::setCentralMeridian(double cenMerid) {
    lon0 = cenMerid * (PI/180);
    return PROJ_SUCCESS;
}

double TransverseMercator::getCentralMeridian() {
    return lon0 * (180/ PI);
}

int TransverseMercator::spheroidChanged() {
    double b = a - a * f;
    esq=(a*a-b*b)/(a*a);
    e1sq = (esq)/(1-esq);
    e1 = (1-sqrt(1-esq))/(1+sqrt(1-esq));
 
    double epow = esq*esq;
    
    A0 = 1 - esq/4 - epow * 3/64;
    A2 = esq + epow/4;
    A4 = epow;
  
    epow *= esq; //^6
    A0 -= ( epow * 5 / 256);
    A2 += ( epow * 15 / 128);
    A4 += ( epow * 3 / 4);
    A6 = epow;
  
    epow *= esq; //^8
    A0 -= ( epow * 175 / 16384 );
    A2 -= ( epow * 455 / 4096 );
    A4 -= ( epow *  77 / 128 );
    A6 -= ( epow *  41 / 32 );
    A8 = epow * -315 / 131072;
  
    A2 *= (3.0/8.0);
    A4 *= (15.0/256.0);
    A6 *= (35.0/3072.0);
  
    return PROJ_SUCCESS;
}

int TransverseMercator::fromLatLong(double *xy, int count) {
    double N, T, C, Q, Q2, Q3, Q4, Q5, Q6 , M;
    double lat;
    double lon;
  
    double coslat, cossqlat;
    double sinlat, sinsqlat;
    double tanlat, tansqlat;
    
    for (int coordIdx=0; coordIdx<count; ++coordIdx,xy+=2) {
        lon = xy[0] * (PI/180);
        lat = xy[1] * (PI/180);
    
        sin_cos(lat, &sinlat, &coslat);
        tanlat = sinlat / coslat;
        sinsqlat = square(sinlat);
        cossqlat = square(coslat);
        tansqlat = square(tanlat);
        
        N = a/sqrt(1-esq*sinsqlat);
        T = tansqlat;
        C = e1sq*cossqlat;
        Q = coslat*(lon-lon0);
        Q2 = Q * Q;
        Q3 = Q2 * Q;
        Q4 = Q3 * Q;
        Q5 = Q4 * Q;
        Q6 = Q5 * Q;
        
        M=a*(A0*lat-A2*sin(2*lat)+A4*sin(4*lat)-A6*sin(6*lat)+A8*sin(8*lat));
    
        xy[0] = (k0*N*(Q+(1-T+C)*Q3/6
         + (5-18*T+T*T+72*C-58*e1sq)*Q5/120)
         + x0);
        
        xy[1] = (k0*(M+N*tanlat*(Q*Q/2+(5-T+9*C+4*C*C)*Q4/24
         + (61-58*T+T*T+600*C-330*e1sq)*Q6/720))
         + y0);
    }
  
    return PROJ_SUCCESS;
}



int TransverseMercator::toLatLong(double *xy, int count) {
    double N1, T1, C1, R1, D, M;
    double mu, phi1;
  
    double sinphi1, sinsqphi1;
    double cosphi1, cossqphi1;
    double tanphi1, tansqphi1;
    
    double x, y, lat, lon;
    double delta;
    double eff, eff1;
    
    const double errmax = highPrecision ? epsilon / 100000 : epsilon;
    const int maxiter = highPrecision ? 1000 : 100;
    
    for (int coordIdx=0; coordIdx<count; ++coordIdx,xy+=2) {
        x = xy[0] - x0;
        y = xy[1] - y0;
    
        M = y / k0;
        mu = M/(a*A0);
    
        // This is how Chuck calculated phi1.
        phi1 = mu + (3*e1/2-27*e1*e1*e1/32)*sin(2*mu)
            + (21*e1*e1/16-55*e1*e1*e1*e1/32)*sin(4*mu)
            +(151*e1*e1*e1/96)*sin(6*mu);
    
        // But that is just an approximation.  Below,
        // I have added the iterative method from
        // "Explanation of Control Survey Data Terms"
        // to improve precision.  Chuck's approximation
        // is cheaper than an iteration and generally
        // reduces the number of iterations by 1,
        // so I left it in instead of starting with mu.
         
        int iter = 0;
        do {
            sin_cos(2*phi1, &sinphi1, &cosphi1);
            eff = A0*phi1 - A2 * sinphi1 - M/a;
            eff1 = A0 - 2*A2*cosphi1;
      
            sin_cos(4*phi1, &sinphi1, &cosphi1);
            eff += A4*sinphi1;
            eff1 += 4*A4*cosphi1;
      
            sin_cos(6*phi1, &sinphi1, &cosphi1);
            eff -= A6*sinphi1;
            eff1 -= 6*A6*cosphi1;
      
            sin_cos(8*phi1, &sinphi1, &cosphi1);
            eff += A8*sinphi1;
            eff1 -= 8*A8*cosphi1;
      
            delta = eff/eff1;
            
            phi1 -= delta;
        } while ((fabs(delta) > errmax) && (++iter < maxiter));
    
    
        sin_cos(phi1,&sinphi1,&cosphi1);
        tanphi1 = sinphi1 / cosphi1;
        sinsqphi1 = square(sinphi1);
        cossqphi1 = square(cosphi1);
        tansqphi1 = square(tanphi1);
    
        N1 = a/sqrt(1-esq*sinsqphi1);
        T1 = tansqphi1;
        C1 = e1sq*cossqphi1;
        R1 = a*(1-esq)/pow(1-esq*sinsqphi1, 1.5);
        D = x/(N1*k0);
    
        lat = (
            phi1 - (N1*tanphi1/R1) * (
                D*D/2
              - (5+3*T1+10*C1-4*C1*C1-9*e1sq)*D*D*D*D/24
              + (61+90*T1+298*C1+45*T1*T1-252*e1sq-3*C1*C1)*D*D*D*D*D*D/720
            )
        );
        
        xy[1] = lat * (180/PI);
        
        lon = (
            D-(1+2*T1+C1)*D*D*D/6
          + (5-2*C1+28*T1-3*C1*C1+8*e1sq+24*T1*T1)*D*D*D*D*D/120
        ) / cosphi1;
        
        xy[0] = (lon + lon0) * (180/PI);
    }
  
    return PROJ_SUCCESS;
}



int PrepareMTM(TransverseMercator &tm, int zone, int atlantic) {
    if (zone <= 0 || zone > 25) return PROJ_E_PARAM;
    tm.setCentralMeridian(-(zone * 3.0 + 49.5));
    tm.setFalseOffsets( atlantic ? 500000 + 1000000 * zone : 304800, 0);
    tm.setScaleFactor(0.9999);
    return PROJ_SUCCESS;
}

int PrepareUTM(TransverseMercator &tm, int zone, int northern) {
    if (zone <= 0 || zone > 60) return PROJ_E_PARAM;
  
    tm.setScaleFactor(0.9996);
    tm.setCentralMeridian((zone - 1) * 6 - 180 + 3);
    tm.setFalseOffsets(500000, northern ? 0 : 10000000);
    
    return PROJ_SUCCESS;
}

