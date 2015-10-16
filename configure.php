#!/usr/bin/env php
<?php
error_reporting(E_ALL);
ini_set('display_errors','On');

$php_version_req = "5.4";
$php_cli = $output = $newContent = null;
$found_cvs_path = FALSE;

//Check that php is working via the system path...this would seem to be inferred, but you'd be surprised...
printf("Checking PHP...\n");
$php_cli = exec("which php");
exec($php_cli . " -v", $v_output);

if ((strpos($v_output[0], "PHP ") !== false) and (strpos($v_output[1], "Copyright ") !== false)) {
	printf("PHP path $php_cli verified.\n");
}
else {
	printf("ERROR: PHP path $php_cli test failed: $v_output\n");
	exit(1);
}

//Check that the php version is greater than or equal to minimum required version
printf("Checking PHP version...\n");
$php_version_text = phpversion();
$php_v = explode('.', $php_version_text);
$php_version = $php_v[0] . "." . $php_v[1];

if($php_version >= $php_version_req) {
	printf("PHP version $php_version meets minimum requirement of $php_version_req.\n");

}
else {
	printf("ERROR: PHP version $php_version does not meet minimum requirement of $php_version_req.\n  You must upgrade PHP to run irrpt.\n");
	exit(1);
}

printf("Checking CVS...\n");
$cvs_path = exec("which cvs");

if(preg_match('/cvs/i', $cvs_path)) {
	printf("CVS is installed $cvs_path.\n");
}
else {
	printf("ERROR: CVS is not installed.\n  You must install CVS to run irrpt.\n");
	exit(1);
}

printf("Checking installed path...\n");
$i_path = exec("pwd");
printf("Installed path: $i_path\n");
$fh = @fopen("$i_path/conf/irrpt.conf", "r");
if($fh) {
	while (($line = fgets($fh)) !== false) {
		if(preg_match('/^\$cfg\[\'paths\'\]\[\'base\'\]/', $line)) {
			printf("Updating installed path in irrpt.conf...\n");
			$line = '$cfg[\'paths\'][\'base\']           ' . "= \"$i_path/\";\n"; 
		}
		if(preg_match('/^\$cfg\[\'tools\'\]\[\'cvs\'\]/', $line)) {
			printf("Updating cvs path in irrpt.conf...\n");
			$line = '$cfg[\'tools\'][\'cvs\']           ' . "= \"$cvs_path\";\n"; 
		}
		$newContent .= $line;
	}

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
	fclose($irrpt_conf);
}
else {
	printf("ERROR: Could not write $i_path/conf/irrpt.conf.  Please ensure the file is writeable by the current user.\n");
}
printf("Checking cvs directory...\n");
if ( !file_exists("$i_path/db") ) {
	//cvs -d /home/irrpt/db/CVS init
	printf("WARNING: Path to cvs files is missing.  Would you like to restore the CVS directory to it's default location of $i_path/db?\n");
	$stdin = fopen('php://stdin', 'r');
	$yes = $no = FALSE;

	while (!$yes && !$no) {
		echo '(y/n)? ';
		$input = trim(fgets($stdin));

		if ($input == 'y') {
			printf("CVS root will be restored $i_path/db.\n");
			$restore_res = exec("cvs -d $i_path/db/ init");
			echo "$restore_res\n";	
			break;
		}
	
		if ($input == 'n') {
			printf("CVS root will not be restored.  Run 'cvs -d /path/to/cvs init to restore manually.  CVS archiving of files may not work.\n");
			break;
		}

	}
}
else {
	printf("CVS directory located at $i_path/db.\n");
}

printf("\nirrpt configuration verified.  Please edit ./conf/irrdb.conf to add the ASNs and objects you wish to track.\n");

?>
