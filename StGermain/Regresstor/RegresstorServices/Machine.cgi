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
	&RegresstorCommon::displayError( 'Machine', $error, $config->database, $config->host );
	exit;
}
else {
	# Create SQL statement
	my $queryString =
		qq{
			SELECT ID, Name, Hardware, OS, Kernel
			FROM Machine 
		};
		
	my $cgi = new CGI;
	my @params= $cgi->param();
	if ( @params ) {
		my $numparams = @params;
		for ( my $ii = 0; $ii < $numparams; $ii++ ) {
			if ( ! $cgi->param( $params[$ii] ) eq undef || $cgi->param( $params[$ii] ) eq '0' ) {
				$queryString = $queryString . " AND " . $params[$ii] . " = '" . $cgi->param( $params[$ii] ) . "'";
			}
		}
	}
														
	my $query = $dbh->prepare( $queryString );

	# Fire up the SQL command
	$query->execute();

	# Translate results into perl data structure
	my $result = RegresstorTable->new( 'ID', 'Name', 'Hardware', 'OS', 'Kernel' );
	$result->fillData( $query );

	# Clean up DBI 
	$query->finish();
	$dbh->disconnect();

	# open the html template
	my $template = HTML::Template->new( filename => 'Machine.tmpl');

	# fill in some parameters
	$template->param( data => $result->table );

	# send the obligatory Content-Type and print the template output
	print "Content-Type: text/html\n\n", $template->output;
}

