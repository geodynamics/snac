package QueryShortcuts;
use strict;

use DBI;

sub Exists {
	shift;		# remove class name
	unless ( @_ == 2 ) {
		return undef;
	}

	my $dbh = shift;
	my $queryString = shift;
	
	my $query = $dbh->prepare( $queryString );
	$query->execute();
	
	my @queryResult = $query->fetchrow_array();
	$query->finish();

	if ( @queryResult ) {
		return 1;
	}
	else {
		return 0;
	}
}

sub GetValue {
	shift;		# remove class name
	unless ( @_ == 2 ) {
		return undef;
	}

	my $dbh = shift;
	my $queryString = shift;

	my $resultValue;
	my $query = $dbh->prepare( $queryString );
	$query->execute();
	
	my @queryResult = $query->fetchrow_array();
	$query->finish();

	if ( @queryResult ) {
		$resultValue = $queryResult[0];
	}
	else {
		$resultValue = undef;
	}

	return $resultValue;
}

sub Insert {
	shift;		# remove class name
	unless ( @_ == 2 ) {
		return;
	}

	my $dbh = shift;
	my $insertString = shift;
	
	my $insertCommand = $dbh->prepare( $insertString );
	$insertCommand->execute();
	$insertCommand->finish();
}

sub GetValueInsertIfNotExists {
	shift;		# remove class name
	unless ( @_ == 3 ) {
		return undef;
	}

	my $dbh = shift;
	my $queryString = shift;
	my $insertString = shift;
	
	my $value = QueryShortcuts->GetValue( $dbh, $queryString );

	if ( $value == undef ) {
		QueryShortcuts->Insert( $dbh, $insertString );

		$value = QueryShortcuts->GetValue( $dbh, $queryString );
	}
	
	return $value;
}

sub GetTuple {
	shift;		# remove class name
	unless ( @_ == 2 ) {
		return undef;
	}

	my $dbh = shift;
	my $queryString = shift;

	my $query = $dbh->prepare( $queryString );
	$query->execute();
	
	my @queryResult = $query->fetchrow_array();
	$query->finish();

	return @queryResult;
}

sub GetData {
	shift;		# remove class name
	unless ( @_ == 2 ) {
		return undef;
	}

	my $dbh = shift;
	my $queryString = shift;

	my @results = ();
	my $query = $dbh->prepare( $queryString );
	$query->execute();

	while ( my @row = $query->fetchrow_array() ) {
		push( @results, \@row );
	}
	$query->finish();

	return @results;
}


1;
__END__
