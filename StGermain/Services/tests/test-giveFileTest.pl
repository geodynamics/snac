#!/usr/bin/perl

use SOAP::Lite;

my $urn = 'urn:StGermain';
my $url = 'http://localhost/~alan/cgi-bin/Codelet/services.cgi';


my $returnValue;

my $filename = shift;
if ( $filename eq "" ) {
	$filename = "test-giveFileTest.pl";
}
print "Using file: $filename\n";

unless ( open( MYFILE, $filename ) ) {
	die "Can't open file $filename. Exiting...\n";
}

my $contents = "";
my $line = "";

$line = <MYFILE>;
while ( $line ne "" ) { 
	$contents = $contents . $line;
	$line = <MYFILE>;
}

print "Attempting to call talk() to $url...\n";

$returnValue = SOAP::Lite
	->uri( $urn )
	->proxy( $url )
	->giveFileTest( {
		FileName => $filename,
		FileContents => $contents } );

unless ( $returnValue->fault ) {
	my $result = $returnValue->result->{retVal};
	print("$result\n" );
}
else {
	my $error = join ',',
		$returnValue->fault->{faultcode},
		$returnValue->fault->{faultstring};
	print "$error\n";
}


