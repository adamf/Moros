all: FTOLED

clean:
	rm -rf FTOLED

FTOLED: FTOLED-master.zip
	rm -rf FTOLED-master FTOLED
	unzip $<
	mv FTOLED-master $@
	rm $<

FTOLED-master.zip:
	curl https://codeload.github.com/freetronics/FTOLED/zip/master > $@
