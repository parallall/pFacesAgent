
.PHONY: clean push pull


clean:
	rm -f *.o
	rm -f -r Debug
	rm -f -r Release
	rm -f -r x64	
	rm -f -r bin
	rm -f ./lib_x86/*.lib
	rm -f ./lib_x64/*.lib
	rm -f ./lib/*.a
	rm -f -r ./include/config4cpp
	rm -f -r ./include/symbolic

pull:
	git pull
	
push: pull clean
	git add -A .
	git commit -m"Automated commit from the Makeflle"
	git push
	