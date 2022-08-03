all:
	git pull
	git submodule sync
	hugo

commit:
	git commit -m "add new blog" -a

test:
	hugo server

# git clone https://gitee.com/helight/xblog
# git remote add github https://github.com/helight/xblog
# git remote rename origin gitee
github:
	git pull gitee master
	git push github master


