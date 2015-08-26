<?php

/*
 *  A simple class for doing raw IRRd queries. This was originally done with 
 *  IRRToolSet, but we were forced to whip this up after not being able to 
 *  find any network admin who could actually manage to successfully compile 
 *  IRRToolSet out of the box. :)
 * 
 *  Also includes a few functions to make processing the class easier.
 *
 */

 /*
 * {
 *   $routes = $irr->get_v4_routes_by_origin($object);
 *   if ($routes == FALSE) {
 *     $data = $irr->get_data_by_set($object);
 *     $routes = array_merge($routes, $data['routes4']);
 *   }
 * }
 */

/* Figure out if we have an AUT-NUM or an AS-SET, and resolve it for v4 addresses */
function resolve_v4($irr, $object)
{
  $asnlist = array();
  $routes4 = $irr->get_v4_routes_by_origin($object);
  if ($routes4 == FALSE)
  {
    if (($data = $irr->get_data_by_set($object, '4' )) != FALSE)
    {
      $routes4 = $data['routes4'];
      $asnlist = $data['asns'];
    }
  }
  else
  {
    $asnlist[0] = $object;
  }

  if ($routes4 == FALSE) {
    status(STATUS_NOTICE, "No registered v4 routes found for this AS.");
  }

  $result = array($routes4, $asnlist);
  return $result;
}

/* Figure out if we have an AUT-NUM or an AS-SET, and resolve it for v6 addresses */
function resolve_v6($irr, $object)
{
  $asnlist = array();
  $routes6 = $irr->get_v6_routes_by_origin($object);
  if ($routes6 == FALSE)
  {
    if (($data = $irr->get_data_by_set($object, '6')) != FALSE)
    {
      $routes6 = $data['routes6'];
      $asnlist = $data['asns'];
    }
  }
  else
  {
    $asnlist[0] = $object;
  }

  if($routes6 == FALSE) {
    status(STATUS_NOTICE, "No registered v6 routes found for this AS.");
  }

  $result = array($routes6, $asnlist);
  return $result;
}


class IRRQuery {
  var $host;
  var $port;
  var $fp;

  var $caching;
  var $cache;

  var $lastcommand;

  function __construct()
  {
    $this->caching = FALSE;
  }

  function _multi_mode()
  {
    $this->_send("!!\n");

    return TRUE;
  }

  function _identify()
  {
    $this->_send("!nIRRPowerTools\n");
    $this->_response();

    return TRUE;
  }

  function _send($output)
  {
    $this->lastcommand = rtrim($output);

    if (fwrite($this->fp, $output) == FALSE) {
      status(STATUS_ERROR, "IRR Query - Unexpected write() error on socket.");
      return FALSE;
    }

    return TRUE;
  }

  function _readline()
  {
    if (($data = fgets($this->fp, 256)) == FALSE) {
      status(STATUS_ERROR, "IRR Query - Unexpected read() error on socket.");
      return FALSE;
    }

    return $data;
  }

  function _response()
  {
    $data = "";
    $datalen = 0;

    $header = rtrim($this->_readline());

    switch ($header[0]) {
      case "C":
        return TRUE;
      case "D":
        status(STATUS_WARNING, "IRR Query - key not found - Query: " . $this->lastcommand);
        return FALSE;
      case "E":
        return TRUE;
      case "F":
        status(STATUS_WARNING, "Query ({$this->lastcommand}) failed:" . substr($header, 1));
        return FALSE;
    }

    $header = rtrim($header);

    if ($header[0] == 'A')
      $datalen = intval(substr($header,1));
    else {
      status(STATUS_ERROR, "IRR Query - Parse error looking for data length.");
      return FALSE;
    }

    while (strlen($data) < $datalen)
      $data .= $this->_readline();

    if ($datalen != strlen($data))
      status(STATUS_ERROR, "IRR Query - Data read doesn't match expected length.");

    $footer = $this->_readline();

    return rtrim($data);
  }

  /*
  * Not sure why this is a good idea, but IRR still lets people register
  * classful routes with no /prefixlength. Scan for this and fix it.
  */
  function _classful_fix($routes)
  {
    for ($i=0;$i<sizeof($routes);$i++) {
      if (strrpos($routes[$i], "/") == FALSE) {
        $octets = explode(".", $routes[$i]);
        $first = intval($octets[0]);

        if (($first) > 0 && ($first < 128))
          $length = "/8";
        else if (($first) > 0 && ($first < 192))
          $length = "/16";
        else if (($first) > 0 && ($first < 255))
          $length = "/24";
        else {
          status(STATUS_ERROR, "Invalid prefix: {$routes[$i]}");
          unset($routes[$i]);
        }

        $routes[$i] .= $length;
      }
    }

    return $routes;
  }

  function connect($host, $port)
  {
    $this->host = $host;
    $this->port = $port;

    $this->fp = fsockopen($host, $port, $errno, $errstr, 30);

    if (!$this->fp) {
      status_error(STATUS_ERROR, "{$errstr} ({$errno})");
      return FALSE;
    }

    $this->_multi_mode();
    $this->_identify();

    return $this;
  }

  function close()
  {
    $this->_send("!q\n");

    fclose($this->fp);
  }

  function set_sources($sources)
  {
    $this->_send("!s-{$sources}\n");

    if (($results = $this->_response()) == FALSE) {
      return FALSE;
    }

    return explode(" ", $results);
  }

  function get_sources()
  {
    $this->_send("!s-lc\n");

    if (($results = $this->_response()) == FALSE) {
      return FALSE;
    }

    return $results;
  }

  function cache_set($mode)
  {
    if ($mode) {
      $this->caching = TRUE;
    } else {
      $this->caching = FALSE;
    }
  }

  function cache_clear()
  {
    unset($this->cache);
  }


  function _cache_query($type, $record)
  {
    if ($this->caching) {
      if (isset($this->cache[$type][$record])) {
        /* printf("Cache hit: %s\n", $record); */
        $this->cache[$type][$record]['hits']++;

        return $this->cache[$type][$record]['data'];
      }
    }

    return FALSE;
  }

  function _cache_add($type, $record, $data)
  {
    $this->cache[$type][$record]['data'] = $data;
    $this->cache[$type][$record]['time'] = time();
    $this->cache[$type][$record]['hits'] = 0;
  }

  function get_v4_routes_by_origin($origin)
  {

    /* Prepend 'AS' to set if not provided */

    if (preg_match("/^AS./", $origin) == 0) 
    {
      $origin = 'AS'.$origin;
    }

    /* Anything cached? */

    if (($results = $this->_cache_query("origin", $origin)) != FALSE)
      return $results;

    /* Get v4 prefixes */

    $this->_send("!g{$origin}\n");
    if (($results = $this->_response()) == FALSE)
    {
      return FALSE;
    }

    else 
    {
      $results = $this->_classful_fix(explode(" ", $results));

      natsort($results);
      $results = array_values(array_unique($results));

      if ($this->caching)
        $this->_cache_add("origin", $origin, $results);
    }
    return $results;
  }

  function get_v6_routes_by_origin($origin)
  {

    /* Prepend 'AS' to set if not provided */

    if (preg_match("/^AS./", $origin) == 0) 
    {
      $origin = 'AS'.$origin;
    }

    /* Anything cached? */

    if (($results = $this->_cache_query("origin6", $origin)) != FALSE)
      return $results;

    /* Get v6 prefixes */

    $this->_send("!6{$origin}\n");
    if (($results = $this->_response()) == FALSE)
    {
      return FALSE;
    }

    else 
    {
      $results = explode(" ", $results);

      $results = array_map('strtolower', $results);
      natsort($results);
      $results = array_values(array_unique($results));

      if ($this->caching)
        $this->_cache_add("origin6", $origin, $results);
    }
    return $results;
  }

  function get_data_by_set($set, $version)
  {
    $routes4 = array();
    $routes6 = array();
    $results4= array();
    $results6 = array();

    $stats['missing_autnum']  = 0;
    $stats['missing_autnum_v6']  = 0;

    /* XXX - This should be caching results, maybe? */

    /* Prepend 'AS' to set if not provided */
    if (preg_match("/^AS./", $set) == 0) 
    {
      $set = 'AS'.$set;
    }

    /* Query the set data */
    $this->_send("!i{$set},1\n");
    if (($response = $this->_response()) == FALSE)
      return FALSE;
    $autnumlist = explode(" ", $response);

    /* Detect route-set, otherwise assume as-set or autnum */
    if ((strchr($response, '.') != FALSE) && (stristr($response, "AS") == FALSE)) {
      $routes4 = $this->_classful_fix($autnumlist);
    } 
    else 
    {
      for ($i = 0; $i < sizeof($autnumlist); $i++) 
      {
        if($version == '4')
        {
          $results4 = $this->get_v4_routes_by_origin($autnumlist[$i]);
          if ($results4 == FALSE) 
          {
            $stats['missing_autnum']++;
            continue;
          }
        }
        if($version == '6')
        {
          $results6 = $this->get_v6_routes_by_origin($autnumlist[$i]);
          if ($results6 == FALSE) 
          {
            $stats['missing_autnum_v6']++;
            continue;
          }
        }

        $routes4 = array_merge($routes4, $results4);
        $routes6 = array_merge($routes6, $results6);
      }
    }
    natsort($routes4);
    natsort($routes6);

    $results['routes4'] = array_values(array_unique($routes4));
    $results['routes6'] = array_values(array_unique($routes6));

    $results['asns']   = array_values(array_unique($autnumlist));
    $results['stats']  = $stats;

    return $results;
  }

  function get_members_by_set($set, $recursive)
  {
    $routes = array();

    /* Prepend 'AS' to set if not provided */
    if (preg_match("/^AS./", $set) == 0) 
    {
      $set = 'AS'.$set;
    }

    /* Query the set data */
    if ($recursive == TRUE)
      $this->_send("!i{$set},1\n");
    else
      $this->_send("!i{$set}\n");

    if (($response = $this->_response()) == FALSE)
      return FALSE;

    $data = explode(" ", $response);

    $data = array_values(array_unique($data));
    return $data;
  }

}

?>
