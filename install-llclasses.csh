#!/bin/csh -f

set app=llclasses
set dstdir=~/opt/bin

# xcodebuild -list -project $app.xcodeproj
# rm -rf DerivedData/
# xcodebuild -configuration Release -alltargets clean
xcodebuild -scheme $app -configuration Release clean build
if ($status != 0) then
  say -v karen "Failed to build $app"
  exit -1
endif

# echo -------------------
# find ./DerivedData -type f -name $app -perm +111 -ls
set src=./DerivedData/Build/Products/Release/$app

echo
echo "---Install $src"
cp $src ${dstdir}/

echo
echo "---Files "
ls -al $src  ${dstdir}/$app

echo
echo "---Signed---"
codesign -dv  ${dstdir}/$app |& grep Sig
codesign -dv  ${dstdir}/$app |& grep adhoc > /dev/null
if ($status == 0) then
  say -v karen "Failed to sign $app"
endif

echo "[done]"