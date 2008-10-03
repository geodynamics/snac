#!/usr/bin/perl

use DBI;
use SOAP::Lite;
use Data::Dumper;

use lib '../Common';
use Dictionary;
use QueryShortcuts;

my $urn = 'urn:StGermain';
my $url = 'http://localhost/~alan/cgi-bin/Codelet/services.cgi';

sub testSubmit {
	my $arg = shift;
	my $returnValue;

	print "Attempting to call talk() to $url...\n";

	$returnValue = SOAP::Lite
		->uri( $urn )
		->proxy( $url )
		->submitPlugin( $arg );

	unless ( $returnValue->fault ) {
		my $result = $returnValue->result->{BinaryLib};
		use Data::Dumper;
		print Dumper( $result );
	}
	else {
		my $error = join ',',
			$returnValue->fault->{faultcode},
			$returnValue->fault->{faultstring};
		print "$error\n";
	}
}

my $config = Dictionary->new();
my $dbh = DBI->connect( "DBI:mysql:$config->{database}:$config->{host}", $config->{user}, $config->{password} )
	or die "Unable to connect to $config->{database} at $config->{host} as $config->{user}\n";

my $maxVersion = QueryShortcuts->GetValue(
	$dbh,
	qq{
		SELECT MAX(Version)
		FROM Plugin
	} );
if ( $maxVersion == undef ) {
	$maxVersion = '0';
}

$maxVersion += 1;

# Just make a Plugin record
my $test = {
	Name => 'TestPlugin',
	Version => $maxVersion,
	Author => 'Alan Lo',
	Description => 'A dummy plugin',
	Organisation => 'VPAC' };

print "===== TEST =====\n";
print Dumper( $test );
testSubmit( $test );

# Try to submit a record with incomplete args
my $test = {
	Version => $maxVersion,
	Author => 'Alan Lo',
	Description => 'A dummy plugin',
	Organisation => 'VPAC' };
	
print "===== TEST =====\n";
print Dumper( $test );
testSubmit( $test );

# Submit a binary
my $test = {
	Name => 'TestPlugin',
	Version => $maxVersion,
	Author => 'Alan Lo',
	Description => 'A dummy plugin',
	Organisation => 'VPAC',
	Hardware => 'i686',
	OS => 'linux',
	Kernel => 'gentoo',
	CC => 'gcc',
	MachineName => 'dummy.vpac.org',
	BinaryLib => 'Pretend this string is binary data' 
};

print "===== TEST =====\n";
print Dumper( $test );
testSubmit( $test );

# Submit the binary again to force check
my $test = {
	Name => 'TestPlugin',
	Version => $maxVersion,
	Author => 'Alan Lo',
	Description => 'A dummy plugin',
	Organisation => 'VPAC',
	Hardware => 'i686',
	OS => 'linux',
	Kernel => 'gentoo',
	CC => 'gcc',
	MachineName => 'dummy.vpac.org',
	BinaryLib => 'Pretend this string is binary data' 
};

print "===== TEST =====\n";
print Dumper( $test );
testSubmit( $test );
	
