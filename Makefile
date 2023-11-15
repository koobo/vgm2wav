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

