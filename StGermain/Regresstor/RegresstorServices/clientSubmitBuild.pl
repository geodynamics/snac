#!/usr/bin/perl -w

use SOAP::Lite;

print "Submitting build results to server\n";

my $response = SOAP::Lite
	-> uri( 'urn:Regresstor' )
	-> proxy( 'http://localhost/~alan/cgi-bin/RegresstorServices/Regresstor.cgi' )
	-> submitBuild(	{
		Project => "TheBlah",
		BuildOptions =>"debug",
		Revision => "123",
		MachineName => "loony.vpac.org",
		Hardware => "sgi",
		OS => "Linux",
		Kernel => "SuSE-123",
		Who => "bob",
		LocalModification => "1",
		Compiled => "1" } )
	-> result;

if ( exists $response->{errorMsg} ) {
	print "Error: $response->{errorMsg}\n";
}
else {
	print "OK. BuildID = $response->{buildID}\n";
}

