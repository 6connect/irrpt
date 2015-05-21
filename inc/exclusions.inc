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
		return $routes;	/* Nothing in the exclusions file */

	/* This is pretty darn nasty, but it gets the job done */
	for($i=0;$i<sizeof($routes);$i++) {
		$pfx_a = explode("/", $routes[$i]);

		for($j=0;$j<sizeof($exclusions);$j++) {
			$pfx_b = $exclusions[$j];

			/* Mask both routes and look for overlaps */
			$a = ip2long($pfx_a[0]) & (0xFFFFFFFF << (32 - $pfx_b[1]));
			$b = ip2long($pfx_b[0]) & (0xFFFFFFFF << (32 - $pfx_a[1]));

			if ($a == $b) {
				status(STATUS_WARNING, "Excluded Route {$pfx_a[0]}/{$pfx_a[1]}");
				unset($routes[$i]);
			}
		}
	}

	return array_values($routes);
}

?>
