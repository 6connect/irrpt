#!/usr/bin/perl

use JUNOS::Device;
use JUNOS::Trace;
use Getopt::Std;
use Term::ReadKey;

# Define the constants used in this example
use constant REPORT_SUCCESS => 1;
use constant REPORT_FAILURE => 0;
use constant STATE_CONNECTED => 1;
use constant STATE_LOCKED => 2;
use constant STATE_CONFIG_LOADED => 3;

use constant VALID_ACCESSES => "telnet|ssh|clear-text|ssl";
use constant VALID_ACTIONS => "merge|replace|override";

my $load_action = "merge";
my $login = "";
my $password = "";

# print the usage of this script
sub output_usage
{
    my $usage = "Usage: $0 [options] <target> <request>

  Where:

  <target>   The hostname of the target router.
  <request>  name of a specific file containing the configuration

  Options:

    -l <login>    A login name accepted by the target router.
    -p <password> The password for the login name.
    -m <access>	  Access method, can be clear-text, ssl, ssh or telnet.
    -a            Load action, can be 'merge', 'replace' or 'override'.
                  The default is 'merge'.
    -d            turn on debug, full blast.\n\n";

    die $usage;
}

# grace_shutdown
# To gracefully shutdown.  Recognized 3 states:  1 connected, 2 locked, 
# 3 config_loaded
# Put eval around each step to make sure the next step is performed no
# matter what.
sub graceful_shutdown
{
    my ($jnx, $req, $state, $success) = @_;

    if ($state >= STATE_CONFIG_LOADED) {
        print "Rolling back configuration ...\n";
	eval {
            $jnx->load_configuration(rollback => 0);
	};
    }

    if ($state >= STATE_LOCKED) {
        print "Unlocking configuration database ...\n";
	eval {
            $jnx->unlock_configuration();
	};
    }

    if ($state >= STATE_CONNECTED) {
        print "Disconnecting from the router ...\n";
	eval {
	    $jnx->request_end_session();
            $jnx->disconnect();
	}
    }

    if ($success) {
        die "REQUEST $req SUCCEEDED\n";
    } else {
        die "REQUEST $req FAILED\n";
    }
}

sub read_file
{
    my $input_file = shift;
    my $input_string = "";
    my %escape_symbols = (
                qq(")           => '&quot;',
                qq(>)           => '&gt;',
                qq(<)           => '&lt;',
                qq(')           => '&apos;',
                qq(&)           => '&amp;'
    );
    my $char_class = join ("|", map { "($_)" } keys %escape_symbols);

    open(FH, $input_file) or return undef;

    while(<FH>) {
	my $line = $_;
        $line =~ s/<configuration-text>//g;
        $line =~ s/<\/configuration-text>//g;
	$line =~ s/($char_class)/$escape_symbols{$1}/ge;
	$input_string .= $line;
    }

    return "<configuration-text>$input_string</configuration-text>";
}




# Set AUTOFLUSH to true
$| = 1;

# Check arguments
my %opt;
getopts('l:p:dm:hta:', \%opt) || output_usage();
output_usage() if $opt{h};

# Check whether trace should be turned on
JUNOS::Trace::init(1) if $opt{d};

$load_action = $opt{a} if $opt{a};
output_usage() unless (VALID_ACTIONS =~ /$load_action/);

# Retrieve command line arguments
my $hostname = shift || output_usage();
my $cfgfile = shift || output_usage();

# Retrieve the access method, can only be telnet or ssh.
my $access = $opt{m} || "telnet";
output_usage() unless (VALID_ACCESSES =~ /$access/);

# Check whether login name has been entered.  Otherwise prompt for it
if ($opt{l}) {
    $login = $opt{l};
} else {
    print STDERR "login: ";
    $login = ReadLine 0;
    chomp $login;
}

# Check whether password has been entered.  Otherwise prompt for it
if ($opt{p}) {
    $password = $opt{p};
} else {
    print STDERR "password: ";
    ReadMode 'noecho';
    $password = ReadLine 0;
    chomp $password;
    ReadMode 'normal';
    print STDERR "\n";
}

my %deviceinfo = (
        access => $access,
        login => $login,
        password => $password,
        hostname => $hostname,
);

# Initialize the XML Parser
my $parser = new XML::DOM::Parser;

# connect TO the JUNOScript server
my $jnx = new JUNOS::Device(%deviceinfo);
unless ( ref $jnx ) {
    die "ERROR: $deviceinfo{hostname}: failed to connect.\n";
}

# Lock the configuration database before making any changes
print "Locking configuration database ...\n";
my $res = $jnx->lock_configuration();
my $err = $res->getFirstError();
if ($err) {
    print "ERROR: $deviceinfo{hostname}: failed to lock configuration.  Reason: $err->{message}.\n";
    graceful_shutdown($jnx, $cfgfile, STATE_CONNECTED, REPORT_FAILURE);
}

# Load the configuration
print "Loading configuration from $cfgfile ...\n";
if (! -f $cfgfile) {
    print "ERROR: Cannot load configuration in $cfgfile\n";
    graceful_shutdown($jnx, $cfgfile, STATE_LOCKED, REPORT_FAILURE);
}

my $cfg = $parser->parsestring(read_file($cfgfile));

unless ( ref $cfg ) {
    print "ERROR: Cannot parse $cfgfile\n";
    graceful_shutdown($jnx, $xmlfile, STATE_LOCKED, REPORT_FAILURE);
}

#
# Put the load_configuration in an eval block to make sure if the rpc-reply
# has any parsing errors, the grace_shutdown will still take place.  Do
# not leave the database in an exclusive lock state.
#
eval {
    $res = $jnx->load_configuration(
	    format => "text", 
	    action => $load_action,
	    configuration => $cfg);
};

if ($@) {
    print "ERROR: Failed to load the configuration from $xmlfile.   Reason: $@\n";
    graceful_shutdown($jnx, $xmlfile, STATE_CONFIG_LOADED, REPORT_FAILURE);
    exit(1);
} 

unless ( ref $res ) {
    print "ERROR: Failed to load the configuration from $xmlfile\n";
    graceful_shutdown($jnx, $xmlfile, STATE_LOCKED, REPORT_FAILURE);
}

$err = $res->getFirstError();
if ($err) {
    print "ERROR: Failed to load the configuration.  Reason: $err->{message}\n";
    graceful_shutdown($jnx, $xmlfile, STATE_CONFIG_LOADED, REPORT_FAILURE);
}

# Commit the change
print "Commiting configuration from $cfgfile ...\n";
$res = $jnx->commit_configuration();
$err = $res->getFirstError();
if ($err) {
    print "ERROR: Failed to commit configuration.  Reason: $err->{message}.\n";
    graceful_shutdown($jnx, $cfgfile, STATE_CONFIG_LOADED, REPORT_FAILURE);
}

# Cleanup
graceful_shutdown($jnx, $cfgfile, STATE_LOCKED, REPORT_SUCCESS);
