#!/bin/bash
clear

version='1.9.0' 
name='udock'
directory=${name}-${version}
extension='orig.tar.xz'
archive=${name}_${version}.${extension}
extensionPaquet='deb'
archi='amd64'
paquet=${name}_${version}_${archi}.${extensionPaquet}

echo "Realisation d'un .deb en mode binaire only"

#clean
echo "Nettoyage..."
sudo rm -r packaging
sudo rm ./$paquet
mkdir packaging
cd ./packaging

#creation des reps
echo "Copie des reps..."
mkdir DEBIAN
mkdir opt
mkdir usr
mkdir usr/share
mkdir usr/share/applications
mkdir usr/bin

#Copie des fichiers binaires
echo "Copie des fichiers binaires..."
mkdir ./opt/$directory
cp -r ../../run ./opt/$directory
cp ../../build_linux/bin/Release/udock ./opt/$directory/run/udock

#copie des .so, on gere pas de dependances car on est tout pas gentil, on met les so a la main
cp ../binaires_libs/libalut.so.0.1.0 ./opt/$directory/run/libalut.so
cp ../binaires_libs/libboost_filesystem.so.1.58.0 ./opt/$directory/run/libboost_filesystem.so
cp ../binaires_libs/libboost_system.so.1.58.0 ./opt/$directory/run/libboost_system.so
cp ../binaires_libs/libboost_thread.so.1.58.0 ./opt/$directory/run/libboost_thread.so
cp ../binaires_libs/libcurl.so.4.3.0 ./opt/$directory/run/libcurl.so
cp ../binaires_libs/libfreetype.so.6.11.1 ./opt/$directory/run/libfreetype.so
cp ../binaires_libs/libGL.so.1.2.0 ./opt/$directory/run/libGL.so
cp ../binaires_libs/libGLEW.so.1.10.0 ./opt/$directory/run/libGLEW.so
cp ../binaires_libs/libGLU.so.1.3.1 ./opt/$directory/run/libGLU.so
cp ../binaires_libs/libopenal.so.1.14.0 ./opt/$directory/run/libopenal.so
cp ../binaires_libs/libpng16.so.16.17.0 ./opt/$directory/run/libpng16.so
cp ../binaires_libs/libpthread.so ./opt/$directory/run/libpthread.so
cp ../binaires_libs/librt-2.19.so ./opt/$directory/run/librt.so
cp ../binaires_libs/libSDL2-2.0.so.0.2.0 ./opt/$directory/run/libSDL2.so
cp ../binaires_libs/libz.so.1.2.8 ./opt/$directory/run/libz.so

#Copie des fichiers de config
echo "Copie de la config..."
echo "Package: udock
Version: $version
Architecture: $archi
Maintainer: guillaume <guillaume.levieux@cnam.fr>
Section: sciences
Priority: optional
Installed-Size: 20000
Description: Interactive Docking Entertainment System
  Udocks allows to perform molecular docking in and interactive, real time fashion.
" >> DEBIAN/control

#creation d'un lanceur
echo "#!/bin/bash
cd /opt/$directory/run
./$name" >> usr/bin/udock
chmod 755 usr/bin/udock

#copie du raccourcis
echo "#!/usr/bin/env xdg-open

[Desktop Entry]
Version=3.5.1
Encoding=UTF-8
Name=Udock
GenericName=Udock
Comment=Interactive Docking game
Exec=$name
Icon=/opt/$directory/icons/udock.png
Terminal=false
Type=Application
Categories=Science
Name[fr_FR]=Udock" >> usr/share/applications/udock.desktop


#copie icone
mkdir opt/$directory/icons
cp ../base-a-copier/binaire/udock.png opt/$directory/icons/udock.png

#creation du packet
echo "Creation du packet..."
cd ..
sudo chown -R root:root ./packaging #sinon erreur de mauvais id a l'install...
sudo dpkg -b packaging $paquet




