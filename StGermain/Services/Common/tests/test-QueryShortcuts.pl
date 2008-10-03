#!/usr/bin/perl

use DBI;
use Data::Dumper;

use lib '..';
use Dictionary;
use QueryShortcuts;

my $config = Dictionary->new();
my $dbh = DBI->connect( "DBI:mysql:$config->{database}:$config->{host}", $config->{user}, $config->{password} )
	or die "Unable to connect to $config->{database} at $config->{host} as $config->{user}\n";

my $existsPlugin = QueryShortcuts->Exists(
	$dbh,
	qq{
		SELECT ID
		FROM Plugin
	} );
if ( $existsPlugin == 1 ) {
	print "Plugin table has data\n";
}
else {
	print "Plugin table does not have data\n";
}

my $maxVersion = QueryShortcuts->GetValue(
	$dbh,
	qq{
		SELECT MAX(Version)
		FROM Plugin
	} );
if ( $maxVersion == undef ) {
	$maxVersion = '0';
}

printf "$maxVersion\n";


my $pluginID = QueryShortcuts->GetValueInsertIfNotExists(
	$dbh,
	qq/
		SELECT ID
		FROM Plugin
		WHERE Name = 'QStest'
		AND   Version = '1'
	/,
	qq/
		INSERT INTO Plugin( Name, Version, Author, Description, Organisation )
		VALUES (
			'QStest',
			'1',
			'Alan Lo',
			'blah blah blah',
			'VPAC' )
	/ );

if ( $pluginID == undef ) {
	print "Can't insert\n";
}
else {
	print "Plugin ID is $pluginID\n";
}

my @data = QueryShortcuts->GetData(
	$dbh,
	qq/
		SELECT *
		FROM CompiledBinary
	/ );

print Dumper( @data );
