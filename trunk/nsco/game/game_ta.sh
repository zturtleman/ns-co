#!/bin/bash

# set the path appropriate
PATH=${PATH}:../../bin

# command to compile the stuff
CC="q3lcc -DQ3_VM -DMISSIONPACK -DCGAME -S -Wf-target=bytecode -Wf-g -I../../cgame -I../../game -I../../ui"
ASM="q3asm -f"

# ordered list of files to compile
FILES="../g_main.c ../g_syscalls.c ../bg_misc.c ../bg_lib.c ../bg_pmove.c ../bg_slidemove.c ../q_math.c ../q_shared.c ../ai_dmnet.c ../ai_dmq3.c ../ai_main.c ../ai_chat.c ../ai_cmd.c ../ai_team.c ../g_active.c ../g_arenas.c ../g_bot.c ../g_client.c ../g_cmds.c ../g_combat.c ../g_items.c ../g_mem.c ../g_misc.c ../g_missile.c ../g_mover.c ../g_session.c ../g_spawn.c ../g_svcmds.c ../g_target.c ../g_team.c ../g_trigger.c ../g_utils.c ../g_weapon.c ../ai_vcmd.c"

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
echo "assembling ../game_ta"
if ${ASM} ../game_ta ; then
	
	# report all worked fine
	echo "done"

else
	
	# catch an error
	echo "ERROR"
	exit 1

fi # if ${ASM} ../game_ta ; then

# we're finished now
cd ..
