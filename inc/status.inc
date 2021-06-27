<?php

define("STATUS_INFO",     0);
define("STATUS_NOTICE",   1);
define("STATUS_WARNING",  2);
define("STATUS_ERROR",    3);
define("STATUS_DEBUG",    4);

function status($level, $data)
{
    global $o_verbose;
    global $o_quiet;
    global $o_debug;

    switch ($level) {
        case STATUS_INFO:
            if (!$o_quiet)
                printf("%s\n", $data);
            break;
        case STATUS_NOTICE:
            if ($o_verbose && !$o_quiet)
                printf("   - %s\n", $data);
            break;
        case STATUS_WARNING:
            if ($o_verbose && !$o_quiet)
                printf("   * WARNING: %s\n", $data);
            break;
        case STATUS_ERROR:
            printf("   ! ERROR: %s\n", $data);
            break;
        case STATUS_DEBUG:
            if ($o_debug) printf("   ! DEBUG: %s\n", $data);
            break;
        default:
            printf("ERROR: Unknown status level.\n");
    }
}

?>
