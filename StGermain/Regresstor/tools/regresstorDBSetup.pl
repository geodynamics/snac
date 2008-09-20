#!/usr/bin/perl -w

# regresstorDBSetup.pl
#
# Perl script which manages the installation of the regresstor database on a machine.

use strict;

use DBI;
use XML::Simple;

my @commands = ( "install", "createdb", "createtable", "createconfig" );
my @descriptions = (
	"This is the command to call when installing Regresstor database from scratch.",
	"Creates the database only.",
	"Creates database table schema only.",
	"Creates xml configuration file for Regresstor web services." );
my @helpFunctions = (
	"printInstall",
	"printCreateDB",
	"printCreateTable",
	"printCreateConfig" );
my @commandFunctions = (
	"commandInstall",
	"commandCreateDB",
	"commandCreateTable",
	"commandCreateConfig" );

my @options = ( "database", "host", "user", "password", "filename" );
my $defaults = {
	database => 'regresstordb',
	host => 'localhost',
	user => 'regresstor',
	password => 'regresstor123',
	filename => './RegresstorConfig.xml' };
my $values = {
	database => undef,
	host => undef,
	user => undef,
	password => undef,
	filename => undef };

# Prints summary of tool usage
sub printUsage() {
	print "regresstorDBSetup.pl\n";
	print "Unified tool for installing the Regresstor database.\n";
	print "Usage:\n";
	print "\tregresstorDBSetup.pl <command> [args]\n";
	print "or\n";
	print "\tregresstorDBSetup.pl help <command>\n";
	print "\n";
	print "Available commands:\n";
	foreach my $i ( 0 .. $#commands ) {
		print "\t$commands[$i]\n";
	}
}
# Prints the common requirements for each command
sub printRequirements() {
	print "Requirements Checklist:\n";
	print "\t- MYSQL\n";
	print "\t- mysqld server running\n";
	print "\t- MYSQL user with database creation permissions\n";
}
# Prints the common options of each command
sub printCommonOptions() {
	print "Options:\n";
	print "\tdatabase=[$defaults->{database}]\t: The name of the Regresstor database\n";
	print "\thost=[$defaults->{host}]\t: The host of the database server \n";
	print "\tuser=[$defaults->{user}]\t: The user name for database access\n";
	print "\tpassword=[$defaults->{password}]\t: The password for database access. Note: use simple non-critical one.\n";
}
# Prints install command
sub printInstall() {
	print "Performs all operations: createdb, createtable and createconfig.\n";
	&printRequirements();
	print "\n";
	print "regresstorDBSetup.pl install [options]\n";
	&printCommonOptions();
}
# Prints the createdb command
sub printCreateDB() {
	print "Creates the database on mysql.\n";
	&printRequirements();
	print "\n";
	print "regresstorDBSetup.pl createdb [options]\n";
	&printCommonOptions();
}
# Prints the createtable command
sub printCreateTable() {
	print "Creates the database tables on the target existing mysql database\n";
	&printRequirements();
	print "\n";
	print "regresstorDBSetup.pl createtable [options]\n";
	&printCommonOptions();
}
# Prints the createconfig command
sub printCreateConfig() {
	print "Creates a configuration file for the Regresstor web services to use.\n";
	print "\n";
	print "regresstorDBSetup.pl createconfig [options]\n";
	&printCommonOptions();
	print "\tfilename=[$defaults->{filename}]\t: The file to write configurations to.\n";
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

sub commandCreateDB() {

	print "Creating database '$values->{database}'...\n";

	my $dbh;
	
	# Check if database already exists
	$dbh = DBI->connect( 
		"DBI:mysql:$values->{database}:$values->{host}", 
		$values->{user},
		$values->{password},
		{ PrintError => 0, RaiseError => 0 } );
	if ( $dbh ) {
		$dbh->disconnect();
		print "Database '$values->{database}' already exists on this server! Exiting...\n";
		exit;
	}

	# Connect to the 'mysql' database
	$dbh = DBI->connect(
		"DBI:mysql:mysql:$values->{host}",
		$values->{user},
		$values->{password} );
	unless ( $dbh ) {	
		print "Unable to connect to MYSQL! Exiting...\n";
		die;
	}

	my $createDBCommand = $dbh->prepare(
		qq{
			CREATE DATABASE $values->{database}
		} );
	$createDBCommand->execute();
	$createDBCommand->finish();
	$dbh->disconnect();

	# Check if database creation successful
	$dbh = DBI->connect(
		"DBI:mysql:$values->{database}:$values->{host}",
		$values->{user},
		$values->{password} );
	unless ( $dbh ) {
		print "Database creation failed! Exiting...\n";
		die;
	}
	$dbh->disconnect();

	print "Database '$values->{database}' successfully created.\n";
}

sub commandCreateTable() {
	
	print "Creating tables for database '$values->{database}'...\n";

	my $dbh;

	$dbh = DBI->connect(
		"DBI:mysql:$values->{database}:$values->{host}",
		$values->{user},
		$values->{password} );
	unless ( $dbh ) {
		print "Unable to connect to database $values->{database} on $values->{host}. Exiting...\n";
		die;
	}
	
	# Test to see if any of the tables already exist in this database
	# If so - bomb out because database integrity is invalid

	my @tableList = ( "Machine", "Project", "Build", "Run", "UnitTestCheck", "FailedUnitSubTest" );
	my $showTablesQuery = $dbh->prepare(
		qq{
			SHOW TABLES
		} );
	$showTablesQuery->execute();
	while ( my @row = $showTablesQuery->fetchrow_array() ) {
		foreach my $i ( 0 .. @tableList ) {
			if ( $row[0] eq $tableList[$i] ) {
				# A table exists!!!
				$showTablesQuery->finish();
				$dbh->disconnect();
				print "Table '$tableList[$i]' already exists in database '$values->{database}'. Please check the database integrity on MYSQL. Exiting...\n";
				die;
			}
		}
	}
	$showTablesQuery->finish();

	my @createTableSQL = (
		qq{
			CREATE TABLE Machine (
				ID int(11) unsigned NOT NULL auto_increment,
				Name char(50) NOT NULL default '',
				Hardware char(20) NOT NULL default '',
				OS char(20) NOT NULL default '',
				Kernel char(100) NOT NULL default '',
				PRIMARY KEY  (ID),
				UNIQUE KEY Name (Name,Hardware,OS,Kernel)
			) TYPE=MyISAM
		},
		qq{
			CREATE TABLE Project (
				ID int(10) unsigned NOT NULL auto_increment,
				Name char(50) NOT NULL default '',
				MailTo char(200) NOT NULL default '',
				MailFrom char(50) NOT NULL default '',
				PRIMARY KEY  (ID),
				UNIQUE KEY Name (Name)
			) TYPE=MyISAM;
		},
		qq{
			CREATE TABLE Build (
				ID int(11) unsigned NOT NULL auto_increment,
				ProjectID int(11) unsigned NOT NULL default '0',
				BuildOptions varchar(200) NOT NULL default '',
				CC varchar(20) NOT NULL default '',
				Revision int(11) NOT NULL default '0',
				MachineID int(11) unsigned NOT NULL default '0',
				Who varchar(20) NOT NULL default '',
				LocalModification tinyint(4) NOT NULL default '0',
				Date date NOT NULL default '0000-00-00',
				Time time NOT NULL default '00:00:00',
				Compiled tinyint(4) NOT NULL default '0',
				Makefile blob NOT NULL,
				Log longblob NOT NULL,
				PRIMARY KEY  (ID),
				UNIQUE KEY Project (ProjectID,BuildOptions,CC,Revision,MachineID,Who,LocalModification,Date,Time)
			) TYPE=MyISAM
		},
		qq{
			CREATE TABLE Run (
				ID int(10) unsigned NOT NULL auto_increment,
				BuildID int(10) unsigned NOT NULL default '0',
				StartDate date NOT NULL default '0000-00-00',
				StartTime time NOT NULL default '00:00:00',
				EndDate date default NULL,
				EndTime time default NULL,
				NotificationSent tinyint(4) NOT NULL default '0',
				PRIMARY KEY  (ID),
				UNIQUE KEY BuildID (BuildID,StartDate,StartTime)
				) TYPE=MyISAM
		},
		qq{ 
			CREATE TABLE UnitTestCheck (
				ID int(11) unsigned NOT NULL auto_increment,
				RunID int(11) unsigned NOT NULL default '0',
				UnitName varchar(100) NOT NULL default '',
				TestName varchar(50) NOT NULL default '',
				Passed tinyint(4) NOT NULL default '0',
				PRIMARY KEY  (ID),
				UNIQUE KEY RunID (RunID,TestName)
			) TYPE=MyISAM
		},
		qq{
			CREATE TABLE FailedUnitSubTest (
				ID bigint(20) unsigned NOT NULL auto_increment,
				UnitTestCheckID int(11) unsigned NOT NULL default '0',
				SubTest varchar(100) NOT NULL default '',
				Output longblob NOT NULL,
				PRIMARY KEY  (ID),
				UNIQUE KEY UnitTestCheck_ID (UnitTestCheckID,SubTest)
			) TYPE=MyISAM
		} );

	foreach my $i ( 0 .. @createTableSQL - 1 ) {
		my $createTableCommand =$dbh->prepare( $createTableSQL[$i] );
		$createTableCommand->execute();
		$createTableCommand->finish();
	}

	# Check if table creation was successful

	my @tables = ();
	$showTablesQuery->execute();
	while ( my @row = $showTablesQuery->fetchrow_array() ) {
		foreach my $i ( 0 .. @tableList - 1 ) {
			if ( $row[0] eq $tableList[$i] ) {
				# A table exists!!!
				push( @tables, $row[0] );
			}
		}
	}
	$showTablesQuery->finish();
	$dbh->disconnect();
	
	my $tablesCount = @tables;
	my $createTableSQLCount = @createTableSQL;

	if ( $tablesCount != $createTableSQLCount ) {
		print "Unable to create Tables. Exiting...\n";
		die;
	}
	print "Regresstor tables successfully created on database '$values->{database}'.\n";
}

sub commandCreateConfig() {
	print "Creating configuration file...\n";
	
	my $xml = new XML::Simple(
	        NoAttr=>1,
        	RootName=>'config' );
                                                                                                                                    
	my $data = $xml->XMLout( $values );
	open configOutput, ">$values->{filename}"
		or die "Unable to open file $values->{filename} for writing.\n";
	print configOutput $data;
	close configOutput;
	print "Successfully writting configurations to $values->{filename}.\n";
	print "Copy $values->{filename} to your RegresstorServices directory on your web server.\n";
}

sub commandInstall() {
	print "Begin Regresstor database installation...\n";
	&commandCreateDB();
	&commandCreateTable();
	&commandCreateConfig();
	print "Regresstor database installation complete.\n";
}


foreach my $i ( 0 .. @commands - 1 ) {
	if ( $command eq $commands[$i] ) {
		eval( $commandFunctions[$i] );
		exit;
	}
}
print "Unknown command $command.\n";

__END__

