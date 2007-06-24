#!/bin/bash

# set the path appropriate
PATH=${PATH}:`pwd`/../../bin

# command to compile the stuff
CC="q3lcc -DQ3_VM -DMISSIONPACK -S -Wf-target=bytecode -Wf-g -I../../cgame -I../../game -I../../ui"
ASM="q3asm -f"

# ordered list of files to compile
FILES="../ui_main.c ../../game/bg_misc.c ../../game/bg_lib.c ../../game/q_math.c ../../game/q_shared.c ../ui_atoms.c ../ui_players.c ../ui_util.c ../ui_shared.c ../ui_gameinfo.c"

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
echo "assembling ../ui"
if ${ASM} ../ui ; then
	
	# report all worked fine
	echo "done"

else
	
	# catch an error
	echo "ERROR"
	exit 1

fi # if ${ASM} ../ui ; then

# we're finished now
cd ..
