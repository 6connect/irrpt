<?php

/* v6 helper functions */

/* mostly inspired by @mikemackintosh */
/* https:\/\/www.mikemackintosh.com\/5-tips-for-working-with-ipv6-in-php */

/* Create a v6 netmask from a subnet integer value */
function _v6_mask_int2hex($int) 
{
  $hosts = (128 - $int);
  $networks = 128 - $hosts;

  $_m = str_repeat("1", $networks).str_repeat("0", $hosts);

  $_hexMask = null;
  foreach( str_split( $_m, 4) as $segment){
    $_hexMask .= base_convert( $segment, 2, 16);
  }

  $mask = substr(preg_replace("/([A-f0-9]{4})/", "$1:", $_hexMask), 0, -1);
  return $mask;
}

/* Stupid check for v4 vs v6 */
function _ip_version($txt) 
{
  return strpos($txt, ":") === false ? 4 : 6;
}

/* expand v6 notation with all it's zeros */
function _v6_expand($ip) 
{
    $hex = unpack("H*hex", inet_pton($ip));         
    $ip = substr(preg_replace("/([A-f0-9]{4})/", "$1:", $hex['hex']), 0, -1);
    return $ip;
}

/* return network portion of the prefix */
function _v6_to_network($pfx, $mask) 
{
  /* convert v6 address into bin form */
#  print "$pfx\n";
  $pfx_bin= inet_pton( $pfx );
  /* do the same with the provided mask */
  $mask_bin = inet_pton( _v6_mask_int2hex($mask));
#  print "$mask_bin\n";
  /* apply mask to prefix and reverse the binary conversion to a printable string to get the network portion*/
  $network = inet_ntop( pack( "A16", $pfx_bin & $mask_bin ) );
 # print "$network\n\n";
  return _v6_expand($network);
}

?>
