# shell script function assumes input of:
# ${1} - executable 
# ${2} - action
#
# And has optional values:
# ${3} - ClubStGermainDir [default current dir]
# 		 NB. Does not have to be base directory, as long as directory
#    	 is inside a StGermain based project.
# ${4} - softlink location [default current dir]


# record current directory
lngermain () {
	CURR_DIR=`pwd`

	# record input
	executable=${1}
	action=${2}
	clubStGermainDir=${3}
	location=${4}
		
	# Check for any inputs.
	# Function needs at least an executable and an action 
	
	if test "${executable}x" != "x"; then
		if test "${action}x" != "x"; then
			# check for dir inputs
			# if none, assume current directory
			if test "${clubStGermainDir}x" = "x"; then
				 echo "Assuming script run from within StGermain based project."
				 clubStGermainDir=$CURR_DIR
				 location=$CURR_DIR
			fi 
			
			if test "${clubStGermainDir}x" = ".x"; then
				 clubStGermainDir=$CURR_DIR
			fi	 
			
			if test "${location}x" = "x"; then
				 echo "Assuming softlink's directory"
				 location=$CURR_DIR
			fi
			
			if test "${location}x" = ".x"; then
				 location=$CURR_DIR
			fi	 
			#Run programs if $3 and $4 are directories
			if test -d ${clubStGermainDir}; then
				if test -d ${location}; then
					cd ${clubStGermainDir}
					
					# find executableTester in the local VMAKE dir
					TEST_SCRIPT=./VMake/executableTester.sh
					DOWN_LVL=.
					until test -r ${TEST_SCRIPT} ; do
							TEST_SCRIPT=../${TEST_SCRIPT}
							DOWN_LVL=../$DOWN_LVL
							# If there is no VMake directory detected by root dir,
							# stop and exit.						
							if test -d ${DOWN_LVL}/${CURR_DIR}; then
								echo "No VMake directory detected. Root directory reached. Stopping script."
								exit
							fi	
					done
					# Change to softlink location 
					cd ${location}
					# Source shell-script in directory VMAKE 
					source ${clubStGermainDir}/${TEST_SCRIPT}
					
					# Create or remove softlink
					softlinkCreateOrRemove ${executable}  ${action} `dirname ${clubStGermainDir}/${TEST_SCRIPT}`
					
					# Return to original directory and finish
					cd $CURR_DIR
				else
					# Error for bad softink location
					echo "Error in softlinker.sh: ${location} is not a directory"
					echo "Input format should be:"
					echo 'lngermain ${executable} ${action=create/remove} [${ClubStGermain dir} ${softlink location dir} ]'				
				fi 
			else		 
				# error for bad StGermain based code location
				echo "Error in softlinker.sh: ${clubStGermainDir} is not a directory"
				echo "Input format should be:"
				echo 'lngermain ${executable} ${action=create/remove} [${ClubStGermain dir} ${softlink location dir} ]'
			fi			
		else 
			# Error for incorrect/missing action to perform
			echo "Error: missing action input. Input format should be:"
			echo 'lngermain ${executable} ${action=[create/remove]} [${ClubStGermain dir} ${softlink location dir}]'
	
		fi
	else 
		# Error for bad/missing executable name
		echo "Error: missing executable input. Input format should be:"
		echo 'lngermain ${executable} ${action=create/remove} [${ClubStGermain dir} ${softlink location dir} ]'
	fi
}
