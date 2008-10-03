
package Regresstor;

use DBI;
use Compress::Zlib;

use lib '.';
use RegresstorConfig;

sub submitBuild {
	shift;			# Remove class name

	unless ( @_ == 1 ) { 
		return { errorMsg => "Invalid number of arguments" };
	}

	my $arguments = shift;	# A Hash (dictionary) of values

	my $project;
	my $buildOptions;
	my $CC;
	my $revision;
	my $machineName;
	my $hardware;
	my $OS;
	my $kernel;
	my $who;
	my $localModification;
	my $date;
	my $time;
	my $compiled;
	my $makefile;
	my $makefileCompressed;
	my $log;
	my $logCompressed;

	# Gather all the required information as variables

	unless ( exists $arguments->{Project} ) {
		return { errorMsg => "Argument Project not given" };
	}
	$project = $arguments->{Project};
	unless ( exists $arguments->{BuildOptions} ) {
		return { errorMsg => "Argument BuildOptions not given" };
	}
	$buildOptions= $arguments->{BuildOptions};
	unless ( exists $arguments->{CC} ) {
		return { errorMsg => "Argument CC not given" };
	}
	$CC = $arguments->{CC};
	unless ( exists $arguments->{Revision} ) {
		return { errorMsg => "Argument Revision not given" };
	}
	$revision = $arguments->{Revision};
	unless ( exists $arguments->{MachineName} ) {
		return { errorMsg => "Argument Machine_Name not given" };
	}
	$machineName = $arguments->{MachineName};
	unless ( exists $arguments->{Hardware} ) {
		return { errorMsg => "Argument Hardware not given" };
	}
	$hardware = $arguments->{Hardware};
	unless ( exists $arguments->{OS} ) {
		return { errorMsg => "Argument OS not given" };
	}
	$OS = $arguments->{OS};
	unless ( exists $arguments->{Kernel} ) {
		return { errorMsg => "Argument Kernel not given" };
	}
	$kernel= $arguments->{Kernel};
	unless ( exists $arguments->{Who} ) {
		return { errorMsg => "Argument Who not given" };
	}
	$who = $arguments->{Who};
	unless ( exists $arguments->{LocalModification} ) {
		return { errorMsg => "Argument LocalModification not given" };
	}
	$localModification = $arguments->{LocalModification};		
	unless ( exists $arguments->{Compiled} ) {
		return { errorMsg => "Argument Compiled not given" };
	}
	$compiled = $arguments->{Compiled};
	unless ( exists $arguments->{Makefile} ) {
		return { errorMsg => "Argument Makefile not given" };
	}
	$makefile = $arguments->{Makefile};
	unless ( exists $arguments->{Log} ) {
		return { errorMsg => "Argument Log not given" };
	}
	$log = $arguments->{Log};

	# Record the server's date and time
	my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime time;
	$year = 1900 + $year;	# Adjust year
	$mon = 1 + $mon;	# Adjust month

	$date = "$year-$mon-$mday";
	$time = "$hour:$min:$sec";

	# Compress attachments
	$makefileCompressed = compress( $makefile, Z_BEST_COMPRESSION );
	$logCompressed = compress( $log, Z_BEST_COMPRESSION );

	##### Authentication? If so, put here #####
	


	my $config = RegresstorConfig->new();
	my $dbh = DBI->connect( $config->getConnectionString, $config->user, $config->password )
			or return { errorMsg => "Unable to estiblish connection with database server." };

	# Find the ProjectID value	
	my $projectID;
	
	my $projectIDQueryString = 
		qq{
			SELECT ID
			FROM Project
			WHERE Name = '$project'
		};
	my $projectIDQuery = $dbh->prepare( $projectIDQueryString );
	$projectIDQuery->execute();
	my @projectResult = $projectIDQuery->fetchrow_array();
	$projectIDQuery->finish();

	if ( @projectResult ) {
		# Project exists and grab the ID value
		$projectID = $projectResult[0];
	}
	else {
		# Insert Project tuple
		my $projectCommand = $dbh->prepare(
			qq{
				INSERT INTO Project( Name )
				VALUES ( '$project' )
			} );
		$projectCommand->execute();
		$projectCommand->finish();

		$projectQuery = $dbh->prepare( $projectIDQueryString );
		$projectQuery->execute();
		@projectResult = $projectQuery->fetchrow_array();
		$projectQuery->finish();
		if ( @projectResult ) {
			$projectID = $projectResult[0];
		}
		else {
			# Command did not succeed
			$dbh->disconnect();
			return { errorMsg => "Unable to create new entry for project." };
		}
	}

	# Find the MachineID value	
	my $machineID;
	
	my $machineIDQueryString = 
		qq{
			SELECT ID
			FROM Machine
			WHERE Name = '$machineName'
			AND   Hardware = '$hardware'
			AND   OS = '$OS'
			AND   Kernel = '$kernel'
		};

	my $machineIDQuery = $dbh->prepare( $machineIDQueryString );
	$machineIDQuery->execute();
	my @machineResult = $machineIDQuery->fetchrow_array();
	$machineIDQuery->finish();

	if ( @machineResult ) {
		# Machine exists grab the ID value
		$machineID = $machineResult[0];
	}
	else {
		# Machine does not exist. Add new tuple and grab the new ID again
		my $machineCommand= $dbh->prepare(
			qq{
				INSERT INTO Machine ( Name, Hardware, OS, Kernel )
				VALUES ( '$machineName', '$hardware', '$OS', '$kernel' )
			} );
	        $machineCommand->execute();
	        $machineCommand->finish();

		# Search for the new ID
		$machineIDQuery = $dbh->prepare( $machineIDQueryString );
		$machineIDQuery->execute();
	        @machineResult = $machineIDQuery->fetchrow_array();
	        $machineIDQuery->finish();
		if ( @machineResult ) {
			$machineID = $machineResult[0];
		}
		else {
			# Command did not succeed
			$dbh->disconnect();
			return { errorMsg => "Unable to create new entry for machine." };
		}
	}

	# Now that we have all the information, attempt to insert into database the Build entry
	my $buildCommand = $dbh->prepare(
		qq{
			INSERT INTO Build
				(
				ProjectID, 
				BuildOptions, 
				CC, 
				Revision, 
				MachineID, 
				Who, 
				LocalModification, 
				Date, 
				Time, 
				Compiled,
				Makefile,
				Log )
			VALUES	(
				'$projectID',
				'$buildOptions',
				'$CC',
				'$revision',
				'$machineID',
				'$who',
				'$localModification',
				'$date',
				'$time',
				'$compiled',
				?, ? )
		} );
	$buildCommand->execute( $makefileCompressed, $logCompressed );
	$buildCommand->finish();

	my $buildIDQuery = $dbh->prepare(
		qq{
			SELECT ID
			FROM Build
			WHERE ProjectID='$projectID'
			AND   BuildOptions='$buildOptions'
			AND   CC='$CC'
			AND   Revision='$revision'
			AND   MachineID='$machineID'
			AND   Who='$who'
			AND   LocalModification='$localModification'
			AND   Date='$date'
			AND   Time='$time'
		} );
	$buildIDQuery->execute();
	my @buildResult = $buildIDQuery->fetchrow_array();	
	$buildIDQuery->finish();

	$dbh->disconnect();
	
	if ( @buildResult ) {
		my $buildID = $buildResult[0];
		
		# Submit is successful and database record inserted.
		# Return the build ID
		return { buildID => $buildID };		
	}
	else {
		return { errorMsg => "Unable to insert Build tuple into database." };
	}
}

sub submitCheck {
	shift;			# Remove class name

	unless ( @_ == 1 ) { 
		return { errorMsg => "Invalid number of arguments" };
	}

	my $arguments = shift;	# A Hash (dictionary) of values

	my $runID;
	my $unitName;
	my $testName;
	my $passed;
	my $output;
	my @failedSubTests;

	# Gather all the required information as variables

	unless ( exists $arguments->{RunID} ) {
		return { errorMsg => "Argument RunID not given" };
	}
	$runID = $arguments->{RunID};
	unless ( exists $arguments->{UnitName} ) {
		return { errorMsg => "Argument UnitName not given" };
	}
	$unitName = $arguments->{UnitName};
	unless ( exists $arguments->{TestName} ) {
		return { errorMsg => "Argument TestName not given" };
	}
	$testName = $arguments->{TestName};
	$testName = substr( $testName, 0, 50 ); # Truncate string to TestName field width

	if ( exists $arguments->{Output} ) {
		$output = $arguments->{Output};
	}
	else {
		$output = undef;
	}
	

	if ( exists $arguments->{FailedSubTests} ) {
		$passed = 0;
# Comment out until I work out how to get array passed from csoap to perl soap
#		@failedSubTests = @{ $arguments->{FailedSubTests} };
		@failedSubTests = $arguments->{FailedSubTests};
	}
	else {
		$passed = 1;
		$failedSubTests = undef;
	}

	##### Authentication? If so, put here #####

	my $outputCompressed = '';

	if ( $output ) {
		$outputCompressed = compress( $output, Z_BEST_COMPRESSION );
	}

	# Check to see if entry exists!
	my $unitTestCheckID;

	my $config = RegresstorConfig->new();
	my $dbh = DBI->connect( $config->getConnectionString, $config->user, $config->password )
			or return { errorMsg => "Unable to estiblish connection with database server." };

	my $unitTestCheckIDQueryString = 
		qq{
			SELECT ID, Passed
			FROM UnitTestCheck
			WHERE RunID='$runID'
			AND   testName='$testName'
		};

	my $unitTestCheckIDQuery= $dbh->prepare( $unitTestCheckIDQueryString );
	$unitTestCheckIDQuery->execute();
	my @unitTestCheckResult = $unitTestCheckIDQuery->fetchrow_array();
	$unitTestCheckIDQuery->finish();

	if ( @unitTestCheckResult ) {
		# Entry exists, this is probably a checkDiff submit
		$unitTestCheckID = $unitTestCheckResult[0];

		unless( $passed ) {
			if ( $unitTestCheckResult[1] == 1 ) {
				# If this second submit is a fail, change the "Passed" status of this 
				# test to false if it was originally true.
				my $updateUnitCheckCommand = $dbh->prepare(
					qq{
						UPDATE UnitTestCheck
						SET Passed='0'
						WHERE ID='$unitTestCheckID'
					} );
				$updateUnitCheckCommand->execute();
				$updateUnitCheckCommand->finish();
			}
		}
	}
	else {
		my $unitTestCheckCommand= $dbh->prepare(
			qq{
				INSERT INTO UnitTestCheck ( RunID, UnitName, TestName, Passed )
				VALUES ( '$runID', '$unitName', '$testName', '$passed' )
			});
		$unitTestCheckCommand->execute();
		$unitTestCheckCommand->finish();
		
		# Look for the newly added tuple
		$unitTestCheckIDQuery= $dbh->prepare( $unitTestCheckIDQueryString );
		$unitTestCheckIDQuery->execute();
		@unitTestCheckResult = $unitTestCheckIDQuery->fetchrow_array();
		$unitTestCheckIDQuery->finish();

		if ( @unitTestCheckResult ) {
			$unitTestCheckID = $unitTestCheckResult[0];
		}	
		else {
			return { errorMsg => "Unable to add tuple to UnitTestCheck table" };
		}
	}
	
	unless ( $passed ) {
		# Add the failed sub test entries too
		my $failCount = @failedSubTests;
		for ( my $counter = 0; $counter < $failCount; $counter++ ) {
			my $failedUnitSubTestCommand = $dbh->prepare(
				qq{
					INSERT INTO FailedUnitSubTest ( UnitTestCheckID, SubTest, Output )
					VALUES ( '$unitTestCheckID', '$failedSubTests[$counter]', ? )
				} );

	                $failedUnitSubTestCommand->execute( $outputCompressed );
        	        $failedUnitSubTestCommand->finish();
		}
	}	

	$dbh->disconnect();		

	return { unitTestCheckID => $unitTestCheckID };

}


sub beginRun {
	shift;			# Remove class name

	unless ( @_ == 1 ) { 
		return { errorMsg => "Invalid number of arguments" };
	}

	my $arguments = shift;	# A Hash (dictionary) of values

	my $buildID;

	# Gather all the required information as variables

	unless ( exists $arguments->{BuildID} ) {
		return { errorMsg => "Argument BuildID not given" };
	}
	$buildID = $arguments->{BuildID};

	##### Authentication? If so, put here #####
	
	my $config = RegresstorConfig->new();
	my $dbh = DBI->connect( $config->getConnectionString, $config->user, $config->password )
			or return { errorMsg => "Unable to estiblish connection with database server." };

	my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime time;
	$year = 1900 + $year;	# Adjust year
	$mon = 1 + $mon;	# Adjust month

	$date = "$year-$mon-$mday";
	$time = "$hour:$min:$sec";

	my $insertTupleCommand = $dbh->prepare(
		qq{
			INSERT INTO Run ( BuildID, StartDate, StartTime, EndDate, EndTime )
			VALUES ( '$buildID', '$date', '$time', '0000-00-00', '00:00:00' )
		} );
	$insertTupleCommand->execute();
	$insertTupleCommand->finish();
	
	# Read back
	my $runID;
	my $runIDQuery = $dbh->prepare(
		qq{
			SELECT ID
			FROM Run
			WHERE BuildID='$buildID'
			AND   StartDate='$date'
			AND   StartTime='$time'
		} );
	$runIDQuery->execute();
	my @runIDQueryResult = $runIDQuery->fetchrow_array();
	$runIDQuery->finish();

	if ( @runIDQueryResult ) {
		$runID = $runIDQueryResult[0];
	}
	else {
		$dbh->disconnect();
		return { errorMsg => "Unable to add Run tuple" };
	}


	$dbh->disconnect();		

	return { runID => $runID };

}


sub endRun {
	shift;			# Remove class name

	unless ( @_ == 1 ) { 
		return { errorMsg => "Invalid number of arguments" };
	}

	my $arguments = shift;	# A Hash (dictionary) of values

	my $runID;

	# Gather all the required information as variables

	unless ( exists $arguments->{RunID} ) {
		return { errorMsg => "Argument RunID not given" };
	}
	$runID = $arguments->{RunID};
	
	##### Authentication? If so, put here #####

	my $config = RegresstorConfig->new();
	my $dbh = DBI->connect( $config->getConnectionString, $config->user, $config->password )
			or return { errorMsg => "Unable to estiblish connection with database server." };

	
	my $checkRunExistQuery = $dbh->prepare(
		qq{
			SELECT ID
			FROM Run
			WHERE ID='$runID'
		} );
	$checkRunExistQuery->execute();
	my @checkRunExistQueryResult = $checkRunExistQuery->fetchrow_array();
	unless ( @checkRunExistQueryResult ) {
		$dbh->disconnect();
		return { errorMsg => "Run does not exist" };
	}

	my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime time;
	$year = 1900 + $year;	# Adjest year
	$mon = 1 + $mon;	# Adjust month

	$date = "$year-$mon-$mday";
	$time = "$hour:$min:$sec";

	my $updateRunCommand = $dbh->prepare(
		qq{
			UPDATE Run
			SET EndDate='$date', EndTime='$time'
			WHERE ID='$runID'
		} );
	$updateRunCommand->execute();
	$updateRunCommand->finish();
	
	return { runID => runID }; 

}

1;
__END__



sub die_with_fault {
    die SOAP::Fault->faultcode('Server.Custom') # will be qualified
                   ->faultstring('Died in server method')
                   ->faultdetail(bless {code => 1} => 'BadError')
                   ->faultactor('http://www.soaplite.com/custom');
  }
