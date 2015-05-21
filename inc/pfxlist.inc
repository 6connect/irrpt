<?php

function pfxlist_generate($format, $asn, $pfxstr, $pfxlen)
{
	global $cfg;

	$file = $cfg['paths']['db'] . $asn . $cfg['aggregate']['suffix'];

	if (!file_exists($file) || !is_readable($file)) {
		status(STATUS_ERROR, "Cannot open IRRDB prefix list file {$file}");
		return FALSE;
	}

	if (!($pfxfile = fopen($file, "r"))) {
		status(STATUS_ERROR, "Cannot open IRRDB prefix list file {$file}");
		return FALSE;
	}

	switch ($format) {
	    case 'cisco':
		pfxlist_generate_cisco($pfxfile, $asn, $pfxstr, $pfxlen);
		break;
	    case 'juniper':
		pfxlist_generate_juniper($pfxfile, $asn, $pfxstr, $pfxlen);
		break;
	    case 'extreme':
		pfxlist_generate_extreme($pfxfile, $asn, $pfxstr, $pfxlen);
		break;
	    case 'force10':
		pfxlist_generate_force10($pfxfile, $asn, $pfxstr, $pfxlen);
		break;
	    default:
		break;
	}

	fclose($pfxfile);
}

function pfxlist_generate_juniper($pfxfile, $asn, $pfxstr, $pfxlen)
{
	$pfxname = sprintf($pfxstr, $asn);
	global $cfg;

	printf("policy-options {\n");
	printf("    replace: policy-statement %s {\n", $pfxname);
	printf("        term prefixes {\n");
	printf("            from {\n");

	while (!feof($pfxfile)) {
		if (!($line = rtrim(fgets($pfxfile, 64))))
			continue;

		printf("                route-filter %s upto /%d;\n", 
			$line, $pfxlen);
	}

	printf("            }\n");
	printf("            then next policy;\n");
	printf("        }\n");
	printf("        then reject;\n");
	printf("    }\n");
	printf("}\n");

	return 0;
}

function pfxlist_generate_cisco($pfxfile, $asn, $pfxstr, $pfxlen)
{
	$pfxname = sprintf($pfxstr, $asn);
	global $cfg;

	printf("conf t\n");
	printf("no ip prefix-list %s\n", $pfxname);

	while (!feof($pfxfile)) {
		if (!($line = rtrim(fgets($pfxfile, 64))))
			continue;

		$prefix = explode("/", $line);

	        printf("ip prefix-list %s permit %s/%d", $pfxname, $prefix[0],
			$prefix[1]);

		if ($prefix[1] < $pfxlen)
			printf(" le %s\n", $pfxlen);
		else    
			printf("\n");
	}

	printf("end\n");
	printf("write mem\n");

	return 0;
}

function pfxlist_generate_extreme($pfxfile, $asn, $pfxstr, $pfxlen)
{
	/* ExtremeWare can not handle : names, replace with - */
	$pfxname = str_replace(":","-", sprintf($pfxstr, $asn));
	global $cfg;

	printf("delete access-profile %s\n", $pfxname);
	printf("create access-profile %s type ipaddress\n", $pfxname);

	$counter = 10;

	while (!feof($pfxfile)) {
		if (!($line = rtrim(fgets($pfxfile, 64))))
			continue;

		$prefix = explode("/", $line);

		printf("configure access-profile %s add %d permit ipaddress %s/%d", $pfxname, $counter += 10, $prefix[0], $prefix[1]);

		if ($prefix[1] < $pfxlen)
			printf("\n", $pfxlen);
		else
			printf(" exact\n");
	}

	printf("save\n");

	return 0;
}

function pfxlist_generate_force10($pfxfile, $asn, $pfxstr, $pfxlen)
{
	$pfxname = sprintf($pfxstr, $asn);
	global $cfg;

	printf("conf t\n");
	printf("no ip prefix-list %s\n", $pfxname);
	printf("ip prefix-list %s\n", $pfxname);

	while (!feof($pfxfile)) {
		if (!($line = rtrim(fgets($pfxfile, 64))))
			continue;

		$prefix = explode("/", $line);

	        printf("%s permit %s/%d", $pfxname, $prefix[0],
			$prefix[1]);

		if ($prefix[1] < $pfxlen)
			printf(" le %s\n", $pfxlen);
		else    
			printf("\n");
	}

	printf("end\n");
	printf("write mem\n");

	return 0;
}


?>
