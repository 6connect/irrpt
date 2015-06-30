<?

$exclusions = Array();

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
  /* EJ: ...oh dear, what did I get myself into... */

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
       
        /* apply the mask for prefix b on both, the current prefix and the one from the ecludes, to get the network portion  */ 
        $a = _v6_to_network($pfx_a[0], $pfx_b[1] );
        $b = _v6_to_network($pfx_b[0], $pfx_b[1]);

        /* If both are identical, so the current prefix fits into the to be excluded one, we take the route out */
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

  /* reindex array and return */
  return array_values($routes);
}

?>
