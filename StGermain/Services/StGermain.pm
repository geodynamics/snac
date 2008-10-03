
package StGermain;

use DBI;
use SOAP::Transport::HTTP;

use lib 'Common';
use StGermainPrivate;
use Dictionary;
use QueryShortcuts;

sub ping {
	shift;		# remove class

	unless ( @_ == 1 ) {
		StGermainPrivate->SOAP_Fault_NumArgs();
	}

	my $arguments = shift; # Hash (dictionary) of values

	StGermainPrivate->Assert_KeyIsInHash( $arguments, 'pingMessage' );

	my $talkMsg = $arguments->{pingMessage};

	return { pongMessage => $talkMsg };
}

sub giveFileTest {
	shift;		# remove class

	unless ( @_ == 1  ) {
		StGermainPrivate->SOAP_Fault_NumArgs();
	}

	my $arguments = shift;

	StGermainPrivate->Assert_KeyIsInHash( $arguments, 'FileName' );
	StGermainPrivate->Assert_KeyIsInHash( $arguments, 'FileContents' );

	my $fileName = $arguments->{Filename};
	my $fileContexts = $arguments->{FileContents};

	my $length = length $fileContexts;

	my $returnMessage = sprintf ( "File %s received. Size %s bytes", $filename, $length );

	return { retVal => $returnMessage };
}

sub submitPlugin {
	shift;		# remove class

	unless ( @_ == 1  ) {
		StGermainPrivate->SOAP_Fault_NumArgs();
	}

	my $arguments = shift;
	my $submitResult = {};

	my $pluginSubmitted = 0;

	# Mandatory info
	StGermainPrivate->Assert_KeyIsInHash( $arguments, 'Name' );
	StGermainPrivate->Assert_KeyIsInHash( $arguments, 'Version' );
	StGermainPrivate->Assert_KeyIsInHash( $arguments, 'Author' );
	StGermainPrivate->Assert_KeyIsInHash( $arguments, 'Description' );
	StGermainPrivate->Assert_KeyIsInHash( $arguments, 'Organisation' );

	my $config = Dictionary->new();
	my $dbh = DBI->connect( "DBI:mysql:$config->{database}:$config->{host}", $config->{user}, $config->{password} )
		or die StGermainPrivate->SOAP_Fault( "Unable to connect to database server $config->{database} at $config->{host} as $config->{user}" );

	my $pluginID = QueryShortcuts->GetValueInsertIfNotExists(
		$dbh,
		qq/
			SELECT ID
			FROM Plugin
			WHERE Name = '$arguments->{Name}'
			AND   Version = '$arguments->{Version}'
		/,
		qq/
			INSERT INTO Plugin( Name, Version, Author, Description, Organisation )
			VALUES ( 
				'$arguments->{Name}',
				'$arguments->{Version}',
				'$arguments->{Author}',
				'$arguments->{Description}',
				'$arguments->{Organisation}' )
		/ );

	if ( $pluginID == undef ) {
		die StGermainPrivate->SOAP_Fault( 'Unable to add Plugin information to database' );
	}
	
	if ( exists $arguments->{BinaryLib} ) {
		$submitResult->{BinaryLib} = StGermainPrivate->SubmitBinaryPlugin( $config, $dbh, $pluginID, $arguments );
		$pluginSubmitted = 1;
	}

	# Other forms of submission goes here

	unless ( $pluginSubmitted == 1 ) {
		die StGermainPrivate->SOAP_Fault( 'No plugin submitted' );
	}

	return $submitResult;
}

sub getPlugin {
	shift;		# remove class

	unless ( @_ == 1  ) {
		StGermainPrivate->SOAP_Fault_NumArgs();
	}

	my $arguments = shift;
	my $getPluginResult = {};

	my $pluginRetrived = 0;

	# Mandatory info
	StGermainPrivate->Assert_KeyIsInHash( $arguments, 'Name' );

	my $Version = undef;
	if ( exists $arguments->{Version} ) {
		$Version = $arguments->{Version};
	}

	my $config = Dictionary->new();
	my $dbh = DBI->connect( "DBI:mysql:$config->{database}:$config->{host}", $config->{user}, $config->{password} )
		or die StGermainPrivate->SOAP_Fault( "Unable to connect to database server $config->{database} at $config->{host} as $config->{user}" );

	# Potentially, in a later upgrade, Version is a range or a condition
	# Such as Version > xxx 

	# Currently, if Version specified then the plugin fetched must be of that
	# version. If not given, it fetches a list of all plugins it knows of under that name
	# in most recent order.
	my $pluginCondition;
	if ( $Version == undef ) {
		$pluginCondition="Name='$arguments->{Name}'";
	}
	else {
		$pluginCondition="Name='$arguments->{Name}' AND Version='$Version'";
	}

	# This is where the money is.
	# A smart fetching algorithm is here is what can really make this useful.
	# Potentially connect with the 'Error' service to work things out a bit here.

	# for now, you must have matching Hardware, OS and CC for a binary plugin
	if ( 	exists $arguments->{Hardware} &&
		exists $arguments->{OS} &&
		exists $arguments->{CC} )
	{
		if ( exists $config->{binaryserver} ) {
			$getPluginResult->{BinaryLib} = StGermainPrivate->GetBinaryPlugin(
				$config,
				$dbh,
				$pluginCondition,
				$arguments );
			if ( $getPluginResult->{BinaryLib} != undef ) {
				$pluginRetrived = 1;
			}
		}
		else {
			$getPluginResult->{binarydebug} = 'This server does not provide a binary download service';
		}
	}

	# Put other forms of retrival here

	unless ( $pluginRetrived == 1 ) {
		die StGermainPrivate->SOAP_Fault( 
qq/
No matching plugin found
$getPluginResult->{binarydebug}
/ );
	}

	return $getPluginResult;
}

1;
__END__
