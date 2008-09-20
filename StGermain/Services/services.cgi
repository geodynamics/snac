#!/usr/bin/perl -w

use StGermain;

use SOAP::Transport::HTTP;
SOAP::Transport::HTTP::CGI
	-> dispatch_to('StGermain')
	-> handle;

1;
__END__
