
package RegresstorCommon;
use strict;

use HTML::Template;

my $errorTemplate = "Error.tmpl";

sub displayError() {
	my $argLength = @_;

	unless ( $argLength == 4 ) {
		warn "Usage: displayError( title, errorMsg, database, host )\n";
		return;
	}

	my $template = HTML::Template->new( filename => $errorTemplate );
	$template->param( title => $_[0] );
	$template->param( errorMsg => $_[1] );
	$template->param( database => $_[2] );
	$template->param( host => $_[3] );
	print "Content-Type: text/html\n\n", $template->output;
}

1
;
__END__
