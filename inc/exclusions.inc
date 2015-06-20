<?

$exclusions = Array();


/* Helper functions */

/* Create a v6 netmask from a subnet integer value */
function _netmask_v6($int) {
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
function _ip_version($txt) {
  return strpos($txt, ":") === false ? 4 : 6;
}


/* Main exclusion function */

/* AKA: Thou shalt not route here */
function exclusions_filter($file, $routes)
{
  global $exclusions;

  if (!$routes) {
    status(STATUS_WARNING, "No routes processed in query.");
    return FALSE;
  }

  if (!$exclusions) {
    /* Open the file with the list of excluded routes */
    if (!($excfg = fopen($file, "r"))) {
      status(STATUS_ERROR, "Unable to open the exclusions config file, aborting.");
      exit(-1);
    }

    for($i=0;!feof($excfg);) {
      $line = rtrim(fgets($excfg, 256));

      /* Skip junk lines and comments */
      if ((strlen($line) < 1) || ($line[0] == '#'))
        continue;

      $exclusions[$i++] = explode("/", $line);
    }

    fclose($excfg);
  }

  if ($exclusions == FALSE)
    return $routes; /* Nothing in the exclusions file */

  /* RAS: This is pretty darn nasty, but it gets the job done */
  /* EJ: ...holy crap what did I get myself into... */

  for($i=0;$i<sizeof($routes);$i++) {
    $pfx_a = explode("/", $routes[$i]);


    for($j=0;$j<sizeof($exclusions);$j++) {
      $pfx_b = $exclusions[$j];


      /* Both prefixes need to be the same type, checking for v4 here... */
      if (_ip_version($pfx_a[0]) == '4' and _ip_version($pfx_b[0]) == '4')
      {
        /* Mask both routes and look for overlaps */
        $a = ip2long($pfx_a[0]) & (0xFFFFFFFF << (32 - $pfx_b[1]));
        $b = ip2long($pfx_b[0]) & (0xFFFFFFFF << (32 - $pfx_a[1]));

        if ($a == $b) {
          status(STATUS_WARNING, "Excluded Route {$pfx_a[0]}/{$pfx_a[1]}");
          unset($routes[$i]);
        }
      }

      /* Both prefixes need to be the same type, checking for v6 here... */
      elseif (_ip_version($pfx_a[0]) == '6' and _ip_version($pfx_b[0]) == '6')
      {
        /* Now let's do this for v6... ugh */
        /* Ok, after some noodling and inspiration from @mikemackintosh */
        /* https://www.mikemackintosh.com/5-tips-for-working-with-ipv6-in-php/ */
        /* this may be a way to do it... */

        /* First we will convert both v6 addresses into bin form */
        $a = current( unpack( "A16", inet_pton( $pfx_a[0] ) ) );
        $b = current( unpack( "A16", inet_pton( $pfx_b[0] ) ) );
        /* We also convert the exploded netmask for the prefix that will be excluded (b) */
        $mask = current( unpack( "A16", inet_pton( _netmask_v6($pfx_b[1]) ) ) );
        /* That's the netmask we are interested in, this and no smaller prefixes are used */

        /* apply the bitmask and reverse the binary conversion to a printable string */
        /* Which gives us the network portion of the two v6 addresses */
        $a = inet_ntop( pack( "A16", $a & $mask ) );
        $b = inet_ntop( pack( "A16", $b & $mask) );

        /* If both of those are identical, we exclude the prefix */
        if ($a == $b) {
          status(STATUS_WARNING, "Excluded Route {$pfx_a[0]}/{$pfx_a[1]}");
          unset($routes[$i]);
        }
      }

      else
      {
        continue;
      }
    }
  }

  return array_values($routes);
}

?>
