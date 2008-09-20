package Dictionary;
use strict;

use XML::Simple;

my $Dictionary_Default_Filename = "Config.xml";

sub new {
	my $type = shift;
	my $self = {};
	bless( $self );

	if ( @_ ) {
		$self->readXMLFile( @_ );
	}
	else {
		$self->readXMLFile( $Dictionary_Default_Filename );
	}
	
	return $self;
}
sub readXMLFile() {
	my $self = shift;
	my $argLength = @_;
	unless ( $argLength == 1 ) {
		return;
	}
	my $xml = XML::Simple->new();
	my @data = $xml->XMLin( $_[0] );
	my %config = %{ $data[0] };

	foreach my $dataKey ( keys(%config) ) {
		$self->{$dataKey} = $config{$dataKey};
	}
}

1;
__END__
