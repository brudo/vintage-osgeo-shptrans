/** 
 * dstereo.cpp - published as part of SHPTRANS
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


#include "dstereo.h"

#include <math.h>
#include "math87.h"


int DoubleStereographic::setOrigin( double lon, double lat) {
    if (lon < -180 || lon > 180 || lat < -90 || lat > 90) return PROJ_E_PARAM;
  
    lon0 = lon * (PI/180);
    lat0 = lat * (PI/180);

    return (f==0) ? PROJ_SUCCESS : spheroidChanged();
}

double DoubleStereographic::getOriginLongitude() {
    return lon0 * (180/PI);
}

double DoubleStereographic::getOriginLatitude() {
    return lat0 * (180/PI);
}



int DoubleStereographic::spheroidChanged() {
    if (f <= 0) return PROJ_E_SPHEROID;

    double b = a - a * f;

    e_squared=(a*a-b*b)/(a*a);
    e = sqrt(e_squared);
    
    r = sqrt(1.0-e_squared) * a / ( 1.0 - e_squared * square(sin(lat0)) );
    
    c1 = sqrt( 1.0 + (e_squared * pow(cos(lat0),4.0)/(1 - e_squared) ) );

    slon0 = c1*lon0;
    
    sin_slat0 = sin(lat0)/c1;
    slat0 = asin(sin_slat0);
    cos_slat0 = cos(slat0);

    c2 = (
        tan((PI/4) + slat0 / 2) 
      / pow(tan((PI/4) + lat0 / 2) * pow( (1.0-e*sin(lat0))/(1.0+e*sin(lat0)), e/2), c1)
    );

    return PROJ_SUCCESS;
}


int DoubleStereographic::fromLatLong(double *xy, int numPoints)
{
    double lon, lat;
    double slon, slat, delta_slon;
    double sin_delta_slon, cos_delta_slon, sin_slat, cos_slat;

    for (int i=0;i<numPoints;++i,xy+=2) {
        lon = xy[0] * (PI/180);
        lat = xy[1] * (PI/180);
      
        slon = c1*lon;
      
        slat = 2.0 * (atan(c2 * pow(tan((PI/4)+lat/2) * pow((1.0-e*sin(lat))/(1.0+e*sin(lat)), e/2),c1)) - (PI/4));

        sin_cos(slon-slon0, &sin_delta_slon, &cos_delta_slon);
        sin_cos(slat, &sin_slat, &cos_slat);
      
        double common_terms = (2 * k0 * r) / (1.0  +  sin_slat*sin_slat0  +  cos_slat * cos_slat0 * cos_delta_slon);
   
        xy[0] = x0 + common_terms * (cos_slat * sin_delta_slon);
        xy[1] = y0 + common_terms * (sin_slat * cos_slat0  -  cos_slat * sin_slat0 * cos_delta_slon);
    }
 
    return PROJ_SUCCESS;
}


int DoubleStereographic::toLatLong(double *xy, int numPoints)
{

    double dif;
    double sin_delta, cos_delta;
    
    const double errmax = highPrecision ? epsilon / 100000 : epsilon;
    const int maxiter = highPrecision ? 1000 : 100;
  
    for (int i=0;i<numPoints;++i,xy+=2) {

        double dx = (xy[0]-x0) / k0;
        double dy = (xy[1]-y0) / k0;
        double s = sqrt(dx*dx + dy*dy);
      
        if (s <= errmax) {
            // avoid divide-by-zero for this special case
            xy[0] = lon0 * (180/PI);
            xy[1] = lat0 * (180/PI);
            continue;
        }
        
        double cos_beta=dx/s;
        double sin_beta=dy/s;
        
        sin_cos( 2.0*atan(0.5*s/r), &sin_delta, &cos_delta);
        
        double slat = asin(sin_slat0*cos_delta + sin_delta*cos_slat0*sin_beta);
    
        double slon = slon0 + asin( sin_delta*cos_beta / cos(slat) );
        
        // longitude is easy
        double lon = slon/c1;

        // Latitude requires an iterative method to get from latitude on the 
        // sphere (slat) to the latitude on the spheroid (lat).

        double tan_45_plus_half_slat = tan((PI/4) + slat/2);
        
        double lat = slat;
        double dif;
        
        int iter = 0;
        do {
            double e_sin_lat = e * sin(lat);
            double sec_45_plus_half_lat = 1.0 / cos((PI/4) + lat/2);
            double tan_45_plus_half_lat = tan((PI/4) + lat/2);
      
            double fun = c2 * pow( 
                tan_45_plus_half_lat * pow( (1.0-e_sin_lat)/(1.0+e_sin_lat), e/2), c1
            ) - tan_45_plus_half_slat;
      
            double fundif = (
                (c1*c2)
              * pow( tan_45_plus_half_lat * pow( (1.0-e_sin_lat)/(1.0+e_sin_lat), e/2), (c1-1.0) )
              * pow( (1.0-e_sin_lat)/(1.0+e_sin_lat),e/2)
              * (
                    0.5 * square(sec_45_plus_half_lat)
                  - e_squared * cos(lat) * tan_45_plus_half_lat / (1.0-square(e_sin_lat))
                )
            );
            
            dif=fun/fundif;
            lat-=dif;
        } while ((fabs(dif) > errmax) && (++iter < maxiter));
        // repeat, getting closer each time, until further iterations don't matter
        // short circuit if too many iterations, e.g. unprojecting infinity doesn't
        // work
    
        xy[0] = lon * (180/PI);
        xy[1] = lat * (180/PI);
    } // for each coord
  
    return PROJ_SUCCESS;
}

