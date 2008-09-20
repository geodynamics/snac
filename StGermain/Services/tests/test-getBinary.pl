#!/usr/bin/perl

use DBI;
use SOAP::Lite;
use Data::Dumper;

use lib '../Common';
use Dictionary;
use QueryShortcuts;

my $urn = 'urn:StGermain';
my $url = 'http://localhost/~alan/cgi-bin/Codelet/services.cgi';

my $downloadDir='downloads';

sub testGetPlugin {
	my $arg = shift;
	my $returnValue;

	print "Attempting to call talk() to $url...\n";

	$returnValue = SOAP::Lite
		->uri( $urn )
		->proxy( $url )
		->getPlugin( $arg );

	my $result = $returnValue->result;
	unless ( $returnValue->fault ) {
		use Data::Dumper;
		print Dumper( $result );
	}
	else {
		my $error = join ',',
			$returnValue->fault->{faultcode},
			$returnValue->fault->{faultstring};
		print "$error\n";
		exit 1;
	}

	if ( exists $result->{BinaryLib} ) {
		my $binaryLibInfo = $result->{BinaryLib};
		my @CompiledBinaryIDs = @{ $binaryLibInfo->{CompiledBinaryIDs} };
		print "Received binary download url $binaryLibInfo->{URL}\n";
		print "Possible download ID numbers: ";
		foreach $downloadID_I ( 0 .. @CompiledBinaryIDs - 1 ) {
			print "$CompiledBinaryIDs[$downloadID_I] ";
		}
		print "\n";

		print "Downloading..\n";

		my $binaryArgs = {
			IDs => \@CompiledBinaryIDs
		};
		$returnValue = SOAP::Lite
			->uri( 'urn:BinaryServer' )
			->proxy( $binaryLibInfo->{URL} )
			->getBinary( $binaryArgs );
			
		$result = $returnValue->result;
		unless ( $returnValue->fault ) {
			use Data::Dumper;
			print Dumper( $result );
		}
		else {
			my $error = join ',',
				$returnValue->fault->{faultcode},
				$returnValue->fault->{faultstring};
			print "$error\n";
			exit 1;
		}

		$filename = "$downloadDir/$result->{Name}module.so";
		unless ( open( WRITEFILE, ">$filename" ) ) {
			die "Unable to open file $filename for writing\n";
		}

		print WRITEFILE $result->{BinaryLib};
	}
}

my $config = Dictionary->new();
my $dbh = DBI->connect( "DBI:mysql:$config->{database}:$config->{host}", $config->{user}, $config->{password} )
	or die "Unable to connect to $config->{database} at $config->{host} as $config->{user}\n";

# get the first of any latest plugin
my @plugin = QueryShortcuts->GetTuple(
	$dbh,
	qq{
		SELECT Name, MAX(Version), Hardware, OS, CC
		FROM Plugin, CompiledBinary
		WHERE Plugin.ID = CompiledBinary.PluginID
		GROUP BY Name
	} );

# Ask for a plugin info
my $test = {
	Name => $plugin[0],
	Version => $plugin[1],
	Hardware => $plugin[2],
	OS => $plugin[3],
	CC => $plugin[4]
};

print "===== TEST =====\n";
print Dumper( $test );
testGetPlugin( $test );
	
# Ask for a plugin info with any version
my $test = {
	Name => $plugin[0],
	Hardware => $plugin[2],
	OS => $plugin[3],
	CC => $plugin[4]
};

print "===== TEST =====\n";
print Dumper( $test );
testGetPlugin( $test );
	
