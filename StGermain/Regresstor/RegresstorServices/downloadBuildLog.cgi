#!/usr/bin/perl -w

# Simple Page which shows ALL contents of the FailedUnitSubTest table

use DBI;
use CGI;
use Compress::Zlib;

use lib '.';
use RegresstorConfig;
use RegresstorCommon;
use RegresstorTable;

# An error message
my $error;

my $cgi = new CGI;
my @params = $cgi->param();

unless ( @params ) {
	&RegresstorCommon::displayError( 'downloadBuildLog', "No ID specfied", " ", " " );
	exit;
}
unless ( $params[0] eq "ID" ) {
	&RegresstorCommon::displayError( 'downloadBuildLog', "No ID specfied", " ", " " );
	exit;
}
my $ID = $cgi->param( 'ID' );

my $config = RegresstorConfig->new();

my $dbh = DBI->connect( $config->getConnectionString, $config->user, $config->password )
		or $error = "Unable to connect to database.";
if ( $error ) {
	&RegresstorCommon::displayError( 'downloadBuildLog', $error, $config->database, $config->host );
	exit;
}
else {
	# Create SQL statement
	my $query = $dbh->prepare(
		qq{
			SELECT Log
			FROM Build 
			WHERE ID='$ID'
		});
	$query->execute();
	my @result = $query->fetchrow_array();
	unless ( @result ) {
		&RegresstorCommon::displayError( 'downloadBuildLog', "File does not exists", $config->database, $config->host );
		exit;
	}
	$query->finish();
	$dbh->disconnect();

	my $data = $result[0];
	my $uncompressedData = uncompress( $data );

	if ( ! $uncompressedData ) {
		&RegresstorCommon::displayError( 'downloadBuildLog', "No data", $config->database, $config->host );
		exit;
	}
	print "Content-Type: text/plain\n\n", $uncompressedData;
}

