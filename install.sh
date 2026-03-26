#! /bin/sh

echo "INSTALLING NECTAR..."

git clone -j8 https://github.com/ne-foss-org/nectar
cd nectar
cd src/CompilerKit
sudo nebuild ck-posix.json
cd ..
cd DebuggerKit
sudo nebuild dk-nk-posix.json
cd ..
cd CommandLine
nebuild posix/cppdrv.json posix/ld64-posix.json posix/pef-amd64-asm.json posix/pef-amd64-drv.json posix/ptx-drv.json
sudo cp cppdrv /usr/local/bin/cppdrv-nectar
sudo cp ld64 /usr/local/bin/ld64-nectar
sudo cp pef-amd64-asm /usr/local/bin/
sudo cp pef-amd64-necdrv /usr/local/bin/
sudo cp ptx-necdrv /usr/local/bin/

echo "== WELCOME TO NECTAR =="
echo "== HELP =="
echo "pef-amd64-necdrv <FLAGS> <FILES>"
