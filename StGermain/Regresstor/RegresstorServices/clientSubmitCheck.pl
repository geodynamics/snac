#!/usr/bin/perl -w

use SOAP::Lite;

print "Submitting build results to server\n";

my @subTests = ( "a", "b", "c", "d" );


my $response = SOAP::Lite
	-> uri( 'urn:Regresstor' )
	-> proxy( 'http://localhost/~alan/cgi-bin/RegresstorServices/Regresstor.cgi' )
	-> submitCheck(	{
		BuildID => 17,
		UnitName => "Base/Foundation/BlahX",
		TestName => "testBlah.c",
		FailedSubTests => \@subTests
		} );
	-> result;

if ( exists $response->{errorMsg} ) {
	print "Error: $response->{errorMsg}\n";
}
else {
	print "OK. UnitTestCheckID = $response->{unitTestCheckID}\n";
}


__END__
