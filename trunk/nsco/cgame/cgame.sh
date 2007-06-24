#!/bin/bash

# set the path appropriate
PATH=${PATH}:`pwd`/../../bin

# command to compile the stuff
CC="q3lcc -DQ3_VM -DMISSIONPACK -DCGAME -S -Wf-target=bytecode -Wf-g -I../../cgame -I../../game -I../../ui"
ASM="q3asm -f"

# ordered list of files to compile
FILES="../../game/bg_misc.c ../../game/bg_pmove.c ../../game/bg_slidemove.c ../../game/bg_lib.c ../../game/q_math.c ../../game/q_shared.c ../cg_consolecmds.c ../cg_scripting.c ../cg_draw.c ../cg_drawtools.c ../cg_effects.c ../cg_ents.c ../cg_event.c ../cg_info.c ../cg_localents.c ../cg_main.c ../cg_marks.c ../cg_players.c ../cg_playerstate.c ../cg_predict.c ../cg_scoreboard.c ../cg_servercmds.c ../cg_snapshot.c ../cg_view.c ../cg_weapons.c ../cg_enviroment.c ../cg_seals.c ../cg_newDraw.c ../cg_mem.c ../bg_regexp.c ../../ui/ui_shared.c"

# create the directory containing the bytecode
# after the compile
if [ ! -d vm ] ; then
	# create vm directory if it doesn't exist
	mkdir vm
	cd vm
else
	# if vm directory exists erase it and generate a new one
	rm -rf vm
	mkdir vm
	cd vm
fi # if [ ! -d vm ] ; then

# compile the list FILES
for i in ${FILES} ; do
	
	# print the file we are compiling
	echo compiling $i
	
	# compile the file
	if ${CC} $i ; then
	
		# report all worked fine
		echo "done"

	else
		
		# catch an error
		echo "ERROR"
		exit 1
		
	fi # if ${CC} $i ; then

done # for i in ${FILES} ; do



# assemble it
echo "assembling ../cgame"
if ${ASM} ../cgame ; then
	
	# report all worked fine
	echo "done"

else
	
	# catch an error
	echo "ERROR"
	exit 1

fi # if ${ASM} ../cgame ; then

# we're finished now
cd ..
