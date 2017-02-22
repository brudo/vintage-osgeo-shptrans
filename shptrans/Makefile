CC = g++
CFLAGS=-O4 -D_MT -mthreads -DINPLACE_MMAP -march=pentium -mcpu=pentiumpro -funroll-loops
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS = -Wl,--strip-all

.PHONY: all clean clean_objects clean_targets
.IGNORE: clean_objects clean_targets
.SILENT: clean_objects clean_targets

OBJS = shptrans.ro main.o gshift.o intgrid.o projbase.o tmerc.o dstereo.o

exe: shptrans.exe
zip: shptrans.zip

%.o : %.cpp
	$(CC) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

%.ro: %.rc
	rc /i "%MSSdk%\include" $<
	windres -O coff $*.res $@
	cmd /c del /f $*.res

shptrans.exe: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

readme.txt: shptrans.exe
	cmd /c shptrans -help > readme.txt

license.txt: shptrans.exe
	cmd /c shptrans -license > license.txt

shptrans.zip: shptrans.exe readme.txt license.txt
	cmd /c shptrans -version | zip -z $@ $^

clean: clean_objects clean_targets

clean_objects: 
	echo Removing intermediate files
	cmd /c del /f $(OBJS) 2> nul

clean_targets:
	echo Removing target files
	cmd /c del /f shptrans.exe shptrans.zip 2> nul

intgrid.o: intgrid.h
gshift.o: gshift.h intgrid.h
projbase.o: projbase.h
dstereo.o: dstereo.h projbase.h
tmerc.o: tmerc.h projbase.h
main.o: intgrid.h gshift.h tmerc.h projbase.h podarray.h
