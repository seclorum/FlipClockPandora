
release:
	rm  -rf flipclock.pnd flipclock.iso &&\
	mkisofs -o flipclock.iso -R ./flipclock_pandora/ &&\
	cat flipclock_pandora/PXML.xml >> flipclock.iso &&\
	cat flipclock_pandora/flipclock.png >> flipclock.iso &&\
	mv flipclock.iso flipclock.pnd
	ls -alF *.pnd


clean:
	rm -rf *.pnd *.iso
