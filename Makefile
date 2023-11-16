all:
	make -f Makefile.000
	make -f Makefile.020
	make -f Makefile.020fpu
	make -f Makefile.040
	make -f Makefile.060

clean:
	make -f Makefile.000 clean
	make -f Makefile.020 clean
	make -f Makefile.020fpu clean
	make -f Makefile.040 clean
	make -f Makefile.060 clean

dist:
	rm -f vgm2wav-gme.lha
	lha a vgm2wav-gme.lha vgm2wav.000
	lha a vgm2wav-gme.lha vgm2wav.020
	lha a vgm2wav-gme.lha vgm2wav.020fpu
	lha a vgm2wav-gme.lha vgm2wav.040
	lha a vgm2wav-gme.lha vgm2wav.060



