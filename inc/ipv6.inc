<?php

/* v6 helper functions */

/* Stupid check for v4 vs v6 */
function _ip_version($txt) 
{
    return strpos($txt, ":") === false ? 4 : 6;
}

/* Create a v6 netmask from a subnet integer value */
function _v6_mask_int2hex($int) 
{
    $hosts    = (128 - $int);
    $networks = (128 - $hosts);

    $_m = str_repeat("1", $networks).str_repeat("0", $hosts);

    $_hexMask = null;
    foreach( str_split( $_m, 4) as $segment) {
        $_hexMask .= base_convert( $segment, 2, 16);
    }

    $mask = substr(preg_replace("/([A-f0-9]{4})/", "$1:", $_hexMask), 0, -1);
    return $mask;
}

/* expand v6 notation with all its zeros - 2602:fde5:: => 2602:fde5:0000:0000:0000:0000:0000:0000 */
function _v6_expand($ip) 
{
    $hex = unpack("H*hex", inet_pton($ip));         
    $ip  = substr(preg_replace("/([A-f0-9]{4})/", "$1:", $hex['hex']), 0, -1);
    return $ip;
}

/* compress v6 notation and all its zeros - 2602:fde5:0000:0000:0000:0000:0000:0000 => 2602:fde5:: */
function _v6_compress($ip) 
{
    $ip = inet_ntop(inet_pton($ip));
    return $ip;
}

/* return network portion of the prefix - 2602:fde5:1::90A/36 => 2602:fde5:0000:0000:0000:0000:0000:0000 */
function _v6_to_network($pfx, $mask) 
{
    /* convert v6 address into bin form */
    $pfx_bin  = inet_pton( $pfx );

    /* do the same with the provided mask */
    $mask_bin = inet_pton( _v6_mask_int2hex($mask));

    /* apply mask to prefix and reverse the binary conversion to a printable string to get the network portion*/
    $network  = inet_ntop( pack( "A16", $pfx_bin & $mask_bin ) );

    return _v6_expand($network);
}

?>
