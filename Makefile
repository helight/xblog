all:
	git pull
	hugo
#	git submodule init
#	git submodule update
sub:
	cd themes/hugo-universal-theme
	git pull

