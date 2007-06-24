#!/bin/bash

game() {
	echo "Making everything in game"
	rm -f vm/game*
	cd game
	if ./game.sh > /dev/null ; then
		echo "done"
	else
		echo "ERROR"
		cd ..
		exit 1
	fi
	cd ..
}

cgame() {
	echo "Making everything in cgame"
	rm -f vm/cgame*
	cd cgame
	if ./cgame.sh > /dev/null ; then
		echo "done"
	else
		echo "ERROR"
		cd ..
		exit 1
	fi
	cd ..
}

ui() {
	echo "Making everything in ui"
	rm -f vm/ui*
	cd ui
	if ./ui.sh > /dev/null ; then
		echo "done"
	else
		echo "ERROR"
		cd ..
		exit 1
	fi
	cd ..
}

packcode() {
	echo "Packing to pak6.pk3"
	rm -f vm/*.map
	rm -f pak6.pk3
	zip -r -9 pak6.pk3 vm
	cd ..
	cd nsco
	unzip -l pak6.pk3
#	cp pak6.pk3 /cygdrive/c/Programme/Quake3/seals
	cp pak6.pk3 ~/.q3a/seals
}

packmedia() {
	echo "Packing media to pak7.pk3"
	rm -f pak7.pk3
	cd media
	zip -r -9 ../pak7.pk3 *
	cd ..
	unzip -l pak7.pk3
#	cp pak7.pk3 /cygdrive/c/Programme/Quake3/seals
	cp pak7.pk3 ~/.q3a/seals
}

# create the directory containing the bytecode
# after the compile
if [ ! -d vm ] ; then
  # create vm directory if it doesn't exist
  mkdir vm
fi # if [ ! -d vm ] ; then

case "$1" in
	game)
		game
		packcode
		;;
	cgame)
		cgame
		packcode
		;;
	ui)
		ui
		packcode
		;;
	all)
		game
		cgame
		ui
		packcode
		;;
	pack)
		packcode
		;;
	release)
		vim game/bg_public.h
		vim cgame/cg_info.c
		game
		cgame
		ui
		packcode
		;;
	media)
		packmedia
		;;
	help)
		echo "compile.sh [game|cgame|ui|pack|media|all]"
		echo "  all == game ; cgame ; ui ; pack"
		echo
		;;
	*)
		game
		cgame
		ui
		packcode
		;;
esac
