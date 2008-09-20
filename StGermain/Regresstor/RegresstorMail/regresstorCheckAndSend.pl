#!/usr/bin/perl

use DBI;
                                                                                                                                    
use lib '.';
use RegresstorMailConfig;
use RegresstorEmail;

my $error;

my $config = RegresstorMailConfig->new();
                                                                                                                                    
my $dbh = DBI->connect( $config->getConnectionString, $config->user, $config->password )
                or $error = "Unable to connect with database\n";

if ( $error ) {
	print $error;
	die;
}

my @emails = ();

my $emailQuery = $dbh->prepare(
	qq{
		SELECT ProjectID, EndDate
		FROM Build, Run
		WHERE Build.ID = Run.BuildID
		AND   NotificationSent = 0
		AND   EndDate <> '0000-00-00'
		AND   EndTime <> '00:00:00'
		GROUP BY ProjectID, EndDate 
	});
$emailQuery ->execute();

my @emailQuery = ();
while ( my @row = $emailQuery->fetchrow_array() ) {
	my $newEmail = RegresstorEmail::new();
	$newEmail->project( $row[0] );
	$newEmail->date( $row[1] );
	push( @emails, $newEmail );
}
$emailQuery->finish();

foreach my $counter ( 0 .. @emails - 1 ) {
	my $subjectCompilesQuery = $dbh->prepare(
		qq{
			SELECT Sum( Compiled ), Count( Compiled )
			FROM Build, Run
			WHERE Build.ID = Run.BuildID
			AND   NotificationSent = 0
			AND   EndDate <> '0000-00-00'
			AND   EndTime <> '00:00:00'
			AND   ProjectID = ?
			AND   EndDate = ?
		});
	$subjectCompilesQuery->execute( RegresstorEmail::project( $emails[$counter] ), RegresstorEmail::date( $emails[$counter] ) );
	my @compilesResult = $subjectCompilesQuery->fetchrow_array();
	$subjectCompilesQuery->finish();

	my $subjectTestsQuery = $dbh->prepare(
		qq{
			SELECT Sum( Passed ), Count( Passed )
			FROM Build, Run, UnitTestCheck
			WHERE Build.ID = Run.BuildID
			AND   Run.ID = UnitTestCheck.RunID
			AND   NotificationSent = 0
			AND   EndDate <> '0000-00-00'
			AND   EndTime <> '00:00:00'
			AND   ProjectID = ?
			AND   EndDate = ?
		});
	$subjectTestsQuery->execute( RegresstorEmail::project( $emails[$counter] ), RegresstorEmail::date( $emails[$counter] ) );
	my @testsResult= $subjectTestsQuery->fetchrow_array();
	$subjectTestsQuery->finish();

	my $projectNameQuery = $dbh->prepare(
		qq{
			Select Name
			From Project
			WHERE ID = ?
		} );
	$projectNameQuery->execute( RegresstorEmail::project( $emails[$counter] ) );
	my @projectNameResult = $projectNameQuery->fetchrow_array();
	$projectNameQuery->finish();
	my $projectName = $projectNameResult[0];

	my $subject = "Regression Test: " . $projectName . ". " .
					"$compilesResult[0] / $compilesResult[1] Compiles, " .
					"$testsResult[0] / $testsResult[1] Passes.";
	
	RegresstorEmail::subject( $emails[$counter], $subject );

	my $contentsQuery = $dbh->prepare(
		qq{
			SELECT EndDate, EndTime, Revision, Machine.Name, Hardware, OS, Compiled, 
				Sum( Passed ), Count( Passed ) - Sum( Passed )
			FROM Build, Machine, Run, UnitTestCheck
			WHERE Machine.ID = Build.MachineID
			AND   Build.ID = Run.BuildID
			AND   Run.ID = UnitTestCheck.RunID
			AND   NotificationSent = 0
			AND   EndDate <> '0000-00-00'
			AND   EndTime <> '00:00:00'
			AND   ProjectID = ?
			AND   EndDate = ?
			GROUP BY Run.ID
			UNION
			SELECT EndDate, EndTime, Revision, Machine.Name, Hardware, OS, Compiled, 0, 0
			FROM Build, Machine, Run
			LEFT JOIN UnitTestCheck ON Run.ID = UnitTestCheck.RunID
			WHERE Machine.ID = Build.MachineID
			AND   Build.ID = Run.BuildID
			AND   NotificationSent = 0
			AND   EndDate <> '0000-00-00'
			AND   EndTime <> '00:00:00'
			AND   ProjectID = ?
			AND   EndDate = ?
			AND   UnitTestCheck.RunID IS NULL       # LEFT JOIN and this is equiv of NOT IN
			GROUP BY Run.ID
		});
	$contentsQuery->execute( $emails[$counter]->{PROJECT}, $emails[$counter]->{DATE}, 
		$emails[$counter]->{PROJECT}, $emails[$counter]->{DATE} );

	my @contentsData = ();
	my @colWidths;

	while ( my @tuple = $contentsQuery->fetchrow_array() ) {
		foreach my $ii ( 0 .. @tuple - 1 ) {
			# Hardcode Compiled index
			if ( $ii == 6 ) {
				if ( $tuple[$ii] eq "1" ) {
					$tuple[$ii] = "yes";
				}
				else {
					$tuple[$ii] = "no";
				}
			}

			if ( $ii == 0 ) {
				@colWidths[$ii] = 0;
			}
			my $colLength = length $tuple[$ii];
			if ( $colLength > @colWidths[$ii] ) {
				@colWidths[$ii] = $colLength;
			}
		}
		push( @contentsData, \@tuple );
	}
	$contentsQuery->finish();

	my @headings = ( 
		"*Date*", 
		"*Time*", 
		"*Revision*", 
		"*Machine*", 
		"*Hardware*", 
		"*OS*", 
		"*Compiled*",
		"*Passes*",
		"*Fails" );
	for my $heading_I ( 0 .. @headings - 1 ) {
		my $headingLength = length $headings[$heading_I];
		if ( $headingLength > @colWidths[$heading_I] ) {
			@colWidths[$heading_I] = $headingLength;
		}
	}

	my $printString = "| ";

	foreach my $len_I ( 0 .. @colWidths - 1) {
		$printString = $printString . sprintf( "%%%ds | ", @colWidths[$len_I] );
	}
	$printString = $printString . "\n";

	my $contents = sprintf( 
		$printString, 
		$headings[0],
		$headings[1],
		$headings[2],
		$headings[3],
		$headings[4],
		$headings[5],
		$headings[6],
		$headings[7],
		$headings[8] );

	foreach my $tuple_I ( 0 .. @contentsData - 1 ) {
		my @tupleData = @{ @contentsData[$tuple_I] };
		$contents = $contents . sprintf(
			$printString,
			$tupleData[0],
			$tupleData[1],
			$tupleData[2],
			$tupleData[3],
			$tupleData[4],
			$tupleData[5],
			$tupleData[6],
			$tupleData[7],
			$tupleData[8] );
	}
	
	$contents = $contents . "\n";
	$contents = $contents . "Goto " . $config->regresstorserver . "/Summary.cgi for more information\n\n";

	RegresstorEmail::contents( $emails[$counter], $contents );

	my $mailQuery = $dbh->prepare(
		qq{
			SELECT MailTo, MailFrom
			FROM Project
			WHERE ID = ?
		});
	$mailQuery->execute( $emails[$counter]->{PROJECT} );
	my @mailResults = $mailQuery->fetchrow_array();
	$mailQuery->finish();

	if ( @mailResults ) {
		$emails[$counter]->{MAILTO} = $mailResults[0];
		$emails[$counter]->{MAILFROM} = $mailResults[1];

		my @mailtoarray = split( /@/, $mailResults[0] );
		$emails[$counter]->{SUBJECT} = "\[$mailtoarray[0]\] " .$emails[$counter]->{SUBJECT};
		
		
		RegresstorEmail::sendmail( $emails[$counter], $config->smtpserver );
		print "Email sent: $emails[$counter]->{SUBJECT}\n";

		my $updateCommand= $dbh->prepare(
			qq{
				UPDATE Run, Build
				SET NotificationSent='1'
				WHERE Run.BuildID = Build.ID
				AND   NotificationSent = 0
				AND   EndDate <> '0000-00-00'
				AND   EndTime <> '00:00:00'
				AND   ProjectID = ?
				AND   EndDate = ?
			});
		$updateCommand->execute( $emails[$counter]->{PROJECT}, $emails[$counter]->{DATE} );
		$updateCommand->finish();
	}
	else {
		print "Failed to send email: $emails[$counter]->{SUBJECT}\n";
	}

}

$dbh->disconnect();


