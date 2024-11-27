#
#
#
set dst=v4

mkdir -p ~/Dropbox/dlang-c++/llclasses/$dst/src 

rsync -av --exclude-from ~/rsync-exclude.txt llclasses/ ~/Dropbox/dlang-c++/llclasses/$dst/src/

# mkdir -p ~/Dropbox/dlang-c++/llclasses/$dst/aux
# rsync -av --exclude-from ~/rsync-exclude.txt dtree      ~/Dropbox/dlang-c++/llclasses/$dst/aux/
# rsync -av --exclude-from ~/rsync-exclude.txt img        ~/Dropbox/dlang-c++/llclasses/$dst/aux/
# rsync -av --exclude-from ~/rsync-exclude.txt images     ~/Dropbox/dlang-c++/llclasses/$dst/aux/

# cp  *.html     ~/Dropbox/dlang-c++/llclasses/$dst/aux/

