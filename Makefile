all:
	git submodule init
	git submodule update
	git pull
	hugo