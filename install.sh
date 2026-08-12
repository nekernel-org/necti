#! /bin/sh

# Author: Makarov
# Imported-By: Amlal

echo "==> WELCOME TO NCC."
echo "==> INSTALLING NCC..."

if command -v dnf >/dev/null 2>&1; then
    sudo dnf groupinstall "Development Tools" -y
    sudo dnf install cmake nasm boost-devel -y

elif command -v yum >/dev/null 2>&1; then
    sudo yum groupinstall "Development Tools"
    sudo yum install nasm cmake boost-devel

elif command -v pacman >/dev/null 2>&1; then
    sudo pacman -Sy --noconfirm nasm base-devel cmake boost

elif command -v zypper >/dev/null 2>&1; then
    sudo zypper refresh
    sudo zypper install -t pattern devel_basis
    sudo zypper install nasm cmake boost-devel
    
else
    echo "Unsupported distribution. Please check the readme and install dependencies manually."
fi

if [ $? -ne 0 ]; then
    echo "Installation failed (sudo or package manager issue). Exiting."
    exit 1
fi

git clone -j8 https://github.com/ne-app-eu/ncc
cd ncc

sudo cp -r public/GenericsLibrary /usr/local/include/
chmod +x public/share/bin/nectar
sudo cp public/share/bin/nectar /usr/local/bin/

cd private/src/CompilerKit
sudo nebuild ck-posix.json
cd ..
cd DebuggerKit
sudo nebuild dk-nk-posix.json
cd ..
cd CommandLine
nebuild posix/ld64-posix.json posix/pef-amd64-asm.json posix/pef-amd64-drv.json posix/ptx-drv.json
#sudo cp cppdrv /usr/local/bin/cppdrv-nectar
sudo cp ld64 /usr/local/bin/ld64-nectar
sudo cp pef-amd64-asm /usr/local/bin/
sudo cp pef-amd64-necdrv /usr/local/bin/
sudo cp ptx-necdrv /usr/local/bin/

echo "==> WELCOME TO NCC"
echo "==> HELP"
echo "nectar <FILES>"