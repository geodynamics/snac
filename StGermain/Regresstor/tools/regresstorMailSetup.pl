#!/usr/bin/perl -w

# regresstorMailSetup.pl
#
# Perl script which manages the installation of the regresstor database on a machine.

use strict;

use DBI;
use XML::Simple;

my @commands = ( "createconfig", "projectsetup" );
my @descriptions = (
	"Creates xml configuration file for email scripts.",
	"Add/Change mail settings for projects on the regresstor database" );
my @helpFunctions = (
	"printCreateConfig",
	"printProjectSetup" );
my @commandFunctions = (
	"commandCreateConfig",
	"commandProjectSetup" );

my @options = ( "smtpserver", "regresstorserver", "database", "host", "user", "password", "filename", "project", "mailto", "mailfrom" );
my $defaults = {
	smtpserver => '',
	regresstorserver => 'http://localhost/cgi-bin/RegresstorServices',
	database => 'regressiondb',
	host => 'localhost',
	user => 'regresstoruser',
	password => 'regresstor',
	filename => './RegresstorMailConfig.xml',
	project => '',
	mailto => '',
	mailfrom => '' };
my $values = {
	smtpserver => undef,
	regresstorserver => undef,
	database => undef,
	host => undef,
	user => undef,
	password => undef,
	filename => undef,
	project => undef,
	mailto => undef,
	mailfrom => undef };

# Prints summary of tool usage
sub printUsage() {
	print "regresstorMailSetup.pl\n";
	print "Unified tool for installing the Regresstor database.\n";
	print "Usage:\n";
	print "\tregresstorMailSetup.pl <command> [args]\n";
	print "or\n";
	print "\tregresstorMailSetup.pl help <command>\n";
	print "\n";
	print "Available commands:\n";
	foreach my $i ( 0 .. $#commands ) {
		print "\t$commands[$i]\n";
	}
}
# Prints the common requirements for each command
sub printRequirements() {
	print "Requirements Checklist:\n";
	print "\t- Perl\n";
	print "\t- Perl Net::SMTP\n";
}
# Prints the common options of each command
sub printCommonOptions() {
	print "Options:\n";
	print "\tdatabase=[$defaults->{database}]\t: The name of the Regresstor database\n";
	print "\thost=[$defaults->{host}]\t: The host of the database server \n";
	print "\tuser=[$defaults->{user}]\t: The user name for database access\n";
	print "\tpassword=[$defaults->{password}]\t: The password for database access. Note: use simple non-critical one.\n";
}
# Prints the createconfig command
sub printCreateConfig() {
	print "Creates a configuration file for the Regresstor web services to use.\n";
	print "\n";
	print "regresstorMailSetup.pl createconfig [options]\n";
	&printCommonOptions();
	print "\tfilename=[$defaults->{filename}]\t: The file to write configurations to.\n";
	print "\tsmtpserver=[$defaults->{smtpserver}]\t: The smtp server for sending email.\n";
	print "\tregresstorserver[$defaults->{regresstorserver}]\t: The server for regresstor web services.\n";
}

# Prints the projectsetup command
sub printProjectSetup() {
	print "Add/Change mail settings for a project in the regresstor database.\n";
	print "\n";
	print "regresstorMailSetup.pl projectsetup [options]\n";
	&printCommonOptions();
	print "\tproject=[$defaults->{project}]\t: The name of the project to set.\n";
	print "\tmailto=[$defaults->{mailto}]\t: The mail list to send results to.\n";
	print "\tmailfrom=[$defaults->{mailfrom}]\t: The email to reply to.\n";
}

if ( $#ARGV < 0 ) {
	&printUsage();
	exit;
}

my $command = $ARGV[0];
if ( $command eq "--help" ) {
	&printUsage();
	exit;
}
if ( $command eq "help" ) {
	if ( $#ARGV < 1) {
		&printUsage();
		exit;
	}
	else {
		my $helpCommand = $ARGV[1];
		foreach my $i ( 0 .. @commands - 1 ) {
			if ( $helpCommand eq $commands[$i] ) {
				eval( $helpFunctions[$i] );
				exit;
			}
		}
		print "Unknown Command.\n";
		&printUsage();
		exit;
	}
}

# Fetch options for commands 
foreach my $argnum ( 1 .. $#ARGV ) {
	if ( $ARGV[$argnum] =~ /=/ ) {
		my @param = ();
		my $length = 0;
		
		@param = split( /=/, $ARGV[$argnum] );
		$length = @param;

		my $found = 0;
		
		if ( ! $length == 2 ) {
			print "Invalid option: $ARGV[$argnum]\n";
			print "Please specify in the form of <param>=<value>\n";
			exit;
		}
		foreach my $i ( 0 .. @options - 1 ) {
			if ( $param[0] eq $options[$i] ) {
				$values->{$options[$i]} = $param[1];
				$found = 1;
			}
		}
		if ( ! $found ) {
			print "Unknown param: $param[0]\n";
			exit;
		}
	}
	else {
		print "Invalid option: $ARGV[$argnum]\n";
		print "Please specify in the form of <param>=<value>\n";
		exit;
	}
}

foreach my $i ( 0 .. @options -1 ) {
	if ( ! $values->{$options[$i]} ) {
		$values->{$options[$i]} = $defaults->{$options[$i]};
	}
}

print "Using setttings:\n";
print "database = $values->{database}\n";
print "host = $values->{host}\n";
print "user = $values->{user}\n";
print "password= $values->{password}\n";
print "\n";

sub commandCreateConfig() {
	print "Creating configuration file...\n";

	unless ( $values->{smtpserver} ) {
		print "Error. You need to specify a smtpserver to send email with.\n";
		die;
	}

	unless ( $values->{regresstorserver} ) {
		print "Error. You need to specify regresstorserver to say where web services are located.\n"; 
		die;
	}

	my $xml = new XML::Simple(
	        NoAttr=>1,
        	RootName=>'config' );
                                                                                                                                    
	my $data = $xml->XMLout( $values );
	open configOutput, ">$values->{filename}"
		or die "Unable to open file $values->{filename} for writing.\n";
	print configOutput $data;
	close configOutput;
	print "Successfully writting configurations to $values->{filename}.\n";
	print "Copy $values->{filename} to your RegresstorMail directory on your mail server.\n";
}

sub commandProjectSetup() {
	
	unless ( $values->{project} ) {
		print "Required param: project\n";
		die;
	}
	unless ( $values->{mailto} ) {
		print "Required param: mailto\n";
		die;
	}
	unless ( $values->{mailfrom} ) {
		print "Required param: mailfrom\n";
		die;
	}

	print "Adding/Changing settings for project $values->{project}...\n";

	my $dbh = DBI->connect(
		"DBI:mysql:$values->{database}:$values->{host}",
		$values->{user},
		$values->{password} );
        unless ( $dbh ) {
                print "Unable to connect to database $values->{database} on $values->{host}. Exiting...\n";
                die;
        }

	my $projectID;
	my $projectIDQuery = $dbh->prepare(
		qq/
			SELECT ID
			FROM Project
			WHERE Name='$values->{project}'
		/ );
	$projectIDQuery->execute();
	my @projectResult = $projectIDQuery->fetchrow_array();
	$projectIDQuery->finish();

	my $projectCommand;
	
	if ( @projectResult ) {
		$projectID = $projectResult[0];

		$projectCommand = $dbh->prepare(
			qq/
				UPDATE Project
				SET MailTo='$values->{mailto}', MailFrom='$values->{mailfrom}'
				WHERE ID='$projectID'
			/ );
	}
	else {
		# Create new project
		$projectCommand= $dbh->prepare(
			qq/
				INSERT INTO Project ( Name, MailTo, MailFrom )
				VALUES ( '$values->{project}', '$values->{mailto}', '$values->{mailfrom}' )
			/ );
	}
	$projectCommand->execute();
	$projectCommand->finish();

	my $projectQuery = $dbh->prepare(
		qq/
			SELECT ID, MailTo, MailFrom
			FROM Project
			WHERE Name='$values->{project}'
		/ );
	$projectQuery->execute();
	@projectResult = $projectQuery->fetchrow_array();
	$projectQuery->finish();

	$dbh->disconnect();
	
	my $commandFailed = 1;
	
	if ( @projectResult ) {
		if ( $projectResult[1] eq $values->{mailto} && $projectResult[2] eq $values->{mailfrom} ) {
			$commandFailed = 0;
		}
	}

	if ( $commandFailed ) {
		print "Failed to add/change project settings.\n";
		die;
	}

	print "Successfully add/changed settings for project.\n";
}

foreach my $i ( 0 .. @commands - 1 ) {
	if ( $command eq $commands[$i] ) {
		eval( $commandFunctions[$i] );
		exit;
	}
}
print "Unknown command $command.\n";

__END__

