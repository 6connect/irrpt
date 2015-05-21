<?php

define("STATUS_INFO",		0);
define("STATUS_NOTICE",		1);
define("STATUS_WARNING",	2);
define("STATUS_ERROR",		3);

function status($level, $data)
{
	global $o_verbose;

	switch ($level) {
	    case STATUS_INFO:
		printf("%s\n", $data);
		break;
	    case STATUS_NOTICE:
		if ($o_verbose)
			printf("   - %s\n", $data);
		break;
	    case STATUS_WARNING:
		if ($o_verbose)
			printf("   * WARNING: %s\n", $data);
		break;
	    case STATUS_ERROR:
		printf("   ! ERROR: %s\n", $data);
		break;
	    default:
		printf("ERROR: Unknown status level.\n");
	}
}

?>
