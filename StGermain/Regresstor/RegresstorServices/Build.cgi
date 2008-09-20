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

my $config = RegresstorConfig->new();

my $dbh = DBI->connect( $config->getConnectionString, $config->user, $config->password )
		or $error = "Unable to connect to database.";

if ( $error ) {
	&RegresstorCommon::displayError( 'Build', $error, $config->database, $config->host );
	exit;
}
else {
	# Create SQL statement
	my $queryString = 
		qq{
			SELECT Build.ID, Project.Name AS Project, BuildOptions, CC, Revision,
				MachineID, Machine.Name AS MachineName, Who, LocalModification,
				Date, Time, Compiled
			FROM Build, Project, Machine
			WHERE Build.ProjectID = Project.ID
			AND   Build.MachineID = Machine.ID
		};

	my $cgi = new CGI;
	my @params= $cgi->param();
	if ( @params ) {
		my $numparams = @params;
		for ( my $ii = 0; $ii < $numparams; $ii++ ) {
			if ( ! $cgi->param( $params[$ii] ) eq undef || $cgi->param( $params[$ii] ) eq '0' ) {
				$queryString = $queryString . " AND " . $params[$ii] . "= '" . $cgi->param( $params[$ii] ) . "'";
			}
		}
	}

	my $queryString = $queryString . " ORDER BY Date DESC, Time DESC";

	my $query = $dbh->prepare( $queryString );

	# Fire up the SQL command
	$query->execute();

	# Translate results into perl data structure
	my $result = RegresstorTable->new( 
		'ID',
		'Project',
		'BuildOptions', 
		'CC',
		'Revision', 
		'MachineID', 
		'MachineName',
		'Who', 
		'LocalModification', 
		'Date', 
		'Time', 
		'Compiled' );
	$result->fillData( $query );

	# Clean up DBI 
	$query->finish();
	$dbh->disconnect();

	# open the html template
	my $template = HTML::Template->new( filename => 'Build.tmpl');

	# fill in some parameters
	$template->param( data => $result->table );

	# send the obligatory Content-Type and print the template output
	print "Content-Type: text/html\n\n", $template->output;
}

