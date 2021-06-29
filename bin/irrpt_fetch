#!/usr/bin/env php
<?php

$currentpath = dirname(realpath(__FILE__));
require("{$currentpath}/../conf/irrpt.conf");
require("{$currentpath}/../inc/irrquery.inc");
require("{$currentpath}/../inc/exclusions.inc");
require("{$currentpath}/../inc/aggregate.inc");
require("{$currentpath}/../inc/cvs.inc");
require("{$currentpath}/../inc/status.inc");
require("{$currentpath}/../inc/ipv6.inc");
require("{$currentpath}/../inc/utils.inc");


$irr           = new IRRQuery;   /* Open our IRRQuery class */
$cvs           = new CVS;        /* Open our CVS class */
$count         = 0;
$offset        = 0;

$o_asn         = 0; /* Default ASN to fetch: all */
$o_debug       = 0; /* Default to debug off */
$o_verbose     = 1; /* Default to verbose on */
$o_4           = 0; /* Default to fetch both v4 and v6 by setting this to 0 */
$o_6           = 0; /* Default to fetch both v4 and v6 by setting this to 0 */
$o_quiet       = 0;
$o_cvs         = 1;
$o_irrdb       = NULL;
$o_irrdbConf   = array();

function track($file)
{
    global $cvs;
    global $cfg;

    if (($rev = $cvs->update($file)) == FALSE) {
        status(STATUS_ERROR, "Error: Unable to perform CVS update, aborting.");
        exit(1);
    }

    if ($rev['old'] == "0.00")
        status(STATUS_INFO, sprintf("   - Importing %-45s version %s", $file, $rev['new']));
    else if ($rev['old'] != $rev['new'])
        status(STATUS_INFO, sprintf("   - Updating  %-45s version %s -> %s", $file, $rev['old'], $rev['new']));

    return $rev;
}

function update_email($file, $email, $asString, $object, $rev, $type)
{
    global $cvs;
    global $cfg;
    $content = "";

    if ($rev['old'] == $rev['new']) {
        return;
    }

    $headers  = "From: {$cfg['update']['from']}\n";
    $headers .= "Reply-To: {$cfg['update']['reply-to']}\n";
    $headers .= "Date: " . date("r") . "\n";

    if ($rev['old'] != "0.00") {
        $content .= "Changes for {$asString} (object {$object}) ({$type}):\n\n";
        $content .= $cvs->get_diff($file, $rev);
        $subject = $cfg['update']['subject'] . "{$asString} ({$type} Changes)";
    } else {
        $subject = $cfg['update']['subject'] . "{$asString} ({$type} Initial Import)";
    }

    $content .= "Complete list for {$asString} (object {$object}) ({$type}):\n";
    $content .= $cvs->get_complete($file);


    if (!($email == "-" || $email == "none" || !isset($email))) {
        status(STATUS_INFO, "   - Sending update notification to {$email}");
        mail($email, $subject, $content, $headers);
    }

    if (isset($cfg['update']['localcopy'])) {
        status(STATUS_INFO, "   - Sending local-copy notification to {$email}");
        mail($cfg['update']['localcopy'], $subject, $content, $headers);
    }
}


function process_v4($irr, $object, $routefile4, $aggfile4)
{
    global $cfg;

    $routes4  = array();
    $asnlist4 = array();

    $resolve_v4_result = resolve_v4($irr, $object);
    list($routes4, $asnlist4) = $resolve_v4_result;

    /* Strip out excluded routes that can not be registered */
    status(STATUS_NOTICE, "Filtering against excluded routes.");
    $routes4 = exclusions_filter($cfg['cfgfiles']['exclusions'], $routes4);

    /* Write the routes we've just looked up into a file */
    status(STATUS_NOTICE, "Writing routes to local database.");

    if (($output4 = @fopen($routefile4, "w")) == FALSE) 
    {
        status(STATUS_WARNING, "Can not open IRRDB output file for {$asString}, skipping this record. (check perms?)");
    }

    if (is_array($routes4))
    {
        for($i = 0; $i < sizeof($routes4); $i++)
        {
            fwrite($output4, sprintf("%s\n", $routes4[$i]));
        }
    }
    fclose($output4);
    update_file_permissions($routefile4, $cfg['cfgfiles']['umask']);

    /* Aggregate the route file */
    status(STATUS_NOTICE, "Aggregating v4 routes.");
    $aggregated_routes4 = aggregate_routes($routefile4);

    /* print result to a file */
    if (($afile4 = @fopen($aggfile4, "w")) == FALSE)
    {
        status(STATUS_WARNING, "Can not open agg route file, skipping this record.");
    }

    if (is_array($aggregated_routes4))
    {
        for ($i = 0; $i < sizeof($aggregated_routes4); $i++)
        {
            fwrite($afile4, "$aggregated_routes4[$i]");
        }
    }
    fclose($afile4);

    update_file_permissions($aggfile4, $cfg['cfgfiles']['umask']);

    return $asnlist4;
}

function process_v6($irr, $object, $routefile6, $aggfile6)
{
    global $cfg;

    $routes6   = array();
    $asnlist6  = array();

    $resolve_v6_result = resolve_v6($irr, $object);
    $routes6tmp        = array();
    list($routes6, $asnlist6) = $resolve_v6_result;

    /* Strip out excluded routes that can not be registered */
    status(STATUS_NOTICE, "Filtering against excluded routes.");
    $routes6 = exclusions_filter($cfg['cfgfiles']['exclusions'], $routes6);


    /* Write the routes we've just looked up into a file */
    status(STATUS_NOTICE, "Writing routes to local database.");

    /* Write routes to file */
    if ( ($output6 = @fopen($routefile6, "w")) == FALSE ) 
    {
        status(STATUS_WARNING, "Can not open IRRDB output file for {$asString}, skipping this record.");
    }

    if ( is_array($routes6) )
    {
        for($i = 0; $i < sizeof($routes6); $i++)
        {
            fwrite($output6, sprintf("%s\n", $routes6[$i]));
        }
    }
    fclose($output6);
    update_file_permissions($routefile6, $cfg['cfgfiles']['umask']);

    /* Aggregate the route file */
    status(STATUS_NOTICE, "Aggregating v6 routes.");
    $aggregated_routes6 = aggregate_routes($routefile6);

    /* print result to a file */
    if (($afile6 = @fopen($aggfile6, "w")) == FALSE)
    {
        status(STATUS_WARNING, "Can not open agg route file, skipping this record. (Check perms?)");
    }

    if (is_array($aggregated_routes6))
    {
        for ($i = 0; $i < sizeof($aggregated_routes6); $i++)
        {
            fwrite($afile6, "$aggregated_routes6[$i]");
        }
    }
    fclose($afile6);
    update_file_permissions($aggfile6, $cfg['cfgfiles']['umask']);

    return $asnlist6;
}

function process_as($asString, $asNumber, $count, $object, $irr, $email, $o_quiet, $o_4, $o_6, $o_cvs)
{
    global $cvs;
    global $cfg;

    status(STATUS_INFO, "Processing {$asString} [$object] (Record {$count})");

    /* Figure out if we have an AUT-NUM or an AS-SET, and resolve it */
    status(STATUS_NOTICE, "Parsed IRR Object {$object}");

    /* init files and ensure files are writeable by the effective user id*/
    $routefile4 = $cfg['paths']['db'] . $asNumber . ".4";
    check_file_perms($routefile4);

    $aggfile4 = $routefile4 . $cfg['aggregate']['suffix'];
    check_file_perms($aggfile4);

    $routefile6 = $cfg['paths']['db'] . $asNumber . ".6";
    check_file_perms($routefile6);

    $aggfile6 = $routefile6 . $cfg['aggregate']['suffix'];
    check_file_perms($aggfile6);

    /* call v4 / v6 processing and aggregation */
    $asnlist4 = array();
    $asnlist6 = array();

    /* Process V4 */
    if( $o_4 == 1 || ($o_4 == 0 && $o_6 == 0) )
    {
        status(STATUS_NOTICE, "Fetching v4 routes.");
        status(STATUS_DEBUG, "process_v4(\$irr, $object, $routefile4, $aggfile4);");
        $asnlist4 = process_v4($irr, $object, $routefile4, $aggfile4);
    }

    /* Process V6 */
    if( $o_6 == 1 || ($o_4 == 0 && $o_6 == 0) )
    {
        status(STATUS_NOTICE, "Fetching v6 routes.");
        $asnlist6 = process_v6($irr, $object, $routefile6, $aggfile6);
    }


    /* merge files into common v4 & v6 file */
    $routefile = $cfg['paths']['db'] . $asNumber;
    check_file_perms($routefile);
    $aggfile = $routefile . $cfg['aggregate']['suffix'];
    check_file_perms($aggfile);

    concat_files($routefile4, $routefile6, $routefile);
    concat_files($aggfile4, $aggfile6, $aggfile);

    update_file_permissions($routefile, $cfg['cfgfiles']['umask']);
    update_file_permissions($aggfile, $cfg['cfgfiles']['umask']);

    /* Log the ASNs behind this object, for future AS-PATH use */
    $asnlist = array_merge($asnlist4, $asnlist6);
    $asnfile = $routefile . $cfg['fetch']['asn_suffix'];
    status(STATUS_NOTICE, "Writing ASN list to local database.");
    if (($output = @fopen($asnfile, "w")) == FALSE) 
    {
        status(STATUS_WARNING, "Can not open ASN list output file for {$asString}, skipping this record.");
    }

    for($i = 0; $i < sizeof($asnlist); $i++)
    {
        fwrite($output, sprintf("%s\n", preg_replace("/[aA][sS]/", "", $asnlist[$i])));
    }
    fclose($output);
    update_file_permissions($asnfile, $cfg['cfgfiles']['umask']);

    /* Perform CVS tracking */
    if ($o_cvs == 1)
    {
        status(STATUS_NOTICE, "Tracking data in CVS.");
        $cvs->init($cfg['paths']['cvsroot']);
        check_file_perms($cfg['paths']['cvsroot']);

        $rev = track($routefile4);
        $rev = track($routefile6);
        $rev = track($aggfile4);
        $rev = track($aggfile6);
        $rev = track($routefile);
        $rev_a = track($aggfile);

        /* Send e-mail updates */
        status(STATUS_NOTICE, "Send update email.");
        switch ($cfg['fetch']['emailonchange']) {
            case "both":
                update_email($routefile, $email, $asString, $object, $rev, "Full");
                update_email($aggfile, $email, $asString, $object, $rev_a, "Aggregated");
                break;
            case "full":
                update_email($routefile, $email, $asString, $object, $rev, "Full");
                break;
            case "aggregate":
                update_email($aggfile, $email, $asString, $object, $rev_a, "Aggregated");
                break;
            case "none":
            case "no":
                break;
        }
    }
}

function load_irrdb($o_quiet, $o_irrdbfile)
{
    global $cfg;
    global $o_irrdbConf;
    $count    = 0;

    // Load the IRRDB file and store entries into a hash
    if (!($irrdb = @fopen($cfg['cfgfiles']['irrdb_list'], "r"))) 
    {
        status(STATUS_ERROR, "Unable to open irrdb config file, aborting.");
        exit(-1);
    }

    /* Parse the IRRDB config file */
    while( !feof($irrdb) )
    {
        $line    = rtrim(fgets($irrdb, 256));

        /* Skip comments and junk lines */
        if ((strlen($line) == 0) || ($line[0] == "#"))
            continue;

        /* Skip lines that do not start with a number */
        if ( ! preg_match("/^[\d+]/", $line) )
            continue;

        $results    = preg_split( "/[ \t]+/", $line);
        $asNumber   = $results[0];
        $object     = $results[1];

        if ( isset($results[2]) && filter_var($results[2], FILTER_VALIDATE_EMAIL) )
        {
            // EMail is optional in the configuration file
            $email    = $results[2];
        } else {
            $email    = NULL;
        }

        $asString = "AS" . $asNumber;

        $count++;

        if ( ! isset($o_irrdbConf[$asNumber]) )
        {
            $o_irrdbConf[$asNumber]['asn']      = $asNumber;
            $o_irrdbConf[$asNumber]['email']    = $email;
            $o_irrdbConf[$asNumber]['object']   = $object;
        }
    }
    fclose($irrdb); 
}

/********** PROCESSING STARTS HERE *********/


/* Set UID specified in the config file */
if (posix_geteuid() == 0) {
    if ($cfg['fetch']['set_uid']) {
        if (!($user = posix_getpwnam($cfg['fetch']['set_uid'])))
            $user = posix_getpwuid($cfg['fetch']['set_uid']);

        if (!$user) {
            printf("Unable to change to the specified UID, aborting.\n");
            exit(1);
        }

        posix_setuid($user['uid']);
        posix_seteuid($user['uid']);
    }
}

// disable cvs tracking per config file option 
if ($cfg['tools']['nocvs']) {
    $o_cvs = 0;
}


/* Parse through the cmdline options. */
for ($offset = 1; $offset < $_SERVER['argc']; $offset++) 
{
    if (substr($_SERVER['argv'][$offset], 0, 1) != "-")
        break;

    switch($_SERVER['argv'][$offset]) {
        case "-h":
        case "--help":
            printf("Usage: %s [-dh46qv] [-f file] [--nocvs] [object]\n", $_SERVER['argv'][0]);
            exit(1);
        case "-q":
        case "--quiet":
            $o_quiet = 1;
            break;
        case "-d":
        case "--debug":
            $o_debug = 1;
            break;
        case "-v":
        case "--verbose":
            $o_verbose = 1;
            break;
        case "--nocvs":
            $o_cvs = 0;
            break;
        case "-4":
        case "--4":
            $o_4 = 1;
            break;
        case "-6":
        case "--6":
            $o_6 = 1;
            break;
        case "-f":
        case "--file":
            $o_irrdb = $_SERVER['argv'][$offset+1];
            $offset++;
            break;
    }
}

/* Set Timezone */
date_default_timezone_set($cfg['global']['timezone']);

/* Set Memory Limit */
ini_set("memory_limit",$cfg['global']['memory_limit']);

/* Open the file with the list of IRR objects we will be tracking */
/* provided via command line or through the config file */
if( $o_irrdb )
{
    $cfg['cfgfiles']['irrdb_list'] = $o_irrdb;
}
load_irrdb($o_quiet,$cfg['cfgfiles']['irrdb_list']);

/* Establish a connection with our IRR Query whois server */
if ($irr->connect($cfg['fetch']['host'], $cfg['fetch']['port']) == FALSE) 
{
    status(STATUS_ERROR, "Unable to connect to IRR Query whois server " . 
           $cfg['fetch']['host'] . ", aborting.");
    exit(-1);
}

/* Optionally enable a local cache of prefixes per aut-num record */
if ($cfg['fetch']['cache']) {
    $irr->cache_set(TRUE);
}

/* If we don't want to query all IRR sources, set the new sources now */
if ($cfg['fetch']['sources'] != "all")
{
    $irr->set_sources($cfg['fetch']['sources']);
}

// check AS parameter if provided 
if( isset($_SERVER['argv'][$offset+0]) )
{
    if (preg_match("/^AS./i", $_SERVER['argv'][$offset+0]))
    {
      // MATCH: Object listed as AS12345 or AS-ALPHATEST
      $asString = strtoupper($_SERVER['argv'][$offset+0]);
      $object   = $asString;

      $asNumber = preg_replace("/[aA][sS]/", "", $asString);

      status(STATUS_DEBUG, "\$asNumber = $asNumber");
      if ( is_numeric($asNumber) == FALSE )
      {
          $asNumber = $asString;
      }

      $count++;

      // NOTE: NOT ENABLED
      // This section intentionally left out for people who wish
      // to pull routes for a single ASN, ie: AS12345 instead of 
      // having it lookup the object ID for AS 12345 which might be
      // AS-ALPHATEST
      //
      //status(STATUS_INFO, "DBG: matched /^AS./i as $asNumber.");
      //if( isset($o_irrdbConf[$asNumber]['object']) )
      //{
      //    $object    = $o_irrdbConf[$asNumber]['object'];
      //} else {
      //    $object = $asString;
      //}
      status(STATUS_DEBUG, "process_as( $asString, $asNumber, cnt: $count, str: $object, 'none', $o_quiet, $o_4, $o_6, $o_cvs)");
      process_as($asString, $asNumber, $count, $object, $irr, 'none', $o_quiet, $o_4, $o_6, $o_cvs);
    }
    elseif ( is_numeric($_SERVER['argv'][$offset+0]) &&
          (int)$_SERVER['argv'][$offset+0] > 0
          && (int)$_SERVER['argv'][$offset+0] <= 4294967295)
    {
        $asString = "AS" . $_SERVER['argv'][$offset+0];
        $asNumber = $_SERVER['argv'][$offset+0];
        $count++;

        //
        if (isset($o_irrdbConf[$asNumber]['object']))
        {
            $object = $o_irrdbConf[$asNumber]['object'];
        } else {
            $object = $asString;
        }
        status(STATUS_DEBUG, "process_as( $asString, $asNumber, cnt: $count, str: $object, 'none', $o_quiet, $o_4, $o_6, $o_cvs)");
        process_as($asString, $asNumber, $count, $object, $irr, 'none', $o_quiet, $o_4, $o_6, $o_cvs);
    }
    else
    {
        status(STATUS_ERROR, "Invalid AS or AS-SET input, aborting.");
        exit(-1);
    }
}
else {
    // NOTE: Cycle through array for ASN numbers and their objects
    status(STATUS_INFO, "Reading irrdbConf array list for ASNs, Objects, Email");
    $count = 0;

    foreach( $o_irrdbConf as $v1 )
    {
        $count++;
        foreach( $v1 as $v2 )
        {
            //print "----\n";
            //print "V1 ASN: " . $v1['asn'] . "\n";
            //print "V1 Obj: " . $v1['object'] . "\n";
            //print "V1 Ema: " . $v1['email'] . "\n";

            $asString   = "AS" . $v1['asn'];
            $asNumber   = $v1['asn'];
            $object     = $v1['object'];
            $email      = $v1['email'];
        }

        process_as( $asString, $asNumber, $count, $object, $irr, $email, $o_quiet, $o_4, $o_6, $o_cvs);
    } // END: foreach $o_irrdbConf
}

if( $o_quiet == 0 )
{
    status(STATUS_INFO, "Completed processing of {$count} IRR object(s).");
}


?>
