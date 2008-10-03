package StGermainPrivate;
use strict;

use DBI;
use SOAP::Transport::HTTP;

use QueryShortcuts;

sub SOAP_Fault {
	shift;		# remove class
	my $message = shift;

	return SOAP::Fault
		->faultcode( 'StGermain.Error' )
		->faultstring( $message );
}

sub SOAP_Fault_NumArgs {
	shift;		# remove class
	die StGermainPrivate->SOAP_Fault( 'Invalid number of arguments' );
}
sub SOAP_Fault_NoKey {
	shift;		# remove class
	my $key = shift;
	die StGermainPrivate->SOAP_Fault( "No value for {$key}" );
}

sub Assert_KeyIsInHash {
	shift;		# remove class
	my $hash = shift;
	my $key = shift;

	unless ( exists $hash->{$key} ) {
		StGermainPrivate->SOAP_Fault_NoKey( $key );
	}
}

sub SubmitBinaryPlugin {
	shift;		# remove class
	my $config = shift;
	my $dbh = shift;
	my $pluginID = shift;
	my $arguments = shift;

	StGermainPrivate->Assert_KeyIsInHash( $arguments, 'Hardware' );
	StGermainPrivate->Assert_KeyIsInHash( $arguments, 'OS' );
	StGermainPrivate->Assert_KeyIsInHash( $arguments, 'Kernel' );
	StGermainPrivate->Assert_KeyIsInHash( $arguments, 'CC' );
	StGermainPrivate->Assert_KeyIsInHash( $arguments, 'MachineName' );

	my $compiledBinaryID = QueryShortcuts->GetValue(
		$dbh,
		qq/
			SELECT ID
			FROM CompiledBinary
			WHERE PluginID = '$pluginID'
			AND   Hardware = '$arguments->{Hardware}'
			AND   OS = '$arguments->{OS}'
			AND   CC = '$arguments->{CC}'
		/ );

	if ( $compiledBinaryID != undef ) {
		return { message => "Binary for {$arguments->{Name}, $arguments->{Version}, $arguments->{Hardware}, $arguments->{OS}, $arguments->{CC}} already exists" };
	}

	QueryShortcuts->Insert(
		$dbh,
		qq/
			INSERT INTO CompiledBinary( PluginID, Hardware, OS, Kernel, CC, MachineName, BinaryLib )
			VALUES ( 
				'$pluginID',
				'$arguments->{Hardware}',
				'$arguments->{OS}',
				'$arguments->{Kernel}',
				'$arguments->{CC}',
				'$arguments->{MachineName}',
				'$arguments->{BinaryLib}' )
		/ );

	return { message => "Binary plugin sucessfully submitted" };
}

sub GetBinaryPlugin {
	shift;		# remove class
	my $config = shift;
	my $dbh = shift;
	my $pluginCondition = shift;
	my $arguments = shift;

	my $queryString =
		qq/
			SELECT CompiledBinary.ID
			FROM Plugin, CompiledBinary
			WHERE Plugin.ID = CompiledBinary.PluginID
		/;

	$queryString = $queryString . " AND " . $pluginCondition;

	# Use 3 separate if statements
	# Although all three values are required now, they could potentially be omitted too.

	if ( exists $arguments->{Hardware} ) {
		$queryString = $queryString . " AND Hardware='$arguments->{Hardware}'";
	}
	if ( exists $arguments->{OS} ) {
		$queryString = $queryString . " AND OS='$arguments->{OS}'";
	}
	if ( exists $arguments->{CC} ) {
		$queryString = $queryString . " AND CC='$arguments->{CC}'";
	}

	$queryString = $queryString . " ORDER BY Version DESC ";

	my @CompiledBinaryIDs = QueryShortcuts->GetData( $dbh, $queryString );
	my @idList = ();

	# Recompact the data into a single array
	foreach my $binaryID_I ( 0 .. @CompiledBinaryIDs - 1 ) {
		my @binaryIDRow = @{ $CompiledBinaryIDs[$binaryID_I] };
		push( @idList, $binaryIDRow[0] );
	}

	if ( @CompiledBinaryIDs ) {
		return {
			URL => $config->{binaryserver},
			CompiledBinaryIDs => \@idList
		};
	}
	return undef;
}



1;
__END__

