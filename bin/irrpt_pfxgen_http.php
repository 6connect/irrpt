<?php
// SGR 4.12.2019
header("Content-Type: text/plain");
$currentpath = dirname(realpath(__FILE__));
require("{$currentpath}/../conf/irrpt.conf");
require("{$currentpath}/../inc/pfxlist.inc");
require("{$currentpath}/../inc/ipv6.inc");
require("{$currentpath}/../inc/status.inc");
require("{$currentpath}/../inc/utils.inc");

/* Load our defaults from the master config file */
$o_pfxstr	    = $cfg['pfxgen']['default_pfxstr'];
$o_pfxstr_v6  = $cfg['pfxgen']['default_pfxstr_v6'];
$o_pfxlen	    = $cfg['pfxgen']['default_pfxlen'];
$o_pfxlen_v6	= $cfg['pfxgen']['default_pfxlen_v6'];
$o_format	    = $cfg['pfxgen']['default_format'];

$o_4 = 0; /* Default to fetch both v4 and v6 by setting this to 0 */
$o_6 = 0; /* Default to fetch both v4 and v6 by setting this to 0 */

// parse the arg path
$arg = explode("/", $_SERVER['QUERY_STRING']);

// URI /<as_int> with default Format
if (count($arg) == 1 & is_numeric($arg[0])) {
	$asn = $arg[0];
}
// URI /<os>/<as_int>
elseif (count($arg) == 2 & is_string($arg[0]) &  is_numeric($arg[1])) {
	$o_format = $arg[0];
	$asn = $arg[1];
}
else {
	print("! URL error /<os>/<as_int> or /<as_int>");
	exit(1);
}

print("! Generate for ". $o_format ." for ASN". $asn."\n");

if (pfxlist_generate($o_format, $asn, $o_pfxstr, $o_pfxstr_v6, $o_pfxlen, $o_pfxlen_v6, $o_4, $o_6) < 0) {
	printf("! Error generating prefix-list, aborting.\n");
	exit(1);
}

 ?>
