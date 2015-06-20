<?

$currentpath = dirname(realpath(__FILE__));
require("{$currentpath}/../conf/irrpt.conf");

function aggregate_routes($routefile, $aggfile)
{

  global $cfg;

  /* Aggregate the route file with an external tool */
  if (!file_exists($cfg['tools']['aggregate']) || !is_executable($cfg['tools']['aggregate'])) {
    status(STATUS_ERROR, "Can not open aggregate tool, aborting.");
    exit(1);
  }
  $cmd  = "{$cfg['tools']['aggregate']} -q {$cfg['aggregate']['params']}";
  $cmd .= " < {$routefile} > {$aggfile}";
  system($cmd);

  /*
  ## Sort the IP addresses

  ## Handle overlaps

  ## Keep aggregating until we don't change anything

  ## Print out the IP addresses to a file 
  */

}

?>
