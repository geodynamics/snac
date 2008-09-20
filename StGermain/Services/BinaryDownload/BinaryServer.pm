
package BinaryServer;

use lib '../Common';
use DBI;
use SOAP::Transport::HTTP;

use StGermainPrivate;
use Dictionary;
use QueryShortcuts;

sub getBinary {
	shift;		# remove class

	unless ( @_ == 1  ) {
		StGermainPrivate->SOAP_Fault_NumArgs();
	}

	my $arguments = shift;
	my $getPluginResult = {};

	my $pluginRetrived = 0;

	# Mandatory info
	StGermainPrivate->Assert_KeyIsInHash( $arguments, 'IDs' );

	my $config = Dictionary->new();
	my $dbh = DBI->connect( "DBI:mysql:$config->{database}:$config->{host}", $config->{user}, $config->{password} )
		or die StGermainPrivate->SOAP_Fault( "Unable to connect to database server $config->{database} at $config->{host} as $config->{user}" );

	# Very simple approach to downloading plugins
	# Receive a list of possible plugins to download and attempt to fetch them in order

	my @IDs = @{ $arguments->{IDs} };
	my $l = @IDs;
	foreach $ID_I ( 0 .. @IDs - 1 ) {
		if ( $pluginRetrived == 0 ) {
			my @plugin = QueryShortcuts->GetTuple(
				$dbh,
				qq/
					SELECT Name, Version, Author, Description, Organisation,
						Hardware, OS, Kernel, CC, MachineName, BinaryLib
					FROM Plugin, CompiledBinary
					WHERE Plugin.ID = CompiledBinary.PluginID
					AND   CompiledBinary.ID = '$IDs[$ID_I]'
				/ );
			if ( @plugin ) {
				$getPluginResult = {
					Name => $plugin[0],
					Version => $plugin[1],
					Author => $plugin[2],
					Description => $plugin[3],
					Organisation => $plugin[4],
					Hardware => $plugin[5],
					OS => $plugin[6],
					Kernel => $plugin[7],
					CC => $plugin[8],
					MachineName => $plugin[9],
					BinaryLib => $plugin[10] 
				};
				$pluginRetrived = 1;
			}
		}
	}

	unless ( $pluginRetrived == 1 ) {
		die StGermainPrivate->SOAP_Fault( 'Unable to retrive plugin' );
	}

	return $getPluginResult;
}

1;
__END__
