<?php

/*
 *  A simple class for doing raw IRRd queries. This was originally done with 
 *  IRRToolSet, but we were forced to whip this up after not being able to 
 *  find any network admin who could actually manage to successfully compile 
 *  IRRToolSet out of the box. :)
 */
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

  function get_routes_by_origin($origin)
  {
    if (($results = $this->_cache_query("origin", $origin)) != FALSE)
      return $results;

    $this->_send("!g{$origin}\n");

    if (($results = $this->_response()) == FALSE)
      return FALSE;

    $results = $this->_classful_fix(explode(" ", $results));

    natsort($results);
    $results = array_values(array_unique($results));

    if ($this->caching)
      $this->_cache_add("origin", $origin, $results);

    return $results;
  }

  function get_data_by_set($set)
  {
    $routes = array();
    $stats['missing_autnum']  = 0;

    /* XXX - This should be caching results, maybe? */

    /* Query the set data */
    $this->_send("!i{$set},1\n");
    if (($response = $this->_response()) == FALSE)
      return FALSE;
    $autnumlist = explode(" ", $response);

    /* Detect route-set, otherwise assume as-set or autnum */
    if ((strchr($response, '.') != FALSE) && (stristr($response, "AS") == FALSE)) {
      $routes = $this->_classful_fix($autnumlist);
    } else {
      for ($i = 0; $i < sizeof($autnumlist); $i++) {
        $results = $this->get_routes_by_origin($autnumlist[$i]);

        if ($results == FALSE) {
          $stats['missing_autnum']++;
          continue;
        }

        $routes = array_merge($routes, $results);
      }
    }
    natsort($routes);

    $results['routes'] = array_values(array_unique($routes));
    $results['asns']   = array_values(array_unique($autnumlist));
    $results['stats']  = $stats;

    return $results;
  }

  function get_members_by_set($set, $recursive)
  {
    $routes = array();

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
