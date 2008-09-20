
package RegresstorTable;
use strict;

sub new {
	my $type = shift;
	my $self = {};

	my $argLength = @_;

	if ( $argLength < 1 ) {
		warn "Usage: RegresstorTable->new( fields, ... )\n";
	}
	bless( $self );
	$self->fields( @_ );
	$self->table( undef );

	return $self;
}

sub fields {
	my $self = shift;
	if ( @_ ) { @{ $self->{FIELDS} } = @_; }
	return @{ $self->{FIELDS} };
}

sub table {
	my $self = shift;
	if ( @_ ) { @{ $self->{TABLE} } = @_; }
	return @{ $self->{TABLE} };
}

sub fillData {
	my $self = shift;
	my $argLength = @_;

	if ( $argLength != 1 ) {
		warn "Usage: fillData( query )\n";
		return;
	}
	my $query = shift;
	my @fieldArray = $self->fields;
	my @result = ();

	while ( my @row = $query->fetchrow_array() ) {
		my %rowData;
		my $fieldLength = @fieldArray;
		for ( my $counter = 0; $counter < $fieldLength; $counter++ ) {
			$rowData{$fieldArray[$counter]} = $row[$counter];
		}

		push( @result, \%rowData );
	}
	$self->table( \@result );
}



1;
__END__

