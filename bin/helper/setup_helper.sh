#!/bin/bash

. config/base.sh

# This function is intended to download and check a given file.
# The argument should be one of these:
#    BSP..board support package
#    RFS..root file system
#    SRC..kernel source
#
# The hashsums can be found in the L4T/RXX.Y.Z.sh files
# If this function is called in the normal setup.sh context and one of the MD5 check fails,
# the user can try to download the file again by hitting 'y' key.
# If this function is called in the test.sh context, there will be an automatic retry upto three attempts.
download_and_check_file () {
        # First argument should be a string BSP|RFS|SRC .
        if [[ -z $1 ]]
        then 
                echo "Variable name as parameter missing ..."
                echo "Expected BSP, RFS or SRC"
                exit 1
        fi

        DL_STRIKE=0
        DL_RESULT=0
        DL_AUTO_RETRY=3

        # Creating variables (e.g. BSP_URL_UNRESOLVED, BSP_URL, BSP_FILE, BSP_FILE_CHECKSUM).
        URL_UNRESOLVED_VAR="$1_URL_UNRESOLVED"
        URL_UNRESOLVED_VAR=${!URL_UNRESOLVED_VAR}
        URL_VAR="$1_URL"
        URL_VAR=${!URL_VAR}
        FILE_VAR="$1_FILE"
        FILE_VAR=${!FILE_VAR}
        CHECKSUM_VAR="$1_FILE_CHECKSUM"
        CHECKSUM_VAR=${!CHECKSUM_VAR}
        user_retry_input=

        echo ""
        case $1 in
        BSP|RFS|SRC)
                echo "  Trying to download $1 file $FILE_VAR..."
                if [[ $TEST_COMMAND == $PARENT_COMMAND ]]
                then
                        echo "  ($DL_AUTO_RETRY attempts will be made)"
                else
                        user_retry_input="y"
                        DL_AUTO_RETRY=0
                fi
                echo ""
        ;;
        *)
                echo "Unknown option $1"
                exit 1
                ;;
        esac

        # The while is looping until either the retry count is reached (test mode) 
        # or the user did not confirm the retry.
        while [[ $DL_STRIKE -lt $DL_AUTO_RETRY || "y" == $user_retry_input ]] 
        do
                if [[ -e $FILE_VAR ]]; then 
                        rm -f $FILE_VAR
                fi

                if [[ -z $URL_UNRESOLVED_VAR ]]; then
                        wget $URL_VAR/$FILE_VAR
                else
                        wget -O $FILE_VAR $URL_UNRESOLVED_VAR
                fi

                DL_STRIKE=$((DL_STRIKE+1))
                
                echo "$CHECKSUM_VAR $FILE_VAR" | md5sum -c
                DL_RESULT=$?
                if [[ 0 != $DL_RESULT ]]
                then 
                        echo ""
                        echo "Something is wrong with downloaded file ${FILE_VAR}!"
                        CHECKSUM_TMP=`eval md5sum ${FILE_VAR} | cut -d " " -f 1`
                        echo "Checksum of downloaded file is:            ${CHECKSUM_TMP} ($DL_STRIKE. try)" 
                        echo "Checksum of downloaded file should be:     ${CHECKSUM_VAR}"
                        echo ""
                        if [[ $TEST_COMMAND != $PARENT_COMMAND ]]
                        then
                                echo "Retry download (y)?"
                                read user_retry_input
                        fi
                else
                        echo ""
                        echo "${CHECKSUM_VAR} seems to be ok."
                        echo ""
                        break
                fi
        done

        if [[ $DL_RESULT != 0 ]] 
        then
                echo "Could not download $1 file $FILE_VAR!"
                exit 1
        fi
}
