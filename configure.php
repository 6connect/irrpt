#!/usr/bin/env php
<?php
error_reporting(E_ALL);
ini_set('display_errors','On');

$php_version_req = "5.4";
$php_cli = $output = $newContent = null;
$found_cvs_path = FALSE;

printf("IRRPT configuration script.\n");

//Check that php is working via the system path...this would seem to be inferred, but you'd be surprised...
log_setup("Checking PHP ...");
$php_cli = exec("which php");
exec($php_cli . " -v", $v_output);

if ((strpos($v_output[0], "PHP ") !== false) and (strpos($v_output[1], "Copyright ") !== false)) {
	printf("$php_cli verified.\n");
} else {
    printf("ERROR: PHP path $php_cli test failed: $v_output\n");
    exit(1);
}

// ---
// Check that the php version is greater than or equal to minimum
// required version

log_setup("Checking PHP Version...");
if( version_compare(phpversion(), $php_version_req, '<') )
{
	printf("ERROR: PHP version " . phpversion() . " does not meet minimum");
	printf(" requirement of $php_version_req.\n\n");
	printf("       You must upgrade PHP to run irrpt.\n");
	exit(1);
} 
else {
	printf(phpversion() . " meets minimum requirement of $php_version_req.\n");
}

log_setup("Checking CVS...");
$cvs_path = exec("which cvs");

if(preg_match('/cvs/i', $cvs_path)) {
	printf($cvs_path . "\n");
}
else {
	printf("ERROR: CVS is not installed.\n\n");
	printf("You must install CVS to run irrpt.\n");
	exit(1);
}

// ---
// Check to ensure we have the right path information in the files

log_setup("Checking installed path...");
$i_path = __DIR__;
printf($i_path . "\n");

$fh = @fopen("$i_path/conf/irrpt.conf", "r");
if($fh) {
	log_setup("Updating irrpt.conf...");
	while( ($line = fgets($fh)) !== false ) {
		if( preg_match('/^\$cfg\[\'paths\'\]\[\'base\'\]/', $line) ) {
			printf("installed path changed. ");
            $line = '$cfg[\'paths\'][\'base\']           ' . "= \"$i_path/\";\n";
        }
		if( preg_match('/^\$cfg\[\'tools\'\]\[\'cvs\'\]/', $line) )
		{
			printf("cvs path changed.");
            $line = '$cfg[\'tools\'][\'cvs\']           ' . "= \"$cvs_path\";\n";
        }
        $newContent .= $line;
    }
	printf("\n");
    fclose($fh);
}
else {
	printf("ERROR: Unable to locate or read $i_path/conf/irrpt.conf.  Configuration aborted.\n");
	exit(1);
}

if(empty($newContent)) {
	printf("ERROR: Unable to to find any content in $i_path/conf/irrpt.conf.  Configuration aborted.\n");
	exit(1);
}

$irrpt_conf = @fopen("$i_path/conf/irrpt.conf", "w");
if($irrpt_conf) {
	fwrite($irrpt_conf, $newContent);
}
else {
    fclose($irrpt_conf);
	printf("ERROR: Could not write $i_path/conf/irrpt.conf.\n\n");
	printf("Please ensure the file is writeable by the current user.\n");
}

// ---
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
			echo (!empty($restore_res)) ? "CVS init...$restore_res" : FALSE;	
			$a_res = exec("mkdir $i_path/db/CVS");
			$a_res .= exec("echo 'D/CVSROOT////' > $i_path/db/CVS/Entries");
			$a_res .= exec("echo '.' > $i_path/db/CVS/Repository");
			echo (!empty($a_res)) ? "CVS init...$a_res" : FALSE;	
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

printf("\nirrpt configuration verified.  Please edit ./conf/irrdb.conf to add the ASNs and objects you wish to track.\n");

function log_setup($text) {
   print sprintf("%-30s", $text);
}

?>
