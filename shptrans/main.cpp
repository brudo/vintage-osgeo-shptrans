/**
 * main.cpp - published as part of SHPTRANS
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
   SHPTRANS - Shapefile translation utility

   This program is optimized for -inplace conversions.  In
   inplace mode, on Win32, the conversion is done using
   memory-mapped files, which are very fast.  On other
   platforms and in non-inplace mode, C stdio is used, which
   is also fairly quick.  However, in non-inplace mode it
   does have to copy the DBF file.  On a large shapefile with
   lots of attributes, this can take longer than the
   reprojection.  (However on Win32 the DBF is copied
   asynchronously.)
*/


/*
COMPILE-TIME OPTIONS:

INPLACE_MMAP: Use a memory-mapped file for the
              -inplace mode.  That used to be
              how everything was done, but I
              added stdio-based routines for
              portability.  It uses Win32
              API, but I suppose it could
              also be done on *nix using mmap.
              This is the default, but you can
              say "-DINPLACE_MMAP=0"

FORCE_ALIGN:  Force the double precision numbers
              to be 64-bit aligned when they
              are processed.  This takes extra
              work, copying the data out and
              then back.  It makes little or
              no difference on Intel, but on
              Sparc you seem to get a bus
              error without it.  (Defaults to
              1 on non-Intel hardware.)

BIGENDIAN:    It is assumed that if _X86_ isn't
              defined, the system is big-endian.
              If this is wrong, explicitly
              define BIGENDIAN to be 0.
*/


#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <string.h>
#include <ctype.h>

#include "byteswap.h"
#include "podarray.h"


#if BIGENDIAN
# define BIG_END(a) (a)
# define LITTLE_END(a) ReturnSwapped(a)
#else
# define BIG_END(a) ReturnSwapped(a)
# define LITTLE_END(a) (a)
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define INPLACE_MMAP 1
#endif

#ifdef unix
#include <unistd.h>
#define DBF_FORK
#endif

#ifdef __MINGW32__
#define strncmpi strnicmp
#endif

#include "gshift.h"
#include "projbase.h"
#include "dstereo.h"
#include "tmerc.h"



void showusage(FILE *file) {
    if (file) {
        fputs(
        "Usage: shptrans <inshp> <outshp | -inplace> {-precise}\n"
        "                -from=<proj,datum{,units}> {-fromoffset=x,y} {-fromscale=k}\n"
        "                -to=<proj,datum{,units}> {-tooffset=x,y} {-toscale=k}\n"
        "       shptrans {-usage|-help|-version|-credits|-license}\n"
        ,file);
    }
}

void showversion(FILE *file) {
    if (file) {
        fputs(
        "SHPTRANS - Shapefile coordinate transformation utility version 1.1.4.\n"
        "Copyright (c) 1999-2004 Bruce Dodson and others.  All rights reserved.\n"
        ,file);
    }
}

void showcredits(FILE *file) {
    if (file) {
        fputs(
        "  NTV2 support was originally based on NADCONV by Tom Moore and William Lau.\n"
        "  NADCONV is a verbatim C translation of the FORTRAN program INTGRID.\n"
        "  Copyright (c) 1996 Her Majesty the Queen in Right of Canada.\n\n"

        "  TM projection support started with a public domain UTM conversion\n"
        "  routine by Chuck Gantz.  Precision was improved using formulas from\n"
        "  the LRIS publication \"Explanation of Control Survey Data Terms.\"\n\n"

        "  Double Stereographic support was implemented using formulas supplied by\n"
        "  Derek Davidson, with help from the derivations and other documentation\n"
        "  found in the LRIS publication \"Explanation of Control Survey Data Terms.\"\n\n"

        "  The ArcView 3.x graphical front end was contributed by ESRI Canada Limited.\n"

        ,file);
    }
}


void showlicense(FILE *file) {
    if (file) {
        showversion(file);

        fputs("\n\n"
        "This software is protected by copyright law and is made available\n"
        "under the following license:\n\n"

        "  SHPTRANS is Copyright (c) 1999-2004 Bruce Dodson and others.\n"
        "  All rights reserved.\n\n"

        "  Permission to use, copy, modify, merge, publish, perform,\n"
        "  distribute, sublicense, and/or sell copies of this original work\n"
        "  of authorship (the \"Software\") and derivative works thereof, is\n"
        "  hereby granted free of charge to any person obtaining a copy of\n"
        "  the Software, subject to the following conditions:\n\n"

        "  1. Redistributions of source code must retain the above copyright\n"
        "     notice, this list of conditions and the following disclaimers.\n\n"

        "  2. Redistributions in binary form must reproduce the above copyright\n"
        "     notice, this list of conditions and the following disclaimers in\n"
        "     the documentation and/or other materials provided with the\n"
        "     distribution.\n\n"

        "  3. Neither the names of the copyright holders, nor the names of any\n"
        "     contributing authors, may be used to endorse or promote products\n"
        "     derived from the Software without specific prior written permission.\n\n"

        "  4. If you modify a copy of the Software, or any portion thereof,\n"
        "     you must cause the modified files to carry prominent notices\n"
        "     stating that you changed the files.\n\n"

        "  THE SOFTWARE IS PROVIDED \"AS IS\" WITHOUT WARRANTY OF ANY KIND.\n"
        "  THE COPYRIGHT HOLDERS AND CONTRIBUTING AUTHORS DISCLAIM ANY AND\n"
        "  ALL WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT\n"
        "  LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR\n"
        "  A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n\n"

        "  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTING AUTHORS\n"
        "  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, HOWEVER\n"
        "  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF\n"
        "  CONTRACT, TORT OR OTHERWISE, ARISING IN ANY WAY OUT OF THE USE\n"
        "  OR DISTRIBUTION OF THE SOFTWARE, EVEN IF ADVISED OF THE\n"
        "  POSSIBILITY OF SUCH DAMAGE.\n\n"

        "End of License\n\n\n\n"



        "NTv2 Support:\n"
        "  Support for Canadian NTv2 datum conversion as implemented in SHPTRANS\n"
        "  was originally based on a program called NADCONV, which was published\n"
        "  as part of the Open Geographic Datastore Interface (OGDI) project.\n"
        "  The source code for NADCONV was licensed under the following terms:\n\n"

        "    Copyright (c) 1996 Her Majesty the Queen in Right of Canada.\n\n"

        "    Permission to use, copy, modify and distribute this software and\n"
        "    its documentation for any purpose and without fee is hereby\n"
        "    granted, provided that the above copyright notice appear in all\n"
        "    copies, that both the copyright notice and this permission notice\n"
        "    appear in supporting documentation, and that the name of Her\n"
        "    Majesty the Queen in Right of Canada not be used in advertising\n"
        "    or publicity pertaining to distribution of the software without\n"
        "    specific, written prior permission. Her Majesty the Queen in Right\n"
        "    of Canada makes no representations about the suitability of this\n"
        "    software for any purpose. It is provided \"as is\" without express\n"
        "    or implied warranty.\n\n"

        "  The original source code for NADCONV has been enhanced in accordance\n"
        "  with those terms.  The enhancements are part of SHPTRANS, and are\n"
        "  Copyright (c) 1999-2004 by Bruce Dodson.\n\n"

        "  In order to exercise rights in SHPTRANS, or in any other work that\n"
        "  incorporates the SHPTRANS implementation of NTv2, you must do so in a\n"
        "  manner that satisfies the conditions of both the NADCONV permission\n"
        "  notice and the SHPTRANS license.\n"
        ,file);
    }
}


void showhelp(FILE *file) {
    if (file) {
        showversion(file);
        putc('\n',file);
        showusage(file);

        fputs("\n\n"
        "SUPPORTED PROJECTIONS: UTM, MTM, NBDS, GEO\n"
        "  UTM: Universal Transverse Mercator\n"
        "      Append zone number, e.g. utm20\n"
        "      Append S for southern hemisphere (false northing 10000000m).\n"
        "  MTM: Modified Transverse Mercator (a 3 degree Tranverse Mercator)\n"
        "      Append zone number, e.g. mtm5\n"
        "      Append Q to get a constant false easting of 304800m.\n"
        "      Otherwise Nova Scotia standards are used for easting.\n"
        "  TM: Transverse Mercator (generic).\n"
        "      Append the central longitude for the projection, e.g. tm-59.\n"
        "      Other parameters default to false easting=304800m; scale=0.9999.\n"
        "  NBDS: New Brunswick Double Stereographic\n"
        "      False offsets are inferred from the datum according to\n"
        "      New Brunswick standards.\n"
        "  PEIDS: PEI Double Stereographic\n"
        "      False offsets are inferred from the datum according to\n"
        "      Prince Edward Island standards.\n"
        "  GEO: No projection.  Use geographic latitude/longitude.\n"
        "      If only the datum is specified, this is the default.\n\n"

        "DATUM / SPHEROID SUPPORT: NAD27, NAD83, ATS77.\n"
        "  NAD27: North American Datum 1927 (spheroid: Clarke 1866)\n"
        "  NAD83: North American Datum 1983 (spheroid: GRS 1980)\n"
        "  ATS77: Average Terrerstrial Spheroid (spheroid: ATS 1977)\n\n"
        "  If 'from' and 'to' datum are different, the relevant grid-shift files are\n"
        "  required, and will be located by the program as follows:\n"
        "  * For NAD27 to/from NAD83, the environment variable SHPTRANS_GRIDSHIFT_NTV2\n"
        "    should contain the full path for the NTV2 gridshift file; otherwise the\n"
        "    file (NTV2_0.GSB or May76v20.GSB) should be placed in the same folder as\n"
        "    SHPTRANS.EXE.  If ArcGIS Desktop 8.1 or higher is installed, SHPTRANS will\n"
        "    also look for a gridshift file in the location where ArcGIS expects them:\n"
        "    in pedata\\ntv2\\canada, under the ArcGIS installation directory.\n"
        "  * For ATS77 to/from NAD83, the environment variable SHPTRANS_GRIDSHIFT_7783\n"
        "    should contain the full path for the ATS77 gridshift file; otherwise the\n"
        "    file GS7783.GSB (or one of the newer high-precision GSB files NB7783V2,\n"
        "    PE7783V2, NS7783V2, or NS778301) should be in the same folder as SHPTRANS.\n"
        "    If ArcGIS Desktop 8.1 or higher is installed, SHPTRANS will also look for\n"
        "    the ATS77 gridshift files in the standard location, pedata\\ntv2\\canada.\n"
        "  * For ATS77 to/from NAD27, both of the above are required, and NAD83 is used\n"
        "    as an intermediate step.\n"
        "  * If more than one gridshift file of a given type (ATS77 or NAD27) is found,\n"
        "    SHPTRANS will arbitrarily choose one of them.  However, SHPTRANS always\n"
        "    checks the environment variables before searching the default locations,\n"
        "    so you can avoid ambiguity by specifying the full path to the gridshift\n"
        "    file (including the filename itself) in the environment.  For example, if\n"
        "    you sometimes work in New Brunswick, and other times in PEI, you may need\n"
        "    to set the environment variable SHPTRANS_GRIDSHIFT_7783.\n\n"

        "SUPPORTED UNITS FOR PROJECTED COORDINATE SYSTEMS:\n"
        "    If the coordinate system is projected, the default units are meters.\n"
        "    However, this default can be overridden by specifying the units as the\n"
        "    third item in the -from or -to parameter.  The following units are\n"
        "    supported: meters (m), centimeters (cm), kilometers (km), feet (ft),\n"
        "    inches (in), yards (yd), and miles (mi).  You may specify either the\n"
        "    abbreviation or the full name for the units.  US Survey Feet can also\n"
        "    be specified, using the keyword us_survey_feet or the abbreviation us_ft.\n\n"

        "    If the units are not meters, the default false offsets are rescaled to\n"
        "    whatever units were specified.  e.g. in a TM projection, there is a false\n"
        "    easting of 304800 meters by default.  If the units are changed to feet,\n"
        "    this is automatically rescaled to 1000000 feet.  This might not be what\n"
        "    you want; in that case, see the -fromoffset and -tooffset parameters.\n\n"

        "OTHER OPTIONS:\n"
        "  -fromoffset=xoffset,yoffset: If the source data is in a projected coordinate\n"
        "    system that uses non-standard false offsets, use this option to supply the\n"
        "    correct false easting and northing (respectively, xoffset and yoffset).\n"
        "    The offsets should be given in same units as were specified in the '-from'\n"
        "    parameter (meters by default).\n"
        "  -tooffset=xoffset,yoffset: If the data is to be projected but the default\n"
        "    false offsets are not appropriate, use this option to specify a different\n"
        "    false easting and northing (respectively, xoffset and yoffset).  The\n"
        "    offsets should be given in the same units as were specified in the '-to'\n"
        "    parameter (meters by default).\n"
        "  -fromscale=scale: If the source data is in a projected coordinate system\n"
        "    but is using a non-standard scale factor, use this option to supply the\n"
        "    correct scale factor.\n"
        "  -toscale=scale: If the data is to be projected using a non-default scale\n"
        "    factor, use this option to specify the desired scale factor.\n"
        "  -inplace: Overwrite the original shapefile instead of creating a new one.\n"
        "    This saves space and is faster, but may corrupt the shapefile if an error\n"
        "    occurs.  Do not use this option unless you have a backup or can recreate\n"
        "    the shapefile easily.\n"
        "  -precise: Reverse projections and reverse gridshifts use an iterative method\n"
        "    in which the amount of error reduces with each iteration until it reaches\n"
        "    a predefined tolerance.  The default error tolerance is much lower than\n"
        "    is needed for typical GIS uses.  The -precise option sets an even lower\n"
        "    tolerance, and may yield better results for higher precision datasets or\n"
        "    where the data will be projected back and forth many times.\n"
        "  -verbose: Provide extra diagnostic information, useful for testing.  Some\n"
        "    non-fatal transformation errors may be reported with this option selected.\n"
        ,file);
    }
}




enum shptrans_err {
    err_none = 0,
    err_usage,
    err_params,
    err_gshift,
    err_exists,
    err_create,
    err_magic,
    err_intern,
    err_io,
    err_mem,
    err_abort,
    err_tran = err_intern
};

char * shptrans_err_msg[] = {
    "",
    "Incorrect usage",
    "Bad projection parameters",
    "Bad grid-shift file or file not found",
    "Output file already exists",
    "Could not create output file",
    "Not a shapefile (bad header)",
    "Internal error or bad shapefile",
    "File I/O error or bad shapefile",
    "Out of memory",
    "Cancelled at user request"
};


#define M_PER_FT (0.3048)
#define M_PER_US_FT (12/39.37)

double meters_per_unit(char const *name) {
    if (!name) return 1; // default to meters, if no units specified

    struct unit_t {
        char name[16];
        double meters;
    };

    unit_t units[] = {
        "meters", 1,
        "metres", 1,
        "m"     , 1,

        "kilometers", 1000,
        "kilometres", 1000,
        "km",         1000,

        "centimeters", 0.01,
        "centimetres", 0.01,
        "cm"         , 0.01,

        "feet"  , M_PER_FT,
        "ft"    , M_PER_FT,

        "yards" , M_PER_FT * 36,
        "yd"    , M_PER_FT * 36,

        "miles" , M_PER_FT * 5280,
        "mi"    , M_PER_FT * 5280,

        "inches", M_PER_FT / 12,
        "in"    , M_PER_FT / 12,

        "us_survey_feet", M_PER_US_FT,
        "us_survey_ft", M_PER_US_FT,
        "us_ft", M_PER_US_FT,

        "fathoms", M_PER_FT * 6.0  // one oddball for good measure
    };


    for (int i = 0; i < (sizeof(units) / sizeof(unit_t)); ++i) {
        if (0==strcmpi(name, units[i].name)) {
            return units[i].meters;
        }
    }

    return 0;
}




FILE *shpOut = NULL, *shp=NULL, *shxOut=NULL, *shx=NULL;

HANDLE dbf_thread = NULL;

GridShift gs_nad27, gs_ats77;
DoubleStereographic ds[2];
TransverseMercator tm[2];
NullProjection nullProj;

ProjectionBase *prj[2] = { NULL, NULL };
GridShift *gs[2] = { NULL, NULL };

int inPlace = 0;
int changed = 0;
int verbose = 0;


volatile bool userAbort = false;









FILE *errfile = NULL;

void open_errfile(char *filename) {
    if (errfile) fclose(errfile);
    errfile = fopen(filename,"w");
    if (errfile) setbuf(errfile,NULL);
}


void print_error(char *format, ...) {
    va_list arg_ptr;
    va_start (arg_ptr, format);
    vfprintf (stderr, format, arg_ptr);
    va_end(arg_ptr);

    if (errfile) {
        va_start(arg_ptr, format);
        vfprintf(errfile, format, arg_ptr);
        va_end(arg_ptr);
    }
}




shptrans_err check_files(char *fromShp, char *toShp);
shptrans_err setup_coordsys(char *fromCS,char *toCS, char *fromOffsets,char*toOffsets, char*fromScale,char*toScale);
shptrans_err apply_transform(char *fromShp, char *toShp);


char * stripext(char * fname) {
    char * pc;
    char * ext_start;
    ext_start = fname + strlen(fname);
    for( pc = ext_start-1; pc >= fname && *pc != '\\'; --pc) {
        if (*pc == '.') {
            *pc = '\0';
            ext_start = pc+1;
            break;
        }
    }

    return ext_start;
}

void swapext(char *fname, char const*ext) {
    stripext(fname);
    strcat(fname,".");
    strcat(fname,ext);
}

void normalize_path(char *fname) {
    char work[MAX_PATH];
    char *filepart;

    DWORD result = GetFullPathName(fname, MAX_PATH, work, &filepart);
    if (result && (result<MAX_PATH)) {
        work[MAX_PATH-1]='\0';
        strcpy(fname,work);
    }
}




shptrans_err open_gridshift_file(GridShift &gs, char *envVarName, char *gsbFilenames) {

    char *envFile = getenv(envVarName);

    if ( envFile ) {
        if (err_none == gs.open(envFile)) return err_none;
    }

    if (gsbFilenames) {

        enum { lookInEnvVar, lookInAppDir, lookInArcDir };

        for (int placeToLook = 0; placeToLook < 3 ; ++placeToLook) {

            char fname[MAX_PATH] = "";
            char * filepart = NULL;

            if (placeToLook == lookInEnvVar) {
                if (envFile && (strlen(envFile) + 1 < sizeof(fname))) {
                    strcpy(fname, envFile);
                    strcat(fname, "\\");
                    filepart = fname + strlen(fname);
                }
            } else if (placeToLook == lookInAppDir) {
                DWORD result = GetModuleFileName(GetModuleHandle(NULL), fname, sizeof(fname));
                if (result && ( result < sizeof(fname) )) {
                    filepart = strrchr(fname,'\\');
                    if (filepart) *(++filepart) = 0;
                }
            } else if (placeToLook = lookInArcDir) {
# ifdef _WIN32
                HKEY hKeyArcGIS = NULL; // Check 9 first.  8 and 9 cannot co-exist, so if 9 is there and 8 is too, that's just registry dirt.
                if ( (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\ESRI\\CoreRuntime", 0, KEY_READ, &hKeyArcGIS))
                  || (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\ESRI\\ArcInfo\\Desktop\\8.0", 0, KEY_READ, &hKeyArcGIS)) ) {
                    DWORD cbData = MAX_PATH - 16;
                    if (ERROR_SUCCESS == RegQueryValueEx(hKeyArcGIS, "pedata", 0, 0, (unsigned char*)fname, &cbData)) {
                        strcpy(fname + cbData - 1, "\\ntv2\\canada\\");
                        filepart = fname + strlen(fname);
                    }
                    RegCloseKey(hKeyArcGIS);
                }
# endif
            }

            if (filepart) {
                char *gsbFile;
                char *gsbToken;

                gsbFile = gsbFilenames;
                while ( (gsbToken = strchr(gsbFile,':')) ) {
                    if ((gsbToken > gsbFile) && ((filepart - fname) + (gsbToken-gsbFile) < sizeof(fname)) ) {
                        memcpy(filepart, gsbFile, gsbToken-gsbFile);
                        filepart[gsbToken-gsbFile] = '\0';
                        if (0 == gs.open(fname)) return err_none;
                    }
                    gsbFile = gsbToken+1;
                }

                if (*gsbFile && ((filepart - fname) + strlen(gsbFile) < sizeof(fname)) ) {
                    strcpy(filepart, gsbFile);
                    if (0 == gs.open(fname)) return err_none;
                }
            }

        }
    }

    print_error("Environment variable not set, or GSB file not found: %%%s%%\n",envVarName);
    return err_gshift;
}








int main(int argc, char **argv) {
    SetConsoleTitle("SHPTRANS");

    shptrans_err errcode = err_none;

    int i;

    char *toBase = 0;
    char fromShp[MAX_PATH] = "";
    char toShp[MAX_PATH] = "";
    char filename[MAX_PATH] = "";

    char *fromCS = NULL;
    char *toCS = NULL;

    char *fromOff = NULL;
    char *toOff = NULL;

    char *fromScale = NULL;
    char *toScale = NULL;

    for (i=1; i<argc; ++i) {
        if (!strcmpi(argv[i],"-inplace")) {
            inPlace = 1;
            strcpy(toShp, fromShp);

        } else if (!strcmpi(argv[i],"-precise")) {
            ProjectionBase::highPrecision = true;
            GridShift::highPrecision = true;

        // someone said that equal-signs break on W95 because they change to spaces.
        // Not sure if that's true, but am willing to accomodate just in case.

        } else if ( !fromCS && (!strncmpi(argv[i],"-from=",6) || !strncmpi(argv[i],"-from ",6)) ) {
            fromCS = argv[i] + 6;
        } else if (!fromCS && !strcmpi(argv[i],"-from") && (i+1<argc)) {
            fromCS = argv[++i];

        } else if (!toCS && (!strncmpi(argv[i],"-to=",4) || !strncmpi(argv[i],"-to ",4)) ) {
            toCS = argv[i] + 4;
        } else if (!toCS && !strcmpi(argv[i],"-to") && (i+1<argc)) {
            toCS = argv[++i];

        } else if (!fromOff && !(strncmpi(argv[i],"-fromoffset=",12) || !strncmpi(argv[i],"-fromoffset ",12)) ) {
            fromOff = argv[i] + 12;
        } else if (!fromOff && !strcmpi(argv[i],"-fromoffset") && (i+1<argc)) {
            fromOff = argv[++i];

        } else if (!toOff && !(strncmpi(argv[i],"-tooffset=",10) || !strncmpi(argv[i],"-tooffset ",10)) ) {
            toOff = argv[i] + 10;
        } else if (!toOff && !strcmpi(argv[i],"-tooffset") && (i+1<argc)) {
            toOff = argv[++i];

        } else if (!fromScale && !(strncmpi(argv[i],"-fromscale=",11) || !strncmpi(argv[i],"-fromscale ",11)) ) {
            fromScale = argv[i] + 11;
        } else if (!fromScale && !strcmpi(argv[i],"-fromscale") && (i+1<argc)) {
            fromScale = argv[++i];

        } else if (!toScale && !(strncmpi(argv[i],"-toscale=",9) || !strncmpi(argv[i],"-toscale ",9)) ) {
            toScale = argv[i] + 9;
        } else if (!toScale && !strcmpi(argv[i],"-toscale") && (i+1<argc)) {
            toScale = argv[++i];

        } else if ( !errfile && (!strncmpi(argv[i],"-errfile=",9) || !strncmpi(argv[i],"-errfile ",9)) ) {
            open_errfile(argv[i] + 9);
        } else if (!errfile && !strcmpi(argv[i],"-errfile") && (i+1<argc)) {
            open_errfile(argv[++i]);

        } else if (!strcmpi(argv[i],"-help")) {
            showhelp(stdout); return 0;
        } else if (!strcmpi(argv[i],"-version")) {
            showversion(stdout); return 0;
        } else if (!strcmpi(argv[i],"-credits")) {
            showversion(stdout); putchar('\n'); showcredits(stdout); return 0;
        } else if (!strcmpi(argv[i],"-license")) {
            showlicense(stdout); return 0;
        } else if (!strcmpi(argv[i],"-usage")) {
            showusage(stdout); return 0;

        } else if (!strcmpi(argv[i],"-verbose")) {
            verbose = 1;


        } else if (argv[i][0] == '-') {
            showusage(stderr); showusage(errfile); return err_usage;

        } else if (!*fromShp) {
            strcpy(fromShp,argv[i]);
            if (inPlace) strcpy(toShp, fromShp);
        } else if (!*toShp) {
            struct stat statbuf;

            strcpy(toShp,argv[i]);

            if (stat(toShp,&statbuf)==0) {
                if (statbuf.st_mode & S_IFDIR) {
                    int slen;
                    slen = strlen(toShp);
                    if (toShp[slen-1] != '\\') {
                        //see if this looks like a naked drive letter.
                        if (toShp[slen-1] != ':') {
                            toShp[slen] = '\\';
                            toShp[slen+1] = '\0';
                        }
                    }

                    toBase = strrchr(fromShp,'\\');
                    if (!toBase) toBase = strchr(fromShp,':');
                    if (toBase) ++toBase; else toBase = fromShp;
                    strcat(toShp,toBase);
                }
            }

            stripext(toShp);

        } else {
            showusage(stderr); showusage(errfile); return err_usage;
        }
    }

    //if (fromOff || toOff) { showusage(stderr); return 1; } // overriding offsets not yet supported.

    if (!(fromCS && toCS)) { showusage(stderr); showusage(errfile); return 1; }
    if (!(*fromShp && *toShp)) { showusage(stderr); showusage(errfile); return 1; }

    normalize_path(fromShp);
    normalize_path(toShp);

    if (!inPlace && (0 == strcmpi(fromShp, toShp)) ) {
        fputs("Error: Input and output filenames are the same.\n",stderr);
        return err_exists;
    }

    errcode = check_files(fromShp, toShp);
    if (errcode) return errcode;

    errcode = setup_coordsys(fromCS,toCS,fromOff,toOff,fromScale,toScale);
    if (errcode) return errcode;

    errcode = apply_transform(fromShp, toShp);

    if (errcode) {
        if (shp) fclose(shp);
        if (shx) fclose(shx);

        if (!inPlace) {
            if (shpOut) fclose(shpOut);
            if (shxOut) fclose(shxOut);

            if (dbf_thread) {
                TerminateThread(dbf_thread, errcode);
                CloseHandle(dbf_thread);
            }

            if (errcode != err_exists) {
                //note: errcode is probably create if the input didn't
                //exist, even if the output does.  But I already made
                //sure the 3 output files didn't already exist so this
                //is relatively safe.
                swapext(toShp,"dbf");
                remove(toShp);
                swapext(toShp,"shx");
                remove(toShp);
                swapext(toShp,"shp");
                remove(toShp);
            }
        }

        swapext(fromShp,"shp");
        if (errcode == err_abort) {
            print_error("\nSHPTRANS cancelled at user request.\n");
        } else {
            print_error("\nSHPTRANS Error: %s\n   while processing %s\n",
              shptrans_err_msg[errcode], fromShp);
        }
        if (inPlace) {
            if (changed) {
                print_error(
                "\nThe program aborted part-way through.  Since the '-inplace' option was\n"
                "used, and parts of the shapefile were modified, the file may now be\n"
                "corrupt.  Please restore the shapefile from a recent backup.\n"
                );
            }
        } else {
            if ((errcode != err_create) && (errcode != err_exists)) {
                // remove the corrupt partial files
                bool deleteFailed = false;

                swapext(toShp,"shp");
                if (!DeleteFile(toShp) && (GetLastError()!=ERROR_FILE_NOT_FOUND)) deleteFailed = true;
                swapext(toShp,"shx");
                if (!DeleteFile(toShp) && (GetLastError()!=ERROR_FILE_NOT_FOUND)) deleteFailed = true;
                swapext(toShp,"dbf");
                if (!DeleteFile(toShp) && (GetLastError()!=ERROR_FILE_NOT_FOUND)) deleteFailed = true;

                if (deleteFailed) {
                    swapext(toShp,"shp");
                    print_error(
                    "\nThe output file is incomplete, but could not be deleted.\n"
                    "Please remove by hand: %s, shx, and dbf.\n", toShp);
                }
            }
        }
    }

    if (errfile) fclose(errfile); //it's unbuffered so this isn't really needed.
    return errcode;
}






shptrans_err check_files(char *fromShp, char *toShp) {
    int i;
    char * extns[3] = {"shp","shx","dbf"};
    for (i = 0; i<2;++i) {
        swapext(fromShp, extns[i]);
        if (access(fromShp,(F_OK))!=0) {
            print_error("Error: Input file %s not found.\n",fromShp);
            return err_create;
        }

        if (!inPlace) {
            swapext(toShp,extns[i]);
            if (access(toShp,(F_OK))==0) {
                print_error("Error: Output file %s already exists.\n", toShp);
                return err_exists;
            }
        }
    }

    // if the path strings are different, but the files are the
    // same (e.g. an absolute path and a relative path to same
    // file, it would have failed already with 'already exists'.
    return err_none;
}





shptrans_err setup_coordsys(
    char *fromCS,char *toCS,
    char *fromOffset=0, char *toOffset=0,
    char *fromScale=0, char *toScale=0
) {
    char * coordSys[2] = { fromCS, toCS };
    char * offset[2] = { fromOffset, toOffset };
    char * scale[2] = { fromScale, toScale };

    bool is_peids[2] = {false, false};

    char * from_to[2] = { "from", "to" };
    char * From_To[2] = { "From", "To" };

    double offsetE, offsetN;
    double scaleFact;
    double unitFact;

    char *gsFile;
    char *prjstr[2], *datstr[2], *units[2];

    int zone, extra;

    int i;

    for (i = 0; i<2 ; ++i) {
        prjstr[i] = strtok(coordSys[i],",;");
        datstr[i] = strtok(NULL,  ",;");
        if (!datstr[i]) {
            datstr[i] = prjstr[i];
            prjstr[i] = "geo";
        } else {
            units[i] = strtok(NULL, ",;");
        }
    }

    // I think this was redundant, and anyway was undocumented.
    //if (!datstr[0]) {
    //    datstr[0] = prjstr[0]; prjstr[0] = "geo";
    //    datstr[1] = prjstr[1]; prjstr[1] = "geo";
    //}

    for (i = 0; i < 2; ++i) {
        if (0==strcmpi(prjstr[i],"nbds")) {
            printf("%s NB Double Stereographic",From_To[i]);
            prj[i] = ds+i;
            ds[i].setOriginNB();
            ds[i].setFalseOffsets(300000,800000);
            //false offsets overridden later if nad83
        } else if (0==strcmpi(prjstr[i],"peids")) {
            printf("%s PEI Double Stereographic", From_To[i]);
            prj[i] = ds+i;
            ds[i].setOriginPEI();
            is_peids[i] = true;
            //false offsets will be applied later
        } else if (0==strncmpi(prjstr[i],"utm",3)) {
            zone = atoi(prjstr[i]+3);
            extra = tolower(prjstr[i][strlen(prjstr[i]) - 1]);
            if (0 == PrepareUTM(tm[i], zone, (extra != 's'))) {
                printf("%s UTM zone %d",From_To[i], zone);
                prj[i] = tm+i;
            }
        } else if (0==strncmpi(prjstr[i],"mtm",3)) {
            zone = atoi(prjstr[i]+3);
            extra = tolower(prjstr[i][strlen(prjstr[i]) - 1]);
            if (0 == PrepareMTM(tm[i], zone, (extra != 'q'))) {
                printf("%s MTM zone %d",From_To[i], zone);
                prj[i] = tm+i;
            }
        } else if (0==strncmpi(prjstr[i],"tm",2)) {
            errno = 0;
            double centralMeridian = strtod(prjstr[i]+2,NULL);
            if ((errno == 0) && (centralMeridian <= 180) && (centralMeridian >= -180)) {
                tm[i].setCentralMeridian(centralMeridian);
                tm[i].setScaleFactor(0.9999);
                tm[i].setFalseOffsets(304800,0);
                printf("%s Transverse Mercator (central meridian %.2f)", From_To[i], centralMeridian);
                prj[i] = tm+i;
            }
        } else if (0==strncmpi(prjstr[i],"geo",3)) {
            printf("%s Geographic", From_To[i]);
            prj[i] = &nullProj;
        }

        if (!(prj[i])) {
            print_error("Error: Unrecognized '%s' projection: %s", from_to[i], prjstr[i]);
            return err_params;
        }

        if (0==strcmpi(datstr[i],"nad27")) {
            puts(", NAD27 (Clarke 1866)");
            gs[i] = &gs_nad27;
            prj[i]->setSpheroid(6378206.4,1.0/294.978698199567);

            if (is_peids[i]) ds[i].setFalseOffsets(300000,300000);

        } else if (0==strcmpi(datstr[i],"ats77")) {
            puts(", ATS77 (ATS 1977)");
            gs[i] = &gs_ats77;
            prj[i]->setSpheroid(6378135.0, 1.0/298.257);

            if (is_peids[i]) ds[i].setFalseOffsets(700000,400000);

        } else if (0==strcmpi(datstr[i],"nad83")) {
            puts(", NAD83 (GRS 1980)");
            gs[i] = NULL;
            prj[i]->setSpheroid(6378137.0, 1.0/298.257222099653);

            ds[i].setFalseOffsets(2500000, 7500000);

            if (is_peids[i]) ds[i].setFalseOffsets(400000,800000);

        } else {
            puts("");
            print_error("Error: Unrecognized '%s' datum: %s", from_to[i], datstr[i]);
            return err_params;
        }

        unitFact = 1;
        if (units[i]) {
            if (prj[i] == &nullProj) {
                print_error("Error: Units cannot be overridden for lat/long coordinates.");
                return err_params;
            }

            unitFact = meters_per_unit(units[i]);
            if (unitFact <= 0) {
                print_error("Error: Unrecognized '%s' units: %s", from_to[i], units[i]);
                return err_params;
            }

            printf("  Overriding '%s' units with %s.\n", from_to[i], units[i], unitFact);

            // The units are taken into account by adjusting the false offsets and
            // the scale factor.  See comments below.
        }

        if (offset[i]) {
            if (prj[i] == &nullProj) {
                print_error("Error: False offsets cannot be specified for lat/long coordinates.");
                return err_params;
            }

            char * offtok = strtok(offset[i],",;");
            offsetE = (offtok && *offtok) ? atof(offtok) : prj[i]->getFalseEasting() / unitFact;
            offtok = strtok(NULL, ",;");
            offsetN = offtok ? atof(offtok) : prj[i]->getFalseNorthing() / unitFact;
            printf("  Overriding '%s' offset with %.3f,%.3f (units=%s)\n",from_to[i], offsetE, offsetN, units[i] ? units[i] : "m");

            // The offsets are assumed to be in the same units as the coordinates, and
            // therefore they should not be rescaled.
            prj[i]->setFalseOffsets(offsetE, offsetN);

        } else if (units[i]) {
            // if non-default units are used, the default false offset needs to be rescaled
            // from meters to the chosen units.  e.g. in MTM8Q, if the units are feet, the
            // default false easting of 304800m becomes 1000000ft, just like it used to be
            // before Canada converted to metric.

            offsetE = prj[i]->getFalseEasting() / unitFact;
            offsetN = prj[i]->getFalseNorthing() / unitFact;
            prj[i]->setFalseOffsets(offsetE, offsetN);
        }

        if (scale[i]) {
            if (prj[i] == &nullProj) {
                print_error("Error: Scale factor cannot be specified for lat/long coordinates.");
                return err_params;
            }

            scaleFact = atof(scale[i]);
            if (scaleFact <= 0) {
                print_error("Error: Invalid scale factor.");
                return err_params;
            }

            printf("  Overriding '%s' scale factor with %.5f\n", from_to[i], scaleFact);
            prj[i]->setScaleFactor(scaleFact);
        }

        if (units[i]) {
            // To avoid an extra calculation, the unit factor can be shoehorned into
            // the prj scaleFactor.  Note that, when projecting from lat/long, the
            // scale factor is applied to both x and y right before the false
            // offsets are added.  (When unprojecting, the scale factor is applied
            // after the false offsets are subtracted.)

            scaleFact = prj[i]->getScaleFactor();
            prj[i]->setScaleFactor(scaleFact / unitFact);
        }
    }

    if (gs[0] == gs[1]) {
        gs[0] = gs[1] = NULL;
    } else {
        shptrans_err errcode = err_none;

        if (gs[0] == &gs_ats77 || gs[1] == &gs_ats77) {
            errcode = open_gridshift_file(gs_ats77, "SHPTRANS_GRIDSHIFT_7783",
              "NB7783V2.GSB:NS7783V2.GSB:NS778301.GSB:PE7783V2.GSB:GS7783.GSB");
            if (errcode != err_none) return errcode;
        }

        if (gs[0] == &gs_nad27 || gs[1] == &gs_nad27) {
            errcode = open_gridshift_file(gs_nad27, "SHPTRANS_GRIDSHIFT_NTV2",
              "NTV2_0.GSB:MAY76V20.GSB");
            if (errcode != err_none) return errcode;
        }
    }

    return err_none;
}








class FileCopier {
  protected:
    char fromFile[MAX_PATH];
    char toFile[MAX_PATH];

  public:
    FileCopier() { fromFile[0] = toFile[0] = '\0'; }
    FileCopier(char const*from, char const*to, char const *ext = NULL) {
        setFilenames(from,to,ext);
    }
    void setFilenames(char const*from, char const*to, char const *ext = NULL) {
        strcpy(fromFile,from); strcpy(toFile,to);
        if (ext) {
            swapext(fromFile,ext);
            swapext(toFile,ext);
        }
    }

    int copy();

} copyDbf;


int FileCopier::copy() {
    shptrans_err errcode = err_none;

# ifdef _WIN32
    if (CopyFile(fromFile, toFile, TRUE)) {
        SetFileAttributes(toFile, FILE_ATTRIBUTE_NORMAL);
    } else {
        errcode = (GetLastError()==ERROR_FILE_EXISTS) ? err_exists : err_create;
    }

# else

    FILE*pIn, *pOut;
    char buffer[4096];
    int n_read=0, n_write=0;

    pIn = fopen(fromFile, "rb");
    pOut = fopen(toFile, "wb");

    while ((n_read = fread(buffer,1,4096,pIn)) > 0) {
        if (n_read != (n_write = fwrite(buffer, 1, n_read, pOut)) ) {
            errcode = err_create;
            break;
        }
    }

    fclose(pIn);
    fclose(pOut);

# endif

  return errcode;
}




DWORD WINAPI CopyFileThreadFunc(void*arg) {
    FileCopier *cf = (FileCopier*)arg;
    return (DWORD) cf->copy();
}












static int dotsWritten = -1;
static int nextDot=5;

static HANDLE avSem=0;
static int semReleases=0;

void FinishStatus(int err) {
    SetConsoleTitle("SHPTRANS");
    if (dotsWritten >= 0) { //i.e. if StartStatus was called
        if (!err) {
            for (;dotsWritten<20;++dotsWritten) {
                putchar('.');
            }

            if (avSem) {
                for (;semReleases<=100;++semReleases) {
                    ReleaseSemaphore(avSem,1,0);
                }
            }
        }

        dotsWritten = -1;
        putchar('\n');
        fflush(stdout);
    }
}



void StartStatus(char *message) {
    char *avHandleStr = getenv("SYNCEXEC_PROGRESS_HANDLE");
    if (avHandleStr) {
        if (1!=sscanf(avHandleStr, "%u", &avSem)) avSem=0;
        semReleases = 0;
        putenv("SYNCEXEC_PROGRESS_HANDLE=0"); //so it will only work once.
    }

    FinishStatus(0);
    fputs(message, stdout);
    fflush(stdout);
    dotsWritten = 0;
}



float UpdateStatus(float percentDone) {
    int pct = (int)percentDone;
    while (pct >= nextDot) {
        putchar('.'); fflush(stdout);
        ++dotsWritten;
        nextDot += 5;
    }

    if (avSem) {
        while (semReleases < pct) {
            ReleaseSemaphore(avSem,1,0);
            ++semReleases;
        }
    }
    char consoleTitle[30];
    sprintf(consoleTitle,"SHPTRANS (%d%%)",pct);
    SetConsoleTitle(consoleTitle);

    return pct+1;
}











inline void expand_box(double *box, double *xy, int count) {
    double *px, *py;

    while (count--) {
        px = xy++; py = xy++;

        if (*px < box[0]) box[0] = *px;
        if (*px > box[2]) box[2] = *px;

        if (*py < box[1]) box[1] = *py;
        if (*py > box[3]) box[3] = *py;
    }
}

inline void init_box(double *box, double *xy, int count) {
    box[2] = box[0] = xy[0];
    box[1] = box[3] = xy[1];
    if (count > 1) {
        expand_box(box, xy+2,count-1);
    }
}



inline void rescale_coordinates(double factor, double *xy, int count) {
    count *=2;
    while (count--) {
        *xy *= factor;
    }
}












shptrans_err apply_transform(char *fromShp, char *toShp) {
    if (userAbort) return err_abort;


    shptrans_err errcode;

    long shxLen = 0;
    long nrecs = 0;
    unsigned long shxData[2];

    long *pRec = 0;
    unsigned long recLen, recPos;
    double *pPts = 0, *pPtsOrig=0;
    double *pBox = 0;
    double recBox[4];
    double totalBox[4];

    float percentDone = 0;
    float percentNext = 0;

    long shpSize;

    pod_array<long,1> recBuf;

    long errCount = 0;

#if FORCE_ALIGN
    pod_array<double,2> coordBuf(1024);
#endif

    long recno = 0;
    long shpType = 0;
    long numPts = 0;
    int tran_err;

    long totalPts = 0;


    char shpHead[100];
    char shxHead[100];

#if INPLACE_MMAP
    HANDLE hShp = INVALID_HANDLE_VALUE;
    HANDLE hShpMap = NULL;
    char *shpData = NULL;
#endif

    swapext(fromShp,"shp"); swapext(toShp,"shp");



#if INPLACE_MMAP
    if (inPlace)  {
        hShp = CreateFile(toShp, GENERIC_READ|GENERIC_WRITE,
          0, 0 , OPEN_EXISTING,
          FILE_ATTRIBUTE_NORMAL, NULL);

        if (hShp == INVALID_HANDLE_VALUE) return err_create;

        DWORD loSize, hiSize;
        loSize = GetFileSize(hShp, &hiSize);
        if ((hiSize!=0) || (loSize<100)) return err_intern;

        hShpMap = CreateFileMapping(hShp,0,PAGE_READWRITE, 0, loSize, NULL);
        if (!hShpMap) return err_intern;

        shpData = (char*) MapViewOfFile(hShpMap, FILE_MAP_WRITE, 0,0,0);
        if (!shpData) return err_intern;

    } else {
#endif
        recBuf.resize(2048);

        shp = fopen(fromShp, inPlace ? "rb+" : "rb");
        if (!shp) return err_create;
        if (100!=fread(shpHead,1,100,shp)) return err_magic;

        if (inPlace) {
            shpOut = shp;
        } else {
            shpOut = fopen(toShp,"wb+");
            if (!shpOut || (100!=fwrite(shpHead, 1, 100, shpOut))) {
                return err_create;
            }
        }
#if INPLACE_MMAP
    }
#endif

    swapext(toShp,"shx"); swapext(fromShp,"shx");
    shx = fopen(fromShp, inPlace ? "rb+" : "rb");

    if (!shx) return err_create;
    if ((100!=fread(shxHead,1,100,shx))) return err_magic;

    if (inPlace) {
        shxOut = shx;
    } else {
        shxOut = fopen(toShp,"wb+");
        if (!shxOut || (100!=fwrite(shxHead, 1, 100, shxOut))) {
            return err_create;
        }
    }

    if (userAbort) return err_abort;

    if (!inPlace) {
        // Now start the DBF copying, in a second thread.
        // This is done asynchronously so that we can begin
        // doing CPU-intensive stuff while the hard drive is
        // thrashing on the DBF.  It's a small win, but worth
        // it for a big DBF.

        copyDbf.setFilenames(fromShp, toShp, "dbf");

        DWORD copyThreadId = 0;
        dbf_thread = CreateThread(NULL,256,CopyFileThreadFunc,
                                 &copyDbf, 0, &copyThreadId);
        if (!dbf_thread) return err_create;
    }

    shxLen = BIG_END(*(long*)(shxHead + 24)) * 2;
    nrecs = (shxLen - 100) / 8;

    StartStatus("Transforming coordinates");

    // Loop through the records.

    for (int i = 0; i < nrecs; ++i) {
        if (userAbort) return err_abort;

        percentDone = 100.0 * i / nrecs;
        if (percentDone >= percentNext) {
            percentNext = UpdateStatus(percentDone);
        }

        // lookup the record via the SHX
        if (2 != fread(shxData,4,2,shx)) return err_io;
        recLen = (BIG_END(shxData[1]) >> 1) + 2; //+2 for rec-header
        recPos = BIG_END(shxData[0]) << 1;

#if INPLACE_MMAP
        if (inPlace) {
            pRec = (long*)(shpData+recPos);
        } else {
#endif
            if (0 != fseek(shp, recPos, SEEK_SET)) return err_io;
            pRec = recBuf.reserve(recLen); //extras to fix alignment
            if (!pRec) return err_mem;
            if (recLen != fread(pRec, 4, recLen, shp)) return err_io;
#if INPLACE_MMAP
        }
#endif

        shpType = pRec[2]; // SHP type and data are in Intel order

        pBox = pPts = pPtsOrig = NULL;

        if (shpType < 30) {
           switch (shpType % 10) {
             case 1:
               numPts = 1;
               pPts = (double*) (pRec + 3);
               pBox = 0;
               break;
             case 3: case 5:
               pBox = (double*) (pRec + 3);
               numPts = pRec[12];
               pPts = (double*) (pRec + 13 + pRec[11]);
               break;
             case 8:
               pBox = (double*) (pRec + 3);
               numPts = pRec[11];
               pPts = (double*) (pRec + 12);
               break;
           }
        } else if (shpType == 31) {
            pBox = (double*) (pRec + 3);
            numPts = pRec[12];
            pPts = (double*) (pRec + 13 + pRec[11] * 2);
        }

        tran_err = 0;
        if (pPts) {

#     if FORCE_ALIGN
           if ( ((unsigned long)pPts) & 7) {
               pPtsOrig = pPts;
               pPts = coordBuf.reserve(numPts);
               memcpy(pPts, pPtsOrig, numPts*16);
           }
           if ( ((unsigned long)pBox) & 7) {
               memcpy(recBox, pBox, 32); //0 is aligned so no null test
               pBox = recBox;
           }
#     endif

           //NEED TO BYTESWAP COORDS HERE TO COMPLETE THE
           //BIG-ENDIAN PORT.

           // apply transformations (fn namesake)

           tran_err = prj[0]->toLatLong(pPts, numPts);

           if (!tran_err) {
               tran_err = (gs[0] && gs[0]->forward(pPts, numPts)) ||
                          (gs[1] && gs[1]->reverse(pPts, numPts));

                         //fromLatLong first to avoid short-circuit
               tran_err = prj[1]->fromLatLong(pPts, numPts) || tran_err;
           }

           if (tran_err && verbose) {
               print_error("\nSHPTRANS: Error in record %d.",i+1);
           }

           if (pBox) {
               //update this record's bounding box
               //as well as the shapefile's bbox
               init_box(pBox, pPts, numPts);
               if (totalPts) {
                   expand_box(totalBox, pBox, 2);
               } else {
                   init_box(totalBox, pBox, 2);
               }
#if FORCE_ALIGN
               if (pBox==recBox) {
                   memcpy(pRec + 3, pBox,32);
               }
#endif

           } else {
               // e.g. a simple point record.
               //just update the shp header
               if (totalPts) {
                   expand_box(totalBox, pPts, numPts);
               } else {
                   init_box(totalBox, pPts, numPts);
               }
           }

#if FORCE_ALIGN
           if (pPtsOrig) {
               memcpy(pPts, pPtsOrig, numPts*16);
           }
#endif

           totalPts += numPts;
           changed=1;
        }

        if (!inPlace) {
            shxData[0] = BIG_END( ftell(shpOut)>>1 );
            if (2 != fwrite(shxData, 4, 2, shxOut)) return err_io;
            if (recLen != fwrite(pRec, 4, recLen, shpOut)) return err_io;
        } else {
#if !INPLACE_MMAP
            fseek(shpOut,recPos, SEEK_SET);
            if (recLen != fwrite(pRec, 4, recLen, shpOut)) return err_io;
#endif
        }
    } //for each rec

    if (!inPlace) {
        shpSize = BIG_END(ftell(shpOut) >> 1);
        fseek(shpOut, 24, SEEK_SET);
        if (1 != fwrite(&shpSize, 4, 1, shpOut)) return err_io;

        fclose(shx);
        fclose(shp);
    }
    shx = shp = NULL;

#if INPLACE_MMAP
    if (inPlace) {
        memcpy(shpData + 36, &totalBox, 32);

        // Dispose of filemapping
        UnmapViewOfFile(shpData);
        CloseHandle(hShpMap);
        hShpMap = shpData = NULL;

        // Update the SHP's date.  The SHX gets updated
        // automatically since ANSI file I/O was used.
        SYSTEMTIME stm;
        FILETIME ftm;
        GetSystemTime(&stm);
        SystemTimeToFileTime(&stm,&ftm);
        SetFileTime(hShp, NULL, &ftm, &ftm);

        CloseHandle(hShp); hShp = INVALID_HANDLE_VALUE;

    } else {
#endif
        // write the bounding box to the SHP and SHX headers
        fseek(shpOut, 36, SEEK_SET);
        if (4 != fwrite(&totalBox, 8, 4, shpOut)) return err_io;
        fclose(shpOut); shpOut = NULL;
#if INPLACE_MMAP
    }
#endif

    fseek(shxOut, 36, SEEK_SET);
    if (4 != fwrite(&totalBox, 8, 4, shxOut)) return err_io;
    fclose(shxOut); shxOut = NULL;

    FinishStatus(errcode);

    if (!inPlace) {
        DWORD waitResult = WAIT_TIMEOUT;
        while  (waitResult==WAIT_TIMEOUT && !userAbort) {
            waitResult = WaitForSingleObject(dbf_thread,1000);
        }

        if (waitResult == WAIT_OBJECT_0) {
            GetExitCodeThread(dbf_thread,(DWORD*)&errcode);
            CloseHandle(dbf_thread);
            dbf_thread = NULL;
        } else if (userAbort) {
            errcode = err_abort;
        } else {
            errcode = err_intern;
        }

        if (errcode) return errcode;
    }
    return err_none;
}






class ConsoleHelper {
  public:
    static BOOL WINAPI CtrlHandler(DWORD dwCtrlType);
    ConsoleHelper() {
        GetConsoleTitle(origTitle, sizeof(origTitle));
        origTitle[255] = '\0';
        SetConsoleCtrlHandler(CtrlHandler,TRUE);
    }
    ~ConsoleHelper() {
        SetConsoleTitle(origTitle);
        SetConsoleCtrlHandler(CtrlHandler, FALSE);
    }

   private:
     char origTitle[256];

     ConsoleHelper(ConsoleHelper&);
     void operator=(ConsoleHelper&);
};

BOOL WINAPI ConsoleHelper::CtrlHandler(DWORD dwCtrlType) {
    if (userAbort) {
        if (dwCtrlType == CTRL_BREAK_EVENT) {
            return FALSE;
        }
    }
    userAbort = true; //it should break soon
    return TRUE;
}

static ConsoleHelper consoleHelper;


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
