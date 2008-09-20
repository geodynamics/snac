
package RegresstorMailConfig;
use strict;

use XML::Simple;

my $defaultConfigFilename = "RegresstorMailConfig.xml";

sub new {
	my $type = shift;
	my $self = {};
	$self->{DATABASE} = undef;
	$self->{HOST} = undef;
	$self->{USER} = undef;
	$self->{PASSWORD} = undef;
	$self->{SMTPSERVER} = undef;
	$self->{REGRESSTORSERVER} = undef;
	bless( $self );

	if ( @_ ) {
		$self->readConfigFile( @_ );
	}
	else {
		$self->readConfigFile( $defaultConfigFilename );
	}
	
	return $self;
}
sub readConfigFile() {
	my $self = shift;
	my $argLength = @_;
	unless ( $argLength == 1 ) {
		warn "Usage readConfigFile( FILENAME )\n";
		return;
	}
	my $xml = XML::Simple->new();
	my @data = $xml->XMLin( $_[0] );
	
	$self->database( $data[0]->{database} );
	$self->host( $data[0]->{host} );
	$self->user( $data[0]->{user} );
	$self->password( $data[0]->{password} );
	$self->smtpserver( $data[0]->{smtpserver} );
	$self->regresstorserver( $data[0]->{regresstorserver} );
}

sub database {
	my $self = shift;
	if (@_) { $self->{DATABASE} = shift; }
	return $self->{DATABASE};
}

sub host {
	my $self = shift;
	if (@_) { $self->{HOST} = shift; }
	return $self->{HOST};
}

sub user {
	my $self = shift;
	if (@_) { $self->{USER} = shift; }
	return $self->{USER};
}

sub password {
	my $self = shift;
	if (@_) { $self->{PASSWORD} = shift; }
	return $self->{PASSWORD};
}

sub smtpserver {
	my $self = shift;
	if (@_) { $self->{SMTPSERVER} = shift; }
	return $self->{SMTPSERVER};
}

sub regresstorserver {
	my $self = shift;
	if (@_) { $self->{REGRESSTORSERVER} = shift; }
	return $self->{REGRESSTORSERVER};
}

sub getConnectionString {
	my $self = shift;
	return "DBI:mysql:" . $self->database . ":" . $self->host;
}

1;
__END__
