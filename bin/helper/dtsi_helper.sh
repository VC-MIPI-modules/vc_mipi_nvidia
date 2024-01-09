#!/bin/bash

. config/base.sh

DTSI_KEY=""

# This function should determine the DTSI_KEY, which is either a combination 
# of the carrier board and the som (Auvidea J...) or the carrier board itself (NV devkits).
function extract_and_set_key_from_config {
        if [[ -z $VC_MIPI_BOARD ]]
        then
                echo "VC_MIPI_BOARD is empty! Exiting."
                exit 1
        fi

        if [[ -z $VC_MIPI_SOM ]]
        then
                echo "VC_MIPI_SOM is empty! Exiting."
                exit 1
        fi

        if [[ "$VC_MIPI_BOARD" =~ ^"NV_DevKit" ]] 
        then
                DTSI_KEY=$VC_MIPI_BOARD
        else
                part_str_board=$VC_MIPI_BOARD
                part_str_som=""
                case "$VC_MIPI_SOM" in 
                Nano|NanoSD|Nano2GB)
                        part_str_som="Nano"
                        ;;
                XavierNX|XavierNXSD)
                        part_str_som="XavierNX"
                        ;;
                OrinNX)
                        part_str_som="OrinNX"
                        ;;
                AGXXavier|TX2)
                        part_str_som=$VC_MIPI_SOM
                        ;;
                TX2NX) 
                        part_str_som=$VC_MIPI_SOM
                        part_str_board="${part_str_board}D"
                        ;;
                *)
                        echo "Unknown som detected! Exiting."
                        exit 1
                        ;;
                esac

                DTSI_KEY="${part_str_board}_${part_str_som}"
        fi

        found=0
        # Since associate arrays don't allow duplicate keys, the first occurance of the key is the only one.
        for key in "${!DTSI_FILE_DICT[@]}" 
        do 
                if [[ $DTSI_KEY == $key ]]
                then
                        found=1
                        break
                fi
        done

        if [[ 0 == $found ]]
        then
                echo "Could not find DTSI KEY! Exiting."
                exit 1
        fi
}

# This function should copy a given device-tree file into an existing destination directory.
# First argument is the dtsi file path.
# Second argument is the destination directory.
function copy_dtsi_if_dest_exists {
        SOURCE_FILE="$1"
        DEST_DIR="$2"

        if [[ -z $SOURCE_FILE ]]
        then
                echo "No source file given! Exiting."
                exit 1
        fi 

        if [[ ! -f $SOURCE_FILE ]]
        then
                echo "Source file $SOURCE_FILE does not exist! Exiting."
                exit 1
        fi 

        if [[ -z $DEST_DIR ]]
        then
                echo "No dest dir given. Skipping file $SOURCE_FILE!"
                return 0
        fi 

        if [[ ! -e $DEST_DIR ]]
        then
                return 0
        fi

        echo "Copy $SOURCE_FILE to $DEST_DIR"
        cp -R $SOURCE_FILE $DEST_DIR
}

#This function tries to copy all possible device-tree files, which have an appropriate destination directory.
function copy_dtsi_files {
        if [[ -z $DTSI_KEY ]]
        then
                echo "Could not get DTSI_KEY! Exiting."
                exit 1
        fi

        for dtsi_key in "${!DTSI_FILE_DICT[@]}" 
        do 
                # Skip the actual dtsi file ...
                if [[ $dtsi_key == $DTSI_KEY ]]
                then
                        continue
                fi

                # ... try to copy all the other files ...
                src_file="$DT_CAM_DIR/$dtsi_key/${DTSI_FILE_DICT[$dtsi_key]}"
                dest_dir="${DTSI_DEST_DICT[$dtsi_key]}"
                
                copy_dtsi_if_dest_exists $src_file $dest_dir
        done

        # ... and copy the actual dtsi file as the last entry.
        # Otherwise it might be overridden by another dtsi file with the same name from another board.
        src_file="$DT_CAM_DIR/$DTSI_KEY/${DTSI_FILE_DICT[$DTSI_KEY]}"
        dest_dir="${DTSI_DEST_DICT[$DTSI_KEY]}"
        
        copy_dtsi_if_dest_exists $src_file $dest_dir
}
