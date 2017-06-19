#!/bin/bash
clear

version='1.9.0' 
name='udock'
directory=${name}-${version}
extension='orig.tar.xz'
archive=${name}_${version}.${extension}
extensionPaquet='deb'
archi='i386'
paquet=${name}_${version}_${archi}.${extensionPaquet}

echo "Realisation d'un .deb en mode binaire only"

#clean
echo "Nettoyage..."
sudo rm -r packaging
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

#Copie des fichiers de config
echo "Copie de la config..."
echo "Package: udock
Version: $version
Depends: libsdl2-2.0-0, libglew1.10, libglewmx1.10, libfreetype6, libopenal1, libalut0
Architecture: i386
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
./engine" >> usr/bin/udock
chmod 755 usr/bin/udock

#copie du raccourcis
echo "#!/usr/bin/env xdg-open

[Desktop Entry]
Version=3.5.1
Encoding=UTF-8
Name=Udock
GenericName=Udock
Comment=Interactive Docking game
Exec=udock
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
rm -r $paquet
dpkg -b packaging $paquet




