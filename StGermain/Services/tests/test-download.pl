#!/usr/bin/perl

use DBI;
use SOAP::Lite;
use Data::Dumper;

my $urn = 'urn:BinaryServer';
my $url = 'http://localhost/~alan/cgi-bin/Codelet/BinaryDownload/services.cgi';

my $downloadDir='downloads';

print "Attempting to call talk() to $url...\n";

my @args = ( '1' );

$returnValue = SOAP::Lite
	->uri( $urn )
	->proxy( $url )
	->getBinary( { IDs => \@args } );

my $result = $returnValue->result;
unless ( $returnValue->fault ) {
	use Data::Dumper;
	print Dumper( $result );
	my $fileName = $downloadDir . "/" . $result->{Name} . "module.so";
	print "Writing to file $fileName...\n";
	open(FILEWRITE, ">$fileName");
	print FILEWRITE $result->{BinaryLib};
}
else {
	my $error = join ',',
		$returnValue->fault->{faultcode},
		$returnValue->fault->{faultstring};
	print "$error\n";
	exit 1;
}

