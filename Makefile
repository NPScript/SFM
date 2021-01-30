default_target: all

all:
	g++ sfm.cpp -lncurses -lboost_filesystem -o sfm

clean:
	rm sfm

install: all
	cp sfm /usr/local/bin/sfm

uninstall:
	rm /usr/local/bin/sfm

.PHONY: all
