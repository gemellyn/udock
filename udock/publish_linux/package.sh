#!/bin/bash
clear

version='1.9.0' 
name='udock'
directory=${name}-${version}
extension='orig.tar.xz'
archive=${name}_${version}.${extension}

echo "Realisation d'un .deb avec les sources... pas fini, voir le mode binaire only"

#clean
echo "Nettoyage..."
sudo rm -r packaging
mkdir packaging
cd ./packaging

#Creation du tar
echo "Copie des fichiers..."
mkdir ./$directory
cp -r ../../run ./$directory
cp -r ../../build_linux ./$directory
cp -r ../../src ./$directory	
echo "Compression du tar $archive..."
tar cfJ ./$archive ./$directory

#on rentre dans le rep
cd $directory

#on attaque
echo "Creation du package..."
dh_make -e guillaume.levieux@cnam.fr

#on edite
echo "Copie des fichiers de config..."
cd debian
rm -rf *ex *EX README*
cp ../../../base-a-copier/* ./

#on compile le binaire
echo "Compilation du binaire... (attention genere cle avec gpg si plus valide)"
cd ../build_linux
#recup la cle avec gpg --list-keys guillaume et mettre a la suite de 0x les 8 chiffres de la clé publique
debuild -k0x146D9B70 -sa --lintian-opts -i 



