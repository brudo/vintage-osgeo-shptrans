# vintage-osgeo-shptrans
Based on the final state of the CVS repository for SHPTRANS on SourceForge,
this is a projection and datum tranformation utility I developed in the late
1990's and maintained through the 2000's. It uses Esri's open Shapefile format
and had the ability to reproject a shapefile in-place, or very quickly produce
a projected / transformed copy with all attributes preserved exactly.

It was differentiated from command line tools based on ArcInfo Workstation
or the projection capabilities of ArcView GIS or MapObjects in that day,
or what was available in ArcSDE / ArcGIS's Projection Engine, in open source
libraries like PROJ.4, by its early support for Eastern Canadian projections
like Double Stereographic and a very fast engine for NTv2 grid-shift
transformations, which were not yet supported in ArcGIS Desktop.

At first, the only other tool supporting the same projections that I knew of was from
a small independent software developer (Blue Marble), which was some great tools but
had a few drawbacks including relatively slow performance and a non-trivial cost.

Considering that many provincial departments, municipalities and small businesses were
working with limited budgets, but were now being required to update their data and workflows
to modern standard coordinate reference systems, I decided to fill in the gap and make sure
cost was not a factor, and it ended up working so well that it gained some renown in its day.

Eventually, ArcInfo and the full ArcGIS platform did gain full support for everything
SHPTRANS offered to fill in this gap. Meanwhile all of the projections SHPTRANS supports
have gained EPSG IDs and can easily be supported in any modern GIS coordintate projection /
transformation engine. But for nostagic purposes, and in case the code is of interest,
I decided to copy it here for easier access.

### Datum Support
SHPTRANS supports three datums / spheroids: NAD83 (GRS 1980 / WGS 1984), NAD27 (Clarke 1866),
and ATS77 (roughly, WGS 1972). If you want to transform between datums, a grid-shift file is
required in NTv2 GSB format. If you have both the NAD27-NAD83 grid-shift file and an appropriate
ATS77-NAD83 grid shift for your study area, SHPTRANS will convert in either direction between all
three datums. If you have neither grid-shift file, you will not be able to convert between datums,
but you can still use SHPTRANS to go from one projection to another, using the same datum.

### Projection Support
SHPTRANS supports the following projections: UTM (tested for North America; intended to support
all 60 zones in both hemispheres), MTM 3-degree (tested for Atlantic Canada; believed to also
support Quebec), arbitrary TM (any TM projection with 0 as the latitude of origin); as well as
the Double Stereographic projections used in New Brunswick and Prince Edward Island. The input
and output coordinate system can be any of these projections, or lat/long decimal degrees. If a
projection is specified, the map units are taken as metric by default. (However, with the latest
development versions, you can specify other units such as kilometres, feet, or miles.)

One limitation for today's world is that it does not support WebMercator!

## License
The license for SHPTRANS is OSI approved and reads as follows: [(TLDR)](https://tldrlegal.com/license/historic-permission-notice-and-disclaimer-%28hpnd%29)

> SHPTRANS is Copyright (c) 1999-2004 Bruce Dodson and others.
> All rights Reserved.
>
> Permission to use, copy, modify, merge, publish, perform, distribute, sublicense, and/or sell copies of this original work of authorship (the "Software") and derivative works thereof, is hereby granted free of charge to any person obtaining a copy of the Software, subject to the following conditions:
> 
> 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimers.
> 
> 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimers in the documentation and/or other materials provided with the distribution.
> 
> 3. Neither the names of the copyright holders, nor the names of any contributing authors, may be used to endorse or promote products derived from the Software without specific prior written permission.
> 
> 4. If you modify a copy of the Software, or any portion thereof, you must cause the modified files to carry prominent notices stating that you changed the files.
> 
> THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. THE COPYRIGHT HOLDERS AND CONTRIBUTING AUTHORS DISCLAIM ANY AND ALL WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
> 
> IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTING AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING IN ANY WAY OUT OF THE USE OR DISTRIBUTION OF THE SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
