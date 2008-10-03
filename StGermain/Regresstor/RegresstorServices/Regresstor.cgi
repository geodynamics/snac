#!/usr/bin/perl -w

use SOAP::Transport::HTTP;

SOAP::Transport::HTTP::CGI
	-> dispatch_to('Regresstor')
	-> handle;

