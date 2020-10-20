<?php

function objgrp_generate($format, $asn, $grpstr, $grpstr_v6, $o_4, $o_6)
{
	global $cfg;

    if(!check_asn_arg($asn, $asn_number, $asn_string)) {
        status(STATUS_ERROR, "Invalid ASN {$asn}");
        return FALSE;
    }

	// only v4
	if( $o_4 == 1 && $o_6 == 0 )
	{
		$file = $cfg['paths']['db'] . $asn_number . '.4' . $cfg['aggregate']['suffix'];
	}
	// only v6
	elseif( $o_6 == 1 && $o_4 == 0 )
	{
		$file = $cfg['paths']['db'] . $asn_number . '.6' . $cfg['aggregate']['suffix'];
	}
	// v4 and v6
	else
	{
		$file = $cfg['paths']['db'] . $asn_number . $cfg['aggregate']['suffix'];
	}

	if (!file_exists($file) || !is_readable($file)) {
		status(STATUS_ERROR, "Cannot open IRRDB prefix list file {$file}");
		return FALSE;
	}

	if (!($pfxfile = fopen($file, "r"))) {
		status(STATUS_ERROR, "Cannot open IRRDB prefix list file {$file}");
		return FALSE;
	}

	switch ($format) {
	    case 'ciscoxr':
		objgrp_generate_iosxr($pfxfile, $asn_number, $grpstr, $grpstr_v6, $o_4, $o_6);
		break;
	    default:
		break;
	}

	fclose($pfxfile);
}

function objgrp_generate_iosxr($pfxfile, $asn, $grpstr, $grpstr_v6, $o_4, $o_6)
{
	$pfxname = sprintf($grpstr, $asn);
	$pfxname_v6 = sprintf($grpstr_v6, $asn);
	$pfx1st = 'Y';
	$pfx1st_v6 = 'Y';
	global $cfg;

	printf("conf\n");
  if( $o_4 == 1 && $o_6 == 0 )
  {
    printf("no object-group network ipv4 %s\n", $pfxname);
  }
  elseif( $o_6 == 1 && $o_4 == 0 )
  {
    printf("no object-group network ipv6 %s\n", $pfxname_v6);
  }
  else
  {
    printf("no object-group network ipv4 %s\n", $pfxname);
    printf("no object-group network ipv6 %s\n", $pfxname_v6);
  }

	while (!feof($pfxfile)) {
		if (!($line = rtrim(fgets($pfxfile, 64))))
			continue;

		$prefix = explode("/", rtrim($line));
		if (preg_match('/:/', $prefix[0]) && $pfx1st == 'N')
		{
			printf("exit\n");
			printf("object-group network ipv6 %s\n", $pfxname_v6);
			$pfx1st_v6 = 'N';
		}
		elseif ($pfx1st_v6 == 'Y' && preg_match('/:/', $prefix[0]))
		{
			printf("object-group network ipv6 %s\n", $pfxname_v6);
			$pfx1st_v6 = 'N';
		}
		elseif ($pfx1st == 'Y')
		{
			$pfx1st = 'N';
    			printf("object-group network ipv4 %s\n", $pfxname);
		}
		printf("  %s/%d\n", $prefix[0], $prefix[1]);
	}

	if ($pfx1st == 'N' || $pfx1st_v6 == 'N')
	{
		printf("exit\n");
	}
	printf("commit\n");
	printf("exit\n");

	return 0;
}

?>
