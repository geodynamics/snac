#!/usr/bin/perl -w

use BinaryServer;

use SOAP::Transport::HTTP;
SOAP::Transport::HTTP::CGI
	-> dispatch_to('BinaryServer')
	-> handle;

1;
__END__
