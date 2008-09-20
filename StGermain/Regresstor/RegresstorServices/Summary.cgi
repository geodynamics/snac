#!/usr/bin/perl -w

# Simple Page which shows ALL contents of the FailedUnitSubTest table

use CGI;
use DBI;
use HTML::Template;

use lib '.';
use RegresstorConfig;
use RegresstorCommon;
use RegresstorTable;

# An error message
my $error;

# open the html template
my $template = HTML::Template->new( filename => 'Summary.tmpl');

my $config = RegresstorConfig->new();

my $dbh = DBI->connect( $config->getConnectionString, $config->user, $config->password )
		or $error = "Unable to connect to database.";

if ( $error ) {
	&RegresstorCommon::displayError( 'Build', $error, $config->database, $config->host );
	exit;
}

my $queryString = "";

my $testsQuery =
	qq{
		SELECT Machine.Name AS Name, Project.Name AS Project, BuildOptions, CC, Revision, Who, LocalModification,
			Run.EndDate AS Date, Run.EndTime AS Time, Compiled, 
			SUM( Passed ) AS Passes, COUNT( Passed ) - SUM( Passed ) AS Fails, COUNT( Passed ) AS Total,
			MachineID, BuildID, RunID
		FROM Build, Machine, Project, Run, UnitTestCheck
		WHERE Build.MachineID = Machine.ID
		AND   Build.ProjectID = Project.ID
		AND   Run.BuildID = Build.ID
		AND   UnitTestCheck.RunID = Run.ID
	};

my $zeroTestQuery =
	qq{
		SELECT Machine.Name AS Name, Project.Name AS Project, BuildOptions, CC, Revision, Who, LocalModification,
			Run.EndDate AS Date, Run.EndTime AS Time, Compiled, 
			0 AS Passes, 0 AS Fails, 0 AS Total,
			MachineID, BuildID, RunID
		FROM Build, Machine, Project, Run
		LEFT JOIN UnitTestCheck ON Run.ID = UnitTestCheck.RunID
		WHERE Build.MachineID = Machine.ID
		AND   Build.ProjectID = Project.ID
		AND   Run.BuildID = Build.ID
		AND   UnitTestCheck.RunID IS NULL  
	};
	
my $cgi = new CGI;
my @params= $cgi->param();
my $paramKey;
my $paramValue;
my $hasDateRange = 0;
my $msg = '';

my $condition = "";

if ( @params ) {
	my $numparams = @params;
	for ( my $ii = 0; $ii < $numparams; $ii++ ) {
		if ( ! $cgi->param( $params[$ii] ) eq undef || $cgi->param( $params[$ii] ) eq '0' ) {
			if ( $params[$ii] =~ m/Date/ ) {
				$hasDateRange = 1;
			}
			$condition .= " AND " . $params[$ii] . "= '" . $cgi->param( $params[$ii] ) . "'";
			$paramValue = $cgi->param( $params[$ii] );
		}
		else {
			$paramValue = '';
		}
		# back fill the search params into the form
		if ( $params[$ii] eq 'Machine.Name' ) {
			$paramKey = 'searchMachine';
		}
		elsif ( $params[$ii] eq 'Project.Name' ) {
			$paramKey = 'searchProject';
		}
		elsif ( $params[$ii] eq 'Date>' ) {
			$paramKey = 'minDate';
		}
		elsif ( $params[$ii] eq 'Date<' ) {
			$paramKey = 'maxDate';
		}
		elsif ( $params[$ii] eq 'Time>' ) {
			$paramKey = 'minTime';
		}
		elsif ( $params[$ii] eq 'Time<' ) {
			$paramKey = 'maxTime';
		}
		else {
			$paramKey = "search" . $params[$ii];
		}
		$template->param( $paramKey => $paramValue );
	}
}

if ( $hasDateRange == 0 ) {
	#my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime time;
	my @today = localtime time;
	my @yesterday = localtime( time - 86400 );
	
	my $myMin = sprintf( "%d-%d-%d", $yesterday[5] + 1900, $yesterday[4] + 1, $yesterday[3] );
	my $myMax = sprintf( "%d-%d-%d", $today[5]     + 1900, $today[4]     + 1, $today[3]     );
	
	$condition .= " AND DATE >= '" . $myMin . "' AND DATE <= '" . $myMax . "' ";

	$template->param( minDate => $myMin );
	$template->param( maxDate => $myMax );
}

$condition .= 
	qq{
		GROUP BY Run.ID
	};

my $result = RegresstorTable->new( 
	'Name',
	'Project',
	'BuildOptions',
	'CC',
	'Revision',
	'Who',
	'LocalModification',
	'Date',
	'Time',
	'Compiled',
	'Passes',
	'Fails',
	'Total',
	'MachineID',
	'BuildID',
	'RunID' );

$queryString .= $testsQuery . $condition;
$queryString .= " UNION ";
$queryString .= $zeroTestQuery . $condition;
$queryString .=	" ORDER BY Date DESC, Name, CC, BuildOptions, Time DESC";

#$msg = $queryString;

my $query = $dbh->prepare( $queryString );
$query->execute();
$result->fillData( $query );

# Clean up DBI 
$query->finish();
$dbh->disconnect();

# fill in the query results
$template->param( data => $result->table );
$template->param( message => $msg );

# send the obligatory Content-Type and print the template output
print "Content-Type: text/html\n\n", $template->output;

