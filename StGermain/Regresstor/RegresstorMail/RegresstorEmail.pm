
package RegresstorEmail;
use strict;

use Net::SMTP;

sub new() {
	my $type = shift;
	my $self = {};

	$self->{PROJECT};
	$self->{DATE};

	$self->{MAILTO};
	$self->{MAILFROM};
	$self->{SUBJECT};
	$self->{CONTENTS};
	bless( $self );

	return $self;
}

sub project {
	my $self = shift;
	if (@_) { $self->{PROJECT} = shift; }
	return $self->{PROJECT};
}

sub date {
	my $self = shift;
	if (@_) { $self->{DATE} = shift; }
	return $self->{DATE};
}

sub mailto {
	my $self = shift;
	if (@_) { $self->{MAILTO} = shift; }
	return $self->{MAILTO};
}

sub mailfrom {
	my $self = shift;
	if (@_) { $self->{MAILFROM} = shift; }
	return $self->{MAILFROM};
}

sub subject {
	my $self = shift;
	if (@_) { $self->{SUBJECT} = shift; }
	return $self->{SUBJECT};
}

sub contents {
	my $self = shift;
	if (@_) { $self->{CONTENTS} = shift; }
	return $self->{CONTENTS};
}

sub sendmail {
	my $self = shift;

	unless ( @_ ) {
		print "You must provide an smtp server to send with!\n";
		die;
	}
	my $smtp = shift;

	my $mail = Net::SMTP->new( $smtp ) or die "Failed to create email.\n";

	$mail->mail( $self->{MAILFROM} );
	$mail->to( $self->{MAILTO} );

	$mail->data();
	$mail->datasend( "To: $self->{MAILTO}\n" );
	$mail->datasend( "Subject: $self->{SUBJECT}\n" );
	$mail->datasend( "Content-type: text/plain\n\n" );
	$mail->datasend( "$self->{CONTENTS}\n" );
	$mail->dataend();

	$mail->quit(); # Send email 
}

1;
__END__

