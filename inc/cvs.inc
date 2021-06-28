<?php

/* Cheesey class for our CVS access */
class CVS {
    var $cvsroot;

    function init($cvsroot)
    {
        $this->cvsroot = $cvsroot;
        putenv("CVSROOT={$cvsroot}");
    }

    function get_diff($file, $rev)
    {
        global $cfg;

        $results = "";

        if (!file_exists($cfg['tools']['cvs']) || !is_executable($cfg['tools']['cvs']))
            return FALSE;

        if (!file_exists($file) || !is_readable($file))
            return FALSE;

        $cmd  = "{$cfg['tools']['cvs']} diff {$cfg['diff']['params']} ";
        $cmd .= "-r {$rev['old']} -r {$rev['new']} {$file}";

        if (!($cvscmd = popen($cmd, "r")))
            return FALSE;

        $format = strtolower($cfg['diff']['output_format']);

        while (!feof($cvscmd)) {
            $line = fgets($cvscmd, 256);
            $line = str_replace($cfg['paths']['base'], "", $line);

            if ($format != "fulldiff") {
                $pfx = substr($line, 0, 1);

                if (($pfx != "+") && ($pfx != "-"))
                    continue;
            }

            if ($format == "english") {
                if (substr($line, 0, 3) == "+++") {
                    $line = str_replace("+++", "New Revision", $line);
                } else if (substr($line, 0, 3) == "---") {
                    $line = str_replace("---", "Old Revision", $line);
                } else {
                    $line = str_replace("+", "Add ", $line);
                    $line = str_replace("-", "Remove ", $line);
                }
            }

            $results .= $line;
        }

        fclose($cvscmd);

        return $results;
    }

    function get_complete($file)
    {
        $results = "";

        if (!($routes = fopen($file, "r")))
            return FALSE;

        while (!feof($routes)) {
            $results .= fgets($routes, 256);
        }

        fclose($routes);

        return $results;
    }

    function update($file)
    {
        global $cfg;

        if (!file_exists($cfg['tools']['cvs']) || !is_executable($cfg['tools']['cvs']))
            return FALSE;

        chdir($cfg['paths']['db']);

        /* CVS add */
        if (!($results = popen("{$cfg['tools']['cvs']} add ${file} 2>&1", "r")))
            return FALSE;

        while (!feof($results)) {
            $line = rtrim(fgets($results, 256));

            if (strstr($line, "already exists")) {
                $tmp = explode(" ", $line);
                $rev['old'] = $tmp[8];
                $rev['new'] = $tmp[8];
            }
        }

        pclose($results);



        /* CVS commit */
        if (!($results = popen("{$cfg['tools']['cvs']} commit -m update {$file}", "r")))
            return FALSE;

        while (!feof($results)) {
            $line = rtrim(fgets($results, 256));

            if (strstr($line, "initial revision")) {
                $tmp = explode(":", $line);
                $rev['old'] = "0.00";
                $rev['new'] = trim($tmp[1]);

                return $rev;
            }

            if (strstr($line, "new revision")) {
                $tmp = explode(":", $line);
                $rev['old'] = trim($tmp[2]);
                $tmp = explode(";", $tmp[1]);
                $rev['new'] = trim($tmp[0]);

                return $rev;
            }
        }

        pclose($results);

        return $rev;
    }
}

?>
