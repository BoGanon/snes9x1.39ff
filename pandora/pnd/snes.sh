#!/bin/bash


echo "ROM filename is $1"

PWD=`pwd`
echo "PWD pre-run $PWD"

# set HOME so SRAM saves go to the right place
HOME=.
export HOME

# enable higher quality audio
#0 - off, 1 - 8192, 2 - 11025, 3 - 16000, 4 - 22050, 5 - 32000 (default), 6 - 44100, 7 - 48000
#ARGS='-soundquality 7'
ARGS=''

PICKUPARGS=`cat args.txt`
if [ $PICKUPARGS ]
then
    # http://wiki.arcadecontrols.com/wiki/Snes9x#Command_Line_Parameters
    ARGS=$PICKUPARGS
fi

# run it!
if [ $(echo "$1" | cut -d'.' -f2) = "7z" ] 
then
	FILEITEM=$(eval zenity --width=800 --height=400 --list --title="Which\ ROM?" --column="Name" `./util/7za l -slt "$1" | grep ^Path | sed -e's/^Path = /"/g' -e's/$/"/' | sed '1d'`)
	if [ $? = 0 ]; then
 		zenity --info --title="$1" --text="Extracting ROM from 7z file, please wait..."
		./util/7za e -y -o"/tmp/" "$1" "$FILEITEM"
		FILENAME="/tmp/$FILEITEM"
		./snes9x $ARGS "$FILENAME"
		rm "$FILENAME"
	fi
else
	./snes9x $ARGS "$1"
fi
