#!/usr/bin/perl

use SOAP::Lite;

my $urn = 'urn:StGermain';
my $url = 'http://localhost/~alan/cgi-bin/Codelet/services.cgi';

print "Attempting to call talk() to $url...\n";

my $returnValue;

$returnValue = SOAP::Lite
	->uri( $urn )
	->proxy( $url )
	->ping( { pingMessage => "Hello there!" } );

use Data::Dumper;

print "===== Env    ===========================\n";
print Dumper( $returnValue );
print "===== Result ===========================\n";
print Dumper( $returnValue->result );
print "===== Fault  ===========================\n";
print Dumper( $returnValue->fault );


print "===== Output ===========================\n";
unless ( $returnValue->fault ) {
	my $result = $returnValue->result->{pongMessage};
	print("$result\n" );
}
else {
	my $error = join ',',
		$returnValue->fault->{faultcode},
		$returnValue->fault->{faultstring};
	print "$error\n"
}


