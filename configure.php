#!/usr/bin/env php
<?php

error_reporting(E_ALL);
ini_set('display_errors','On');

$php_version_req    = "7.2.0";
$php_cli            = $output = $newContent = null;
$found_cvs_path     = FALSE;
$i_path             = __DIR__;

printf("IRRPT configuration script.\n");

// -------------------------------------------------------------------------------------
// CHECK: PHP valid on system?

log_setup("Checking PHP ...");
if ( php_sapi_name() === "cli" )
{
    printf(PHP_BINARY . " verified.\n");
} else {
    printf("ERROR: PHP path test failed or not running from CLI: $v_output\n");
    exit(1);
}

// -------------------------------------------------------------------------------------
// CHECK: minimum PHP version needs to be 7.2

log_setup("Checking PHP Version...");
if ( version_compare(phpversion(), $php_version_req, '<') )
{
    printf("ERROR: PHP version " . phpversion() . " does not meet minimum");
    printf(" requirement of $php_version_req.\n\n");
    printf("       You must upgrade PHP to run irrpt.\n");
    exit(1);
} 
else {
    printf("OK " . phpversion() . " meets minimum requirement of $php_version_req.\n");
}

// -------------------------------------------------------------------------------------
// CHECK: CVS

log_setup("Checking CVS...");
$cvs_path = exec("which cvs");

if (preg_match('/cvs/i', $cvs_path)) {
    printf($cvs_path . "\n");
    log_setup("CVS commit support disabled by default now\n");
    $cvs_notsupported = "TRUE"; // Default TRUE
} else {
    log_setup("No CVS detected, disabling CVS support\n");
    $cvs_notsupported = "TRUE";
}


// ------------------------------------------------------------
// Check the CVS directory
log_setup("Checking CVS directory...");
if ( !file_exists("$i_path/db") ) {
    printf("WARNING: Path to cvs files is missing.\n");  
    printf("Restore the CVS directory to it's default location of $i_path/db?\n");

    $stdin = fopen('php://stdin', 'r');
    $yes = $no = FALSE;

    while (!$yes && !$no) {
        echo '(y/n)? ';
        $input = trim(fgets($stdin));

        if ($input == 'y') {
            printf("CVS root will be restored $i_path/db.\n");
            $restore_res = exec("cvs -d $i_path/db/ init");
            echo (!empty($restore_res)) ? "CVS init...$restore_res" : NULL;    

            if (!mkdir("$i_path/db/CVS")) {
                printf("ERROR: CVS restore aborted, could not create $i_path/db/CVS.\n\n");
                exit(1);

            }

            if (file_put_contents("$i_path/db/CVS/Entries", 'D/CVSROOT////') === FALSE) {
                printf("ERROR: CVS restore aborted, could not write $i_path/db/CVS/Entries.\n\n");
                exit(1);
            }
            if (file_put_contents("$i_path/db/CVS/Repository", '.') === FALSE) {
                printf("ERROR: CVS restore aborted, could not write $i_path/db/CVS/Repository.\n\n");
                exit(1);
            }
            break;
        }
    
        if ($input == 'n') {
            printf("CVS root will not be restored.  Run 'cvs -d /path/to/cvs init to restore ");
            printf("manually.  CVS archiving of files may not work.\n");
            break;
        }

    }
}
else {
    printf("$i_path/db.\n");
}

// --------------------------------------------------------------------------
// CHECK: ensure we have the right path information in the files

log_setup("Checking installed path ...");
printf($i_path . "\n");

$fh = FALSE;
if( is_readable("$i_path/conf/irrpt.conf") === TRUE )
{
    $fh = @fopen("$i_path/conf/irrpt.conf", "r");
} elseif ( is_readable("$i_path/conf/irrpt.conf.dist") === TRUE ) {
    $fh = @fopen("$i_path/conf/irrpt.conf.dist", "r");
}


if ( $fh )
{
    while( ($line = fgets($fh)) !== false ) {
        if ( preg_match('/^\$cfg\[\'paths\'\]\[\'base\'\]/', $line) )
        {
            printf("  -> installed path changed.\n");
            $line = '$cfg[\'paths\'][\'base\']          ' . "= \"$i_path/\";\n";
        }

        if ( preg_match('/^\$cfg\[\'tools\'\]\[\'cvs\'\]/', $line) )
        {
            printf("  -> cvs path changed.\n");
            $line = '$cfg[\'tools\'][\'cvs\']           ' . "= \"$cvs_path\";\n";
        }

        if ( preg_match('/^\$cfg\[\'tools\'\]\[\'nocvs\'\]/', $line) )
        {
            printf("  -> CVS option changed.\n");
            $line = '$cfg[\'tools\'][\'nocvs\']         ' . "= $cvs_notsupported;\n";
        }

        $newContent .= $line;
    }
    printf("\n");
    fclose($fh);
} else {
    printf("ERROR: Unable to locate or read $i_path/conf/irrpt.conf.  Configuration aborted.\n");
    exit(1);
}

if (empty($newContent)) {
    printf("ERROR: Unable to to find any content in $i_path/conf/irrpt.conf.  Configuration aborted.\n");
    exit(1);
}

$irrpt_conf = @fopen("$i_path/conf/irrpt.conf", "w");
if ($irrpt_conf) {
    fwrite($irrpt_conf, $newContent);
}
else {
    fclose($irrpt_conf);
    printf("ERROR: Could not write $i_path/conf/irrpt.conf.\n\n");
    printf("Please ensure the file is writeable by the current user.\n");
    exit(1);
}

log_setup("irrpt configuration verified.\n");
log_setup(" Please edit ./conf/irrpt.conf to set more options\n");
log_setup(" Please edit ./conf/irrdb.conf to add the ASNs and objects you wish to track.\n");

// ----
function log_setup($text) {
   print sprintf("%-30s", $text);
}

?>
