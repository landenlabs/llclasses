#!/bin/csh -f

set app=llclasses
xcodebuild -list -project $app.xcodeproj

# rm -rf DerivedData/
xcodebuild -scheme $app -configuration Release clean build
# xcodebuild -configuration Release -alltargets clean


find ./DerivedData -type f -name $app -perm +111 -ls
set src=./DerivedData/Build/Products/Release/$app

echo "File ß$src"
ls -al $src
cp $src ~/opt/bin/